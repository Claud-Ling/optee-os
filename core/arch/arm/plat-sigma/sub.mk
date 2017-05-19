global-incdirs-y += .
srcs-y += main.c
srcs-$(CFG_ARM64_core) += entry_a64.S
subdirs-y += $(PLATFORM_FLAVOR)
subdirs-y += drivers pta

#
# Hack to generate platform specific asm-defines.h
# Not include <mk/compile.mk> here to avoid double process_srcs
asm-defines-file := $(platform-dir)/plat-asm-defines.c
h-file-$(asm-defines-file) := $(out-dir)/$(sm)/include/generated/$(basename $(notdir $(asm-defines-file))).h
$(eval $(call gen-asm-defines-file,$(asm-defines-file),$(h-file-$(asm-defines-file))))
asm-defines-file :=
