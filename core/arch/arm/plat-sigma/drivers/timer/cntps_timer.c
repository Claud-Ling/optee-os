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
 * Date:    2017/02/08
 *
 */
#include <arm.h>
#include <stdint.h>
#include <assert.h>
#include <trace.h>
#include <kernel/tz_ssvce.h>	/*flush_dcache_range*/
#include <kernel/misc.h>	/*get_core_pos*/
#include <platform_config.h>
#include <utee_defines.h>
#include <cntps_timer.h>
#include <keep.h> 	/*KEEP_PAGER*/
#include <initcall.h>
#include <kernel/interrupt.h>

/*
 * Define register access functions
 */
DEFINE_U32_REG_READWRITE_FUNCS(cntps_ctl_el1)
DEFINE_U32_REG_READWRITE_FUNCS(cntps_cval_el1)
DEFINE_U32_REG_READWRITE_FUNCS(cntps_tval_el1)

/* Physical timer control register bit fields shifts and masks */
#define CNTP_CTL_ENABLE_SHIFT   0
#define CNTP_CTL_IMASK_SHIFT    1
#define CNTP_CTL_ISTATUS_SHIFT  2

#define CNTP_CTL_ENABLE_MASK    1
#define CNTP_CTL_IMASK_MASK     1
#define CNTP_CTL_ISTATUS_MASK   1

#define get_cntp_ctl_enable(x)  ((x >> CNTP_CTL_ENABLE_SHIFT) & \
					CNTP_CTL_ENABLE_MASK)
#define get_cntp_ctl_imask(x)   ((x >> CNTP_CTL_IMASK_SHIFT) & \
					CNTP_CTL_IMASK_MASK)
#define get_cntp_ctl_istatus(x) ((x >> CNTP_CTL_ISTATUS_SHIFT) & \
					CNTP_CTL_ISTATUS_MASK)

#define set_cntp_ctl_enable(x)  (x |= 1 << CNTP_CTL_ENABLE_SHIFT)
#define set_cntp_ctl_imask(x)   (x |= 1 << CNTP_CTL_IMASK_SHIFT)

#define clr_cntp_ctl_enable(x)  (x &= ~(1 << CNTP_CTL_ENABLE_SHIFT))
#define clr_cntp_ctl_imask(x)   (x &= ~(1 << CNTP_CTL_IMASK_SHIFT))

/*******************************************************************************
 * Data structure to keep track of per-cpu secure generic timer context across
 * power management operations.
 ******************************************************************************/
typedef struct timer_context {
	uint64_t cval;
	uint32_t ctl;
	uint64_t pct_res;	/*count residual*/
	uint64_t pct_res_old;	/*backed count residual*/
} timer_context_t;

static timer_context_t pcpu_timer_context[CFG_TEE_CORE_NB_CORE];

/*******************************************************************************
 * This function initializes the generic timer to fire every 0.5 second
 ******************************************************************************/
void cntps_timer_start(void)
{
	uint64_t cval;
	uint32_t ctl = 0;

	/* The timer will fire every 0.5 second */
	cval = read_cntpct() + (read_cntfrq() / CFG_CNTPS_HZ);
	write_cntps_cval_el1(cval);

	/* Enable the secure physical timer */
	set_cntp_ctl_enable(ctl);
	write_cntps_ctl_el1(ctl);
}

/*******************************************************************************
 * This function deasserts the timer interrupt and sets it up again
 ******************************************************************************/
void cntps_timer_handler(void)
{
	/* Ensure that the timer did assert the interrupt */
	assert(get_cntp_ctl_istatus(read_cntps_ctl_el1()));

	/*
	 * Disable the timer and reprogram it. The barriers ensure that there is
	 * no reordering of instructions around the reprogramming code.
	 */
	isb();
	write_cntps_ctl_el1(0);
	cntps_timer_start();
	isb();
}

/*******************************************************************************
 * This function deasserts the timer interrupt prior to cpu power down
 ******************************************************************************/
void cntps_timer_stop(void)
{
	/* Disable the timer */
	write_cntps_ctl_el1(0);
}

/*******************************************************************************
 * This function saves the timer context prior to cpu suspension
 ******************************************************************************/
void cntps_timer_save(void)
{
	uint32_t linear_id = get_core_pos();

	pcpu_timer_context[linear_id].cval = read_cntps_cval_el1();
	pcpu_timer_context[linear_id].ctl = read_cntps_ctl_el1();
	pcpu_timer_context[linear_id].pct_res_old = pcpu_timer_context[linear_id].pct_res;
	pcpu_timer_context[linear_id].pct_res = read_cntpct();
	flush_dcache_range((uint64_t) &pcpu_timer_context[linear_id],
			   sizeof(pcpu_timer_context[linear_id]));
}

/*******************************************************************************
 * This function restores the timer context post cpu resummption
 ******************************************************************************/
void cntps_timer_restore(void)
{
	uint32_t linear_id = get_core_pos();

	write_cntps_cval_el1(pcpu_timer_context[linear_id].cval);
	write_cntps_ctl_el1(pcpu_timer_context[linear_id].ctl);

	/*
	 * keep counting in case recover from suspend failure.
	 * for system counter hasn't experienced reset yet.*/
	if (read_cntpct() > pcpu_timer_context[linear_id].pct_res) {
		pcpu_timer_context[linear_id].pct_res = pcpu_timer_context[linear_id].pct_res_old;
	}
}

/*******************************************************************************
 * This function returns the lapsed time since last power on
 * It already takes over suspend/resume
 ******************************************************************************/
TEE_Result cntpct_timer_get_sys_time(TEE_Time *time)
{
	uint64_t cntpct = read_cntpct();
	uint32_t cntfrq = read_cntfrq();
	uint64_t pct_res;

	pct_res = pcpu_timer_context[get_core_pos()].pct_res;
	time->seconds = (cntpct + pct_res) / cntfrq;
	time->millis = ((cntpct + pct_res) % cntfrq) / (cntfrq / TEE_TIME_MILLIS_BASE);

	return TEE_SUCCESS;
}

/*******************************************************************************
 * Interrupt handler
 ******************************************************************************/
static enum itr_return cntps_timer_itr_cb(struct itr_handler *h __maybe_unused)
{
	cntps_timer_handler();
	return ITRR_HANDLED;
}

static struct itr_handler cntps_timer_itr = {
	.it = ARM_IRQ_SEC_PHY_TIMER,
	.flags = ITRF_TRIGGER_LEVEL,
	.handler = cntps_timer_itr_cb,
};
KEEP_PAGER(cntps_timer_itr);

static TEE_Result cntps_timer_init(void)
{
	itr_add(&cntps_timer_itr);
	itr_enable(ARM_IRQ_SEC_PHY_TIMER);
	cntps_timer_start();	/* start timer for the primary core */
	return TEE_SUCCESS;
}
driver_init(cntps_timer_init);

