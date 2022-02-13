#include "decoding.h"
#include <memory>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        av_log(NULL, AV_LOG_ERROR, "Usage: %s <input file> <output file>\n", argv[0]);
        return -1;
    }
    const char* input_url = argv[1];
    const char* output_url = argv[2];
    av_log(NULL, AV_LOG_INFO, "transcoding task: %s -> %s\n", input_url, output_url);

    auto dec = std::make_unique<Decoding>(input_url);
    auto ret = dec->Open();
    if (ret != AVERROR_OK) {
        return ret;
    }
    dec->DumpInputFormat();
    dec->Run();
    dec->Close();

    return 0;
}