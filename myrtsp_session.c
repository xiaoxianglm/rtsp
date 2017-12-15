#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "myrtsp.h"
#include "myrtsp_rtp.h"
#include "myrtsp_session.h"
#include "rmff.h"
#include "real.h"

#define RTSP_OPTIONS_PUBLIC "Public"
#define RTSP_OPTIONS_SERVER "Server"
#define RTSP_OPTIONS_LOCATION "Location"
#define RTSP_OPTIONS_REAL "RealChallenge1"
#define RTSP_SERVER_TYPE_REAL "Real"
#define RTSP_SERVER_TYPE_HELIX "Helix"
#define RTSP_SERVER_TYPE_UNKNOWN "unknown"


/*
 * closes an rtsp connection
 */

static void rtsp_close(rtsp_t *s) {

    if (s->server_state)
    {
        if (s->server_state == RTSP_PLAYING)
            rtsp_request_teardown (s, NULL);
        close(s->s);
    }

    free(s->path);
    free(s->host);
    free(s->mrl);
    free(s->session);
    free(s->user_agent);
    free(s->server);
    rtsp_free_answers(s);
    rtsp_unschedule_all(s);
    free(s);
}

//rtsp_session_t *rtsp_session_start(char *mrl) {
rtsp_session_t *rtsp_session_start(int fd, char **mrl, char *path, char *host,
        int port, int *redir, uint32_t bandwidth, char *user, char *pass) {

    rtsp_session_t *rtsp_session = NULL;
    char *server;
    char *mrl_line = NULL;

    rtsp_session = malloc (sizeof (rtsp_session_t));
    rtsp_session->s = NULL;
    rtsp_session->real_session = NULL;
    rtsp_session->rtp_session = NULL;

    //connect:
    *redir = 0;

    /* connect to server */
    rtsp_session->s=rtsp_connect(fd,*mrl,path,host,port,NULL);
    if (!rtsp_session->s)
    {
        printf("rtsp_session: failed to connect to server %s\n", path);
        free(rtsp_session);
        return NULL;
    }

    /* looking for server type */
    if (rtsp_search_answers(rtsp_session->s,RTSP_OPTIONS_SERVER))
        server=strdup(rtsp_search_answers(rtsp_session->s,RTSP_OPTIONS_SERVER));
    else {
        if (rtsp_search_answers(rtsp_session->s,RTSP_OPTIONS_REAL))
            server=strdup(RTSP_SERVER_TYPE_REAL);
        else
            server=strdup(RTSP_SERVER_TYPE_UNKNOWN);
    }
    if (strstr(server,RTSP_SERVER_TYPE_REAL) || strstr(server,RTSP_SERVER_TYPE_HELIX))
    {
        /* we are talking to a real server ... */

        rmff_header_t *h=real_setup_and_get_header(rtsp_session->s, bandwidth,user, pass);
        if (!h || !h->streams[0]) {
            rmff_free_header(h);
            /* got an redirect? */
            if (rtsp_search_answers(rtsp_session->s, RTSP_OPTIONS_LOCATION))
            {
                free(mrl_line);
                mrl_line=strdup(rtsp_search_answers(rtsp_session->s, RTSP_OPTIONS_LOCATION));
                printf("rtsp_session: redirected to %s\n",mrl_line);
                rtsp_close(rtsp_session->s);
                free(server);
                free(*mrl);
                free(rtsp_session);
                /* tell the caller to redirect, return url to redirect to in mrl */
                *mrl = mrl_line;
                *redir = 1;
                return NULL;
                //	goto connect; /* *shudder* i made a design mistake somewhere */
            } else
            {
                printf("rtsp_session: session can not be established.\n");
                rtsp_close(rtsp_session->s);
                free (server);
                free(rtsp_session);
                return NULL;
            }
        }

        rtsp_session->real_session = init_real_rtsp_session ();
        if(!strncmp(h->streams[0]->mime_type, "application/vnd.rn-rmadriver", h->streams[0]->mime_type_size) ||
                !strncmp(h->streams[0]->mime_type, "application/smil", h->streams[0]->mime_type_size)) {
            rtsp_session->real_session->header_len = 0;
            rtsp_session->real_session->recv_size = 0;
            rtsp_session->real_session->rdt_rawdata = 1;
            printf("smil-over-realrtsp playlist, switching to raw rdt mode\n");
        } else {
            rtsp_session->real_session->header_len =
                rmff_dump_header (h, (char *) rtsp_session->real_session->header,RTSP_HEADER_SIZE);

            if (rtsp_session->real_session->header_len < 0) {
                printf("rtsp_session: error while dumping RMFF headers, session can not be established.\n");
                free_real_rtsp_session(rtsp_session->real_session);
                rtsp_close(rtsp_session->s);
                free (server);
                free (mrl_line);
                free(rtsp_session);
                return NULL;
            }

            rtsp_session->real_session->recv =
                xbuffer_copyin (rtsp_session->real_session->recv, 0,
                        rtsp_session->real_session->header,
                        rtsp_session->real_session->header_len);

            rtsp_session->real_session->recv_size =
                rtsp_session->real_session->header_len;
        }
        rtsp_session->real_session->recv_read = 0;
        rmff_free_header(h);

    } else /* not a Real server : try RTP instead */
    {
        char *public = NULL;

        /* look for the Public: field in response to RTSP OPTIONS */
        if (!(public = rtsp_search_answers (rtsp_session->s, RTSP_OPTIONS_PUBLIC)))
        {
            rtsp_close (rtsp_session->s);
            free (server);
            free (mrl_line);
            free (rtsp_session);
            return NULL;
        }

        /* check for minimalistic RTSP RFC compliance */
        if (!strstr (public, RTSP_METHOD_DESCRIBE)
                || !strstr (public, RTSP_METHOD_SETUP)
                || !strstr (public, RTSP_METHOD_PLAY)
                || !strstr (public, RTSP_METHOD_TEARDOWN))
        {
            printf("Remote server does not meet minimal RTSP 1.0 compliance.\n");
            rtsp_close (rtsp_session->s);
            free (server);
            free (mrl_line);
            free (rtsp_session);
            return NULL;
        }

        rtsp_session->rtp_session = rtp_setup_and_play (rtsp_session->s,user,pass);

        /* neither a Real or an RTP server */
        if (!rtsp_session->rtp_session)
        {
            printf( "rtsp_session: unsupported RTSP server. ");
            printf( "Server type is '%s'.\n", server);
            rtsp_close (rtsp_session->s);
            free (server);
            free (mrl_line);
            free (rtsp_session);
            return NULL;
        }
    }
    free(server);

    return rtsp_session;
}

