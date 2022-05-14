
#include "SDL.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  av_log_set_level(AV_LOG_VERBOSE);
  if (argc < 2) {
    av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> \n", argv[0]);
    return -1;
  }
  const char *input_url = argv[1];

  // init SDL
  int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "SDL_Init failed, err %d\n", ret);
    return -1;
  }

  AVFormatContext *input_format_ctx = NULL;
  ret = avformat_open_input(&input_format_ctx, input_url, NULL, NULL);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "avformat_open_input %s failed, ret %d\n",
           input_url, ret);
    goto exit;
  }

  ret = avformat_find_stream_info(input_format_ctx, NULL);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "avformat_find_stream_info failed, ret %d\n",
           ret);
    goto exit;
  }
  // av_dump_format(input_format_ctx, 0, input_url, 0);

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
    goto exit;
  }

  const AVCodec *codec = avcodec_find_decoder(
      input_format_ctx->streams[video_stream_index]->codecpar->codec_id);
  if (codec == NULL) {
    av_log(NULL, AV_LOG_ERROR, "find codec for %d failed\n",
           input_format_ctx->streams[video_stream_index]->codecpar->codec_id);
    goto exit;
  }

  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
  assert(codec_ctx != NULL);

  ret = avcodec_parameters_to_context(
      codec_ctx, input_format_ctx->streams[video_stream_index]->codecpar);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "avcodec_parameters_to_context failed, ret %d\n",
           ret);
    goto exit;
  }

  ret = avcodec_open2(codec_ctx, codec, NULL);
  if (ret != 0) {
    av_log(NULL, AV_LOG_ERROR, "avcodec_open2 failed, ret %d\n", ret);
    goto exit;
  }

  AVFrame *frame = av_frame_alloc();
  assert(frame != NULL);
  AVPacket *pkt = av_packet_alloc();
  assert(pkt != NULL);

  SDL_Window *screen = SDL_CreateWindow(
      "My Video Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      codec_ctx->width, codec_ctx->height,
      //   SDL_WINDOW_FULLSCREEN |
      SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
  assert(screen);
  SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, 0);
  assert(renderer);
  // Allocate a place to put our YUV image on that screen
  SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           codec_ctx->width, codec_ctx->height);
  assert(texture);

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

    SDL_UpdateYUVTexture(texture, NULL, frame->data[0], frame->linesize[0],
                         frame->data[1], frame->linesize[1], frame->data[2],
                         frame->linesize[2]);
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
      break;
    } else {
      // av_log(NULL, AV_LOG_VERBOSE, "SDL event type %u\n", event.type);
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  }

exit:
  if (pkt) {
    av_packet_free(&pkt);
  }
  if (frame) {
    av_frame_free(&frame);
  }
  avcodec_close(codec_ctx);
  if (codec_ctx) {
    avcodec_free_context(&codec_ctx);
  }
  if (input_format_ctx) {
    avformat_close_input(&input_format_ctx);
  }
  SDL_Quit();
  return 0;
}