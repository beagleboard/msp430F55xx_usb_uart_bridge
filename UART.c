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
/*  Adapted from SimpleUSBBackchannel bcUART.c found in MSP430F5529 User's Guide
 *  Luke Ledbetter
 *  Texas Instruments Inc.
 *  July 2021
 *  Built with CCS v10.3
 */
#include <msp430.h>
#include "UART.h"
#include "driverlib.h"
#include "msp430f5529.h"
#include "ringbuf.h"

#define UART_PRESCALAR 208
#define UART_FIRST_MOD 0
#define UART_SECOND_MOD 3

//Receive buffer for the uart, incoming data needs a place to go to otherwise they will be overwritten.
static uint8_t UartRcvBuf[BC_RXBUF_SIZE];
static ringbuf_t rx_ring;

//Initializes the Uart
void UartInit(void)
{
        // P4SEL |= BIT4+BIT5;                          // P4.4,5 = USCI_A1 TXD/RXD
        // UCA1CTL1 |= UCSWRST;                         // **Put state machine in reset**
        // UCA1CTL1 |= UCSSEL_2;                        // SMCLK
        // UCA1BR0 = UCA1_BR0;                          // 24MHz / 16 / 9600 (see User's Guide)
        // UCA1BR1 = UCA1_BR1;
        // UCA1MCTL |= UCA1_OS | UCA1_BRF | UCA1_BRS;   // Modulation UCBRSx=0, UCBRFx=4
        // UCA1CTL1 &= ~UCSWRST;                        // **Initialize USCI state machine**
        // UCA1IE |= UCRXIE;                            // Enable USCI_A1 RX interrupt

        USCI_A_UART_initParam param = {0};

        param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
        param.clockPrescalar = UART_PRESCALAR;
        param.firstModReg = UART_FIRST_MOD;
        param.secondModReg = UART_SECOND_MOD;
        param.parity = USCI_A_UART_NO_PARITY;
        param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
        param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
        param.uartMode = USCI_A_UART_MODE;
        param.overSampling = USCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

        if (STATUS_FAIL == USCI_A_UART_init(USCI_A1_BASE, &param)) {
            return;
        }

        USCI_A_UART_enable(USCI_A1_BASE);
        USCI_A_UART_clearInterrupt(USCI_A1_BASE, USCI_A_UART_RECEIVE_INTERRUPT);

        __enable_interrupt();                        // Enable interrupts globally

        USCI_A_UART_enableInterrupt(USCI_A1_BASE, USCI_A_UART_RECEIVE_INTERRUPT);

        RINGBUF_init(&rx_ring, UartRcvBuf, BC_RXBUF_SIZE);
}

void UartSend(uint8_t* buf, uint16_t len)
{
    uint16_t i = 0;

    while(i < len)
    {
        //Put the bit into the Transmit buffer, then increment i
        UCA1TXBUF = *(buf+(i++));

        //Wait for each bit
        while(!(UCTXIFG==(UCTXIFG & UCA1IFG))&&((UCA1STAT & UCBUSY) == UCBUSY));
    }
}

uint16_t UartReceive(uint8_t* buf, uint16_t len)
{
    uint16_t count;

    if (len == 0) {
        return 0;
    }

    UCA1IE &= ~UCRXIE;

    count = RINGBUF_receiveDataInBuffer(&rx_ring, buf, len);

    //restore interrupts
    UCA1IE |= UCRXIE;

    return count;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCA1IV,4))
          {
            case 0:break;
            case 2:
              RINGBUF_push(&rx_ring, UCA1RXBUF);
             break;
            case 4: break;
            default: break;
          }
}
