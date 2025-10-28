#include "src/mini_mk2_interface.h"
#include "src/logo.h"

// CONFIGURATION FOR USER - YOU CAN CHANGE THESE VALUES TO WHATEVER YOU WANT ====

const int splash_screen_mode = 1; //What the splash screen will show upon startup of the launchpad. Possible values are:
// 0: Show logo as defined in logo.h for logo_timeout (line 14) milliseconds.
// 1: Show scrolling text as defined in startup_text (line 16)
// 2: Show animation

//Startup animation
const int splash_animation_mode = 0;

//Startup logo
const int logo_timeout = 1000;

//Startup scrolling text
char startup_text[] = "OpenLaunchpad";
int startup_text_frame_time = 50; //The time it will take the scrolling text to move on pixel further.

//Screensavers
bool screensaver_enabled = true; //Whether screensavers will show up at all.
int screensaver_type = 0; //The type of screensaver that will show. Possible values are:
// 0: Just black
int screensaver_timeout = 120; //The amount of time that has to pass in which absolutely nothing happened on the launchpad for it to start showing screensavers.

//USB device
char device_name[] = "OpenLaunchpad Mini MK2"; //The name for the device that will show up when connected to a computer via USB.

// END CONFIGURATION FOR USER ====================================================








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
};

bool animation_playing = false;

void setup() {
  Launchpad::Begin();

  // press_params.red_enabled = true;
  // press_params.green_enabled = true;
  // press_params.max_radius = 10;
  // press_params.initial_radius = 0;
  // press_params.frame_time = 15;
  // press_params.thickness = 0.8;
  // press_params.damping = 0.9;

  switch(splash_screen_mode) {
    case 0:
      DrawBitmap(LOGO_BITMAP);
      delay(logo_timeout);
      break;
    case 1:
      ScrollString(startup_text, std::size(startup_text), startup_text_frame_time, 1);
      break;
    case 2:
      ShockwaveParameters params;
      params.origin_x = 15;
      params.origin_y = 0;
      params.red_enabled = true;
      params.green_enabled = true;
      params.max_radius = 30;
      params.initial_radius = 10;
      params.frame_time = 5;
      params.thickness = 0.8;
      params.damping = 0.9;
      
      ShockwaveAtPosition(params);
      break;
  }
}

void loop() {}

void ShockwaveAtPosition(const ShockwaveParameters parameters) {
  animation_playing = true;
  
  float _progress = 0.0;
  int _current_radius = parameters.initial_radius;
  int _frame_delay = parameters.frame_time;

  while (_current_radius < parameters.max_radius) {

    for (int x = 0; x < 9; x++) {
      for (int y = 0; y < 9; y++) {
        double _distance_to_origin = sqrt(sq(parameters.origin_x - x) + sq(parameters.origin_y - y));

        if ((_distance_to_origin >= _current_radius && _distance_to_origin <= _current_radius + parameters.thickness) || (_distance_to_origin <= _current_radius && _distance_to_origin >= _current_radius - parameters.thickness)) {
          int _color_at_point[2] = {floor(float(parameters.red_enabled) * 4.0 * (1.0 - _progress)), floor(float(parameters.green_enabled) * 4.0 * (1.0 - _progress))};
          
          memcpy(Launchpad::led_states[x][y], _color_at_point, 2 * sizeof(int));
        }
        else {
          Launchpad::led_states[x][y][0] = 0;
          Launchpad::led_states[x][y][1] = 0;
        }
      }
    }

    _current_radius++;
    delay(_frame_delay);
    _frame_delay /= parameters.damping;
    _progress = float(_current_radius) / float(parameters.max_radius);
  }

  animation_playing = false;
}

void DrawBitmap(int bitmap_data[9][9][2]) {
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 9; x++) {
      memcpy(Launchpad::led_states[x][y], bitmap_data[y][x], 2 * sizeof(int));
    }
  }
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

void ScrollChars(OL_CHAR input_chars[], int char_string_length, int frame_time, int height) { //string is array of 
  animation_playing = true;
  
  int _origin_x = 9;
  int _character_offsets[char_string_length] = {0};
  int _total_string_length = 0;

  for (int idx = 1; idx < char_string_length; idx++) {
    _character_offsets[idx] = OL_CHARACTERS[input_chars[idx - 1]].width + 1 + _character_offsets[idx - 1];
    _total_string_length += OL_CHARACTERS[input_chars[idx - 1]].width + 1;
  }

  while (_origin_x > _total_string_length * -1 - 6) {

    Launchpad::ClearDisplayBuffer();
    for (int char_idx = 0; char_idx < char_string_length; char_idx++) {
      DrawCharacter(input_chars[char_idx], _character_offsets[char_idx] + _origin_x, height + OL_CHARACTERS[input_chars[char_idx]].y_offset, 4, 1);
    }
    
    delay(frame_time);
    _origin_x -= 1;
  }

  animation_playing = false;
}

void ScrollString(char input_string[], int string_size, int frame_time, int height) {
  OL_CHAR _result_string[string_size - 1] = {OL_CHAR_QUESTION_MARK};

  for (int char_idx = 0; char_idx < string_size - 1; char_idx++) {
    _result_string[char_idx] = GetOLCharIndex(input_string[char_idx]);
  }

  ScrollChars(_result_string, string_size - 1, frame_time, height);
}

void DrawCharacter(int character_index, int posx, int posy, int red, int green) {
  int _char_width = OL_CHARACTERS[character_index].width;

  if (posx > -5 && posx < 9 && posy > -7 && posy < 9){
    for (int x = 0; x < _char_width; x++) {
      for (int y = 0; y < 6; y++) {
        if (x + posx > 8 || y + posy > 8) {
          break;
        }

        Launchpad::led_states[x + posx][y + posy][0] = red * OL_CHARACTERS[character_index].bitmap[y][x];
        Launchpad::led_states[x + posx][y + posy][1] = green * OL_CHARACTERS[character_index].bitmap[y][x];
      }
    }
  }
}

void Launchpad::onButtonDown(int posx, int posy) {
  // press_params.origin_x = posx;
  // press_params.origin_y = posy;

  // Launchpad::ShockwaveAtPosition(press_params);

  // Launchpad::DrawCharacter(1, 2, 2, 3, 1);
  if (!animation_playing) {
    Launchpad::led_states[posx][posy][0] = 3;
    Launchpad::led_states[posx][posy][1] = 0;
  }
  if (Launchpad::button_states[0][0] == 1 && Launchpad::button_states[7][0] == 1) {
    ScrollString(startup_text, std::size(startup_text), startup_text_frame_time, 1);
  }
}

void Launchpad::onButtonUp(int posx, int posy) {
  if (!animation_playing) {
    Launchpad::led_states[posx][posy][0] = 0;
    Launchpad::led_states[posx][posy][1] = 0;
  }
}
