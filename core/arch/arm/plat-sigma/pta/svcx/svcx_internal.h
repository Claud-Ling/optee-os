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
#include <tee_api_types.h>
#include <tee_api_defines.h>
#include <pta_svcx.h>

#ifndef __SVCX_INTERNAL_H__
#define __SVCX_INTERNAL_H__

#ifndef __ASSEMBLY__

#define ASSERT_PARAM_TYPE(pt) \
do { \
	if ((pt) != param_types) \
		return TEE_ERROR_BAD_PARAMETERS; \
} while (0)

TEE_Result svcx_not_supported(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]);

#ifdef CFG_MMAP_API
/*
 * add one user map for calling TA
 */
TEE_Result svcx_map(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]);

/*
 * deletes the mappings for the specified address range, and causes further references
 * to addresses within the range to generate invalid memory references
 */
TEE_Result svcx_unmap(uint32_t param_types, TEE_Param params[TEE_NUM_PARAMS]);
#else
#define  svcx_map	svcx_not_supported
#define  svcx_unmap	svcx_not_supported
#endif

#endif /* !__ASSEMBLY__ */
#endif /* __SVCX_INTERNAL_H__ */
