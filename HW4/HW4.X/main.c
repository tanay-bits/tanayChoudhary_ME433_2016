#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

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

void initSPI1() {
    SPI1CON = 0; // turn off the SPI module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 0x2; // baud rate to 7.5 MHz; SPI1BRG = (48000000/(2*desired))-1
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.MODE16 = 1; // 16 bits of data sent per transfer
    SPI1CONbits.ON = 1; // turn on SPI1
    
    RPB15Rbits.RPB15R = 0b0011; //B15 is SS1 output
    RPB13Rbits.RPB13R = 0b0011; //B13 is SDO1 output
    TRISBbits.TRISB15 = 0; //SS1 pin is digital output
//    CS = 0; //Chip Select on DAC requires active low to enable
}

// Send a 16-bit word via SPI
unsigned int SPI1_IO(unsigned int write) {
//    CS = 0; // CS pin on DAC requires active low to enable
    SPI1BUF = write; // send a data byte via SPI
//    CS = 1; // stop transfer of data
    return SPI1BUF; // clear the rx buffer by reading from it
}

// Tell DAC to output analog voltage; channel 0: DAC_A, channel 1: DAC_B
void setVoltage(char channel, char voltage) {
    CS = 0; // CS pin on DAC requires active low to enable
    char configbits = channel << 3 | 0b0011;
    unsigned int word = (configbits << 12) | (voltage << 4);
    SPI1_IO(word);
    CS = 1; // stop transfer of data
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
    
    // do your TRIS and LAT commands here
//    TRISAbits.TRISA4 = 0; // A4 as output
//    LATAbits.LATA4 = 1;   // A4 initially high
    
    // SPI initialize
    initSPI1();
    
    __builtin_enable_interrupts();
    _CP0_SET_COUNT(0);
    while(1) {
        while(PORTBbits.RB4 == 0){
            LATAbits.LATA4 = 0;     // keep LED off when push button is pressed
        }
        
	    
		if (_CP0_GET_COUNT()>6000){ // the core timer runs at half the CPU speed
            LATAbits.LATA4 = !LATAbits.LATA4;   // toggle LED at 2000 Hz
            _CP0_SET_COUNT(0);                  // reset count
        }
    }
    
    
}