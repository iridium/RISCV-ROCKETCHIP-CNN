#=======================================================================
# Makefile for all KC705 examples
#-----------------------------------------------------------------------
# See LICENSE for license details.

# check RISCV environment variable
ifndef RISCV
$(error Please set environment variable RISCV. Please take a look at README)
endif

#--------------------------------------------------------------------
# Build rules
#--------------------------------------------------------------------

FPGA_DIR ?= $(TOP)/vsim
BASE_DIR ?= .

DRIVER_DIR = $(BASE_DIR)/../driver
RISCV_PREFIX=riscv64-unknown-elf-
RISCV_GCC = $(RISCV_PREFIX)gcc
RISCV_DUMP = $(RISCV_PREFIX)objdump
RISCV_GCC_OPTS = -static -std=gnu99 -g -ggdb -Os -Og -ffast-math -fno-common -fno-builtin-printf -mcmodel=medany -I$(DRIVER_DIR) -I$(BASE_DIR)
# RISCV_GCC_OPTS = -march=rv64imafdc -mabi=lp64d -mcmodel=medany -ffreestanding -static -nostdlib -nostartfiles -std=gnu99 -g -ggdb -Os -Og \
# 				 -I$(DRIVER_DIR) -I$(BASE_DIR)
RISCV_DUMP_OPTS = -D -S -l
RISCV_LINK = $(RISCV_PREFIX)gcc -T $(DRIVER_DIR)/test.ld 
RISCV_LINK_OPTS = -nostdlib -nostartfiles -Xlinker -M=mapfile

JPG_LIB_DIR = $(DRIVER_DIR)/jpeg-6b
# -march=rv64imafd

#--------------------------------------------------------------------
# Objects
#--------------------------------------------------------------------

default: demo
demo: generate_mmc demo_date2020.riscv

TARGETS = read_img_test1 demo_date2020 test_read_img_from_SD testInterrupt testInterruptDemo CNN_testbench new_vga_sobel_test new_vga_test_8b dram hello sdcard trace jump flash tag selftest eth lowrisc testPolling

# HMI and CNN stuff
# DRIVER_TARGETS = CNN_RISCV convolution softmax maxpool syscalls hid memory memcmp memset strcmp strcpy strnlen wordcopy elf sdhci-minion-hash-md5 mini-printf random dhcp-client ethlib stublib logo # minion_helper diskio ff
# MMC = blk_legacy.o crt.o ctype.o display_options.o div64.o elf.o ff.o hid.o logo.o lowrisc_mmc.o memory.o mini-printf.o mmc_legacy.o mmc.o sdhci-minion-hash-md5.o string.o strto.o syscalls.o time.o ethlib.o dhcp-client.o random.o

# Original Targets
DRIVER_TARGETS = syscalls hid memory memcmp memset strcmp strcpy strnlen wordcopy elf sdhci-minion-hash-md5 mini-printf random dhcp-client ethlib stublib logo
MMC = blk_legacy.o crt.o ctype.o display_options.o div64.o elf.o ff.o hid.o logo.o lowrisc_mmc.o memory.o mini-printf.o mmc_legacy.o mmc.o sdhci-minion-hash-md5.o string.o strto.o syscalls.o time.o ethlib.o dhcp-client.o random.o


#### DRIVER_TARGETS = syscalls hid memory elf sdhci-minion-hash-md5 mini-printf random dhcp-client ethlib logo ff fonctions_boot
#### DRIVER_TARGETS = syscalls hid memory memcmp memset strcmp strcpy strnlen wordcopy elf sdhci-minion-hash-md5 mini-printf random dhcp-client ethlib stublib logo ff fonctions_boot
# Reading from SD card
# DRIVER_TARGETS = syscalls hid memory memcmp memset strcmp strcpy strnlen wordcopy elf sdhci-minion-hash-md5 mini-printf random dhcp-client ethlib stublib logo ff fonctions_boot


