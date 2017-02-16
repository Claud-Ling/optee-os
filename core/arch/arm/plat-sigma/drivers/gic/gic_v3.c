/*
 * Copyright (c) 2016-2017, Sigma Designs Inc.
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
 * Date:    2017/02/06
 *
 */

#include <assert.h>
#include <kernel/panic.h>
#include <kernel/misc.h>
#include <util.h>
#include <io.h>
#include <trace.h>
#include <gic_v3.h>
#include <platform_internal.h>
#include "gic_common.h"
#include "gic_common_private.h"
#include "gicv3_private.h"
#include "gicv3_helpers_a64.h"

/* Number of Private Peripheral Interrupt */
#define NUM_PPI	32

/* Number of Software Generated Interrupt */
#define NUM_SGI			16

/* Number of Non-secure Software Generated Interrupt */
#define NUM_NS_SGI		8
#define MIN_SEC_SGI_ID		8

/* Number of interrupts in one register */
#define NUM_INTS_PER_REG	32

/*
 * Constant passed to the interrupt handler in the 'id' field when the
 * framework does not read the gic registers to determine the interrupt id.
 */
#define INTR_ID_UNAVAILABLE	0xFFFFFFFF

/*
 * ICC_SGIR
 */
#define ICC_SGIR_AFFINITY_3_SHIFT	48
#define ICC_SGIR_AFFINITY_2_SHIFT	32
#define ICC_SGIR_AFFINITY_1_SHIFT	16
#define ICC_SGIR_IRM_SHIFT		40
#define ICC_SGIR_INITID_SHFIT		24
#define ICC_SGIR_TLIST_SHIFT		0

#define MK_SGIR_VAL(aff3,aff2,aff1,id,tlist)	\
	((((uint64_t)(aff3)) << ICC_SGIR_AFFINITY_3_SHIFT) |	\
	 (((uint64_t)(aff2)) << ICC_SGIR_AFFINITY_2_SHIFT) |	\
	 (((uint64_t)(aff1)) << ICC_SGIR_AFFINITY_1_SHIFT) |	\
	 ((id) << ICC_SGIR_INITID_SHFIT) |	\
	 ((tlist) << ICC_SGIR_TLIST_SHIFT))

/*
 * IROUTER SHIFTS
 */
#define IROUTER_AFFINITY_3_SHIFT	32
#define IROUTER_AFFINITY_2_SHIFT	16
#define IROUTER_AFFINITY_1_SHIFT	8
#define IROUTER_AFFINITY_0_SHIFT	0

#define MK_IROUTER_VAL(aff3,aff2,aff1,aff0,irm)	\
	((((uint64_t)(aff3)) << IROUTER_AFFINITY_3_SHIFT) |	\
	 (((uint64_t)(aff2)) << IROUTER_AFFINITY_2_SHIFT) |	\
	 (((uint64_t)(aff1)) << IROUTER_AFFINITY_1_SHIFT) |	\
	 (((uint64_t)(aff0)) << IROUTER_AFFINITY_0_SHIFT) |	\
	 (((irm) & IROUTER_IRM_MASK) << IROUTER_IRM_SHIFT))


static void gic_op_add(struct itr_chip *chip, size_t it, uint32_t flags);
static void gic_op_enable(struct itr_chip *chip, size_t it);
static void gic_op_disable(struct itr_chip *chip, size_t it);
static void gic_op_raise_pi(struct itr_chip *chip, size_t it);
static void gic_op_raise_sgi(struct itr_chip *chip, size_t it,
			uint8_t cpu_mask);
static void gic_op_set_affinity(struct itr_chip *chip, size_t it,
			uint8_t cpu_mask);

static const struct itr_ops gic_ops = {
	.add = gic_op_add,
	.enable = gic_op_enable,
	.disable = gic_op_disable,
	.raise_pi = gic_op_raise_pi,
	.raise_sgi = gic_op_raise_sgi,
	.set_affinity = gic_op_set_affinity,
};

static int gic_irq_in_rdist(unsigned int id)
{
	return id < MIN_SPI_ID;
}

static bool gic_it_is_enabled(struct gic_data *gd,  unsigned int id)
{
	uint32_t reg_val;
	int bit_num = id & ((1 << ISENABLER_SHIFT) - 1);

	if (gic_irq_in_rdist(id)) { /*PPIs & SGIs*/
		vaddr_t gicr_base = gd->gicr_base[get_core_pos()];
		reg_val = gicr_read_isenabler0(gicr_base);
	} else {	/*SPIs*/
		vaddr_t gicd_base = gd->gicd_base;
		reg_val = gicd_read_isenabler(gicd_base, id);
	}
	return !!(reg_val & (1 << bit_num));
}

