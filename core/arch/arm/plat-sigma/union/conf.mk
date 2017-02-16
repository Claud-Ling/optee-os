# Number of cores
$(call force,CFG_TEE_CORE_NB_CORE,2)
CFG_NUM_THREADS ?= CFG_TEE_CORE_NB_CORE

# ARM GIC ARCH Version
$(call force,CFG_GIC_V3,y)

#CNTPS TIMER
#$(call force,CFG_CNTPS_TIMER,y)
# fire every 0.5 second
CFG_CNTPS_HZ	?= 2

# TEE statistics
CFG_TEE_STATS	?= y
