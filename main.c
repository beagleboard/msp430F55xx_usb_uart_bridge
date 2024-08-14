/* --COPYRIGHT--,BSD
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
/* ****************************************************************************
 * USB To UART Bridge
 *
 * This example simply takes the input from one communication channel
 * and sends it to the other. So if the signal is received via USB it will
 * be sent to UART and vice versa. This demo uses CDC USB protocol, if HID or
 * another protocol is needed then changes will need to be made to the USB
 * functions called and included files.
 *
 * ----------------------------------------------------------------------------
 * Clock Configuration
 * ----------------------------------------------------------------------------
 * ACLK = default REFO ~32768 Hz, SMCLK = 24MHz
 * At this speed the voltage core needs to be set to 3
 * Clock ranges from 20Mhz to 25Mhz, lower speeds could cause data issues
 *
 * ----------------------------------------------------------------------------
 * Baud Rate Configuration
 * ----------------------------------------------------------------------------
 * Baud rate is configured in the UART.h file and can be updated by changing
 * the appropriate constants there. More information is in the application brief.
 *
 * Baud rate is default configured to 9600.
 * Baud rate ranges from 9600-11520, rates higher or lower could cause data issues
 *
 * ----------------------------------------------------------------------------
 * GUI Usage
 * ----------------------------------------------------------------------------
 * No additional setup in the code is required for the GUI.
 * Follow the ReadMe included in the GUI to find the setup for GUI operation.
 * For additional details on working the GUI see the included video and application brief.
 *
 * ----------------------------------------------------------------------------
 * Stand Alone Usage
 * ----------------------------------------------------------------------------
 * To use this as a stand alone, run the code then open 2 separate terminals (PuTTy, Docklight, etc.)
 * Connect one to the USB COM port (USB will only appear when program starts)
 * and the other to the UART COM port (COM ports are found in the device manager).
 * UART terminal connection needs a baud rate of 9600. Type on the USB terminal and the result
 * will appear on the UART terminal. Type on the UART terminal and the result will appear on the
 * USB terminal.
 * More information can be found in the application brief.
 *
 *           MSP430F5529
 *         ---------------
 *     /|\|               |
 *      | |               |
 *      --| RST           |
 *        |               |
 *        |   P4.4/UCA1TXD|---> USB
 *        |   P4.5/UCA1RXD|<--- UART
 *        |               |
 *
 *  Luke Ledbetter
 *  Texas Instruments Inc.
 *  July 2021
 *  Built with CCS v10.3
 */
#include <string.h>

#include "driverlib.h"

#include "USB_config/descriptors.h"
#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/usb.h"                 // USB-specific functions
#include "USB_API/USB_CDC_API/UsbCdc.h"             // USB CDC Protocol is used
#include "USB_app/usbConstructs.h"

/*
 * NOTE: Modify hal.h to select a specific evaluation board and customize for
 * your own board.
 */

#include "hal.h"
#include "UART.h"
// Global flags set by events
volatile uint8_t bCDCDataReceived_event = FALSE;  // Flag set by event handler to 
                                                  // indicate data has been
                                                  // received into USB buffer

#define BUFFER_SIZE 128

static uint8_t usbToUart[BUFFER_SIZE], uartToUsb[BUFFER_SIZE];


/*----------------------------------------------------------------------------+
 | Main Routine                                                                |
 +----------------------------------------------------------------------------*/