static int __maybe_unused gic_it_get_group(struct gic_data *gd, unsigned int id)
{
	int grp_status_bit, grp_mod_bit, grp;
	int bit_num = id & ((1 << IGROUPR_SHIFT) - 1);
	if (gic_irq_in_rdist(id)) { /*PPIs & SGIs*/
		vaddr_t gicr_base = gd->gicr_base[get_core_pos()];
		grp_status_bit = (gicr_read_igroupr0(gicr_base) >> bit_num) & 1;
		grp_mod_bit = (gicr_read_igrpmodr0(gicr_base) >> bit_num) & 1;
	} else {	/*SPIs*/
		vaddr_t gicd_base = gd->gicd_base;
		grp_status_bit = (gicd_read_igroupr(gicd_base, id) >> bit_num) & 1;
		grp_mod_bit = (gicd_read_igrpmodr(gicd_base, id) >> bit_num) & 1;
	}

	if (grp_mod_bit == 0) {
		if (grp_status_bit == 0)
			grp = INTR_GROUP0;
		else
			grp = INTR_GROUP1NS;
	} else {
		if (grp_status_bit == 0)
			grp = INTR_GROUP1S;
		else {
			grp = INTR_GROUP1NS;	/*reserved, treated as G1NS anyway*/
		}
	}
	return grp;
}

static void gic_it_add(struct gic_data *gd, unsigned int id)
{
	vaddr_t gicr_base = gd->gicr_base[get_core_pos()];
	vaddr_t gicd_base = gd->gicd_base;

	if (gic_irq_in_rdist(id)) { /*PPIs & SGIs*/
		/* Configure this interrupt as a secure interrupt */
		gicr_clr_igroupr0(gicr_base, id);

		/* Configure this interrupt as G1S interrupt */
		gicr_set_igrpmodr0(gicr_base, id);

		/* Set the priority of this interrupt */
		gicr_set_ipriorityr(gicr_base,
				    id,
				    GIC_HIGHEST_SEC_PRIORITY);
	} else { /*SPIs*/
		uint64_t gic_affinity_val;
		/* Configure this interrupt as a secure interrupt */
		gicd_clr_igroupr(gicd_base, id);

		/* Configure this interrupt as G1S interrupt */
		gicd_set_igrpmodr(gicd_base, id);
		/* Set the priority of this interrupt */
		gicd_set_ipriorityr(gicd_base,
				      id,
				      GIC_HIGHEST_SEC_PRIORITY);

		/* Target SPIs to the primary CPU */
		gic_affinity_val =
			gicd_irouter_val_from_mpidr(0, 0);
		gicd_write_irouter(gicd_base,
				   id,
				   gic_affinity_val);

		/* Enable the secure SPIs now that they have been configured */
		if (!(gicd_read_ctlr(gicd_base) & CTLR_ENABLE_G1S_BIT)) {
			gicd_set_ctlr(gicd_base, CTLR_ENABLE_G1S_BIT, RWP_TRUE);
		}
	}
}

static void gic_it_enable(struct gic_data *gd, unsigned int id)
{
	/* Enable this interrupt */
	if (gic_irq_in_rdist(id)) { /*PPIs & SGIs*/
		vaddr_t gicr_base = gd->gicr_base[get_core_pos()];
		gicr_set_isenabler0(gicr_base, id);
	} else {
		vaddr_t gicd_base = gd->gicd_base;
		gicd_set_isenabler(gicd_base, id);
	}
}

static void gic_it_disable(struct gic_data *gd, unsigned int id)
{
	/* Disable this interrupt */
	if (gic_irq_in_rdist(id)) { /*PPIs & SGIs*/
		vaddr_t gicr_base = gd->gicr_base[get_core_pos()];
		gicr_set_icenabler0(gicr_base, id);
		gicr_wait_for_pending_write(gicr_base);
	} else {
		vaddr_t gicd_base = gd->gicd_base;
		gicd_set_icenabler(gicd_base, id);
		gicd_wait_for_pending_write(gicd_base);
	}
}

static void gic_it_set_pending(struct gic_data *gd, unsigned int id)
{
	/* Set pending of this interrupt */
	if (gic_irq_in_rdist(id)) { /*PPIs & SGIs*/
		vaddr_t gicr_base = gd->gicr_base[get_core_pos()];
		gicr_set_ispendr0(gicr_base, id);
	} else {
		vaddr_t gicd_base = gd->gicd_base;
		gicd_set_ispendr(gicd_base, id);
	}
}

