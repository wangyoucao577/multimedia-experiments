
#include "encoding.h"

int Encoding::hw_encoder_init(AVCodecContext *ctx,
                              const enum AVHWDeviceType type) {
  int err = 0;

  if ((err = av_hwdevice_ctx_create(&hw_device_ctx_, type, NULL, NULL, 0)) <
      0) {
    av_log(NULL, AV_LOG_ERROR, "Failed to create specified HW device.\n");
    return err;
  }
  ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx_);

  auto hw_frame_ctx = av_hwframe_ctx_alloc(hw_device_ctx_);
  if (!hw_frame_ctx) {
    av_log(NULL, AV_LOG_ERROR, "Failed to create hw frame context.\n");
    return -1;
  }

  auto frames_ctx = (AVHWFramesContext *)hw_frame_ctx->data;
  frames_ctx->format = ctx->pix_fmt;
  frames_ctx->sw_format = ctx->sw_pix_fmt;
  frames_ctx->width = ctx->width;
  frames_ctx->height = ctx->height;
  // frames_ctx->initial_pool_size = 32;

  auto ret = av_hwframe_ctx_init(hw_frame_ctx);
  if (ret < 0)
    return ret;

  ctx->hw_frames_ctx = av_buffer_ref(hw_frame_ctx);
  av_buffer_unref(&hw_frame_ctx);

  return err;
}

Encoding::~Encoding() { release(); }

void Encoding::Close() { release(); }

void Encoding::release() {
  Join();

  if (!frame_queue_.empty()) {
    assert(false);
    while (!frame_queue_.empty()) {
      auto f = frame_queue_.front();
      frame_queue_.pop();

      if (f.frame) {
        av_frame_free(&f.frame);
      }
    }
  }

  if (enc_ctx_) {
    for (auto i = 0; i < nb_streams_; ++i) {
      avcodec_free_context(&enc_ctx_->codec_ctx);
      av_packet_free(&enc_ctx_->pkt);
    }
    av_free(enc_ctx_);
    enc_ctx_ = nullptr;
  }
  nb_streams_ = 0;

  if (hw_device_ctx_) {
    av_buffer_unref(&hw_device_ctx_);
  }

  if (ofmt_ctx_) {
    if (!(ofmt_ctx_->oformat->flags & AVFMT_NOFILE)) {
      avio_closep(&ofmt_ctx_->pb);
    }

    avformat_free_context(ofmt_ctx_);
    ofmt_ctx_ = nullptr;
  }
}

