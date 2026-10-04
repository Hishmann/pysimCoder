all: dependencies allfiles lib install
reduced: files lib install

LIB = libpyblk.a
LIBDIR=$(CURDIR)/../lib
GENERIC_INC = $(PYSUPSICTRL)/CodeGen/Common/include
TARGET_INC = ../include
INCLUDE =  -I$(GENERIC_INC) -I$(TARGET_INC)

COMMON_DIR = $(PYSUPSICTRL)/CodeGen/Common
EXT_LIBS = $(PYSUPSICTRL)/ExtLibs
GENERATED_INC = $(CURDIR)/../include-generated

OMK_COMPILED_INC_PATH = $(GENERATED_INC)
OMK_COMPILED_LIB_PATH = $(LIBDIR)


# RTEMS

ifndef RTEMS_MAKEFILE_PATH
$(warning Specify RTEMS_MAKEFILE_PATH)
$(warning make RTEMS_MAKEFILE_PATH=/path/to/rtems/BSP/directory)
$(error RTEMS_MAKEFILE_PATH not defined, cannot continue)
endif

ifeq ($(wildcard $(RTEMS_MAKEFILE_PATH)/Makefile.inc),)
$(error RTEMS Makefile.inc not found at RTEMS_MAKEFILE_PATH)
endif

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc
include $(RTEMS_CUSTOM)
include $(CONFIG.CC)


# RTEMS BSP variables

BSP_NAME := $(notdir $(patsubst %/,%,$(RTEMS_MAKEFILE_PATH)))
BSP_FOLDER = $(PYSUPSICTRL)/CodeGen/rtems/bsp
BSP_BUILD_OUTPUT = $(BSP_FOLDER)/$(BSP_NAME)/build
BSP_LIB_OUTPUT = $(BSP_FOLDER)/$(BSP_NAME)/lib
BSP_GENERATED_INC = $(BSP_FOLDER)/$(BSP_NAME)/include-generated

bsp_folder:
	@mkdir -p $(BSP_GENERATED_INC)
	@mkdir -p $(BSP_BUILD_OUTPUT)
	@mkdir -p $(BSP_LIB_OUTPUT)

dependencies: bsp_folder


# Include, Source files, Flags

INCLUDE +=  -I$(GENERIC_INC) -I$(TARGET_INC)

SRCALL = $(wildcard $(COMMON_DIR)/common_dev/*.c)
SRCALL += $(wildcard $(COMMON_DIR)/posix/*.c)
SRCALL += $(wildcard $(COMMON_DIR)/can/*.c)

ifneq ($(wildcard $(RTEMS_ROOT)/$(RTEMS_TARGET)/$(RTEMS_BSP)/lib/include/dev/can/ctucanfd.h),)
SRCALL += can.c
endif

# Test project files
# 	SRCALL += mz_apo_ENC.c mz_apo_LED.c mz_apo_DCmot.c

OBJ = $(notdir $(SRCALL:.c=.o))

BSP_OBJ = $(addprefix $(BSP_BUILD_OUTPUT)/,$(OBJ))
BSP_LIB = $(addprefix $(BSP_LIB_OUTPUT)/,$(LIB))

CWD = $(shell pwd)

OBJEX = $(SRC:.c=.o)
DEFINES = -D PLOTTER_SCRIPT=\"$(CWD)/scope.py\"
CC_FLAGS = -c $(DBG) $(INCLUDE) $(CFLAGS) $(CPU_CFLAGS) $(CPU_DEFINES) $(CFLAGS_OPTIMIZE_V)
CC_FLAGS += -I$(BSP_GENERATED_INC)
ifeq ($(SHV),1)
CC_FLAGS += -I$(COMMON_DIR)/shv/include
endif
CC_FLAGS_SCP = -c $(DBG) $(INCLUDE) $(CFLAGS) $(DEFINES)

EXTLIB_CFLAGS=$(CC_FLAGS)
EXTLIB_CXXFLAGS=$(CC_FLAGS)
EXTLIB_LDFLAGS=$(CC_FLAGS)


# SHV

ifeq ($(SHV),1)
SRCALL += $(wildcard $(COMMON_DIR)/shv/*.c)

CONFIG_SHV_LIBS4C_PLATFORM="rtems"
include $(COMMON_DIR)/rules/make-ulut.inc
include $(COMMON_DIR)/rules/make-shv-libs4c.inc

# Overriding CURDIR leads to OMK_COMPILED_INC_PATH and OMK_COMPILED_LIB_PATH pointing to correct directories in the bsp folder
make-shv-libs4c: CURDIR := $(BSP_BUILD_OUTPUT)
make-ulut: CURDIR := $(BSP_BUILD_OUTPUT)
dependencies: bsp_folder make-ulut make-shv-libs4c
endif


# Build

allfiles:

files:

EXCLUDE = comedi_analog_input.c comedi_analog_output.c comedi_digital_input.c comedi_digital_output.c
SRC=$(filter-out $(EXCLUDE),$(SRCALL))

scope.o: scope.c
	$(CC) $(CC_FLAGS_SCP) $<

$(BSP_BUILD_OUTPUT)/%.o: %.c
	$(CC) $(CC_FLAGS) $< -o $@

$(BSP_BUILD_OUTPUT)/%.o: sensors/%.c
	$(CC) $(CC_FLAGS) $< -o $@

ifeq ($(SHV),1)
$(BSP_BUILD_OUTPUT)/%.o: $(COMMON_DIR)/shv/%.c
	$(CC) $(CC_FLAGS) $< -o $@
endif

$(BSP_BUILD_OUTPUT)/%.o: $(COMMON_DIR)/shv/%.c
	$(CC) $(CC_FLAGS) $< -o $@

$(BSP_BUILD_OUTPUT)/%.o: $(COMMON_DIR)/CAN_dev/%.c
	$(CC) $(CC_FLAGS) $< -o $@

$(BSP_BUILD_OUTPUT)/%.o: $(COMMON_DIR)/common_dev/%.c
	$(CC) $(CC_FLAGS) $< -o $@

$(BSP_BUILD_OUTPUT)/%.o: $(COMMON_DIR)/Faulhaber_dev/%.c
	$(CC) $(CC_FLAGS) $< -o $@

$(BSP_BUILD_OUTPUT)/%.o: $(COMMON_DIR)/Maxon_dev/%.c
	$(CC) $(CC_FLAGS) $< -o $@

$(BSP_BUILD_OUTPUT)/%.o: $(COMMON_DIR)/posix/%.c
	$(CC) $(CC_FLAGS) $< -o $@

$(BSP_BUILD_OUTPUT)/%.o: $(COMMON_DIR)/can/%.c
	$(CC) $(CC_FLAGS) $< -o $@

lib: $(BSP_OBJ)
	$(AR) r $(BSP_LIB) $(BSP_OBJ)

# Prev version
install:
# 	mkdir -p ../lib
# 	cp $(BSP_LIB) ../lib

clean:
	@echo "RM: lib/*.a bsp"
	@rm -rf $(LIBDIR)/*.a $(BSP_FOLDER)
