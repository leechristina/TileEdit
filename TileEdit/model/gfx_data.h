#ifndef GFX_DATA_H
#define GFX_DATA_H

#define COLLISION 2
#define FOLDER_SIZE 11
#define FILE_SIZE 300

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h> //capatibility for older c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

struct Tile
{
	uint8_t width;
	uint8_t height;
};

struct Metadata
{
	char * filename;
	struct Tile tile;
	int img_width;
	int img_height;
	uint32_t *map_rows;
	uint32_t *map_cols;
	int startx;
	int starty;
	int endx;
	int endy;
};

struct Tilemap
{
	struct Metadata metadata;
	bool *collisionmap;
	int *tilemap; //bottom layer
	int *tilemap1;
};

//Screen dimension constants
extern const int SCREEN_WIDTH; // 1400;
extern const int SCREEN_HEIGHT; //  900;

extern uint32_t map_rows;
extern uint32_t map_cols;

extern int tile_rows;
extern int tile_cols;

extern int disp_start_rows;
extern int disp_start_cols;
extern int disp_rows;
extern int disp_cols;

extern int tile_width;
extern int tile_height;

extern int tiles_start;
extern int map_size;

extern SDL_Window* mainWindow;
extern SDL_Renderer* mainRenderer;
extern SDL_Texture* pngTexture;

//text data
struct TextData {
		//The actual hardware texture
		SDL_Texture* texture;
		//font this text uses

		char * textureText;
		TTF_Font* ttfFont;
		SDL_Color textColor;

		//Image dimensions
		int width;
		int height;
};

extern struct TextData textTexture;
extern TTF_Font* loadedTTFFont;

extern bool holding_tex;
extern bool mousedown;
//const int FOLDER_SIZE = 11;
extern char folder[FOLDER_SIZE];
//extern char file[FILE_SIZE];

//tile being moved by mouse
extern SDL_Rect active_tex_rect;
//current tile being rendered on map
extern SDL_Rect active_map_tex_rect;
//draw tile gfx
//w and h need to be less than width and height of image to prevent warping due to stretching
extern SDL_Rect srcTileRect;
//w and h need to be same as srcTileRect to prevent warping of image due to stretching
extern SDL_Rect dstTileRect;
extern SDL_Rect dstTileMap;
extern SDL_Rect dstTileMapPlaced;

void closeit(struct Tilemap* tilemap_data);
//TextData functions
void setColor(struct TextData* textTexture, uint8_t r, uint8_t g, int8_t b, uint8_t a );
//free texture
void freeit();

//data
//initialize array with values
void init_arr(int *arr, int val, int num);
void printTileMap(int *tilemap);

void initSDL_Rects();
void setSDL_Rects(struct Metadata metadata);
void setSDL_Tileset_Rects();

void blockcpy(int* dest, int* src, int d_rows, int d_cols, int s_rows, int s_cols);

bool readTileMapFile(struct Tilemap* tilemap_data, const int map_rows, const int map_cols);
void writeTileMapFile(struct Tilemap* tilemap_data, const int map_rows, const int map_cols);
bool readConfigFile(struct Tilemap* tilemap_data);
void readFilesInDir(char folder[]);

#endif