int Encoding::Open(const AVCodecContext *v_dec_ctx,
                   const AVCodecContext *a_dec_ctx) {
  if (opened) {
    return AVERROR_OK;
  }

  auto ret = avformat_alloc_output_context2(&ofmt_ctx_, nullptr, nullptr,
                                            output_file_.c_str());
  if (ret < 0) {
    av_log(NULL, AV_LOG_ERROR, "open output failed, err: (%d)%s\n", ret,
           av_err2str(ret));
    return ret;
  }

  enc_ctx_ = (EncodingContext *)av_calloc(kMaxStreams, sizeof(EncodingContext));
  if (!enc_ctx_) {
    av_log(NULL, AV_LOG_ERROR, "alloc encoding context failed\n");
    release();
    return AVERROR(ENOMEM);
  }

  auto dec_ctxs = {v_dec_ctx, a_dec_ctx};
  for (auto dec_ctx : dec_ctxs) {
    if (!dec_ctx) {
      continue;
    }

    auto stream = avformat_new_stream(ofmt_ctx_, NULL);
    if (!stream) {
      av_log(NULL, AV_LOG_ERROR, "new output stream failed\n");
      release();
      return AVERROR_INVALIDDATA;
    }

    // currently we use the same codec for encoding
    const AVCodec *encoder = nullptr;
    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && config_ctx_ &&
        !config_ctx_->hw_encoder_name.empty()) {
      encoder = avcodec_find_encoder_by_name(
          config_ctx_->hw_encoder_name.c_str()); // hw encoder
    } else {
      encoder = avcodec_find_encoder(dec_ctx->codec_id);
    }
    if (!encoder) {
      av_log(NULL, AV_LOG_ERROR, "Necessary encoder not found\n");
      release();
      return AVERROR_INVALIDDATA;
    }

    auto enc_ctx = avcodec_alloc_context3(encoder);
    if (!enc_ctx) {
      av_log(NULL, AV_LOG_ERROR, "Failed to allocate the encoder context\n");
      release();
      return AVERROR(ENOMEM);
    }

    auto stream_index = nb_streams_;
    nb_streams_++;

    enc_ctx_[stream_index].codec_ctx = enc_ctx;
    enc_ctx_[stream_index].pkt = av_packet_alloc();
    if (!enc_ctx_[stream_index].pkt) {
      av_log(NULL, AV_LOG_ERROR, "Failed to allocate the encoder packet\n");
      release();
      return AVERROR(ENOMEM);
    }

    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
      enc_ctx->height = dec_ctx->height;
      enc_ctx->width = dec_ctx->width;
      enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;

      // set pix_fmt for software or hardware encoder
      if (config_ctx_ &&
          !config_ctx_->hw_encoder_name.empty()) { // hardware encoder
        if (config_ctx_->hwaccel_device_type == AV_HWDEVICE_TYPE_CUDA) {
          enc_ctx->pix_fmt = AV_PIX_FMT_CUDA;
          enc_ctx->sw_pix_fmt = AV_PIX_FMT_YUV420P;
        } else {
          av_log(NULL, AV_LOG_ERROR, "unsupported device type %d\n",
                 config_ctx_->hwaccel_device_type);
          return -1;
        }
      } else { // software encoder
        /* take first format from list of supported formats */
        if (encoder->pix_fmts) {
          enc_ctx->pix_fmt = encoder->pix_fmts[0];
        } else {
          enc_ctx->pix_fmt = dec_ctx->pix_fmt;
        }
      }
      av_log(NULL, AV_LOG_VERBOSE, "encode pix_fmt %d, sw_pix_fmt %d\n",
             enc_ctx->pix_fmt, enc_ctx->sw_pix_fmt);

      /* video time_base can be set to whatever is handy and supported by
       * encoder */
      enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
      av_log(NULL, AV_LOG_VERBOSE,
             "[encoding] stream %d type %s set codec time base %d/%d from "
             "decode framerate %d/%d\n",
             stream_index, av_get_media_type_string(enc_ctx->codec_type),
             enc_ctx->time_base.num, enc_ctx->time_base.den,
             dec_ctx->framerate.num, dec_ctx->framerate.den);
    } else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
      enc_ctx->bit_rate = 192000;
      enc_ctx->sample_rate = dec_ctx->sample_rate;
      enc_ctx->sample_fmt = dec_ctx->sample_fmt;
      enc_ctx->channels = dec_ctx->channels;
      enc_ctx->channel_layout = dec_ctx->channel_layout;
    } else {
      assert(false);
    }

    if (ofmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER) {
      enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (v_dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && config_ctx_ &&
        !config_ctx_->hw_encoder_name.empty() &&
        config_ctx_->hwaccel_device_type !=
            AV_HWDEVICE_TYPE_NONE) { // init hardware encoder
      ret = hw_encoder_init(enc_ctx, config_ctx_->hwaccel_device_type);
      if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "hw_encoder_init failed, err %d\n", ret);
        return -1;
      }
    }

    AVDictionary *opts = NULL;
    ret = avcodec_open2(enc_ctx, encoder, &opts);
    if (ret != 0) {
      av_log(NULL, AV_LOG_ERROR, "open codec failed, err (%d)%s\n", ret,
             av_err2str(ret));
      release();
      return ret;
    }

    ret = avcodec_parameters_from_context(stream->codecpar, enc_ctx);
    if (ret < 0) {
      av_log(NULL, AV_LOG_ERROR, "codec params to muxer failed, err (%d)%s\n",
             ret, av_err2str(ret));
      release();
      return ret;
    }
    stream->time_base = enc_ctx->time_base;
    av_log(
        NULL, AV_LOG_VERBOSE,
        "[encoding] stream %d type %s time_base %d/%d, codec time_base %d/%d\n",
        stream_index, av_get_media_type_string(enc_ctx->codec_type),
        ofmt_ctx_->streams[0]->time_base.num,
        ofmt_ctx_->streams[0]->time_base.den, enc_ctx->time_base.num,
        enc_ctx->time_base.den);

    enabled_media_types_.insert(dec_ctx->codec_type);
  }

  if (!(ofmt_ctx_->oformat->flags & AVFMT_NOFILE)) {
    ret = avio_open(&ofmt_ctx_->pb, output_file_.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) {
      av_log(NULL, AV_LOG_ERROR,
             "Could not open output file '%s', err (%d)%s\n",
             output_file_.c_str(), ret, av_err2str(ret));
      release();
      return ret;
    }
  }

  /* init muxer, write output file header */
  ret = avformat_write_header(ofmt_ctx_, NULL);
  if (ret < 0) {
    av_log(NULL, AV_LOG_ERROR,
           "Error occurred when opening output file, err (%d)%s\n", ret,
           av_err2str(ret));
    release();
    return ret;
  }

  for (int i = 0; i < nb_streams_; ++i) {
    av_log(
        NULL, AV_LOG_VERBOSE,
        "[encoding] after avformat_write_header, stream %d type %s time_base "
        "%d/%d\n",
        i,
        av_get_media_type_string(ofmt_ctx_->streams[i]->codecpar->codec_type),
        ofmt_ctx_->streams[i]->time_base.num,
        ofmt_ctx_->streams[i]->time_base.den);
  }

  opened = true;
  return AVERROR_OK;
}

