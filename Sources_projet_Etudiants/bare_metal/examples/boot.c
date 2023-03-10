// See LICENSE for license details.

//#include <stdio.h>
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

FATFS FatFs;   // Work area (file system object) for logical drive

// max size of file image is 16M
#define MAX_FILE_SIZE 0x1000000

// 4K size read burst
#define SD_READ_SIZE 4096

char md5buf[SD_READ_SIZE];

void sd_main(int sw)
{
  FIL fil;                // File object
  FRESULT fr;             // FatFs return code
  uint8_t *boot_file_buf = (uint8_t *)(get_ddr_base()) + ((uint64_t)get_ddr_size()) - MAX_FILE_SIZE; // at the end of DDR space
  uint8_t *memory_base = (uint8_t *)(get_ddr_base());

  // Register work area to the default drive
  if(f_mount(&FatFs, "", 1)) {
    printf("Fail to mount SD driver!\n");
    return 1;
  }

  // Open a file
  printf("Load boot.bin into memory\n");
  fr = f_open(&fil, "boot.bin", FA_READ);
  if (fr) {
    printf("Failed to open boot!\n");
    return (int)fr;
  }

  // Read file into memory
  uint8_t *hashbuf;
  uint32_t fsize = 0;           // file size count
  uint32_t br;                  // Read count
  do {
    char *sum;
    fr = f_read(&fil, boot_file_buf+fsize, SD_READ_SIZE, &br);  // Read a chunk of source file
    if (!fr)
      {
	hid_send('\b');
	hid_send("|/-\\"[(fsize/SD_READ_SIZE)&3]);
	fsize += br;
      }
  } while(!(fr || br == 0));
  fsize = fil.fsize;

  // Close the file
  if(f_close(&fil)) {
    printf("fail to close file!");
    return 1;
  }
  if(f_mount(NULL, "", 1)) {         // unmount it
    printf("fail to umount disk!");
    return 1;
  }

  printf("Loaded %d bytes to memory address %x from boot.bin of %d bytes.\n", fsize, boot_file_buf, fsize);
#if 1
  hashbuf = hash_buf(boot_file_buf, fsize);
  printf("hash = %s\n", hashbuf);
#endif 
  // read elf
  printf("load elf to DDR memory\n");
  if(br = load_elf(boot_file_buf, fsize))
    printf("elf read failed with code %d", br);
  printf("Boot the loaded program...\n");

  uintptr_t mstatus = read_csr(mstatus);
  mstatus = INSERT_FIELD(mstatus, MSTATUS_MPP, PRV_M);
  mstatus = INSERT_FIELD(mstatus, MSTATUS_MPIE, 1);
  write_csr(mstatus, mstatus);
  write_csr(mepc, memory_base);
  asm volatile ("mret");
}

int just_jump (void)
{
  uint8_t *memory_base = (uint8_t *)(get_ddr_base());
  uintptr_t mstatus = read_csr(mstatus);
  mstatus = INSERT_FIELD(mstatus, MSTATUS_MPP, PRV_M);
  mstatus = INSERT_FIELD(mstatus, MSTATUS_MPIE, 1);
  write_csr(mstatus, mstatus);
  write_csr(mepc, memory_base);
  asm volatile ("mret");
}

#define HELLO "Hello LowRISC! "__TIMESTAMP__": "

int main (void)
{
  int i, sw = sd_resp(31);

  loopback_test(8, (sw & 0xF) == 0xF);
  
  printf("lowRISC boot program\nVersion Phelma v0.0 - Built at "__TIMESTAMP__"\n=====================================\n");

  for (i = 10000000; i; i--)
    write_led(i);

  if (sw & 1)
      {
        printf(HELLO"Jumping to DRAM because SW0 is high ..\n");
        just_jump();
      }
  if (sw & 2)
      {
        printf(HELLO"Booting from FLASH because SW1 is high ..\n");
        sd_main(sw>>3);
      }
  if (sw & 4)
      {
        printf(HELLO"Booting from Ethernet because SW2 is high ..\n");
        eth_main();
      }
  printf(HELLO"Turn on SW0 for gdb loading, SW1 for SD-card loading, or SW2 for Ethernet loading\n");
  for (i = 1; i; i++)
    write_led(i>>16);
}

int lowrisc_init(unsigned long addr, int ch, unsigned long quirks);
void tohost_exit(long code);

unsigned long get_tbclk (void)
{
	unsigned long long tmp = 1000000;
	return tmp;
}

char *env_get(const char *name)
{
  return (char *)0;
}

int init_mmc_standalone(int sd_base_addr);

DSTATUS disk_initialize (uint8_t pdrv)
{
  printf("\nu-boot based first stage boot loader\n");
  init_mmc_standalone(sd_base_addr);
  return 0;
}

int ctrlc(void)
{
	return 0;
}

void *find_cmd_tbl(const char *cmd, void *table, int table_len)
{
  return (void *)0;
}

unsigned long timer_read_counter(void)
{
  return read_csr(0xb00) / 10;
}

void __assert_fail (const char *__assertion, const char *__file,
                           unsigned int __line, const char *__function)
{
  printf("assertion %s failed, file %s, line %d, function %s\n", __assertion, __file,  __line, __function);
  tohost_exit(1);
}

void *memalign(size_t alignment, size_t size)
{
  char *ptr = malloc(size+alignment);
  return (void*)((-alignment) & (size_t)(ptr+alignment));
}

int do_load(void *cmdtp, int flag, int argc, char * const argv[], int fstype)
{
  return 1;
}

int do_ls(void *cmdtp, int flag, int argc, char * const argv[], int fstype)
{
  return 1;
}

int do_size(void *cmdtp, int flag, int argc, char * const argv[], int fstype)
{
                return 1;
}

DRESULT disk_read (uint8_t pdrv, uint8_t *buff, uint32_t sector, uint32_t count)
{
  while (count--)
    {
      read_block(buff, sector++);
      buff += 512;
    }
  return FR_OK;
}

DRESULT disk_write (uint8_t pdrv, const uint8_t *buff, uint32_t sector, uint32_t count)
{
  return FR_INT_ERR;
}

DRESULT disk_ioctl (uint8_t pdrv, uint8_t cmd, void *buff)
{
  return FR_INT_ERR;
}

DSTATUS disk_status (uint8_t pdrv)
{
  return FR_INT_ERR;
}

void part_init(void *bdesc)
{

}

void part_print(void *desc)
{

}

void dev_print(void *bdesc)
{

}

unsigned long mmc_berase(void *dev, int start, int blkcnt)
{
        return 0;
}

unsigned long mmc_bwrite(void *dev, int start, int blkcnt, const void *src)
{
        return 0;
}

const char version_string[] = "LowRISC minimised u-boot for SD-Card";
