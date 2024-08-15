/* --COPYRIGHT--,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
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
/*
 * ======== hal.c ========
 *
 */
#include "msp430.h"

#include "driverlib.h"

#include "USB_API/USB_Common/device.h"
#include "USB_config/descriptors.h"

#include "hal.h"

#define GPIO_ALL                                                               \
  GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 |      \
      GPIO_PIN6 | GPIO_PIN7

/*
 * This function drives all the I/O's as output-low, to avoid floating inputs
 * (which cause extra power to be consumed).  This setting is compatible with
 * TI FET target boards, the F5529 Launchpad, and F5529 Experimenters Board;
 * but may not be compatible with custom hardware, which may have components
 * attached to the I/Os that could be affected by these settings.  So if using
 * other boards, this function may need to be modified.
 */
void USBHAL_initPorts(void) {
  // Unused pins, low power state
  GPIO_setOutputLowOnPin(GPIO_PORT_P1,
                         GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);
  GPIO_setAsOutputPin(GPIO_PORT_P1,
                      GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);
  GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
  GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);
  GPIO_setOutputLowOnPin(GPIO_PORT_PJ,
                         GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);
  GPIO_setAsOutputPin(GPIO_PORT_PJ,
                      GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);
  GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0);
  GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0);

  // LEDs
  GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);
  GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3);

  // Buzzer
  GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN3);
  GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN3);

  // I2C
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                             GPIO_PIN1 | GPIO_PIN2);

  // SPI, Not used by MSP430
  GPIO_setAsInputPin(GPIO_PORT_P1,
                     GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);

  // CC1352R RESET_N - Default as input, set output low for reset operation
  GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN0);
  GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);

  // CC1352R UART
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                             GPIO_PIN4 | GPIO_PIN5);

  // I2C
  // GPIO_setAsInputPin(    GPIO_PORT_P4, GPIO_PIN1|GPIO_PIN2);
  //  GPIO_setAsInputPinWithPullDownResistor(    GPIO_PORT_P4,
  //  GPIO_PIN1|GPIO_PIN2);

  // CC1352R BOOT_N - Default as input, set output low for boot operation
  GPIO_setAsInputPin(GPIO_PORT_P4, GPIO_PIN7);

  GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6 | GPIO_PIN7);

  // CC1352R UART CTRL - default disconnected, HIGH to connect
  GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN6);
}

void USBHAL_initClocks(uint32_t mclkFreq) {
  UCS_initClockSignal(UCS_FLLREF, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);

  UCS_initClockSignal(UCS_ACLK, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);

  UCS_initFLLSettle(mclkFreq / 1000, mclkFreq / 32768);
}
// Released_Version_5_20_06_02

void hal_ext_uart(uint8_t active) {
  if (active) {
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
    USCI_A_UART_enableInterrupt(USCI_A1_BASE, USCI_A_UART_RECEIVE_INTERRUPT);
  } else {
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);
    USCI_A_UART_disableInterrupt(USCI_A1_BASE, USCI_A_UART_RECEIVE_INTERRUPT);
  }
}

uint8_t hal_ext_boot_read(void) {
  return GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7);
}

void hal_ext_reset(uint8_t active) {
  if (active) {
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
  } else {
    GPIO_setAsInputPin(GPIO_PORT_P2, GPIO_PIN0);
  }
}

void hal_ext_boot(uint8_t active) {
  if (active) {
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN7);
  } else {
    GPIO_setAsInputPin(GPIO_PORT_P4, GPIO_PIN7);
  }
}

void hal_ext_led(int led, int state) {
  switch (led) {
  case 1:
    if (state)
      GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN2);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN2);
    break;
  case 2:
    if (state)
      GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN3);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN3);
    break;
  case 3:
    /* Labeled on schematic as 900M */
    if (state)
      GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN1);
    else
      GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN1);
    break;
  }
}

void hal_ext_i2c(void) {
  // Assign I2C pins to USCI_B0
  GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4,
                                             GPIO_PIN1 | GPIO_PIN2);

  // Initialize I2C as a slave device
  USCI_B_I2C_initSlave(USCI_B1_BASE, SLAVE_ADDRESS);

  // Set in receive mode
  USCI_B_I2C_setMode(USCI_B1_BASE, USCI_B_I2C_RECEIVE_MODE);

  // Enable I2C Module to start operations
  USCI_B_I2C_enable(USCI_B1_BASE);

  // Enable receive interrupt
  USCI_B_I2C_clearInterrupt(USCI_B1_BASE, USCI_B_I2C_RECEIVE_INTERRUPT +
                                              USCI_B_I2C_STOP_INTERRUPT);
  USCI_B_I2C_enableInterrupt(USCI_B1_BASE, USCI_B_I2C_RECEIVE_INTERRUPT +
                                               USCI_B_I2C_STOP_INTERRUPT);
}

void hal_ext_buzzer(int state) {
  switch (state) {
  case 0: /* Buzz short and high */
    for (int i = 0; i < 5; i++) {
      GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN3);
      __delay_cycles(100);
      GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN3);
      __delay_cycles(100);
    }
    break;
  case 1: /* Buzz medium */
    for (int i = 0; i < 20; i++) {
      GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN3);
      __delay_cycles(10000);
      GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN3);
      __delay_cycles(10000);
    }
    break;
  case 2: /* Off */
    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN3);
    break;
  case 3: /* On */
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN3);
    break;
  }
}

void hal_ext_i2c_handle(enum i2c_cmd cmd) {
  switch (cmd) {
  case BUZZER_SHORT:
    hal_ext_buzzer(0);
    break;
  case BUZZER_LONG:
    hal_ext_buzzer(1);
    break;
  case LED1_ON:
    hal_ext_led(1, 1);
    break;
  case LED1_OFF:
    hal_ext_led(1, 0);
    break;
  case LED2_ON:
    hal_ext_led(2, 1);
    break;
  case LED2_OFF:
    hal_ext_led(2, 0);
    break;
  case LED3_ON:
    hal_ext_led(3, 1);
    break;
  case LED3_OFF:
    hal_ext_led(3, 0);
    break;
  default:
    break;
  }
}

//******************************************************************************
//
// This is the USCI_B0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(USCI_B1_VECTOR))) USCI_B1_ISR(void)
#endif
{
  switch (__even_in_range(UCB1IV, 12)) {
  case USCI_I2C_UCSTPIFG:
    // Exit LPM0 if data was transmitted
    __bic_SR_register_on_exit(LPM0_bits);
    __no_operation();
    break;
  case USCI_I2C_UCNACKIFG: // NAK received (master only)
    __no_operation();
    break;
  // Vector 10: Data received - RXIFG
  case USCI_I2C_UCRXIFG: {
    // Get received data
    uint8_t bytesReceived = USCI_B_I2C_slaveGetData(USCI_B1_BASE);
    hal_ext_i2c_handle(bytesReceived);
    break;
  }
  case USCI_I2C_UCTXIFG:
    break;
  default:
    break;
  }
}