void Encoding::DumpInputFormat() const {
  if (!opened) {
    return;
  }
  av_dump_format(ofmt_ctx_, 0, output_file_.c_str(), 1);
}

void Encoding::Join() {
  if (!t_.joinable()) {
    return;
  }
  t_.join();
}

int Encoding::RunAsync() {
  if (!opened) {
    return AVERROR_OK;
  }
  t_ = std::thread(&Encoding::run, this);

  return AVERROR_OK;
}

int Encoding::pushFrame(const AVFrame *frame, AVMediaType media_type) {
  while (true) {
    std::unique_lock<std::mutex> mtx(mtx_);

    if (config_ctx_ && config_ctx_->max_cache_frames > 0) {
      if (frame_queue_.size() >= config_ctx_->max_cache_frames) {
        mtx.unlock();
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
        continue;
      }
    }

    auto new_f = av_frame_clone(frame);
    frame_queue_.emplace(AVFrameWithMediaType{new_f, media_type});
    break;
  }
  cv_.notify_one();

  return AVERROR_OK;
}

int Encoding::SendFrame(const AVFrame *frame, AVMediaType media_type) {
  auto it = enabled_media_types_.find(media_type);
  if (it == enabled_media_types_.end()) {
    return AVERROR_OK; // ignore disabled media type
  }
  return pushFrame(frame, media_type);
}

