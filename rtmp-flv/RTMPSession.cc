
#include "RTMPSession.h"

#include <librtmp/log.h>

RTMPSession::RTMPSession(std::string url) : url_(url) {
  RTMP_debuglevel = RTMP_LOGINFO;

  rtmp_ = RTMP_Alloc();
  if (NULL == rtmp_) {
    throw kRTMPSessionErrnoAllocFailed;
  }

  RTMP_Init(rtmp_);

  // be aware that rtmp internal uses address of `url_.c_str()`,
  // so the url_ has to be saved for later `RTMP_Connect`.
  if (!RTMP_SetupURL(rtmp_, (char *)(url_.c_str()))) {
    throw kRTMPSessionErrnoSetupURLFailed;
  }
}

RTMPSession::~RTMPSession() {
  if (rtmp_) {
    RTMP_Free(rtmp_);
    rtmp_ = nullptr;
  }
}

void RTMPSession::Connect() {

  rtmp_->Link.timeout = 10;
  rtmp_->Link.lFlags |= RTMP_LF_LIVE;
  RTMP_SetBufferMS(rtmp_, 3600 * 1000); // 1hour

  if (!RTMP_Connect(rtmp_, NULL)) {
    throw kRTMPSessionErrnoConnectFailed;
  }

  if (!RTMP_ConnectStream(rtmp_, 0)) {
    Close();
    throw kRTMPSessionErrnoConnectFailed;
  }
}

void RTMPSession::Close() { return RTMP_Close(rtmp_); }

int RTMPSession::Read(char *buff, int buff_len) {
  return RTMP_Read(rtmp_, buff, buff_len);
}
