global-incdirs-y += include
subdirs-$(CFG_GIC_V3) += gic
subdirs-y += timer
subdirs-y += smccc fw otp
subdirs-y += turing
ifeq ($(CFG_WITH_SOFTWARE_PRNG),n)
subdirs-y += rng
endif
