#include <Arduino.h>
#include "stm32f103xb.h"
#include "timer.h"
#include "stm32f1xx.h"
#include "HardwareTimer.h"
#include "mini_mk2_interface.h"

#define DS_LED PB15
#define CP_LED PB13
#define NPL_SW PA4
#define CP_SW PA5
#define Q_SW PA6
#define MUX1 PB0
#define MUX2 PB1
#define MUX3 PB2

int Launchpad::pixel_colors[9][9][2] = {{0,0}}; //3D array that can be written to directly indicating the intensity of the Red and Green LEDs per pixel. Max intensity is 7.
Launchpad::PixelState Launchpad::pixel_states[9][9] = {{Launchpad::SOLID}};
bool Launchpad::button_states[9][9] = {{0}};; //2D array which is read-only 

bool previous_button_states[9][9] = {{0}};

//int led_index_map[9][9] = {{0, 13, 14, 15, 16, 17, 18, 19, 20}, {0, 13, 14, 15, 16, 17, 18, 19, 20}, {0, 13, 14, 15, 16, 17, 18, 19, 20}, {1, 9, 10, 11, 12, 21, 22, 23, 24}, {1, 9, 10, 11, 12, 21, 22, 23, 24}, {1, 9, 10, 11, 12, 21, 22, 23, 24}, {2, 4, 5, 6, 8, 25, 26, 27, 28}, {2, 4, 5, 6, 8, 25, 26, 27, 28}, {2, 4, 5, 6, 8, 25, 26, 27, 28}};
int led_index_map[9][9] = {{0, 11, 12, 13, 14, 15, 16, 17, 18}, {0, 11, 12, 13, 14, 15, 16, 17, 18}, {0, 11, 12, 13, 14, 15, 16, 17, 18}, {1, 7, 8, 9, 10, 19, 20, 21, 22}, {1, 7, 8, 9, 10, 19, 20, 21, 22}, {1, 7, 8, 9, 10, 19, 20, 21, 22}, {2, 3, 4, 5, 6, 23, 24, 25, 26}, {2, 3, 4, 5, 6, 23, 24, 25, 26}, {2, 3, 4, 5, 6, 23, 24, 25, 26}};
int button_index_map[3][32][2] = {
  {{6,8},{6,7},{6,6},{6,5},{6,4},{6,3},{6,2},{6,1},{3,8},{3,7},{3,6},{3,5},{3,4},{3,3},{3,2},{3,1},{0,8},{0,7},{0,6},{0,5},{0,4},{0,3},{0,2},{0,1},{0,0},{8,0},{8,0},{8,0},{8,0},{6,0},{3,0},{0,0}},
  {{7,8},{7,7},{7,6},{7,5},{7,4},{7,3},{7,2},{7,1},{4,8},{4,7},{4,6},{4,5},{4,4},{4,3},{4,2},{4,1},{1,8},{1,7},{1,6},{1,5},{1,4},{1,3},{1,2},{1,1},{1,0},{8,0},{8,0},{8,0},{8,0},{7,0},{4,0},{1,0}},
  {{8,8},{8,7},{8,6},{8,5},{8,4},{8,3},{8,2},{8,1},{5,8},{5,7},{5,6},{5,5},{5,4},{5,3},{5,2},{5,1},{2,8},{2,7},{2,6},{2,5},{2,4},{2,3},{2,2},{2,1},{2,0},{8,0},{8,0},{8,0},{8,0},{8,0},{5,0},{2,0}}
  };
//int button_index_map[3][32][2] = {{{4,1}}};

int led_register_map_green[27] = {0,1,2, 8,9,10,11, 16,17,18,19, 24,25,26,27, 32,33,34,35, 40,41,42,43, 48,49,50,51};
int led_register_map_red[27] = {3,4,5, 12,13,14,15, 20,21,22,23, 28,29,30,31, 36,37,38,39, 44,45,46,47, 52,53,54,55};

int fragment[27][2] = {{0,0}};

int mux_idx = 1;
int frame = 0; //4 max, when this reaches 4 it gets reset to 0

float sin_time = 0.0;

HardwareTimer *DisplayTimer = new HardwareTimer(TIM3);  // DisplayTimer provides a stable refresh rate for the LED matrix and reading out buttons.

void triggerMux(int index) { //-1 enables all, 0 disables all
  switch (index) {
    case -1:
      digitalWrite(MUX2, LOW);
      digitalWrite(MUX3, LOW);
      digitalWrite(MUX1, LOW);
      break;
    case 0:
      digitalWrite(MUX1, HIGH);
      digitalWrite(MUX2, HIGH);
      digitalWrite(MUX3, HIGH);
    break;
    case 1:
      digitalWrite(MUX1, LOW);
      digitalWrite(MUX2, HIGH);
      digitalWrite(MUX3, HIGH);
      break;
    case 2:
      digitalWrite(MUX2, LOW);
      digitalWrite(MUX1, HIGH);
      digitalWrite(MUX3, HIGH);
      break;
    case 3:
      digitalWrite(MUX3, LOW);
      digitalWrite(MUX2, HIGH);
      digitalWrite(MUX1, HIGH);
      break;
  }
}

bool readState_Q_SW() {
  return digitalRead(Q_SW);
}

void setState_NPL_SW(bool state) {
  digitalWrite(NPL_SW, state);
}

void setState_DS_LED(bool inv_state) {
  digitalWrite(DS_LED, !inv_state);
}

void HC164_clockCycle() {
  digitalWrite(CP_LED, HIGH);
  digitalWrite(CP_LED, LOW);
}

void HC165_clockCycle() {
  digitalWrite(CP_SW, HIGH);
  digitalWrite(CP_SW, LOW);
}

