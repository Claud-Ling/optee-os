PLATFORM_FLAVOR ?= union

# 32-bit flags
arm32-platform-cpuarch		:= cortex-a53
arm32-platform-cflags		+= -mcpu=$(arm32-platform-cpuarch)
arm32-platform-aflags		+= -mcpu=$(arm32-platform-cpuarch)
core_arm32-platform-aflags	+= -mfpu=neon

$(call force,CFG_GENERIC_BOOT,y)
$(call force,CFG_HWSUPP_MEM_PERM_PXN,y)
$(call force,CFG_16550_UART,y)
$(call force,CFG_PM_STUBS,y)
$(call force,CFG_SECURE_TIME_SOURCE_CNTPCT,y)
$(call force,CFG_WITH_ARM_TRUSTED_FW,y)
$(call force,CFG_CRYPTO_WITH_CE,n)
$(call force,CFG_TEE_NUM_UMAPS,8)
$(call force,CFG_MMAP_API,y)
$(call force,CFG_CACHE_API,y)
$(call force,CFG_OTP_API,y)
$(call force,CFG_MEMSTAT_API,y)

# Local tee internal API extensions
$(call force,CFG_HAVE_LIBUTEEX,y)
CFG_LIBUTEEX_DIR	:= $(platform-dir)/libuteex

ta-targets = ta_arm32

ifeq ($(CFG_ARM64_core),y)
$(call force,CFG_WITH_LPAE,y)
ta-targets += ta_arm64
else
$(call force,CFG_ARM32_core,y)
endif

CFG_TEE_CORE_EMBED_INTERNAL_TESTS ?= y
CFG_TEE_FS_KEY_MANAGER_TEST ?= y
CFG_WITH_STACK_CANARIES ?= y
CFG_WITH_SOFTWARE_PRNG ?= n

# Debug Options
#CFG_TEE_CORE_DEBUG ?= y
#CFG_TEE_CORE_LOG_LEVEL ?= 4
#CFG_TEE_TA_LOG_LEVEL ?= 4

include $(platform-dir)/$(PLATFORM_FLAVOR)/conf.mk
