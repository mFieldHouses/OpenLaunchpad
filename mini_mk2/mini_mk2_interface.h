namespace Launchpad {
  int led_states[9][9][2]; //3D array that can be written to directly indicating the intensity of the Red an Green LEDs per pixel. Max intensity is 3.
  bool button_states[9][9]; //2D array which is read-only 

  void Begin(); //Internally sets up everything that needs to be set up. To be called in _ready().
  void FlushDisplay();


  void onButtonDown(int posx, int posy); //Callback which is called when any button is pressed.
  void onButtonUp(int posx, int posy); //Callback which is called when any button is released.

  void onAnythingHappened(); //Callback which is called when anything happens, be it a button press or a MIDI message coming in.


  struct ShockWaveParameters;
  void ShockwaveAtPosition(const ShockwaveParamaters& parameters); //Animate a shockwave at a certain position.
  
  void ScrollString(char input_string[], int string_size, int frame_time, int height);
  void ScrollChars(char input_string[], int string_size, int frame_time, int height);

  void DrawBitmap(int bitmap_data[9][9][2]);
  void DrawCharacter(int character_index, int posx, int posy, int red, int green)
}