int Encoding::run() {
  int finished_streams = 0;

  auto last_time = std::chrono::high_resolution_clock::now();

  while (finished_streams != nb_streams_) {
    AVFrameWithMediaType new_frame;
    {
      std::unique_lock<std::mutex> lk(mtx_);
      if (frame_queue_.empty()) {
        using namespace std::chrono_literals;
        cv_.wait_for(lk, 50ms);
      }

      auto curr_time = std::chrono::high_resolution_clock::now();
      auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                             curr_time - last_time)
                             .count();
      if (duration_ms >= 1000) {
        av_log(NULL, AV_LOG_VERBOSE, "frame queue size %lu\n",
               frame_queue_.size());
        last_time = curr_time;
      }

      if (frame_queue_.empty()) {
        continue; // maybe empty even if notified
      }

      new_frame = frame_queue_.front();
      frame_queue_.pop();
    }

    int stream_index = findEncodingContextIndex(new_frame.media_type);
    if (stream_index < 0) {
      //   av_log(NULL, AV_LOG_WARNING, "ignore media type %s\n",
      //          av_get_media_type_string(new_frame.media_type));
      if (new_frame.frame) {
        av_frame_free(&new_frame.frame);
      }
      continue;
    }
    auto &enc_ctx = enc_ctx_[stream_index];

    if (new_frame.frame) { // convert to encoder time base,
                           // nvenc may output dts<pts without this
      new_frame.frame->pts =
          av_rescale_q(new_frame.frame->pts, kFundamentalTimeBase,
                       enc_ctx.codec_ctx->time_base);
      new_frame.frame->pkt_dts =
          av_rescale_q(new_frame.frame->pkt_dts, kFundamentalTimeBase,
                       enc_ctx.codec_ctx->time_base);
      new_frame.frame->pkt_duration =
          av_rescale_q(new_frame.frame->pkt_duration, kFundamentalTimeBase,
                       enc_ctx.codec_ctx->time_base);
    }

    auto ret = avcodec_send_frame(enc_ctx.codec_ctx, new_frame.frame);
    if (new_frame.frame) {
      if (new_frame.frame->buf[0]) {
        enc_ctx.in_count++; // ignore blank frame
      }

      av_frame_free(&new_frame.frame);
    }
    if (ret < 0) {
      av_log(NULL, AV_LOG_WARNING, "send frame failed, err (%d)%s\n", ret,
             av_err2str(ret));
      //   if (error_callback_) {
      //     error_callback_(ret);
      //   }
      return ret;
    }

    ret = receive_packets(stream_index, enc_ctx);
    assert(ret != AVERROR_OK);
    if (ret == AVERROR(EAGAIN)) {
      if (enc_ctx.out_count == 0) {
        av_log(NULL, AV_LOG_VERBOSE,
               "[Encoding] stream %d type %s no packet available, curr in %d, "
               "fill in "
               "more data and try "
               "again later\n",
               stream_index, av_get_media_type_string(new_frame.media_type),
               enc_ctx.in_count);
      }
    } else if (ret == AVERROR_EOF) {
      av_log(NULL, AV_LOG_INFO,
             "[Encoding] stream %d type %s encoder has been flushed\n",
             stream_index, av_get_media_type_string(new_frame.media_type));
      ++finished_streams;
      continue;
    } else {
      av_log(NULL, AV_LOG_ERROR,
             "stream %d receive frame failed unexpectly, err (%d)%s\n",
             stream_index, ret, av_err2str(ret));
      //   if (error_callback_) {
      //     error_callback_(ret);
      //   }
      return ret;
    }
  }

  auto ret = av_write_trailer(ofmt_ctx_);
  if (ret != AVERROR_OK) {
    av_log(NULL, AV_LOG_ERROR, "[Encoding] write trailer failed, (%d)%s\n", ret,
           av_err2str(ret));
    return ret;
  }

  // statistics
  for (auto i = 0; i < nb_streams_; i++) {
    if (!enc_ctx_[i].codec_ctx) {
      continue;
    }
    av_log(NULL, AV_LOG_INFO,
           "[Encoding] stream %d type %s total read frames %d, encoded "
           "packets %d\n",
           i, av_get_media_type_string(enc_ctx_[i].codec_ctx->codec_type),
           enc_ctx_[i].in_count, enc_ctx_[i].out_count);
  }

  return AVERROR_OK;
}

int Encoding::findEncodingContextIndex(AVMediaType media_type) const {
  if (!enc_ctx_) {
    return -1;
  }

  for (int i = 0; i < nb_streams_; ++i) {
    if (!enc_ctx_[i].codec_ctx) {
      continue;
    }

    if (enc_ctx_[i].codec_ctx->codec_type == media_type) {
      return i;
    }
  }

  return -1;
}

int Encoding::receive_packets(int stream_index, EncodingContext &enc_ctx) {

  auto ret = AVERROR_OK;
  do {
    ret = avcodec_receive_packet(enc_ctx.codec_ctx, enc_ctx.pkt);
    if (ret != AVERROR_OK) {
      break;
    }
    enc_ctx.out_count++;

    /* prepare packet for muxing */
    enc_ctx.pkt->stream_index = stream_index;

    av_packet_rescale_ts(enc_ctx.pkt, enc_ctx.codec_ctx->time_base,
                         ofmt_ctx_->streams[stream_index]->time_base);

    av_log(NULL, AV_LOG_VERBOSE,
           "[Encoding] stream %d type %s muxing frame count %d, pts %" PRId64
           " dts %" PRId64 ", duration %" PRId64 ", time base %d/%d\n",
           stream_index,
           av_get_media_type_string(enc_ctx.codec_ctx->codec_type),
           enc_ctx.out_count, enc_ctx.pkt->pts, enc_ctx.pkt->dts,
           enc_ctx.pkt->duration,
#if LIBAVCODEC_VERSION_MAJOR >= 59 && LIBAVCODEC_VERSION_MINOR >= 4
           enc_ctx.pkt->time_base.num, enc_ctx.pkt->time_base.den
#else
           0, 0
#endif
    );

    /* mux encoded frame */
    ret = av_interleaved_write_frame(ofmt_ctx_, enc_ctx.pkt);

    av_packet_unref(enc_ctx.pkt);

  } while (ret == AVERROR_OK);

  return ret;
}