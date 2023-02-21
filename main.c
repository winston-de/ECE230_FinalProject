#include <lcd8or4bitsece230winter23.h>
#include "msp.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "utilities.h"
#include "keypadscan_subroutines.h"

#define ACLK 32768 //Hz
#define BEEP_PERIOD       ACLK/8    //1 second


enum Status
{
    NO, YES
};
extern char NewKeyPressed;
extern char FoundKey;

int LIMIT = 3;
int x_count = 0;
bool return_to_neutral = false;
bool break_in = false;

// Bluetooth module prompts

char break_alert1[] =
        "\rBreak-in attempt on your safe, please verify this was you or contact relevant authorities \n";
char break_alert2[] =
        "\rYou can verify it was you by entering Master-Key and resetting the system \n\n";

// NOTES ARE DEFINED HERE

#define SMCLK 48000000 //Hz
#define TimerA0Prescaler 4 //Timer A prescaler
#define TimerA0Clock  SMCLK/TimerA0Prescaler
#define RestNote 1

// Note frequencies C3-B5
#define FrequencyC3 130.81 //Hz
#define FrequencyD3 146.83 //Hz
#define FrequencyE3 164.81//Hz
#define FrequencyF3 174.61//Hz
#define FrequencyG3 196//Hz
#define FrequencyA3 220 //Hz
#define FrequencyB3 246.94//Hz

#define FrequencyC4 261.63 //Hz
#define FrequencyD4 293.66 //Hz
#define FrequencyE4 329.63//Hz
#define FrequencyF4 349.23//Hz
#define FrequencyG4 392//Hz
#define FrequencyA4 440 //Hz
#define FrequencyB4 493.88//Hz

#define FrequencyC5 523.25 //Hz
#define FrequencyD5 587.33 //Hz
#define FrequencyE5 659.25//Hz
#define FrequencyF5 698.46//Hz
#define FrequencyG5 783.99//Hz
#define FrequencyA5 880 //Hz
#define FrequencyB5 1046.50//Hz

// All NOTES : C3 - B5
#define NOTEC3  TimerA0Clock/FrequencyC3
#define NOTED3  TimerA0Clock/FrequencyD3
#define NOTEE3  TimerA0Clock/FrequencyE3
#define NOTEF3  TimerA0Clock/FrequencyF3
#define NOTEG3  TimerA0Clock/FrequencyG3
#define NOTEA3  TimerA0Clock/FrequencyA3
#define NOTEB3  TimerA0Clock/FrequencyB3

#define NOTEC4  TimerA0Clock/FrequencyC4
#define NOTED4  TimerA0Clock/FrequencyD4
#define NOTEE4  TimerA0Clock/FrequencyE4
#define NOTEF4  TimerA0Clock/FrequencyF4
#define NOTEG4  TimerA0Clock/FrequencyG4
#define NOTEA4  TimerA0Clock/FrequencyA4
#define NOTEB4  TimerA0Clock/FrequencyB4

#define NOTEC5  TimerA0Clock/FrequencyC5
#define NOTED5  TimerA0Clock/FrequencyD5
#define NOTEE5  TimerA0Clock/FrequencyE5
#define NOTEF5  TimerA0Clock/FrequencyF5
#define NOTEG5  TimerA0Clock/FrequencyG5
#define NOTEA5  TimerA0Clock/FrequencyA5
#define NOTEB5  TimerA0Clock/FrequencyB5

/**
 * main.c
 */
void main(void)
{
    /* Stop Watchdog timer */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    // The peripherals are configured here
    configHFXT();
    configLFXT();
    ConfigureUART_A0();
    LCD_Initializtion();

    initServoMotor();

    initSpeaker();
    initBluetooth();

    __enable_irq();


    // display message on LCD upon reset
    lcd_clear();
    lcd_SetLineNumber(FirstLine);
    lcd_puts("Reset...");
    lcd_SetLineNumber(SecondLine);
    lcd_puts("Enter Command");

    kepadconfiguration();

//clear keypad output pins to be 0 to be ready for input interrupt
//Do not change input pin values
    KeypadPort->OUT = (KeypadPort->OUT & ~KeypadOutputPins)
            | (0b11110000 & KeypadOutputPins);


    // The main while loop (keeps looping till power is ON)
    while (1)
    {


        break_in = false; // set the break_in Mode to false when entered back in Neutral state
        if (NewKeyPressed == YES)
        {
            NewKeyPressed = NO;

            // check to see if command key is pressed
            int key_pressed = convert_key_val(FoundKey);
            keyPressed(FoundKey);

            // switch statement based on the command key that is pressed
            switch (key_pressed)
            {

            case 'A':   // enter the Unlock mode (here you enter password to unlock safe)
                lcd_clear();
                enterCode();

                printInitialDisplay(); // reset the display
                break;

            case 'B':   // enter the Change passcode mode
                lcd_clear();
                setCode();

                printInitialDisplay(); // reset the display
                break;

            case 'C':   // Lock the safe again
                lcd_clear();
                lock();    // turns the servo back to the LOCKED angle

                printInitialDisplay(); // reset the display
                break;

            case 'D':   // Enter the Reset mode
                lcd_clear();
                restoreDefault();

                printInitialDisplay(); // reset the display
                break;

            default:
                // code to execute when none of the above cases match

                // nothing happens if no command key is pressed
            }

        } // key pressed - if

    } // close while
} // close main


