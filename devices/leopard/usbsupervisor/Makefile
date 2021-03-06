###################################################################
# Makefile                                                         #
####################################################################

.SUFFIXES:				# ignore builtin rules
.PHONY: all debug release clean

####################################################################
# Definitions                                                      #
####################################################################

# uniq is a function which remove duplicate elements from a list
uniq = $(strip $(if $1,$(firstword $1) \
       $(call uniq,$(filter-out $(firstword $1),$1))))

DEVICE = EFM32LG990F256
PROJECTNAME = usbdcomposite

OBJ_DIR = build
EXE_DIR = exe
LST_DIR = lst

SDK=/Applications/SimplicityStudio_v3/developer/sdks/efm32/v2
CM3LIB=../../../cm3lib
GECKOLIB=../../common
SAPI=../../../sockpuppet/sapi

####################################################################
# Definitions of toolchain.                                        #
# You might need to do changes to match your system setup          #
####################################################################

# Change path to the tools according to your system configuration
# DO NOT add trailing whitespace chars, they do matter !
WINDOWSCS  ?= GNU Tools ARM Embedded\4.9 2015q1
LINUXCS    ?= ~/gcc-arm-none-eabi-4_8-2014q3

RMDIRS     := rm -rf
RMFILES    := rm -rf
ALLFILES   := /*.*
NULLDEVICE := /dev/null
SHELLNAMES := $(ComSpec)$(COMSPEC)

# Try autodetecting the environment
ifeq ($(SHELLNAMES),)
  # Assume we are making on a Linux platform
  TOOLDIR := $(LINUXCS)
else
  QUOTE :="
  ifneq ($(COMSPEC),)
    # Assume we are making on a mingw/msys/cygwin platform running on Windows
    # This is a convenient place to override TOOLDIR, DO NOT add trailing
    # whitespace chars, they do matter !
    TOOLDIR := $(PROGRAMFILES)/$(WINDOWSCS)
    ifeq ($(findstring cygdrive,$(shell set)),)
      # We were not on a cygwin platform
      NULLDEVICE := NUL
    endif
  else
    # Assume we are making on a Windows platform
    # This is a convenient place to override TOOLDIR, DO NOT add trailing
    # whitespace chars, they do matter !
    SHELL      := $(SHELLNAMES)
    TOOLDIR    := $(ProgramFiles)/$(WINDOWSCS)
    RMDIRS     := rd /s /q
    RMFILES    := del /s /q
    ALLFILES   := \*.*
    NULLDEVICE := NUL
  endif
endif

# Create directories and do a clean which is compatible with parallell make
$(shell mkdir $(OBJ_DIR)>$(NULLDEVICE) 2>&1)
$(shell mkdir $(EXE_DIR)>$(NULLDEVICE) 2>&1)
$(shell mkdir $(LST_DIR)>$(NULLDEVICE) 2>&1)
ifeq (clean,$(findstring clean, $(MAKECMDGOALS)))
  ifneq ($(filter $(MAKECMDGOALS),all debug release),)
    $(shell $(RMFILES) $(OBJ_DIR)$(ALLFILES)>$(NULLDEVICE) 2>&1)
    $(shell $(RMFILES) $(EXE_DIR)$(ALLFILES)>$(NULLDEVICE) 2>&1)
    $(shell $(RMFILES) $(LST_DIR)$(ALLFILES)>$(NULLDEVICE) 2>&1)
  endif
endif

CC      = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-gcc$(QUOTE)
LD      = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-ld$(QUOTE)
AR      = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-ar$(QUOTE)
OBJCOPY = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-objcopy$(QUOTE)
DUMP    = $(QUOTE)$(TOOLDIR)/bin/arm-none-eabi-objdump$(QUOTE)

####################################################################
# Flags                                                            #
####################################################################

# -MMD : Don't generate dependencies on system header files.
# -MP  : Add phony targets, useful when a h-file is removed from a project.
# -MF  : Specify a file to write the dependencies to.
DEPFLAGS = -MMD -MP -MF $(@:.o=.d)

#
# Add -Wa,-ahld=$(LST_DIR)/$(@F:.o=.lst) to CFLAGS to produce assembly list files
#
override CFLAGS += -D$(DEVICE) -std=c99 -Wall -Wextra -mcpu=cortex-m3 -mthumb \
-mfix-cortex-m3-ldrd -ffunction-sections \
-fdata-sections -fomit-frame-pointer -DDEBUG_EFM_USER  \
$(DEPFLAGS)

override ASMFLAGS += -x assembler-with-cpp -D$(DEVICE) -Wall -Wextra -mcpu=cortex-m3 -mthumb -DDEBUG_EFM_USER 


#
# NOTE: The -Wl,--gc-sections flag may interfere with debugging using gdb.
#

LDSTITCHFLAGS = -Xlinker -Map=$(LST_DIR)/$(PROJECTNAME).map -mcpu=cortex-m3 \
	        -mthumb -Tefm32lg-package.ld \
		 -Wl,--gc-sections 

override LDFLAGS += -Xlinker -Map=$(LST_DIR)/$(PROJECTNAME).map -mcpu=cortex-m3 \
 -mthumb -Tefm32lg.ld \
 -Wl,--gc-sections 




LIBS = -Wl,--start-group -lgcc -lc -lnosys   -Wl,--end-group


INCLUDEPATHS += \
-I. \
-I.. \
-I$(CM3LIB) \
-I$(GECKOLIB) \
-I$(SDK)/CMSIS/Include \
-I$(SDK)/Device/SiliconLabs/EFM32LG/Include \
-I$(SDK)/emlib/inc \
-I$(SDK)/usb/inc \
-I$(SDK)/kits/common/drivers \
-I$(SDK)/kits/common/bsp \
-I$(SDK)/kits/EFM32LG_STK3600/config \
-I../../../sockpuppet/sapi

####################################################################
# Files                                                            #
####################################################################


C_SRC +=  \
$(SDK)/kits/common/bsp/bsp_bcc.c \
$(SDK)/kits/common/bsp/bsp_stk.c \
$(SDK)/kits/common/bsp/bsp_stk_leds.c \
$(SDK)/kits/common/bsp/bsp_trace.c \
$(SDK)/Device/SiliconLabs/EFM32LG/Source/system_efm32lg.c \
$(SDK)/emlib/src/em_assert.c \
$(SDK)/emlib/src/em_cmu.c \
$(SDK)/emlib/src/em_dma.c \
$(SDK)/emlib/src/em_ebi.c \
$(SDK)/emlib/src/em_emu.c \
$(SDK)/emlib/src/em_gpio.c \
$(SDK)/emlib/src/em_int.c \
$(SDK)/emlib/src/em_lcd.c \
$(SDK)/emlib/src/em_msc.c \
$(SDK)/emlib/src/em_rtc.c \
$(SDK)/emlib/src/em_system.c \
$(SDK)/emlib/src/em_timer.c \
$(SDK)/emlib/src/em_leuart.c \
$(SDK)/usb/src/em_usbd.c \
$(SDK)/usb/src/em_usbdch9.c \
$(SDK)/usb/src/em_usbhal.c \
$(SDK)/usb/src/em_usbdep.c \
$(SDK)/usb/src/em_usbdint.c \
$(SDK)/usb/src/em_usbtimer.c \
./main.c \
./descriptors.c \
./faux-cdc.c \
$(GECKOLIB)/console_leuart.c \
$(SDK)/kits/common/drivers/dmactrl.c \
$(SDK)/kits/common/drivers/segmentlcd.c \
$(CM3LIB)/ringbuffer.c \
$(SAPI)/pendsv-launcher.c \
./systemcalls.c 


s_SRC += 

S_SRC += ${SAPI}/svchandler.S \
	 startup_efm32lg.S \
	testsyscall.S

####################################################################
# Rules                                                            #
####################################################################

C_FILES = $(notdir $(C_SRC) )
S_FILES = $(notdir $(S_SRC) $(s_SRC) )
#make list of source paths, uniq removes duplicate paths
C_PATHS = $(call uniq, $(dir $(C_SRC) ) )
S_PATHS = $(call uniq, $(dir $(S_SRC) $(s_SRC) ) )

C_OBJS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.o))
S_OBJS = $(if $(S_SRC), $(addprefix $(OBJ_DIR)/, $(S_FILES:.S=.o)))
s_OBJS = $(if $(s_SRC), $(addprefix $(OBJ_DIR)/, $(S_FILES:.s=.o)))
C_DEPS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.d))
OBJS = $(C_OBJS) $(S_OBJS) $(s_OBJS)

vpath %.c $(C_PATHS)
vpath %.s $(S_PATHS)
vpath %.S $(S_PATHS)

# Default build is debug build
all:      debug

debug:    CFLAGS += -DDEBUG -O2 -g
debug:    $(EXE_DIR)/$(PROJECTNAME).bin

release:  CFLAGS += -DNDEBUG -O0 -g 
release:  $(EXE_DIR)/$(PROJECTNAME).bin

# Create objects from C SRC files
$(OBJ_DIR)/%.o: %.c
	@echo "Building file: $<"
	$(CC) $(CFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Assemble .s/.S files
$(OBJ_DIR)/%.o: %.s
	@echo "Assembling $<"
	$(CC) $(ASMFLAGS) $(INCLUDEPATHS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.S
	@echo "Assembling $<"
	$(CC) $(ASMFLAGS) $(INCLUDEPATHS) -c -o $@ $<


forth.o: ../usbforth/LEOPARD.img
	arm-none-eabi-objcopy -O elf32-littlearm \
		-B arm --rename-section\
		.data=.forth -I binary ../usbforth/LEOPARD.img\
		forth.o

# Link
$(EXE_DIR)/$(PROJECTNAME).out: $(OBJS)
	@echo "Linking target: $@"
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $(EXE_DIR)/$(PROJECTNAME).out
	$(CC) $(LDSTITCHFLAGS) $(OBJS) forth.o $(LIBS) -o $(EXE_DIR)/$(PROJECTNAME)-package.out

# Create binary file
$(EXE_DIR)/$(PROJECTNAME).bin: $(EXE_DIR)/$(PROJECTNAME).out
	@echo "Creating binary file"
	$(OBJCOPY) -O ihex $(EXE_DIR)/$(PROJECTNAME).out $(EXE_DIR)/$(PROJECTNAME).hex
	$(OBJCOPY) -O binary $(EXE_DIR)/$(PROJECTNAME).out $(EXE_DIR)/$(PROJECTNAME).bin
# Uncomment next line to produce assembly listing of entire program
	$(DUMP) -h -S -C $(EXE_DIR)/$(PROJECTNAME).out>$(LST_DIR)/$(PROJECTNAME)out.lst

clean:
ifeq ($(filter $(MAKECMDGOALS),all debug release),)
	$(RMDIRS) $(OBJ_DIR) $(LST_DIR) $(EXE_DIR)
endif

# include auto-generated dependency files (explicit rules)
ifneq (clean,$(findstring clean, $(MAKECMDGOALS)))
-include $(C_DEPS)
endif
