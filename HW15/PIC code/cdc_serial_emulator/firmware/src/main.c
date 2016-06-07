/*******************************************************************************
  MPLAB Harmony Project Main Source File

  Company:
    Microchip Technology Inc.
  
  File Name:
    main.c

  Summary:
    This file contains the "main" function for an MPLAB Harmony project.

  Description:
    This file contains the "main" function for an MPLAB Harmony project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state 
    machines of all MPLAB Harmony modules in the system and it calls the 
    "SYS_Tasks" function from within a system-wide "super" loop to maintain 
    their correct operation. These two functions are implemented in 
    configuration-specific files (usually "system_init.c" and "system_tasks.c")
    in a configuration-specific folder under the "src/system_config" folder 
    within this project's top-level folder.  An MPLAB Harmony project may have
    more than one configuration, each contained within it's own folder under
    the "system_config" folder.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

//Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "system/common/sys_module.h"   // SYS function prototypes

// Initialize PWM's
void initPWM() {
    RPA0Rbits.RPA0R = 0b0101;   //set A0 as OC1
    RPA1Rbits.RPA1R = 0b0101;   //set A1 as OC2
//    RPA4Rbits.RPA4R = 0b0101;   //set A4 (LED) as OC4
    
    T2CONbits.TCKPS = 0b100; //Timer2 prescaler N=16 (1:16)
    PR2 = 2999; //period = (PR2+1) * N * 20.833 ns = 1000 us => 1 kHz
    TMR2 = 0; // initial Timer2 count is 0
//    IPC2bits.T2IP = 5; // priority for Timer2 interrupt
//    IFS0bits.T2IF = 0; // clear Timer2 interrupt flag
//    IEC0bits.T2IE = 1; // enable Timer2 interrupt
    T2CONbits.ON = 1; // turn on Timer2
    
    OC1CONbits.OCTSEL = 0; //use Timer 2
    OC1CONbits.OCM = 0b110; //PWM mode without fault pin; other OC1CON bits are defaults
    OC1RS = 0; // initialize duty cycle = OC1RS/(PR2+1) = 50%
    OC1R = 0; // initialize before turning OC1 on; afterward it is read-only
    OC1CONbits.ON = 1; // turn on OC1
    
    OC2CONbits.OCTSEL = 0; //use Timer 2
    OC2CONbits.OCM = 0b110; //PWM mode without fault pin; other OC2CON bits are defaults
    OC2RS = 0; // initialize duty cycle = OC2RS/(PR2+1) = 50%
    OC2R = 0; // initialize before turning OC2 on; afterward it is read-only
    OC2CONbits.ON = 1; // turn on OC2
    
//    OC4CONbits.OCTSEL = 0; //use Timer 2
//    OC4CONbits.OCM = 0b110; //PWM mode without fault pin; other OC4CON bits are defaults
//    OC4RS = 1500; // initialize duty cycle = OC4RS/(PR2+1) = 50%
//    OC4R = 1500; // initialize before turning OC4 on; afterward it is read-only
//    OC4CONbits.ON = 1; // turn on OC4
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    __builtin_disable_interrupts();
    
    /* Initialize all MPLAB Harmony modules, including application(s). */
    SYS_Initialize ( NULL );
    
    
    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // TRIS and LAT commands for LED indicator and motor directions
    TRISAbits.TRISA4 = 0; // A4 as output
    LATAbits.LATA4 = 0;   // LED initially OFF
    
    TRISBbits.TRISB7 = 0; // B7 as output -- LEFT WHEEL
    LATBbits.LATB7 = 0;   // B7 initially -- FWD
    
    TRISBbits.TRISB9 = 0; // B9 as output -- RIGHT WHEEL
    LATBbits.LATB9 = 0;   // B9 initially -- FWD

    initPWM();
    
    __builtin_enable_interrupts();

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );

    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

