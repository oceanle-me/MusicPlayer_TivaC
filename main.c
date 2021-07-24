

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
//#include "inc/hw_ints.h"
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
#include "driverlib/udma.h"
#include "driverlib/timer.h"
#include "driverlib/ssi.h"
#include "ff.h"

#include "driverlib/pwm.h"

void    PWM0Gen0IntHandler(void);
void    InitPWM(void);
void    InitDebug(void);
void    PlayMusic(void);


void    GPIOIntHandler(void);
void    InitGPIO(void);




FATFS FatFs;        /* FatFs work area needed for each volume */
FIL Fil;            /* File object needed for each open file */
DIR DirMusic;
FILINFO FileInfo;



UINT out_stream (   /* Returns number of bytes sent or stream status */
        const BYTE *p,  /* Pointer to the data block to be sent */
        UINT btf        /* >0: Transfer call (Number of bytes to be sent). 0: Sense call */
);

uint8_t SoundArray[500];
uint8_t * pSA_Write = SoundArray;
uint8_t * pSA_Read = SoundArray;
uint32_t rx_32;
uint32_t u32CountWriteSoundArray;
uint32_t u32CountReadSoundArray;
bool writeEnable = true;

int main() {
    SysCtlClockSet(SYSCTL_SYSDIV_3 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    InitGPIO();


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);

    InitDebug();
    InitPWM();

    FRESULT fr;


    if(f_mount(&FatFs, "", 0)==FR_OK)     /* Give a work area to the default drive */{
        fr = f_findfirst(&DirMusic,&FileInfo, "0:","?*.WAV");
        if(fr==FR_OK)
        {
            DBG("Connection succeed! Open 0: directory  \n");
            while (fr == FR_OK && FileInfo.fname[0]) {         /* Repeat while an item is found */
//                DBG("%s\n", FileInfo.fname);                /* Print the object name */
                //                PlayMusic(&FileInfo.fname);
                fr = f_findnext(&DirMusic, &FileInfo);               /* Search for next item */

//                fr = f_findnext(&DirMusic, &FileInfo);
//                fr = f_findnext(&DirMusic, &FileInfo);
//                fr = f_findnext(&DirMusic, &FileInfo);
//                fr = f_findnext(&DirMusic, &FileInfo);
                PlayMusic();
            }
            f_closedir(&DirMusic);
            DBG("Close 0: directory  \n");
            for (;;) ;
        }

    }

        DBG("Connection failed! Please Check your hardware and reset it!\n");
    for (;;) ;
}


UINT out_stream (   /* Returns number of bytes sent or stream status */
        const BYTE *p,  /* Pointer to the data block to be sent */
        UINT btf        /* >0: Transfer call (Number of bytes to be sent). 0: Sense call */
)
{
    UINT cnt = 0;
    if (btf == 0) {     /* Sense call */
        /* Return stream status (0: Busy, 1: Ready) */
        /* When once it returned ready to sense call, it must accept a byte at least */
        /* at subsequent transfer call, or f_forward will fail with FR_INT_ERR. */
        //        if (FIFO_READY) cnt = 1;
        cnt = 1;
    }
    else {              /* Transfer call */
        do {    /* Repeat while there is any data to be sent and the stream is ready */
            //           FIFO_PORT = *p++;
            //            UARTCharPut(UART0_BASE, *p++);
            *pSA_Write= *p++;
            pSA_Write++;
            cnt++;
            u32CountWriteSoundArray++;
//            If FatFs has written data into SoundArray[500] buffer fully,
//            then wait until all the data has been read by PWM IntHandler.
//            Because speed of PWM reload is much slower than speed of reading SDcard.
            if(u32CountWriteSoundArray==500){
                writeEnable=false;
                while(!writeEnable) {}  //stop here if not writeEnable
                pSA_Write=SoundArray;
                u32CountWriteSoundArray=0;
            }

            while(0);

        }
        while (cnt < btf /*&& FIFO_READY*/);
    }

    return cnt;
}

void    PWM0Gen0IntHandler(void){
    //each 1/16KHz , PWM will reload the 8-bit value into PWM pulse width register.
    PWMGenIntClear(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_LOAD);
    u32CountReadSoundArray++;
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, *pSA_Read);
    pSA_Read++;
    //  if PWMIntHandler has read all the data from SoundArray[500] completely,
    //  then enable FatFs to read data from SDCard and reload SoundArray[500]
    //  Reset value of variables to PWM reread the new data from SoundArray.

    if (u32CountReadSoundArray == 500){
        writeEnable=true;
        u32CountReadSoundArray=0;
        pSA_Read=SoundArray;
    }
}

