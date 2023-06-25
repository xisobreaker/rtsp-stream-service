#pragma once

#include "httpauth.h"
#include "rtspdefs.h"
#include <memory>

typedef struct {
    int                  seq;
    char                *user_agent;
    char                 session_id[512];
    char                 auth[128];
    HTTPAuthState        auth_state;
    RTSPControlTransport control_transport;
} RTSPContext;

std::shared_ptr<char> rtsp_method_encode(RTSPContext *ctx, const char *method, const char *uri, const char *headers);
