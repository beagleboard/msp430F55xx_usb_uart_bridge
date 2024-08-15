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
/*  Adapted from SimpleUSBBackchannel bcUART.h found in MSP430F5529 User's Guide
 *  Luke Ledbetter
 *  Texas Instruments Inc.
 *  July 2021
 *  Built with CCS v10.3
 */

#ifndef UART_H_
#define UART_H_

#include "stdint.h"

// Receive Buffer Size
#define BC_RXBUF_SIZE (128)
// Threshold required to wake the Receive
#define BC_RX_WAKE_THRESH (1)

// Default Baud rate set is 9600
#define UCA1_OS 1 // 1 = oversampling mode, 0 = low-freq mode
#define UCA1_BR0                                                               \
  156 // Value of UCA1BR0 register                    24MHz / 16 / 9600 (see
      // User's Guide)
#define UCA1_BR1 0 // Value of UCA1BR1 register
#define UCA1_BRS                                                               \
  0 // Value of UCBRS field in UCA1MCTL register    Modulation UCBRSx=0,
    // UCBRFx=4
#define UCA1_BRF 4 // Value of UCBRF field in UCA1MCTL register

void UartInit(void);
void UartSend(uint8_t *buf, uint16_t len);
uint16_t UartReceive(uint8_t *buf, uint16_t len);

#endif /* UART_H_ */
