// Header file for LSM6DS33 IMU
#ifndef _IMU_H    /* Guard against multiple inclusion */
#define _IMU_H

// Address of Slave
#define IMU_ADDRESS 0b1101011
// Registers
#define WHO_AM_I 0x0F
#define CTRL1_XL 0x10
#define CTRL2_G 0x11
#define CTRL3_C 0x12
#define OUT_TEMP_L 0x20
#define OUT_TEMP_H 0x21
#define OUTX_L_G 0x22
#define OUTX_H_G 0x23
#define OUTY_L_G 0x24
#define OUTY_H_G 0x25
#define OUTZ_L_G 0x26
#define OUTZ_H_G 0x27
#define OUTX_L_XL 0x28
#define OUTX_H_XL 0x29
#define OUTY_L_XL 0x2A
#define OUTY_H_XL 0x2B
#define OUTZ_L_XL 0x2C
#define OUTZ_H_XL 0x2D

void initIMU(void);
void i2c_write_single(unsigned char address, unsigned char data);
void i2c_read_multiple(char address, char reg, unsigned char * data, char length);

#endif