#ifndef FONCTIONS_BOOT
#define FONCTIONS_BOOT


//#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <memory.h>
#include "encoding.h"
#include "mini-printf.h"
#include "diskio.h"
#include "ff.h"
#include "bits.h"
#include "hid.h"
#include "eth.h"
#include "elfriscv.h"
#include "lowrisc_memory_map.h"

int just_jump (void);
int lowrisc_init(unsigned long addr, int ch, unsigned long quirks);
void tohost_exit(long code);
unsigned long get_tbclk (void);
char *env_get(const char *name);
int init_mmc_standalone(int sd_base_addr);
DSTATUS disk_initialize (uint8_t pdrv);
int ctrlc(void);
void *find_cmd_tbl(const char *cmd, void *table, int table_len);
unsigned long timer_read_counter(void);
void __assert_fail (const char *__assertion, const char *__file, unsigned int __line, const char *__function);
void *memalign(size_t alignment, size_t size);
int do_load(void *cmdtp, int flag, int argc, char * const argv[], int fstype);
int do_ls(void *cmdtp, int flag, int argc, char * const argv[], int fstype);
int do_size(void *cmdtp, int flag, int argc, char * const argv[], int fstype);
DRESULT disk_read (uint8_t pdrv, uint8_t *buff, uint32_t sector, uint32_t count);
DRESULT disk_write (uint8_t pdrv, const uint8_t *buff, uint32_t sector, uint32_t count);
DRESULT disk_ioctl (uint8_t pdrv, uint8_t cmd, void *buff);
DSTATUS disk_status (uint8_t pdrv);
void part_init(void *bdesc);
void part_print(void *desc);
void dev_print(void *bdesc);
unsigned long mmc_berase(void *dev, int start, int blkcnt);
unsigned long mmc_bwrite(void *dev, int start, int blkcnt, const void *src);

#endif