#--------------------------------------------------------------------
# Build Drivers
#--------------------------------------------------------------------
HEADERS += $(wildcard $(DRIVER_DIR)/*.h)
DRIVER_OBJS = $(addsuffix .o, $(DRIVER_TARGETS))

$(DRIVER_OBJS): %.o:$(DRIVER_DIR)/%.c $(HEADERS)
	$(RISCV_GCC) $(RISCV_GCC_OPTS) -c $< -o $@

crt.o: $(DRIVER_DIR)/crt.S
	$(RISCV_GCC) $(RISCV_GCC_OPTS) -c $< -o $@

junk += $(DRIVER_OBJS) crt.o

#--------------------------------------------------------------------
# Building Targets
#--------------------------------------------------------------------

RISCV_FILES = $(addsuffix .riscv, $(TARGETS))
HEX_FILES = $(addsuffix .hex, $(TARGETS))
DUMP_FILES = $(addsuffix .dump, $(TARGETS))

all: $(HEX_FILES)

dump: $(DUMP_FILES)

$(RISCV_FILES): %.riscv:$(BASE_DIR)/%.c $(HEADERS) $(DRIVER_OBJS) crt.o $(DRIVER_DIR)/test.ld
	$(RISCV_LINK) $(RISCV_GCC_OPTS) $(RISCV_LINK_OPTS) -o $@ $< $(DRIVER_OBJS) crt.o $(MMC)
	riscv64-unknown-elf-objdump -d -S $@ > $*.dis

$(HEX_FILES): %.hex:%.riscv
	riscv64-unknown-elf-objcopy -I elf64-little -O verilog $< cnvmem.mem
	iverilog ../script/cnvmem.v -o cnvmem
	./cnvmem
	mv cnvmem.coe $*.coe
	mv cnvmem.hex $@

$(DUMP_FILES): %.dump:%.riscv
	$(RISCV_DUMP) $(RISCV_DUMP_OPTS) $< > $@

.PHONY: all dump
junk += $(RISCV_FILES) $(HEX_FILES) $(DUMP_FILES)

U = lib/strto.c  lib/time.c  lib/div64.c  lib/ctype.c   drivers/block/blk_legacy.c  drivers/mmc/lowrisc_mmc.c  drivers/mmc/mmc_legacy.c \
drivers/mmc/mmc.c lib/string.c  lib/display_options.c ../driver/mini-printf.c ../driver/hid.c ../driver/logo.c ../driver/syscalls.c boot.c ../driver/ff.c \
../driver/memory.c ../driver/sdhci-minion-hash-md5.c ../driver/elf.c ../driver/ethlib.c ../driver/dhcp-client.c ../driver/random.c



# MMC = crt.o blk_legacy.o  ctype.o display_options.o div64.o elf.o ff.o hid.o logo.o lowrisc_mmc.o memory.o mini-printf.o mmc_legacy.o mmc.o sdhci-minion-hash-md5.o string.o strto.o syscalls.o time.o ethlib.o dhcp-client.o random.o



boot: $U
	riscv64-unknown-elf-gcc -c -o crt.o ../driver/crt.S
	riscv64-unknown-elf-gcc -c -g -Iinclude -I./arch/riscv/include -I../driver -include ./include/linux/kconfig.h -D__KERNEL__ -D__UBOOT__ -Wall -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding -fshort-wchar -Os -fno-stack-protector -fno-delete-null-pointer-checks -g -fstack-usage -Wno-format-nonliteral -ffixed-gp -fpic -fno-strict-aliasing -fno-common -gdwarf-2 -I. -fno-builtin-printf -fno-builtin-memcpy $U 
	riscv64-unknown-elf-gcc -o $@ -g -nostdlib -nostartfiles boot.o $(MMC) -T ../driver/test.ld

MY_APPLICATION_C_FILES = lib/strto.c  lib/time.c  lib/div64.c  lib/ctype.c   drivers/block/blk_legacy.c  drivers/mmc/lowrisc_mmc.c  drivers/mmc/mmc_legacy.c \
drivers/mmc/mmc.c lib/string.c  lib/display_options.c ../driver/mini-printf.c ../driver/hid.c ../driver/logo.c ../driver/syscalls.c application.c ../driver/ff.c \
../driver/memory.c ../driver/sdhci-minion-hash-md5.c ../driver/elf.c ../driver/ethlib.c ../driver/dhcp-client.c ../driver/random.c 

CNN_RISCV_MANCINI = 		../cnn_riscv_mancini/src/top_cnn_mancini.c \
							../cnn_riscv_mancini/src/flush_mem.c \
							../cnn_riscv_mancini/src/convolution.c \
							../cnn_riscv_mancini/src/maxpool.c \
							../cnn_riscv_mancini/src/reshape.c \
							../cnn_riscv_mancini/src/perceptron.c \
							../cnn_riscv_mancini/src/argmax.c \
							../cnn_riscv_mancini/src/overlays.c

CNN_RISCV_MANCINI_INCLUDE = ../cnn_riscv_mancini/include

APPLICATION_O_FILES = crt.o blk_legacy.o ctype.o display_options.o div64.o elf.o ff.o hid.o logo.o lowrisc_mmc.o memory.o mini-printf.o \
      mmc_legacy.o mmc.o sdhci-minion-hash-md5.o string.o strto.o syscalls.o time.o ethlib.o dhcp-client.o random.o \
	  top_cnn_mancini.o \
	  flush_mem.o \
	  convolution.o \
	  maxpool.o \
	  reshape.o \
	  perceptron.o \
	  argmax.o \
	  overlays.o

application: $(MY_APPLICATION_C_FILES)
	riscv64-unknown-elf-gcc -c -o crt.o ../driver/crt.S
	riscv64-unknown-elf-gcc -c -g -Iinclude -I$(CNN_RISCV_MANCINI_INCLUDE) -I./arch/riscv/include -I../driver -include ./include/linux/kconfig.h \
							 -D__KERNEL__ -D__UBOOT__ -w -fno-builtin -ffreestanding -fno-math-errno -fshort-wchar -Os -fno-stack-protector -fno-delete-null-pointer-checks \
							 -g -fstack-usage -ffixed-gp -fpic -fno-strict-aliasing -fno-common -gdwarf-2 -I. -fno-builtin-printf -fno-builtin-memcpy \
							 $(MY_APPLICATION_C_FILES) $(CNN_RISCV_MANCINI)
	riscv64-unknown-elf-gcc -o $@ -g -w -nostdlib -nostartfiles $(APPLICATION_O_FILES) application.o -T ../driver/test.ld

#--------------------------------------------------------------------
# clean up
#--------------------------------------------------------------------

.PHONY: clean
clean:
	rm -rf $(junk)
	rm *.o *.dis *.riscv *.su *.tmp

.PHONY: cleanall
cleanall:
	rm -rf $(junk)
	rm *.o *.dis *.riscv *.su *.tmp boot application mapfile
