global-incdirs-y += .
srcs-y += main.c
srcs-$(CFG_ARM64_core) += entry_a64.S
subdirs-y += $(PLATFORM_FLAVOR)
subdirs-y += drivers pta
