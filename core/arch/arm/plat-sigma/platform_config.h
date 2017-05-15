/*
 * Copyright (c) 2015, Linaro Limited
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

#include <sd_def.h>
#include <stdint.h>

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

/* Make stacks aligned to data cache line length */
#define STACK_ALIGNMENT		CACHE_LSZ_GRANUEL

#ifdef ARM64
#ifdef CFG_WITH_PAGER
#error "Pager not supported for ARM64"
#endif
#endif /* ARM64 */

/* UART */
#define CONSOLE_UART_BASE       SD_BOOT_UART_BASE
#define CONSOLE_BAUDRATE	SD_UART_BAUDRATE
#define CONSOLE_UART_CLK_IN_HZ	SD_UART_CLK_HZ

#define DRAM0_BASE		SD_DRAM_BASE
#define DRAM0_SIZE		ROUNDDOWN(SD_DRAM_SIZE, CORE_MMU_DEVICE_SIZE)

#ifdef CFG_WITH_PAGER

#error "Pager not supported for trix"

#else /* CFG_WITH_PAGER */

#define TZDRAM_BASE		SD_TEE_DRAM_BASE
#define TZDRAM_SIZE		(SD_SEC_DRAM_LIMIT - SD_TEE_DRAM_BASE)

#endif /* CFG_WITH_PAGER */


#define CFG_SHMEM_START		ROUNDUP(SD_WSM_BASE, CORE_MMU_DEVICE_SIZE)
#define CFG_SHMEM_SIZE		ROUNDDOWN((SD_WSM_BASE + SD_WSM_SIZE - CFG_SHMEM_START), \
				CORE_MMU_DEVICE_SIZE)

#define CFG_TEE_RAM_VA_SIZE	SD_TEE_DRAM_SIZE

#define CFG_TEE_LOAD_ADDR	SD_TEE_DRAM_BASE

#ifdef CFG_WITH_PAGER

#error "Pager not supported for trix"

#else /* CFG_WITH_PAGER */

#define CFG_TEE_RAM_PH_SIZE	CFG_TEE_RAM_VA_SIZE
#define CFG_TEE_RAM_START	SD_TEE_DRAM_BASE
#define CFG_TA_RAM_START	ROUNDUP((SD_TEE_DRAM_BASE + CFG_TEE_RAM_VA_SIZE), \
					CORE_MMU_DEVICE_SIZE)

#define CFG_TA_RAM_SIZE		ROUNDDOWN((SD_SEC_DRAM_LIMIT - SD_TEE_DRAM_LIMIT),\
					  CORE_MMU_DEVICE_SIZE)

#endif /* CFG_WITH_PAGER */

#ifndef ASM
#define HAVE_PLAT_STATES	1
void plat_update_preemption_state(void);
void plat_update_rpc_state(void);
void plat_update_std_smc_state(uint32_t fid);
#endif /* !ASM */
#endif /* PLATFORM_CONFIG_H */
