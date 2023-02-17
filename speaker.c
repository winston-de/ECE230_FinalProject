/*
 * speaker.c
 *
 *  Created on: Feb 16, 2023
 *      Author: dejongwm
 */
#include "msp.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include "csHFXT.h"

/* TODO add note defines */
#define NOTECNT 3

#define SpeakerPort P2
#define Speaker BIT4  //P2.4
//48MHz system clock and 4:1 prescaler
//12MHz Timer A clock
//Half period count = (48,000,000/Note frequency)
#define SMCLK 48000000 //Hz
#define TimerA0Prescaler 4 //Timer A prescaler
#define TimerA0Clock  SMCLK/TimerA0Prescaler

#define ACLK 32768 //Hz
#define FULL_NOTE       ACLK    //1 second

// Note A4 - 440 Hz, B4 - 493.88 Hz, C5 - 523.26 Hz
#define NOTEA4  27273
#define NOTEB4  24297
#define NOTEC5  22933


void initSpeaker(void) {
    SpeakerPort->DIR |= Speaker;            // set P2.4 as output
    SpeakerPort->SEL0 |= Speaker;           // P2.4 set to TA0.1
    SpeakerPort->SEL1 &= ~Speaker;  // Option 0b01
    //set Timer A0 CCR0 compare for period and CCR1 compare for duty cycle of a note

    // initialize timer A0 to count up, repeat, and output a signal, without interrupts
    TIMER_A0->CCTL[1] = 0x60;
    TIMER_A0->CTL = 0x294;

    /* Configure Timer_A1 and CCRs */
    // Set initial period in CCR0 register. This assumes timer starts at 0
    TIMER_A1->CCR[0] = 0;
    // Configure CCR0 for Compare mode with interrupt enabled (no output mode - 0)
    TIMER_A1->CCTL[0] = 0x0010;
    // Configure Timer_A1 in UP Mode with source ACLK prescale 1:1 and no interrupt
    TIMER_A1->CTL = 0b0000000100010100;  //0x0114

    /* Configure global interrupts and NVIC */
    // Enable TA1 TA1CCR0 compare interrupt
    NVIC->ISER[0] |= (1) << TA1_0_IRQn;
}

void playNote(unsigned int note, unsigned int length) {
    TIMER_A0->CCR[0] = note - 1;
    // Set high pulse-width in CCR1 register (determines duty cycle)
    TIMER_A0->CCR[1] = (note / 2) - 1;

    TIMER_A1->CCR[0] += length;

    TIMER_A1->CTL |= (0b010000);
    TIMER_A0->CTL |= (0b010000);
}

/* Timer_A1 CCR0 interrupt service routine, i.e., the handler */
//Timer A1 Up CCR0 compare interrupt to generate note duration
//by setting up TIMER_A1->CCR[0].
//The same compare interrupt can be used to generate 100ms rest
//by turning off Speaker for 100ms with a compare interrupt
//toggle LED1
void TA1_0_IRQHandler(void)
{
    /* Check if interrupt triggered by CCR0 */

    TIMER_A1->CTL &= ~(0b110000); // stop timer A1
    TIMER_A0->CTL &= ~(0b110000); // stop timer A0

    //clear interrupt flag
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
}

