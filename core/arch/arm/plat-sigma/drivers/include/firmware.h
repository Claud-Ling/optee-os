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
 * Brief:   This file decribes firmware interfaces
 * Author:  Tony He <tony_he@sigmadesigns.com>
 * Date:    2017/05/15
 *
 */

#ifndef __FIRMWARE_H__
#define __FIRMWARE_H__

#include <tee_api_types.h>	/*TEE_Result*/
#include <types_ext.h>

#ifndef ASM
/*
 *
 * struct secure_fw_ops -
 *    represents the various operations provided by firmware
 *
 */
struct secure_fw_ops {
	/*
	 * debug only
	 */
	const char* name;
	/*
	 * read otp
	 */
	TEE_Result (*fuse_read)(const size_t ofs, void *va, uint32_t *size, uint32_t *pprot);
	/*
	 * write otp
	 */
	TEE_Result (*fuse_write)(const size_t ofs, void *va, uint32_t *size, const uint32_t prot);
	/*
	 * check memory access state
	 */
	TEE_Result (*get_mem_state)(const paddr_t pa, const size_t len, uint32_t *pstate);
};

#define FW_OPS(tag, fusrd_fn,			\
		fuswr_fn, mem_state_fn)		\
struct secure_fw_ops fw_ops = {			\
	.name = tag,				\
	.fuse_read = fusrd_fn,			\
	.fuse_write = fuswr_fn,			\
	.get_mem_state = mem_state_fn,		\
};

TEE_Result fw_register_drv(struct secure_fw_ops *ops);
TEE_Result fw_fuse_read(const size_t ofs, void *va, uint32_t *size, uint32_t *pprot);
TEE_Result fw_fuse_write(const size_t ofs, void *va, uint32_t *size, const uint32_t prot);
TEE_Result fw_get_mem_state(const paddr_t pa, const size_t len, uint32_t *pstate);

#endif /* !ASM */
#endif /* __FIRMWARE_H__ */
