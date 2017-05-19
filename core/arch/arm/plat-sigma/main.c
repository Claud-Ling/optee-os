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
 * Date:    2016/12/26
 *
 */

#include <console.h>
#include <drivers/ns16550.h>
#include <kernel/generic_boot.h>
#include <kernel/panic.h>
#include <kernel/pm_stubs.h>
#include <kernel/misc.h>
#include <mm/tee_pager.h>
#include <mm/core_memprot.h>
#include <platform_config.h>
#include <stdint.h>
#include <tee/entry_std.h>
#include <tee/entry_fast.h>
#include <keep.h>	/*KEEP_PAGER*/
#include <sm/optee_smc.h>
#include "platform_internal.h"

#ifdef CFG_GIC_V3
#include <gic_v3.h>
#endif
#ifdef CFG_CNTPS_TIMER
#include <cntps_timer.h>
#endif

#ifdef CFG_TEE_STATS
/*******************************************************************************
 * Per cpu data structure to keep track of TEE activity
 ******************************************************************************/
work_statistics_t tee_stats[CFG_TEE_CORE_NB_CORE];
KEEP_PAGER(tee_stats);
#endif

static void fast_smc_main(struct thread_smc_args *args);
static void main_fiq(void);
static unsigned long cpu_off_main(unsigned long a0, unsigned long a1);
static unsigned long cpu_suspend_main(unsigned long a0, unsigned long a1);
static unsigned long cpu_resume_main(unsigned long a0, unsigned long a1);
static unsigned long system_off_main(unsigned long a0, unsigned long a1);
static unsigned long system_reset_main(unsigned long a0, unsigned long a1);

static const struct thread_handlers handlers = {
	.std_smc = tee_entry_std,
	.fast_smc = fast_smc_main,
	.fiq = main_fiq,
	.cpu_on = cpu_on_entry,
	.cpu_off = cpu_off_main,
	.cpu_suspend = cpu_suspend_main,
	.cpu_resume = cpu_resume_main,
	.system_off = system_off_main,
	.system_reset = system_reset_main,
};

register_phys_mem(MEM_AREA_IO_NSEC, CONSOLE_UART_BASE, 0x1000);

/*console spin lock*/
unsigned int console_lock = 0;
KEEP_PAGER(console_lock);

#ifdef CFG_GIC_V3
static struct gic_data gic_data;
KEEP_PAGER(gic_data);

register_phys_mem(MEM_AREA_IO_SEC, BASE_GICD_BASE, 0x20000);
register_phys_mem(MEM_AREA_IO_SEC, BASE_GICR_BASE, (0x20000 * CFG_TEE_CORE_NB_CORE));

/*
 * override main_init_gic function
 */
void main_init_gic(void)
{
	vaddr_t gicd_base;
	vaddr_t gicr_base;

	gicd_base = (vaddr_t)phys_to_virt(BASE_GICD_BASE,
					  MEM_AREA_IO_SEC);
	gicr_base = (vaddr_t)phys_to_virt(BASE_GICR_BASE,
					  MEM_AREA_IO_SEC);

	if (!gicd_base || !gicr_base)
		panic();

	/* Initialize GIC */
	gic_init(&gic_data, gicd_base, gicr_base);
	itr_init(&gic_data.chip);
}
#endif

const struct thread_handlers *generic_boot_get_handlers(void)
{
#ifdef CFG_TEE_STATS
	uint32_t old_itr_status __maybe_unused; 
	uint32_t linear_id = get_core_pos();

	/* Update this cpu's statistics */
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;
	tee_stats[linear_id].cpu_on_count++;

#if TRACE_LEVEL >= TRACE_FLOW
	old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&console_lock);
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets %d cpu on requests",
	     read_mpidr_el1(),
	     tee_stats[linear_id].smc_count,
	     tee_stats[linear_id].eret_count,
	     tee_stats[linear_id].cpu_on_count);
	cpu_spin_unlock(&console_lock);
	thread_unmask_exceptions(old_itr_status);
#endif
#endif /*CFG_TEE_STATS*/

	return &handlers;
}

static void fast_smc_main(struct thread_smc_args *args)
{
#ifdef CFG_TEE_STATS
	uint32_t linear_id = get_core_pos();
	/* Update this cpu's statistics */
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;
#if TRACE_LEVEL >= TRACE_FLOW
	FMSG("TEE: cpu 0x%lx received fast smc 0x%lx", read_mpidr_el1(), args->a0);
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets", read_mpidr_el1(),
		tee_stats[linear_id].smc_count,
		tee_stats[linear_id].eret_count);
#endif
#endif /*CFG_TEE_STATS*/
	tee_entry_fast(args);
}

