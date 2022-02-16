
#include "decoding.h"

#include <thread>

Decoding::~Decoding() {
  release(); // double check to make sure resources can be released properly
}

void Decoding::release() {
  Join();

  if (pkt_) {
    av_packet_free(&pkt_);
  }

  if (dec_ctx_) {
    for (auto i = 0; i < nb_streams_; ++i) {
      avcodec_free_context(&dec_ctx_->codec_ctx);
      av_frame_free(&dec_ctx_->frame);
    }
    av_free(dec_ctx_);
    dec_ctx_ = nullptr;
  }
  nb_streams_ = 0;

  if (ifmt_ctx_) {
    avformat_close_input(&ifmt_ctx_);
  }
}

void Decoding::Close() { release(); }

void Decoding::DumpInputFormat() const {
  if (!opened) {
    return;
  }

  av_dump_format(ifmt_ctx_, 0, input_file_.c_str(), 0);
}

int Decoding::Open() {
  if (opened) {
    return AVERROR_OK;
  }

  auto ret = avformat_open_input(&ifmt_ctx_, input_file_.c_str(), NULL, NULL);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "open input failed, err: (%d)%s\n", ret,
           av_err2str(ret));
    return ret;
  }
  assert(ifmt_ctx_ != nullptr);

  ret = avformat_find_stream_info(ifmt_ctx_, NULL);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "find input stream info failed, err: (%d)%s\n",
           ret, av_err2str(ret));
    release();
    return ret;
  }

  nb_streams_ = ifmt_ctx_->nb_streams;
  assert(nb_streams_ > 0);

  dec_ctx_ = (DecodingContext *)av_calloc(nb_streams_, sizeof(DecodingContext));
  if (!dec_ctx_) {
    av_log(NULL, AV_LOG_ERROR, "alloc decoding context failed\n");
    release();
    return AVERROR(ENOMEM);
  }

  for (auto i = 0; i < nb_streams_; ++i) {
    auto stream = ifmt_ctx_->streams[i];
    assert(stream);

    if (stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
        stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
      av_log(NULL, AV_LOG_WARNING, "ignore non A/V stream %d, type (%d)%s\n", i,
             stream->codecpar->codec_type,
             av_get_media_type_string(stream->codecpar->codec_type));
      continue;
    }

    auto dec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!dec) {
      av_log(NULL, AV_LOG_ERROR, "no valid decoder for %d(%s)\n",
             stream->codecpar->codec_id,
             avcodec_get_name(stream->codecpar->codec_id));
      release();
      return AVERROR_DECODER_NOT_FOUND;
    }
    av_log(NULL, AV_LOG_INFO, "valid decoder %s for stream %d\n", dec->name, i);

    dec_ctx_[i].codec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx_[i].codec_ctx) {
      av_log(NULL, AV_LOG_ERROR, "avcodec alloc context failed\n");
      release();
      return AVERROR(ENOMEM);
    }

    ret =
        avcodec_parameters_to_context(dec_ctx_[i].codec_ctx, stream->codecpar);
    if (ret < 0) {
      av_log(NULL, AV_LOG_ERROR,
             "avcodec parameters to context failed, err (%d)%s\n", ret,
             av_err2str(ret));
      release();
      return ret;
    }

    if (dec_ctx_[i].codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
      dec_ctx_[i].codec_ctx->framerate =
          av_guess_frame_rate(ifmt_ctx_, stream, NULL);
    }

    AVDictionary *opts = NULL;
    // av_dict_set(&opts, "threads", "auto", 0);
    ret = avcodec_open2(dec_ctx_[i].codec_ctx, dec, &opts);
    if (ret != 0) {
      av_log(NULL, AV_LOG_ERROR, "open codec failed, err (%d)%s\n", ret,
             av_err2str(ret));
      release();
      return ret;
    }
    dec_ctx_[i].frame = av_frame_alloc();
    assert(dec_ctx_[i].frame);
  }
  pkt_ = av_packet_alloc();
  assert(pkt_);

  opened = true;
  return AVERROR_OK;
}

int Decoding::Run() {
  if (!opened) {
    av_log(NULL, AV_LOG_ERROR, "decoding has NOT been opened yet\n");
    return AVERROR_UNKNOWN;
  }

  return run(); // run in sync mode
}

