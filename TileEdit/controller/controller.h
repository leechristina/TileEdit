#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "view.h"

//events
int get_tile_clicked(SDL_Event event, struct Metadata metadata);
int get_map_tile_clicked(int mouseX, int mouseY, struct Metadata metadata);
void get_position_tex(int position, struct Metadata metadata);
bool inTileArea(SDL_Event event, struct Metadata metadata);
bool inMapArea(int mouseX, int mouseY, struct Metadata metadata);

#endif