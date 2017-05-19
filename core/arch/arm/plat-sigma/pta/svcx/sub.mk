srcs-y += pta_entry.c svcx_common.c
srcs-$(CFG_MMAP_API) += svcx_mmap.c
srcs-$(CFG_OTP_API) += svcx_otp.c
srcs-$(CFG_MEMSTAT_API) += svcx_memstat.c
