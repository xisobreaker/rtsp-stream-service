#pragma once

#define RTSP_DEFAULT_PORT 554
#define RTSPS_DEFAULT_PORT 322
#define RTSP_MAX_TRANSPORTS 8
#define RTSP_DEFAULT_AUDIO_SAMPLERATE 44100
#define RTSP_RTP_PORT_MIN 5000
#define RTSP_RTP_PORT_MAX 65000
#define SDP_MAX_SIZE 16384

enum RTSPMethod {
    RTSP_METHOD_DESCRIBE,
    RTSP_METHOD_ANNOUNCE,
    RTSP_METHOD_OPTIONS,
    RTSP_METHOD_SETUP,
    RTSP_METHOD_PLAY,
    RTSP_METHOD_PAUSE,
    RTSP_METHOD_TEARDOWN,
    RTSP_METHOD_GET_PARAMETER,
    RTSP_METHOD_SET_PARAMETER,
    RTSP_METHOD_REDIRECT,
    RTSP_METHOD_RECORD,
    RTSP_METHOD_UNKNOWN = -1,
};

/**********************************************************
* Status code, 摘抄自RFC 7826
*
/  "100"  ; Continue
/  "200"  ; OK
/  "301"  ; Moved Permanently
/  "302"  ; Found
/  "303"  ; See Other
/  "304"  ; Not Modified
/  "305"  ; Use Proxy
/  "400"  ; Bad Request
/  "401"  ; Unauthorized
/  "402"  ; Payment Required
/  "403"  ; Forbidden
/  "404"  ; Not Found
/  "405"  ; Method Not Allowed
/  "406"  ; Not Acceptable
/  "407"  ; Proxy Authentication Required
/  "408"  ; Request Timeout
/  "410"  ; Gone
/  "412"  ; Precondition Failed
/  "413"  ; Request Message Body Too Large
/  "414"  ; Request-URI Too Long
/  "415"  ; Unsupported Media Type
/  "451"  ; Parameter Not Understood
/  "452"  ; reserved
/  "453"  ; Not Enough Bandwidth
/  "454"  ; Session Not Found
/  "455"  ; Method Not Valid In This State
/  "456"  ; Header Field Not Valid for Resource
/  "457"  ; Invalid Range
/  "458"  ; Parameter Is Read-Only
/  "459"  ; Aggregate Operation Not Allowed
/  "460"  ; Only Aggregate Operation Allowed
/  "461"  ; Unsupported Transport
/  "462"  ; Destination Unreachable
/  "463"  ; Destination Prohibited
/  "464"  ; Data Transport Not Ready Yet
/  "465"  ; Notification Reason Unknown
/  "466"  ; Key Management Error
/  "470"  ; Connection Authorization Required
/  "471"  ; Connection Credentials Not Accepted
/  "472"  ; Failure to Establish Secure Connection
/  "500"  ; Internal Server Error
/  "501"  ; Not Implemented
/  "502"  ; Bad Gateway
/  "503"  ; Service Unavailable
/  "504"  ; Gateway Timeout
/  "505"  ; RTSP Version Not Supported
/  "551"  ; Option Not Supported
/  "553"  ; Proxy Unavailable
**********************************************************/
enum RTSPStatusCode {
    RTSP_STATUS_CONTINUE = 100,
    RTSP_STATUS_OK = 200,
    RTSP_STATUS_CREATED = 201,
    RTSP_STATUS_LOW_ON_STORAGE_SPACE = 250,
    RTSP_STATUS_MULTIPLE_CHOICES = 300,
    RTSP_STATUS_MOVED_PERMANENTLY = 301,
    RTSP_STATUS_MOVED_TEMPORARILY = 302,
    RTSP_STATUS_SEE_OTHER = 303,
    RTSP_STATUS_NOT_MODIFIED = 304,
    RTSP_STATUS_USE_PROXY = 305,
    RTSP_STATUS_BAD_REQUEST = 400,
    RTSP_STATUS_UNAUTHORIZED = 401,
    RTSP_STATUS_PAYMENT_REQUIRED = 402,
    RTSP_STATUS_FORBIDDEN = 403,
    RTSP_STATUS_NOT_FOUND = 404,
    RTSP_STATUS_METHOD = 405,
    RTSP_STATUS_NOT_ACCEPTABLE = 406,
    RTSP_STATUS_PROXY_AUTH_REQUIRED = 407,
    RTSP_STATUS_REQ_TIME_OUT = 408,
    RTSP_STATUS_GONE = 410,
    RTSP_STATUS_LENGTH_REQUIRED = 411,
    RTSP_STATUS_PRECONDITION_FAILED = 412,
    RTSP_STATUS_REQ_ENTITY_2LARGE = 413,
    RTSP_STATUS_REQ_URI_2LARGE = 414,
    RTSP_STATUS_UNSUPPORTED_MTYPE = 415,
    RTSP_STATUS_PARAM_NOT_UNDERSTOOD = 451,
    RTSP_STATUS_CONFERENCE_NOT_FOUND = 452,
    RTSP_STATUS_BANDWIDTH = 453,
    RTSP_STATUS_SESSION = 454,
    RTSP_STATUS_STATE = 455,
    RTSP_STATUS_INVALID_HEADER_FIELD = 456,
    RTSP_STATUS_INVALID_RANGE = 457,
    RTSP_STATUS_RONLY_PARAMETER = 458,
    RTSP_STATUS_AGGREGATE = 459,
    RTSP_STATUS_ONLY_AGGREGATE = 460,
    RTSP_STATUS_TRANSPORT = 461,
    RTSP_STATUS_UNREACHABLE = 462,
    RTSP_STATUS_INTERNAL = 500,
    RTSP_STATUS_NOT_IMPLEMENTED = 501,
    RTSP_STATUS_BAD_GATEWAY = 502,
    RTSP_STATUS_SERVICE = 503,
    RTSP_STATUS_GATEWAY_TIME_OUT = 504,
    RTSP_STATUS_VERSION = 505,
    RTSP_STATUS_UNSUPPORTED_OPTION = 551,
    RTSP_STATUS_UNAVAILABLE_PROXY = 553,
};

