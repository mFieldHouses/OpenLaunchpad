#include "stm32f103xb.h"
#include "timer.h"
#include "stm32f1xx.h"

#include "HardwareTimer.h"

#include "logo.h"







const int NUMLEDS = 81;

int led_status[9][9][2] = {{0,0}};
bool button_status[9][9] = {{0}};

//int led_index_map[9][9] = {{0, 13, 14, 15, 16, 17, 18, 19, 20}, {0, 13, 14, 15, 16, 17, 18, 19, 20}, {0, 13, 14, 15, 16, 17, 18, 19, 20}, {1, 9, 10, 11, 12, 21, 22, 23, 24}, {1, 9, 10, 11, 12, 21, 22, 23, 24}, {1, 9, 10, 11, 12, 21, 22, 23, 24}, {2, 4, 5, 6, 8, 25, 26, 27, 28}, {2, 4, 5, 6, 8, 25, 26, 27, 28}, {2, 4, 5, 6, 8, 25, 26, 27, 28}};
int led_index_map[9][9] = {{0, 11, 12, 13, 14, 15, 16, 17, 18}, {0, 11, 12, 13, 14, 15, 16, 17, 18}, {0, 11, 12, 13, 14, 15, 16, 17, 18}, {1, 7, 8, 9, 10, 19, 20, 21, 22}, {1, 7, 8, 9, 10, 19, 20, 21, 22}, {1, 7, 8, 9, 10, 19, 20, 21, 22}, {2, 3, 4, 5, 6, 23, 24, 25, 26}, {2, 3, 4, 5, 6, 23, 24, 25, 26}, {2, 3, 4, 5, 6, 23, 24, 25, 26}};
int button_index_map[3][32][2] = {
  {{6,7},{6,6},{6,5},{6,4},{6,3},{6,2},{6,1},{3,8},{3,7},{3,6},{3,5},{3,4},{3,3},{3,2},{3,1},{0,8},{0,7},{0,6},{0,5},{0,4},{0,3},{0,2},{0,1},{0,0},{2,2},{2,2},{2,2},{1,1},{1,1},{1,1},{1,1},{1,1}},
  {{7,7},{7,6},{7,5},{7,4},{7,3},{7,2},{7,1},{4,8},{4,7},{4,6},{4,5},{4,4},{4,3},{4,2},{4,1},{1,8},{1,7},{1,6},{1,5},{1,4},{1,3},{1,2},{1,1},{1,0},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2}},
  {{8,7},{8,6},{8,5},{8,4},{8,3},{8,2},{8,1},{5,8},{5,7},{5,6},{5,5},{5,4},{5,3},{5,2},{5,1},{2,8},{2,7},{2,6},{2,5},{2,4},{2,3},{2,2},{2,1},{2,0},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2}}
  };

int led_register_map_green[27] = {0,1,2, 8,9,10,11, 16,17,18,19, 24,25,26,27, 32,33,34,35, 40,41,42,43, 48,49,50,51};
int led_register_map_red[27] = {3,4,5, 12,13,14,15, 20,21,22,23, 28,29,30,31, 36,37,38,39, 44,45,46,47, 52,53,54,55};

int fragment[27][2] = {{0,0}};

int time_multiplier = 10;

int next_frame_timeout;

int x = 0;
int y = 0;

int mux_idx = 1;
int frame = 0; //4 max, when this reaches 4 it gets reset to 0

#define DS_LED PB15
#define CP_LED PB13
#define NPL_SW PA4
#define CP_SW PA5
#define Q_SW PA6
#define MUX1 PB0
#define MUX2 PB1
#define MUX3 PB2

HardwareTimer *MyTim = new HardwareTimer(TIM3);  // TIM3 is MCU hardware peripheral instance, its definition is provided in CMSIS

