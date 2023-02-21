/*! \file */
/*!
 * servoDriver.c
 *
 * Description: Servo motor driver for MSP432P4111 Launchpad.
 *              Assumes SMCLK configured with 48MHz HFXT as source.
 *              Uses Timer_A2 and P5.6 (TA2.1)
 *
 *  Created on:
 *      Author:
 */

#include "servoDriver.h"
#include "msp.h"

/* Global Variables  */

uint16_t pulseWidthTicks = SERVO_MIN_ANGLE;

#define UNLOCKED_ANGLE 120
#define LOCKED_ANGLE 0

#define SERVO_PORT P5
#define SERVO_BIT BIT6


void initServoMotor(void) {
    // TODO configure servo pin (P5.6) for primary module function (TA2.1),
    //  output, initially LOW

    SERVO_PORT->DIR |= SERVO_BIT;
    SERVO_PORT->SEL1 &= ~(SERVO_BIT);
    SERVO_PORT->SEL0 |= SERVO_BIT;
    SERVO_PORT->OUT &= ~(SERVO_BIT); // clear output


    /* Configure Timer_A2 and CCR1 */
    // Set period of Timer_A2 in CCR0 register for Up Mode
    TIMER_A2->CCR[0] = SERVO_TMR_PERIOD;
    // Set initial positive pulse-width of PWM in CCR1 register
    TIMER_A2->CCR[1] = SERVO_MIN_ANGLE;

    // TODO configure TA2CCR1 for Compare mode, Reset/Set output mode, with interrupt disabled
    TIMER_A2->CCTL[1] = 0b11100000;

    // Configure Timer_A2 in Up Mode, with source SMCLK, prescale 16:1, and
    //  interrupt disabled  -  tick rate will be 3MHz (for SMCLK = 48MHz)
    // TODO configure Timer_A2 (requires setting control AND expansion register)
    TIMER_A2->CTL = 0b1011010000; // prescale of 8
    TIMER_A2->EX0 = 0b001; // divide prescale by 2

}

void incrementTenDegree(void) {
    // update pulse-width for <current angle> + <10 degrees>
    pulseWidthTicks += TEN_DEGREE_TICKS;
    if (pulseWidthTicks > SERVO_MAX_ANGLE) {
        pulseWidthTicks = SERVO_MIN_ANGLE;
    }

    TIMER_A2->CCR[1] = pulseWidthTicks;

}

void setServoAngle(uint8_t angle) {
    pulseWidthTicks = (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE)*angle/180;

    TIMER_A2->CCR[1] = SERVO_MIN_ANGLE + pulseWidthTicks;
}

void unlock() {
    setServoAngle(UNLOCKED_ANGLE);
}

void lock() {
    setServoAngle(LOCKED_ANGLE);
}
