
#ifndef _RTMP_SESSION_H_
#define _RTMP_SESSION_H_

#include <librtmp/rtmp.h>
#include <string>

enum RTMPSessionErrorCode {
  kRTMPSessionErrnoOK = 0,

  kRTMPSessionErrnoAllocFailed,
  kRTMPSessionErrnoSetupURLFailed,
  kRTMPSessionErrnoConnectFailed,
  kRTMPSessionErrnoConnectStreamFailed,
};

class RTMPSession {
public:
  explicit RTMPSession(std::string url);
  ~RTMPSession();

  void Connect();
  void Close();

  int Read(char *buff, int buff_len);

private:
  RTMP *rtmp_{nullptr};
  std::string url_;
};

#endif
