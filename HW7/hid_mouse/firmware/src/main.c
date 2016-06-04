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
//#include "ILI9163C.h"    // LCD library
#include "i2c.h"         // I2C2 lib
#include "imu.h"         // LSM6DS33 IMU lib

volatile short temp_raw, accX_raw, accY_raw, accZ_raw, gyroX_raw, gyroY_raw, gyroZ_raw;
//#define MAX_LENGTH 50   // max string length to be printed on LCD at once

// Initialize I2C2
void initI2C2() {
    // turn off analog on I2C pins
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    i2c_master_setup();
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all MPLAB Harmony modules, including application(s). */
    SYS_Initialize ( NULL );

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
//    SPI1_init();
//    LCD_init();
    
    __builtin_enable_interrupts();
    
//    LCD_clearScreen(BLACK);   
//    char message[MAX_LENGTH];  
    unsigned char data[14];
//    float accX, accY;
    _CP0_SET_COUNT(0);
    while ( true )
    {
        if (_CP0_GET_COUNT() > 480000) {    // 50 Hz     
            i2c_read_multiple(IMU_ADDRESS, OUT_TEMP_L, data, 14);
            temp_raw = data[1] << 8 | data[0];
            gyroX_raw = data[3] << 8 | data[2];
            gyroY_raw = data[5] << 8 | data[4];
            gyroZ_raw = data[7] << 8 | data[6];
            accX_raw = data[9] << 8 | data[8];
            accY_raw = data[11] << 8 | data[10];
            accZ_raw = data[13] << 8 | data[12];
            
//            accX = accX_raw * 2.0 / 32768; // accel in g
//            accY = accY_raw * 2.0 / 32768; // accel in g
//            accZ = accZ_raw * 2.0 / 32768; // accel in g
            
//            sprintf(message, "temp raw: %x    ", temp_raw);
//            LCD_drawString(10, 10, message, WHITE);
//            
//            sprintf(message, "accX: %f g    ", accX);
//            LCD_drawString(10, 20, message, WHITE);
//            
//            sprintf(message, "accY: %f g    ", accY);
//            LCD_drawString(10, 30, message, WHITE);
//            
//            sprintf(message, "accZ: %f g    ", accZ);
//            LCD_drawString(10, 40, message, WHITE);
//            
//            sprintf(message, "gyroX raw: %i    ", gyroX_raw);
//            LCD_drawString(10, 50, message, WHITE);
//            
//            sprintf(message, "gyroY raw: %i    ", gyroY_raw);
//            LCD_drawString(10, 60, message, WHITE);
//            
//            sprintf(message, "gyroZ raw: %i    ", gyroZ_raw);
//            LCD_drawString(10, 70, message, WHITE);
            
            _CP0_SET_COUNT(0);
        }
        
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );

    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

