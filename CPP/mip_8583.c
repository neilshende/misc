/* mip_iso8583_client.c
 *
 * Demo client that connects to a remote TCP endpoint (e.g. Mastercard MIP)
 * and receives ISO 8583 messages in a loop, parsing them with the included
 * demo parser.
 *
 * WARNING: This demo uses plain TCP. Use TLS in real deployments.
 *
 * Build:
 *   gcc -o mip_iso8583_client mip_iso8583_client.c
 *
 * Run:
 *   ./mip_iso8583_client <host> <port>
 *
 * Example:
 *   ./mip_iso8583_client 127.0.0.1 5000
 *
 * By default assumes a 2-byte big-endian length prefix before each ISO8583 message.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/select.h>

/* Configuration */
#define MAX_MSG_LEN 8192
#define LEN_PREFIX_BYTES 2   /* number of bytes in length prefix (1,2,4 are common) */
#define LEN_PREFIX_BIG_ENDIAN 1 /* 1 = network (big endian), 0 = little-endian */

/* --- Utility logging --- */
static void logf(const char *fmt, ...) {
    va_list ap;
    char tbuf[64];
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    strftime(tbuf, sizeof(tbuf), "%F %T", &tm);

    printf("%s ", tbuf);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
}

/* --- Socket helpers --- */
static int connect_with_timeout(const char *host, const char *port, int timeout_seconds) {
    struct addrinfo hints, *res = NULL, *rp;
    int s = -1;
    int rc;

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    if ((rc = getaddrinfo(host, port, &hints, &res)) != 0) {
        logf("getaddrinfo(%s:%s) failed: %s", host, port, gai_strerror(rc));
        return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s < 0) continue;

        /* set non-blocking for connect timeout */
        int flags = fcntl(s, F_GETFL, 0);
        if (flags >= 0) fcntl(s, F_SETFL, flags | O_NONBLOCK);

        if (connect(s, rp->ai_addr, rp->ai_addrlen) < 0) {
            if (errno == EINPROGRESS) {
                fd_set wf;
                FD_ZERO(&wf);
                FD_SET(s, &wf);
                struct timeval tv;
                tv.tv_sec = timeout_seconds;
                tv.tv_usec = 0;
                int sel = select(s + 1, NULL, &wf, NULL, &tv);
                if (sel > 0) {
                    /* check for socket error */
                    int err = 0;
                    socklen_t len = sizeof(err);
                    if (getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
                        close(s);
                        s = -1;
                        continue;
                    }
                    /* connected */
                } else {
                    /* timeout or select error */
                    close(s);
                    s = -1;
                    continue;
                }
            } else {
                close(s);
                s = -1;
                continue;
            }
        }

        /* restore blocking mode */
        if (flags >= 0) fcntl(s, F_SETFL, flags);

        /* success */
        break;
    }

    freeaddrinfo(res);
    if (s < 0) {
        logf("Could not connect to %s:%s", host, port);
        return -1;
    }

    logf("Connected to %s:%s (fd=%d)", host, port, s);
    return s;
}

/* Read exactly n bytes from fd into buf (blocking unless socket non-blocking). */
static ssize_t read_n_bytes(int fd, void *buf, size_t n, int timeout_seconds) {
    size_t got = 0;
    char *p = buf;
    while (got < n) {
        fd_set rf;
        FD_ZERO(&rf);
        FD_SET(fd, &rf);
        struct timeval tv;
        tv.tv_sec = timeout_seconds;
        tv.tv_usec = 0;
        int sel = select(fd + 1, &rf, NULL, NULL, &tv);
        if (sel == 0) {
            /* timeout */
            return -2; /* special code for timeout */
        } else if (sel < 0) {
            return -1;
        }
        ssize_t r = recv(fd, p + got, n - got, 0);
        if (r <= 0) return r;
        got += r;
    }
    return (ssize_t)got;
}

/* --- ISO8583 demo parser functions (adapted from prior example) --- */

/* Convert hex char to value */
static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    return -1;
}

/* Print a readable hex dump of bytes */
static void hexdump(const unsigned char *b, int len) {
    for (int i = 0; i < len; ++i) {
        printf("%02X", b[i]);
    }
    printf("\n");
}

/* Test bit in 8-byte bitmap (bit 1..64). */
static int bitmap_test(const unsigned char *bitmap, int bit) {
    if (!bitmap) return 0;
    if (bit < 1 || bit > 64) return 0;
    int idx = (bit - 1) / 8;
    int offset = 7 - ((bit - 1) % 8); /* MSB first */
    return (bitmap[idx] >> offset) & 1;
}

/* Read ASCII digits length indicator of width 'digits' and return numeric value; advances pos */
static int read_len_digits_buf(const unsigned char *msg, int msglen, int *pos, int digits) {
    if (*pos + digits > msglen) return -1;
    int value = 0;
    for (int i = 0; i < digits; ++i) {
        unsigned char c = msg[*pos + i];
        if (!isdigit(c)) return -1;
        value = value * 10 + (c - '0');
    }
    *pos += digits;
    return value;
}

