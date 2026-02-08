#ifndef VIEW_H
#define VIEW_H

#include "gfx_data.h"

bool init();
bool loadMedia(struct Tilemap* tilemap_data);
bool loadText();
bool loadTextTextureFromSurface( struct TextData* textTexture);

void render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip );

void DrawMapGrid(struct Metadata metadata);
void DrawTileGrid(int tile_rows, int tile_cols, struct Metadata metadata);
void drawMapCollision(bool *collision, struct Metadata metadata, bool showCollision);
void drawMapTiles(int *tilemap, struct Metadata metadata);
void get_map_position_tex(int position, struct Metadata metadata);

#endif