#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include "myxbuffer.h"


typedef struct {
  uint32_t size;
  uint32_t chunk_size;
} xbuffer_header_t;

#define XBUFFER_HEADER_SIZE sizeof (xbuffer_header_t)



void *xbuffer_init(int chunk_size) {
  uint8_t *data=calloc(1,chunk_size+XBUFFER_HEADER_SIZE);

  xbuffer_header_t *header=(xbuffer_header_t*)data;

  header->size=chunk_size;
  header->chunk_size=chunk_size;

  return data+XBUFFER_HEADER_SIZE;
}



void *xbuffer_free(void *buf) {
  if (!buf) {
    return NULL;
  }

  free(((uint8_t*)buf)-XBUFFER_HEADER_SIZE);

  return NULL;
}



void *xbuffer_copyin(void *buf, int index, const void *data, int len) {
    if (!buf || !data || index < 0 || len < 0 || len > INT_MAX - index) {
    return NULL;
  }

  buf = xbuffer_ensure_size(buf, index+len);
  memcpy(((uint8_t*)buf)+index, data, len);

  return buf;
}



void *xbuffer_ensure_size(void *buf, int size) {
  xbuffer_header_t *xbuf;
  int new_size;

  if (!buf) {
    return 0;
  }

  xbuf = ((xbuffer_header_t*)(((uint8_t*)buf)-XBUFFER_HEADER_SIZE));

  if (size < 0 || size > INT_MAX - XBUFFER_HEADER_SIZE - xbuf->chunk_size)
    return NULL;

  if (xbuf->size < size) {
    new_size = size + xbuf->chunk_size - (size % xbuf->chunk_size);
    xbuf->size = new_size;
    buf = ((uint8_t*)realloc(((uint8_t*)buf)-XBUFFER_HEADER_SIZE,
          new_size+XBUFFER_HEADER_SIZE)) + XBUFFER_HEADER_SIZE;
  }

  return buf;
}



void *xbuffer_strcat(void *buf, char *data) {
  size_t blen, dlen;
  if (!buf || !data) {
    return NULL;
  }
  blen = strlen(buf);
  dlen = strlen(data);
  if (blen >= INT_MAX || dlen > INT_MAX - 1 - blen)
    return NULL;

  buf = xbuffer_ensure_size(buf, blen+dlen+1);

  strcat(buf, data);

  return buf;
}
