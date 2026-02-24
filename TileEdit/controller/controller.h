#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "view.h"

//events
int get_tile_clicked(SDL_Event event, struct Metadata metadata);
int get_map_tile_clicked(int mouseX, int mouseY, struct Metadata metadata);
int onClickTileSet(SDL_Event event, struct Metadata metadata);
void onClickTileMap(SDL_Event event, struct Tilemap tilemap_data, bool* mousedown, bool* mouse_pointer, int curr_tilemap, int position);
void onClickCollisionMap(SDL_Event event, struct Tilemap tilemap_data);
//toggle show or hide mouse pointer
void onPressP(bool* mouse_pointer);
//edit collision map
int onPressE(bool showCollision, int curr_tilemap);
//double the width by adding blank space to the right of any existing map	
void onPressX(struct Tilemap* tilemap_data);
int get_position(SDL_Event event, struct Metadata metadata);
void get_position_tex(int position, struct Metadata metadata);
bool inTileArea(SDL_Event event, struct Metadata metadata);
bool inMapArea(int mouseX, int mouseY, struct Metadata metadata);

#endif