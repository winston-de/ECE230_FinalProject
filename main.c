#include <lcd8or4bitsece230winter23.h>
#include "msp.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "utilities.h"
#include "keypadscan_subroutines.h"

#define ACLK 32768 //Hz
#define BEEP_PERIOD       ACLK/8    //1 second

//// Note A4 - 440 Hz, B4 - 493.88 Hz, C5 - 523.26 Hz
//#define NOTEA4  27273
//#define NOTEB4  24297
//#define NOTEC5  22933

enum Status {NO, YES};
extern char NewKeyPressed;
extern char FoundKey;



int LIMIT = 3;
int x_count = 0;



// Bluetooth module prompts

char break_alert1[] = "\rBreak-in attempt on your safe, please verify this was you or contact relevant authorities \n";
char break_alert2[] = "\rYou can verify it was you by entering Master-Key and resetting the system \n\n";

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


    configHFXT();
    configLFXT();
    ConfigureUART_A0();
    LCD_Initializtion();

    initServoMotor();

    initSpeaker();
    initBluetooth();

    __enable_irq();

    lcd_clear();
    lcd_SetLineNumber(FirstLine);

   lcd_puts("Reset...");
   lcd_SetLineNumber(SecondLine);
   lcd_puts("Enter Command");
   lcd_SetLineNumber(FirstLine);

//    printf("keyscan started: press a key on your 4x4 keypad ....\r\n");
    kepadconfiguration();

//clear keypad output pins to be 0 to be ready for input interrupt
//Do not change input pin values
    KeypadPort->OUT = (KeypadPort->OUT & ~KeypadOutputPins)
            | (0b11110000 & KeypadOutputPins);
    while (1)
    {
        if (NewKeyPressed == YES)
        {
            NewKeyPressed = NO;
            lcd_clear();

            int key_pressed = convert_key_val(FoundKey);
            keyPressed(FoundKey);

            switch (key_pressed) {

            case 'A':
                enterCode();
                break;

            case 'B':
                setCode();
                break;

            case 'C':
                lock();
                break;

            case 'D':
                restoreDefault();

            default:
                  // code to execute when none of the above cases match
                lcd_putch('X') ;
            }



        } // key pressed - if


    } // close while
} // close main




int max_code_length = 16;
char master_code[16] = {'#', '*'};         //{'1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '1', '*'};
char user_code[16] = {'1', '2', '3', '4','*' };
char entered_code[16];



void enterCode(void){

    int i;
    char lcd_data[16];


//    for(i = 0; i < max_code_length; i++){
//       entered_code[i] = NULL;
//    }

    clearArray(entered_code);


    inputKeystrokes(entered_code);

    if(arraysEqual(entered_code, user_code)){


        lcd_printArray(entered_code);

        unlock();
        lcd_SetLineNumber(SecondLine);
        lcd_puts("Unlocked");
        x_count = 0; // reset tries upon success
        NewKeyPressed = NO;
        return;

    } else {


        lcd_printArray(entered_code);

        x_count ++;
        lcd_SetLineNumber(SecondLine);
        lcd_puts("Wrong -> #");


        sprintf(lcd_data, " %d",x_count);
        lcd_puts(lcd_data);



        if(x_count == LIMIT){

            // ADD ALERT TO BLUETOOTH MODULE HERE

            printMessage(break_alert1, sizeof(break_alert1));
            printMessage(break_alert2, sizeof(break_alert2));


            lcd_clear();
            lcd_puts(" --TIMED OUT-- ");


            char key;

            while(1){

                NewKeyPressed = NO;
                while(NewKeyPressed != YES) {
                     debounce();

                     if(NewKeyPressed == YES){

                         key = convert_key_val(FoundKey);
                     }
                }

                if(key == 'D'){
                   restoreDefault();
                   break;
               }
            }



        }

        NewKeyPressed = NO;
        return;

    } // end for if wrong code was entered


} // end of enterCode



