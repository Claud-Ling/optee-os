/*
 * Copyright (c) 2017, Sigma Designs Inc.
 * All rights reserved.
 *
 * Brief:  This file contains sigmadesigns defined tee internal api extensions for OP-TEE
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
#include <types_ext.h>	/*paddr_t*/
#include <pta_svcx.h>

#ifndef TEE_API_EXT_H
#define TEE_API_EXT_H

#ifndef ASM
/*
 * @fn		TEE_Result TEE_Ext_Init(void);
 * @brief	TEE Extension constructor that might speed up all later on calls to
 *		other extension APIs declared in this header file.
 *		Optional. But shall always come in couple with TEE_Ext_DeInit.
 * @return	TEE_SUCCESS on success. Otherwise error code.
 */
TEE_Result TEE_Ext_Init(void);

/*
 * @fn		TEE_Result TEE_Ext_DeInit(void);
 * @brief	TEE Extension deconstructor.
 *		Must be called if TEE_Ext_Init was called. Otherwise optional.
 * @return	void.
 */
void TEE_Ext_DeInit(void);

/*
 * @fn		void *TEE_Mmap(const paddr_t pa, const size_t len, const uint32_t prot,
 *				const uint32_t attr, uint32_t *err);
 * @brief	add one user map for memory [pa, pa+len) to the calling TA with specified attributes.
 * @param[in]	<pa>   - specifies physical memory address, must be page size (4K) aligned.
 * @param[in]	<len>  - specifies memory length, in bytes.
 * @param[in]	<prot> - specifies protection value for this map, is of type of enum tee_umap_prot.
 * @param[in]	<attr> - specifies attributes value for this map, is of type of enum tee_umap_attr.
 * @param[out]	<err>  - used to return error code, giving NULL to ignore.
 * @return	virtual address on success. Otherwise NULL.
 */
void *TEE_Mmap(const paddr_t pa, const size_t len, const uint32_t prot,
		const uint32_t attr, uint32_t *err);

/*
 * @fn		void *TEE_Munmap(void *va, const size_t len);
 * @brief	delete the specified user map from the calling TA.
 * @param[in]	<va>   - virtual memory address, obtained in an former TEE_Mmap call.
 * @param[in]	<len>  - specifies memory length, in bytes.
 * @return	TEE_SUCCESS on success. Otherwise error code.
 */
TEE_Result TEE_Munmap(void *va, size_t len);

/*
 * @fn		TEE_Result TEE_OtpWrite(const uint32_t id, void *va, uint32_t *const size,
 *					const uint32_t prot);
 * @brief	program OTP as specified by id. Accept call from TA ULI only.
 * @param[in]	<id>   - otp id, is of type of enum tee_otp_id.
 * @param[in]	<va>   - pointer of buffer with write data.
 * @param[inout]<size> - pointer of integar with write buffer size on entry and otp length on exit.
 * @param[in]	<prot> - specifies protection value, is of type of enum tee_otp_prot.
 * @return	TEE_SUCCESS on success. Otherwise error code.
 */
TEE_Result TEE_OtpWrite(const uint32_t id, void *va, uint32_t *const size,
			const uint32_t prot);

/*
 * @fn		TEE_Result TEE_MemState(const paddr_t pa, const size_t len,
 *					uint32_t *pstate);
 * @brief	probe access state for specified memory block.
 * @param[in]	<pa>   - specifies physical memory address.
 * @param[in]	<len>  - specifies memory length, in bytes.
 * @param[out]	<pstate> - pointer of integar filled with memory access state on success,
 *			   is of type of enum tee_mem_state.
 * @return	TEE_SUCCESS on success. Otherwise error code.
 */
TEE_Result TEE_MemState(const paddr_t pa, const size_t len,
			uint32_t *pstate);
#endif /* !ASM */
#endif /* TA_HELPER_SVCX_H */
