/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
 */

#include <assert.h>
#include <kernel/panic.h>
#include <kernel/misc.h>
#include <util.h>
#include <io.h>
#include <trace.h>
#include "gic_common.h"
#include "gic_common_private.h"
#include "gicv3_private.h"

/*
 * Accessor to read the GIC Distributor IGRPMODR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
unsigned int gicd_read_igrpmodr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> IGRPMODR_SHIFT;
	return read32(base + GICD_IGRPMODR + (n << 2));
}

/*
 * Accessor to write the GIC Distributor IGRPMODR corresponding to the
 * interrupt `id`, 32 interrupt IDs at a time.
 */
void gicd_write_igrpmodr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> IGRPMODR_SHIFT;
	write32(val, base + GICD_IGRPMODR + (n << 2));
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
unsigned int gicd_get_igrpmodr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igrpmodr(base, id);

	return (reg_val >> bit_num) & 0x1;
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
void gicd_set_igrpmodr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igrpmodr(base, id);

	gicd_write_igrpmodr(base, id, reg_val | (1 << bit_num));
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Distributor IGRPMODR.
 */
void gicd_clr_igrpmodr(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicd_read_igrpmodr(base, id);

	gicd_write_igrpmodr(base, id, reg_val & ~(1 << bit_num));
}

/*
 * Accessor to read the GIC Re-distributor IPRIORITYR corresponding to the
 * interrupt `id`, 4 interrupts IDs at a time.
 */
unsigned int gicr_read_ipriorityr(uintptr_t base, unsigned int id)
{
	unsigned n = id >> IPRIORITYR_SHIFT;
	return read32(base + GICR_IPRIORITYR + (n << 2));
}

/*
 * Accessor to write the GIC Re-distributor IPRIORITYR corresponding to the
 * interrupt `id`, 4 interrupts IDs at a time.
 */
void gicr_write_ipriorityr(uintptr_t base, unsigned int id, unsigned int val)
{
	unsigned n = id >> IPRIORITYR_SHIFT;
	write32(val, base + GICR_IPRIORITYR + (n << 2));
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * from GIC Re-distributor IGROUPR0.
 */
unsigned int gicr_get_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igroupr0(base);

	return (reg_val >> bit_num) & 0x1;
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor IGROUPR0.
 */
void gicr_set_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igroupr0(base);

	gicr_write_igroupr0(base, reg_val | (1 << bit_num));
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Re-distributor IGROUPR0.
 */
void gicr_clr_igroupr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igroupr0(base);

	gicr_write_igroupr0(base, reg_val & ~(1 << bit_num));
}

/*
 * Accessor to get the bit corresponding to interrupt ID
 * from GIC Re-distributor IGRPMODR0.
 */
unsigned int gicr_get_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	return (reg_val >> bit_num) & 0x1;
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor IGRPMODR0.
 */
void gicr_set_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	gicr_write_igrpmodr0(base, reg_val | (1 << bit_num));
}

/*
 * Accessor to clear the bit corresponding to interrupt ID
 * in GIC Re-distributor IGRPMODR0.
 */
void gicr_clr_igrpmodr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << IGRPMODR_SHIFT) - 1);
	unsigned int reg_val = gicr_read_igrpmodr0(base);

	gicr_write_igrpmodr0(base, reg_val & ~(1 << bit_num));
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor ISENABLER0.
 */
void gicr_set_isenabler0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISENABLER_SHIFT) - 1);

	gicr_write_isenabler0(base, (1 << bit_num));
}

/*
 * Accessor to set the byte corresponding to interrupt ID
 * in GIC Re-distributor IPRIORITYR.
 */
void gicr_set_ipriorityr(uintptr_t base, unsigned int id, unsigned int pri)
{
	write8(pri & GIC_PRI_MASK, base + GICR_IPRIORITYR + id);
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor ICENABLER0.
 */
void gicr_set_icenabler0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISENABLER_SHIFT) - 1);

	gicr_write_icenabler0(base, (1 << bit_num));
}

/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor ISPENDR0.
 */
void gicr_set_ispendr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ISPENDR_SHIFT) - 1);
	write32((1 << bit_num), base + GICR_ISPENDR0);
}
/*
 * Accessor to set the bit corresponding to interrupt ID
 * in GIC Re-distributor ICPENDR0.
 */
void gicr_set_icpendr0(uintptr_t base, unsigned int id)
{
	unsigned bit_num = id & ((1 << ICPENDR_SHIFT) - 1);
	write32((1 << bit_num), base + GICR_ICPENDR0);
}
