#include "mini_mk2_interface.h"

bool cell_states[8][8] = {{false}};

void setup() {
  Launchpad::Begin();
  delay(1);
  
  Launchpad::pixel_colors[7][0][0] = 0;
  Launchpad::pixel_colors[7][0][1] = 7;
  Launchpad::pixel_states[7][0] = Launchpad::FLASH_2;

  RandomiseField();
}

bool process = true;

void loop() {
  if (process) {
    UpdateFieldDisplay();
    LifeTick();
  }
  delay(250);
}

void Launchpad::onButtonDown(int posx, int posy) {
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
  else {
    cell_states[posx][posy - 1] = !cell_states[posx][posy - 1];
    //Launchpad::pixel_colors[posx][posy][0] = 7;
    UpdateFieldDisplay();
  }
}

void Launchpad::onButtonUp(int posx, int posy) {
}

void UpdateFieldDisplay() {
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      Launchpad::pixel_colors[x][y + 1][0] = int(cell_states[x][y]) * 7;
      Launchpad::pixel_colors[x][y + 1][1] = 0;
    }
  }
}

int GetNeighborCount(int posx, int posy) {
  int result = 0;

  if (cell_states[(posx + 1) % 8][posy]) {
    result++;
  }
  if (cell_states[(posx + 1) % 8][(posy + 1) % 8]) {
    result++;
  }
  if (cell_states[(posx + 1) % 8][(posy + -1) % 8]) {
    result++;
  }
  if (cell_states[(posx -1) % 8][(posy)]) {
    result++;
  }
  if (cell_states[(posx - 1) % 8][(posy + 1) % 8]) {
    result++;
  }
  if (cell_states[(posx - 1) % 8][(posy - 1) % 8]) {
    result++;
  }
  if (cell_states[posx][(posy + 1) % 8]) {
    result++;
  }
  if (cell_states[posx][(posy - 1) % 8]) {
    result++;
  }

  return result;
}

void LifeTick() {
  bool new_cell_states[8][8] = {{false}};

  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      int neighbor_count = GetNeighborCount(x, y);

      if (cell_states[x][y] == true) {
        if (neighbor_count < 2 || neighbor_count >= 4) {
          new_cell_states[x][y] = false;
        }
        else if (neighbor_count >= 2 && neighbor_count < 4) {
          new_cell_states[x][y] = true;
        }
      }
      else {
        if (neighbor_count == 3) {
          new_cell_states[x][y] = true;
        }
      }
    }
  }

  memcpy(cell_states, new_cell_states, sizeof(bool) * 64);
}

void RandomiseField() {
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 8; y++) {
      //Launchpad::pixel_colors[x][y + 1][0] = round(random(1, 4));
      cell_states[x][y] = bool(random(0,2));
    }
  }
}