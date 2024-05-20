#pragma once

#define SPACE_CHARS " \t\r\n"
#define RTSP_DEFAULT_PORT 554
#define RTSPS_DEFAULT_PORT 322
#define RTSP_RTP_PORT_MIN 5000
#define RTSP_RTP_PORT_MAX 65000
#define SDP_MAX_SIZE 16384
#define RTSP_NOPTS_VALUE ((int64_t)UINT64_C(0x8000000000000000))
#define RTSP_DEFAULT_AUDIO_SAMPLERATE 44100

/**********************************************************
 * RTSP 数据传输模式。简单模式或隧道（基于HTTP）。
 **********************************************************/
enum RTSPControlTransport {
    RTSP_MODE_PLAIN, // 一般的 RTSP
    RTSP_MODE_TUNNEL // 基于 HTTP 的 RTSP
};

/**********************************************************
 * RTSP 命令字
 **********************************************************/
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
 * Status code, 见 RFC 7826
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

/**********************************************************
 * RTSP 客户端状态。
 **********************************************************/
enum RTSPClientState {
    RTSP_STATE_IDLE,
    RTSP_STATE_STREAMING,
    RTSP_STATE_PAUSED,
    RTSP_STATE_SEEKING,
};

/**********************************************************
 * RTSP Transport。
 **********************************************************/
enum RTSPTransport {
    RTSP_TRANSPORT_RTP,
    RTSP_TRANSPORT_RDT,
    RTSP_TRANSPORT_RAW,
    RTSP_TRANSPORT_NB
};

/**********************************************************
 * 传输 RTP/etc 数据包的网络层类型。
 **********************************************************/
enum RTSPLowerTransport {
    RTSP_LOWER_TRANSPORT_UDP = 0,
    RTSP_LOWER_TRANSPORT_TCP = 1,
    RTSP_LOWER_TRANSPORT_UDP_MULTICAST = 2,
    RTSP_LOWER_TRANSPORT_NB,
    RTSP_LOWER_TRANSPORT_HTTP = 8,
    RTSP_LOWER_TRANSPORT_HTTPS,
    RTSP_LOWER_TRANSPORT_CUSTOM = 16,
};

/**********************************************************
 * RTSP 服务器类型。
 **********************************************************/
enum RTSPServerType {
    RTSP_SERVER_RTP,
    RTSP_SERVER_REAL,
    RTSP_SERVER_WMS,
    RTSP_SERVER_SATIP,
    RTSP_SERVER_NB
};