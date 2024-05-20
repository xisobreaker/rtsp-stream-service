#include "http_auth.h"
#include "crypto.h"
#include "memutils.h"
#include "strutils.h"
#include "urlcodec.h"
#include <memory>

std::string make_basic_auth(const char *auth)
{
    int  length = strlen(auth);
    char response[length * 2];
    memset(response, 0, length * 2);

    base64_encode(auth, strlen(auth), response, length * 2);
    return std::string(response);
}

std::string make_digest_auth(HTTPAuthState *state, const char *username, const char *password, const char *uri, const char *method)
{
    char        a1hash[64] = {0}, a2hash[64] = {0}, response[64] = {0};
    std::string a1EncryptStr = std::string(username) + ":" + std::string(state->realm) + ":" + std::string(password);
    std::string a2EncryptStr = std::string(method) + ":" + std::string(uri);

    DigestParams *digest = &state->digest_params;
    md5_encrypt_hex((unsigned char *)a1hash, (unsigned char *)a1EncryptStr.c_str(), a1EncryptStr.length());
    md5_encrypt_hex((unsigned char *)a2hash, (unsigned char *)a2EncryptStr.c_str(), a2EncryptStr.length());
    std::string resEncryptStr = std::string(a1hash) + ":" + std::string(digest->nonce) + ":" + std::string(a2hash);
    md5_encrypt_hex((unsigned char *)response, (unsigned char *)resEncryptStr.c_str(), resEncryptStr.length());
    return std::string(response);
}

std::string http_auth_create_response(HTTPAuthState *state, const char *auth, const char *uri, const char *method)
{
    std::shared_ptr<char> authorization(new char[512], std::default_delete<char[]>());
    if (state->auth_type == HTTP_AUTH_BASIC) {
        char *username = url_decode(auth, 0);
        if (username) {
            std::string response = make_basic_auth(username);
            snprint_lcatf(authorization.get(), 512, "Authorization: Basic %s\r\n", response.c_str());
        }
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
                snprint_lcatf(authorization.get(), 512, ", response=\"%s\"\r\n", response.c_str());
            }
            mem_free(username);
        }
    }
    return std::string(authorization.get());
}