// set the various arrays for codes
int max_code_length = 16;
char master_code[16] = { '0', '*' }; // By default set to '0' (But in practice it will be a 15-digit code)
char user_code[16] = { '1', '2', '3', '4', '*' }; // By default it is set to '1234' upon reset as well
char entered_code[16];




// This function takes in the User code and then if the code is correct (equal to user_code) unlocks,
// if not correct it increases the x_wrong (the wrong count), and checks if it is equal to LIMIT
// if it is it enters the break_in mode
void enterCode(void)
{

    int i;
    char lcd_data[16];

    clearArray(entered_code);


    // take in user input (keystrokes)
    inputKeystrokes(entered_code);


    // if the '#' key was entered to go back
    // just returns to Neutral state
    if(return_to_neutral){
        return_to_neutral = false;
        return;
    }


    // Checks if code entered was same as user_code or not
    if (arraysEqual(entered_code, user_code))
    {
        // unlocks the servo and resets the wrong count (x_count)

        lcd_printArray(entered_code);

        unlock();  // sets servo to UNLOCK angle
        playNote(NOTEA5, BEEP_PERIOD);
        lcd_SetLineNumber(SecondLine);
        lcd_puts("Unlocked");
        x_count = 0; // reset tries upon success
        NewKeyPressed = NO;
        return;

    }
    else
    {

        // passcode entered is wrong

        lcd_printArray(entered_code);

        x_count++;
        lcd_SetLineNumber(SecondLine);
        lcd_puts("Wrong -> #");
        playNote(NOTEC3, BEEP_PERIOD);

        sprintf(lcd_data, " %d", x_count);
        lcd_puts(lcd_data);


        // If code is entered wrong 3 times in a row
        if (x_count == LIMIT)
        {


            // Send alert to Bluetooth module
            printMessage(break_alert1, sizeof(break_alert1));
            printMessage(break_alert2, sizeof(break_alert2));

            lcd_clear();
            lcd_puts(" --TIMED OUT-- ");

            char key;


            // TIMED-OUT loop (won't exit till correct Master code is entered to reset the system
            while (1)
            {


                // This loop just waits for keypad input and performs actions based on the Key pressed
                NewKeyPressed = NO;
                while (NewKeyPressed != YES)
                {
                    debounce();

                    if (NewKeyPressed == YES)
                    {

                        key = convert_key_val(FoundKey);
                    }
                }

                if (key == 'D') // If key to enter master code is pressed enter this function
                {
                    break_in = true; // with this boolean true the back function will be disabled till the correct Master code is entered and system is reset
                    restoreDefault();
                    break;
                }
            }
        }

        NewKeyPressed = NO;
        return;

    } // end for if wrong code was entered

} // end of enterCode



// This state is used to change the passcode
// Requires old code first if correct it takes in new code
// If wrong increases x_count and exits back to neutral
void setCode(void)
{

    int i;
    char lcd_data[16];

    lcd_puts("Old code ->");

    clearArray(entered_code);

    // take in old password
    inputKeystrokes(entered_code);



    // if the '#' key was entered to go back
    // just returns to Neutral state
    if(return_to_neutral){
          return_to_neutral = false;
          return;
      }


    if (arraysEqual(entered_code, user_code))
    {
        // if the code was entered correct enters this branch

        lcd_printArray(entered_code);

        lcd_SetLineNumber(SecondLine);
        lcd_puts("Authorized");
        NewKeyPressed = NO;
        lcd_display_delay(); //arbitrary delay
        lcd_clear();

        lcd_SetLineNumber(FirstLine);

        clearArray(user_code);


        // take in user input and set the user_code to be the new passcode
        inputKeystrokes(user_code);

        lcd_printArray(user_code);

        lcd_SetLineNumber(SecondLine);
        lcd_puts("Code Set");
        NewKeyPressed = NO;
        return;

    }
    else
    {

        // id the passcode was wrong enter this branch

        lcd_printArray(entered_code);

        x_count++; // increase wrong count
        lcd_SetLineNumber(SecondLine);
        lcd_puts("Wrong -> #");

        sprintf(lcd_data, " %d", x_count);
        lcd_puts(lcd_data);


        // If code is entered wrong 3 times in a row
        if (x_count == LIMIT)
        {

            // Send alert to Bluetooth module
             printMessage(break_alert1, sizeof(break_alert1));
             printMessage(break_alert2, sizeof(break_alert2));

             lcd_clear();
             lcd_puts(" --TIMED OUT-- ");

             char key;


             // TIMED-OUT loop (won't exit till correct Master code is entered to reset the system
             while (1)
             {


                 // This loop just waits for keypad input and performs actions based on the Key pressed
                 NewKeyPressed = NO;
                 while (NewKeyPressed != YES)
                 {
                     debounce();

                     if (NewKeyPressed == YES)
                     {

                         key = convert_key_val(FoundKey);
                     }
                 }

                 if (key == 'D') // If key to enter master code is pressed enter this function
                 {
                     break_in = true; // with this boolean true the back function will be disabled till the correct Master code is entered and system is reset
                     restoreDefault();
                     break;
                 }
             }
        }

        NewKeyPressed = NO;
        return;

    } // end of else in case of failiure to change

} // end of setCode



