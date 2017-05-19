global-incdirs-$(CFG_HAVE_LIBUTEEX) += include
ifeq ($(CFG_WITH_USER_TA),y)
srcs-$(CFG_HAVE_LIBUTEEX) += tee_api_ext.c
endif
