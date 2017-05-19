/*
 * Copyright (c) 2017, Sigma Designs
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Author:  Tony He <tony_he@sigmadesigns.com>
 * Date:    2017/05/17
 *
 */

#include <compiler.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <types_ext.h>
#include <kernel/tee_ta_manager.h>
#include <tee_api_types.h>
#include <tee_api_defines.h>
#include <otp.h>
#include "svcx_internal.h"

/*
 * TA ULI UUID:
 * 6e9f5255-d981-4aad-8243-42edf4fef4f4
 */
/* This UUID is generated with uuidgen
   the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html */
#define TA_ULI_UUID { 0x6e9f5255, 0xd981, 0x4aad, \
		{ 0x82, 0x43, 0x42, 0xed, 0xf4, 0xfe, 0xf4, 0xf4} }

#ifndef NDEBUG
#define DUMP_BUF_MAX	256
static char *print_buf(char *buf, size_t *remain_size, const char *fmt, ...)
	__attribute__((__format__(__printf__, 3, 4)));

static char *print_buf(char *buf, size_t *remain_size, const char *fmt, ...)
{
	va_list ap;
	size_t len;

	va_start(ap, fmt);
	len = vsnprintf(buf, *remain_size, fmt, ap);
	buf += len;
	*remain_size -= len;
	va_end(ap);
	return buf;
}

static void dump_hex(char *buf, size_t *remain_size, uint8_t *input_buf,
		size_t input_size)
{
	size_t i;

	for (i = 0; i < input_size; i++)
		buf = print_buf(buf, remain_size, "%02X ", input_buf[i]);
}

static void print_hex(uint8_t *input_buf, size_t input_size)
{
	char buf[DUMP_BUF_MAX];
	size_t remain = sizeof(buf);

	dump_hex(buf, &remain, input_buf, input_size);
	DMSG("%s", buf);
}
#endif /* !NDEBUG */

#define FUSE_OFS_INVAL	0xFFFFFFFF
static uint32_t otp_id2ofs(uint32_t id)
{
	switch (id){
	case TEE_OTP_KEY_0:
		return FUSE_OFS_OTP_KEY_0;
	case TEE_OTP_KEY_1:
		return FUSE_OFS_OTP_KEY_1;
	case TEE_OTP_KEY_2:
		return FUSE_OFS_OTP_KEY_2;
	case TEE_OTP_KEY_3:
		return FUSE_OFS_OTP_KEY_3;
	case TEE_OTP_KEY_4:
		return FUSE_OFS_OTP_KEY_4;
	case TEE_OTP_KEY_5:
		return FUSE_OFS_OTP_KEY_5;
	case TEE_OTP_KEY_RSA:
		return FUSE_OFS_RSA_PUB_KEY;
	default:
		return FUSE_OFS_INVAL;
	}
}

TEE_Result svcx_otp_write(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res;
	TEE_Identity clnt;
	TEE_UUID uuid = TA_ULI_UUID;
	uint32_t ofs, len, prot;
	uint32_t test __maybe_unused;

	ASSERT_PARAM_TYPE(TEE_PARAM_TYPES
			  (TEE_PARAM_TYPE_VALUE_INOUT,
			   TEE_PARAM_TYPE_MEMREF_INPUT,
			   TEE_PARAM_TYPE_VALUE_INPUT,
			   TEE_PARAM_TYPE_NONE));

	res = tee_ta_get_client_id(&clnt);
	if (res != TEE_SUCCESS)
		return res;

	/* check if called from TA */
	if (clnt.login != TEE_LOGIN_TRUSTED_APP)
		return TEE_ERROR_SECURITY;

	/* extract params */
	ofs = otp_id2ofs(params[0].value.a);
	prot = params[0].value.b;
	test = params[2].value.a;
	len = params[1].memref.size;

	/* check input params */
	if (FUSE_OFS_INVAL == ofs)
		return TEE_ERROR_NOT_SUPPORTED;

#ifndef NDEBUG
	/* check if test only */
	if (test) {
		DMSG("test fuse write: ofs %03x prot %08x", ofs, prot);
		print_hex(params[1].memref.buffer, params[1].memref.size);
		params[0].value.a = len;
		return TEE_SUCCESS;
	}
#endif /* !NDEBUG */

	/* check if called from TA ULI */
	if (memcmp(&clnt.uuid, &uuid, sizeof(TEE_UUID)) != 0)
		return TEE_ERROR_SECURITY;

	res = otp_fuse_write(ofs, params[1].memref.buffer,
			     &len, prot);

	if (TEE_SUCCESS == res) {
		params[0].value.a = len;
	}
	return res;
}
