//little cellular automata playground demo. Initially based on conways game of life, but the ruleset can be edited however you please, all within the interface on the launchpad.

#include "mini_mk2_interface.h"

bool cell_states[8][8] = {{false}};

enum States {SIMULATE, EDIT_KERNEL, EDIT_RULES};
States state = SIMULATE;

struct ButtonDefinition {
  void (*button_action)();
  int x;
  int y;
  int init_red;
  int init_green;
  Launchpad::PixelState init_state;
};

bool process = true;
int process_time = 250;

int lower_bound_death = 1;
int upper_bound_death = 4;

int lower_bound_revitalise = 3;
int upper_bound_revitalise = 3;

void SpeedButtonsTick() {
  if (Launchpad::pixel_colors[0][0][0] > 0) {
    Launchpad::pixel_colors[0][0][0] = 0;
    Launchpad::pixel_colors[0][0][1] = 0;
    Launchpad::pixel_colors[1][0][0] = 7;
    Launchpad::pixel_colors[1][0][1] = 7;
  }
  else {
    Launchpad::pixel_colors[0][0][0] = 7;
    Launchpad::pixel_colors[0][0][1] = 7;
    Launchpad::pixel_colors[1][0][0] = 0;
    Launchpad::pixel_colors[1][0][1] = 0;
  }
}

void ProcessSpeedUp() {
  process_time -= 50;
  //state = SIMULATE;
}

void ProcessSlowDown() {
  process_time += 50;
}

void ClearFieldDisplay() {
  for (int x = 0; x < 9; x++) {
    for (int y = 0; y < 8; y++) {
      Launchpad::pixel_colors[x][y + 1][0] = 0;
      Launchpad::pixel_colors[x][y + 1][1] = 0;
    }
  }
}

void UpdateFieldDisplay() {
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      Launchpad::pixel_colors[x][y + 1][0] = int(cell_states[x][y]) * 7;
      Launchpad::pixel_colors[x][y + 1][1] = 0;
    }
  }
}

bool editing_kernel = false;
int kernel[3][3] = {
  {1,1,1},
  {1,0,1},
  {1,1,1}
};

void UpdateKernelDisplay() {
  ClearFieldDisplay();

  for (int x = 0; x < 3; x++) {
    for (int y = 0; y < 3; y++) {
      Launchpad::pixel_colors[1 + x][2 + y][0] = kernel[x][y] * 4 - 1;
      Launchpad::pixel_colors[1 + x][2 + y][1] = pow(kernel[x][y], 3) * 3 - 3;
    }
  }

  Launchpad::pixel_colors[2][3][0] = 0;
  Launchpad::pixel_colors[2][3][1] = 0;
}

void ToggleEditKernel() {
  if (editing_kernel) {
    editing_kernel = false;
    state = SIMULATE;
    Launchpad::pixel_colors[3][0][0] = 2;
    Launchpad::pixel_colors[3][0][1] = 0;
    Launchpad::pixel_states[3][0] = Launchpad::FLASH_1;

    Launchpad::pixel_intensity_mask[0][0] = 1.0;
    Launchpad::pixel_intensity_mask[1][0] = 1.0;
    Launchpad::pixel_intensity_mask[2][0] = 1.0;
    Launchpad::pixel_intensity_mask[4][0] = 1.0;
    Launchpad::pixel_intensity_mask[7][0] = 1.0;

    UpdateFieldDisplay();
  }
  else {
    editing_kernel = true;
    state = EDIT_KERNEL;
    Launchpad::pixel_colors[3][0][0] = 7;
    Launchpad::pixel_colors[3][0][1] = 5;
    Launchpad::pixel_states[3][0] = Launchpad::FLASH_4;

    Launchpad::pixel_intensity_mask[0][0] = 0.2;
    Launchpad::pixel_intensity_mask[1][0] = 0.2;
    Launchpad::pixel_intensity_mask[2][0] = 0.2;
    Launchpad::pixel_intensity_mask[4][0] = 0.2;
    Launchpad::pixel_intensity_mask[7][0] = 0.2;

    UpdateKernelDisplay();
  }
}

void DrawSlider(int y_level, int value) {
  for (int x = 0; x < value; x++) {
    Launchpad::pixel_colors[x][y_level][0] = 5;
    Launchpad::pixel_colors[x][y_level][1] = 0;
  }

  Launchpad::pixel_colors[value][y_level][0] = 7;
  Launchpad::pixel_colors[value][y_level][1] = 7;
}

void UpdateSliderDisplay() {
  ClearFieldDisplay();

  DrawSlider(1, lower_bound_death);
  DrawSlider(2, upper_bound_death);
  DrawSlider(4, lower_bound_revitalise);
  DrawSlider(5, upper_bound_revitalise);
}

bool editing_rules = false;

void ToggleEditRules() {
  if (editing_rules) {
    editing_rules = false;
    state = SIMULATE;

    Launchpad::pixel_colors[4][0][0] = 0;
    Launchpad::pixel_colors[4][0][1] = 2;
    Launchpad::pixel_states[4][0] = Launchpad::FLASH_1;

    Launchpad::pixel_intensity_mask[0][0] = 1.0;
    Launchpad::pixel_intensity_mask[1][0] = 1.0;
    Launchpad::pixel_intensity_mask[2][0] = 1.0;
    Launchpad::pixel_intensity_mask[3][0] = 1.0;
    Launchpad::pixel_intensity_mask[7][0] = 1.0;

    UpdateFieldDisplay();
  }
  else {
    editing_rules = true;
    state = EDIT_RULES;

    Launchpad::pixel_colors[4][0][0] = 2;
    Launchpad::pixel_colors[4][0][1] = 7;
    Launchpad::pixel_states[4][0] = Launchpad::FLASH_4;

    Launchpad::pixel_intensity_mask[0][0] = 0.2;
    Launchpad::pixel_intensity_mask[1][0] = 0.2;
    Launchpad::pixel_intensity_mask[2][0] = 0.2;
    Launchpad::pixel_intensity_mask[3][0] = 0.2;
    Launchpad::pixel_intensity_mask[7][0] = 0.2;

    UpdateSliderDisplay();
  }
}

