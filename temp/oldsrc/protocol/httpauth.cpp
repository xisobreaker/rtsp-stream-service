#include "httpauth.h"

#include "md5.h"
#include "memutils.h"
#include "strutils.h"
#include "urlcodec.h"
#include <memory>

using namespace std;

/**
 * 生成 digest 加密串
 * @param state
 * @param username 用户名
 * @param password 密码
 * @param uri      uri 地址
 * @param method   RTSP 命令
 */
std::string make_digest_auth(HTTPAuthState *state, const char *username, const char *password, const char *uri, const char *method)
{
    DigestParams         *digest = &state->digest_params;
    char                  a1hash[33] = {0}, a2hash[33] = {0};
    std::shared_ptr<char> response((char *)mem_malloc(33), MemDeleter());

    string a1EncryptStr = string(username) + ":" + string(state->realm) + ":" + string(password);
    md5_encrypt_hex((unsigned char *)a1hash, (unsigned char *)a1EncryptStr.c_str(), a1EncryptStr.length());
    string a2EncryptStr = string(method) + ":" + string(uri);
    md5_encrypt_hex((unsigned char *)a2hash, (unsigned char *)a2EncryptStr.c_str(), a2EncryptStr.length());
    string resEncryptStr = string(a1hash) + ":" + string(digest->nonce) + ":" + string(a2hash);
    md5_encrypt_hex((unsigned char *)response.get(), (unsigned char *)resEncryptStr.c_str(), resEncryptStr.length());
    return std::string(response.get());
}

void http_auth_handle_header(HTTPAuthState *state, const char *key, const char *value)
{
    // if (!string_(key, "WWW-Authenticate") || !av_strcasecmp(key, "Proxy-Authenticate")) {
    //     const char *p;
    //     if (av_stristart(value, "Basic ", &p) && state->auth_type <= HTTP_AUTH_BASIC) {
    //         state->auth_type = HTTP_AUTH_BASIC;
    //         state->realm[0] = 0;
    //         state->stale = 0;
    //         ff_parse_key_value(p, (ff_parse_key_val_cb)handle_basic_params, state);
    //     } else if (av_stristart(value, "Digest ", &p) && state->auth_type <= HTTP_AUTH_DIGEST) {
    //         state->auth_type = HTTP_AUTH_DIGEST;
    //         memset(&state->digest_params, 0, sizeof(DigestParams));
    //         state->realm[0] = 0;
    //         state->stale = 0;
    //         ff_parse_key_value(p, (ff_parse_key_val_cb)handle_digest_params, state);
    //         choose_qop(state->digest_params.qop, sizeof(state->digest_params.qop));
    //         if (!av_strcasecmp(state->digest_params.stale, "true"))
    //             state->stale = 1;
    //     }
    // } else if (!av_strcasecmp(key, "Authentication-Info")) {
    //     ff_parse_key_value(value, (ff_parse_key_val_cb)handle_digest_update, state);
    // }
}

std::shared_ptr<char> http_auth_create_response(HTTPAuthState *state, const char *auth, const char *uri, const char *method)
{
    shared_ptr<char> authorization((char *)mem_malloc(512), MemDeleter());
    if (state->auth_type == HTTP_AUTH_BASIC) {
    } else if (state->auth_type == HTTP_AUTH_DIGEST) {
        char *username = url_decode(auth, 0);
        if (username) {
            char *password = strchr(username, ':');
            if (password) {
                *password++ = '\0';
                std::string response = make_digest_auth(state, username, password, uri, method);
                snprint_lcatf(authorization.get(), 512, "Authorization: Digest username=\"%s\"", username);
                snprint_lcatf(authorization.get(), 512, ", realm=\"%s\"", state->realm);
                snprint_lcatf(authorization.get(), 512, ", nonce=\"%s\"", state->digest_params.nonce);
                snprint_lcatf(authorization.get(), 512, ", uri=\"%s\"", uri);
                snprint_lcatf(authorization.get(), 512, ", response=\"%s\"", response.c_str());
            }
            mem_free(username);
        }
    }
    return authorization;
}