#include "sdp.h"
#include "strutils.h"

/**
 * @brief 读取下一节点信息
 *
 * @param p
 * @param key
 * @param value
 * @return char*
 */
char *load_next_entry(char *p, char *key, char **value)
{
    if (!p)
        return nullptr;

    char *endl = strstr(p, "\r\n");
    if (endl == nullptr)
        endl = strchr(p, '\n');

    if (endl) {
        while (*endl == '\r' || *endl == '\n') {
            *endl++ = '\0';
        }
    } else {
        endl = &p[strlen(p)];
    }
    if (!p[0] || p[1] != '=') {
        goto fail;
    }

    *key   = p[0];
    *value = &p[2];
    return endl;

fail:
    *key   = 0;
    *value = nullptr;
    return nullptr;
}

#define ALLOCATE_MEM(filed)                                                       \
    do {                                                                          \
        if (!filed) {                                                             \
            filed = (char **)calloc(1, sizeof(*filed));                           \
            if (!filed) {                                                         \
                goto fail;                                                        \
            }                                                                     \
        } else {                                                                  \
            int    n         = filed##_count;                                     \
            char **new_filed = (char **)realloc(filed, sizeof(*filed) * (n + 1)); \
            if (!new_filed) {                                                     \
                goto fail;                                                        \
            }                                                                     \
            filed = new_filed;                                                    \
            memset(&filed[n], 0, sizeof(*filed));                                 \
        }                                                                         \
    } while (0)

struct SDPPayload *sdp_parser(const char *payload)
{
    struct SDPPayload *sdp = nullptr;
    char              *p, key, *value;

    sdp = (struct SDPPayload *)calloc(1, sizeof(struct SDPPayload));
    if (!sdp) {
        goto fail;
    }
    p = sdp->_payload = strdup(payload);
    if (!p) {
        goto fail;
    }

    /* v=  (protocol version) */
    p = load_next_entry(p, &key, &value);
    if (key != 'v') {
        goto fail;
    }
    sdp->proto_version = value[0] - '0';
    if (sdp->proto_version != 0 || value[1]) {
        goto fail;
    }

    /* o=  (originator and session identifier) */
    p = load_next_entry(p, &key, &value);
    if (key != 'o') {
        goto fail;
    } else {
        struct sdp_origin *o = &sdp->origin;
        str_split_values(value, ' ', "sllsss", &o->username, &o->sess_id, &o->sess_version, &o->nettype, &o->addrtype, &o->addr);
    }

    /* s=  (session name) */
    p = load_next_entry(p, &key, &value);
    if (key != 's') {
        goto fail;
    }
    sdp->session_name = value;

    /* i=* (session information) */
    p = load_next_entry(p, &key, &value);
    if (key == 'i') {
        sdp->session_info = value;
        p                 = load_next_entry(p, &key, &value);
    }

    /* u=* (URI of description) */
    if (key == 'u') {
        sdp->uri = value;
        p        = load_next_entry(p, &key, &value);
    }

    /* e=* (email address) */
    while (key == 'e') {
        ALLOCATE_MEM(sdp->emails);
        sdp->emails[sdp->emails_count++] = value;
        p                                = load_next_entry(p, &key, &value);
    }

    /* p=* (phone number) */
    while (key == 'p') {
        ALLOCATE_MEM(sdp->phones);
        sdp->phones[sdp->phones_count++] = value;
        p                                = load_next_entry(p, &key, &value);
    }

    /* c=* (connection information -- not required if included in all media) */
    if (key == 'c') {
        struct sdp_connection *c = &sdp->conn;
        str_split_values(value, ' ', "sss", &c->nettype, &c->addrtype, &c->address);
        p = load_next_entry(p, &key, &value);
    }

