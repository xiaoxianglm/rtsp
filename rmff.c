#include "rmff.h"
#include "myxbuffer.h"
#include "common.h"
#define LOG

static void hexdump (const char *buf, int length) {

  int i;

  printf ("rmff: ascii>");
  for (i = 0; i < length; i++) {
    unsigned char c = buf[i];

    if ((c >= 32) && (c <= 128))
      printf ("%c", c);
    else
      printf (".");
  }
  printf ("\n");

  printf ("rmff: hexdump> ");
  for (i = 0; i < length; i++) {
    unsigned char c = buf[i];

    printf ("%02x", c);

    if ((i % 16) == 15)
      printf ("\nrmff:         ");

    if ((i % 2) == 1)
      printf (" ");

  }
  printf ("\n");
}

/*
 * writes header data to a buffer
 */

static int rmff_dump_fileheader(rmff_fileheader_t *fileheader, char *buffer, int bufsize) {

  if (!fileheader) return 0;

  if (bufsize < RMFF_FILEHEADER_SIZE)
    return -1;

  WB32(buffer, fileheader->object_id);
  WB32(buffer+4, fileheader->size);
  WB16(buffer+8, fileheader->object_version);
  WB32(buffer+10, fileheader->file_version);
  WB32(buffer+14, fileheader->num_headers);

  return RMFF_FILEHEADER_SIZE;
}

static int rmff_dump_prop(rmff_prop_t *prop, char *buffer, int bufsize) {

  if (!prop) return 0;

  if (bufsize < RMFF_PROPHEADER_SIZE)
    return -1;

  WB32(buffer, prop->object_id);
  WB32(buffer+4, prop->size);
  WB16(buffer+8, prop->object_version);
  WB32(buffer+10, prop->max_bit_rate);
  WB32(buffer+14, prop->avg_bit_rate);
  WB32(buffer+18, prop->max_packet_size);
  WB32(buffer+22, prop->avg_packet_size);
  WB32(buffer+26, prop->num_packets);
  WB32(buffer+30, prop->duration);
  WB32(buffer+34, prop->preroll);
  WB32(buffer+38, prop->index_offset);
  WB32(buffer+42, prop->data_offset);
  WB16(buffer+46, prop->num_streams);
  WB16(buffer+48, prop->flags);

  return RMFF_PROPHEADER_SIZE;
}

static int rmff_dump_mdpr(rmff_mdpr_t *mdpr, char *buffer, int bufsize) {

  int s1, s2, s3;

  if (!mdpr) return 0;

  if (!(bufsize > RMFF_MDPRHEADER_SIZE + mdpr->stream_name_size + mdpr->mime_type_size &&
        (unsigned)bufsize - RMFF_MDPRHEADER_SIZE - mdpr->stream_name_size - mdpr->mime_type_size > mdpr->type_specific_len))
    return -1;

  WB32(buffer, mdpr->object_id);
  WB32(buffer+4, mdpr->size);
  WB16(buffer+8, mdpr->object_version);
  WB16(buffer+10, mdpr->stream_number);
  WB32(buffer+12, mdpr->max_bit_rate);
  WB32(buffer+16, mdpr->avg_bit_rate);
  WB32(buffer+20, mdpr->max_packet_size);
  WB32(buffer+24, mdpr->avg_packet_size);
  WB32(buffer+28, mdpr->start_time);
  WB32(buffer+32, mdpr->preroll);
  WB32(buffer+36, mdpr->duration);

  buffer[40] = mdpr->stream_name_size;
  s1=mdpr->stream_name_size;
  memcpy(&buffer[41], mdpr->stream_name, s1);

  buffer[41+s1] = mdpr->mime_type_size;
  s2=mdpr->mime_type_size;
  memcpy(&buffer[42+s1], mdpr->mime_type, s2);

  WB32(buffer+42+s1+s2, mdpr->type_specific_len);
  s3=mdpr->type_specific_len;
  memcpy(&buffer[46+s1+s2], mdpr->type_specific_data, s3);

  return RMFF_MDPRHEADER_SIZE + s1 + s2 + s3;
}

