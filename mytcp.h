#ifndef MY_TCP_H
#define MY_TCP_H
#include <stdio.h>
int connect2Server (char *host, int port);

#define TCP_ERROR_TIMEOUT -3     /* connection timeout */
#define TCP_ERROR_FATAL   -2     /* unable to resolve name */
#define TCP_ERROR_PORT    -1     /* unable to connect to a particular port */
#endif
