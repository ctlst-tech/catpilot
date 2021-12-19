#######################################################################
# Makefile for STM32 board projects

OUTPATH = build
PROJECT = $(OUTPATH)/ctlst-fmuv5
MCU = stm32f765ii
OPENOCD_SCRIPT_DIR ?= /usr/share/openocd/scripts
HEAP_SIZE = 0x400

DIR_MCU = mcu/$(MCU)
DIR_PERIPH = periph
DIR_DRV = drv

################
# Sources

SOURCES_S = $(wildcard mcu/$(MCU)/core/*.s)

SOURCES_C_MCU = $(wildcard $(DIR_MCU)/core/*.c)
SOURCES_C_MCU += $(wildcard $(DIR_MCU)/hal/src/*.c)

SOURCES_C_PERIPH += $(wildcard $(DIR_PERIPH)/rcc/*.c)
SOURCES_C_PERIPH += $(wildcard $(DIR_PERIPH)/gpio/*.c)
SOURCES_C_PERIPH += $(wildcard $(DIR_PERIPH)/usart/*.c)
SOURCES_C_PERIPH += $(wildcard $(DIR_PERIPH)/spi/*.c)
SOURCES_C_PERIPH += $(wildcard $(DIR_PERIPH)/i2c/*.c)
SOURCES_C_PERIPH += $(wildcard $(DIR_PERIPH)/adc/*.c)

SOURCES_C_DRV += $(wildcard $(DIR_DRV)/cli/*.c)

SOURCES_C_RTOS = $(wildcard freertos/core/src/*.c)
SOURCES_C_RTOS += $(wildcard freertos/port/$(MCU)/*.c)
SOURCES_C_HEAP = freertos/MemMang/heap_1.c

SOURCES_C += $(wildcard *.c)
SOURCES_C += $(SOURCES_C_MCU)
SOURCES_C += $(SOURCES_C_PERIPH)
SOURCES_C += $(SOURCES_C_DRV)
SOURCES_C += $(SOURCES_C_RTOS)
SOURCES_C += $(SOURCES_C_HEAP)

SOURCES = $(SOURCES_S) $(SOURCES_C)
OBJS = $(SOURCES_S:.s=.o) $(SOURCES_C:.c=.o)

# Includes and Defines

INC_MCU = -I$(DIR_MCU)/core -I$(DIR_MCU)/hal/inc 

INC_PERIPH += -I$(DIR_PERIPH)/rcc
INC_PERIPH += -I$(DIR_PERIPH)/gpio
INC_PERIPH += -I$(DIR_PERIPH)/usart
INC_PERIPH += -I$(DIR_PERIPH)/spi
INC_PERIPH += -I$(DIR_PERIPH)/i2c
INC_PERIPH += -I$(DIR_PERIPH)/adc

INC_DRV += -I$(DIR_DRV)/cli

INC_RTOS = -Ifreertos/core/inc -Ifreertos/port/$(MCU)
INC_CONF = -Iconf

INCLUDES += $(INC_MCU)
INCLUDES += $(INC_PERIPH)
INCLUDES += $(INC_DRV)
INCLUDES += $(INC_RTOS)
INCLUDES += $(INC_CONF)

DEFINES = -DSTM32 -DSTM32F7 -DHEAP_SIZE=$(HEAP_SIZE)

# Compiler/Assembler/Linker/etc

PREFIX = arm-none-eabi

CC = $(PREFIX)-gcc
CXX = $(PREFIX)-g++
AS = $(PREFIX)-as
AR = $(PREFIX)-ar
LD = $(PREFIX)-gcc
NM = $(PREFIX)-nm
OBJCOPY = $(PREFIX)-objcopy
OBJDUMP = $(PREFIX)-objdump
READELF = $(PREFIX)-readelf
SIZE = $(PREFIX)-size
GDB = $(PREFIX)-gdb
RM = rm -f
OPENOCD = openocd

# Compiler options

MCUFLAGS = -mcpu=cortex-m7 -mlittle-endian -mfloat-abi=hard -mthumb \
           -mno-unaligned-access

DEBUG_OPTIMIZE_FLAGS = -O0 -g3

CFLAGS = -Wall -Wextra
CFLAGS_EXTRA = -nostartfiles -nodefaultlibs -nostdlib \
               -fdata-sections -ffunction-sections

CFLAGS += $(DEFINES) $(MCUFLAGS) $(DEBUG_OPTIMIZE_FLAGS) $(CFLAGS_EXTRA) $(INCLUDES)

LDFLAGS = -specs=nano.specs -specs=nosys.specs $(MCUFLAGS) -Wl,--start-group -lgcc -lc -lg -Wl,--end-group \
          -Wl,--gc-sections -T mcu/$(MCU)/core/STM32F765_FLASH.ld

.PHONY: dirs all clean flash erase

all: dirs $(PROJECT).bin $(PROJECT).asm

dirs: ${OUTPATH}

${OUTPATH}:
	mkdir -p ${OUTPATH}

clean:
	$(RM) $(OBJS) $(PROJECT).elf $(PROJECT).bin $(PROJECT).asm
	rm -rf ${OUTPATH}

# Hardware specific

flash: $(PROJECT).bin
	st-flash write $(PROJECT).bin 0x08000000

erase:
	st-flash erase

gdb-server-ocd:
	$(OPENOCD) -f $(OPENOCD_SCRIPT_DIR)/board/stm32f7discovery.cfg

gdb-server-st:
	st-util

OPENOCD_P=3333
gdb-openocd: $(PROJECT).elf
	$(GDB) --eval-command="target extended-remote localhost:$(OPENOCD_P)" \
           --eval-command="load" $(PROJECT).elf

GDB_P=4242
gdb-st-util: $(PROJECT).elf
	$(GDB) --eval-command="target extended-remote localhost:$(GDB_P)" \
           --eval-command="load" $(PROJECT).elf

$(PROJECT).elf: $(OBJS)

%.elf:
	$(LD) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) -A $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.asm: %.elf
	$(OBJDUMP) -dwh $< > $@
