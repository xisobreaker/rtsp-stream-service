#pragma once

#include "rtsp-defines.h"
#include <memory>

std::shared_ptr<char> http_auth_create_response(HTTPAuthState *state, const char *auth, const char *uri, const char *method);
std::shared_ptr<char> rtsp_method_encode(RTSPContext *ctx, const char *method, const char *uri, const char *headers);
