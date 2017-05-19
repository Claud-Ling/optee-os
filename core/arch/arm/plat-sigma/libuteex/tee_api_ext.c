/*
 * Copyright (c) 2017, Sigma Designs Inc.
 * All rights reserved.
 *
 * Brief:  This file contains sigma designs defined tee internal api extensions
 * Author: Tony He
 * Date:   2017/5/16
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
 */

#include <tee_api.h>
#include <assert.h>
#include <string.h>
#include <tee_api_ext.h>

#define TEE_PARAMS_INITIALIZER { { {0} } }

static TEE_TASessionHandle ext_sess = TEE_HANDLE_NULL;

/* copied from <kernel/misc.h> */
static inline uint64_t reg_pair_to_64(uint32_t reg0, uint32_t reg1)
{
	return (uint64_t)reg0 << 32 | reg1;
}

/* copied from <kernel/misc.h> */
static inline void reg_pair_from_64(uint64_t val, uint32_t *reg0,
			uint32_t *reg1)
{
	*reg0 = val >> 32;
	*reg1 = val;
}

static TEE_Result open_svcx_session(TEE_TASessionHandle *sess, uint32_t *orig)
{
	TEE_Result res;
	TEE_UUID uuid = PTA_SVCX_UUID;

	res = TEE_OpenTASession(&uuid, 0, 0, NULL, sess, orig);
	if (res != TEE_SUCCESS) {
		EMSG("helper_svc_init: TEE_OpenTASession failed\n");
		return res;
	}

	return TEE_SUCCESS;

}

static void close_svcx_session(TEE_TASessionHandle *sess)
{
	if (*sess != TEE_HANDLE_NULL) {
		TEE_CloseTASession(*sess);
		*sess = TEE_HANDLE_NULL;
	}
}

/*
 * constructor
 */
TEE_Result TEE_Ext_Init(void)
{
	uint32_t ret_orig;

	if (TEE_HANDLE_NULL == ext_sess) {
		return open_svcx_session(&ext_sess, &ret_orig);
	}

	return TEE_SUCCESS;
}

/*
 * deconstructor
 */
void TEE_Ext_DeInit(void)
{
	close_svcx_session(&ext_sess);
}

/*****************************************************************************/
/*                 Sigma Designs TEE API Extensions                          */
/*****************************************************************************/

/*
 * user map support (TA requires the REMAP_SUPPORT property)
 * pa must be page size aligned.
 */
void *TEE_Mmap(const paddr_t pa, const size_t len, const uint32_t prot,
		const uint32_t attr, uint32_t *err)
{
	TEE_Result res;
	uintptr_t va;
	uint32_t ret_orig;
	uint32_t param_types;
	TEE_Param params[TEE_NUM_PARAMS] = TEE_PARAMS_INITIALIZER;
	TEE_TASessionHandle s = ext_sess;

	if (TEE_HANDLE_NULL == s) {
		res = open_svcx_session(&s, &ret_orig);
		if (res != TEE_SUCCESS)
			goto OUT;
	}

	param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
			TEE_PARAM_TYPE_VALUE_INPUT,
			TEE_PARAM_TYPE_VALUE_INPUT,
			TEE_PARAM_TYPE_NONE);

	reg_pair_from_64((uint64_t)pa, &params[0].value.a, &params[0].value.b);
	reg_pair_from_64((uint64_t)len, &params[1].value.a, &params[1].value.b);
	params[2].value.a = prot;
	params[2].value.b = attr;

	assert(s != TEE_HANDLE_NULL);
	res = TEE_InvokeTACommand(s, 0, PTA_SVCX_CMD_MMAP,
				  param_types, params, &ret_orig);

	if (s != ext_sess)
		close_svcx_session(&s);

OUT:
	if (err)
		*err = res;

	if (res == TEE_SUCCESS) {
		va = (uintptr_t)reg_pair_to_64(params[0].value.a, params[0].value.b);
		return (void*)va;
	} else {
		return NULL;
	}
}

