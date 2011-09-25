#include "xenopwm.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/io.h>    /* may be <asm/io.h> on some systems */

#include <native/task.h>
#include <native/timer.h>

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 */

#define IEN     (1 << 8)

#define IDIS    (0 << 8)
#define PTU     (1 << 4)
#define PTD     (0 << 4)
#define EN      (1 << 3)
#define DIS     (0 << 3)

#define M0      0
#define M1      1
#define M2      2
#define M3      3
#define M4      4
#define M5      5
#define M6      6
#define M7      7

typedef unsigned long ulong;

#define OFFSET(OFF) (OFF / sizeof(ulong))

#define STACKSIZE 1024


static int RC_NUM = 0;
static RT_TASK pwm_task[8];
static RTIME up_period[8];

// Initialized with default pulse ranges
static int ranges[8][2] = {
  {950, 2050},
  {950, 2050},
  {950, 2050},
  {950, 2050},
  {950, 2050},
  {950, 2050},
  {950, 2050},
  {950, 2050}
};

// File descriptor for /dev/mem
static int fd = -1;
// Memory for triggering gpio
static volatile ulong *gpio = NULL;

void 
pwm_func(void *arg)
{
  int which = (int)arg;

  printf("set_periodic for pwm_func#%i\n", which);
  rt_task_set_periodic(NULL, TM_NOW, 20000000);

  // Toggling the pins
  for(;;)
    {
      //set_data_out has offset 0x94
      gpio[OFFSET(0x6094)]=0x40000000;
      rt_task_sleep(up_period[which]);
      //clear_data_out has offset 0x90
      gpio[OFFSET(0x6090)]=0x40000000;
      rt_task_wait_period(NULL);
    }
}


/* maps integer x in the range [a..b] to [c..d] */
int 
range_map(int a, int b, int c, int d, int x)
{
  if (a == b) return 0;

  return c + (d - c)*(x - a)/(b - a);
}


void 
setpwmwidth(int channel, int percentage)
{
  //printf("%i -> %i\n", channel, percentage);
  up_period[channel] = 1000 * range_map(0,
                                        100, 
                                        ranges[channel][0], 
                                        ranges[channel][1], 
                                        percentage);
}


int 
getpwmwidth(int channel)
{
  return range_map(ranges[channel][0], ranges[channel][1], 
                   0, 100, up_period[channel] / 1000);
}


int 
initpwm(PwmDesc *channels, int nchannels)
{
  int i;
  int retval;

  if(nchannels < 0 || nchannels > 8)
    {
      fprintf(stderr, "Number of channels should be between 1 and 8\n");
      return 1;
    }

  RC_NUM = nchannels;
  for(i = 0; i < RC_NUM; i++)
    {
      ranges[channels[i].channel][0] = channels[i].pwmMinWidth;
      ranges[channels[i].channel][1] = channels[i].pwmMaxWidth;
      up_period[channels[i].channel] = 
        1000 * range_map(0, 100, ranges[i][0], ranges[i][1], 50);
    }

  printf("Pulse lengths initialized\n");

  printf("Setting the pin configuration\n");

  //O_SYNC makes the memory uncacheable

  fd = open("/dev/mem", O_RDWR | O_SYNC);
  if(fd < 0)
  {
    fprintf(stderr, "Could not open memory\n");
    return 0;
  }

  printf("Configuring I/O pads mode\n");
  volatile ulong *pinconf =
    (ulong*)mmap(NULL, 0x10000, PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, 0x48000000);
  if(pinconf == MAP_FAILED)
  {
    fprintf(stderr, "Pinconf Mapping failed\n");
    close(fd);
    return 0;
  }
  // set lower 16 pins to GPIO bank5
  // (EN | PTD | M4) is 0x0C (0b01100)
  pinconf[OFFSET(0x2190)] = (EN | PTD | M4);
  close(fd);

  printf("Configuring GPIO Bank 5\n");

  fd = open("/dev/mem", O_RDWR | O_SYNC);
  if(fd < 0)
  {
    fprintf(stderr, "Could not open memory\n");
    return 0;
  }

  printf("Configuring GPIO\n");

  gpio =
    (ulong*)mmap(NULL, 0x10000, PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, 0x49050000);
  if(gpio == MAP_FAILED)
    {
      fprintf(stderr, "Gpio Mapping failed\n");
      close(fd);
      return 0;
    }

  // 0x4000 0000 - bit 30 is set
  // 0x8000 0000 - bit 31 is set

  // First set all output on bank5 to high
  // (set_data_out has offset 0x94)
  gpio[OFFSET(0x6094)]=0xFFFFFFFF;

  // Configure low 16 GPIO pins on bank 5 as output.
  // GPIO 5 is at physical address 0x49056000 = 0x49050000+0x6000
  // GPIO Output enable (GPIO_OE) is offset by 0x34 for each bank
  // (set low for output)
  gpio[OFFSET(0x6034)] = 0x00000000;
  // Also disable the wakeupenable and irqenable intertupts
  // GPIO clear_Wakeupenable is offset by 0x80 for each bank
  gpio[OFFSET(0x6080)] = 0x0000FFFF;
  // GPIO clear_irqenable1 is offset by 0x60 for each bank
  gpio[OFFSET(0x6060)] = 0x0000FFFF;
  // GPIO clear_irqenable2 is offset by 0x70 for each bank
  gpio[OFFSET(0x6070)] = 0x0000FFFF;

  printf("Starting PWM generation tasks.\n");

  mlockall(MCL_CURRENT|MCL_FUTURE);

  for(i = 0; i < RC_NUM; i++)
    {
      retval = rt_task_create(&pwm_task[i], NULL, 0, 99, 0);
      switch(retval)
        {
        case -ENOMEM: 
          fprintf(stderr,
                  "Not enough real-time memory to createthe pwm task#%i\n", 
                  i);
          return retval;

        case -EEXIST: 
          fprintf(stderr, 
                  "The name for pwm task#%i is already in use\n",
                  i);
          return retval;

        case -EPERM: 
          fprintf(stderr, 
                  "rt_task_create() was called from an asynchronous context\n");
          return retval;

        case 0: // is returned upon success
        default:
          break;
        }
      rt_task_start(&pwm_task[i], &pwm_func, (void*)(channels[i].channel));
    }

  return 0;
}


void 
cleanuppwm()
{
  int i = 0;
  for(; i < RC_NUM; i++)
    rt_task_delete(&pwm_task[i]);
  usleep(20000*2);
  if(fd != -1)
    close(fd);
}

