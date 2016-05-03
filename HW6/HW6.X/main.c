#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c.h"         // I2C2 lib
#include "imu.h"         // LSM6DS33 IMU lib

static volatile short temp_raw, accX_raw, accY_raw, accZ_raw, gyroX_raw, gyroY_raw, gyroZ_raw;

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

// Initialize OC1, OC2, Timer2 for two PWM's
void initPWM() {
    RPA0Rbits.RPA0R = 0b0101;   //set A0 as OC1
    RPA1Rbits.RPA1R = 0b0101;   //set A1 as OC2
    
    T2CONbits.TCKPS = 0b100; //Timer2 prescaler N=16 (1:16)
    PR2 = 2999; //period = (PR2+1) * N * 20.833 ns = 1000 us => 1 kHz
    TMR2 = 0; // initial Timer2 count is 0
    IPC2bits.T2IP = 5; // priority for Timer2 interrupt
    IFS0bits.T2IF = 0; // clear Timer2 interrupt flag
    IEC0bits.T2IE = 1; // enable Timer2 interrupt
    T2CONbits.ON = 1; // turn on Timer2
    
    OC1CONbits.OCTSEL = 0; //use Timer 2
    OC1CONbits.OCM = 0b110; //PWM mode without fault pin; other OC1CON bits are defaults
    OC1RS = 1500; // initialize duty cycle = OC1RS/(PR2+1) = 50%
    OC1R = 1500; // initialize before turning OC1 on; afterward it is read-only
    OC1CONbits.ON = 1; // turn on OC1
    
    OC2CONbits.OCTSEL = 0; //use Timer 2
    OC2CONbits.OCM = 0b110; //PWM mode without fault pin; other OC1CON bits are defaults
    OC2RS = 1500; // initialize duty cycle = OC1RS/(PR2+1) = 50%
    OC2R = 1500; // initialize before turning OC1 on; afterward it is read-only
    OC2CONbits.ON = 1; // turn on OC2
}

void __ISR(_TIMER_2_VECTOR, IPL5SOFT) PWM(void) {
    float accX, accY;
    accX = accX_raw * 2.0 / 32768;  // accel in g
    accY = accY_raw * 2.0 / 32768;  // accel in g
    
    OC1RS = (unsigned int)(1500 + 1500 * accX);
    OC2RS = (unsigned int)(1500 + 1500 * accY);
    
    IFS0bits.T2IF = 0; // clear interrupt flag
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

    // I2C initialize
    initI2C2();
    
    // IMU initialize
    initIMU();
    
    // PWM initialize
    initPWM();
    
    __builtin_enable_interrupts();

//    TRISAbits.TRISA4 = 0; // A4 as output
//    LATAbits.LATA4 = 0;   // A4 initially low
    
    unsigned char data[14];    
    while(1) {
        _CP0_SET_COUNT(0);
        if (_CP0_GET_COUNT() > 480000) {    // 50 Hz
            i2c_read_multiple(IMU_ADDRESS, OUT_TEMP_L, data, 14);
            temp_raw = data[1] << 8 | data[0];
            gyroX_raw = data[3] << 8 | data[2];
            gyroY_raw = data[5] << 8 | data[4];
            gyroZ_raw = data[7] << 8 | data[6];
            accX_raw = data[9] << 8 | data[8];
            accY_raw = data[11] << 8 | data[10];
            accZ_raw = data[13] << 8 | data[12];
        }
        
    }
        
}