static int rmff_dump_cont(rmff_cont_t *cont, char *buffer, int bufsize) {

  int p;

  if (!cont) return 0;

  if (bufsize < RMFF_CONTHEADER_SIZE + cont->title_len + cont->author_len +
      cont->copyright_len + cont->comment_len)
    return -1;

  WB32(buffer, cont->object_id);
  WB32(buffer+4, cont->size);
  WB16(buffer+8, cont->object_version);

  WB16(buffer+10, cont->title_len);
  memcpy(&buffer[12], cont->title, cont->title_len);
  p=12+cont->title_len;

  WB16(buffer+p, cont->author_len);
  memcpy(&buffer[p+2], cont->author, cont->author_len);
  p+=2+cont->author_len;

  WB16(buffer+p, cont->copyright_len);
  memcpy(&buffer[p+2], cont->copyright, cont->copyright_len);
  p+=2+cont->copyright_len;

  WB16(buffer+p, cont->comment_len);
  memcpy(&buffer[p+2], cont->comment, cont->comment_len);

  return RMFF_CONTHEADER_SIZE + cont->title_len + cont->author_len +
         cont->copyright_len + cont->comment_len;
}

static int rmff_dump_dataheader(rmff_data_t *data, char *buffer, int bufsize) {

  if (!data) return 0;

  if (bufsize < RMFF_DATAHEADER_SIZE)
    return -1;

  WB32(buffer, data->object_id);
  WB32(buffer+4, data->size);
  WB16(buffer+8, data->object_version);
  WB32(buffer+10, data->num_packets);
  WB32(buffer+14, data->next_data_header);

  return RMFF_DATAHEADER_SIZE;
}

int rmff_dump_header(rmff_header_t *h, char *buffer, int max) {

  int written=0, size;
  rmff_mdpr_t **stream=h->streams;

  if ((size=rmff_dump_fileheader(h->fileheader, &buffer[written], max)) < 0)
    goto buftoosmall;
  written+=size;
  max -= size;
  if ((size=rmff_dump_prop(h->prop, &buffer[written], max)) < 0)
    goto buftoosmall;
  written+=size;
  max -= size;
  if ((size=rmff_dump_cont(h->cont, &buffer[written], max)) < 0)
    goto buftoosmall;
  written+=size;
  max -= size;
  if (stream)
  {
    while(*stream)
    {
      if ((size=rmff_dump_mdpr(*stream, &buffer[written], max)) < 0)
        goto buftoosmall;
      written+=size;
      max -= size;
      stream++;
    }
  }

  if ((size=rmff_dump_dataheader(h->data, &buffer[written], max)) < 0)
    goto buftoosmall;
  written+=size;

  return written;

buftoosmall:
  printf("rmff_dumpheader: buffer too small, aborting. Please report\n");
  return -1;
}

void rmff_dump_pheader(rmff_pheader_t *h, char *data) {

  data[0]=(h->object_version>>8) & 0xff;
  data[1]=h->object_version & 0xff;
  data[2]=(h->length>>8) & 0xff;
  data[3]=h->length & 0xff;
  data[4]=(h->stream_number>>8) & 0xff;
  data[5]=h->stream_number & 0xff;
  data[6]=(h->timestamp>>24) & 0xff;
  data[7]=(h->timestamp>>16) & 0xff;
  data[8]=(h->timestamp>>8) & 0xff;
  data[9]=h->timestamp & 0xff;
  data[10]=h->reserved;
  data[11]=h->flags;
}

static rmff_fileheader_t *rmff_scan_fileheader(const char *data) {

  rmff_fileheader_t *fileheader=malloc(sizeof(rmff_fileheader_t));

  fileheader->object_id=RB32(data);
  fileheader->size=RB32(&data[4]);
  fileheader->object_version=RB16(&data[8]);
  if (fileheader->object_version != 0)
  {
    printf("warning: unknown object version in .RMF: 0x%04x\n",
      fileheader->object_version);
  }
  fileheader->file_version=RB32(&data[10]);
  fileheader->num_headers=RB32(&data[14]);

  return fileheader;
}