void    InitPWM(void)
{
//      System clock with PLL is 200MHz/3; SYSCTL_SYSDIV_3
//      with SYSCTL_PWMDIV_16, we have PWM Clock is 200MHz/3/16, thus a period is 0.24us.
//      With 8-bit resolution, the max value 2^8 is about 260.
//      So, 1/ (260*0.24us) = 16KHz = sample rate of our music player
//      After the above calculation, we reached a solution for Tiva C launch pad to play audio 8bit - 16Khz
    SysCtlPWMClockSet(SYSCTL_PWMDIV_16);//
    //
    // The PWM peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);

    //
    // Configure PWM0 to count down without synchronization.
    //
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
                    PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

//  Sample rate is 16Khz
    DBG("Clock PWM= %d \n",(SysCtlClockGet()/16 / 16000));
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, (SysCtlClockGet()/16 / 16000));
    //16KHz equivalents  260 steps, 1/16KHz = 260*0.24s
    //

    //
    //200 steps
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, 1);


    IntMasterEnable();
    PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);

    //
    // Enable the PWM0 LOAD interrupt on PWM Gen 0.
    //
    PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_LOAD);

    //
    // Enable the interrupt for PWM Gen 0 on the processor (NVIC).
    //
    IntEnable(INT_PWM0_0);

    //
    // Enable the PWM Out Bit 0 (PB6) output signal.
    //
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);

}
void 	InitDebug(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, SysCtlClockGet());
    DBG("Debug using UARTprintf\n");
}
void    PlayMusic(void){

    if (f_open(&Fil, FileInfo.fname, FA_READ) == FR_OK) {
        DBG("Open File %s\n",FileInfo.fname);

        audioState = ON_AUDIO;
        PWMGenEnable(PWM0_BASE, PWM_GEN_0);

        f_forward(&Fil, out_stream, f_size(&Fil) ,(UINT*) &rx_32);

        f_close(&Fil);                         /* Close the file */
        DBG("Close File %s\n",FileInfo.fname);

        PWMGenDisable(PWM0_BASE, PWM_GEN_0);
        audioState = OFF_AUDIO;
    }
    else {
        DBG("Could not open file %s\n",FileInfo.fname);
    }
}




//GPIO initialization and interrupt enabling
void InitGPIO(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    //unlock GPIO F0
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; ////#include "inc/hw_gpio.h"
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4 );

    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4);
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);

    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);


    IntPrioritySet(INT_GPIOF, 0);
    IntRegister(INT_GPIOF, GPIOIntHandler);
    IntEnable(INT_GPIOF);
    IntMasterEnable();
}

//Interrupt service routine
void GPIOIntHandler(void){
    // On/Off audio if press switch 1
    // play next song if press swith 2

    uint32_t intGPIOStatus;
    IntMasterDisable();

    SysCtlDelay((SysCtlClockGet()/3/1000)*40); //avoid vibration button within 40ms
    intGPIOStatus = GPIOIntStatus(GPIO_PORTF_BASE, true);
    GPIOIntClear(GPIO_PORTF_BASE, intGPIOStatus);

    if (intGPIOStatus == GPIO_INT_PIN_4 ){//SW1
        if(audioState==ON_AUDIO)  {
            audioState=OFF_AUDIO;
            PWMGenDisable(PWM0_BASE, PWM_GEN_0);
        }
        else if(audioState==OFF_AUDIO) {
            audioState=ON_AUDIO;
            PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        }
        GPIO_PORTF_DATA_R ^= 0b100;
    }
    else if (intGPIOStatus == GPIO_INT_PIN_0 ){//SW2
        audioState = NEXT_AUDIO;
        PWMGenEnable(PWM0_BASE, PWM_GEN_0);
        GPIO_PORTF_DATA_R ^= 0b10;
    }

    while(!(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0))) ; //wait here until the button is released. press=0
    SysCtlDelay((SysCtlClockGet()/3/1000)*40); //avoid vibration button within 40ms
    GPIOIntClear(GPIO_PORTF_BASE, intGPIOStatus);

    IntMasterEnable();
}