// Enter this state to reset the system
// Enter master password
void restoreDefault(void)
{
    int i;

    while (1)
    {

        lcd_SetLineNumber(FirstLine);
        lcd_clear();
        lcd_puts("Master Code ::");

        clearArray(entered_code);

           inputKeystrokes(entered_code);



        if(return_to_neutral){
              return_to_neutral = false;
              return;
         }

        if (arraysEqual(entered_code, master_code))
        {

            lcd_SetLineNumber(SecondLine);
            lcd_puts("Reseting...");
            NewKeyPressed = NO;
            lcd_display_delay(); //arbitrary delay
            lcd_clear();

            lcd_SetLineNumber(FirstLine);

            user_code[0] = '1';
            user_code[1] = '2';
            user_code[2] = '3';
            user_code[3] = '4';
            user_code[4] = '*';

            x_count = 0;
            break;
        }

        lcd_timeout();
        break_in = true;

    }

} // end of restoreDefault

void inputKeystrokes(char arr[])
{

    int i = 0;
    while (1)
    {

        NewKeyPressed = NO;
        int key;

        while (NewKeyPressed != YES)
        {
            debounce();

            if (NewKeyPressed == YES)
            {
                key = convert_key_val(FoundKey);
                keyPressed(key);
                arr[i] = key;
                i++;
            }
        }

        if(key == '#' && !break_in){
            return_to_neutral = true;
            break;
        }

        if (key == '*')
        {
            break;
        }
    }

}

void lcd_printArray(char arr[])
{

    int i = 0;
    while (arr[i] != '*')
    {
        lcd_putch(arr[i]);
        i++;
        debounce();

    }

}

void clearArray(char arr[])
{

    int i = 0;
    while (arr[i] != '*')
    {
        arr[i] = NULL;
        i++;
    }
    arr[i] = NULL;

}

// A timer that counts and displays till a minute
void lcd_timeout(void)
{

    char lcd_data[16];
    int s = 0, m = 0;

    uint32_t i;
    uint32_t j = SMCLK;
    for (i = 0; i < SMCLK; i++)
    {

        if (j % (SMCLK / 60) == 0)
        {    // this code is executed every second
            // tick up the value on the display by 1 second, if a minute has passed tick up the minute value

            lcd_clear();
            lcd_SetLineNumber(SecondLine);
            sprintf(lcd_data, "  %d : %d  ", m, s);
            lcd_puts(lcd_data);
            s++;

            if (s % 60 == 0)
            {
                m++;
                s = 0;
            }
        }
        j++;
    }

}

// debounce for 5 ms
void debounce(void)
{
//delay time for debouncing switches
    int delay;
    for (delay = 0; delay < 150; delay++)
        ;
} //end debounce()

void lcd_display_delay(void)
{
    int i;
    for (i = 0; i < 1000000; i++)
        ; //arbitrary delay
}

void keyPressed(char key)
{
    //TODO: add different
    switch (key)
    {
    case 'A':
        playNote(NOTEB5, BEEP_PERIOD);
        break;
    case 'B':
        playNote(NOTEG4, BEEP_PERIOD);
        break;
    case 'C':
        playNote(NOTEG3, BEEP_PERIOD);
        break;
    case 'D':
        playNote(NOTEC3, BEEP_PERIOD);
        break;
    case '*':
    case '#':
        playNote(NOTED4, BEEP_PERIOD);
        break;
    default:
        playNote(NOTEA4, BEEP_PERIOD);
        break;

    }
}

void printInitialDisplay(void) {
    lcd_display_delay();
    lcd_clear();
    lcd_SetLineNumber(FirstLine);
    lcd_puts("Enter Command");

}



