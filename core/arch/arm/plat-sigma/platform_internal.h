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
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
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
 * Date:    2017/02/09
 *
 */

#ifndef __PLATFORM_INTERNAL_H__
#define __PLATFORM_INTERNAL_H__

#include <platform_config.h>
#include <kernel/spinlock.h>

#ifndef ASM

typedef struct work_statistics {
	/* Number of s-el1 interrupts on this cpu */
	uint32_t sel1_intr_count;
	/* Number of non s-el1 interrupts on this cpu which preempted TSP */
	uint32_t preempt_intr_count;
	/* Number of sync s-el1 interrupts on this cpu */
	uint32_t sync_sel1_intr_count;
	/* Number of s-el1 interrupts returns on this cpu */
	uint32_t sync_sel1_intr_ret_count;
	uint32_t rpc_count;		/* Number of RPC calls on this cpu */
	uint32_t rpc_ret_count;		/* Number of RPC returns on this cpu */
	uint32_t smc_count;		/* Number of entries on this cpu */
	uint32_t eret_count;		/* Number of returns on this cpu */
	uint32_t cpu_on_count;		/* Number of cpu on requests */
	uint32_t cpu_off_count;		/* Number of cpu off requests */
	uint32_t cpu_suspend_count;	/* Number of cpu suspend requests */
	uint32_t cpu_resume_count;	/* Number of cpu resume requests */
} __aligned(CFG_CACHE_WRITEBACK_GRANULE) work_statistics_t;

/*
 * console spinlock
 */
extern unsigned int console_lock;

/*
 * Per cpu data structure to keep track of TEE activity
 */
extern work_statistics_t tee_stats[CFG_TEE_CORE_NB_CORE];

unsigned long cpu_on_entry(unsigned long a0, unsigned long a1);
unsigned long cpu_on_main(unsigned long a0, unsigned long a1);

#endif /*!ASM*/
#endif /*__PLATFORM_INTERNAL_H__*/
