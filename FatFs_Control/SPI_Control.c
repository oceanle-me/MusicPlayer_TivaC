/*
 * SPI_Control.c
 *
 *  Created on: 26 thg 6, 2021
 *      Author: Dell
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/tm4c123gh6pm.h"
#include "debug.h"

#include "driverlib/ssi.h"
#include "SPI_Control.h"
//! - SoftSSIClk - PA2      PushPull
//! - SoftSSIFss - PA3      Pull up
//! - SoftSSIRx  - PA4      DO Pull-up  MISO
//! - SoftSSITx  - PA5      DI PushPull MOSI
void DelayUs (uint32_t n){
    SysCtlDelay(SysCtlClockGet()/1000/1000*n/3);
}
void DelayMs (uint32_t n){
    SysCtlDelay(SysCtlClockGet()/1000*n/3);
}
void SPI_Init(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);

    GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_4|GPIO_PIN_5 | GPIO_PIN_3| GPIO_PIN_2,
                GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD); //Push Pull

  GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_3 |GPIO_PIN_4,
  GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);

    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
  // GPIOPinConfigure(GPIO_PA3_SSI0FSS);

    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 |
                   GPIO_PIN_4 | GPIO_PIN_2);

    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER, 200000, 8);

    SSIEnable(SSI0_BASE);

 // Read any residual data from the SSI port.
    uint32_t u8Data_Temp5457542;
    while(SSIDataGetNonBlocking(SSI0_BASE,&u8Data_Temp5457542 ))
    {
    }
}

