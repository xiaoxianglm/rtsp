#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <ctype.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "mytcp.h"


// Connect to a server using a TCP connection, with specified address family
// return -2 for fatal error, like unable to resolve name, connection timeout...
// return -1 is unable to connect to a particular port

size_t mytrlcpy (char *dst, const char *src, size_t size)
{
    size_t len = 0;
    while (++len < size && *src)
        *dst++ = *src++;
    if (len <= size)
        *dst = 0;
    return len + strlen (src) - 1;
}


int connect2Server (char *host, int port)
{
    int err_res = TCP_ERROR_FATAL;
    int socket_server_fd = -1;
    int err;
    socklen_t err_len;
    int ret, count = 0;
    fd_set set;
    struct timeval tv;
    union
    {
        struct sockaddr_in four;
    } server_address;
    size_t server_address_size;
    void *our_s_addr;		// Pointer to sin_addr or sin6_addr
    struct hostent *hp = NULL;
    char buf[255];


    struct timeval to;


    socket_server_fd = socket (AF_INET, SOCK_STREAM, 0);

    if (socket_server_fd == -1)
    {
        printf ("Failed to create socket\n");
        goto err_out;
    }

    to.tv_sec = 10;
    to.tv_usec = 0;
    setsockopt (socket_server_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof (to));
    setsockopt (socket_server_fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof (to));

    our_s_addr = &server_address.four.sin_addr;

    memset (&server_address, 0, sizeof (server_address));


    if (inet_aton (host, our_s_addr) != 1)
    {
        printf ("Resolving %s ...\n", host);
        hp = gethostbyname (host);
        if (hp == NULL)
        {
            printf ("Couldn't resolve name for %s\n", host);
            goto err_out;
        }

        if (AF_INET != hp->h_addrtype)
            goto err_out;

        memcpy (our_s_addr, hp->h_addr_list[0], hp->h_length);
    }

    server_address.four.sin_family = AF_INET;
    server_address.four.sin_port = htons (port);
    server_address_size = sizeof (server_address.four);


    mytrlcpy (buf, inet_ntoa (*((struct in_addr *) our_s_addr)), 255);
    printf ("Connecting to server %s[%s]: %d...\n", host, buf, port);

    // Turn the socket as non blocking so we can timeout on the connection
    fcntl (socket_server_fd, F_SETFL,fcntl (socket_server_fd, F_GETFL) | O_NONBLOCK);

    if (connect
            (socket_server_fd, (struct sockaddr *) &server_address,
             server_address_size) == -1)
    {
        if (errno != EINPROGRESS)
        {

            printf ("Failed to connect to server\n");
            err_res = TCP_ERROR_PORT;
            goto err_out;
        }
    }
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    FD_ZERO (&set);
    FD_SET (socket_server_fd, &set);
    // When the connection will be made, we will have a writeable fd
    while ((ret = select (socket_server_fd + 1, NULL, &set, NULL, &tv)) == 0)
    {
        if (count > 30)
        {
            if (count > 30)
                printf ("connection timeout\n");
            else
                printf ("Connection interrupted by user\n");
            err_res = TCP_ERROR_TIMEOUT;
            goto err_out;
        }
        count++;
        FD_ZERO (&set);
        FD_SET (socket_server_fd, &set);
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
    }
    if (ret < 0)
        printf ("Select failed.\n");
    // Turn back the socket as blocking
    fcntl( socket_server_fd, F_SETFL, fcntl(socket_server_fd, F_GETFL) & ~O_NONBLOCK );


    // Check if there were any errors
    err_len = sizeof (int);
    ret = getsockopt (socket_server_fd, SOL_SOCKET, SO_ERROR, &err, &err_len);
    if (ret < 0)
    {
        printf ("getsockopt failed: %s\n", strerror (errno));
        goto err_out;
    }
    if (err > 0)
    {
        printf ("connect error: %s\n", strerror (err));
        err_res = TCP_ERROR_PORT;
        goto err_out;
    }

    return socket_server_fd;

err_out:
    if (socket_server_fd >= 0)
        close (socket_server_fd);
    return err_res;
}
