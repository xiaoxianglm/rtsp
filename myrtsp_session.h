#ifndef MY_RTSP_SESSION_H
#define MY_RTSP_SESSION_H

#include <stdint.h>
#include "myrtsp.h"
#include "myrtsp_rtp.h"
#include "myrtsp_session.h"
#include "real.h"

#define REAL_HEADER_SIZE 4096

typedef struct rtsp_session_s rtsp_session_t;
struct rtsp_session_s {
    rtsp_t       *s;
    struct real_rtsp_session_t* real_session;
    struct rtp_rtsp_session_t* rtp_session;
};


rtsp_session_t *rtsp_session_start(int fd, char **mrl, char *path, char *host,
  int port, int *redir, uint32_t bandwidth, char *user, char *pass);

int rtsp_session_read(rtsp_session_t *session, char *data, int len);

void rtsp_session_end(rtsp_session_t *session);

#endif /* MY_RTSP_SESSION_H */