int Decoding::run() {
  auto ret = AVERROR_OK;

  while (true) {
    ret = av_read_frame(ifmt_ctx_, pkt_);
    if (ret < 0) {
      if (ret == AVERROR_EOF) {
        break; // all packets have been consumed
      }

      av_log(NULL, AV_LOG_WARNING, "read frame failed, err (%d)%s\n", ret,
             av_err2str(ret));
      if (error_callback_) {
        error_callback_(ret);
      }
      return ret;
    }

    int i = pkt_->stream_index;
    if (!dec_ctx_[i].codec_ctx) {
      // av_log(NULL, AV_LOG_ERROR, "stream %d is not invalid\n", i);
      av_packet_unref(pkt_);
      continue; // ignored stream
    }

    ret = avcodec_send_packet(dec_ctx_[i].codec_ctx, pkt_);
    dec_ctx_[i].in_count++;
    av_packet_unref(pkt_); // pkt_ always requires `unref` after use
    if (ret < 0) {
      av_log(NULL, AV_LOG_WARNING, "send packet failed, err (%d)%s\n", ret,
             av_err2str(ret));
      if (error_callback_) {
        error_callback_(ret);
      }
      return ret;
    }

    ret = receive_frames(i);
    assert(ret != AVERROR_OK);
    if (ret == AVERROR(EAGAIN)) {
      // av_log(NULL, AV_LOG_INFO, "no frame available, fill in more data and
      // try again later\n");
    } else if (ret == AVERROR_EOF) {
      av_log(NULL, AV_LOG_INFO, "stream %d decoder has been flushed\n", i);
      break;
    } else {
      av_log(NULL, AV_LOG_ERROR,
             "stream %d receive frame failed unexpectly, err (%d)%s\n", i, ret,
             av_err2str(ret));
      if (error_callback_) {
        error_callback_(ret);
      }
      return ret;
    }
  }

  for (auto i = 0; i < nb_streams_; ++i) {
    if (!dec_ctx_[i].codec_ctx) {
      continue;
    }

    auto ret = avcodec_send_packet(dec_ctx_[i].codec_ctx,
                                   nullptr); // notify to flush decoder
    if (ret < 0) {
      av_log(NULL, AV_LOG_WARNING,
             "notify to flush decoder failed, err (%d)%s\n", ret,
             av_err2str(ret));
      if (error_callback_) {
        error_callback_(ret);
      }
      return ret;
    }

    ret = receive_frames(i);
    assert(ret != AVERROR_OK && ret != AVERROR(EAGAIN));
    if (ret == AVERROR_EOF) {
      av_log(NULL, AV_LOG_INFO, "stream %d decoder has been flushed\n", i);
      continue;
    } else {
      av_log(NULL, AV_LOG_ERROR,
             "stream %d receive frame failed unexpectly, err (%d)%s\n", i, ret,
             av_err2str(ret));
      if (error_callback_) {
        error_callback_(ret);
      }
      return ret;
    }
  }

  // statistics
  for (auto i = 0; i < nb_streams_; i++) {
    if (!dec_ctx_[i].codec_ctx) {
      continue;
    }
    av_log(NULL, AV_LOG_INFO,
           "stream %d total read frames %d, decoded frames %d\n", i,
           dec_ctx_[i].in_count, dec_ctx_[i].out_count);
  }

  return AVERROR_OK;
}

int Decoding::receive_frames(int stream_index) {
  assert(stream_index >= 0 && stream_index < nb_streams_);
  auto &dec_ctx = dec_ctx_[stream_index];

  auto ret = AVERROR_OK;
  do {
    ret = avcodec_receive_frame(dec_ctx.codec_ctx, dec_ctx.frame);
    if (ret != AVERROR_OK) {
      break;
    }

    dec_ctx.out_count++;

    // callback
    if (data_callback_) {
      data_callback_(stream_index, dec_ctx.codec_ctx->codec_type,
                     dec_ctx.frame);
    }

    av_frame_unref(dec_ctx.frame);

  } while (ret == AVERROR_OK);

  return ret;
}

void Decoding::Join() {
  if (t_.joinable()) {
    t_.join();
  }
}

int Decoding::RunAsync(std::function<ErrorCallback> error_callback) {
  if (!opened) {
    return AVERROR_OK;
  }
  error_callback_ = std::move(error_callback);
  t_ = std::move(std::thread(&Decoding::run, this));

  return AVERROR_OK;
}