    /* b=* (zero or more bandwidth information lines) */
    while (key == 'b') {
        if (!sdp->bw) {
            sdp->bw = (sdp_bandwidth *)calloc(1, sizeof(sdp_bandwidth));
            if (!sdp->bw) {
                goto fail;
            }
        } else {
            sdp_bandwidth *new_bw = (sdp_bandwidth *)realloc(sdp->bw, sizeof(sdp_bandwidth) * (sdp->bw_count + 1));
            if (!new_bw) {
                goto fail;
            }
            sdp->bw = new_bw;
            memset(&sdp->bw[sdp->bw_count], 0, sizeof(sdp_bandwidth));
        }
        int n = sdp->bw_count++;
        str_split_values(value, ':', "ss", &sdp->bw[n].bwtype, &sdp->bw[n].bandwidth);
        p = load_next_entry(p, &key, &value);
    }

    /* One or more time descriptions ("t=" and "r=" lines;)
     * t=  (time the session is active)
     * r=* (zero or more repeat times)*/
    while (key == 't') {
        if (!sdp->times) {
            sdp->times = (sdp_time *)calloc(1, sizeof(sdp_time));
            if (!sdp->times) {
                goto fail;
            }
        } else {
            sdp_time *new_times = (sdp_time *)realloc(sdp->times, sizeof(sdp_time) * (sdp->times_count + 1));
            if (!new_times) {
                goto fail;
            }
            sdp->times = new_times;
            memset(&sdp->times[sdp->times_count], 0, sizeof(sdp_time));
        }
        struct sdp_time *t = &sdp->times[sdp->times_count++];
        str_split_values(value, ' ', "tt", &t->starttime, &t->stoptime);
        p = load_next_entry(p, &key, &value);

        while (key == 'r') {
            if (!t->repeat) {
                t->repeat = (sdp_repeat *)calloc(1, sizeof(sdp_repeat));
                if (!t->repeat) {
                    goto fail;
                }
            } else {
                sdp_repeat *new_repeat = (sdp_repeat *)realloc(t->repeat, sizeof(sdp_repeat) * (t->repeat_count + 1));
                if (!new_repeat) {
                    goto fail;
                }
                t->repeat = new_repeat;
                memset(&t->repeat[t->repeat_count], 0, sizeof(sdp_repeat));
            }
            struct sdp_repeat *r = &t->repeat[t->repeat_count++];
            value                = str_split_values(value, ' ', "tt", &r->interval, &r->duration);
            while (*value) {
                if (!r->offsets) {
                    r->offsets = (time_t *)calloc(1, sizeof(time_t));
                    if (!r->offsets) {
                        goto fail;
                    }
                } else {
                    time_t *new_offsets = (time_t *)realloc(r->offsets, sizeof(time_t) * (r->offsets_count + 1));
                    if (!new_offsets) {
                        goto fail;
                    }
                    r->offsets                   = new_offsets;
                    r->offsets[r->offsets_count] = 0;
                }
                int n = r->offsets_count++;
                value = str_split_values(value, ' ', "t", &r->offsets[n]);
            }
            p = load_next_entry(p, &key, &value);
        }
    }

    /**/
    if (key == 'z') {
        while (*value) {
            if (!sdp->timezone_adj) {
                sdp->timezone_adj = (sdp_timezone_adjustments *)calloc(1, sizeof(sdp_timezone_adjustments));
                if (!sdp->timezone_adj) {
                    goto fail;
                }
            } else {
                struct sdp_timezone_adjustments *new_timezone_adj = NULL;
                new_timezone_adj                                  = (sdp_timezone_adjustments *)realloc(sdp->timezone_adj,
                                                                       sizeof(sdp_timezone_adjustments) * (sdp->timezone_adj_count + 1));
                if (!new_timezone_adj) {
                    goto fail;
                }
                sdp->timezone_adj = new_timezone_adj;
                memset(&sdp->timezone_adj[sdp->timezone_adj_count], 0, sizeof(sdp_timezone_adjustments));
            }
            struct sdp_timezone_adjustments *tz_adj = &sdp->timezone_adj[sdp->timezone_adj_count++];
            value                                   = str_split_values(value, ' ', "tt", &tz_adj->adjust, &tz_adj->offset);
        }
        p = load_next_entry(p, &key, &value);
    }

