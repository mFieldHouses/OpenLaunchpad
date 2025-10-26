#include "stm32f103xb.h"
#include "timer.h"
#include "stm32f1xx.h"
#include "HardwareTimer.h"
#include "logo.h"
#include "standard_font.h"


// CONFIGURATION FOR USER - YOU CAN CHANGE THESE VALUES TO WHATEVER YOU WANT ====

const int splash_screen_mode = 2; //What the splash screen will show upon startup of the launchpad. Possible values are:
// 0: Show logo as defined in logo.h for logo_timeout (line 14) milliseconds.
// 1: Show scrolling text as defined in startup_text (line 16)
// 2: Show animation

const int splash_animation_mode = 0;

const int logo_timeout = 1000;
char startup_text[] = "OpenLaunchpad";
int startup_text_frame_time = 45; //The time before the scrolling text will move one pixel to the left, in milliseconds.
char device_name[] = "OpenLaunchpad Mini MK2"; //The name for the device that will show up when connected to a computer via USB.

// END CONFIGURATION FOR USER ====================================================








int led_status[9][9][2] = {{0,0}};
bool button_status[9][9] = {{0}};
bool previous_button_status[9][9] = {{0}};

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
  MyTim->setOverflow(1200, HERTZ_FORMAT); //1200
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

  // led_status[2][3][1] = 4;
  // led_status[2][3][0] = 4;
  // led_status[2][3][0] = 4;
  // led_status[5][3][0] = 4;
  // led_status[2][2][0] = 4;
  // led_status[5][2][0] = 4;
  // led_status[1][5][0] = 4;
  // led_status[6][5][0] = 4;
  // led_status[2][6][0] = 4;
  // led_status[3][6][0] = 4;
  // led_status[4][6][0] = 4;
  // led_status[5][6][0] = 4;

  // led_status[3][3][0] = 4;
  // led_status[3][4][0] = 3;
  // led_status[3][5][0] = 2;
  // led_status[3][6][0] = 1;
  // led_status[4][3][1] = 4;
  // led_status[4][4][1] = 3;
  // led_status[4][5][1] = 2;
  // led_status[4][6][1] = 1;


  // //writeOrange();

  switch(splash_screen_mode) {
    case 0:
      writeBitmap(LOGO_BITMAP);
      delay(logo_timeout);
      break;
    case 1:
      scrollString(startup_text, std::size(startup_text), startup_text_frame_time);
      break;
    case 2:
      shockWaveAtPosition(8, 0, 1, 1, 14, 5, 0.8, 0.77);
      break;
  }

  timer_callback();
  timer_callback();
  timer_callback();
}

int brightness_subtract = 0;

void timer_callback() {
  frame++;

  //if (frame == 4) {
  mux_idx++;
  //}
  mux_idx = mux_idx % 3;
  frame = frame % 13;

  triggerMux(0);
  displayFragment(mux_idx, frame);
  triggerMux(mux_idx + 1);

  readButtons(mux_idx);

}

int temp_time = 0;

void button_down_callback(int posx, int posy) {
  led_status[posx][posy][0] = 4;
  led_status[posx][posy][1] = 4;
}

void button_up_callback(int posx, int posy) {
  led_status[posx][posy][0] = 0;
  led_status[posx][posy][1] = 0;
}

void loop() {
  
  // if (button_status[0][8] == 1) {
  //   MyTim->setOverflow(750, HERTZ_FORMAT);
  // }
  // else if (button_status[1][8] == 1) {
  //   MyTim->setOverflow(1200, HERTZ_FORMAT);
  // }
  // else {
  //   MyTim->setOverflow(1500, HERTZ_FORMAT);
  // }
  //OL_CHAR _character_index_to_write = OL_CHAR_A;

  for (int x = 0; x < 9; x++) {
    for (int y = 0; y < 9; y++) {
      led_status[x][y][0] = button_status[x][y] * 4;
      led_status[x][y][1] = button_status[x][y] * 4;
    }
  }

  writeCharacter(OL_CHAR_a, 0, 1, 4, 0);
  writeCharacter(OL_CHAR_b, 5, 1, 4, 0);

  // for (int x = 0; x < 9; x++) {
  //   for (int y = 0; y < 9; y++) {
  //     if (button_status[x][y] == true && previous_button_status[x][y] == false) {
  //       button_down_callback(x,y);
  //     }
  //     else if (button_status[x][y] == false && previous_button_status[x][y] == true) {
  //       button_down_callback(x,y);
  //     }
  //   }
  // }

  delay(10);

  //memcpy(previous_button_status, button_status, sizeof(int) * 81);
}

