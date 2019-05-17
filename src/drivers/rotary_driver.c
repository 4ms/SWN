/*
* rotary_driver.c - drivers for rotary encoders
*
* State machine table taken from code copyright 2011 Ben Buxton (bb@cactii.net) and licenced under the GNU GPL Version 3.
*
* Conversion for use with STM32 in 2015 by: Dan Green (danngreen1@gmail.com)
* Additional modifications for SWN project in 2017-2019: Hugo Paris (hugoplho@gmail.com), Dan Green (danngreen1@gmail.com)
*
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
* 
* -----------------------------------------------------------------------------
*/

 
#include "drivers/rotary_driver.h"
#include "drivers/switch_driver.h"


#define HS_R_CCW_BEGIN    0x1
#define HS_R_CW_BEGIN     0x2
#define HS_R_START_M      0x3
#define HS_R_CW_BEGIN_M   0x4
#define HS_R_CCW_BEGIN_M  0x5

#define FS_R_CW_FINAL     0x1
#define FS_R_CW_BEGIN     0x2
#define FS_R_CW_NEXT      0x3
#define FS_R_CCW_BEGIN    0x4
#define FS_R_CCW_FINAL    0x5
#define FS_R_CCW_NEXT     0x6

const unsigned char HS_ttable[6][4] = {

  // R_START (00)
  {HS_R_START_M,             HS_R_CW_BEGIN,         HS_R_CCW_BEGIN,          R_START},

  // R_CCW_BEGIN
  {HS_R_START_M | DIR_CCW,   R_START,            HS_R_CCW_BEGIN,          R_START},

  // HS_R_CW_BEGIN
  {HS_R_START_M | DIR_CW,    HS_R_CW_BEGIN,         R_START,              R_START},

  // HS_R_START_M (11)
  {HS_R_START_M,             HS_R_CCW_BEGIN_M,      HS_R_CW_BEGIN_M,         R_START},

  // R_CW_BEGIN_M
  {HS_R_START_M,             HS_R_START_M,          HS_R_CW_BEGIN_M,         R_START | DIR_CW},

  // R_CCW_BEGIN_M
  {HS_R_START_M,             HS_R_CCW_BEGIN_M,      HS_R_START_M,            R_START | DIR_CCW},
};

const unsigned char FS_ttable[7][4] = {

  // R_START
  {R_START,               FS_R_CW_BEGIN,         FS_R_CCW_BEGIN,          R_START},

  // R_CW_FINAL
  {FS_R_CW_NEXT,             R_START,            FS_R_CW_FINAL,           R_START | DIR_CW},

  // R_CW_BEGIN
  {FS_R_CW_NEXT,             FS_R_CW_BEGIN,         R_START,              R_START},

  // R_CW_NEXT
  {FS_R_CW_NEXT,             FS_R_CW_BEGIN,         FS_R_CW_FINAL,           R_START | DIR_CW},  /* last entry was R_START */

  // R_CCW_BEGIN
  {FS_R_CCW_NEXT,            R_START,            FS_R_CCW_BEGIN,          R_START},

  // R_CCW_FINAL
  {FS_R_CCW_NEXT,            FS_R_CCW_FINAL,        R_START,              R_START | DIR_CCW},

  // R_CCW_NEXT
  {FS_R_CCW_NEXT,            FS_R_CCW_FINAL,        FS_R_CCW_BEGIN,          R_START | DIR_CCW}, /* last entry was R_START */
};


void init_rotary_turn(o_turn *turn_object)
{
  turn_object->state = R_START;
}


int8_t read_rotary_turn(o_rotary *rotary_object){
  update_rotary_queue(&rotary_object->turn);
  return pop_rotary_queue(&rotary_object->turn);
}


void update_rotary_queue(o_turn *turn_object){
  uint8_t tmp_motion;

  unsigned char pinstate = (((turn_object->A_gpio->IDR & turn_object->A_pin) ? 1:0) << 1) | 
                            ((turn_object->B_gpio->IDR & turn_object->B_pin) ? 1:0)       ;

  // Determine new state from the pins and state table
  // HALF_STEP
  // Use the half-step state table (emits a code at 00 and 11)
  if (turn_object->step_size == HALFSTEP){
    turn_object->state = HS_ttable[turn_object->state & 0xf][pinstate];
  }

  // FULL STEP
  // Use the full-step state table (emits a code at 00 only)
  else if (turn_object->step_size == FULLSTEP){
    turn_object->state = FS_ttable[turn_object->state & 0xf][pinstate];
  }

  tmp_motion = turn_object->state & 0x30;

  // update rotation queue if we notice a CW or CCW motion
  if (tmp_motion == DIR_CCW)      {turn_object->queue--;}
  else if (tmp_motion == DIR_CW)  {turn_object->queue++;}

}


int8_t pop_rotary_queue(o_turn *turn_object){
  int8_t tmp;

  if (turn_object->queue == 0){return 0;}
  else {
    tmp = turn_object->queue;
    turn_object->queue =0;
    return tmp;
  }
}


//Note: this does not set the rotary's hwswitch.pressed element, it just reads the GPIO pins and returns the state
//This allows UI conditioning to debounce, calculate short/long presses, etc.
//UI conditioning should set the state, not the driver.
//
uint8_t read_rotary_press(o_rotary *rotary_object){
  uint8_t state;

  state = ((rotary_object->hwswitch.gpio->IDR) & (rotary_object->hwswitch.pin)) ? 0 : 1;
  if(rotary_object->hwswitch.ptype == PULLDOWN){state = 1 - state;}   // accounting for pull down encoders
  return state;
}
