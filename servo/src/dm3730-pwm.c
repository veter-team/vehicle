// dm3730-pwm-demo --- Demonstrate usage of the dm3730-pwm library
// Based on the omap3530-pwm-demo by Thomas Most.
// Modified by Mark A. Yoder 25-July-2011
// Modified by Andrey Nechypurenko <andreynech@gmail.com> 06-Nov-2012 - remove
// dependencies on glib
// Copyright (c) 2010 Thomas W. Most <twm@freecog.net>
//
// The contents of this file may be used subject to the terms of either of the
// following licenses:
//
// GNU LGPL 2.1 license:
//
//   This library is free software; you can redistribute it and/or modify it
//   under the terms of the GNU Lesser General Public License as published by the
//   Free Software Foundation; either version 2.1 of the License, or (at your
//   option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT
//   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
//   for more details.
//
//   You should have received a copy of the GNU Lesser General Public License
//   along with this library; if not, write to the Free Software Foundation,
//   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// MIT license:
//
//   Permission is hereby granted, free of charge, to any person obtaining a copy
//   of this software and associated documentation files (the "Software"), to deal
//   in the Software without restriction, including without limitation the rights
//   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//   copies of the Software, and to permit persons to whom the Software is
//   furnished to do so, subject to the following conditions:
//   
//   The above copyright notice and this permission notice shall be included in
//   all copies or substantial portions of the Software.
//   
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//   THE SOFTWARE.

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

#include "dm3730-pwm.h"

// Clock configuration registers (TRM p. 470)
#define CM_CLKSEL_CORE 0x48004A40
#define CLKSEL_GPT10_MASK (1 << 6)
#define CLKSEL_GPT11_MASK (1 << 7)

#define CM_CLKSEL_FCLK 0x48004A00
#define FCLKSEL_GPT10_MASK (1 << 11)
#define FCLKSEL_GPT11_MASK (1 << 12)

#define CM_CLKSEL_ICLK 0x48004A10
#define ICLKSEL_GPT10_MASK (1 << 11)
#define ICLKSEL_GPT11_MASK (1 << 12)

#define CLKFREQ_SEL 0x48306D40
#define BIT1 (1 << 1)
#define BIT0 (1 << 0)

// GPTIMER register offsets
#define GPT_REG_TCLR 0x024
#define GPT_REG_TCRR 0x028
#define GPT_REG_TLDR 0x02c
#define GPT_REG_TMAR 0x038

// Get a uint32_t pointer to the register in block `instance` at byte
// offset `offset`.
#define REG32_PTR(instance, offset) ((volatile uint32_t*) (instance + offset))


// General purpose timer instances.  Not all of these can actually be
// used for PWM --- see the TRM for more information.
static uint32_t gpt_instance_addrs[] = {
  0x4903e000, // GPTIMER8
  0x49040000, // GPTIMER9
  0x48086000, // GPTIMER10
  0x48088000, // GPTIMER11
};


// The default Linux page size is 4k and the GP timer register
// blocks are aligned to 4k.  Therefore it is convenient to just
// assume that pages are aligned there for the purposes of mmap()
// (since mmap only maps aligned pages).  This function checks
// that assumption and aborts if it is untrue.
static void
check_pagesize(void)
{
  if (getpagesize() != 4096)
    fprintf(stderr, "The page size is %d.  Must be 4096.", getpagesize());
}

// Simply a wrapper around mmap that passes the correct arguments
// for mapping a register block.  `instance_number` must be between
// 1 and 12, or errno will be set to EDOM and MAP_FAILED returned.
// Otherwise the return value is that of `mmap()`.
uint8_t*
pwm_mmap_instance(int mem_fd, int instance_number)
{
  if (instance_number < 8 || instance_number > 11) {
    errno = EDOM;
    return MAP_FAILED;
  }
  int instance_addr = gpt_instance_addrs[instance_number - 8];
  return mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, instance_addr);
}

// The inverse of `pwm_mmap_instance()`, this is simply a wrapper
// arount `munmap()`.  It returns the underlying `munmap()` call's
// return value.
int
pwm_munmap_instance(uint8_t *instance)
{
  return munmap(instance, 4096);
}

// Configure the clocks for GPTIMER10 and GPTIMER11, which can be set to
// use the 13 MHz system clock (otherwise they use the 32 kHz clock like
// the rest of the timers).  Return -1 on failure, with errno set.
int
pwm_config_clock(int mem_fd, bool_t gptimer10_13mhz, bool_t gptimer11_13mhz)
{
  int page_addr = CM_CLKSEL_CORE & 0xfffff000;
  int offset = CM_CLKSEL_CORE & 0xfff;

  uint8_t *registers = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_addr);
  if(registers == MAP_FAILED)
    return -1;

  uint32_t value = *REG32_PTR(registers, offset);
  value &= ~(CLKSEL_GPT10_MASK | CLKSEL_GPT11_MASK);
  if(gptimer10_13mhz) value |= CLKSEL_GPT10_MASK;
  if(gptimer11_13mhz) value |= CLKSEL_GPT11_MASK;
  *REG32_PTR(registers, offset) = value;

  return munmap(registers, 4096);
}