int GetNeighborCount(int posx, int posy) {
  int result = 0;

  result += int(cell_states[(posx + 1) % 8][posy % 8]) * kernel[2][1];
  result += int(cell_states[(posx + 1) % 8][(posy + 1) % 8]) * kernel[2][2];
  result += int(cell_states[(posx + 1) % 8][(posy - 1) % 8]) * kernel[2][0];
  result += int(cell_states[(posx - 1) % 8][posy % 8]) * kernel[0][1];
  result += int(cell_states[(posx - 1) % 8][(posy + 1) % 8]) * kernel[0][2];
  result += int(cell_states[(posx - 1) % 8][(posy - 1) % 8]) * kernel[0][0];
  result += int(cell_states[posx % 8][(posy + 1) % 8]) * kernel[1][2];
  result += int(cell_states[posx % 8][(posy - 1) % 8]) * kernel[1][0];

  return result;
}

void LifeTick() {
  bool new_cell_states[8][8] = {{false}};

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      int neighbor_count = GetNeighborCount(x, y);

      if (cell_states[x][y] == true) {
        if (neighbor_count <= lower_bound_death || neighbor_count >= upper_bound_death) {
          new_cell_states[x][y] = false;
        }
        else {
          new_cell_states[x][y] = true;
        }
      }
      else {
        if (neighbor_count >= lower_bound_revitalise && neighbor_count <= upper_bound_revitalise) {
          new_cell_states[x][y] = true;
        }
      }
    }
  }

  memcpy(cell_states, new_cell_states, sizeof(bool) * 64);
}

void RandomiseField() {
  if (state != SIMULATE) {
    return;
  }

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      cell_states[x][y] = bool(random(0,2));
    }
  }
  state = SIMULATE; //not sure why this is neccesary. Calling this function changes the state for some reason.
  UpdateFieldDisplay();
}

void SetState(States new_state) {
  state = new_state;

  switch (new_state) {
    case SIMULATE:
      editing_kernel = false;
    case EDIT_KERNEL:
      process = false;
  }
}

ButtonDefinition buttons[] = {
  {RandomiseField, 2, 0, 3, 7, Launchpad::SOLID},
  {ProcessSpeedUp, 1, 0, 7, 7, Launchpad::SOLID},
  {ProcessSlowDown, 0, 0, 7, 7, Launchpad::SOLID},
  {ToggleEditKernel, 3, 0, 2, 0, Launchpad::FLASH_1},
  {ToggleEditRules, 4, 0, 0, 2, Launchpad::FLASH_1}
};

void Launchpad::onButtonDown(int posx, int posy) {
  for (int idx = 0; idx < std::size(buttons); idx++) {
    if (buttons[idx].x == posx && buttons[idx].y == posy) {
      buttons[idx].button_action();
      return;
    }
  }

  if (state == SIMULATE) {
    if (posx == 7 && posy == 0) {
      process = !process;

      if (process) {
        Launchpad::pixel_colors[7][0][0] = 0;
        Launchpad::pixel_colors[7][0][1] = 7;
        Launchpad::pixel_states[7][0] = Launchpad::FLASH_2;
      }
      else {
        UpdateFieldDisplay();
        Launchpad::pixel_colors[7][0][0] = 7;
        Launchpad::pixel_colors[7][0][1] = 0;
        Launchpad::pixel_states[7][0] = Launchpad::SOLID;
      }
    }
    else if (state == SIMULATE) {
      cell_states[posx][posy - 1] = !cell_states[posx][posy - 1];
      UpdateFieldDisplay();
    }

    state = SIMULATE;
  }


  else if (state == EDIT_KERNEL) {
    if (posx > 0 && posx < 4 && posy > 1 && posy < 5) {
      kernel[posx - 1][posy - 2] += 1;
      kernel[posx - 1][posy - 2] %= 3;
    }
    UpdateKernelDisplay();
  }

  else if (state == EDIT_RULES) {
    if (posy > 0) {
      switch (posy) {
        case 1:
          lower_bound_death = posx;
          break;
        case 2:
          upper_bound_death = posx;
          break;
        case 4:
          lower_bound_revitalise = posx;
          break;
        case 5:
          upper_bound_revitalise = posx;
          break;
      }

      UpdateSliderDisplay();
    }
  }
}

void Launchpad::onButtonUp(int posx, int posy) {
}

void setup() {
  Launchpad::Begin();
  delay(1);

  for (int idx = 0; idx < std::size(buttons); idx++) {
    Launchpad::pixel_colors[buttons[idx].x][buttons[idx].y][0] = buttons[idx].init_red;
    Launchpad::pixel_colors[buttons[idx].x][buttons[idx].y][1] = buttons[idx].init_green;
    Launchpad::pixel_states[buttons[idx].x][buttons[idx].y] = buttons[idx].init_state;
  }
  
  Launchpad::pixel_colors[7][0][0] = 0;
  Launchpad::pixel_colors[7][0][1] = 7;
  Launchpad::pixel_states[7][0] = Launchpad::FLASH_2;

  RandomiseField();
}

void loop() {
  if (state == SIMULATE) {
    if (process) {
      LifeTick();
      UpdateFieldDisplay();
      SpeedButtonsTick();
    }
  delay(process_time);
  }
}