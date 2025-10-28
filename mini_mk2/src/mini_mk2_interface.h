namespace Launchpad {
  extern int led_states[9][9][2]; //3D array that can be written to directly indicating the intensity of the Red an Green LEDs per pixel. Max intensity is 3.
  extern bool button_states[9][9]; //2D array which is read-only 

  extern void Begin(); //Internally sets up everything that needs to be set up. To be called in _ready().
  extern void FlushDisplay();
  extern void ClearDisplayBuffer();
  extern void ClearDisplay();

  extern void onButtonDown(int posx, int posy); //Callback which is called when any button is pressed.
  extern void onButtonUp(int posx, int posy); //Callback which is called when any button is released.

  extern void onAnythingHappened(); //Callback which is called when anything happens, be it a button press or a MIDI message coming in.
}
