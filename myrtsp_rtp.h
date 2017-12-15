#ifndef MY_RTSP_RTP_H
#define MY_RTSP_RTP_H

#include <sys/types.h>
#include "myrtsp.h"

#define MAX_PREVIEW_SIZE 4096
#define DEFAULT_SEND_FLAGS 0

struct rtp_rtsp_session_t {
  int rtp_socket;
  int rtcp_socket;
  char *control_url;
  int count;
};

struct rtp_rtsp_session_t *rtp_setup_and_play (rtsp_t* rtsp_session,const char *user,const char *pass);
off_t rtp_read (struct rtp_rtsp_session_t* st, char *buf, off_t length);
void rtp_session_free (struct rtp_rtsp_session_t *st);
void rtcp_send_rr (rtsp_t *s, struct rtp_rtsp_session_t *st);

#endif /* MY_RTSP_RTP_H */
