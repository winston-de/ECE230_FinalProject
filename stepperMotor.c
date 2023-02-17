/*! \file */
/*!
 * stepperMotor.c
 *
 * Description: Stepper motor ULN2003 driver for MSP432P4111 Launchpad.
 *              Assumes SMCLK configured with 48MHz HFXT as source.
 *              Uses Timer_A3 and P2.7, P2.6, P2.5, P2.4
 *
 *  Created on:
 *      Author: Winston de Jong
 */

#include "stepperMotor.h"
#include "msp.h"
#include <stdbool.h>

#define STEPS_TO_LOCK 1000;

/* Global Variables  */
const uint8_t stepperSequence[STEP_SEQ_CNT] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};
uint16_t stepPeriod = INIT_PERIOD;
uint8_t currentStep = 0;

uint16_t numSteps = 0;
uint16_t targetNumSteps = 0;
bool busy = 0b0;

volatile bool CCW = 0;


void initStepperMotor(void) {
    // set stepper port pins as GPIO outputs
    STEPPER_PORT->SEL0 = (STEPPER_PORT->SEL0) & ~STEPPER_MASK;
    STEPPER_PORT->SEL1 = (STEPPER_PORT->SEL1) & ~STEPPER_MASK;
    STEPPER_PORT->DIR = (STEPPER_PORT->DIR) | STEPPER_MASK;

    // initialize stepper outputs to LOW
    STEPPER_PORT->OUT = (STEPPER_PORT->OUT) & ~STEPPER_MASK;

    /* Configure Timer_A3 and CCR0 */
    // Set period of Timer_A3 in CCR0 register for Up Mode
    TIMER_A3->CCR[0] = stepPeriod;
    // TODO configure CCR0 for Compare mode with interrupt enabled (no output mode - 0)
    TIMER_A3->CCTL[0] = 0x10; // 0b10000

    // Configure Timer_A3 in Stop Mode, with source SMCLK, prescale 48:1, and
    //  interrupt disabled  -  tick rate will be 4MHz (for SMCLK = 48MHz)
    // TODO configure Timer_A3 (requires setting control AND expansion register)
    TIMER_A3->CTL = 0x282;
    TIMER_A3->EX0 = 0b111; // divide prescaler by 8


    /* Configure global interrupts and NVIC */
    // Enable TA3CCR0 compare interrupt by setting IRQ bit in NVIC ISER0 register
    NVIC->ISER[0] |= 0x1 << TA3_0_IRQn;
}

void setStepFrequency(uint8_t rpm) {
    stepPeriod = (INIT_PERIOD)/rpm;
}

void enableStepperMotor(void) {
    // TODO configure Timer_A3 in Up Mode (leaving remaining configuration unchanged)
    TIMER_A3->CTL |= 0x10;
}

void disableStepperMotor(void) {
    // TODO for future driver use (NOT NEEDED FOR EXERCISE)
    //  Configure Timer_A3 in Stop Mode (leaving remaining configuration unchanged)
    TIMER_A3->CTL &= ~(0x10);
}

void stepClockwise(void) {
    currentStep = (currentStep + 1) % STEP_SEQ_CNT;  // increment to next step position
    // TODO update output port for current step pattern
    //  do this as a single assignment to avoid transient changes on driver signals
    STEPPER_PORT->OUT = (P2->OUT & 0x0F) + (stepperSequence[currentStep] << 4);
}

void stepCounterClockwise(void) {
    currentStep = ((uint8_t)(currentStep - 1)) % STEP_SEQ_CNT;  // decrement to previous step position (counter-clockwise)
    STEPPER_PORT->OUT = (P2->OUT & 0x0F) + (stepperSequence[currentStep] << 4);
}

void toggleDirection(void) {
    CCW ^= 1;
}

void unlock(void) {
   targetNumSteps = STEPS_TO_LOCK;
   numSteps = 0;
   enableStepperMotor();
   CCW = 0;
   busy = 1;
}

void lock(void) {
   targetNumSteps = STEPS_TO_LOCK;
   numSteps = 0;
   enableStepperMotor();
   CCW = 1;
   busy = 1;
}

bool isBusy(void) {
    return isBusy;
}


// Timer A3 CCR0 interrupt service routine
void TA3_0_IRQHandler(void)
{
    /* Not necessary to check which flag is set because only one IRQ
     *  mapped to this interrupt vector     */
    if(CCW == 0) {
        stepClockwise();
    } else {
        stepCounterClockwise();
    }

    numSteps++;
    if(numSteps == targetNumSteps) {
        disableStepperMotor();
        busy = 0;
    }

    TIMER_A3->CCR[0] = stepPeriod; // reset timer count

    // clear timer compare flag in TA3CCTL0
    TIMER_A3->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG; // clear bit 1
}