static rmff_prop_t *rmff_scan_prop(const char *data) {

  rmff_prop_t *prop=malloc(sizeof(rmff_prop_t));

  prop->object_id=RB32(data);
  prop->size=RB32(&data[4]);
  prop->object_version=RB16(&data[8]);
  if (prop->object_version != 0)
  {
    printf("warning: unknown object version in PROP: 0x%04x\n",
      prop->object_version);
  }
  prop->max_bit_rate=RB32(&data[10]);
  prop->avg_bit_rate=RB32(&data[14]);
  prop->max_packet_size=RB32(&data[18]);
  prop->avg_packet_size=RB32(&data[22]);
  prop->num_packets=RB32(&data[26]);
  prop->duration=RB32(&data[30]);
  prop->preroll=RB32(&data[34]);
  prop->index_offset=RB32(&data[38]);
  prop->data_offset=RB32(&data[42]);
  prop->num_streams=RB16(&data[46]);
  prop->flags=RB16(&data[48]);

  return prop;
}

static rmff_mdpr_t *rmff_scan_mdpr(const char *data) {

  rmff_mdpr_t *mdpr=malloc(sizeof(rmff_mdpr_t));

  mdpr->object_id=RB32(data);
  mdpr->size=RB32(&data[4]);
  mdpr->object_version=RB16(&data[8]);
  if (mdpr->object_version != 0)
  {
    printf("warning: unknown object version in MDPR: 0x%04x\n",
      mdpr->object_version);
  }
  mdpr->stream_number=RB16(&data[10]);
  mdpr->max_bit_rate=RB32(&data[12]);
  mdpr->avg_bit_rate=RB32(&data[16]);
  mdpr->max_packet_size=RB32(&data[20]);
  mdpr->avg_packet_size=RB32(&data[24]);
  mdpr->start_time=RB32(&data[28]);
  mdpr->preroll=RB32(&data[32]);
  mdpr->duration=RB32(&data[36]);

  mdpr->stream_name_size=data[40];
  mdpr->stream_name=malloc(mdpr->stream_name_size+1);
  memcpy(mdpr->stream_name, &data[41], mdpr->stream_name_size);
  mdpr->stream_name[mdpr->stream_name_size]=0;

  mdpr->mime_type_size=data[41+mdpr->stream_name_size];
  mdpr->mime_type=malloc(mdpr->mime_type_size+1);
  memcpy(mdpr->mime_type, &data[42+mdpr->stream_name_size], mdpr->mime_type_size);
  mdpr->mime_type[mdpr->mime_type_size]=0;

  mdpr->type_specific_len=RB32(&data[42+mdpr->stream_name_size+mdpr->mime_type_size]);
  mdpr->type_specific_data=malloc(mdpr->type_specific_len);
  memcpy(mdpr->type_specific_data,
      &data[46+mdpr->stream_name_size+mdpr->mime_type_size], mdpr->type_specific_len);

  return mdpr;
}

static rmff_cont_t *rmff_scan_cont(const char *data) {

  rmff_cont_t *cont=malloc(sizeof(rmff_cont_t));
  int pos;

  cont->object_id=RB32(data);
  cont->size=RB32(&data[4]);
  cont->object_version=RB16(&data[8]);
  if (cont->object_version != 0)
  {
    printf("warning: unknown object version in CONT: 0x%04x\n",
      cont->object_version);
  }
  cont->title_len=RB16(&data[10]);
  cont->title=malloc(cont->title_len+1);
  memcpy(cont->title, &data[12], cont->title_len);
  cont->title[cont->title_len]=0;
  pos=cont->title_len+12;
  cont->author_len=RB16(&data[pos]);
  cont->author=malloc(cont->author_len+1);
  memcpy(cont->author, &data[pos+2], cont->author_len);
  cont->author[cont->author_len]=0;
  pos=pos+2+cont->author_len;
  cont->copyright_len=RB16(&data[pos]);
  cont->copyright=malloc(cont->copyright_len+1);
  memcpy(cont->copyright, &data[pos+2], cont->copyright_len);
  cont->copyright[cont->copyright_len]=0;
  pos=pos+2+cont->copyright_len;
  cont->comment_len=RB16(&data[pos]);
  cont->comment=malloc(cont->comment_len+1);
  memcpy(cont->comment, &data[pos+2], cont->comment_len);
  cont->comment[cont->comment_len]=0;

  return cont;
}

