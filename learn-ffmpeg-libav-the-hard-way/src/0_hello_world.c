#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdio.h>

// decode packets into frames
static int decode_packet(AVPacket *v_packet, AVCodecContext *v_codec_context, AVFrame *v_frame);

// save a frame into a .pgm file
static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename);


int main(int argc, const char* argv[]) {
    if (argc < 2) {
        printf("You need to specify a media file.\n");
        return -1;
    }

    AVFormatContext *format_context = NULL;
    int err = avformat_open_input(&format_context, argv[1], NULL, NULL);
    if (err != 0) {
        printf("Could not open the file %s, err %d\n", argv[1], err);
        return -1;
    }

    printf("Format %s, duration %lld us, bit_rate %lld\n", format_context->iformat->name, format_context->duration, format_context->bit_rate);

    err = avformat_find_stream_info(format_context,  NULL);
    if (err < 0) {
        printf("Could not get the stream info, err %d\n", err);
        avformat_close_input(&format_context);
        return -1;
    }

    AVCodec *v_codec = NULL;
    AVCodecParameters *v_codec_params =  NULL;
    int v_stream_index = -1;
    for (int i = 0; i < format_context->nb_streams; i++) {
        AVCodecParameters *local_codec_params =  NULL;
        local_codec_params = format_context->streams[i]->codecpar;
        printf("AVStream->time_base before open coded %d/%d\n", format_context->streams[i]->time_base.num, format_context->streams[i]->time_base.den);
        printf("AVStream->r_frame_rate before open coded %d/%d\n", format_context->streams[i]->r_frame_rate.num, format_context->streams[i]->r_frame_rate.den);
        printf("AVStream->start_time %" PRId64 "\n", format_context->streams[i]->start_time);
        printf("AVStream->duration %" PRId64 "\n", format_context->streams[i]->duration);

        printf("finding the proper decoder (CODEC) for codec_id %d\n", local_codec_params->codec_id);

        AVCodec *local_codec = NULL;

        // finds the registered decoder for a codec ID
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
        local_codec = avcodec_find_decoder(local_codec_params->codec_id);
        if (local_codec==NULL) {
            printf("Unsupported codec for codec_id %d!\n", local_codec_params->codec_id);
            // In this example if the codec is not found we just skip it
            continue;
        }

        // when the stream is a video we store its index, codec parameters and codec
        if (local_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (v_stream_index == -1) {
                v_stream_index = i;
                v_codec = local_codec;
                v_codec_params = local_codec_params;
            }

            printf("Video Codec: resolution %d x %d\n", local_codec_params->width, local_codec_params->height);
        } else if (local_codec_params->codec_type == AVMEDIA_TYPE_AUDIO) {
            printf("Audio Codec: %d channels, sample rate %d\n", local_codec_params->channels, local_codec_params->sample_rate);
        }

        // print its name, id and bitrate
        printf("\tCodec %s ID %d bit_rate %lld\n", local_codec->name, local_codec->id, local_codec_params->bit_rate);
    }

    if (v_stream_index == -1) {
        printf("File %s does not contain a video stream!", argv[1]);
        avformat_close_input(&format_context);
        return -1;
    }

    // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
    AVCodecContext *v_codec_context = avcodec_alloc_context3(v_codec);
    if (!v_codec_context) {
        printf("failed to allocated memory for AVCodecContext\n");
        avformat_close_input(&format_context);
        return -1;
    }

    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    err = avcodec_parameters_to_context(v_codec_context, v_codec_params);
    if (err < 0) {
        printf("failed to copy codec params to codec context, err %d\n", err);
        avcodec_free_context(&v_codec_context);
        avformat_close_input(&format_context);
        return -1;
    }


    // Initialize the AVCodecContext to use the given AVCodec.
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
    err = avcodec_open2(v_codec_context, v_codec, NULL);
    if (err < 0) {
        printf("failed to open codec through avcodec_open2, err %d\n", err);
        avcodec_free_context(&v_codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    AVPacket *v_packet = av_packet_alloc();
    AVFrame *v_frame = av_frame_alloc();
    if (!v_packet || !v_frame) {
        printf("failed to allocated memory for AVPacket or AVFrame\n");
        avcodec_free_context(&v_codec_context);
        avformat_close_input(&format_context);
        return -1;
    }

    int response = 0;
    int how_many_packets_to_process = 8;

    // fill the Packet with data from the Stream
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
    while (av_read_frame(format_context, v_packet) >= 0)
    {
        // if it's the video stream
        if (v_packet->stream_index == v_stream_index) {
            printf("AVPacket->pts %" PRId64 "\n", v_packet->pts);
            response = decode_packet(v_packet, v_codec_context, v_frame);
            if (response < 0) {
                break;
            }
            // stop it, otherwise we'll be saving hundreds of frames
            if (--how_many_packets_to_process <= 0) {
                break;
            }
        }
        // https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
        av_packet_unref(v_packet);
    }

    printf("releasing all the resources\n");
    av_packet_free(&v_packet);
    av_frame_free(&v_frame);
    avcodec_free_context(&v_codec_context);
    avformat_close_input(&format_context);

    return 0;
}


static int decode_packet(AVPacket *v_packet, AVCodecContext *v_codec_context, AVFrame *v_frame)
{
  // Supply raw packet data as input to a decoder
  // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
  int response = avcodec_send_packet(v_codec_context, v_packet);
  if (response < 0) {
    printf("Error while sending a packet to the decoder: %s\n", av_err2str(response));
    return response;
  }

  while (response >= 0)
  {
    // Return decoded output data (into a frame) from a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
    response = avcodec_receive_frame(v_codec_context, v_frame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      printf("Error while receiving a frame from the decoder: %s\n", av_err2str(response));
      return response;
    }

    if (response >= 0) {
      printf(
          "Frame %d (type=%c, size=%d bytes, format=%d) pts %lld key_frame %d [DTS %d]\n",
          v_codec_context->frame_number,
          av_get_picture_type_char(v_frame->pict_type),
          v_frame->pkt_size,
          v_frame->format,
          v_frame->pts,
          v_frame->key_frame,
          v_frame->coded_picture_number
      );

      char frame_filename[1024];
      snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", v_codec_context->frame_number);
      // Check if the frame is a planar YUV 4:2:0, 12bpp
      // That is the format of the provided .mp4 file
      // RGB formats will definitely not give a gray image
      // Other YUV image may do so, but untested, so give a warning
      if (v_frame->format != AV_PIX_FMT_YUV420P)
      {
        printf("Warning: the generated file may not be a grayscale image, but could e.g. be just the R component if the video format is RGB\n");
      }
      // save a grayscale frame into a .pgm file
      save_gray_frame(v_frame->data[0], v_frame->linesize[0], v_frame->width, v_frame->height, frame_filename);
    }
  }
  return 0;
}

static void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;
    f = fopen(filename,"w");
    // writing the minimal required header for a pgm file format
    // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    // writing line by line
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}