void setCode(void){

        int i;
        char lcd_data[16];

        lcd_puts("Old code ->");


        clearArray(entered_code);

       inputKeystrokes(entered_code);

        if(arraysEqual(entered_code, user_code)){

              lcd_printArray(entered_code);

               lcd_SetLineNumber(SecondLine);
               lcd_puts("Authorized");
               NewKeyPressed = NO;
               lcd_display_delay(); //arbitrary delay
               lcd_clear();


               lcd_SetLineNumber(FirstLine);


               clearArray(user_code);


               inputKeystrokes(user_code);

               lcd_printArray(user_code);

                 lcd_SetLineNumber(SecondLine);
                 lcd_puts("Code Set");
                 NewKeyPressed = NO;
                 return;

           } else {


//               lcd_clear();
//
//               lcd_SetLineNumber(FirstLine);
//
//               lcd_printArray(entered_code);
//
//               lcd_SetLineNumber(SecondLine);
//               lcd_puts("Wrong code");
//               lcd_display_delay(); //arbitrary delay
//               return;


           lcd_printArray(entered_code);

                 x_count ++;
                 lcd_SetLineNumber(SecondLine);
                 lcd_puts("Wrong -> #");


                 sprintf(lcd_data, " %d",x_count);
                 lcd_puts(lcd_data);



                 if(x_count == LIMIT){

                     // ADD ALERT TO BLUETOOTH MODULE HERE

                     printMessage(break_alert1, sizeof(break_alert1));
                     printMessage(break_alert2, sizeof(break_alert2));


                     lcd_clear();
                     lcd_puts(" --TIMED OUT-- ");


                     char key;

                     while(1){

                         NewKeyPressed = NO;
                         while(NewKeyPressed != YES) {
                              debounce();

                              if(NewKeyPressed == YES){

                                  key = convert_key_val(FoundKey);
                              }
                         }

                         if(key == 'D'){
                            restoreDefault();
                            break;
                        }
                     }



                 }

                 NewKeyPressed = NO;
                 return;


           } // end of else in case of failiure to change

} // end of setCode



void restoreDefault(void){
      int i;

          while(1){

              lcd_SetLineNumber(FirstLine);
              lcd_clear();
              lcd_puts("Master Code ::");

              clearArray(entered_code);
              inputKeystrokes(entered_code);

               if(arraysEqual(entered_code, master_code)){


                  lcd_SetLineNumber(SecondLine);
                  lcd_puts("Reseting...");
                  NewKeyPressed = NO;
                  lcd_display_delay(); //arbitrary delay
                  lcd_clear();


                  lcd_SetLineNumber(FirstLine);


                  user_code[0] = '1';user_code[1] = '2';user_code[2] = '3';user_code[3] = '4';user_code[4] = '*'; //Better way?

                  x_count = 0;
                  break;
               }

               lcd_timeout();

          }



} // end of restoreDefault



void inputKeystrokes(char arr[]){

      int i = 0;
      while(1){

          NewKeyPressed = NO;
          int key;

          while(NewKeyPressed != YES) {
             debounce();

             if(NewKeyPressed == YES){
                 key = convert_key_val(FoundKey);
                 keyPressed(key);
                 arr[i] = key;
                 i++;
             }
          }
          if(key == '*'){break;}
      }

}


void lcd_printArray(char arr[]){

   int i = 0;
   while(arr[i] != '*'){
       lcd_putch(arr[i]);
       i++;
       debounce();

   }

}


void clearArray(char arr[]){


    int i = 0;
    while(arr[i] != '*'){
        arr[i] = NULL;
        i++;
    }
    arr[i] = NULL;

}




// A timer that counts and displays till a minute
void lcd_timeout(void){

    char lcd_data[16];
    int s = 0, m = 0;

    uint32_t i;
    uint32_t j = SMCLK;
    for(i = 0; i < SMCLK; i++){

        if(j % (SMCLK/60) == 0){    // this code is executed every second
            // tick up the value on the display by 1 second, if a minute has passed tick up the minute value

            lcd_clear();
            lcd_SetLineNumber(SecondLine);
            sprintf(lcd_data, "  %d : %d  ", m,s);
            lcd_puts(lcd_data);
            s++;

            if(s % 60 == 0){
                m++;
                s = 0;
            }
        }
        j++;
    }

}


// debounce for 5 ms
void debounce(void){
//delay time for debouncing switches
    int delay;
    for(delay = 0; delay < 150; delay++);
} //end debounce()


void lcd_display_delay(void){
    int i;
    for(i = 0; i < 5000000; i++); //arbitrary delay
}



void keyPressed(char key) {
    //TODO: add different

    switch(key){

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
    default:
        playNote(NOTEA4, BEEP_PERIOD);
        break;

    }


//    printMessage("please help me i am dying inside", 33);
}