    /**/
    if (key == 'k') {
        sdp->encrypt_key = value;
        p                = load_next_entry(p, &key, &value);
    }

    while (key == 'a') {
        ALLOCATE_MEM(sdp->attributes);
        sdp->attributes[sdp->attributes_count++] = value;
        p                                        = load_next_entry(p, &key, &value);
    }

    while (key == 'm') {
        struct sdp_media *new_medias = (sdp_media *)realloc(sdp->medias, sizeof(sdp_media) * (sdp->medias_count + 1));
        if (!new_medias) {
            goto fail;
        }
        memset(&new_medias[sdp->medias_count], 0, sizeof(sdp_media));
        sdp->medias = new_medias;

        struct sdp_media *m = &sdp->medias[sdp->medias_count++];
        value               = str_split_values(value, ' ', "s", &m->info.type);
        m->info.port        = strtol(value, &value, 10);
        if (*value == '/') {
            m->info.port_n = strtol(value + 1, &value, 10);
        }
        value = str_split_values(value, ' ', "s", &m->info.proto);
        while (*value) {
            int *new_fmt = (int *)realloc(m->info.fmt, sizeof(int));
            if (!new_fmt) {
                goto fail;
            }
            m->info.fmt = new_fmt;
            value       = str_split_values(value, ' ', "i", &m->info.fmt[m->info.fmt_count++]);
        }

        p = load_next_entry(p, &key, &value);
        if (key == 'i') {
            m->title = value;
            p        = load_next_entry(p, &key, &value);
        }

        if (key == 'c') {
            str_split_values(value, ' ', "sss", &m->conn.nettype, &m->conn.addrtype, &m->conn.address);
            p = load_next_entry(p, &key, &value);
        }

        while (key == 'b') {
            sdp_bandwidth *new_bw = (sdp_bandwidth *)realloc(m->bw, sizeof(sdp_bandwidth) * (m->bw_count + 1));
            if (!new_bw) {
                goto fail;
            }
            m->bw = new_bw;
            memset(&m->bw[m->bw_count], 0, sizeof(sdp_bandwidth));
            int n = m->bw_count++;
            str_split_values(value, ':', "ss", &m->bw[n].bwtype, &m->bw[n].bandwidth);
            p = load_next_entry(p, &key, &value);
        }

        if (key == 'k') {
            m->encrypt_key = value;
            p              = load_next_entry(p, &key, &value);
        }

        while (key == 'a') {
            ALLOCATE_MEM(m->attributes);
            m->attributes[m->attributes_count++] = value;
            p                                    = load_next_entry(p, &key, &value);
        }
    }

    return sdp;

fail:
    sdp_destroy(sdp);
    return nullptr;
}