/*
 * entry for s-el1 interrupts, both sync and local ones.
 */
static void main_fiq(void)
{
#ifdef CFG_GIC_V3
	gic_it_handle(&gic_data);
#else
	panic();
#endif
}

unsigned long cpu_on_main(unsigned long a0 __unused, unsigned long a1 __unused)
{
	uint32_t old_itr_status __maybe_unused; 
	uint32_t linear_id __maybe_unused = get_core_pos();
#ifdef CFG_CNTPS_TIMER
	/* Initialize secure/applications state here */
	cntps_timer_start();
#endif

#ifdef CFG_TEE_STATS
	/* Update this cpu's statistics */
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;
	tee_stats[linear_id].cpu_on_count++;

#if TRACE_LEVEL >= TRACE_FLOW
	old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&console_lock);
	FMSG("TEE: cpu 0x%lx turned on", read_mpidr_el1());
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets %d cpu on requests",
		read_mpidr_el1(),
		tee_stats[linear_id].smc_count,
		tee_stats[linear_id].eret_count,
		tee_stats[linear_id].cpu_on_count);
	cpu_spin_unlock(&console_lock);
	thread_unmask_exceptions(old_itr_status);
#endif
#endif /*CFG_TEE_STATS*/
	return 0;
}

static unsigned long cpu_off_main(unsigned long a0, unsigned long a1)
{
	uint32_t old_itr_status __maybe_unused; 
	uint32_t linear_id __maybe_unused = get_core_pos();
#ifdef CFG_CNTPS_TIMER
	/*
	 * This cpu is being turned off, so disable the timer to prevent the
	 * secure timer interrupt from interfering with power down. A pending
	 * interrupt will be lost but we do not care as we are turning off.
	 */
	cntps_timer_stop();
#endif

#ifdef CFG_TEE_STATS
	/* Update this cpu's statistics */
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;
	tee_stats[linear_id].cpu_off_count++;

#if TRACE_LEVEL >= TRACE_FLOW
	old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&console_lock);
	FMSG("TEE: cpu 0x%lx off request", read_mpidr_el1());
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets %d cpu off requests",
		read_mpidr_el1(),
		tee_stats[linear_id].smc_count,
		tee_stats[linear_id].eret_count,
		tee_stats[linear_id].cpu_off_count);
	cpu_spin_unlock(&console_lock);
	thread_unmask_exceptions(old_itr_status);
#endif
#endif /*CFG_TEE_STATS*/
	return pm_do_nothing(a0, a1);
}

static unsigned long cpu_suspend_main(unsigned long a0, unsigned long a1)
{
	uint32_t old_itr_status __maybe_unused; 
	uint32_t linear_id __maybe_unused = get_core_pos();
#ifdef CFG_CNTPS_TIMER
	/*
	 * Save the time context and disable it to prevent the secure timer
	 * interrupt from interfering with wakeup from the suspend state.
	 */
	cntps_timer_save();
	cntps_timer_stop();
#endif

#ifdef CFG_TEE_STATS
	/* Update this cpu's statistics */
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;
	tee_stats[linear_id].cpu_suspend_count++;

#if TRACE_LEVEL >= TRACE_FLOW
	old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&console_lock);
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets %d cpu suspend requests",
		read_mpidr_el1(),
		tee_stats[linear_id].smc_count,
		tee_stats[linear_id].eret_count,
		tee_stats[linear_id].cpu_suspend_count);
	cpu_spin_unlock(&console_lock);
	thread_unmask_exceptions(old_itr_status);
#endif
#endif /*CFG_TEE_STATS*/
	return pm_do_nothing(a0, a1);
}

static unsigned long cpu_resume_main(unsigned long a0, unsigned long a1)
{
	uint32_t old_itr_status __maybe_unused; 
	uint32_t linear_id __maybe_unused = get_core_pos();
#ifdef CFG_CNTPS_TIMER
	/* Restore the generic timer context */
	cntps_timer_restore();
#endif

#ifdef CFG_TEE_STATS
	/* Update this cpu's statistics */
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;
	tee_stats[linear_id].cpu_resume_count++;

#if TRACE_LEVEL >= TRACE_FLOW
	old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&console_lock);
	FMSG("TEE: cpu 0x%lx resumed", read_mpidr_el1());
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets %d cpu resume requests",
		read_mpidr_el1(),
		tee_stats[linear_id].smc_count,
		tee_stats[linear_id].eret_count,
		tee_stats[linear_id].cpu_resume_count);
	cpu_spin_unlock(&console_lock);
	thread_unmask_exceptions(old_itr_status);