/* The demo parser prints a small subset of fields. */
static void parse_and_print_iso8583(const unsigned char *msg, int msglen) {
    if (msglen < 4 + 8) {
        logf("Message too short for MTI+bitmap (len=%d)", msglen);
        return;
    }

    int pos = 0;
    char mti[5] = {0};
    memcpy(mti, msg + pos, 4); pos += 4;
    printf("=== ISO8583 message (len=%d) ===\n", msglen);
    printf("MTI: %s\n", mti);

    unsigned char bitmap[8];
    memcpy(bitmap, msg + pos, 8); pos += 8;
    printf("Primary bitmap: "); hexdump(bitmap, 8);
    int has_secondary = bitmap_test(bitmap, 1);
    if (has_secondary) {
        printf("Secondary bitmap indicated (not fully handled by demo)\n");
        if (pos + 8 <= msglen) {
            unsigned char sec[8];
            memcpy(sec, msg + pos, 8);
            pos += 8;
            printf("Secondary bitmap: "); hexdump(sec, 8);
        }
    }

    for (int field = 2; field <= 64; ++field) {
        if (!bitmap_test(bitmap, field)) continue;
        printf("DE%02d: ", field);
        if (field == 2) {
            int len = read_len_digits_buf(msg, msglen, &pos, 2);
            if (len < 0 || pos + len > msglen) { printf("<error reading DE2>\n"); return; }
            char *buf = malloc(len+1);
            memcpy(buf, msg + pos, len); buf[len] = 0; pos += len;
            printf("PAN (len=%d): %s\n", len, buf);
            free(buf);
        } else if (field == 3) {
            if (pos + 6 > msglen) { printf("<truncated>\n"); return; }
            char tmp[7]; memcpy(tmp, msg + pos, 6); tmp[6]=0; pos += 6;
            printf("Processing Code: %s\n", tmp);
        } else if (field == 4) {
            if (pos + 12 > msglen) { printf("<truncated>\n"); return; }
            char tmp[13]; memcpy(tmp, msg + pos, 12); tmp[12]=0; pos += 12;
            printf("Amount: %s\n", tmp);
        } else if (field == 7) {
            if (pos + 10 > msglen) { printf("<truncated>\n"); return; }
            char tmp[11]; memcpy(tmp, msg + pos, 10); tmp[10]=0; pos += 10;
            printf("Transmission DT: %s\n", tmp);
        } else if (field == 11) {
            if (pos + 6 > msglen) { printf("<truncated>\n"); return; }
            char tmp[7]; memcpy(tmp, msg + pos, 6); tmp[6]=0; pos += 6;
            printf("STAN: %s\n", tmp);
        } else if (field == 12) {
            if (pos + 6 > msglen) { printf("<truncated>\n"); return; }
            char tmp[7]; memcpy(tmp, msg + pos, 6); tmp[6]=0; pos += 6;
            printf("Local time: %s\n", tmp);
        } else if (field == 13) {
            if (pos + 4 > msglen) { printf("<truncated>\n"); return; }
            char tmp[5]; memcpy(tmp, msg + pos, 4); tmp[4]=0; pos += 4;
            printf("Local date: %s\n", tmp);
        } else if (field == 35) {
            int len = read_len_digits_buf(msg, msglen, &pos, 2);
            if (len < 0 || pos + len > msglen) { printf("<error>\n"); return; }
            char *buf = malloc(len+1);
            memcpy(buf, msg + pos, len); buf[len]=0; pos += len;
            printf("Track2 (len=%d): %s\n", len, buf);
            free(buf);
        } else if (field == 37) {
            if (pos + 12 > msglen) { printf("<truncated>\n"); return; }
            char tmp[13]; memcpy(tmp, msg + pos, 12); tmp[12]=0; pos += 12;
            printf("RRN: %s\n", tmp);
        } else if (field == 41) {
            if (pos + 8 > msglen) { printf("<truncated>\n"); return; }
            char tmp[9]; memcpy(tmp, msg + pos, 8); tmp[8]=0; pos += 8;
            printf("Terminal ID: %s\n", tmp);
        } else if (field == 42) {
            if (pos + 15 > msglen) { printf("<truncated>\n"); return; }
            char tmp[16]; memcpy(tmp, msg + pos, 15); tmp[15]=0; pos += 15;
            printf("Merchant ID: %s\n", tmp);
        } else if (field == 49) {
            if (pos + 3 > msglen) { printf("<truncated>\n"); return; }
            char tmp[4]; memcpy(tmp, msg + pos, 3); tmp[3]=0; pos += 3;
            printf("Currency: %s\n", tmp);
        } else {
            /* best-effort: if next two bytes are ASCII digits, assume LLVAR */
            if (pos + 2 <= msglen && isdigit(msg[pos]) && isdigit(msg[pos+1])) {
                int len = read_len_digits_buf(msg, msglen, &pos, 2);
                if (len < 0 || pos + len > msglen) { printf("<error reading unknown LLVAR>\n"); return; }
                char *buf = malloc(len+1);
                memcpy(buf, msg + pos, len); buf[len]=0; pos += len;
                printf("Unknown LLVAR(%d): %s\n", len, buf);
                free(buf);
            } else {
                /* print up to 6 printable bytes */
                int start = pos;
                int consumed = 0;
                while (pos < msglen && consumed < 6 && isprint(msg[pos])) { pos++; consumed++; }
                if (consumed > 0) {
                    char *tmp = malloc(consumed+1);
                    memcpy(tmp, msg + start, consumed); tmp[consumed]=0;
                    printf("Unknown (best-effort %d b): %s\n", consumed, tmp);
                    free(tmp);
                } else {
                    printf("<unknown non-printable or unsupported field>\n");
                }
            }
        }
    }

    if (pos < msglen) {
        printf("Remaining %d bytes after parse: ", msglen - pos);
        hexdump(msg + pos, msglen - pos);
    } else {
        printf("No remaining bytes after parse.\n");
    }

    printf("=== End message ===\n\n");
}