void setup() {
  //setup_timer();

  MyTim->setMode(1, TIMER_OUTPUT_DISABLED);
  MyTim->setOverflow(750, HERTZ_FORMAT); //750
  MyTim->attachInterrupt(timer_callback);
  MyTim->resume();

  pinMode(DS_LED, OUTPUT);
  pinMode(CP_LED, OUTPUT);
  pinMode(NPL_SW, OUTPUT);
  pinMode(CP_SW, OUTPUT);
  pinMode(Q_SW, INPUT);
  pinMode(MUX1, OUTPUT);
  pinMode(MUX2, OUTPUT);
  pinMode(MUX3, OUTPUT);

  led_status[2][3][1] = 4;
  led_status[2][3][0] = 4;
  led_status[2][3][0] = 4;
  led_status[5][3][0] = 4;
  led_status[2][2][0] = 4;
  led_status[5][2][0] = 4;
  led_status[1][5][0] = 4;
  led_status[6][5][0] = 4;
  led_status[2][6][0] = 4;
  led_status[3][6][0] = 4;
  led_status[4][6][0] = 4;
  led_status[5][6][0] = 4;

  //writeOrange();

  timer_callback();
  timer_callback();
  timer_callback();

  delay(2000);
}

int brightness_subtract = 0;

void timer_callback() {
  frame++;

  //if (frame == 4) {
  mux_idx++;
  //}
  mux_idx = mux_idx % 3;
  frame = frame % 4;

  triggerMux(0);
  displayFragment(mux_idx, frame);
  triggerMux(mux_idx + 1);

  readButtons(mux_idx);

}

int temp_time = 0;

void loop() {
  
  // if (button_status[6][1] == 0) {
  //   MyTim->setOverflow(750, HERTZ_FORMAT);
  // }
  // else {
  //   MyTim->setOverflow(20, HERTZ_FORMAT);
  // }

  //temp_time++;
  //brightness_subtract = (sin(temp_time / 20) * 0.5 + 0.5) * 16.0;

  for (int x = 0; x < 9; x++) {
    for (int y = 0; y < 9; y++) {
      led_status[x][y][0] = button_status[x][y] * 4;
    }
  }

  //MyTim->setOverflow(750 * ((sin(temp_time / 30) * 0.5) + 0.55), HERTZ_FORMAT);
  //readButtons(1);

  // for (int idx = 0; idx < 56; idx++) {
  //   // //HC165_clockCycle();
  //   digitalWrite(CP_SW, HIGH);
  //   delayMicroseconds(1);
  //   digitalWrite(CP_SW, LOW);
  //   delayMicroseconds(1);
  //   if (button_index_map[mux_idx][idx] != -1) {
  //     button_status[(idx * 3) + mux_idx - 1][0] = readState_Q_SW();
  //   }
  // }

  // if (get_interrupt_call()) {
  //   temp_mux_idx++;

  //   set_interrupt_call(false);
  // }

  //TIM3->CNT = 1;

  //writeRed();
  //triggerMux((temp_mux_idx % 3) + 1);
  delay(10);
}

void enable_global_interrupts() {

}

void displayFragment(int mux_idx, int frame) {
  
  for (int x = 0; x < 3; x++) { //generate array of led color intensities based on mux index: {{64,0}, {32,0}, {50,0}, {0,64}, {RED, GREEN}, etc..} where 64 is max brightness and 0 is disabled
    for (int y = 0; y < 9; y++) {
      memcpy(fragment[led_index_map[(x * 3) + mux_idx][y]], led_status[(x * 3) + mux_idx][y], sizeof(int));
    }
  }

  int serial_data_out[56] = {0};

  for (int idx = 0; idx < 27; idx++) {
    if (fragment[idx][0] - brightness_subtract > frame) {
      serial_data_out[led_register_map_red[idx]] = 1;
      //serial_data_out[led_register_map_green[idx]] = 1;
    }
    if (fragment[idx][1] - brightness_subtract > frame) {
      serial_data_out[led_register_map_red[idx]] = 1;
      serial_data_out[led_register_map_green[idx]] = 1;
    }

  }
  writeSerialReverse(serial_data_out, 56);

}

void readButtons(int mux_idx) {
  setState_NPL_SW(LOW); //load parallel data into registers
  setState_NPL_SW(HIGH);  //enable serial shift mode

  for (int temp = 0; temp < 32; temp++) {
    HC165_clockCycle();
    button_status[button_index_map[mux_idx][temp][0]][button_index_map[mux_idx][temp][1]] = readState_Q_SW();
  }
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

void writeSerialReverse(int serial_string_in[], int data_string_size) {
  for (int idx = data_string_size - 1; idx >= 0; idx -= 1) {
    setState_DS_LED(serial_string_in[idx]);
    HC164_clockCycle();
  }
}


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