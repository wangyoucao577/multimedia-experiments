
#pragma once

#include "libav_headers.h"
#include <cstddef>
#include <string>

struct DecodingContext {
    AVCodecContext* codec_ctx;
    AVFrame* frame;

    int send_count;
    int recv_count;
};


class Decoding {
public:
    Decoding() = delete;
    Decoding(const Decoding&) = delete;
    Decoding(Decoding&&) = delete;
    Decoding(const std::string& input_file) : input_file_(input_file) {}
    ~Decoding();

public:
    int Open();
    void Close();
    int Run();

    void DumpInputFormat() const;

private:
    void release();

    int run();

private: 
    AVFormatContext* ifmt_ctx_ {nullptr};

    int nb_streams_ {0};
    DecodingContext* dec_ctx_ = {nullptr};  // ctx per stream, length depends on `nb_streams_`

    AVPacket* pkt_ {nullptr};

private:
    bool opened {false};

private:
    const std::string input_file_;
};