static rmff_data_t *rmff_scan_dataheader(const char *data) {

  rmff_data_t *dh=malloc(sizeof(rmff_data_t));

  dh->object_id=RB32(data);
  dh->size=RB32(&data[4]);
  dh->object_version=RB16(&data[8]);
  if (dh->object_version != 0)
  {
    printf("warning: unknown object version in DATA: 0x%04x\n",
      dh->object_version);
  }
  dh->num_packets=RB32(&data[10]);
  dh->next_data_header=RB32(&data[14]);

  return dh;
}

rmff_header_t *rmff_scan_header(const char *data) {

	rmff_header_t *header=malloc(sizeof(rmff_header_t));
	rmff_mdpr_t   *mdpr=NULL;
	int           chunk_size;
	uint32_t      chunk_type;
  const char    *ptr=data;
  int           i;

  header->fileheader=NULL;
	header->prop=NULL;
	header->cont=NULL;
	header->data=NULL;

  chunk_type = RB32(ptr);
  if (chunk_type != RMF_TAG)
  {
    printf("rmff: not an real media file header (.RMF tag not found).\n");
    free(header);
    return NULL;
  }
  header->fileheader=rmff_scan_fileheader(ptr);
  ptr += header->fileheader->size;

	header->streams=malloc(sizeof(rmff_mdpr_t*)*(header->fileheader->num_headers));
  for (i=0; i<header->fileheader->num_headers; i++) {
    header->streams[i]=NULL;
  }

  for (i=1; i<header->fileheader->num_headers; i++) {
    chunk_type = RB32(ptr);

    if (ptr[0] == 0)
    {
      printf("rmff: warning: only %d of %d header found.\n", i, header->fileheader->num_headers);
      break;
    }

    chunk_size=1;
    switch (chunk_type) {
    case PROP_TAG:
      header->prop=rmff_scan_prop(ptr);
      chunk_size=header->prop->size;
      break;
    case MDPR_TAG:
      mdpr=rmff_scan_mdpr(ptr);
      chunk_size=mdpr->size;
      header->streams[mdpr->stream_number]=mdpr;
      break;
    case CONT_TAG:
      header->cont=rmff_scan_cont(ptr);
      chunk_size=header->cont->size;
      break;
    case DATA_TAG:
      header->data=rmff_scan_dataheader(ptr);
      chunk_size=34;     /* hard coded header size */
      break;
    default:
      printf("unknown chunk\n");
      hexdump(ptr,10);
      chunk_size=1;
      break;
    }
    ptr+=chunk_size;
  }

	return header;
}

rmff_header_t *rmff_scan_header_stream(int fd) {

  rmff_header_t *header;
  char *buf=xbuffer_init(1024);
  int index=0;
  uint32_t chunk_type;
  uint32_t chunk_size;

  do {
    buf = xbuffer_ensure_size(buf, index+8);
    recv(fd, buf+index, 8, 0);
    chunk_type=RB32(buf+index); index+=4;
    chunk_size=RB32(buf+index); index+=4;

    switch (chunk_type) {
      case DATA_TAG:
        chunk_size=18;
      case MDPR_TAG:
      case CONT_TAG:
      case RMF_TAG:
      case PROP_TAG:
        buf = xbuffer_ensure_size(buf, index+chunk_size-8);
        recv(fd, buf+index, (chunk_size-8), 0);
	index+=(chunk_size-8);
        break;
      default:
        printf("rmff_scan_header_stream: unknown chunk");
        hexdump(buf+index-8, 8);
        chunk_type=DATA_TAG;
    }
  } while (chunk_type != DATA_TAG);

  header = rmff_scan_header(buf);

  xbuffer_free(buf);

  return header;
}

