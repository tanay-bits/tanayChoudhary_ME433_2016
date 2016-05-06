// I2C Master utilities, 400 kHz
// The functions must be called in the correct order as per the I2C protocol
// I2C pins need pull-up resistors, 2k-10k
#include<xc.h>           // processor SFR definitions
#include "i2c.h"
#include "imu.h"

void i2c_write_single(unsigned char address, unsigned char data) {
    i2c_master_start();                         //ST
    i2c_master_send(IMU_ADDRESS << 1);          //SAD+W - SAK
    i2c_master_send(address);                   //SUB - SAK
    i2c_master_send(data);                      //DATA - SAK
    i2c_master_stop();                          //SP
}

void initIMU(void) {
    i2c_write_single(CTRL1_XL, 0b10000000);  //init accelerometer
    i2c_write_single(CTRL2_G, 0b10000000);   //init gyro
}

void i2c_read_multiple(char address, char reg, unsigned char * data, char length) {
    i2c_master_start();                     //ST
    i2c_master_send(address << 1);          //SAD+W - SAK
    i2c_master_send(reg);                   //SUB - SAK
    i2c_master_restart();                   //SR
    i2c_master_send(address << 1 | 0b1);    //SAD+R - SAK
    
    unsigned char i;
    for (i = 0; i < length; i++) {
        data[i] = i2c_master_recv();            //DATA
        if (i == length-1) {i2c_master_ack(1);} //NMAK if last read
        else {i2c_master_ack(0);}               //MAK if not last read                              
    }
    
    i2c_master_stop();                          //SP
}