/**
 * 传输 RTP/etc 数据包的网络层类型。
 */
enum RTSPLowerTransport {
    RTSP_LOWER_TRANSPORT_UDP = 0,           /**< UDP/unicast */
    RTSP_LOWER_TRANSPORT_TCP = 1,           /**< TCP; interleaved in RTSP */
    RTSP_LOWER_TRANSPORT_UDP_MULTICAST = 2, /**< UDP/multicast */
    RTSP_LOWER_TRANSPORT_NB,
    RTSP_LOWER_TRANSPORT_HTTP = 8,    /**< HTTP tunneled - not a proper
                                           transport mode as such,
                                           only for use via AVOptions */
    RTSP_LOWER_TRANSPORT_HTTPS,       /**< HTTPS tunneled */
    RTSP_LOWER_TRANSPORT_CUSTOM = 16, /**< Custom IO - not a public
                                           option for lower_transport_mask,
                                           but set in the SDP demuxer based
                                           on a flag. */
};

/**
 * Packet profile of the data that we will be receiving. Real servers
 * commonly send RDT (although they can sometimes send RTP as well),
 * whereas most others will send RTP.
 */
enum RTSPTransport {
    RTSP_TRANSPORT_RTP, /**< Standards-compliant RTP */
    RTSP_TRANSPORT_RDT, /**< Realmedia Data Transport */
    RTSP_TRANSPORT_RAW, /**< Raw data (over UDP) */
    RTSP_TRANSPORT_NB
};

/**
 * Client state, i.e. whether we are currently receiving data (PLAYING) or
 * setup-but-not-receiving (PAUSED). State can be changed in applications
 * by calling av_read_play/pause().
 */
enum RTSPClientState {
    RTSP_STATE_IDLE,      /**< not initialized */
    RTSP_STATE_STREAMING, /**< initialized and sending/receiving data */
    RTSP_STATE_PAUSED,    /**< initialized, but not receiving data */
    RTSP_STATE_SEEKING,   /**< initialized, requesting a seek */
};

/**
 * Identify particular servers that require special handling, such as
 * standards-incompliant "Transport:" lines in the SETUP request.
 */
enum RTSPServerType {
    RTSP_SERVER_RTP,   /**< Standards-compliant RTP-server */
    RTSP_SERVER_REAL,  /**< Realmedia-style server */
    RTSP_SERVER_WMS,   /**< Windows Media server */
    RTSP_SERVER_SATIP, /**< SAT>IP server */
    RTSP_SERVER_NB
};

/**
 * RTSP 数据传输模式。简单模式或隧道（基于HTTP）。
 */
enum RTSPControlTransport {
    RTSP_MODE_PLAIN, /**< Normal RTSP */
    RTSP_MODE_TUNNEL /**< RTSP over HTTP (tunneling) */
};

/**
 * 身份验证类型，由弱到强。
 */
enum HTTPAuthType {
    HTTP_AUTH_NONE = 0, /**< 无身份校验 */
    HTTP_AUTH_BASIC,    /**< HTTP 1.0 基本认证 RFC 1945 (及 RFC 2617) */
    HTTP_AUTH_DIGEST,   /**< HTTP 1.1 摘要认证 RFC 2617 */
};