static void __maybe_unused gic_it_clr_pending(struct gic_data *gd, unsigned int id)
{
	/* Clear pending of this interrupt */
	if (gic_irq_in_rdist(id)) { /*PPIs & SGIs*/
		vaddr_t gicr_base = gd->gicr_base[get_core_pos()];
		gicr_set_icpendr0(gicr_base, id);
	} else {
		vaddr_t gicd_base = gd->gicd_base;
		gicd_set_icpendr(gicd_base, id);
	}
}

static void gic_it_raise_sgi(struct gic_data *gd __maybe_unused, unsigned int id, unsigned short cpu_mask)
{
	uint64_t val;
	assert(id < NUM_SGI);

	val = MK_SGIR_VAL(0,0,0,id,cpu_mask);
	if (id < MIN_SEC_SGI_ID) {
		/*raise Non-secure sgi, G1NS*/
		DMSG("CPU%ld: ICC_ASGI1R_EL1: %lx", get_core_pos(), val);
		write_icc_asgi1r_el1(val);
	} else {
		/*raise secure sgi, G1S*/
		DMSG("CPU%ld: ICC_SGI1R_EL1: %lx", get_core_pos(), val);
		write_icc_sgi1r_el1(val);
		// TODO: raise G0S sgi
	}
}

static void gic_it_set_affinity(struct gic_data *gd, unsigned int id, unsigned short cpu_mask)
{
	int enabled;
	uint64_t affinity_val;
	vaddr_t gicd_base;

	assert(!gic_irq_in_rdist(id));
	assert(cpu_mask);

	gicd_base = gd->gicd_base;
	enabled = gic_it_is_enabled(gd, id);
	/* If interrupt was enabled, disable it first*/
	if (enabled) {
		gicd_set_icenabler(gicd_base, id);
		gicd_wait_for_pending_write(gicd_base);
	}

	// TODO: to mask out offline cpus from cpu_mask

	/* Route it to selected CPUs */
	if (__builtin_popcount(cpu_mask) > 1) {
		/*TODO*/
		/*targeting multi cores, just set IRM=1 to route to any participating cpus*/
		affinity_val = MK_IROUTER_VAL(0,0,0,0,1);
	} else {
		int cpu = __builtin_ctz(cpu_mask);
		affinity_val = MK_IROUTER_VAL(0,0,0,cpu,0);
	}
	gicd_write_irouter(gicd_base, id, affinity_val);

	/*
	 * If the interrupt was enabled, enabled it again.
	 * Otherwise, just wait the distributor to complete.
	 */
	if (enabled) {
		gicd_set_isenabler(gicd_base, id);
	} else {
		gicd_wait_for_pending_write(gicd_base);
	}
}


static void gic_op_add(struct itr_chip *chip, size_t it,
		       uint32_t flags __unused)
{
	struct gic_data *gd = container_of(chip, struct gic_data, chip);

	if (it >= gd->max_it)
		panic();

	gic_it_disable(gd, it);	/*disable interrupt before add as expected by itr*/
	gic_it_add(gd, it);
}

static void gic_op_enable(struct itr_chip *chip, size_t it)
{
	struct gic_data *gd = container_of(chip, struct gic_data, chip);

	if (it >= gd->max_it)
		panic();

	gic_it_enable(gd, it);
}

static void gic_op_disable(struct itr_chip *chip, size_t it)
{
	struct gic_data *gd = container_of(chip, struct gic_data, chip);

	if (it >= gd->max_it)
		panic();

	gic_it_disable(gd, it);
}

static void gic_op_raise_pi(struct itr_chip *chip, size_t it)
{
	struct gic_data *gd = container_of(chip, struct gic_data, chip);

	if (it >= gd->max_it)
		panic();

	gic_it_set_pending(gd, it);
}

static void gic_op_raise_sgi(struct itr_chip *chip, size_t it,
			uint8_t cpu_mask)
{
	struct gic_data *gd = container_of(chip, struct gic_data, chip);

	if (it >= gd->max_it)
		panic();

	gic_it_raise_sgi(gd, it, cpu_mask);
}

static void gic_op_set_affinity(struct itr_chip *chip, size_t it,
			uint8_t cpu_mask)
{
	struct gic_data *gd = container_of(chip, struct gic_data, chip);

	if (it >= gd->max_it)
		panic();

	gic_it_set_affinity(gd, it, cpu_mask);
}