#endif
#endif /*CFG_TEE_STATS*/
	return pm_do_nothing(a0, a1);
}

static unsigned long system_off_main(unsigned long a0, unsigned long a1)
{
#ifdef CFG_TEE_STATS
	uint32_t old_itr_status __maybe_unused; 
	uint32_t linear_id = get_core_pos();

	/* Update this cpu's statistics */
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;

#if TRACE_LEVEL >= TRACE_FLOW
	old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&console_lock);
	FMSG("TEE: cpu 0x%lx SYSTEM_OFF request", read_mpidr_el1());
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets requests", read_mpidr_el1(),
	     tee_stats[linear_id].smc_count,
	     tee_stats[linear_id].eret_count);
	cpu_spin_unlock(&console_lock);
	thread_unmask_exceptions(old_itr_status);
#endif
#endif /*CFG_TEE_STATS*/
	return pm_do_nothing(a0, a1);
}

static unsigned long system_reset_main(unsigned long a0, unsigned long a1)
{
#ifdef CFG_TEE_STATS
	uint32_t old_itr_status __maybe_unused; 
	uint32_t linear_id = get_core_pos();

	/* Update this cpu's statistics */
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;

#if TRACE_LEVEL >= TRACE_FLOW
	old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&console_lock);
	FMSG("TEE: cpu 0x%lx SYSTEM_RESET request", read_mpidr_el1());
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets requests", read_mpidr_el1(),
	     tee_stats[linear_id].smc_count,
	     tee_stats[linear_id].eret_count);
	cpu_spin_unlock(&console_lock);
	thread_unmask_exceptions(old_itr_status);
#endif
#endif /*CFG_TEE_STATS*/
	return pm_do_nothing(a0, a1);
}

static vaddr_t console_base(void)
{
	static void *va;

	if (cpu_mmu_enabled()) {
		if (!va)
			va = phys_to_virt(CONSOLE_UART_BASE, MEM_AREA_IO_NSEC);
		return (vaddr_t)va;
	}
	return CONSOLE_UART_BASE;
}

void console_init(void)
{
	/*
	 * do nothing, debug uart(uart0) share with normal world,
	 * everything for uart0 is ready now.
	 */
}

void console_putc(int ch)
{
	vaddr_t base = console_base();

	if (ch == '\n')
		ns16550_putc('\r', base);
	ns16550_putc(ch, base);
}

void console_flush(void)
{
	ns16550_flush(console_base());
}

#ifdef CFG_TEE_STATS
void plat_update_preemption_state(void)
{
	uint32_t old_itr_status __maybe_unused; 
	uint32_t linear_id = get_core_pos();

	tee_stats[linear_id].preempt_intr_count++;
#if TRACE_LEVEL >= TRACE_FLOW
	old_itr_status = thread_mask_exceptions(THREAD_EXCP_ALL);
	cpu_spin_lock(&console_lock);
	FMSG("TEE: cpu 0x%lx: %d preempt interrupt requests",
		read_mpidr_el1(), tee_stats[linear_id].preempt_intr_count);
	cpu_spin_unlock(&console_lock);
	thread_unmask_exceptions(old_itr_status);
#endif
}

void plat_update_rpc_state(void)
{
	uint32_t linear_id = get_core_pos();
	/* Update this cpu's statistics */
	tee_stats[linear_id].rpc_count++;
#if TRACE_LEVEL >= TRACE_FLOW
	FMSG("TEE: cpu 0x%lx: %d rpc request", read_mpidr_el1(),
		tee_stats[linear_id].rpc_count);
#endif
}

void plat_update_std_smc_state(uint32_t fid)
{
	uint32_t linear_id = get_core_pos();
	/* Update this cpu's statistics */
	if (OPTEE_SMC_CALL_RETURN_FROM_RPC == fid)
		tee_stats[linear_id].rpc_ret_count++;
	tee_stats[linear_id].smc_count++;
	tee_stats[linear_id].eret_count++;
#if TRACE_LEVEL >= TRACE_FLOW
	FMSG("TEE: cpu 0x%lx received std smc 0x%x", read_mpidr_el1(), fid);
	FMSG("TEE: cpu 0x%lx: %d smcs, %d erets", read_mpidr_el1(),
		tee_stats[linear_id].smc_count,
		tee_stats[linear_id].eret_count);
	if (OPTEE_SMC_CALL_RETURN_FROM_RPC == fid) {
		FMSG("TEE: cpu 0x%lx: %d rpcs, %d rets", read_mpidr_el1(),
			tee_stats[linear_id].rpc_count,
			tee_stats[linear_id].rpc_ret_count);
	}
#endif
}
#endif /*CFG_TEE_STATS*/
