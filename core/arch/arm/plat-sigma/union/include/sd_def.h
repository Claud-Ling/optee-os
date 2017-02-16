/*
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

#ifndef __SD_DEF_H__
#define __SD_DEF_H__

/*
 * SoC constants
 */
#define CONFIG_SIGMA_SOC_UNION

/*
 * Cache granule
 */
#define CACHE_LSZ_SHIFT		6
#define CACHE_LSZ_GRANUEL	(1 << CACHE_LSZ_SHIFT)
#define CACHE_WRITEBACK_SHIFT   CACHE_LSZ_SHIFT
#define CFG_CACHE_WRITEBACK_GRANULE CACHE_LSZ_GRANUEL

/*
 * secondary boot register (32-bit)
 *
 * +--------------------------------+
 * |31                        4|3  0|
 * +---------------------------+----+
 * | physical entry address    | id |
 * +---------------------------+----+
 *
 * Note:
 * entry address must be 16-byte aligned at least
 */
#define AUX_BOOT_ADDR_REG		0xFB00F07C	/*this belongs to sectimer target,
							 *non-accessible to NS world
							 */
#define AUX_BOOT_ID_BITS		4
#define AUX_BOOT_ID_MASK		((1 << AUX_BOOT_ID_BITS) - 1)
#define AUX_BOOT_ADDR_MASK		(~AUX_BOOT_ID_MASK)

#define MK_AUX_BOOT_VAL(ep, id) 	(((ep) & AUX_BOOT_ADDR_MASK) | \
					 ((id) & AUX_BOOT_ID_MASK))
#define AUX_BOOT_ADDR(val)		((val) & AUX_BOOT_ADDR_MASK)
#define AUX_BOOT_ID(val)		((val) & AUX_BOOT_ID_MASK)

/*
 * Memory related constants
 */

/*
 * Union memory map
 *
 *  0x4000_0000                               -
 *    Available to Linux                      | DRAM1
 *  0x1100_0000                               -
 *    TA RAM: 12 MiB                          |
 *  0x1040_0000                               | TZDRAM
 *    TEE RAM: 1 MiB (CFG_TEE_RAM_VA_SIZE)    |
 *  0x1026_0000 [TZDRAM_BASE, BL32_LOAD_ADDR] -
 *    ATF RAM: 384 KiB                        | ATFRAM
 *  0x1020_0000 [BL2_LOAD_ADDR]               -
 *    Shared memory: 2 MiB                    |
 *  0x1000_0000                               | DRAM0
 *    Available to Linux                      |
 *  0x0000_0000 [DRAM0_BASE]                  -
 *
 */

/* Always assume DDR is 1GB size. */
#define SD_DRAM_BASE			0x00000000
#define SD_DRAM_SIZE			0x40000000

/* World Share Memory 2M@256M */
#define SD_WSM_BASE			0x10000000
#define SD_WSM_SIZE			0x00200000

/* Secure DRAM 14M@258M */
#define SD_SEC_DRAM_BASE		0x10200000
#define SD_SEC_DRAM_SIZE		0x00E00000
#define SD_SEC_DRAM_LIMIT		(SD_SEC_DRAM_BASE + SD_SEC_DRAM_SIZE)

/* Non-secure DRAM 256M@0M, 752M@272M*/
#define SD_NS_DRAM_BASE			SD_DRAM_BASE
#define SD_NS_DRAM_SIZE			(SD_SEC_DRAM_BASE - SD_DRAM_BASE)
#define SD_NS_DRAM_BASE2		(SD_SEC_DRAM_BASE + SD_SEC_DRAM_SIZE)
#define SD_NS_DRAM_SIZE2		(SD_DRAM_SIZE - SD_NS_DRAM_SIZE - SD_SEC_DRAM_SIZE)

/*
 * ATF Memory
 */
#define SD_ATF_DRAM_BASE		SD_SEC_DRAM_BASE
#define SD_ATF_DRAM_SIZE		0x00060000
#define SD_ATF_DRAM_LIMIT		(SD_ATF_DRAM_BASE + SD_ATF_DRAM_SIZE)
/*
 * Trust OS Memory
 */
#define SD_TEE_DRAM_BASE		SD_ATF_DRAM_LIMIT
#define SD_TEE_DRAM_SIZE		0x00100000
#define SD_TEE_DRAM_LIMIT		(SD_TEE_DRAM_BASE + SD_TEE_DRAM_SIZE)

/*
 * Devices space
 */
#define SD_DEVICE_BASE			0xF0000000
#define SD_DEVICE_SIZE			0x0FF00000

/*
 * SRAM, 72k@0xfff00000
 */
#define SD_SRAM_BASE			0xFFF00000
#define SD_SRAM_SIZE			0x00012000

/*
 * Serial related constants
 */
#define SD_BOOT_UART_BASE		0xFB005100
#define SD_UART_CLK_HZ			192000
#define SD_UART_BAUDRATE		115200

/*
 * OTP constants
 */
#define SD_TURING_BASE			0xF1040000
#define SD_OTP_FUSE_BASE		(SD_TURING_BASE + 0x1000)
#define SD_OTP_DATA_BASE		(SD_OTP_FUSE_BASE + 0x100)

/*
 * GIC-500 & interrupt handling related constants
 */
#define BASE_GICD_BASE			0xFFC00000
#define BASE_GICR_BASE			0xFFC40000	/*core 0*/

#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

#define MAX_INTR_EL3			160

#ifndef ASM

#endif /*!ASM*/
#endif /*__SD_DEF_H__*/
