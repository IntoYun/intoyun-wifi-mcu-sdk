/*
 * MD5 internal definitions
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef INTOYUN_MD5_H_
#define INTOYUN_MD5_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


struct MD5Context {
    uint32_t buf[4];
    uint32_t bits[2];
    uint8_t  in[64];
};

void md5_begin(void);
void md5_add(uint8_t * data, uint16_t len);
void md5_calculate(void);
void md5_output(uint8_t *data, uint16_t len, char *signature);
int md5_vector(size_t num_elem, const uint8_t *addr[], const size_t *len, uint8_t *mac);
void MD5Init(struct MD5Context *context);
void MD5Update(struct MD5Context *context, uint8_t const *buf, uint32_t len);
void MD5Final(uint8_t digest[16], struct MD5Context *context);


#ifdef __cplusplus
}
#endif


#endif