void enable_global_interrupts() {

}

void displayFragment(int mux_idx, int frame) {
  
  for (int x = 0; x < 3; x++) { //generate array of led color intensities based on mux index: {{64,0}, {32,0}, {50,0}, {0,64}, {RED, GREEN}, etc..} where 64 is max brightness and 0 is disabled
    for (int y = 0; y < 9; y++) {
      memcpy(fragment[led_index_map[(x * 3) + mux_idx][y]], led_status[(x * 3) + mux_idx][y], 2 * sizeof(int));
    }
  }

  int serial_data_out[56] = {0};

  for (int idx = 0; idx < 27; idx++) {
    if (pow(fragment[idx][0], 2) > frame) {
      serial_data_out[led_register_map_red[idx]] = 1;
      //serial_data_out[led_register_map_green[idx]] = 1;
    }
    if (pow(fragment[idx][1], 2) > frame) {
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

void shockWaveAtPosition(unsigned int origin_x, unsigned int origin_y, int red, int green, int max_radius, int frame_time, float thickness, float damping) {
  float _progress = 0.0;
  int _current_radius = 0;
  int _frame_delay = frame_time;

  while (_current_radius <= max_radius) {
    for (int x = 0; x < 9; x++) {
      for (int y = 0; y < 9; y++) {
        double _distance_to_origin = sqrt(sq(origin_x - x) + sq(origin_y - y));

        if ((_distance_to_origin >= _current_radius && _distance_to_origin <= _current_radius + thickness) || (_distance_to_origin <= _current_radius && _distance_to_origin >= _current_radius - thickness)) {
          int _color_at_point[2] = {floor(float(red * 4) * (1.0 - _progress)), floor(float(green * 4) * (1.0 - _progress))};
          
          memcpy(led_status[x][y], _color_at_point, 2 * sizeof(int));
        }
        else {
          led_status[x][y][0] = 0;
          led_status[x][y][1] = 0;
        }
      }
    }

    _current_radius++;
    delay(_frame_delay);
    _frame_delay /= damping;
    _progress = float(_current_radius) / float(max_radius);
  }
}

void writeBitmap(int bitmap_data[9][9][2]) {
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 9; x++) {
      memcpy(led_status[x][y], bitmap_data[y][x], 2 * sizeof(int));
    }
  }

  timer_callback();
  timer_callback();
  timer_callback();
}

// void scrollString(char input_string[], int frame_time) {
//   for (int idx = 0; idx < sizeof(input_string)/sizeof(char); idx++) {
//     switch (input_string[idx]) {
//       case 'A':
//         break;
//     }
//   }

// }

void scrollString(char input_string[], int string_size, int frame_time) {
  OL_CHAR _result_string[string_size] = {OL_CHAR_QUESTION_MARK};

  for (int char_idx = 0; char_idx < string_size; char_idx++) {
    _result_string[char_idx] = getOLCharIndex(input_string[char_idx]);
  }

  scrollChars(_result_string, string_size, 35);
}

OL_CHAR getOLCharIndex(char character) {
  OL_CHAR _result = static_cast<OL_CHAR>(75); //unknown character
  for (int idx = 0; idx < std::size(OL_CHARACTERS); idx++) {
    if (CHAR_MAP[idx] == character) {
      _result = static_cast<OL_CHAR>(idx);
      break;
    }
  }

  return _result;
}

void scrollChars(OL_CHAR input_chars[], int char_string_length, int frame_time) { //string is array of 
  int _origin_x = 9;

  while (_origin_x > char_string_length * -4 - 15) {
    clearScreen();
    for (int char_idx = 0; char_idx < char_string_length; char_idx++) {
      writeCharacter(input_chars[char_idx], char_idx * 5 + _origin_x, 2, 4, 1);
    }
    
    delay(frame_time);
    _origin_x -= 1;
  }
}

void writeCharacter(int character_index, int posx, int posy, int red, int green) {
  if (posx > -5 && posx < 9 && posy > -7 && posy < 9){
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 6; y++) {
        if (x + posx > 8 || y + posy > 8) {
          break;
        }

        led_status[x + posx][y + posy][0] = red * OL_CHARACTERS[character_index][y][x];
        led_status[x + posx][y + posy][1] = green * OL_CHARACTERS[character_index][y][x];
      }
    }
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

void clearScreen() {
  int _new_led_status[9][9][2] = {{0,0}};
  memcpy(led_status, _new_led_status, sizeof(int) * 81 * 2);
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

// void drawCharacterAtPosition() {}