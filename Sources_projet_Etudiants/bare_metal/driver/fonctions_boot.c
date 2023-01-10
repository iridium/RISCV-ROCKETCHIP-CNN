// See LICENSE for license details.

#include "fonctions_boot.h"

// max size of file image is 16M
#define MAX_FILE_SIZE 0x1000000

// 4K size read burst
#define SD_READ_SIZE 4096




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
