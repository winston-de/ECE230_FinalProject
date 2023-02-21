/*
 * bluetooth.c
 *
 *  Created on: Feb 17, 2023
 *      Author: dejongwm
 */
#include "msp.h"

#define BT_PORT P3
#define RX_MASK BIT3
#define TX_MASK BIT2


void initBluetooth(void) {
    /* Stop Watchdog timer */
     WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

     /* Configure UART pins */
     P3->SEL0 |= RX_MASK | TX_MASK;                // set 2-UART pins as secondary function
     P3->SEL1 &= ~(RX_MASK | TX_MASK);

     /* Configure UART
      *  Asynchronous UART mode, 8N1 (8-bit data, no parity, 1 stop bit),
      *  LSB first, SMCLK clock source
      */
     EUSCI_A2->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
     EUSCI_A2->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
             EUSCI_A_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK

     /* Baud Rate calculation
      * Refer to Section 24.3.10 of Technical Reference manual
      * BRCLK = 48000000, Baud rate = 9600
      * N = fBRCLK / Baud rate = 48000000/9600 = 5000
      * from Technical Reference manual Table 24-5:
      *
      * lookup values for UCOS16, UCBRx, UCBRFx, and UCBRSx in Table 24-5
      * UCOS16 = 1, UCBRx = 312, UCBRFx = 8, UCBRSx = 0
      */
     // set clock prescaler in EUSCI_A2 baud rate control register
     EUSCI_A2->BRW = 312;

     // configure baud clock modulation in EUSCI_A2 modulation control register
     EUSCI_A2->MCTLW = 0x81;


     EUSCI_A2->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;    // Initialize eUSCI
     EUSCI_A2->IFG &= ~EUSCI_A_IFG_RXIFG;        // Clear eUSCI RX interrupt flag
     EUSCI_A2->IE |= EUSCI_A_IE_RXIE;            // Enable USCI_A2 RX interrupt


     // Enable eUSCIA2 interrupt in NVIC module
     NVIC->ISER[0] = (1 << EUSCIA2_IRQn );
}

void printMessage(const char* message, int msgLength) {
    int i;
    for (i = 0; i < msgLength; i++) {
        // Check if the TX buffer is empty first
        while(!(EUSCI_A2->IFG & EUSCI_A_IFG_TXIFG));

        // Send next character of message
        //  Note that writing to TX buffer clears the flag
        EUSCI_A2->TXBUF = message[i];
    }
}

int8_t digitsReceived = 0;

// UART interrupt service routine
void EUSCIA2_IRQHandler(void)
{
    // Check if receive flag is set (value ready in RX buffer)
    if (EUSCI_A2->IFG & EUSCI_A_IFG_RXIFG)
    {
        // Note that reading RX buffer clears the flag
        uint8_t digit = EUSCI_A2->RXBUF;
        digitsReceived++;
        // Echo character back to screen, otherwise user will not be able to
        //  verify what was typed
        while(!(EUSCI_A2->IFG & EUSCI_A_IFG_TXIFG)); // Wait for TX buffer ready
        EUSCI_A2->TXBUF = digit;                 // Echo character to terminal
    }
}
