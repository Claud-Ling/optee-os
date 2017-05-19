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

#include <types_ext.h>
#include <kernel/misc.h> /*reg_pair_to_64/reg_pair_from_64*/
#include <kernel/tee_ta_manager.h>
#include <tee_api_types.h>
#include <tee_api_defines.h>
#include <firmware.h>
#include "svcx_internal.h"

/*
 * query memory access state
 */
TEE_Result svcx_mem_state(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS])
{
	TEE_Result res;
	TEE_Identity clnt;
	paddr_t pa;
	size_t len;
	uint32_t state;

	ASSERT_PARAM_TYPE(TEE_PARAM_TYPES
			  (TEE_PARAM_TYPE_VALUE_INOUT,
			   TEE_PARAM_TYPE_VALUE_INPUT,
			   TEE_PARAM_TYPE_NONE,
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

	res = fw_get_mem_state(pa, len, &state);
	if (TEE_SUCCESS == res) {
		params[0].value.a = state;
	}
	return res;
}