void writeRed() {
  for (int i = 0; i < 7; i++) {
    setState_DS_LED(HIGH);
    for (int idx = 0; idx < 4; idx++) {
      HC164_clockCycle();
    }

    setState_DS_LED(LOW);
    for (int idx = 0; idx < 4; idx++) {
      HC164_clockCycle();
    }
  }  
}

void writeGreen() {
  for (int i = 0; i < 7; i++) {
    setState_DS_LED(LOW);
    for (int idx = 0; idx < 4; idx++) {
      HC164_clockCycle();
    }

    setState_DS_LED(HIGH);
    for (int idx = 0; idx < 4; idx++) {
      HC164_clockCycle();
    }
  }  
}

void writeOrange() {
  setState_DS_LED(HIGH);
  for (int i = 0; i < 56; i++) {
    HC164_clockCycle();
  }  
}

void readButtons(int mux_idx) {
  setState_NPL_SW(LOW); //load parallel data into registers
  setState_NPL_SW(HIGH);  //enable serial shift mode

  for (int temp = 0; temp < 32; temp++) {
    Launchpad::button_states[button_index_map[mux_idx][temp][0]][button_index_map[mux_idx][temp][1]] = readState_Q_SW();
    HC165_clockCycle();
  }
}

void writeSerialReverse(int serial_string_in[], int data_string_size) {
  for (int idx = data_string_size - 1; idx >= 0; idx -= 1) {
    setState_DS_LED(serial_string_in[idx]);
    HC164_clockCycle();
  }
}

float getPixelIntensity(int posx, int posy) {
  switch (Launchpad::pixel_states[posx][posy]) {
    case Launchpad::SOLID:
      return 1.0;
      break;
    case Launchpad::FLASH_1:
      return (sin(sin_time) * 0.5) + 0.5;
      break;
    case Launchpad::FLASH_2:
      return (sin(sin_time * 2.0) * 0.5) + 0.5;
      break;
    default:
      return 1.0;
  }
}

void displayFragment(int mux_idx, int frame) {
  
  for (int x = 0; x < 3; x++) { //generate array of led color intensities based on mux index: {{64,0}, {32,0}, {50,0}, {0,64}, {RED, GREEN}, etc..} where 64 is max brightness and 0 is disabled
    for (int y = 0; y < 9; y++) {
      int resulting_color[2];
      memcpy(resulting_color, Launchpad::pixel_colors[(x * 3) + mux_idx][y], 2 * sizeof(int));
      
      resulting_color[0] = float(resulting_color[0]) * getPixelIntensity((x * 3) + mux_idx, y);
      resulting_color[1] = float(resulting_color[1]) * getPixelIntensity((x * 3) + mux_idx, y);
      
      memcpy(fragment[led_index_map[(x * 3) + mux_idx][y]], resulting_color, 2 * sizeof(int));
    }
  }

  int serial_data_out[56] = {0};

  for (int idx = 0; idx < 27; idx++) {
    if (fragment[idx][0] > frame) {
      serial_data_out[led_register_map_red[idx]] = 1;
      //serial_data_out[led_register_map_green[idx]] = 1;
    }
    if (fragment[idx][1] > frame) {
      serial_data_out[led_register_map_green[idx]] = 1;
    }

  }
  writeSerialReverse(serial_data_out, 56);

}


void display_update_callback() {
  sin_time += 4.0 / 1200.0; //speed up all flashes and such by 4 times by default

  if (mux_idx == 2) {
    frame++;
  }

  mux_idx++;
  mux_idx = mux_idx % 3;
  frame = frame % 9;

  triggerMux(0);
  displayFragment(mux_idx, frame);
  triggerMux(mux_idx + 1);

  readButtons(mux_idx);

  for (int x = 0; x < 9; x++) {
    for (int y = 0; y < 9; y++) {
      if (Launchpad::button_states[x][y] == true && previous_button_states[x][y] == false) {
        Launchpad::onButtonDown(x,y);
      }
      else if (Launchpad::button_states[x][y] == false && previous_button_states[x][y] == true) {
        Launchpad::onButtonUp(x,y);
      }
    }
   }

  memcpy(previous_button_states, Launchpad::button_states, sizeof(bool) * 81);
}

void Launchpad::ClearDisplayBuffer() {
  int _new_pixel_colors[9][9][2] = {{0,0}};
  memcpy(Launchpad::pixel_colors, _new_pixel_colors, sizeof(int) * 81 * 2);
}

void Launchpad::ClearDisplay() {
  int _new_pixel_colors[9][9][2] = {{0,0}};
  memcpy(Launchpad::pixel_colors, _new_pixel_colors, sizeof(int) * 81 * 2);
  Launchpad::FlushDisplay();
}

void Launchpad::FlushDisplay() {
  display_update_callback();
  display_update_callback();
  display_update_callback();
}

void Launchpad::Begin() {
  DisplayTimer->setMode(1, TIMER_OUTPUT_DISABLED);
  DisplayTimer->setOverflow(1200, HERTZ_FORMAT); //1200
  DisplayTimer->attachInterrupt(display_update_callback);
  DisplayTimer->resume();

  pinMode(DS_LED, OUTPUT);
  pinMode(CP_LED, OUTPUT);
  pinMode(NPL_SW, OUTPUT);
  pinMode(CP_SW, OUTPUT);
  pinMode(Q_SW, INPUT);
  pinMode(MUX1, OUTPUT);
  pinMode(MUX2, OUTPUT);
  pinMode(MUX3, OUTPUT);
  
  Launchpad::ClearDisplay();
  Launchpad::FlushDisplay();
}