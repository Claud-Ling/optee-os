/*
 * Auto generated from ../../../union/res/turing_reg.td
 * DO NOT EDIT!
 *
 * Must include stdint.h.
 * The most important part of the file is the MACROs: TURING_OFS_XX
 * and struct describing the turing registers file.
 *
 */

#ifndef __TURING_REG_H__
#define __TURING_REG_H__

#define TURING_OFS_RANDOM_NUMBER_0	0x7004

#ifndef ASM

/* Turing Register File */
struct turing_reg {
	volatile uint32_t hole0[7169];
	volatile uint32_t random_number_0;	/* +0x7004 #RANDOM_NUMBER_0 */
};

#endif /* !ASM */
#endif /* __TURING_REG_H__ */
