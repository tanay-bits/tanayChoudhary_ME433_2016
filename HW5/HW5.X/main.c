#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "ILI9163C.h"    // LCD library
#include "i2c.h"         // I2C2 lib
#include "imu.h"         // LSM6DS33 IMU lib

static volatile short temp_raw, accX_raw, accY_raw, accZ_raw, gyroX_raw, gyroY_raw, gyroZ_raw;
#define MAX_LENGTH 50   // max string length to be printed on LCD at once

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2 // divide input clock (8MHz) to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV (4*24=96MHz)
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get (96/2=48MHz)
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 20 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

// Initialize I2C2
void initI2C2() {
    // turn off analog on I2C pins
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    i2c_master_setup();
}

int main() {
    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // initialize I2C
    initI2C2();
    
    // initialize IMU
    initIMU();
    
    // initialize SPI, LCD
    SPI1_init();
    LCD_init();
    
    __builtin_enable_interrupts();
      
    LCD_clearScreen(BLACK);   
    char message[MAX_LENGTH];  
    unsigned char data[14];
    float accX, accY, accZ;
    _CP0_SET_COUNT(0);
    while(1) {       
        if (_CP0_GET_COUNT() > 480000) {    // 50 Hz     
            i2c_read_multiple(IMU_ADDRESS, OUT_TEMP_L, data, 14);
            temp_raw = data[1] << 8 | data[0];
            gyroX_raw = data[3] << 8 | data[2];
            gyroY_raw = data[5] << 8 | data[4];
            gyroZ_raw = data[7] << 8 | data[6];
            accX_raw = data[9] << 8 | data[8];
            accY_raw = data[11] << 8 | data[10];
            accZ_raw = data[13] << 8 | data[12];
            
            accX = accX_raw * 2.0 / 32768; // accel in g
            accY = accY_raw * 2.0 / 32768; // accel in g
            accZ = accZ_raw * 2.0 / 32768; // accel in g
            
            sprintf(message, "temp raw: %x    ", temp_raw);
            LCD_drawString(10, 10, message, WHITE);
            
            sprintf(message, "accX: %f g    ", accX);
            LCD_drawString(10, 20, message, WHITE);
            
            sprintf(message, "accY: %f g    ", accY);
            LCD_drawString(10, 30, message, WHITE);
            
            sprintf(message, "accZ: %f g    ", accZ);
            LCD_drawString(10, 40, message, WHITE);
            
            sprintf(message, "gyroX raw: %i    ", gyroX_raw);
            LCD_drawString(10, 50, message, WHITE);
            
            sprintf(message, "gyroY raw: %i    ", gyroY_raw);
            LCD_drawString(10, 60, message, WHITE);
            
            sprintf(message, "gyroZ raw: %i    ", gyroZ_raw);
            LCD_drawString(10, 70, message, WHITE);
            
            _CP0_SET_COUNT(0);
        }
    }       
}