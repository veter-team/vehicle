#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "enable_i2c_clocks.h"

#define CM_CLKSEL_ICLK 0x48004A10
#define CM_CLKSEL_FCLK 0x48004A00

#define REG32_PTR(instance, offset) ((volatile uint32_t*) (instance + offset))


int
pwm_fclken_clock(int mem_fd)
{
  int page_addr = CM_CLKSEL_FCLK & 0xfffff000;
  int offset = CM_CLKSEL_FCLK & 0xfff;

  uint8_t *registers = 
    mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_addr);
  if (registers == MAP_FAILED)
    return -1;

  uint32_t value = *REG32_PTR(registers, offset);
  *REG32_PTR(registers, offset) = value | (1 << 16);

  return munmap(registers, 4096);
}


int
pwm_iclken_clock(int mem_fd)
{
  int page_addr = CM_CLKSEL_ICLK & 0xfffff000;
  int offset = CM_CLKSEL_ICLK & 0xfff;

  uint8_t *registers = 
    mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_addr);
  if(registers == MAP_FAILED)
    return -1;

  uint32_t value = *REG32_PTR(registers, offset);
  *REG32_PTR(registers, offset) = value | (1 << 16);

  return munmap(registers, 4096);
}


int 
enable_i2c_clocls(void)
{
  int res;
  int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
  if(mem_fd < 0)
    {
      printf("Can not open /dev/mem\n");
      return mem_fd;
    }

  res = pwm_fclken_clock(mem_fd);
  if(res)
    {
      printf("Error enabling FCLK for I2C2\n");
      return res;
    }
	
  res = pwm_iclken_clock(mem_fd);
  if(res)
    {
      printf("Error enabling ICLK for I2C2\n");
      return res;
    }

  close(mem_fd);
  return 0;
}

