#ifndef COMMON_H
#define COMMON_H

#include <sys/time.h>
#include <time.h>
#include <stdint.h>

typedef enum
{
  FSDPE_OK = 0,
  FSDPE_ILLEGAL_CHARACTER,	 /**< Misplaced '\r', '\n' or '\0' */
  FSDPE_MISSING_VERSION,	 /**< The first line is not like
				    v=... */
  FSDPE_INVALID_VERSION,	 /**< Parse error in version line,
				    perhaps, the version specified in
				    v=... is not valid for FreeSDP */
  FSDPE_MISSING_OWNER,		 /**< No owner line found in its
				    place */
  FSDPE_INVALID_OWNER,		 /**< Parse error in owner line */
  FSDPE_MISSING_NAME,		 /**< No session name found in its
				    place */
  FSDPE_EMPTY_NAME,		 /**< Empty session name line */

  FSDPE_INVALID_CONNECTION,	 /**< Syntax error in connection
				    line */

  FSDPE_INVALID_CONNECTION_ADDRTYPE, /**< Unrecognized address type in
					connection line */
  FSDPE_INVALID_CONNECTION_NETTYPE,  /**< Unrecognized network type in
					connection line */
  FSDPE_INVALID_BANDWIDTH,	     /**< Parse error in bandwidth
					line */
  FSDPE_MISSING_TIME,		 /**< No time period has been given
				    for the session */
  FSDPE_INVALID_TIME,		 /**< Parse error in time line */
  FSDPE_INVALID_REPEAT,		 /**< Parse error in repeat time
				    line */
  FSDPE_INVALID_TIMEZONE,	 /**< Parse error in timezone line */
  FSDPE_INVALID_ENCRYPTION_METHOD, /**< Unknown encryption method */
  FSDPE_INVALID_ATTRIBUTE,	 /**< Syntax error in an attribute
				    line */

  FSDPE_INVALID_ATTRIBUTE_RTPMAP,/**< Parse error in a=rtpmap:... line */
  FSDPE_INVALID_SESSION_TYPE,	 /**< An unknown session type has been
				    specified in a `type:'
				    session-level attribute */

  FSDPE_INVALID_MEDIA,		 /**< Parse error in media line */
  FSDPE_UNKNOWN_MEDIA_TYPE,	 /**< Unknown media type in media
				    line */

  FSDPE_UNKNOWN_MEDIA_TRANSPORT, /**< A media transport has been
				    specified that is unknown */

  FSDPE_OVERFILLED,		 /**< extra unknown lines are at the
				    end of the description */
  FSDPE_INVALID_LINE,		 /**< a line unknown to FreeSDP has been
				    found */
  FSDPE_MISSING_CONNECTION_INFO, /**< No connection information has
				     been provided for the whole
				     session nor one or more media */
  FSDPE_INVALID_INDEX,
  /*  FSDPE_MAXSIZE, description does not fit requested maximun size */
  FSDPE_INTERNAL_ERROR,

  FSDPE_INVALID_PARAMETER,	 /**< Some parameter of the called
				       FreeSDP routine has been given an
				       invalid value. This includes
				       cases such as NULL pointers. */
  FSDPE_BUFFER_OVERFLOW
} fsdp_error_t;

/**
 * @short Type of network
 *
 * Initially, SDP defines "Internet". New network types may be
 * registered with IANA. However, the number of types is expected to
 * be small and rarely extended. In addition, every new network type
 * requires at least one new address type.
 **/
typedef enum
{
  FSDP_NETWORK_TYPE_UNDEFINED,		       /**< Not provided */
  FSDP_NETWORK_TYPE_INET		       /**< Internet */
} fsdp_network_type_t;

/**
 * @short Type of address
 *
 * Initially, IPv4 and IPv6 are defined for the network type
 * Internet. New address types may be registered with IANA.
 **/
typedef enum
{
  FSDP_ADDRESS_TYPE_UNDEFINED,		       /**< Not provided */
  FSDP_ADDRESS_TYPE_IPV4,		      /**< IP version 4 */
  FSDP_ADDRESS_TYPE_IPV6		      /**< IP version 6 */
} fsdp_address_type_t;

/**
 * @short Type of bandwith modifiers
 *
 * Bandwidth modifiers specify the meaning of the bandwidth
 * value. Initially "Conference Total" and "Application Specific" are
 * defined. Both use kilobits as bandwidth unit. "Conference Total"
 * specifies that the bandwidth value is a proposed upper limit to the
 * session bandwidth. "Application Specific" specifies thath the
 * bandwidth value is the application concept of maximum bandwidth.
 **/
