/*
 * Copyright (c) 2016-2017, Sigma Designs Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
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
 * Brief:   This file decribes tee glue codes for sigma designs defined sip
 *          services, which is headed for ATF.
 * Author:  Tony He <tony_he@sigmadesigns.com>
 * Date:    2017/05/14
 *
 */

#include <assert.h>
#include <trace.h>
#include <types_ext.h>
#include <string.h>
#include <kernel/misc.h>	/*reg_pair_from_64*/
#include <mm/core_memprot.h>	/*virt_to_phys*/
#include <platform_config.h>
#include <keep.h> 	/*KEEP_PAGER*/
#include <initcall.h>
#include <smccc.h>
#include <firmware.h>
#include "sd_sip_svc.h"

#ifndef NDEBUG
# define NOISE_ON_FAIL(sret) do {				\
	if ((sret) != SD_SIP_E_SUCCESS) {			\
		DMSG("%s failed ret %ld", __func__, sret);	\
	}							\
}while(0)
#else /* !NDEBUG */
# define NOISE_ON_FAIL(sret) do{}while(0)
#endif /* NDEBUG */

#define call_invoke_fn(...) do {				\
	if (sip_drv.invoke_fn)					\
		sip_drv.invoke_fn(__VA_ARGS__);			\
}while(0)

typedef void (*sip_invoke_fn)(unsigned long, unsigned long, unsigned long,
			unsigned long, unsigned long, unsigned long,
			unsigned long, unsigned long,
			struct smccc_res *);

/**
 * struct fw_sip_svc
 */
struct fw_sip_svc {
	sip_invoke_fn invoke_fn;
};

static struct fw_sip_svc sip_drv;

static TEE_Result sip_to_optee_ret(const int sret)
{
	if (sret == SD_SIP_E_SUCCESS)
		return TEE_SUCCESS;
	else if (sret == SD_SIP_E_INVALID_PARAM)
		return TEE_ERROR_BAD_PARAMETERS;
	else if (sret == SD_SIP_E_NOT_SUPPORTED)
		return TEE_ERROR_NOT_SUPPORTED;
	else if (sret == SD_SIP_E_INVALID_RANGE)
		return TEE_ERROR_SECURITY;
	else if (sret == SD_SIP_E_PERMISSION_DENY)
		return TEE_ERROR_ACCESS_DENIED;
	else if (sret == SD_SIP_E_LOCK_FAIL)
		return TEE_ERROR_ACCESS_CONFLICT;
	else if (sret == SD_SIP_E_SMALL_BUFFER)
		return TEE_ERROR_SHORT_BUFFER;
	else
		return TEE_ERROR_GENERIC;
}

static TEE_Result sip_fuse_read(const size_t ofs, void *va, uint32_t *size, uint32_t *pprot)
{
	struct smccc_res res = {SD_SIP_E_NOT_SUPPORTED};
	uint32_t high, low;
	if (size == NULL)
		return TEE_ERROR_BAD_PARAMETERS;

	reg_pair_from_64(((va != NULL) ? virt_to_phys(va) : 0), &high, &low);
	call_invoke_fn(SD_SIP_FUNC_C_OTP_READ, ofs, high, low, *size, 0, 0, 0, &res);
	NOISE_ON_FAIL(res.a0);
	if (SD_SIP_E_SUCCESS == res.a0) {
		if (pprot != NULL)
			*pprot = res.a1;
		*size = res.a2;
	}

	return sip_to_optee_ret(res.a0);
}

static TEE_Result sip_fuse_write(const size_t ofs, void *va, uint32_t *size, const uint32_t prot)
{
	struct smccc_res res = {SD_SIP_E_NOT_SUPPORTED};
	uint32_t high, low;
	if (size == NULL)
		return TEE_ERROR_BAD_PARAMETERS;

	reg_pair_from_64(((va != NULL) ? virt_to_phys(va) : 0), &high, &low);
	call_invoke_fn(SD_SIP_FUNC_S_OTP_WRITE, ofs, high, low, *size, prot, 0, 0, &res);
	NOISE_ON_FAIL(res.a0);
	if (SD_SIP_E_SUCCESS == res.a0) {
		*size = res.a1;
	}

	return sip_to_optee_ret(res.a0);
}

static TEE_Result sip_get_mem_state(const paddr_t pa, const size_t len, uint32_t *pstate)
{
	struct smccc_res res = {SD_SIP_E_NOT_SUPPORTED};
	uint32_t high, low;
	reg_pair_from_64(pa, &high, &low);
	call_invoke_fn(SD_SIP_FUNC_C_MEM_STATE, high, low, len, 0, 0, 0, 0, &res);
	NOISE_ON_FAIL(res.a0);
	if (SD_SIP_E_SUCCESS == res.a0) {
		if (pstate != NULL)
			*pstate = res.a1;
	}
	return sip_to_optee_ret(res.a0);
}

static FW_OPS( "sd sip",
		sip_fuse_read,
		sip_fuse_write,
		sip_get_mem_state)

static bool sip_api_uid_is_sd_api(sip_invoke_fn fn)
{
	struct smccc_res res;
	fn(SIP_SVC_UID, 0, 0, 0, 0, 0, 0, 0, &res);

	FMSG("SIP UID %lx %lx %lx %lx", res.a0, res.a1, res.a2, res.a3);
	if (res.a0 == SD_SIP_UID_0 && res.a1 == SD_SIP_UID_1 &&
	    res.a2 == SD_SIP_UID_2 && res.a3 == SD_SIP_UID_3)
		return true;
	return false;
}

static bool sip_api_revision_is_compatible(sip_invoke_fn fn)
{
	struct smccc_res res;
	fn(SIP_SVC_VERSION, 0, 0, 0, 0, 0, 0, 0, &res);

	if (res.a0 == SD_SIP_SVC_VERSION_MAJOR &&
	    res.a1 >= SD_SIP_SVC_VERSION_MINOR)
		return true;
	return false;
}

static TEE_Result sip_svc_init(void)
{
	struct fw_sip_svc *sip = &sip_drv;
	sip_invoke_fn fn = smccc_smc;

	memset(sip, 0, sizeof(*sip));
	if (!sip_api_uid_is_sd_api(fn)) {
		EMSG("api uid mismatch");
		return TEE_ERROR_BAD_STATE;
	}

	if (!sip_api_revision_is_compatible(fn)) {
		EMSG("api revision mismatch");
		return TEE_ERROR_BAD_STATE;
	}

	sip->invoke_fn = fn;
	/* register the ops*/
	return fw_register_drv(&fw_ops);
}

service_init(sip_svc_init);
