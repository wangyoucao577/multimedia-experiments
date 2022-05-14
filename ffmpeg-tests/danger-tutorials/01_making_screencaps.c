
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include <assert.h>
#include <stdio.h>

void save_frame(AVFrame *frame, int width, int height, int i) {
  char filename[32];
  sprintf(filename, "frame%d.ppm", i);

  av_log(NULL, AV_LOG_VERBOSE, "write file %s\n", filename);

  // open file
  FILE *file = fopen(filename, "wb");
  if (file == NULL) {
    av_log(NULL, AV_LOG_ERROR, "open file %s failed\n", filename);
    return;
  }

  // Write header
  fprintf(file, "P6\n%d %d\n255\n", width, height);

  // Write pixel data
  for (int y = 0; y < height; y++) {
    fwrite(frame->data[0] + y * frame->linesize[0], 1, width * 3, file);
  }
  fclose(file);
}

int main(int argc, char *argv[]) {
  av_log_set_level(AV_LOG_VERBOSE);
  if (argc < 2) {
    av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> \n", argv[0]);
    return -1;
  }
  const char *input_url = argv[1];

  AVFormatContext *input_format_ctx = NULL;
  int ret = avformat_open_input(&input_format_ctx, input_url, NULL, NULL);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "avformat_open_input %s failed, ret %d\n",
           input_url, ret);
    return -1;
  }

  ret = avformat_find_stream_info(input_format_ctx, NULL);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info failed, ret %d\n",
           ret);
    return -1;
  }
  av_dump_format(input_format_ctx, 0, input_url, 0);

  // find the first video stream
  int video_stream_index = -1;
  for (int i = 0; i < input_format_ctx->nb_streams; i++) {
    if (input_format_ctx->streams[i]->codecpar->codec_type ==
        AVMEDIA_TYPE_VIDEO) {
      video_stream_index = i;
      break;
    }
  }
  if (video_stream_index == -1) {
    av_log(NULL, AV_LOG_ERROR, "no video stream failed\n");
    return -1;
  }

  const AVCodec *codec = avcodec_find_decoder(
      input_format_ctx->streams[video_stream_index]->codecpar->codec_id);
  if (codec == NULL) {
    av_log(NULL, AV_LOG_ERROR, "find codec for %d failed\n",
           input_format_ctx->streams[video_stream_index]->codecpar->codec_id);
    return -1;
  }

  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
  assert(codec_ctx != NULL);

  ret = avcodec_parameters_to_context(
      codec_ctx, input_format_ctx->streams[video_stream_index]->codecpar);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context failed, ret %d\n",
           ret);
    return -1;
  }

  ret = avcodec_open2(codec_ctx, codec, NULL);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failed, ret %d\n", ret);
    return -1;
  }

  AVFrame *frame = av_frame_alloc();
  AVFrame *frame_rgb = av_frame_alloc();
  assert(frame != NULL && frame_rgb != NULL);
  AVPacket *pkt = av_packet_alloc();
  assert(pkt != NULL);

  ret = av_image_alloc(frame_rgb->data, frame_rgb->linesize, codec_ctx->width,
                       codec_ctx->width, AV_PIX_FMT_RGB24, 1);
  assert(ret);

  struct SwsContext *sws_ctx = sws_getContext(
      codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt, codec_ctx->width,
      codec_ctx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
  assert(sws_ctx != NULL);

  int i = 0;
  while (1) {
    ret = av_read_frame(input_format_ctx, pkt);
    if (ret < 0) {
      if (ret == AVERROR_EOF) {
        av_log(NULL, AV_LOG_VERBOSE, "EOF\n");
      } else {
        av_log(NULL, AV_LOG_ERROR, "av_read_frame failed, err %d\n", ret);
      }
      break;
    }

    if (pkt->stream_index != video_stream_index) { // only decode video
      av_packet_unref(pkt);
      continue;
    }

    ret = avcodec_send_packet(codec_ctx, pkt);
    if (ret < 0) { // TODO: will loss last frames here
      av_log(NULL, AV_LOG_WARNING, "avcodec_send_packet, err (%d)%s\n", ret,
             av_err2str(ret));
      return ret;
    }
    av_packet_unref(pkt);

    ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret == AVERROR(EAGAIN)) {
      continue;
    } else if (ret == AVERROR_EOF) {
      av_log(NULL, AV_LOG_INFO, "stream %d type %s decoder has been flushed\n",
             video_stream_index,
             av_get_media_type_string(codec_ctx->codec_type));
      break;
    } else if (ret != 0) {
      av_log(NULL, AV_LOG_ERROR,
             "stream %d receive frame failed unexpectly, err (%d)%s\n",
             video_stream_index, ret, av_err2str(ret));
      break;
    }
    i++;

    sws_scale(sws_ctx, (uint8_t const *const *)frame->data, frame->linesize, 0,
              codec_ctx->height, frame_rgb->data, frame_rgb->linesize);

    if (i % 10 == 1) {
      save_frame(frame_rgb, codec_ctx->width, codec_ctx->height, i);
    }
  }

  av_packet_free(&pkt);
  av_frame_free(&frame);
  av_frame_free(&frame_rgb);
  avcodec_close(codec_ctx);
  avcodec_free_context(&codec_ctx);
  avformat_close_input(&input_format_ctx);
  return 0;
}