typedef enum
{
  FSDP_BW_MOD_TYPE_UNDEFINED,		 /**< Not provided */
  FSDP_BW_MOD_TYPE_UNKNOWN,		 /**< Unknown bandwidth
						  modifier (FreeSDP
						  ignores it) */
  FSDP_BW_MOD_TYPE_CONFERENCE_TOTAL,	 /**< "CT - Conference Total" */
  FSDP_BW_MOD_TYPE_APPLICATION_SPECIFIC, /**< "AS - Application specific" */
  FSDP_BW_MOD_TYPE_RTCP_SENDERS,	 /**< "RS - RTCP bandwidth for
					    senders */
  FSDP_BW_MOD_TYPE_RTCP_RECEIVERS,	 /**< "RR - RTCP bandwidth for
					    receivers */
} fsdp_bw_modifier_type_t;

/**
 * @short encryption method
 *
 * The encryption method specifies the way to get the encryption key.
 **/
typedef enum
{
  FSDP_ENCRYPTION_METHOD_UNDEFINED,    /**< Not provided */
  FSDP_ENCRYPTION_METHOD_CLEAR,	       /**< The key field is the
						 untransformed key */
  FSDP_ENCRYPTION_METHOD_BASE64,       /**< The key is base64
					  encoded */
  FSDP_ENCRYPTION_METHOD_URI,	       /**< The key value provided is
					  a URI pointing to the actual
					  key */
  FSDP_ENCRYPTION_METHOD_PROMPT	       /**< The key is not provided
					  but should be got prompting
					  the user */
} fsdp_encryption_method_t;

/**
 * @short Advised reception/transmission mode
 *
 * Depending on wheter sendrecv, recvonly, sendonly or inactive
 * attribute is given, the tools used to participate in the session
 * should be started in the corresponding transmission
 * mode. FSDP_SENDRECV_SENDRECV is the default for sessions which are
 * not of the conference type broadcast or H332.
 **/
typedef enum
{
  FSDP_SENDRECV_UNDEFINED,		      /**< Not specified */
  FSDP_SENDRECV_SENDRECV,		      /**< Send and receive */
  FSDP_SENDRECV_RECVONLY,		      /**< Receive only */
  FSDP_SENDRECV_SENDONLY,		      /**< Send only */
  FSDP_SENDRECV_INACTIVE		      /**< Do not send nor receive */
} fsdp_sendrecv_mode_t;

/**
 * @short Values for `orient' media attribute.
 *
 * Normally used with whiteboard media, this attribute specifies the
 * orientation of the whiteboard.
 **/
typedef enum
{
  FSDP_ORIENT_UNDEFINED,		     /**< Not specified */
  FSDP_ORIENT_PORTRAIT,			     /**< Portrait */
  FSDP_ORIENT_LANDSCAPE,		     /**< Landscape */
  FSDP_ORIENT_SEASCAPE			     /**< Upside down landscape */
} fsdp_orient_t;

/**
 * @short Type of the conference
 *
 * The following types are initially defined: broadcast, meeting,
 * moderated, test and H332.
 **/
typedef enum
{
  FSDP_SESSION_TYPE_UNDEFINED,		       /**< Not specified */
  FSDP_SESSION_TYPE_BROADCAST,		       /**< Broadcast session */
  FSDP_SESSION_TYPE_MEETING,		       /**< Meeting session */
  FSDP_SESSION_TYPE_MODERATED,		       /**< Moderated session */
  FSDP_SESSION_TYPE_TEST,		       /**< Test (do not display) */
  FSDP_SESSION_TYPE_H332		       /**< H332 session */
} fsdp_session_type_t;

/**
 * @short Media type
 *
 * The following types are defined initially: audio, video,
 * application, data and control.
 **/
typedef enum
{
  FSDP_MEDIA_UNDEFINED,		   /**< Not specified */
  FSDP_MEDIA_VIDEO,		   /**< Video */
  FSDP_MEDIA_AUDIO,		   /**< Audio */
  FSDP_MEDIA_APPLICATION,	   /**< Application, such as whiteboard */
  FSDP_MEDIA_DATA,		   /**< bulk data */
  FSDP_MEDIA_CONTROL		   /**< Control channel */
} fsdp_media_t;