void rmff_scan_pheader(rmff_pheader_t *h, char *data) {

  h->object_version=RB16(data);
  h->length=RB16(data+2);
  h->stream_number=RB16(data+4);
  h->timestamp=RB32(data+6);
  h->reserved=(uint8_t)data[10];
  h->flags=(uint8_t)data[11];
}

rmff_fileheader_t *rmff_new_fileheader(uint32_t num_headers) {

  rmff_fileheader_t *fileheader=malloc(sizeof(rmff_fileheader_t));
  if( !fileheader )
    return NULL;

  fileheader->object_id=RMF_TAG;
  fileheader->size=18;
  fileheader->object_version=0;
  fileheader->file_version=0;
  fileheader->num_headers=num_headers;

  return fileheader;
}

rmff_prop_t *rmff_new_prop (
    uint32_t max_bit_rate,
    uint32_t avg_bit_rate,
    uint32_t max_packet_size,
    uint32_t avg_packet_size,
    uint32_t num_packets,
    uint32_t duration,
    uint32_t preroll,
    uint32_t index_offset,
    uint32_t data_offset,
    uint16_t num_streams,
    uint16_t flags ) {

  rmff_prop_t *prop=malloc(sizeof(rmff_prop_t));

  prop->object_id=PROP_TAG;
  prop->size=50;
  prop->object_version=0;

  prop->max_bit_rate=max_bit_rate;
  prop->avg_bit_rate=avg_bit_rate;
  prop->max_packet_size=max_packet_size;
  prop->avg_packet_size=avg_packet_size;
  prop->num_packets=num_packets;
  prop->duration=duration;
  prop->preroll=preroll;
  prop->index_offset=index_offset;
  prop->data_offset=data_offset;
  prop->num_streams=num_streams;
  prop->flags=flags;

  return prop;
}

rmff_mdpr_t *rmff_new_mdpr(
      uint16_t   stream_number,
      uint32_t   max_bit_rate,
      uint32_t   avg_bit_rate,
      uint32_t   max_packet_size,
      uint32_t   avg_packet_size,
      uint32_t   start_time,
      uint32_t   preroll,
      uint32_t   duration,
      const char *stream_name,
      const char *mime_type,
      uint32_t   type_specific_len,
      const char *type_specific_data ) {

  rmff_mdpr_t *mdpr=calloc(sizeof(rmff_mdpr_t),1);

  mdpr->object_id=MDPR_TAG;
  mdpr->object_version=0;

  mdpr->stream_number=stream_number;
  mdpr->max_bit_rate=max_bit_rate;
  mdpr->avg_bit_rate=avg_bit_rate;
  mdpr->max_packet_size=max_packet_size;
  mdpr->avg_packet_size=avg_packet_size;
  mdpr->start_time=start_time;
  mdpr->preroll=preroll;
  mdpr->duration=duration;
  mdpr->stream_name_size=0;
  if (stream_name) {
    mdpr->stream_name=strdup(stream_name);
    mdpr->stream_name_size=strlen(stream_name);
  }
  mdpr->mime_type_size=0;
  if (mime_type) {
    mdpr->mime_type=strdup(mime_type);
    mdpr->mime_type_size=strlen(mime_type);
  }
  mdpr->type_specific_len=type_specific_len;

  mdpr->type_specific_data = malloc(type_specific_len);
  if( !mdpr->type_specific_data ) {
    free( mdpr->stream_name );
    free( mdpr );
    return NULL;
  }
  memcpy(mdpr->type_specific_data,type_specific_data,type_specific_len);
  mdpr->mlti_data=NULL;

  mdpr->size=mdpr->stream_name_size+mdpr->mime_type_size+mdpr->type_specific_len+46;

  return mdpr;
}