std::string sdp_format(const struct SDPPayload *sdp)
{
    if (sdp == nullptr) {
        return std::string();
    }

    std::string sdp_data;
    sdp_data += str_format("v=%d\r\n", sdp->proto_version);
    sdp_data += str_format("o=%s %ld %ld %s %s %s\r\n",
                           sdp->origin.username,
                           sdp->origin.sess_id,
                           sdp->origin.sess_version,
                           sdp->origin.nettype,
                           sdp->origin.addrtype,
                           sdp->origin.addr);
    sdp_data += str_format("s=%s\r\n", sdp->session_name);
    if (sdp->session_info) {
        sdp_data += str_format("i=%s\r\n", sdp->session_info);
    }
    if (sdp->uri) {
        sdp_data += str_format("u=%s\r\n", sdp->uri);
    }
    for (int i = 0; i < sdp->emails_count; i++) {
        sdp_data += str_format("e=%s\r\n", sdp->emails[i]);
    }
    for (int i = 0; i < sdp->phones_count; i++) {
        sdp_data += str_format("p=%s\r\n", sdp->phones[i]);
    }
    if (sdp->conn.nettype || sdp->conn.addrtype || sdp->conn.address) {
        sdp_data += str_format("c=%s %s %s\r\n", sdp->conn.nettype, sdp->conn.addrtype, sdp->conn.address);
    }
    for (int i = 0; i < sdp->bw_count; i++) {
        sdp_data += str_format("b=%s:%s\r\n", sdp->bw[i].bwtype, sdp->bw[i].bandwidth);
    }
    for (int i = 0; i < sdp->times_count; i++) {
        sdp_data += str_format("t=%ld %ld\r\n", sdp->times[i].starttime, sdp->times[i].stoptime);
        for (int j = 0; j < sdp->times[i].repeat_count; j++) {
            sdp_data += str_format("r=%ld %ld", sdp->times[i].repeat[j].interval, sdp->times[i].repeat[j].duration);
            for (int k = 0; k < sdp->times[i].repeat[j].offsets_count; k++) {
                sdp_data += str_format(" %d", sdp->times[i].repeat[j].offsets[k]);
            }
            sdp_data += "\r\n";
        }
    }
    if (sdp->timezone_adj) {
        sdp_data += "z=";
        for (int i = 0; i < sdp->timezone_adj_count; i++) {
            sdp_data += str_format("%ld %ld ", sdp->timezone_adj[i].adjust, sdp->timezone_adj[i].offset);
        }
        sdp_data += "\r\n";
    }
    if (sdp->encrypt_key) {
        sdp_data += str_format("k=%s\r\n", sdp->encrypt_key);
    }
    for (int i = 0; i < sdp->attributes_count; i++) {
        sdp_data += str_format("a=%s\r\n", sdp->attributes[i]);
    }
    for (int i = 0; i < sdp->medias_count; i++) {
        if (sdp->medias[i].info.port_n) {
            sdp_data += str_format(
                "m=%s %d/%d %s", sdp->medias[i].info.type, sdp->medias[i].info.port, sdp->medias[i].info.port_n, sdp->medias[i].info.proto);
        } else {
            sdp_data += str_format("m=%s %d %s", sdp->medias[i].info.type, sdp->medias[i].info.port, sdp->medias[i].info.proto);
        }
        for (int j = 0; j < sdp->medias[i].info.fmt_count; j++) {
            sdp_data += str_format(" %d", sdp->medias[i].info.fmt[j]);
        }
        sdp_data += "\r\n";
        if (sdp->medias[i].title) {
            sdp_data += str_format("i=%s\r\n", sdp->medias[i].title);
        }
        if (sdp->medias[i].conn.nettype || sdp->medias[i].conn.addrtype || sdp->medias[i].conn.address) {
            sdp_data +=
                str_format("c=%s %s %s\r\n", sdp->medias[i].conn.nettype, sdp->medias[i].conn.addrtype, sdp->medias[i].conn.address);
        }
        for (int j = 0; j < sdp->medias[i].bw_count; j++) {
            sdp_data += str_format("b=%s:%s\r\n", sdp->medias[i].bw[j].bwtype, sdp->medias[i].bw[j].bandwidth);
        }
        if (sdp->medias[i].encrypt_key) {
            sdp_data += str_format("k=%s\r\n", sdp->medias[i].encrypt_key);
        }
        for (int j = 0; j < sdp->medias[i].attributes_count; j++) {
            sdp_data += str_format("a=%s\r\n", sdp->medias[i].attributes[j]);
        }
    }
    return sdp_data;
}

void sdp_destroy(struct SDPPayload *&sdp)
{
    if (sdp) {
        free(sdp->_payload);
        free(sdp->emails);
        free(sdp->phones);
        free(sdp->bw);
        for (int i = 0; i < sdp->times_count; i++) {
            for (int j = 0; j < sdp->times[i].repeat_count; j++) {
                free(sdp->times[i].repeat[j].offsets);
            }
            free(sdp->times[i].repeat);
        }
        free(sdp->times);
        free(sdp->timezone_adj);
        free(sdp->attributes);
        for (int i = 0; i < sdp->medias_count; i++) {
            free(sdp->medias[i].info.fmt);
            free(sdp->medias[i].bw);
            free(sdp->medias[i].attributes);
        }
        free(sdp->medias);
    }
    free(sdp);
    sdp = nullptr;
}