/* --- Main receive loop --- */
int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        return 2;
    }

    const char *host = argv[1];
    const char *port = argv[2];

    int sock = -1;
    int reconnect_delay = 2; /* seconds */

    unsigned char buf[MAX_MSG_LEN];

    while (1) {
        if (sock < 0) {
            sock = connect_with_timeout(host, port, 10);
            if (sock < 0) {
                logf("Reconnect in %d seconds...", reconnect_delay);
                sleep(reconnect_delay);
                /* simple backoff */
                if (reconnect_delay < 60) reconnect_delay *= 2;
                continue;
            }
            reconnect_delay = 2;
        }

        /* Read length prefix */
        unsigned char lenbuf[8];
        ssize_t r = read_n_bytes(sock, lenbuf, LEN_PREFIX_BYTES, 30);
        if (r == -2) {
            logf("Read length prefix timed out (closing socket)");
            close(sock); sock = -1;
            continue;
        } else if (r <= 0) {
            logf("Connection closed or error while reading length prefix (r=%zd). Reconnecting...", r);
            close(sock); sock = -1;
            continue;
        }

        uint32_t msglen = 0;
        if (LEN_PREFIX_BYTES == 1) {
            msglen = lenbuf[0];
        } else if (LEN_PREFIX_BYTES == 2) {
            if (LEN_PREFIX_BIG_ENDIAN) {
                msglen = ((uint32_t)lenbuf[0] << 8) | (uint32_t)lenbuf[1];
            } else {
                msglen = ((uint32_t)lenbuf[1] << 8) | (uint32_t)lenbuf[0];
            }
        } else if (LEN_PREFIX_BYTES == 4) {
            if (LEN_PREFIX_BIG_ENDIAN) {
                msglen = ((uint32_t)lenbuf[0] << 24) | ((uint32_t)lenbuf[1] << 16) |
                         ((uint32_t)lenbuf[2] << 8)  | (uint32_t)lenbuf[3];
            } else {
                msglen = ((uint32_t)lenbuf[3] << 24) | ((uint32_t)lenbuf[2] << 16) |
                         ((uint32_t)lenbuf[1] << 8)  | (uint32_t)lenbuf[0];
            }
        } else {
            logf("Unsupported LEN_PREFIX_BYTES=%d", LEN_PREFIX_BYTES);
            close(sock); sock = -1;
            return 3;
        }

        if (msglen == 0) {
            logf("Received zero-length message; ignoring.");
            continue;
        }
        if (msglen > MAX_MSG_LEN) {
            logf("Received message length %u exceeds MAX_MSG_LEN (%d). Closing connection.", msglen, MAX_MSG_LEN);
            close(sock); sock = -1;
            continue;
        }

        /* Read full message */
        ssize_t got = read_n_bytes(sock, buf, msglen, 30);
        if (got == -2) {
            logf("Read message timed out (closing socket)");
            close(sock); sock = -1;
            continue;
        } else if (got <= 0 || (size_t)got != msglen) {
            logf("Connection closed or partial read (got=%zd expected=%u). Reconnecting...", got, msglen);
            close(sock); sock = -1;
            continue;
        }

        /* Now we have a message of msglen bytes in buf */
        /* Many protocols send MTI + bitmap as ASCII or as binary. This demo assumes ASCII MTI and ASCII/printable fields. */
        parse_and_print_iso8583(buf, msglen);

        /* Loop continues to read next message */
    }

    /* never reached */
    return 0;
}

