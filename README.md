# RTSP-Stream-Service

## RTSP

RTSP(Real-TimeStream Protocol )是一种基于文本的应用层协议，一般默认端口为 554。

RTSP 1.0: [`Real Time Streaming Protocol (RTSP)`](https://www.rfc-editor.org/rfc/rfc2326)

RTSP 2.0: [`Real-Time Streaming Protocol Version 2.0`](https://www.rfc-editor.org/rfc/rfc7826)

当使用 TCP 方式传输数据时，会使用 Interleaved Frame 来传输数据。Interleaved Frame 封装格式使用美元符号 $ 作为起始符（0x24）占一个字节，之后是一个字节的通道标识，然后是两个字节的数据包长度（网络字节序），最后是跟随的数据，如 RTP 包。

RTSP认证主要分两种，基本认证（basic authentication）和摘要认证（digest authentication）。

## SDP

SDP: [`RFC 2327`](https://www.rfc-editor.org/rfc/rfc2327)

Session 描述信息说明

```
v=  (protocol version)
o=  (owner/creator and session identifier).
s=  (session name)
i=* (session information)
u=* (URI of description)
e=* (email address)
p=* (phone number)
c=* (connection information - not required if included in all media)
b=* (bandwidth information)
One or more time descriptions (see below)
z=* (time zone adjustments)
k=* (encryption key)
a=* (zero or more session attribute lines)
Zero or more media descriptions (see below)
```

Time 描述信息说明

```
t=  (time the session is active)
r=* (zero or more repeat times)
```

Media 描述信息说明

```
m=  (media name and transport address)
i=* (media title)
c=* (connection information - optional if included at session-level)
b=* (bandwidth information)
k=* (encryption key)
a=* (zero or more media attribute lines)
```

下面是一个 SDP 描述信息示例：

```
v=0
o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4
s=SDP Seminar
i=A Seminar on the session description protocol
u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps
e=mjh@isi.edu (Mark Handley)
c=IN IP4 224.2.17.12/127
t=2873397496 2873404696
a=recvonly
m=audio 49170 RTP/AVP 0
m=video 51372 RTP/AVP 31
m=application 32416 udp wb
a=orient:portrait
```

```
o=<username> <session id> <version> <network type> <address type> <address>
```

## RTCP

[`RFC 3550`](https://www.rfc-editor.org/rfc/rfc3550) 定义了实时传输协议 RTP 和它的控制协议 RTCP。

协议定义五种 RTCP 报文：

| 类型 | 缩写 | 说明       |
| ---- | ---- | ---------- |
| 200  | SR   | 发送端报告 |
| 201  | RR   | 接收端报告 |
| 202  | SDES | 源端描述   |
| 203  | BYE  | 结束传输   |
| 204  | APP  | 特定应用   |

## RTP



# FFmpeg 命令使用

## 抓取视频流

```bash
ffmpeg -rtsp_transport tcp -i "rtsp://192.168.1.100:554/cam/realmonitor?channel=1&subtype=0" -vcodec copy -an video.mp4
```

## 推送视频流

```bash
ffmpeg -re -stream_loop -1 -i input.mp4 -c copy -f rtsp "rtsp://192.168.1.100:554/cam/realmonitor?channel=1&subtype=0"
```

## 以下是一个完整的 RTSP 协议数据展示

```
OPTIONS rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0 RTSP/1.0
CSeq: 1
User-Agent: Lavf58.29.100

RTSP/1.0 401 Unauthorized
CSeq: 1
WWW-Authenticate: Digest realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05"

OPTIONS rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0 RTSP/1.0
CSeq: 2
User-Agent: Lavf58.29.100
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0", response="455412194d84f2672426f0fa3c9fa51e"

RTSP/1.0 200 OK
CSeq: 2
Server: Rtsp Server/3.0
Public: OPTIONS, DESCRIBE, ANNOUNCE, SETUP, PLAY, RECORD, PAUSE, TEARDOWN, SET_PARAMETER, GET_PARAMETER

DESCRIBE rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0 RTSP/1.0
Accept: application/sdp
CSeq: 3
User-Agent: Lavf58.29.100
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0", response="85309077dec089c2d7c565b892eb3098"

RTSP/1.0 200 OK
CSeq: 3
x-Accept-Dynamic-Rate: 1
Content-Base: rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/
Cache-Control: must-revalidate
Content-Length: 465
Content-Type: application/sdp

v=0
o=- 2254270591 2254270591 IN IP4 0.0.0.0
s=Media Server
c=IN IP4 0.0.0.0
t=0 0
a=control:*
a=packetization-supported:DH
a=rtppayload-supported:DH
a=range:npt=now-
m=video 0 RTP/AVP 96
a=control:trackID=0
a=framerate:10.000000
a=rtpmap:96 H264/90000
a=fmtp:96 packetization-mode=1;profile-level-id=4D002A;sprop-parameter-sets=Z00AKpY1QPAET8s3BQEFAgA=,aO4xsgA=
a=recvonly
m=audio 0 RTP/AVP 8
a=control:trackID=1
a=rtpmap:8 PCMA/8000
a=recvonly
SETUP rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/trackID=0 RTSP/1.0
Transport: RTP/AVP/UDP;unicast;client_port=18292-18293
x-Dynamic-Rate: 0
CSeq: 4
User-Agent: Lavf58.29.100
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/trackID=0", response="56aaf7d7759ef992f1bd1771cbbc0a20"

RTSP/1.0 200 OK
CSeq: 4
Session: 2936719851115;timeout=60
Transport: RTP/AVP/UDP;unicast;client_port=18292-18293;server_port=23938-23939;ssrc=459838E1
x-Dynamic-Rate: 1

SETUP rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/trackID=1 RTSP/1.0
Transport: RTP/AVP/UDP;unicast;client_port=18294-18295
x-Dynamic-Rate: 0
CSeq: 5
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/trackID=1", response="bcc1eb6d307fb31ff20d760d651bea8f"

RTSP/1.0 200 OK
CSeq: 5
Session: 2936719851115;timeout=60
Transport: RTP/AVP/UDP;unicast;client_port=18294-18295;server_port=23940-23941;ssrc=15AE4DD8
x-Dynamic-Rate: 1

PLAY rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
Range: npt=0.000-
CSeq: 6
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="46fe71a4882a5376eb7dcdc13b21707c"

RTSP/1.0 200 OK
CSeq: 6
Session: 2936719851115
Range: npt=0.000000-
RTP-Info: url=trackID=0;seq=14561;rtptime=14561,url=trackID=1;seq=14561;rtptime=14561

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 7
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 7
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 8
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 8
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 9
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 9
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 10
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 10
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 11
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 11
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 12
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 12
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 13
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 13
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 14
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 14
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 15
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 15
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 16
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 16
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 17
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 17
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 18
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 18
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 19
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 19
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 20
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 20
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 21
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 21
Session: 2936719851115

GET_PARAMETER rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 22
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="957abe572391c12dcd82f3a518813f2d"

RTSP/1.0 200 OK
CSeq: 22
Session: 2936719851115

TEARDOWN rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/ RTSP/1.0
CSeq: 23
User-Agent: Lavf58.29.100
Session: 2936719851115
Authorization: Digest username="admin", realm="Login to edbba66a86f75696d122e195153df0a2", nonce="ba0294a902ef637b4ac19025d26d0a05", uri="rtsp://192.168.3.51:554/cam/realmonitor?channel=1&subtype=0/", response="8f4dbd0c98c7adabff9157afcecf5e51"

RTSP/1.0 200 OK
CSeq: 23
Session: 2936719851115
```