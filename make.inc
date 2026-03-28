ifeq ($(ewokos),)
ewokos=../ewokos
endif

ifeq ($(ARCH),)
export ARCH=aarch64
endif

ifeq ($(HW),)
export HW=virt
endif

EWOKOS_ROOT_DIR = $(SW_ROOT_DIR)/$(ewokos)
SYS_ROOT_DIR = $(EWOKOS_ROOT_DIR)/system

SDK_DIR= $(SYS_ROOT_DIR)/build/$(HW)

include $(EWOKOS_ROOT_DIR)/system/platform/$(ARCH)/make.rule

LDFLAGS = -L$(SDK_DIR)/lib
CFLAGS += -I include -I $(SDK_DIR)/include
CXXFLAGS += -I include -I $(SDK_DIR)/include
