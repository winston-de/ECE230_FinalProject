/*
 * utilities.c
 *
 *  Created on: Feb 14, 2023
 *      Author: Simarjit Dhillon
 *
 *
 *      The purpose of this file is to contain miscellaneous helper functions for the Keypad and LCD
 *
 *
 */

#include "utilities.h"

int convert_key_val (char FoundKey) {

    switch (FoundKey) {
      case 1:
          return 0x31 ; // return ASCII for '1'
        break;
      case 2:
          return 0x32 ;// return ASCII for '2'
        break;
      case 3:
          return 0x33 ; // return ASCII for '3'
        break;
      case 4:
          return 0x41 ;// return ASCII for 'A'
        break;
      case 5:
          return 0x34 ; // return ASCII for '4'
        break;
      case 6:
          return 0x35 ; // return ASCII for '5'
        break;
      case 7:
          return 0x36 ; // return ASCII for '6'
        break;
      case 8:
          return 0x42 ; // return ASCII for 'B'
        break;
      case 9:
          return 0x37 ; // return ASCII for '7'
        break;
      case 10:
          return 0x38 ; // return ASCII for '8'
        break;
      case 11:
          return 0x39 ; // return ASCII for '9'
        break;
      case 12:
          return 0x43 ; // return ASCII for 'C'
        break;
      case 13:
          return 0x2A ; // return ASCII for '*'
        break;
      case 14:
          return 0x30 ; // return ASCII for '0'
        break;
      case 15:
          return 0x23 ; // return ASCII for '#'
        break;
      case 16:
          return 0x44 ; // return ASCII for 'D'
        break;
      default:
        // code to execute when none of the above cases match
          return 0x31 ; // return ASCII for '1'
    }

}