/**
 * @short Transport protocol
 *
 * The transport protocol used depends on the address type. Initially,
 * RTP over UDP Audio/Video Profile, and UDP are defined.
 *
 **/
typedef enum
{
  FSDP_TP_UNDEFINED,		  /**< Not specified */
  FSDP_TP_RTP_AVP,		  /**< RTP Audio/Video Profile */
  FSDP_TP_UDP,			  /**< UDP */
  FSDP_TP_TCP,			  /**< TCP */
  FSDP_TP_UDPTL,		  /**< ITU-T T.38*/
  FSDP_TP_VAT,			  /**< old vat protocol (historic)*/
  FSDP_TP_OLD_RTP,		  /**< old rtp protocols (historic)*/
  FSDP_TP_H320			  /**< TODO: add to the parser */
} fsdp_transport_protocol_t;

/**
 * Session-level attributes whose value is specified as a character
 * string in FreeSDP. These values are usually given to
 * fsdp_get_strn_att() in order to get the corresponding value.
 *
 **/
typedef enum
{
  FSDP_SESSION_STR_ATT_CATEGORY,
  FSDP_SESSION_STR_ATT_KEYWORDS,
  FSDP_SESSION_STR_ATT_TOOL,
  FSDP_SESSION_STR_ATT_CHARSET,
} fsdp_session_str_att_t;

/**
 * @short FreeSDP SDP description media object.
 *
 * Object for media specific information in SDP descriptions. Each SDP
 * description may include any number of media section. A
 * fsdp_media_description_t object encapsulates the information in a
 * media section, such as video, audio or whiteboard.
 **/
typedef struct fsdp_media_description_t_s fsdp_media_description_t;

/**
 * @short FreeSDP SDP session description object.
 *
 * Contains all the information extracted from a textual SDP
 * description, including all the media announcements.
 **/
typedef struct fsdp_description_t_s fsdp_description_t;

/**
 * Allocates memory and initializes values for a new
 * fsdp_description_t object. If you call this routine, do not forget
 * about <code>fsdp_description_delete()</code>
 *
 * @return new fsdp_description_t object
 **/
fsdp_description_t *fsdp_description_new (void);

/**
 * Destroys a fsdp_description_t object.
 *
 * @param dsc pointer to the fsdp_description_t object to delete.
 **/
void fsdp_description_delete (fsdp_description_t * dsc);

/**
 * Calling this function over a description is equivalent to calling
 * fsdp_description_delete and then fsdp_description_delete. This
 * function is however more suitable and efficient for description
 * processing loops.
 *
 * @param dsc pointer to the fsdp_description_t object to
 * renew/recycle.
 **/
void fsdp_description_recycle (fsdp_description_t * dsc);

/**
 *  * Returns a string correspondent to the error number.
 *   *
 *    * @param err_no error number.
 *     **/
const char *fsdp_strerror (fsdp_error_t err_no);

       /*@}*//* closes addtogroup common */

char *base64_encode(char *out, int out_size, const uint8_t *in, int in_size);
void md5_sum(char *buf,const unsigned char *data, const unsigned int len);

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)

#   define RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])

#   define WB16(p, darg) do {                \
        unsigned d = (darg);                    \
        ((uint8_t*)(p))[1] = (d);               \
        ((uint8_t*)(p))[0] = (d)>>8;            \
    } while(0)

#   define RL16(x)                           \
    ((((const uint8_t*)(x))[1] << 8) |          \
      ((const uint8_t*)(x))[0])

#   define WL16(p, darg) do {                \
        unsigned d = (darg);                    \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
    } while(0)

#   define RB32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])

#   define WB32(p, darg) do {                \
        unsigned d = (darg);                    \
        ((uint8_t*)(p))[3] = (d);               \
        ((uint8_t*)(p))[2] = (d)>>8;            \
        ((uint8_t*)(p))[1] = (d)>>16;           \
        ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)

#   define RL32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[3] << 24) |    \
               (((const uint8_t*)(x))[2] << 16) |    \
               (((const uint8_t*)(x))[1] <<  8) |    \
                ((const uint8_t*)(x))[0])

#   define WL32(p, darg) do {                \
        unsigned d = (darg);                    \
        ((uint8_t*)(p))[0] = (d);               \
        ((uint8_t*)(p))[1] = (d)>>8;            \
        ((uint8_t*)(p))[2] = (d)>>16;           \
        ((uint8_t*)(p))[3] = (d)>>24;           \
    } while(0)

#endif /* COMMON_H */
