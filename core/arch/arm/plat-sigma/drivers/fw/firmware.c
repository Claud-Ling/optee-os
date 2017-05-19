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
 * Author:  Tony He <tony_he@sigmadesigns.com>
 * Date:    2017/05/15
 *
 */

#include <assert.h>
#include <trace.h>
#include <types_ext.h>
#include <platform_config.h>
#include <keep.h> 	/*KEEP_PAGER*/
#include <firmware.h>

#define call_sec_fw_op(op, ...)					\
	((sec_fw_ops && sec_fw_ops->op) ? sec_fw_ops->op(__VA_ARGS__) : (TEE_ERROR_NOT_SUPPORTED))

/**************************************************************************/
/* secure fw operations                                                   */
/**************************************************************************/
static struct secure_fw_ops *sec_fw_ops = NULL;
KEEP_PAGER(sec_fw_ops);

TEE_Result fw_register_drv(struct secure_fw_ops *ops)
{
	assert(ops != NULL);
	if (NULL == sec_fw_ops) {
		sec_fw_ops = ops;
		return TEE_SUCCESS;
	}
	return TEE_ERROR_GENERIC;
}

TEE_Result fw_fuse_read(const size_t ofs, void *va, uint32_t *size, uint32_t *pprot)
{
	return call_sec_fw_op(fuse_read, ofs, va, size, pprot);
}

TEE_Result fw_fuse_write(const size_t ofs, void *va, uint32_t *size, const uint32_t prot)
{
	return call_sec_fw_op(fuse_write, ofs, va, size, prot);
}

TEE_Result fw_get_mem_state(const paddr_t pa, const size_t len, uint32_t *pstate)
{
	return call_sec_fw_op(get_mem_state, pa, len, pstate);
}
