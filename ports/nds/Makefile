include ../../py/mkenv.mk

CROSS = 1

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# MicroPython feature configurations
MICROPY_ROM_TEXT_COMPRESSION ?= 1

# include py core make definitions
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk


CROSS_COMPILE ?= /opt/wonderful/toolchain/gcc-arm-none-eabi/bin/arm-none-eabi-


DFU = $(TOP)/tools/dfu.py
PYDFU = $(TOP)/tools/pydfu.py

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)
INC += -I/opt/wonderful/thirdparty/blocksds/core/libs/libnds/include
#INC += -I/opt/devkitpro/portlibs/nds/include
#INC += -I/opt/devkitpro/portlibs/armv5te/include

CFLAGS_DSCPU = -march=armv5te -mtune=arm946e-s -mthumb -mthumb-interwork
CFLAGS += $(INC) -Wall -Werror -std=gnu99 $(CFLAGS_DSCPU) -D__NDS__ -DARM9 -fno-exceptions -ffunction-sections # -fdata-sections -fomit-frame-pointer
#LDFLAGS += -nostdlib -T stm32f405.ld -Map=$@.map --cref --gc-sections


CSUPEROPT = -Os # save some code space

# Tune for Debugging or Optimization
CFLAGS += -g  # always include debug info in the ELF
ifeq ($(DEBUG), 1)
CFLAGS += -O0
else
CFLAGS += -Os -DNDEBUG
CFLAGS += -fdata-sections -ffunction-sections
endif

# Flags for optional C++ source code
CXXFLAGS += $(filter-out -std=gnu99,$(CFLAGS))

LD = $(CC)
LDFLAGS = /opt/wonderful/thirdparty/blocksds/core/sys/crts/ds_arm9_crt0.o -g -marm -mthumb-interwork -march=armv5te -mtune=arm946e-s -Wl,-Map,build/firmware.map
LIBS = -nostdlib -L/opt/wonderful/thirdparty/blocksds/core/libs/libnds/lib -T/opt/wonderful/thirdparty/blocksds/core/sys/crts/ds_arm9.mem -T/opt/wonderful/thirdparty/blocksds/core/sys/crts/ds_arm9.ld -Wl,--no-warn-rwx-segments -Wl,--start-group -lnds9 -lc -lgcc -Wl,--end-group

SRC_C = \
	main.c \
	modutime.c \
	uart_core.c \
	shared/libc/printf.c \
	shared/readline/readline.c \
	shared/runtime/pyexec.c \
	shared/runtime/stdout_helpers.c \
	shared/timeutils/timeutils.c \
	$(BUILD)/_frozen_mpy.c \


SRC_C += shared/libc/string0.c
SRC_C += modules/modnds.c modules/modos.c nitrofile.c


SRC_QSTR += shared/readline/readline.c shared/runtime/pyexec.c modules/modnds.c modules/modos.c nitrofile.c

#OBJ += $(PY_CORE_O)
OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_CXX:.cpp=.o))


all: $(BUILD)/firmware.nds


$(BUILD)/firmware.nds: $(BUILD)/firmware.elf
	/opt/wonderful/thirdparty/blocksds/core/tools/ndstool/ndstool -9 ./build/firmware.elf -7 /opt/wonderful/thirdparty/blocksds/core/sys/default_arm7/arm7.elf -c ./build/firmware.nds -d ./nitrofs

$(BUILD)/_frozen_mpy.c: $(TOP)/tests/frozen/frozentest.mpy $(BUILD)/genhdr/qstrdefs.generated.h
	$(ECHO) "MISC freezing bytecode"
	$(Q)$(TOP)/tools/mpy-tool.py -f -q $(BUILD)/genhdr/qstrdefs.preprocessed.h -mlongint-impl=none $< > $@

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(CC) $(LDFLAGS) $(OBJ) -o $(BUILD)/firmware.elf $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(Q)$(OBJCOPY) -O binary -j .isr_vector -j .text -j .data $^ $(BUILD)/firmware.bin

$(BUILD)/firmware.dfu: $(BUILD)/firmware.bin
	$(ECHO) "Create $@"
	$(Q)$(PYTHON) $(DFU) -b 0x08000000:$(BUILD)/firmware.bin $@

deploy: $(BUILD)/firmware.dfu
	$(ECHO) "Writing $< to the board"
	$(Q)$(PYTHON) $(PYDFU) -u $<

# Run emulation build on a POSIX system with suitable terminal settings
run:
	@saved_=`stty -g`; \
	  stty raw opost -echo; \
	  $(BUILD)/firmware.elf; \
	  echo "Exit status: $$?"; \
	  stty $$saved_

test: $(BUILD)/firmware.elf
	$(Q)/bin/echo -e "print('hello world!', list(x+1 for x in range(10)), end='eol\\\\n')\\r\\n\\x04" | $(BUILD)/firmware.elf | tail -n2 | grep "^hello world! \\[1, 2, 3, 4, 5, 6, 7, 8, 9, 10\\]eol"

include $(TOP)/py/mkrules.mk
