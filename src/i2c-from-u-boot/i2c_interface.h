#ifndef __I2C_INTERFACE_H
#define __I2C_INTERFACE_H

#include <stdint.h>

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
void i2c_init(int speed, int slaveaddr);

/*
 * Uninitialize and free ackquired resources
 */
void i2c_uninit();

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uint8_t chip);

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uint8_t chip, uint32_t addr, int alen, uint8_t *buffer, int len);
int i2c_write(uint8_t chip, uint32_t addr, int alen, uint8_t *buffer, int len);

/*
 * Utility routines to read/write registers.
 */
static inline uint8_t i2c_reg_read(uint8_t addr, uint8_t reg)
{
  uint8_t buf;
  i2c_read(addr, reg, 1, &buf, 1);
  return buf;
}

static inline void i2c_reg_write(uint8_t addr, uint8_t reg, uint8_t val)
{
  i2c_write(addr, reg, 1, &val, 1);
}

/*
 * Functions for setting the current I2C bus and its speed
 */

/*
 * i2c_set_bus_num:
 *
 *  Change the active I2C bus.  Subsequent read/write calls will
 *  go to this one.
 *
 *      bus - bus index, zero based
 *
 *      Returns: 0 on success, not 0 on failure
 *
 */
int i2c_set_bus_num(unsigned int bus);

/*
 * i2c_get_bus_num:
 *
 *  Returns index of currently active I2C bus.  Zero-based.
 */
unsigned int i2c_get_bus_num(void);

#endif // __I2C_INTERFACE_H