rmff_cont_t *rmff_new_cont(const char *title, const char *author, const char *copyright, const char *comment) {

  rmff_cont_t *cont=malloc(sizeof(rmff_cont_t));
  if( !cont )
    return NULL;

  cont->object_id=CONT_TAG;
  cont->object_version=0;

  cont->title=NULL;
  cont->author=NULL;
  cont->copyright=NULL;
  cont->comment=NULL;

  cont->title_len=0;
  cont->author_len=0;
  cont->copyright_len=0;
  cont->comment_len=0;

  if (title) {
    cont->title_len=strlen(title);
    cont->title=strdup(title);
  }
  if (author) {
    cont->author_len=strlen(author);
    cont->author=strdup(author);
  }
  if (copyright) {
    cont->copyright_len=strlen(copyright);
    cont->copyright=strdup(copyright);
  }
  if (comment) {
    cont->comment_len=strlen(comment);
    cont->comment=strdup(comment);
  }
  cont->size=cont->title_len+cont->author_len+cont->copyright_len+cont->comment_len+18;

  return cont;
}

rmff_data_t *rmff_new_dataheader(uint32_t num_packets, uint32_t next_data_header) {

  rmff_data_t *data=malloc(sizeof(rmff_data_t));
  if( !data )
    return NULL;

  data->object_id=DATA_TAG;
  data->size=18;
  data->object_version=0;
  data->num_packets=num_packets;
  data->next_data_header=next_data_header;

  return data;
}

void rmff_print_header(rmff_header_t *h) {

  rmff_mdpr_t **stream;

  if(!h) {
    printf("rmff_print_header: NULL given\n");
    return;
  }
  if(h->fileheader)
  {
    printf("\nFILE:\n");
    printf("file version      : %d\n", h->fileheader->file_version);
    printf("number of headers : %d\n", h->fileheader->num_headers);
  }
  if(h->cont)
  {
    printf("\nCONTENT:\n");
    printf("title     : %s\n", h->cont->title);
    printf("author    : %s\n", h->cont->author);
    printf("copyright : %s\n", h->cont->copyright);
    printf("comment   : %s\n", h->cont->comment);
  }
  if(h->prop)
  {
    printf("\nSTREAM PROPERTIES:\n");
    printf("bit rate (max/avg)    : %i/%i\n", h->prop->max_bit_rate, h->prop->avg_bit_rate);
    printf("packet size (max/avg) : %i/%i bytes\n", h->prop->max_packet_size, h->prop->avg_packet_size);
    printf("packets       : %i\n", h->prop->num_packets);
    printf("duration      : %i ms\n", h->prop->duration);
    printf("pre-buffer    : %i ms\n", h->prop->preroll);
    printf("index offset  : %i bytes\n", h->prop->index_offset);
    printf("data offset   : %i bytes\n", h->prop->data_offset);
    printf("media streams : %i\n", h->prop->num_streams);
    printf("flags         : ");
    if (h->prop->flags & PN_SAVE_ENABLED) printf("save_enabled ");
    if (h->prop->flags & PN_PERFECT_PLAY_ENABLED) printf("perfect_play_enabled ");
    if (h->prop->flags & PN_LIVE_BROADCAST) printf("live_broadcast ");
    printf("\n");
  }
  stream=h->streams;
  if(stream)
  {
    while (*stream)
    {
      printf("\nSTREAM %i:\n", (*stream)->stream_number);
      printf("stream name [mime type] : %s [%s]\n", (*stream)->stream_name, (*stream)->mime_type);
      printf("bit rate (max/avg)      : %i/%i\n", (*stream)->max_bit_rate, (*stream)->avg_bit_rate);
      printf("packet size (max/avg)   : %i/%i bytes\n", (*stream)->max_packet_size, (*stream)->avg_packet_size);
      printf("start time : %i\n", (*stream)->start_time);
      printf("pre-buffer : %i ms\n", (*stream)->preroll);
      printf("duration   : %i ms\n", (*stream)->duration);
      printf("type specific data:\n");
      hexdump((*stream)->type_specific_data, (*stream)->type_specific_len);
      stream++;
    }
  }
  if(h->data)
  {
    printf("\nDATA:\n");
    printf("size      : %i\n", h->data->size);
    printf("packets   : %i\n", h->data->num_packets);
    printf("next DATA : 0x%08x\n", h->data->next_data_header);
  }
}

