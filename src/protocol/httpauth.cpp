#include "httpauth.h"

#include "md5.h"
#include "memutils.h"
#include "strutils.h"
#include "urlcodec.h"

using namespace std;

std::shared_ptr<char> make_digest_auth(
    HTTPAuthState *state, const char *username, const char *password, const char *uri, const char *method)
{
    DigestParams         *digest = &state->digest_params;
    char                  a1hash[33] = {0}, a2hash[33] = {0};
    std::shared_ptr<char> response(new char[33], default_delete<char[]>());

    string a1EncryptStr = string(username) + ":" + string(state->realm) + ":" + string(password);
    md5_encrypt_hex((unsigned char *)a1hash, (unsigned char *)a1EncryptStr.c_str(), a1EncryptStr.length());
    string a2EncryptStr = string(method) + ":" + string(uri);
    md5_encrypt_hex((unsigned char *)a2hash, (unsigned char *)a2EncryptStr.c_str(), a2EncryptStr.length());
    string resEncryptStr = string(a1hash) + ":" + string(digest->nonce) + ":" + string(a2hash);
    md5_encrypt_hex((unsigned char *)response.get(), (unsigned char *)resEncryptStr.c_str(), resEncryptStr.length());
    return response;
}

std::shared_ptr<char> http_auth_create_response(HTTPAuthState *state, const char *auth, const char *uri, const char *method)
{
    shared_ptr<char> authorization(new char[512], default_delete<char[]>());
    if (state->auth_type == HTTP_AUTH_BASIC) {
    } else if (state->auth_type == HTTP_AUTH_DIGEST) {
        char *username = url_decode(auth, 0);
        if (username) {
            char *password = strchr(username, ':');
            if (password) {
                *password++ = '\0';
                shared_ptr<char> response = make_digest_auth(state, username, password, uri, method);
                snprint_lcatf(authorization.get(), 512, "Authorization: Digest username=\"%s\"", username);
                snprint_lcatf(authorization.get(), 512, ", realm=\"%s\"", state->realm);
                snprint_lcatf(authorization.get(), 512, ", nonce=\"%s\"", state->digest_params.nonce);
                snprint_lcatf(authorization.get(), 512, ", uri=\"%s\"", uri);
                snprint_lcatf(authorization.get(), 512, ", response=\"%s\"", response.get());
            }
            mem_free(username);
        }
    }
    return authorization;
}