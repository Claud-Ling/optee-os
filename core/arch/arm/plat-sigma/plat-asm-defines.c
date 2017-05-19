/*
 * Copyright (c) 2016, Linaro Limited
 * Copyright (c) 2017, Sigma Designs Inc.
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
 */

#include <sm/sm.h>
#include <types_ext.h>
#include <smccc.h>

#define DEFINES void __defines(void); void __defines(void)

#define DEFINE(def, val) \
	asm volatile("\n==>" #def " %0 " #val : : "i" (val))

DEFINES
{
#ifdef ARM32
	DEFINE(SMCCC_RES_A0, offsetof(struct smccc_res, a0));
	DEFINE(SMCCC_RES_A1, offsetof(struct smccc_res, a1));
	DEFINE(SMCCC_RES_A2, offsetof(struct smccc_res, a2));
	DEFINE(SMCCC_RES_A3, offsetof(struct smccc_res, a3));
#endif /*ARM32*/

#ifdef ARM64
	DEFINE(SMCCC_RES_A0, offsetof(struct smccc_res, a0));
	DEFINE(SMCCC_RES_A2, offsetof(struct smccc_res, a2));
#endif /*ARM64*/
}
