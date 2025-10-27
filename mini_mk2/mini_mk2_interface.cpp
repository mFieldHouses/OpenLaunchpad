#include "logo.h"
#include "standard_font.h"
#include "mini_mk2_interface.h"

int Launchpad::led_states[9][9][2] = {{0,0}};
bool Launchpad::button_states[9][9] = {{0}};
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

#define DS_LED PB15
#define CP_LED PB13
#define NPL_SW PA4
#define CP_SW PA5
#define Q_SW PA6
#define MUX1 PB0
#define MUX2 PB1
#define MUX3 PB2

HardwareTimer *DisplayTimer = new HardwareTimer(TIM3);  // DisplayTimer provides a stable refresh rate for the LED matrix and reading out buttons.

void Launchpad::Begin() {
  DisplayTimer->setMode(1, TIMER_OUTPUT_DISABLED);
  DisplayTimer->setOverflow(750, HERTZ_FORMAT); //750
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
  
  Launchpad::FlushDisplay();
}

void Launchpad::FlushDisplay() {
  display_update_callback();
  display_update_callback();
  display_update_callback();
}

void display_update_callback() {
  if (mux_idx == 2) {
    frame++;
  }

  mux_idx++;
  mux_idx = mux_idx % 3;
  frame = frame % 4;

  triggerMux(0);
  displayFragment(mux_idx, frame);
  triggerMux(mux_idx + 1);

  readButtons(mux_idx);

  for (int x = 0; x < 9; x++) {
    for (int y = 0; y < 9; y++) {
      if (button_states[x][y] == true && previous_button_states[x][y] == false) {
        button_down_callback(x,y);
      }
      else if (button_states[x][y] == false && previous_button_states[x][y] == true) {
        button_up_callback(x,y);
      }
    }
  }

  memcpy(previous_button_states, button_states, sizeof(bool) * 81);
}

void displayFragment(int mux_idx, int frame) {
  
  for (int x = 0; x < 3; x++) { //generate array of led color intensities based on mux index: {{64,0}, {32,0}, {50,0}, {0,64}, {RED, GREEN}, etc..} where 64 is max brightness and 0 is disabled
    for (int y = 0; y < 9; y++) {
      memcpy(fragment[led_index_map[(x * 3) + mux_idx][y]], led_states[(x * 3) + mux_idx][y], 2 * sizeof(int));
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

void readButtons(int mux_idx) {
  setState_NPL_SW(LOW); //load parallel data into registers
  setState_NPL_SW(HIGH);  //enable serial shift mode

  for (int temp = 0; temp < 32; temp++) {
    button_states[button_index_map[mux_idx][temp][0]][button_index_map[mux_idx][temp][1]] = readState_Q_SW();
    HC165_clockCycle();
  }
}

struct ShockwaveParameters {
  unsigned int origin_x; 
  unsigned int origin_y;
  bool red_enabled; 
  bool green_enabled; 
  int max_radius;
  int initial_radius;
  int frame_time;
  float thickness;
  float damping;
}

void Launchpad::ShockWaveAtPosition(const ShockwaveParameters& parameters) {
  float _progress = 0.0;
  int _current_radius = parameters.initial_radius;
  int _frame_delay = parameters.frame_time;

  while (_current_radius < parameters.max_radius) {

    for (int x = 0; x < 9; x++) {
      for (int y = 0; y < 9; y++) {
        double _distance_to_origin = sqrt(sq(parameters.origin_x - x) + sq(parameters.origin_y - y));

        if ((_distance_to_origin >= _current_radius && _distance_to_origin <= _current_radius + parameters.thickness) || (_distance_to_origin <= _current_radius && _distance_to_origin >= _current_radius - parameters.thickness)) {
          int _color_at_point[2] = {floor(float(parameters.red_enabled) * 4.0 * (1.0 - _progress)), floor(float(parameters.green_enabled) * 4.0 * (1.0 - _progress))};
          
          memcpy(led_states[x][y], _color_at_point, 2 * sizeof(int));
        }
        else {
          led_states[x][y][0] = 0;
          led_states[x][y][1] = 0;
        }
      }
    }

    _current_radius++;
    delay(_frame_delay);
    _frame_delay /= parameters.damping;
    _progress = float(_current_radius) / float(parameters.max_radius);
  }
}

void Launchpad::DrawBitmap(int bitmap_data[9][9][2]) {
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 9; x++) {
      memcpy(led_states[x][y], bitmap_data[y][x], 2 * sizeof(int));
    }
  }

  display_update_callback();
  display_update_callback();
  display_update_callback();
}

void Launchpad::ScrollString(char input_string[], int string_size, int frame_time, int height) {
  OL_CHAR _result_string[string_size - 1] = {OL_CHAR_QUESTION_MARK};

  for (int char_idx = 0; char_idx < string_size - 1; char_idx++) {
    _result_string[char_idx] = getOLCharIndex(input_string[char_idx]);
  }

  scrollChars(_result_string, string_size - 1, frame_time, height);
}

OL_CHAR GetOLCharIndex(char character) {
  OL_CHAR _result = static_cast<OL_CHAR>(75); //unknown character
  for (int idx = 0; idx < std::size(OL_CHARACTERS); idx++) {
    if (CHAR_MAP[idx] == character) {
      _result = static_cast<OL_CHAR>(idx);
      break;
    }
  }

  return _result;
}

void Launchpad::ScrollChars(OL_CHAR input_chars[], int char_string_length, int frame_time, int height) { //string is array of 
  int _origin_x = 9;
  int _character_offsets[char_string_length] = {0};
  int _total_string_length = 0;

  for (int idx = 1; idx < char_string_length; idx++) {
    _character_offsets[idx] = OL_CHARACTERS[input_chars[idx - 1]].width + 1 + _character_offsets[idx - 1];
    _total_string_length += OL_CHARACTERS[input_chars[idx - 1]].width + 1;
  }

  while (_origin_x > _total_string_length * -1 - 6) {

    clearScreen();
    for (int char_idx = 0; char_idx < char_string_length; char_idx++) {
      writeCharacter(input_chars[char_idx], _character_offsets[char_idx] + _origin_x, height + OL_CHARACTERS[input_chars[char_idx]].y_offset, 4, 1);
    }
    
    delay(frame_time);
    _origin_x -= 1;
  }
}

void Launchpad::DrawCharacter(int character_index, int posx, int posy, int red, int green) {
  int _char_width = OL_CHARACTERS[character_index].width;

  if (posx > -5 && posx < 9 && posy > -7 && posy < 9){
    for (int x = 0; x < _char_width; x++) {
      for (int y = 0; y < 6; y++) {
        if (x + posx > 8 || y + posy > 8) {
          break;
        }

        led_states[x + posx][y + posy][0] = red * OL_CHARACTERS[character_index].bitmap[y][x];
        led_states[x + posx][y + posy][1] = green * OL_CHARACTERS[character_index].bitmap[y][x];
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
  int _new_led_states[9][9][2] = {{0,0}};
  memcpy(led_states, _new_led_states, sizeof(int) * 81 * 2);
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
