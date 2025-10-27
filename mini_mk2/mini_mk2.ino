#include "stm32f103xb.h"
#include "timer.h"
#include "stm32f1xx.h"
#include "HardwareTimer.h"
#include "mini_mk2_interface.h"

// CONFIGURATION FOR USER - YOU CAN CHANGE THESE VALUES TO WHATEVER YOU WANT ====

const int splash_screen_mode = 2; //What the splash screen will show upon startup of the launchpad. Possible values are:
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


void setup() {
  Launchpad::Begin();

  switch(splash_screen_mode) {
    case 0:
      Launchpad::writeBitmap(LOGO_BITMAP);
      delay(logo_timeout);
      break;
    case 1:
      Launchpad::scrollString(startup_text, std::size(startup_text), startup_text_frame_time, 1);
      break;
    case 2:
      Launchpad::ShockWaveAtPosition(15, 0, 1, 1, 30, 10, 5, 0.8, 0.90);
      break;
  }
}

void loop() {}

void Launchpad::onButtonDown(int posx, int posy) {
  led_states[posx][posy][0] = 3;
  led_states[posx][posy][1] = 3;

  if (button_states[0][0] == 1 && button_states[7][0] == 1) {
    scrollString(startup_text, std::size(startup_text), startup_text_frame_time, 1);
  }
}

void Launchpad::onButtonUp(int posx, int posy) {
  led_states[posx][posy][0] = 0;
  led_states[posx][posy][1] = 0;
}

void onAnythingHappened() {
}
