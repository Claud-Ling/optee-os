
#ifndef __SD_GIC_V3_HELPERS_A64_H__
#define __SD_GIC_V3_HELPERS_A64_H__

#include <arm64.h>

/*******************************************************************************
 * Definitions for CPU system register interface to GICv3
 ******************************************************************************/
#define ICC_SRE_EL1     S3_0_C12_C12_5
#define ICC_SRE_EL2     S3_4_C12_C9_5
#define ICC_SRE_EL3     S3_6_C12_C12_5
#define ICC_CTLR_EL1    S3_0_C12_C12_4
#define ICC_CTLR_EL3    S3_6_C12_C12_4
#define ICC_PMR_EL1     S3_0_C4_C6_0
#define ICC_IGRPEN1_EL3 S3_6_c12_c12_7
#define ICC_IGRPEN0_EL1 S3_0_c12_c12_6
#define ICC_HPPIR0_EL1  S3_0_c12_c8_2
#define ICC_HPPIR1_EL1  S3_0_c12_c12_2
#define ICC_IAR0_EL1    S3_0_c12_c8_0
#define ICC_IAR1_EL1    S3_0_c12_c12_0
#define ICC_EOIR0_EL1   S3_0_c12_c8_1
#define ICC_EOIR1_EL1   S3_0_c12_c12_1
#define ICC_SGI1R_EL1   S3_0_c12_c11_5
#define ICC_SGI0R_EL1   S3_0_c12_c11_7
#define ICC_ASGI1R_EL1  S3_0_c12_c11_6

#ifndef ASM

/* Define read function for renamed system register */
#define DEFINE_U32_RENAME_REG_READ_FUNC(_name, _reg_name)	\
	DEFINE_REG_READ_FUNC_(_name, uint32_t, _reg_name)
/* Define write function for renamed system register */
#define DEFINE_U32_RENAME_REG_WRITE_FUNC(_name, _reg_name)	\
	DEFINE_REG_WRITE_FUNC_(_name, uint32_t, _reg_name)
/* Define read & write function for renamed system register */
#define DEFINE_U32_RENAME_REG_RW_FUNC(_name, _reg_name)	\
	DEFINE_U32_RENAME_REG_READ_FUNC(_name, _reg_name)	\
	DEFINE_U32_RENAME_REG_WRITE_FUNC(_name, _reg_name)

DEFINE_U32_RENAME_REG_RW_FUNC(icc_sre_el1, ICC_SRE_EL1)
DEFINE_U32_RENAME_REG_RW_FUNC(icc_sre_el2, ICC_SRE_EL2)
DEFINE_U32_RENAME_REG_RW_FUNC(icc_sre_el3, ICC_SRE_EL3)
DEFINE_U32_RENAME_REG_RW_FUNC(icc_pmr_el1, ICC_PMR_EL1)
DEFINE_U32_RENAME_REG_RW_FUNC(icc_igrpen1_el3, ICC_IGRPEN1_EL3)
DEFINE_U32_RENAME_REG_RW_FUNC(icc_igrpen0_el1, ICC_IGRPEN0_EL1)
DEFINE_U32_RENAME_REG_READ_FUNC(icc_hppir0_el1, ICC_HPPIR0_EL1)
DEFINE_U32_RENAME_REG_READ_FUNC(icc_hppir1_el1, ICC_HPPIR1_EL1)
DEFINE_U32_RENAME_REG_READ_FUNC(icc_iar0_el1, ICC_IAR0_EL1)
DEFINE_U32_RENAME_REG_READ_FUNC(icc_iar1_el1, ICC_IAR1_EL1)
DEFINE_U32_RENAME_REG_WRITE_FUNC(icc_eoir0_el1, ICC_EOIR0_EL1)
DEFINE_U32_RENAME_REG_WRITE_FUNC(icc_eoir1_el1, ICC_EOIR1_EL1)
DEFINE_U32_RENAME_REG_WRITE_FUNC(icc_sgi1r_el1, ICC_SGI1R_EL1)
DEFINE_U32_RENAME_REG_WRITE_FUNC(icc_asgi1r_el1, ICC_ASGI1R_EL1)
DEFINE_U32_RENAME_REG_WRITE_FUNC(icc_sgi0r_el1, ICC_SGI0R_EL1)

#endif /*!ASM*/
#endif /*__SD_GIC_V3_HELPERS_A64_H__*/