int rtsp_session_read (rtsp_session_t *this, char *data, int len) {

    if (this->real_session) {
        int to_copy=len;
        char *dest=data;
        char *source =
            (char *) (this->real_session->recv + this->real_session->recv_read);
        int fill = this->real_session->recv_size - this->real_session->recv_read;

        if(this->real_session->rdteof)
            return -1;
        if (len < 0) return 0;
        if (this->real_session->recv_size < 0) return -1;
        while (to_copy > fill) {

            memcpy(dest, source, fill);
            to_copy -= fill;
            dest += fill;
            this->real_session->recv_read = 0;
            this->real_session->recv_size =
                real_get_rdt_chunk (this->s, (char **)&(this->real_session->recv), this->real_session->rdt_rawdata);
            if (this->real_session->recv_size < 0) {
                this->real_session->rdteof = 1;
                this->real_session->recv_size = 0;
            }
            source = (char *) this->real_session->recv;
            fill = this->real_session->recv_size;

            if (this->real_session->recv_size == 0) {
                printf( "librtsp: %d of %d bytes provided\n", len-to_copy, len);
                return len-to_copy;
            }
        }

        memcpy(dest, source, to_copy);
        this->real_session->recv_read += to_copy;

        printf( "librtsp: %d bytes provided\n", len);

        return len;
    }
    else if (this->rtp_session)
    {
        int l = 0;

        l = read_rtp_from_server (this->rtp_session->rtp_socket, data, len);
        /* send RTSP and RTCP keepalive  */
        rtcp_send_rr (this->s, this->rtp_session);

        if (l == 0)
            rtsp_session_end (this);
        return 0;
        return l;
    }

    return 0;
}

void rtsp_session_end(rtsp_session_t *session) {

    rtsp_close(session->s);
    if (session->real_session)
        free_real_rtsp_session (session->real_session);
    if (session->rtp_session)
        rtp_session_free (session->rtp_session);
    free(session);
}