void main (void)
{
    uint16_t count;
    uint16_t bytesSent, bytesReceived, lastCount = 0;
    uint8_t ret;

    WDT_A_hold(WDT_A_BASE);         // Stop watchdog timer

    PMM_setVCore(PMM_CORE_LEVEL_3); // Minimum Vcore setting required for the USB API is PMM_CORE_LEVEL_2 .
                                    // Need PMM_CORE_LEVEL_3 for 24MHz clock speed
    USBHAL_initPorts();             // Config GPIOS for low-power (output low)
    USBHAL_initClocks(24000000);    // Config clocks. MCLK=SMCLK=FLL=24MHz; ACLK=REFO=32kHz
    // hal_ext_i2c();
    USB_setup(TRUE, TRUE);          // Init USB & events; if a host is present, connect
    UartInit();

    hal_ext_led(3, 0);
    hal_ext_uart(FALSE);

    while (1)
        {
            if (lastCount) {
                ret = USBCDC_sendData(uartToUsb, lastCount, CDC0_INTFNUM);
                if (ret == USBCDC_SEND_STARTED || ret == USBCDC_SEND_COMPLETE) {
                    lastCount = 0;
                }
            } else if (!(USBCDC_getInterfaceStatus(CDC0_INTFNUM,&bytesSent,&bytesReceived) & USBCDC_WAITING_FOR_SEND)) {
                count = UartReceive(uartToUsb, BUFFER_SIZE);
                ret = USBCDC_sendData(uartToUsb, count, CDC0_INTFNUM);
                if (!(ret == USBCDC_SEND_STARTED || ret == USBCDC_SEND_COMPLETE)) {
                    lastCount = count;
                }
            }

            // Check the USB state and directly main loop accordingly
            switch (USB_getConnectionState())
            {
                // This case is executed while your device is enumerated on the
                // USB host
                case ST_ENUM_ACTIVE:
                    hal_ext_led(3, 1);
                    hal_ext_uart(TRUE);
                    // Exit LPM because of a data-receive event, and
                    // fetch the received data
                    if (bCDCDataReceived_event){

                        // Clear flag early -- just in case execution breaks
                        // below because of an error
                        bCDCDataReceived_event = FALSE;

                        count = USBCDC_receiveDataInBuffer(usbToUart, BUFFER_SIZE, CDC0_INTFNUM);
                        // Count has the number of bytes received into dataBuffer
                        if (count){
                           //Send the count to the UART
                           UartSend(usbToUart, count);
                        }
                    }
                    break;

                // These cases are executed while your device is disconnected from
                // the host (meaning, not enumerated); enumerated but suspended
                // by the host, or connected to a powered hub without a USB host
                // present.
                case ST_PHYS_DISCONNECTED:
                case ST_ENUM_SUSPENDED:
                case ST_PHYS_CONNECTED_NOENUM_SUSP:
                    __bis_SR_register(LPM3_bits + GIE);
                    _NOP();
                    break;

                // The default is executed for the momentary state
                // ST_ENUM_IN_PROGRESS.  Usually, this state only last a few
                // seconds.  Be sure not to enter LPM3 in this state; USB
                // communication is taking place here, and therefore the mode must
                // be LPM0 or active-CPU.
                case ST_ENUM_IN_PROGRESS:
                default:;
            }

        }  //while(1)
}                               // main()

/*  
 * ======== UNMI_ISR ========
 */
#if defined(__TI_COMPILER_VERSION__) || (__IAR_SYSTEMS_ICC__)
#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR (void)
#elif defined(__GNUC__) && (__MSP430__)
void __attribute__ ((interrupt(UNMI_VECTOR))) UNMI_ISR (void)
#else
#error Compiler not found!
#endif
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG ))
    {
        case SYSUNIV_NONE:
            __no_operation();
            break;
        case SYSUNIV_NMIIFG:
            __no_operation();
            break;
        case SYSUNIV_OFIFG:
            UCS_clearFaultFlag(UCS_XT2OFFG);
            UCS_clearFaultFlag(UCS_DCOFFG);
            SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
            break;
        case SYSUNIV_ACCVIFG:
            __no_operation();
            break;
        case SYSUNIV_BUSIFG:
            // If the CPU accesses USB memory while the USB module is
            // suspended, a "bus error" can occur.  This generates an NMI.  If
            // USB is automatically disconnecting in your software, set a
            // breakpoint here and see if execution hits it.  See the
            // Programmer's Guide for more information.
            SYSBERRIV = 0; // clear bus error flag
            USB_disable(); // Disable
    }
}