int
pwm_fclken_clock(int mem_fd, bool_t gptimer10_13mhz, bool_t gptimer11_13mhz)
{
  int page_addr = CM_CLKSEL_FCLK & 0xfffff000;
  int offset = CM_CLKSEL_FCLK & 0xfff;

  uint8_t *registers = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_addr);
  if(registers == MAP_FAILED)
    return -1;

  uint32_t value = *REG32_PTR(registers, offset);
  value &= ~(FCLKSEL_GPT10_MASK | FCLKSEL_GPT11_MASK);
  if(gptimer10_13mhz) value |= FCLKSEL_GPT10_MASK;
  if(gptimer11_13mhz) value |= FCLKSEL_GPT11_MASK;
  *REG32_PTR(registers, offset) = value;

  return munmap(registers, 4096);
}


int
pwm_iclken_clock(int mem_fd, bool_t gptimer10_13mhz, bool_t gptimer11_13mhz)
{
  int page_addr = CM_CLKSEL_ICLK & 0xfffff000;
  int offset = CM_CLKSEL_ICLK & 0xfff;

  uint8_t *registers = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_addr);
  if(registers == MAP_FAILED)
    return -1;

  uint32_t value = *REG32_PTR(registers, offset);
  value &= ~(ICLKSEL_GPT10_MASK | ICLKSEL_GPT11_MASK);
  if(gptimer10_13mhz) value |= ICLKSEL_GPT10_MASK;
  if(gptimer11_13mhz) value |= ICLKSEL_GPT11_MASK;
  *REG32_PTR(registers, offset) = value;

  return munmap(registers, 4096);
}


int
pwm_clkfreq_sel(int mem_fd, bool_t gptimer10_13mhz, bool_t gptimer11_13mhz)
{
  int page_addr = CLKFREQ_SEL & 0xfffff000;
  int offset = CLKFREQ_SEL & 0xfff;

  uint8_t *registers = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_addr);
  if(registers == MAP_FAILED)
    return -1;

  uint32_t value = *REG32_PTR(registers, offset);
  value &= ~(BIT1 | BIT0);
  if(gptimer10_13mhz) value |= BIT1;
  if(gptimer11_13mhz) value |= BIT0;
  *REG32_PTR(registers, offset) = value;

  return munmap(registers, 4096);
}


// Calculate the resolution of the PWM (the number of clock ticks
// in the period), which is passed to `pwm_config_timer()`.
uint32_t
pwm_calc_resolution(int pwm_frequency, int clock_frequency)
{
  float pwm_period = 1.0 / pwm_frequency;
  float clock_period = 1.0 / clock_frequency;
  return (uint32_t)(pwm_period / clock_period);
}

// Initialize the control registers of the specified timer
// instance for PWM at the specified resolution. 
void
pwm_config_timer(uint8_t *instance, uint32_t resolution, float duty_cycle)
{
  uint32_t counter_start = 0xffffffff - resolution;
  uint32_t dc = 0xffffffff - ((uint32_t) (resolution * duty_cycle));

  // Edge condition: the duty cycle is set within two units of the overflow
  // value.  Loading the register with this value shouldn't be done (TRM 16.2.4.6).
  if (0xffffffff - dc <= 2) {
    dc = 0xffffffff - 2;
  }

  // Edge condition: TMAR will be set to within two units of the overflow 
  // value.  This means that the resolution is extremely low, which doesn't
  // really make sense, but whatever.
  if (0xffffffff - counter_start <= 2) {
    counter_start = 0xffffffff - 2;
  }

  *REG32_PTR(instance, GPT_REG_TCLR) = 0; // Turn off
  *REG32_PTR(instance, GPT_REG_TCRR) = counter_start;
  *REG32_PTR(instance, GPT_REG_TLDR) = counter_start;
  *REG32_PTR(instance, GPT_REG_TMAR) = dc;
  *REG32_PTR(instance, GPT_REG_TCLR) = (
					(1 << 0)  | // ST -- enable counter
					(1 << 1)  | // AR -- autoreload on overflow
					(1 << 6)  | // CE -- compare enabled
					(1 << 7)  | // SCPWM -- invert pulse
					(2 << 10) | // TRG -- overflow and match trigger
					(1 << 12)   // PT -- toggle PWM mode
					);
}

int
pwm_open_devmem(void)
{
  check_pagesize();

  return open("/dev/mem", O_RDWR | O_SYNC);
}

void
pwm_close_devmem(int dev_fd)
{
  /* This function is useful! */
  close(dev_fd);
}
