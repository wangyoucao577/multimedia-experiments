
#include "decoding.h"

enum AVPixelFormat Decoding::get_hw_format(AVCodecContext *ctx,
                                           const enum AVPixelFormat *pix_fmts) {

  if (!config_ctx_ ||
      config_ctx_->hwaccel_device_type != AV_HWDEVICE_TYPE_CUDA) {
    av_log(NULL, AV_LOG_ERROR, "invalid hw device type %d.\n",
           config_ctx_ ? config_ctx_->hwaccel_device_type
                       : AV_HWDEVICE_TYPE_NONE);
    return AV_PIX_FMT_NONE;
  }

  const enum AVPixelFormat *p;

  for (p = pix_fmts; *p != -1; p++) {
    if (*p == hw_pix_fmt_)
      return *p;
  }

  av_log(NULL, AV_LOG_ERROR, "Failed to get HW surface format.\n");
  return AV_PIX_FMT_NONE;
}

int Decoding::hw_decoder_init(const AVCodec *dec, AVCodecContext *ctx,
                              const enum AVHWDeviceType type) {
  int err = 0;

  for (int i = 0;; i++) { // find hw config
    const AVCodecHWConfig *config = avcodec_get_hw_config(dec, i);
    if (!config) {
      av_log(NULL, AV_LOG_ERROR,
             "Decoder %s does not support device type %s.\n", dec->name,
             av_hwdevice_get_type_name(config_ctx_->hwaccel_device_type));
      return AVERROR_DECODER_NOT_FOUND;
    }
    if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
        config->device_type == config_ctx_->hwaccel_device_type) {
      hw_pix_fmt_ = config->pix_fmt;
      break;
    }
  }

  if ((err = av_hwdevice_ctx_create(&hw_device_ctx_, type, NULL, NULL, 0)) <
      0) {
    fprintf(stderr, "Failed to create specified HW device.\n");
    return err;
  }
  ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx_);
  // ctx->extra_hw_frames = 21; // try to fix the `No decoder surface left`
  // error

  return err;
}

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

  if (hw_device_ctx_) {
    av_buffer_unref(&hw_device_ctx_);
  }
  if (hw_frames_ctx_) {
    av_buffer_unref(&hw_frames_ctx_);
  }
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

      // hwaccel cuda, video  only
      if (config_ctx_ &&
          config_ctx_->hwaccel_device_type == AV_HWDEVICE_TYPE_CUDA) {

        ret = hw_decoder_init(dec, dec_ctx_[i].codec_ctx,
                              config_ctx_->hwaccel_device_type);
        if (ret < 0) {
          av_log(NULL, AV_LOG_ERROR, "hw_decoder_init failed, ret %d\n", ret);
          return ret;
        }
      }
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

    if (dec_ctx_[i].codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
      av_log(NULL, AV_LOG_INFO,
             "<decoding> stream time_base %d/%d, codec time base %d/%d, pkt "
             "time base %d/%d\n",
             stream->time_base.num, stream->time_base.den,
             dec_ctx_[i].codec_ctx->time_base.num,
             dec_ctx_[i].codec_ctx->time_base.den,
             dec_ctx_[i].codec_ctx->pkt_timebase.num,
             dec_ctx_[i].codec_ctx->pkt_timebase.den);
    }
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

    av_packet_rescale_ts(pkt_, ifmt_ctx_->streams[i]->time_base,
                         kFundamentalTimeBase); // convert to unified timebase

    av_log(NULL, AV_LOG_VERBOSE,
           "<decoding> stream %d type %s read packet pts %" PRId64
           ", dts %" PRId64 ", duration %" PRId64 ", time_base %d/%d\n",
           i, av_get_media_type_string(dec_ctx_[i].codec_ctx->codec_type),
           pkt_->pts, pkt_->dts, pkt_->duration,
#if LIBAVCODEC_VERSION_MAJOR >= 59 && LIBAVCODEC_VERSION_MINOR >= 4
           pkt_->time_base.num, pkt_->time_base.den
#else
           0, 0
#endif
    );

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
      if (dec_ctx_[i].out_count == 0) {
        av_log(NULL, AV_LOG_VERBOSE,
               "<Decoding> stream %d type %s no packet available, curr in %d, "
               "fill in "
               "more data and try "
               "again later\n",
               i, av_get_media_type_string(dec_ctx_[i].codec_ctx->codec_type),
               dec_ctx_[i].in_count);
      }
    } else if (ret == AVERROR_EOF) {
      av_log(NULL, AV_LOG_INFO,
             "<Decoding> stream %d type %s decoder has been flushed\n", i,
             av_get_media_type_string(dec_ctx_[i].codec_ctx->codec_type));
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
      av_log(NULL, AV_LOG_INFO,
             "<Decoding> stream %d type %s decoder has been flushed\n", i,
             av_get_media_type_string(dec_ctx_[i].codec_ctx->codec_type));
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
           "<Decoding> stream %d type %s total read packets %d, decoded frames "
           "%d\n",
           i, av_get_media_type_string(dec_ctx_[i].codec_ctx->codec_type),
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
    if (ret != AVERROR_OK && ret != AVERROR_EOF) {
      break;
    }

    if (ret == AVERROR_OK) {
      dec_ctx.out_count++;

      if (dec_ctx.codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO && config_ctx_ &&
          config_ctx_->hwaccel_device_type ==
              AV_HWDEVICE_TYPE_CUDA) { // hwaccel cuda

        if (dec_ctx.frame != nullptr &&
            config_ctx_->hwaccel_output_format_cuda &&
            config_ctx_->enable_cuda_frames_caching) {
          // caches frames in cuda memory, not in decoder's internal memory

          if (!hw_frames_ctx_) { // get frames ctx for
                                 // get_buffer/transfer_data
            auto src_hw_Frames_ctx_data =
                (AVHWFramesContext *)dec_ctx.frame->hw_frames_ctx->data;

            hw_frames_ctx_ = av_hwframe_ctx_alloc(hw_device_ctx_);
            if (!hw_frames_ctx_) {
              av_log(NULL, AV_LOG_ERROR, "Error av_hwframe_ctx_alloc\n");
              break;
            }

            auto hw_frames_ctx_data = (AVHWFramesContext *)hw_frames_ctx_->data;
            hw_frames_ctx_data->format = src_hw_Frames_ctx_data->format;
            hw_frames_ctx_data->sw_format = src_hw_Frames_ctx_data->sw_format;
            hw_frames_ctx_data->width = src_hw_Frames_ctx_data->width;
            hw_frames_ctx_data->height = src_hw_Frames_ctx_data->height;
            ret = av_hwframe_ctx_init(hw_frames_ctx_);
            if (ret != 0) {
              av_log(NULL, AV_LOG_ERROR, "Error av_hwframe_ctx_init, err %d\n",
                     ret);
              break;
            }
          }

          AVFrame *new_frame = av_frame_alloc();
          ret = av_hwframe_get_buffer(hw_frames_ctx_, new_frame, 0);
          if (ret != 0) {
            av_log(NULL, AV_LOG_ERROR, "Error av_hwframe_get_buffer, err %d\n",
                   ret);
            av_frame_free(&new_frame);
            break;
          }

          /* retrieve data from GPU to cache */
          if ((ret = av_hwframe_transfer_data(new_frame, dec_ctx.frame, 0)) <
              0) {
            av_log(NULL, AV_LOG_ERROR,
                   "Error transferring the data to system memory, err %d\n",
                   ret);
            av_frame_free(&new_frame);
            break;
          }

          ret = av_frame_copy_props(new_frame, dec_ctx.frame);
          if (ret < 0) {
            av_frame_free(&new_frame);
            break;
          }

          av_frame_unref(dec_ctx.frame);
          av_frame_move_ref(dec_ctx.frame, new_frame);
          av_frame_free(&new_frame);
        }
      }
    }

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
  t_ = std::thread(&Decoding::run, this);

  return AVERROR_OK;
}

const AVCodecContext *Decoding::CodecContext(AVMediaType media_type) const {
  if (!dec_ctx_ || nb_streams_ == 0) {
    return nullptr;
  }

  for (int i = 0; i < nb_streams_; ++i) {
    if (dec_ctx_[i].codec_ctx->codec_type == media_type) {
      return dec_ctx_[i].codec_ctx;
    }
  }

  return nullptr;
}