void sdp_print(const struct SDPPayload *sdp)
{
    printf("**********************************************\n");
    printf("proto version : %d\n", sdp->proto_version);
    printf("origin\n");
    printf("  username    : %s\n", sdp->origin.username);
    printf("  sess_id     : %lld\n", sdp->origin.sess_id);
    printf("  sess_version: %lld\n", sdp->origin.sess_version);
    printf("  nettype     : %s\n", sdp->origin.nettype);
    printf("  addrtype    : %s\n", sdp->origin.addrtype);
    printf("  addr        : %s\n", sdp->origin.addr);
    printf("session name  : %s\n", sdp->session_name);
    printf("session info  : %s\n", sdp->session_info);
    printf("uri           : %s\n", sdp->uri);

    for (int i = 0; i < sdp->emails_count; i++) {
        printf("emails[%d]: %s\n", i, sdp->emails[i]);
    }
    for (int i = 0; i < sdp->phones_count; i++) {
        printf("phones[%d]: %s\n", i, sdp->phones[i]);
    }

    printf("conn.nettype  : %s\n", sdp->conn.nettype);
    printf("conn.addrtype : %s\n", sdp->conn.addrtype);
    printf("conn.address  : %s\n", sdp->conn.address);

    for (int i = 0; i < sdp->bw_count; i++) {
        printf("bw[%d].bandwidth: %s\n", i, sdp->bw[i].bandwidth);
        printf("bw[%d].bwtype   : %s\n", i, sdp->bw[i].bwtype);
    }

    for (int i = 0; i < sdp->times_count; i++) {
        printf("times[%d].starttime: %ld\n", i, sdp->times[i].starttime);
        printf("times[%d].stoptime : %ld\n", i, sdp->times[i].stoptime);
        for (int j = 0; j < sdp->times[i].repeat_count; j++) {
            printf("times[%d].repeat[%d].duration: %ld\n", i, j, sdp->times[i].repeat[j].duration);
            printf("times[%d].repeat[%d].interval: %ld\n", i, j, sdp->times[i].repeat[j].interval);
            for (int k = 0; k < sdp->times[i].repeat[j].offsets_count; k++) {
                printf("times[%d].repeat[%d].offsets[%d]: %ld\n", i, j, k, sdp->times[i].repeat[j].offsets[k]);
            }
        }
    }

    for (int i = 0; i < sdp->timezone_adj_count; i++) {
        printf("timezone_adj[%d].adjust : %ld\n", i, sdp->timezone_adj[i].adjust);
        printf("timezone_adj[%d].offset : %ld\n", i, sdp->timezone_adj[i].offset);
    }

    printf("encrypt_key : %s\n", sdp->encrypt_key);
    for (int i = 0; i < sdp->attributes_count; i++) {
        printf("attributes[%d] : %s\n", i, sdp->attributes[i]);
    }
    for (int i = 0; i < sdp->medias_count; i++) {
        printf("***************** media[%d] *******************\n", i);
        printf("title      : %s\n", sdp->medias[i].title);
        printf("info.type  : %s\n", sdp->medias[i].info.type);
        printf("info.port  : %d\n", sdp->medias[i].info.port);
        printf("info.port_n: %d\n", sdp->medias[i].info.port_n);
        printf("info.proto : %s\n", sdp->medias[i].info.proto);
        for (int j = 0; j < sdp->medias[i].info.fmt_count; j++) {
            printf("info.fmt[%d]: %d\n", j, sdp->medias[i].info.fmt[j]);
        }

        for (int j = 0; j < sdp->medias[i].bw_count; j++) {
            printf("bw[%d].bandwidth: %s\n", j, sdp->medias[i].bw[j].bandwidth);
            printf("bw[%d].bwtype   : %s\n", j, sdp->medias[i].bw[j].bwtype);
        }
    }
    printf("**********************************************\n");
}