TEE_Result TEE_Munmap(void *va, const size_t len)
{
	TEE_Result res;
	uint32_t ret_orig;
	uint32_t param_types;
	TEE_Param params[TEE_NUM_PARAMS] = TEE_PARAMS_INITIALIZER;
	TEE_TASessionHandle s = ext_sess;

	if (TEE_HANDLE_NULL == s) {
		res = open_svcx_session(&s, &ret_orig);
		if (res != TEE_SUCCESS)
			return res;
	}

	param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
			TEE_PARAM_TYPE_VALUE_INPUT,
			TEE_PARAM_TYPE_NONE,
			TEE_PARAM_TYPE_NONE);

	reg_pair_from_64((uintptr_t)va, &params[0].value.a, &params[0].value.b);
	reg_pair_from_64((uint64_t)len, &params[1].value.a, &params[1].value.b);

	assert(s != TEE_HANDLE_NULL);
	res = TEE_InvokeTACommand(s, 0, PTA_SVCX_CMD_MUNMAP,
				  param_types, params, &ret_orig);

	if (s != ext_sess)
		close_svcx_session(&s);

	return res;
}

TEE_Result TEE_OtpWrite(const uint32_t id, void *va, uint32_t *const size,
			const uint32_t prot)
{
	TEE_Result res;
	uint32_t ret_orig;
	uint32_t param_types;
	TEE_Param params[TEE_NUM_PARAMS] = TEE_PARAMS_INITIALIZER;
	TEE_TASessionHandle s = ext_sess;

	if (NULL == size)
		return TEE_ERROR_BAD_PARAMETERS;

	if (TEE_HANDLE_NULL == s) {
		res = open_svcx_session(&s, &ret_orig);
		if (res != TEE_SUCCESS)
			return res;;
	}

	param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
			TEE_PARAM_TYPE_MEMREF_INPUT,
			TEE_PARAM_TYPE_VALUE_INPUT,
			TEE_PARAM_TYPE_NONE);

	params[0].value.a = id;
	params[0].value.b = prot;
	params[1].memref.buffer = va;
	params[1].memref.size = *size;
	params[2].value.a = 0;	/* non-test */

	assert(s != TEE_HANDLE_NULL);
	res = TEE_InvokeTACommand(s, 0, PTA_SVCX_CMD_OTP_WRITE,
				  param_types, params, &ret_orig);
	if (TEE_SUCCESS == res)
		*size = params[0].value.a;

	if (s != ext_sess)
		close_svcx_session(&s);

	return res;
}

TEE_Result TEE_MemState(const paddr_t pa, const size_t len,
			uint32_t *pstate)
{
	TEE_Result res;
	uint32_t ret_orig;
	uint32_t param_types;
	TEE_Param params[TEE_NUM_PARAMS] = TEE_PARAMS_INITIALIZER;
	TEE_TASessionHandle s = ext_sess;

	if (NULL == pstate)
		return TEE_ERROR_BAD_PARAMETERS;

	if (TEE_HANDLE_NULL == s) {
		res = open_svcx_session(&s, &ret_orig);
		if (res != TEE_SUCCESS)
			return res;;
	}

	param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
			TEE_PARAM_TYPE_VALUE_INPUT,
			TEE_PARAM_TYPE_NONE,
			TEE_PARAM_TYPE_NONE);

	reg_pair_from_64((uint64_t)pa, &params[0].value.a, &params[0].value.b);
	reg_pair_from_64((uint64_t)len, &params[1].value.a, &params[1].value.b);

	assert(s != TEE_HANDLE_NULL);
	res = TEE_InvokeTACommand(s, 0, PTA_SVCX_CMD_MEM_STATE,
				  param_types, params, &ret_orig);

	if (TEE_SUCCESS == res)
		*pstate = params[0].value.a;

	if (s != ext_sess)
		close_svcx_session(&s);

	return res;
}
