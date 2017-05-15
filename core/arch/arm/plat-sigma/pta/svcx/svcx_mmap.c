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
 * Date:    2017/05/16
 *
 */

#include <types_ext.h>
#include <kernel/misc.h> /*reg_pair_to_64/reg_pair_from_64*/
#include <tee_api_types.h>
#include <tee_api_defines.h>
#include <mm/tee_mmu.h>
#include <mm/core_memprot.h>
#include "svcx_internal.h"

static uint32_t xflags_to_attr(const uint32_t prot, const uint32_t attr)
{
	uint32_t mattr = TEE_MATTR_VALID_BLOCK | TEE_MATTR_PRW;

	if (prot & TEE_MAP_PROT_READ)
		mattr |= TEE_MATTR_UR;
	if (prot & TEE_MAP_PROT_WRITE)
		mattr |= TEE_MATTR_UW;
	if (prot & TEE_MAP_PROT_EXEC)
		mattr |= (TEE_MATTR_PX | TEE_MATTR_UX);

	if (attr & TEE_MAP_ATTR_CACHED)
		mattr |= (TEE_MATTR_CACHE_CACHED << TEE_MATTR_CACHE_SHIFT);
	if (attr & TEE_MAP_ATTR_GLOBAL)
		mattr |= TEE_MATTR_GLOBAL;
	if (attr & TEE_MAP_ATTR_SECURE)
		mattr |= TEE_MATTR_SECURE;

	return mattr;
}

static TEE_Result tee_plat_check_umap(paddr_t pa, size_t len, const uint32_t prot, const uint32_t attr)
{
	(void)prot;
	(void)attr;
	/*
	 * Check if we can map the specified memory
	 * Only external RAM is allowed
	 */
	if (!core_pbuf_is(CORE_MEM_MULTPURPOSE, pa, len))
		return TEE_ERROR_SECURITY;

	return TEE_SUCCESS;
}

/*
 * add one user map for calling TA
 * map <len> bytes memory starting from physical addr <pa>
 * <prot> could be used to specify the desired protection of the mapping (XPROT)
 * <attr> could be used to specify the desired attributes of the mapping (XMAP)
 */
TEE_Result svcx_map(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res;
	struct tee_ta_session *s = NULL;
	TEE_Identity clnt;
	paddr_t pa;
	vaddr_t va;
	size_t len;
	uint32_t prot, attr;

	ASSERT_PARAM_TYPE(TEE_PARAM_TYPES
			  (TEE_PARAM_TYPE_VALUE_INOUT,
			   TEE_PARAM_TYPE_VALUE_INPUT,
			   TEE_PARAM_TYPE_VALUE_INPUT,
			   TEE_PARAM_TYPE_NONE));

	res = tee_ta_get_client_id(&clnt);
	if (res != TEE_SUCCESS)
		return res;

	/* check if called from TA */
	if (clnt.login != TEE_LOGIN_TRUSTED_APP)
		return TEE_ERROR_NOT_SUPPORTED;

	/* extract params */
	pa = (paddr_t)reg_pair_to_64(params[0].value.a, params[0].value.b);
	len = reg_pair_to_64(params[1].value.a, params[1].value.b);
	prot = params[2].value.a;
	attr = params[2].value.b;

	/* check input params */
	res = tee_plat_check_umap(pa, len, prot, attr);
	if (res != TEE_SUCCESS)
		return res;

	/* get calling session */
	s = tee_ta_get_calling_session();
	if (s == NULL)
		return TEE_ERROR_BAD_STATE;

	res = tee_mmu_umap_mmap(to_user_ta_ctx(s->ctx),
				pa, len,
				xflags_to_attr(prot, attr),
				&va);
	if (res != TEE_SUCCESS)
		return res;

	/* fill out param */
	reg_pair_from_64(va, &params[0].value.a, &params[0].value.b);
	return TEE_SUCCESS;
}

/*
 * deletes the mappings for the specified address range, and causes further references
 * to addresses within the range to generate invalid memory references
 */
TEE_Result svcx_unmap(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res;
	struct tee_ta_session *s = NULL;
	TEE_Identity clnt;
	vaddr_t va;
	size_t len;

	ASSERT_PARAM_TYPE(TEE_PARAM_TYPES
			  (TEE_PARAM_TYPE_VALUE_INPUT,
			   TEE_PARAM_TYPE_VALUE_INPUT,
			   TEE_PARAM_TYPE_NONE,
			   TEE_PARAM_TYPE_NONE));

	res = tee_ta_get_client_id(&clnt);
	if (res != TEE_SUCCESS)
		return res;

	/* check if calling from TA */
	if (clnt.login != TEE_LOGIN_TRUSTED_APP)
		return TEE_ERROR_NOT_SUPPORTED;

	/* extract params */
	va = (vaddr_t)reg_pair_to_64(params[0].value.a, params[0].value.b);
	len = reg_pair_to_64(params[1].value.a, params[1].value.b);

	/* get calling session */
	s = tee_ta_get_calling_session();
	if (s == NULL)
		return TEE_ERROR_BAD_STATE;

	return tee_mmu_umap_munmap(to_user_ta_ctx(s->ctx),
				 va, len);
}
