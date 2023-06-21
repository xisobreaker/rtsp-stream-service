#include "rtsp-utils.h"
#include "md5.h"
#include "rtsp-defines.h"
#include "strutils.h"
#include <cstring>
#include <memory>

using namespace std;

void split_video_url(RTSPUrlInfo *info, const char *url)
{
    const char *pos = nullptr;
    if ((pos = strchr(url, ':'))) {
        string_lcpy(info->proto, url, (pos + 1 - url));

        pos++; // skip ':'
        if (*pos == '/') {
            pos++;
        }
        if (*pos == '/') {
            pos++;
        }
    } else {
        string_lcpy(info->path, url, sizeof(info->path));
        return;
    }

    const char *lstr = pos + strcspn(pos, "/?#");
    string_lcpy(info->path, lstr, sizeof(info->path));

    if (lstr != pos) {
        const char *begin = pos, *end = nullptr;
        while ((end = strchr(pos, '@')) && end < lstr) {
            string_lcpy(info->authorization, begin, end + 1 - begin);
            pos = end + 1;
        }

        const char *brk = nullptr, *col = nullptr;
        if (*pos == '[' && (brk = strchr(pos, ']')) && brk < lstr) {
            string_lcpy(info->hostname, pos + 1, brk - pos);
            if (brk[1] == ':') {
                info->port = atoi(brk + 2);
            }
        } else if ((col = strchr(pos, ':')) && col < lstr) {
            string_lcpy(info->hostname, pos, col + 1 - pos);
            info->port = atoi(col + 1);
        } else {
            string_lcpy(info->hostname, pos, lstr + 1 - pos);
        }
    }
}

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
