#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>         // math lib for sin function
#include "i2c_master_noint.h" // i2c2 lib

#define CS LATBbits.LATB15 // chip select pin for SPI

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

// Initialize SPI1
void initSPI1() {
    // turn off analog on SPI pins
    ANSELBbits.ANSB15 = 0;
    ANSELBbits.ANSB13 = 0;
    
    SPI1CON = 0; // turn off the SPI module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1; // baud rate to fastest; SPI1BRG = (48000000/(2*desired))-1
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.MODE16 = 1; // 16 bits of data sent per transfer
    SPI1CONbits.ON = 1; // turn on SPI1
    
//    RPB15Rbits.RPB15R = 0b0011; //B15 is SS1 output
    RPB13Rbits.RPB13R = 0b0011; //B13 is SDO1 output
    TRISBbits.TRISB15 = 0; //SS1 pin is digital output
}

// Send a 16-bit word via SPI
unsigned short SPI1_IO(unsigned short write) {
    SPI1BUF = write; // send a data byte via SPI
    while (!SPI1STATbits.SPIRBF) { // wait to receive the byte
        ;
    }
    return SPI1BUF; // clear the rx buffer by reading from it
}

// Tell DAC to output analog voltage; channel 0: DAC_A, channel 1: DAC_B
void setVoltage(char channel, unsigned char voltage) {
    CS = 0; // CS pin on DAC requires active low to enable
    char configbits = channel << 3 | 0b0111;
    unsigned short word = (configbits << 12) | (voltage << 4);
    SPI1_IO(word);
    CS = 1; // stop transfer of data
}

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

    // SPI initialize
    initSPI1();
    
    // I2C initialize
    initI2C2();
    
    // Expander initialize
    initExpander();
    
    __builtin_enable_interrupts();
      
    char channelA = 0;
    char channelB = 1;
    unsigned char voltageA;
    unsigned char voltageB;

    _CP0_SET_COUNT(0);
    while(1) {
        // sine wave at 10 Hz
        voltageA = 127 + 127*sin(6.28/2400000 * _CP0_GET_COUNT());
        setVoltage(channelA, voltageA);
        
        // triangle wave at 5 Hz
        voltageB = 255.0/4800000 * _CP0_GET_COUNT();
        setVoltage(channelB, voltageB);
        
        setExpander(0,1);
//        unsigned short i = 0;
//        if (getExpander(7) == 0) {
////            while (i < 24000) {
////                i++;
////            }
//            setExpander(0,1);
//        } // turn on LED
//        else {setExpander(0,0);
//        } // turn off LED  
    }
        
}