static size_t probe_max_it(vaddr_t gicd_base)
{
	unsigned int num_ints;

	num_ints = gicd_read_typer(gicd_base);
	num_ints &= TYPER_IT_LINES_NO_MASK;
	num_ints = (num_ints + 1) << 5;
	return num_ints;
}

/*
 * This function returns the highest priority pending interrupt at
 * the Interrupt controller
 */
static uint32_t gic_read_hppir(void)
{
	unsigned int irqnr;

	irqnr = read_icc_hppir1_el1() & HPPIR1_EL1_INTID_MASK;
	return (irqnr == GIC_SPURIOUS_INTERRUPT) ?
				INTR_ID_UNAVAILABLE : irqnr;
}

/*
 * This function returns the highest priority pending interrupt at
 * the Interrupt controller and indicates to the Interrupt controller
 * that the interrupt processing has started.
 */
static uint32_t gic_read_iar(void)
{
	return (read_icc_iar1_el1() & IAR1_EL1_INTID_MASK);
}

/*
 * This functions is used to indicate to the interrupt controller that
 * the processing of the interrupt corresponding to the `id` has
 * finished.
 */
static void gic_eof_irq(uint32_t id)
{
	write_icc_eoir1_el1(id);
	isb();
}

/*
 * Initialize GIC driver, do simple things here, i.e. set base addr.
 * Note the GICv3 driver is initialized in EL3 and does not need
 * to be initialized again in SEL1.
 */
void gic_init(struct gic_data *gd, vaddr_t gicd_base,
			vaddr_t gicr_base)
{
	unsigned int proc_num;
	uint64_t typer_val;
	uintptr_t gicrif_base = gicr_base;
	assert(gicd_read_ctlr(gicd_base) & CTLR_ARE_S_BIT);
	assert(read_icc_sre_el1() & 0x1); /*SRE must be on*/

	gd->gicd_base = gicd_base;
	/*
	 * Iterate over the Redistributor frames. Store the base address of each
	 * frame in the platform provided array. Use the "Processor Number"
	 * field to index into the array.
	 */
	do {
		typer_val = gicr_read_typer(gicrif_base);
		proc_num = (typer_val >> TYPER_PROC_NUM_SHIFT) &
			TYPER_PROC_NUM_MASK;
		assert(proc_num < CFG_TEE_CORE_NB_CORE);
		gd->gicr_base[proc_num] = gicrif_base;
		gicrif_base += (1 << GICR_PCPUBASE_SHIFT);
	} while (!(typer_val & TYPER_LAST_BIT));

	gd->max_it = probe_max_it(gicd_base);
	gd->chip.ops = &gic_ops;
}

void gic_dump_state(struct gic_data *gd)
{
	int i;
	int cpu_id __maybe_unused;

	cpu_id = get_core_pos();
	DMSG("GICR_CTLR: 0x%x", read32(gd->gicr_base[cpu_id] + GICR_CTLR));
	DMSG("GICD_CTLR: 0x%x", read32(gd->gicd_base + GICD_CTLR));

	for (i = 0; i < (int)gd->max_it; i++) {
		if (gic_it_is_enabled(gd, i)) {
			DMSG("irq%d: enabled, group:%d", i,
			     gic_it_get_group(gd, i));
		}
	}
}

void gic_it_handle(struct gic_data *gd __maybe_unused)
{
	uint32_t id;
	uint32_t old_itr_status __maybe_unused; 

	id = gic_read_hppir();

	if (id == INTR_ID_UNAVAILABLE) {
		DMSG("ignoring spurious interrupt");
	} else {
		id = gic_read_iar();
		itr_handle(id);
		gic_eof_irq(id);

#ifdef CFG_TEE_STATS
		/* Update the statistics and print some messages */
		tee_stats[get_core_pos()].sel1_intr_count++;
#if TRACE_LEVEL >= TRACE_FLOW
		old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
		cpu_spin_lock(&console_lock);
		FMSG("TEE: cpu 0x%lx handled S-EL1 interrupt %d",
		       read_mpidr_el1(), id);
		FMSG("TEE: cpu 0x%lx: %d S-EL1 requests",
		     read_mpidr_el1(), tee_stats[get_core_pos()].sel1_intr_count);
		cpu_spin_unlock(&console_lock);
		thread_unmask_exceptions(old_itr_status);
#endif
#endif /*CFG_TEE_STATS*/
	}
}

