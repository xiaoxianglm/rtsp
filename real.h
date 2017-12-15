
#ifndef MY_REAL_H
#define MY_REAL_H

#include "rmff.h"
#include "myrtsp.h"

#define REAL_HEADER_SIZE 4096

struct real_rtsp_session_t {
  /* receive buffer */
  uint8_t *recv;
  int recv_size;
  int recv_read;

  /* header buffer */
  uint8_t header[REAL_HEADER_SIZE];
  int header_len;
  int header_read;

  int rdteof;

  int rdt_rawdata;
};

int real_get_rdt_chunk(rtsp_t *rtsp_session, char **buffer, int rdt_rawdata);
rmff_header_t *real_setup_and_get_header(rtsp_t *rtsp_session, uint32_t bandwidth,
  char *username, char *password);
struct real_rtsp_session_t *init_real_rtsp_session (void);
void free_real_rtsp_session (struct real_rtsp_session_t* real_session);

#endif /* MY_REAL_H */
