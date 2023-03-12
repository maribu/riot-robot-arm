/*
 * Copyright (c) 2018 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "net/coap.h"
#include "net/gnrc/netif.h"
#include "net/ipv6.h"
#include "net/nanocoap.h"
#include "net/nanocoap_sock.h"
#include "net/sock/udp.h"
#include "net/sock/util.h"
#include "od.h"
#include "shell.h"

static int _blockwise_cb(void *arg, size_t offset, uint8_t *buf, size_t len, int more)
{
    (void)arg;
    (void)more;

    printf("offset %03u: ", (unsigned)offset);
    for (unsigned i = 0; i < len; ++i) {
        putchar(buf[i]);
    }
    puts("");

    return 0;
}

static int coap_client_cmd(int argc, char **argv)
{
    /* Ordered like the RFC method code numbers, but off by 1. GET is code 0. */
    const char *method_codes[] = { "get", "post", "put", "delete" };
    int res;

    if (argc < 3) {
        goto error;
    }

    int code_pos = -1;
    for (size_t i = 0; i < ARRAY_SIZE(method_codes); i++) {
        if (strcmp(argv[1], method_codes[i]) == 0) {
            code_pos = i;
            break;
        }
    }
    if (code_pos == -1) {
        goto error;
    }

    switch (code_pos) {
    case COAP_METHOD_GET - 1:
        res = nanocoap_get_blockwise_url(argv[2], COAP_BLOCKSIZE_32,
                                         _blockwise_cb, NULL);
        break;
    case COAP_METHOD_POST - 1:
    case COAP_METHOD_PUT - 1:
        ;
        char response[32];
        nanocoap_sock_t sock;
        res = nanocoap_sock_url_connect(argv[2], &sock);
        if (res) {
            break;
        }
        if (code_pos == 1) {
            res = nanocoap_sock_post(&sock, sock_urlpath(argv[2]),
                                     argv[3], strlen(argv[3]),
                                     response, sizeof(response));
        } else {
            res = nanocoap_sock_put(&sock, sock_urlpath(argv[2]),
                                    argv[3], strlen(argv[3]),
                                    response, sizeof(response));
        }
        nanocoap_sock_close(&sock);
        if (res > 0) {
            response[res] = 0;
            printf("response: %s\n", response);
        }
        break;
    case COAP_METHOD_DELETE - 1:
        res = nanocoap_sock_delete_url(argv[2]);
        break;
    default:
        printf("TODO: implement %s request\n", method_codes[code_pos]);
        return -1;
    }

    if (res) {
        printf("res: %d\n", res);
    }
    return res;

error:
    printf("usage: %s <get|post|put|delete> <url> [data]\n", argv[0]);
    return -1;
}

SHELL_COMMAND(coap, "CoAP client", coap_client_cmd);
