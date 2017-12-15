#ifndef MY_SDPPLIN_H
#define MY_SDPPLIN_H

#include "rmff.h"
#include "myrtsp.h"

typedef struct {

  char *id;
  char *bandwidth;

  int stream_id;
  char *range;
  char *length;
  char *rtpmap;
  char *mimetype;
  int min_switch_overlap;
  int start_time;
  int end_one_rule_end_all;
  int avg_bit_rate;
  int max_bit_rate;
  int avg_packet_size;
  int max_packet_size;
  int end_time;
  int seek_greater_on_switch;
  int preroll;

  int duration;
  char *stream_name;
  int stream_name_size;
  char *mime_type;
  int mime_type_size;
  char *mlti_data;
  int mlti_data_size;
  int  rmff_flags_length;
  char *rmff_flags;
  int  asm_rule_book_length;
  char *asm_rule_book;

} sdpplin_stream_t;

typedef struct {

  int sdp_version, sdpplin_version;
  char *owner;
  char *session_name;
  char *session_info;
  char *uri;
  char *email;
  char *phone;
  char *connection;
  char *bandwidth;

  int flags;
  int is_real_data_type;
  int stream_count;
  char *title;
  char *author;
  char *copyright;
  char *keywords;
  int  asm_rule_book_length;
  char *asm_rule_book;
  char *abstract;
  char *range;
  int avg_bit_rate;
  int max_bit_rate;
  int avg_packet_size;
  int max_packet_size;
  int preroll;
  int duration;

  sdpplin_stream_t **stream;

} sdpplin_t;

sdpplin_t *sdpplin_parse(char *data);

void sdpplin_free(sdpplin_t *description);

#endif /* MY_SDPPLIN_H */
