

#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

#include "FlvCommon.h"
#include "FlvHeader.h"
#include "FlvTag.h"
#include "RTMPSession.h"

#define DUMP_FLV_FILE

// TODO: fix raw dump
//   currently raw audio can not play due to missed ADTS header
//   raw video can not play due to no sps/pps headers as well as NALU start
//   header
// #define DUMP_RAW_AUDIO_FILE
// #define DUMP_RAW_VIDEO_FILE

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cout << "Usage:" << endl;
    cout << "librtmp_test <rtmp_url>" << endl;
    return 0;
  }

#ifdef DUMP_FLV_FILE
  FILE *fp = fopen("test.flv", "wb");
  if (!fp) {
    cout << "Open file test.flv failed" << endl;
    return -1;
  }
#endif
#ifdef DUMP_RAW_AUDIO_FILE
  FILE *fp_aac = fopen("test.aac", "wb");
  if (!fp_aac) {
    cout << "Open file test.aac failed" << endl;
    return -1;
  }
#endif
#ifdef DUMP_RAW_VIDEO_FILE
  FILE *fp_h264 = fopen("test.h264", "wb");
  if (!fp_h264) {
    cout << "Open file test.h264 failed" << endl;
    return -1;
  }
#endif

  RTMPSession *rtmp_session = nullptr;
  try {
    rtmp_session = new RTMPSession(argv[1]);
    rtmp_session->Connect();
  } catch (RTMPSessionErrorCode e) {
    cout << "Init RTMP session failed, error code " << static_cast<int>(e)
         << ", h_error (" << h_errno << ") " << hstrerror(h_errno) << endl;
    if (rtmp_session) {
      delete rtmp_session;
    }
    return -1;
  }

  unsigned long long thisRecvedBytes = 0;
  int buff_size = 5 * 1000 * 1000; // 5MB
  char *buff = new char[buff_size];
  memset(buff, 0, buff_size);

  int64_t start_time_us = FlvCommonUtils::GetCurrentTimeMillseconds();
  int nRead = 0;
  bool first_read = true;
  int offset = 0;
  bool next_previous_tag_size = true;
  while (nRead = rtmp_session->Read(buff + offset, buff_size - offset)) {
    cout << "this recv bytes: " << nRead << endl;

#ifdef DUMP_FLV_FILE
    int nWrite = fwrite(buff + offset, 1, nRead, fp);
    assert(nWrite == nRead);
#endif

    // parse flv
    int useful_bytes = nRead + offset;
    offset = 0;
    while (offset < useful_bytes) {
      try {
        if (first_read) {
          FlvHeader fh(buff + offset, useful_bytes - offset);
          assert(fh.Verify());
          fh.Dump();
          offset += fh.cost_bytes();
        }
        first_read = false;

        if (next_previous_tag_size) {
          cout << "previous tag size: "
               << FlvTag::FetchPreviousTagSize(buff + offset,
                                               useful_bytes - offset)
               << endl;
          offset += FlvTag::kPreviousTagSizeTypeLength;
        } else {
          FlvTag ft(buff + offset, useful_bytes - offset);
          ft.Dump();

#ifdef DUMP_RAW_AUDIO_FILE
          if (ft.GetTagType() == kFlyTagTypeAudio && ft.GetTagDataLength()) {
            // TODO: write的ES文件是否正确?? 待验证
            int es_write =
                fwrite(ft.GetDataPointer(), 1, ft.GetTagDataLength(), fp_aac);
            assert(es_write == ft.GetTagDataLength());
          }
#endif
#ifdef DUMP_RAW_VIDEO_FILE
          if (ft.GetTagType() == kFlyTagTypeVideo && ft.GetTagDataLength()) {
            // TODO: write的ES文件是否正确?? 待验证
            int es_write =
                fwrite(ft.GetDataPointer(), 1, ft.GetTagDataLength(), fp_h264);
            assert(es_write == ft.GetTagDataLength());
          }
#endif
          offset += ft.cost_bytes(); // for header
        }
        next_previous_tag_size = !next_previous_tag_size;
      } catch (FlvException &e) {
        // cout << "FlvException catched, err:" << e.ErrorCode() << ", msg:" <<
        // e.msg() << endl;
        cout << e << endl;
        cout << "useful bytes: " << useful_bytes << ", offset: " << offset
             << endl;
        break;
      }
    }
    if (offset >= useful_bytes) {
      offset = 0;
    }

    //码率统计
    thisRecvedBytes += nRead;
    int64_t curr_time_us = FlvCommonUtils::GetCurrentTimeMillseconds();
    int64_t delta_us = curr_time_us - start_time_us;
    if (delta_us >= 1000000) {
      cout << "[rtmp recv kbps:" << thisRecvedBytes * 8 * 1000 / delta_us << "]"
           << endl;
      start_time_us = curr_time_us;
      thisRecvedBytes = 0;
    }
  }

#ifdef DUMP_FLV_FILE
  if (fp) {
    fclose(fp);
  }
#endif
#ifdef DUMP_RAW_AUDIO_FILE
  if (fp_aac) {
    fclose(fp_aac);
  }
#endif
#ifdef DUMP_RAW_VIDEO_FILE
  if (fp_h264) {
    fclose(fp_h264);
  }
#endif

  if (buff) {
    delete[] buff;
    buff = nullptr;
  }

  if (rtmp_session) {
    rtmp_session->Close();
    delete rtmp_session;
    rtmp_session = nullptr;
  }

  return 0;
}