void rmff_fix_header(rmff_header_t *h) {

  int num_headers=0;
  int header_size=0;
  rmff_mdpr_t **streams;
  int num_streams=0;

  if (!h) {
    printf("rmff_fix_header: fatal: no header given.\n");
    return;
  }

  if (!h->streams) {
    printf("rmff_fix_header: warning: no MDPR chunks\n");
  } else
  {
    streams=h->streams;
    while (*streams)
    {
      num_streams++;
      num_headers++;
      header_size+=(*streams)->size;
      streams++;
    }
  }

  if (h->prop) {
    if (h->prop->size != 50)
    {
#ifdef LOG
      printf("rmff_fix_header: correcting prop.size from %i to %i\n", h->prop->size, 50);
#endif
      h->prop->size=50;
    }
    if (h->prop->num_streams != num_streams)
    {
#ifdef LOG
      printf("rmff_fix_header: correcting prop.num_streams from %i to %i\n", h->prop->num_streams, num_streams);
#endif
      h->prop->num_streams=num_streams;
    }
    num_headers++;
    header_size+=50;
  } else
    printf("rmff_fix_header: warning: no PROP chunk.\n");

  if (h->cont) {
    num_headers++;
    header_size+=h->cont->size;
  } else
    printf("rmff_fix_header: warning: no CONT chunk.\n");

  if (!h->data) {
#ifdef LOG
    printf("rmff_fix_header: no DATA chunk, creating one\n");
#endif
    h->data=malloc(sizeof(rmff_data_t));
    h->data->object_id=DATA_TAG;
    h->data->object_version=0;
    h->data->size=34;
    h->data->num_packets=0;
    h->data->next_data_header=0;
  }
  num_headers++;


  if (!h->fileheader) {
#ifdef LOG
    printf("rmff_fix_header: no fileheader, creating one");
#endif
    h->fileheader=malloc(sizeof(rmff_fileheader_t));
    h->fileheader->object_id=RMF_TAG;
    h->fileheader->size=34;
    h->fileheader->object_version=0;
    h->fileheader->file_version=0;
    h->fileheader->num_headers=num_headers+1;
  }
  header_size+=h->fileheader->size;
  num_headers++;

  if(h->fileheader->num_headers != num_headers) {
#ifdef LOG
    printf("rmff_fix_header: setting num_headers from %i to %i\n", h->fileheader->num_headers, num_headers);
#endif
    h->fileheader->num_headers=num_headers;
  }

  if(h->prop) {
    if (h->prop->data_offset != header_size) {
#ifdef LOG
      printf("rmff_fix_header: setting prop.data_offset from %i to %i\n", h->prop->data_offset, header_size);
#endif
      h->prop->data_offset=header_size;
    }
    if (h->prop->num_packets == 0) {
      int p=(int)(h->prop->avg_bit_rate/8.0*(h->prop->duration/1000.0)/h->prop->avg_packet_size);
#ifdef LOG
      printf("rmff_fix_header: assuming prop.num_packets=%i\n", p);
#endif
      h->prop->num_packets=p;
    }
    if (h->data->num_packets == 0) {
#ifdef LOG
      printf("rmff_fix_header: assuming data.num_packets=%i\n", h->prop->num_packets);
#endif
      h->data->num_packets=h->prop->num_packets;
    }

#ifdef LOG
    printf("rmff_fix_header: assuming data.size=%i\n", h->prop->num_packets*h->prop->avg_packet_size);
#endif
    h->data->size=h->prop->num_packets*h->prop->avg_packet_size;
  }
}

int rmff_get_header_size(rmff_header_t *h) {

  if (!h) return 0;
  if (!h->prop) return -1;

  return h->prop->data_offset+18;

}

void rmff_free_header(rmff_header_t *h) {

  if (!h) return;

  free(h->fileheader);
  free(h->prop);
  free(h->data);
  if (h->cont)
  {
    free(h->cont->title);
    free(h->cont->author);
    free(h->cont->copyright);
    free(h->cont->comment);
    free(h->cont);
  }
  if (h->streams)
  {
    rmff_mdpr_t **s=h->streams;

    while(*s) {
      free((*s)->stream_name);
      free((*s)->mime_type);
      free((*s)->type_specific_data);
      free(*s);
      s++;
    }
    free(h->streams);
  }
  free(h);
}
