
#include "encoding.h"

Encoding::~Encoding() { release(); }

void Encoding::Close() { release(); }

void Encoding::release() {
  if (frame_) {
    av_frame_free(&frame_);
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

  if (ofmt_ctx_) {
    avformat_close_input(&ofmt_ctx_);
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
    auto encoder = avcodec_find_encoder(dec_ctx->codec_id);
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
    enc_ctx_[nb_streams_].codec_ctx = enc_ctx;
    enc_ctx_[nb_streams_].pkt = av_packet_alloc();
    if (!enc_ctx_[nb_streams_].pkt) {
      av_log(NULL, AV_LOG_ERROR, "Failed to allocate the encoder packet\n");
      release();
      return AVERROR(ENOMEM);
    }
    ++nb_streams_;

    if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
      enc_ctx->height = dec_ctx->height;
      enc_ctx->width = dec_ctx->width;
      enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;

      /* take first format from list of supported formats */
      if (encoder->pix_fmts) {
        enc_ctx->pix_fmt = encoder->pix_fmts[0];
      } else {
        enc_ctx->pix_fmt = dec_ctx->pix_fmt;
      }

      /* video time_base can be set to whatever is handy and supported by
       * encoder */
      enc_ctx->time_base = av_inv_q(dec_ctx->framerate);
    } else if (dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
      // TODO:
      assert(false);
    } else {
      assert(false);
    }

    if (ofmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER) {
      enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = avcodec_open2(enc_ctx, encoder, nullptr);
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

  opened = true;
  return AVERROR_OK;
}

void Encoding::DumpInputFormat() const {
  if (!opened) {
    return;
  }
  av_dump_format(ofmt_ctx_, 0, output_file_.c_str(), 1);
}
