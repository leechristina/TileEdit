/* 
Copyright (C) 2025 Christina Lee

As described here: https://opensource.org/licenses/BSD-3-Clause

The 3-Clause BSD License
========================

Redistribution and use in source and binary forms, with or without modification, are permitted provided 
that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and 
the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and 
the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or 
promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED 
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*
Author:
Christina Lee
electronsandsuch.com
electronsandsuch@gmail.com

Keyboard Shortcuts:
p: toggle pointer off and on.
s: save tile.map file
d: drop texture being held
m: print metadata info
0: draw on background layer 0
1: draw on layer 1

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h> //capatibility for older c
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#define COLLISION 2
#define FOLDER_SIZE 11
#define FILE_SIZE 300

//Screen dimension constants
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 900;

uint32_t map_rows = 10;
uint32_t map_cols = 20;

int tile_rows = 10;
int tile_cols = 15;

int disp_start_rows = 0;
int disp_start_cols = 0;
int disp_rows = 10;
int disp_cols = 20;

int tile_width = 32;
int tile_height = 32;

int tiles_start = 0;
int map_size = 0;

//The window we'll be rendering to
SDL_Window* mainWindow = NULL;

//The window renderer
SDL_Renderer* mainRenderer = NULL;

//Current displayed PNG image
SDL_Texture* pngTexture = NULL;

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

struct TextData textTexture;
TTF_Font* loadedTTFFont = NULL;

bool holding_tex = false;
bool mousedown = false;
//const int FOLDER_SIZE = 11;
char folder[FOLDER_SIZE] = "tilesets/";
char file[FILE_SIZE] = "tilesets/wood_tileset.png";

//tile being moved by mouse
SDL_Rect active_tex_rect;
//current tile being rendered on map
SDL_Rect active_map_tex_rect;
//draw tile gfx
//w and h need to be less than width and height of image to prevent warping due to stretching
SDL_Rect srcTileRect;
//w and h need to be same as srcTileRect to prevent warping of image due to stretching
SDL_Rect dstTileRect;
SDL_Rect dstTileMap;
SDL_Rect dstTileMapPlaced;

bool init();
bool loadMedia(struct Tilemap* tilemap_data);
bool loadText();
void closeit(struct Tilemap* tilemap_data);

void blockcpy(int* dest, int* src, int d_rows, int d_cols, int s_rows, int s_cols);
bool loadTextTextureFromSurface( struct TextData* textTexture);
void render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip );
//TextData functions
void setColor(struct TextData* textTexture, uint8_t r, uint8_t g, int8_t b, uint8_t a );
//free texture
void freeit();

void initSDL_Rects();
void setSDL_Rects(struct Metadata metadata);
void setSDL_Tileset_Rects();
void DrawMapGrid(struct Metadata metadata);
void DrawTileGrid(int tile_rows, int tile_cols, struct Metadata metadata);
void drawMapCollision(bool *collision, struct Metadata metadata, bool showCollision);
void drawMapTiles(int *tilemap, struct Metadata metadata);
int get_tile_clicked(SDL_Event event, struct Metadata metadata);
int get_map_tile_clicked(int mouseX, int mouseY, struct Metadata metadata);
void get_position_tex(int position, struct Metadata metadata);
void get_map_position_tex(int position, struct Metadata metadata);
bool inTileArea(SDL_Event event, struct Metadata metadata);
bool inMapArea(int mouseX, int mouseY, struct Metadata metadata);

//initialize array with values
void init_arr(int *arr, int val, int num);
void printTileMap(int *tilemap);

bool readTileMapFile(struct Tilemap* tilemap_data, const int map_rows, const int map_cols);
void writeTileMapFile(struct Tilemap* tilemap_data, const int map_rows, const int map_cols);
bool readConfigFile(struct Tilemap* tilemap_data);
void readFilesInDir(char folder[]);


int main( int argc, char* args[] )
{
	bool showCollision = false;
	// editCollision = false;

	
	map_size = map_rows * map_cols;

	SDL_Log("tiles_start: %d pixels map size: %d pixels", tiles_start, map_size);
    
	struct Tilemap tilemap_data = {
		.metadata.filename = file,
		.metadata.map_rows = &map_rows,
		.metadata.map_cols = &map_cols,
		.metadata.tile.width = tile_width,
		.metadata.tile.height = tile_height,
		.metadata.startx = 0,
		.metadata.endx = map_cols
	};
	readConfigFile(&tilemap_data);
	tiles_start = tile_height * map_rows + tile_height;
	initSDL_Rects();

	SDL_Log("startx: %d endx: %d", tilemap_data.metadata.startx, tilemap_data.metadata.endx);

	int mouseX, mouseY;	
	int position = -1;
    
	int curr_tilemap = 0; 

	tilemap_data.tilemap = calloc(map_size, sizeof(int));
	tilemap_data.tilemap1 = calloc(map_size, sizeof(int));
	tilemap_data.collisionmap = calloc(map_size, sizeof(bool));
	init_arr(tilemap_data.tilemap, -1, map_size); 
	init_arr(tilemap_data.tilemap1, -1, map_size); 

	bool mouse_pointer = true;
	printTileMap(tilemap_data.tilemap);
	printTileMap(tilemap_data.tilemap1);

	SDL_Rect held_piece_rect = {
		.x = 0,
		.y = 0,
		.w = SCREEN_WIDTH,
		.h = SCREEN_HEIGHT
	};	
    printf("Tile Editor!\n\n");

	char folder[] = "tilesets";
	char filename[] = "tile.map";
	bool readFile = false;
	if (!readTileMapFile(&tilemap_data, map_rows, map_cols))
	{
		printf("Tilemap file %s does not exist\n", filename);
	}
	else {
		readFile = true;
		SDL_Log("else readTileMapFile success\n");
		SDL_Log("width: %d", tilemap_data.metadata.tile.width);
		//set width and height of tiles in SDL_RECTs
		setSDL_Rects(tilemap_data.metadata);
		tiles_start = tilemap_data.metadata.tile.height * map_rows + tilemap_data.metadata.tile.height;
	}
	readFilesInDir(folder);

	if( !init())
	{
		printf( "Initialization failed!\n" );
	}
	else
	{
		if( !loadMedia(&tilemap_data))
		{
			printf( "SDL image and text loading failed!\n" );
		}
		else
		{	
			//if read in a file, need to get tilemap_data
			if (readFile)
			{
				setSDL_Rects(tilemap_data.metadata);
			}
			bool quit = false;
			SDL_Event e;

			while( !quit )
			{
				while( SDL_PollEvent( &e ) )
				{
					int m_cols = *tilemap_data.metadata.map_cols;
	                int m_rows = *tilemap_data.metadata.map_rows;
					switch(e.type)
					{
						case SDL_QUIT:
					    	quit = true;
				    	    break;

						case SDL_MOUSEBUTTONDOWN:
						    if (inTileArea(e, tilemap_data.metadata))
							{
								printf("clicked mouse in tile area!\n");
								SDL_ShowCursor(SDL_DISABLE);
								position = get_tile_clicked(e, tilemap_data.metadata);
								printf("position: %d\n", position);
								//get active_tex_rect
								get_position_tex(position, tilemap_data.metadata);
								printf("active_tex_rect.x: %d\n", active_tex_rect.x);
								printf("active_tex_rect.y: %d\n", active_tex_rect.y);
								printf("active_tex_rect.w: %d\n", active_tex_rect.w);
								printf("active_tex_rect.h: %d\n\n", active_tex_rect.h);							
								holding_tex = true;
							}
							else if (holding_tex && inMapArea(e.button.x, e.button.y, tilemap_data.metadata))
							{
								printf("clicked map area with tile!\n");
								int index = get_map_tile_clicked(e.button.x, e.button.y, tilemap_data.metadata);
								printf("index: %d\n", index);
								printf("position: %d\n", position);
								switch(curr_tilemap)
								{
									case 0:
									     tilemap_data.tilemap[index] = position;
										 printf("tilemap[index]: %d\n", tilemap_data.tilemap[index]);
										 printf("tilemap[0]: %d\n", tilemap_data.tilemap[0]);
										 printf("tilemap[1]: %d\n", tilemap_data.tilemap[1]);
										 printf("tilemap[2]: %d\n", tilemap_data.tilemap[2]);
									     break;
									case 1:
									     tilemap_data.tilemap1[index] = position;
									     break;
									case COLLISION: 
										SDL_Log("click collision start -- index: %d collisionmap[index]: %d", index, tilemap_data.collisionmap[index]);
										tilemap_data.collisionmap[index] = !tilemap_data.collisionmap[index];
										SDL_Log("index: %d collisionmap[index]: %d", index, tilemap_data.collisionmap[index]);
										break;
								}
								mousedown = true;
								mouse_pointer = false;
							}
							//can edit collision map
							else if (inMapArea(e.button.x, e.button.y, tilemap_data.metadata) && curr_tilemap == COLLISION)
							{
								
								int index = get_map_tile_clicked(e.button.x, e.button.y, tilemap_data.metadata);
								SDL_Log("click collision start no tile -- index: %d collisionmap[index]: %d", index, tilemap_data.collisionmap[index]);
								tilemap_data.collisionmap[index] = !tilemap_data.collisionmap[index];
								SDL_Log("index: %d collisionmap[index]: %d", index, tilemap_data.collisionmap[index]);
							}
							break;
						case SDL_MOUSEBUTTONUP:
						    //stop dragging behavior
							mousedown=false;
							break;
						case SDL_KEYDOWN:
							switch( e.key.keysym.sym )
							{
								//toggle show or hide collision squares
								case SDLK_c:
									SDL_Log("showCollision");
									showCollision = !showCollision;
									break;
								//toggle show or hide mouse pointer
								case SDLK_p:
									printf("p pressed");
									if (!mouse_pointer)
									{
										SDL_ShowCursor(SDL_ENABLE);
										mouse_pointer = true;
									}
									else
									{
										SDL_ShowCursor(SDL_DISABLE);
										mouse_pointer = false;
									}
									break;
								//drop tile	
								case SDLK_d:
									SDL_Log("d pressed!");
									SDL_ShowCursor(SDL_ENABLE);
									holding_tex = false;
									mouse_pointer = true;
									break;
								//edit collision map
								case SDLK_e:
									SDL_Log("e pressed");
									SDL_Log(" showCollision: %d curr_tilemap = %d\n", showCollision, curr_tilemap);
									if (showCollision)
									{
										//editCollision = true;
										curr_tilemap = COLLISION;
									}	
									SDL_Log(" showCollision: %d curr_tilemap = %d\n", showCollision, curr_tilemap);
									break;
								//save file	
								case SDLK_s:
									SDL_Log("s pressed: save");
									writeTileMapFile(&tilemap_data, map_rows, map_cols);	
									break;
								case SDLK_m:
									SDL_Log("metadata: filename: %s tile width: %d tile height: %d", tilemap_data.metadata.filename, tilemap_data.metadata.tile.width, tilemap_data.metadata.tile.height);
									SDL_Log("map width(cols): %u map height(rows): %u", *tilemap_data.metadata.map_cols, *tilemap_data.metadata.map_rows);
									break;
								//double the width by adding blank space to the right of any existing map	
								case SDLK_x:
									SDL_Log("Increase x");
									printTileMap(tilemap_data.tilemap);
									printTileMap(tilemap_data.tilemap1);
								    //cpy current to temp
									int *tilemap_tmp = calloc(m_cols * m_rows, sizeof(int)); 
									memcpy(tilemap_tmp, tilemap_data.tilemap, sizeof(int) * m_cols * m_rows);
									int *tilemap1_tmp = calloc(m_cols * m_rows, sizeof(int));
									memcpy(tilemap1_tmp, tilemap_data.tilemap1, sizeof(int) * m_cols * m_rows);

									//record larger column size
									*tilemap_data.metadata.map_cols *= 2;
									tilemap_data.metadata.endx = *tilemap_data.metadata.map_cols + tilemap_data.metadata.startx; 
									//increase size of tilemaps
									tilemap_data.tilemap = calloc((m_cols * 2) * *tilemap_data.metadata.map_rows, sizeof(int)); 
									tilemap_data.tilemap1 = calloc((m_cols * 2) * *tilemap_data.metadata.map_rows, sizeof(int));
									init_arr(tilemap_data.tilemap, -1, m_cols * 2 * m_rows); 
									init_arr(tilemap_data.tilemap1, -1, m_cols * 2 * m_rows); 
									printTileMap(tilemap_data.tilemap);
									printTileMap(tilemap_data.tilemap1);
									//move saved tilemap data back
									//void blockcpy(void* dest, void* src, int d_rows, int d_cols, int s_rows, int s_cols)
									blockcpy(tilemap_data.tilemap, tilemap_tmp, (int)m_rows, (int)m_cols * 2, (int)m_rows, (int)m_cols);
									blockcpy(tilemap_data.tilemap1, tilemap1_tmp, (int)m_rows, (int)m_cols * 2, (int)m_rows, (int)m_cols);
									
									printTileMap(tilemap_data.tilemap);
									printTileMap(tilemap_data.tilemap1);
									break;
								//double the height by adding blank space to the top of any existing map
								case SDLK_y:
									break;
								//up, down, left right arrow keys (move 5 up,down,left, right or by the max amt tiles left if less than 5)
								case SDLK_LEFT:
									//TODO: Debug
									SDL_Log("Go left");
									SDL_Log("before startx: %d endx: %d", tilemap_data.metadata.startx, tilemap_data.metadata.endx);
									//if curr start >= 5
									//if (tilemap_data.metadata.startx >= 5)
									//{
                                		tilemap_data.metadata.startx += 5;
										tilemap_data.metadata.endx += 5;
									//}
									//else if curr start >= 0
									/*
									else if (tilemap_data.metadata.startx >= 0)
									{
										uint32_t temp = tilemap_data.metadata.startx;
										tilemap_data.metadata.startx = 0;
										tilemap_data.metadata.endx -= temp;
									}
									*/
									SDL_Log("after startx: %d endx: %d", tilemap_data.metadata.startx, tilemap_data.metadata.endx);
									//else if curr start = 0 do nothing
									break;

								case SDLK_RIGHT:
									//TODO: Debug
									SDL_Log("go right");
									SDL_Log("before startx: %d endx: %d", tilemap_data.metadata.startx, tilemap_data.metadata.endx);
									uint32_t end_disp = tilemap_data.metadata.endx;
									//if curr end < m_cols - 5
									//if (end_disp <= m_cols - 5)
									//{
                                    	tilemap_data.metadata.startx -= 5;
										tilemap_data.metadata.endx -= 5;
									//}
									//else if curr end <= m_cols
									/*
									else if (end_disp <= m_cols)
									{
										uint32_t temp = tilemap_data.metadata.startx;
										tilemap_data.metadata.startx += temp; 
										tilemap_data.metadata.endx += temp; 
									}
									*/
									SDL_Log("after startx: %d endx: %d", tilemap_data.metadata.startx, tilemap_data.metadata.endx);
									//end = size;
									//else if curr end = size
									break;
									
								case SDLK_0:
								    SDL_Log("Layer 0");
								    curr_tilemap = 0;
									break;
								case SDLK_1:
									SDL_Log("Layer 1");
									curr_tilemap = 1;
									break;

							}
						
				    }
				}	

				//Clear screen
				SDL_SetRenderDrawColor( mainRenderer, 0, 0, 0, 255 );
				SDL_RenderClear( mainRenderer );
				
				SDL_GetMouseState(&mouseX, &mouseY);
				//if dragging a texture across tilemap area
				if (holding_tex && inMapArea(mouseX, mouseY, tilemap_data.metadata) && mousedown == true)
				{
					printf("clicked map area with tile!\n");
					//SDL_ShowCursor(SDL_ENABLE);
					//add position to tilemap array
					//active_tex_rect = NULL;
					
					int index = get_map_tile_clicked(mouseX, mouseY, tilemap_data.metadata);
					printf("index: %d\n", index);
					printf("position: %d\n", position);
					switch(curr_tilemap)
					{
						case 0:
							tilemap_data.tilemap[index] = position;
							printf("tilemap[index]: %d\n", tilemap_data.tilemap[index]);
							printf("tilemap[0]: %d\n", tilemap_data.tilemap[0]);
							printf("tilemap[1]: %d\n", tilemap_data.tilemap[1]);
							printf("tilemap[2]: %d\n", tilemap_data.tilemap[2]);
							break;
						case 1:
							tilemap_data.tilemap1[index] = position;
							break;
						case COLLISION:
							SDL_Log("edit collisionmap -- index: %d collisionmap[index]: %d ", index, tilemap_data.collisionmap[index]);
							tilemap_data.collisionmap[index] = !tilemap_data.collisionmap[index];
							SDL_Log("end edit collisionmap -- index: %d collisionmap[index]: %d ", index, tilemap_data.collisionmap[index]);
							break;
					}
					//position = -1;

					//printTileMap(tilemap);
					//holding_tex = false;
				}

				//printTileMap(tilemap);
				DrawMapGrid(tilemap_data.metadata);
				drawMapTiles(tilemap_data.tilemap, tilemap_data.metadata);
				drawMapTiles(tilemap_data.tilemap1, tilemap_data.metadata);
				drawMapCollision(tilemap_data.collisionmap, tilemap_data.metadata, showCollision);

				//draw tileset image and tile grid
                SDL_RenderCopy(mainRenderer, pngTexture, &srcTileRect, &dstTileRect);
				DrawTileGrid(tile_rows, tile_cols, tilemap_data.metadata);

				render( ( SCREEN_WIDTH - textTexture.width ) / 2, ( SCREEN_HEIGHT - textTexture.height ) / 2, NULL, 0.0, NULL, SDL_FLIP_NONE );

				//if holding a map piece
				if (holding_tex)
				{
					//printf("MouseX: %d, MouseY: %d", MouseX, MouseY);
					//draw held piece
					SDL_GetMouseState(&mouseX, &mouseY);
					held_piece_rect.x = mouseX-tilemap_data.metadata.tile.width/2;
					held_piece_rect.y = mouseY-tilemap_data.metadata.tile.width/2;
					held_piece_rect.w = tilemap_data.metadata.tile.width;
					held_piece_rect.h = tilemap_data.metadata.tile.height;	
					SDL_RenderCopy(mainRenderer, pngTexture, &active_tex_rect, &held_piece_rect);
				} 
				//Update screen
				SDL_RenderPresent( mainRenderer );
				//writeTileMapFile(&tilemap_data, MAP_ROWS, MAP_COLS);

			}

		}
	}
	//Free resources and close SDL
	closeit(&tilemap_data);

	return 0;
}

//SDL
//init the window, renderer, SDL_image, and SDL_ttf
bool init()
{
	bool success = true;

	if( SDL_Init(SDL_INIT_EVERYTHING) < 0 )
	{
		printf( "SDL initialization failed! Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		mainWindow = SDL_CreateWindow( "Tile Editor!", 
			SDL_WINDOWPOS_UNDEFINED, 
			SDL_WINDOWPOS_UNDEFINED, 
			SCREEN_WIDTH, 
			SCREEN_HEIGHT, 
			SDL_WINDOW_SHOWN );
		if( mainWindow == NULL )
		{
			printf( "SDL window creation failed! Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			mainRenderer = SDL_CreateRenderer( mainWindow, -1, SDL_RENDERER_ACCELERATED );
			if( mainRenderer == NULL )
			{
				printf( "SDL Renderer could not be created! Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor( mainRenderer, 255, 255, 255, 255 );
				//add transparency rendering
				SDL_SetRenderDrawBlendMode(mainRenderer, SDL_BLENDMODE_BLEND);

			}
            int imgFlags = IMG_INIT_PNG;
            if( !( IMG_Init( imgFlags ) & imgFlags ) )
            {
                printf( "SDL_image initialization failed! Error: %s\n", IMG_GetError() );
                success = false;
            }
			if( TTF_Init() == -1 )
			{
					printf( "SDL_ttf initialization failed! Error: %s\n", TTF_GetError() );
					success = false;
			}
		}
	}

	return success;
}


//DATA HANDLING

//initialize array with values
void init_arr(int* arr, int val, int num)
{
	printf("enter init_arr\n");
	for (int i = 0; i < num; ++ i)
	{
		//append first char from c and null terminator
		arr[i] = val;
	}
	printf("exit init_arr\n");
}

void printTileMap(int *tilemap)
{
	printf("enter printTileMap map_rows: %d map_cols: %d\n", map_rows, map_cols);
	for (int i=0; i < map_rows; ++i)
	{
		for(int j=0; j < map_cols; ++j)
		{
			//set active_tex_rect
			printf("%d ",tilemap[i*map_rows + j]);
		}
		printf("\n");
	}
	printf("exit printTileMap\n");
}

bool inTileArea(SDL_Event e, struct Metadata metadata)
{
	return e.button.x >= 0 && e.button.x < metadata.tile.width*tile_cols && e.button.y >= tiles_start 
	&& e.button.y < tiles_start + metadata.tile.height*tile_rows;
}

void readFilesInDir(char folder[])
{
	fprintf(stdout, "enter readFilesInDir\n");
	struct dirent *dir_entry;  // Pointer for directory entry
    DIR *dr = opendir(folder);

    if (dr == NULL) 
    {
        printf("Could not open directory %s", folder );
    }

    // Refer https://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    while ((dir_entry = readdir(dr)) != NULL)
            printf("%s\n", dir_entry->d_name);

    closedir(dr); 
	fprintf(stdout, "exit readFilesInDir\n");   
}

int readNewline(FILE* file)
{
	char newline = ' ';
	int items_written = fread(&newline, sizeof(char), 1, file);	

	return items_written;
}

bool readConfigFile(struct Tilemap* tilemap_data)
{
	FILE* fileh;
	SDL_Log("folder: %s", folder);
	char* configFile = "config/config.dat";
	
    fileh = fopen(configFile, "r");
	if (fileh == NULL) {
        printf("Error opening %s!", configFile);
        return false;
    }
    int items_written = 0;
	char fname_sizec[4] = "000";
	char tilewc[4] = "000";
	char tilehc[4] = "000";
	uint8_t fname_size = 0;
	items_written = fread(&fname_sizec, sizeof(char), 3, fileh);
	fname_sizec[3]='\0';
    readNewline(fileh);
	fname_size = atoi(fname_sizec);
    items_written = fread(tilemap_data->metadata.filename, sizeof(char), fname_size, fileh);
	tilemap_data->metadata.filename[fname_size]='\0';

	char * temp = calloc(FOLDER_SIZE + fname_size + 2, sizeof(char));
	strcpy(temp, folder);
	strcat(temp, tilemap_data->metadata.filename);
	strcpy (tilemap_data->metadata.filename, temp);
	strcpy(file, tilemap_data->metadata.filename);

	readNewline(fileh);
	items_written = fread(&tilewc, sizeof(char), 3, fileh);
	tilewc[3]='\0';
	tilemap_data->metadata.tile.width = atoi(tilewc);
    readNewline(fileh);
	items_written = fread(&tilehc, sizeof(char), 3, fileh);
	tilemap_data->metadata.tile.height = atoi(tilehc);
	tilehc[3]='\0';
	tile_width = tilemap_data->metadata.tile.width;
	tile_height = tilemap_data->metadata.tile.height;
	
	fclose(fileh);

	SDL_Log("End readConfigFile() fname_sizec: %s, fname_size: %d, tilemap_data->metadata.filename: %s\n", fname_sizec, fname_size, tilemap_data->metadata.filename);
	SDL_Log("tilewc: %s tilehc: %s", tilewc, tilehc);

	return true;
}

//todo: tile width, tile height, filename in a metadata struct
//bool readTileMapFile(int tilemap[], struct Metadata *metadata, const int c_map_rows, const int c_map_cols)
bool readTileMapFile(struct Tilemap* tilemap_data, const int c_map_rows, const int c_map_cols)
{
	SDL_Log("enter readTileMapFile");
	FILE* file;
	
    file = fopen("tile.map", "rb");

    if (file == NULL) {
        printf("Error opening tile.map!");
        return false;
    }

	int items_written = 0;
	//int tile = -2;
	uint8_t fname_size = 0;
	items_written = fread(&fname_size, sizeof(uint8_t), 1, file);
    items_written = fread(tilemap_data->metadata.filename, sizeof(char), fname_size, file);
	SDL_Log("sizeof(struct Tile): %ld", sizeof(struct Tile));
	SDL_Log("Read Filename: %s\n", tilemap_data->metadata.filename);
	//get width and height of a tile
	items_written = fread(&(tilemap_data->metadata.tile), sizeof(struct Tile), 1, file);
	SDL_Log("Finished fread 1");

	//get size of tilemap memory
	SDL_Log("sizeof(uint32_t): %d", sizeof(uint32_t));
	SDL_Log("map_cols before read: %u\n",*tilemap_data->metadata.map_cols); //20
	SDL_Log("map_rows before read: %u\n",*tilemap_data->metadata.map_rows); //10
	items_written = fread(tilemap_data->metadata.map_cols, sizeof(uint32_t), 1, file); //width of map
	items_written = fread(tilemap_data->metadata.map_rows, sizeof(uint32_t), 1, file); //height of map
	SDL_Log("map_cols: %u\n",*tilemap_data->metadata.map_cols); //20
	SDL_Log("map_rows: %u\n",*tilemap_data->metadata.map_rows); //10
	
	//allocate tilemap memory
	int m_cols = *tilemap_data->metadata.map_cols;
	int m_rows = *tilemap_data->metadata.map_rows;
	int m_size = m_cols * m_rows;
	tilemap_data->tilemap = calloc(m_size, sizeof(int));
	tilemap_data->tilemap1 = calloc(m_size, sizeof(int));
	init_arr(tilemap_data->tilemap, -1, m_size); 
	init_arr(tilemap_data->tilemap1, -1, m_size);

	//read data into tilemap memory
	items_written = fread(tilemap_data->tilemap, sizeof(int), m_rows*m_cols, file);
	items_written = fread(tilemap_data->tilemap1, sizeof(int), m_rows*m_cols, file);
	items_written = fread(tilemap_data->collisionmap, sizeof(bool), m_rows*m_cols, file);

    SDL_Log("Finished fread 2");
	SDL_Log("Read Filename: %s\n", tilemap_data->metadata.filename);
	SDL_Log("Read Width: %d\n", tilemap_data->metadata.tile.width);
	SDL_Log("Read Height: %d\n", tilemap_data->metadata.tile.height);
    fclose(file);
	file = NULL;
	SDL_Log("exit readTileMapFile");	
	return true;
}

void writeTileMapFile(struct Tilemap* tilemap_data, const int c_map_rows, const int c_map_cols)
{
	SDL_Log("writeTileMapFile");
	fprintf(stdout,"tilemap_data->metadata.filename: %s\n", tilemap_data->metadata.filename);
	fprintf(stdout,"strlen(tilemap_data->metadata.filename): %d\n", strlen(tilemap_data->metadata.filename));
    FILE* file = fopen("tile.map", "wb");

    if (file == NULL) {
        printf("Error creating tile.map");
        exit(1);
    }

    //int tile;
	int m_cols = *tilemap_data->metadata.map_cols;
	int m_rows = *tilemap_data->metadata.map_rows;
	uint8_t fname_size = (uint8_t)strlen(tilemap_data->metadata.filename);
	int items_written = 0;
	items_written = fwrite(&fname_size, sizeof(uint8_t), 1, file);
	items_written = fwrite(tilemap_data->metadata.filename, sizeof(char), strlen(tilemap_data->metadata.filename), file);
	items_written = fwrite(&tilemap_data->metadata.tile, sizeof(struct Tile), 1, file); //width and height of tile
	items_written = fwrite(tilemap_data->metadata.map_cols, sizeof(int), 1, file); //width of map
	items_written = fwrite(tilemap_data->metadata.map_rows, sizeof(int), 1, file); //height of map
	items_written = fwrite(tilemap_data->tilemap, sizeof(int), m_rows*m_cols, file);
	items_written = fwrite(tilemap_data->tilemap1, sizeof(int), m_rows*m_cols, file);
	items_written = fwrite(tilemap_data->collisionmap, sizeof(bool), m_rows*m_cols, file);
	//tilemap_data.collisionmap = calloc(map_size, sizeof(bool));
    //printf("Wrote Filename: %s", metadata.filename);
    fclose(file);
	file = NULL;
}


bool inMapArea(int mouseX, int mouseY, struct Metadata metadata)
{
	return mouseX >= 0 && 
		mouseX < metadata.tile.width*map_cols && 
	    mouseY >= 0 && 
		mouseY < metadata.tile.height*map_rows;
}


//get index of map tile clicked
int get_map_tile_clicked(int mouseX, int mouseY, struct Metadata metadata)
{
    int col = mouseX/metadata.tile.width;
    int row = mouseY/metadata.tile.height;
    int tile_index = row*map_cols+col;

    return tile_index;
}

//get index of png tile clicked
int get_tile_clicked(SDL_Event event, struct Metadata metadata)
{

	int yval = event.button.y; 
	int start_val = tiles_start; //needed to convert this to int before selection worked
    int col = event.button.x/metadata.tile.width;
    int row = (yval-start_val)/metadata.tile.height;
	SDL_Log("col: %d row: %d", col, row);
    int tile_index = row*tile_cols+col;

    return tile_index;
}

//get src rectangle to use for mouse moved texture piece 
void get_position_tex(int position, struct Metadata metadata)
{
	int y = position / tile_cols;
	int x = position % tile_cols;
	active_tex_rect.x = x*metadata.tile.width;
    active_tex_rect.y = y*metadata.tile.height;
}

bool loadMedia(struct Tilemap* tilemap_data)
{
	fprintf(stdout, "enter loadMedia file: %s tilemap_data->metadata.filename: %s\n", file, tilemap_data->metadata.filename);
	bool success = true;

	pngTexture = IMG_LoadTexture(mainRenderer, file);
	if( pngTexture == NULL )
	{
		printf( "Failed to load PNG image %s!\n", file );
		success = false;
	}
	float w = 0.0;
	float h = 0.0;
	fprintf(stdout, "tile_width: %d tile_height: %d\n", tile_width, tile_height);
	fprintf(stdout, "metadata img_width: %d img_height: %d\n", tilemap_data->metadata.img_width, tilemap_data->metadata.img_height);
	SDL_QueryTexture(pngTexture, NULL, NULL, &tilemap_data->metadata.img_width, &tilemap_data->metadata.img_height);
	fprintf(stdout, "metadata img_width: %d img_height: %d\n", tilemap_data->metadata.img_width, tilemap_data->metadata.img_height);
	//tile_width = tilemap_data->metadata.tile.width;
	//tile_height = tilemap_data->metadata.tile.height;
	tilemap_data->metadata.img_width = tilemap_data->metadata.img_width - tilemap_data->metadata.img_width % tile_width;
	tilemap_data->metadata.img_height = tilemap_data->metadata.img_height - tilemap_data->metadata.img_height % tile_height;
	fprintf(stdout, "tilemap_data width: %d height: %d\n", tilemap_data->metadata.img_width, tilemap_data->metadata.img_height);
	fprintf(stdout, "tile_cols: %d tile_rows: %d\n", tile_cols, tile_rows);
	tile_cols = tilemap_data->metadata.img_width/tile_width;
	tile_rows = tilemap_data->metadata.img_height/tile_height;
	fprintf(stdout, "tile_cols: %d tile_rows: %d\n", tile_cols, tile_rows);
	setSDL_Tileset_Rects();

	success = loadText();

	return success;
}

void closeit(struct Tilemap* tilemap_data)
{
	//Free text texture if it exists
	if( textTexture.texture != NULL )
	{
		SDL_DestroyTexture( textTexture.texture );
		textTexture.texture = NULL;
		textTexture.width = 0;
		textTexture.height = 0;
	}
	//dispose of ttf font
	TTF_CloseFont(loadedTTFFont);
	loadedTTFFont = NULL;
	textTexture.ttfFont = NULL;
	//Free image texture if it exists
	if( pngTexture != NULL )
	{
		SDL_DestroyTexture( pngTexture );
		pngTexture = NULL;
	}
	//free window
	SDL_DestroyWindow( mainWindow );

	free(tilemap_data->tilemap);
	tilemap_data->tilemap = NULL;
	free(tilemap_data->tilemap1);
	tilemap_data->tilemap1 = NULL;	
	//filename not currently malloc'ed but assigned a static array so doesn't need free
	//free(tilemap_data->metadata.filename);
	tilemap_data->metadata.filename = NULL;
	tilemap_data = NULL;
	mainWindow = NULL;
	SDL_Quit();
}

//precondition: dest size >= src size
void blockcpy(int* dest, int* src, int d_rows, int d_cols, int s_rows, int s_cols)
{
	int src_i = 0;
	for (int row = 0; row < d_rows; ++row)
	{
		for (int col=0; col < d_cols; ++col)
		{
			if (col < s_cols && row < s_rows)
			{
				//dst[src_i] = src[row * d_cols + col];
				dest[row * d_cols + col] = src[src_i];
				++src_i;
			}
		}
	}
}

void setColor(struct TextData* textTexture, uint8_t r, uint8_t g, int8_t b, uint8_t a)
{
	textTexture->textColor.r = r;
	textTexture->textColor.g = g;
	textTexture->textColor.b = b;
	textTexture->textColor.a = a;
}

void DrawMapGrid(struct Metadata metadata)
{
    //Render gray outlined rect
    SDL_Rect outlineRect = { 
        0,   //x 
        0,   //y
        metadata.tile.width,  //width 
        metadata.tile.height   //height
    };
    for (int row = 0; row < map_rows; ++row)
    {
        for (int col = 0; col < map_cols; ++col)
        {
            SDL_SetRenderDrawColor( mainRenderer, 170, 170, 170, 255 );		
            SDL_RenderDrawRect( mainRenderer, &outlineRect );
            outlineRect.x += metadata.tile.width;
        }
        outlineRect.x = 0;
        outlineRect.y += metadata.tile.height;
    }
}

void drawMapCollision(bool *collision, struct Metadata metadata, bool showCollision)
{
	//printf("drawMapTiles");
	const int tile_width = metadata.tile.width;
	const int tile_height = metadata.tile.height;
	int disp_startx = metadata.startx; //base on metadata
	int disp_endx = metadata.endx; // base on metadata
	int disp_cols = disp_endx - disp_startx + 1;
	//SDL_Log("disp_startx: %u disp_endx: %u", disp_startx, disp_endx);
	for (int i=0; i < map_rows; ++i)
	{
		//move ahead disp_startx
		for(int j=0; j < map_cols; ++j)
		{
			//if location != -1 
			//TODO AND location j < disp_endx
			if (j + disp_startx >= 0 && j + disp_startx < map_cols && collision[i*map_cols + j + disp_startx])
			{
				dstTileMapPlaced.x = j * tile_width ;
				dstTileMapPlaced.y = i * tile_height;
				//(mainRenderer, pngTexture, &active_map_tex_rect, &dstTileMapPlaced);
				
				if (showCollision)
				{ 
					SDL_SetRenderDrawColor(mainRenderer,
                   	0, 0, 255,
                	90);
					SDL_RenderFillRect(mainRenderer, &dstTileMapPlaced);
				}
			}
		}
		//move ahead map_cols - disp_endx
	}
}

void drawMapTiles(int *tilemap, struct Metadata metadata)
{
	//printf("drawMapTiles");
	const int tile_width = metadata.tile.width;
	const int tile_height = metadata.tile.height;
	int disp_startx = metadata.startx; //base on metadata
	int disp_endx = metadata.endx; // base on metadata
	int disp_cols = disp_endx - disp_startx + 1;
	//SDL_Log("disp_startx: %u disp_endx: %u", disp_startx, disp_endx);
	for (int i=0; i < map_rows; ++i)
	{
		//move ahead disp_startx
		for(int j=0; j < map_cols; ++j)
		{
			//if location != -1 
			//TODO AND location j < disp_endx
			if (j + disp_startx >= 0 && j + disp_startx < map_cols && tilemap[i*map_cols + j + disp_startx] != -1)
			{
				//set active_map_tex_rect
				get_map_position_tex(tilemap[i*map_cols + j + disp_startx], metadata);
				dstTileMapPlaced.x = j * tile_width ;
				dstTileMapPlaced.y = i * tile_height;
				SDL_RenderCopy(mainRenderer, pngTexture, &active_map_tex_rect, &dstTileMapPlaced);
			}
		}
		//move ahead map_cols - disp_endx
	}
}

//rectangle for tiled png
void get_map_position_tex(int position, struct Metadata metadata)
{
	int y = position / tile_cols;
	int x = position % tile_cols;
	active_map_tex_rect.x = x*metadata.tile.width;
    active_map_tex_rect.y = y*metadata.tile.height;
}

void DrawTileGrid(int tile_rows, int tile_cols, struct Metadata metadata)
{

    //Render gray outlined rect
    SDL_Rect outlineRect = { 
        0,   //x 
        metadata.tile.height*map_rows + metadata.tile.height,   //y
        metadata.tile.width,  //width 
        metadata.tile.height   //height
    };
    for (int row = 0; row < tile_rows; ++row)
    {
        for (int col = 0; col < tile_cols; ++col)
        {
			SDL_SetRenderDrawColor( mainRenderer, 170, 170, 170, 100 );		
            SDL_RenderDrawRect( mainRenderer, &outlineRect );
            outlineRect.x += metadata.tile.width;
        }
        outlineRect.x = 0;
        outlineRect.y += metadata.tile.height;
    }
}

bool loadText()
{
	bool success = true;
	char* font_name = "Catamaran-Black.ttf";

	//load all fonts used
	loadedTTFFont = TTF_OpenFont(font_name , 28 );
	if( loadedTTFFont == NULL )
	{
		textTexture.ttfFont = NULL;
		printf( "Failed to load font %s! Error: %s\n", font_name, TTF_GetError() );
		success = false;
	}
	else
	{
		textTexture.ttfFont = loadedTTFFont;
		textTexture.textureText = "Tile Editor!!";
		
		setColor(&textTexture, 255, 0, 255, 255);
		if( !loadTextTextureFromSurface( &textTexture ) )
		{
			printf( "Failed to render text texture from surface!\n" );
			success = false;
		}
	}

	return success;
}

bool loadTextTextureFromSurface(struct TextData* textTexture )
{
	freeit();

	//temp surface for text before converting to texture
	SDL_Surface* textSurface = TTF_RenderText_Solid( textTexture->ttfFont, textTexture->textureText, textTexture->textColor );
	if( textSurface == NULL )
	{
		printf( "Failed to render text surface! Error: %s\n", TTF_GetError() );
	}
	else
	{
        textTexture->texture = SDL_CreateTextureFromSurface( mainRenderer, textSurface );
		if(textTexture->texture == NULL )
		{
			printf( "Failed to create texture from rendered text! Error: %s\n", SDL_GetError() );
		}
		else
		{
			textTexture->width = textSurface->w;
			textTexture->height = textSurface->h;
		}
		if( TTF_Init() == -1 )
		{
			printf( "Failed to initialize SDL_ttf! Error: %s\n", TTF_GetError() );
		}
		SDL_FreeSurface( textSurface );
	}
	
	return textTexture->texture != NULL;
}

void render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	SDL_Rect renderQuad = { x, y, textTexture.width, textTexture.height };
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}
	SDL_RenderCopyEx( mainRenderer, textTexture.texture, clip, &renderQuad, angle, center, flip );
}

void freeit()
{
	//Free texture if it exists
	if( textTexture.texture != NULL )
	{
		SDL_DestroyTexture( textTexture.texture );
		textTexture.texture = NULL;
		textTexture.width = 0;
		textTexture.height = 0;
	}
}

//set the global SDL_RECTs
void setSDL_Rects(struct Metadata metadata)
{
	SDL_Log("Enter setSDL_Rects");
	//tile being moved by mouse
	active_tex_rect.w = metadata.tile.width;
	active_tex_rect.h = metadata.tile.height;

	//current tile being rendered on map
	active_map_tex_rect.w = metadata.tile.width;
	active_map_tex_rect.h = metadata.tile.height;

	//draw tile gfx
	//w and h need to be less than width and height of image to prevent warping due to stretching
	srcTileRect.w = metadata.img_width;
	srcTileRect.h = metadata.img_height;

	//w and h need to be same as srcTileRect to prevent warping of image due to stretching
	dstTileRect.y = metadata.tile.height*map_rows + metadata.tile.height;
	dstTileRect.w = metadata.img_width;
	dstTileRect.h = metadata.img_height;

	dstTileMap.w = metadata.tile.width;
	dstTileMap.h = metadata.tile.height;

	dstTileMapPlaced.w = metadata.tile.width;
	dstTileMapPlaced.h = metadata.tile.height;

	tile_width = metadata.tile.width;
	tile_height = metadata.tile.height;

	tile_cols = metadata.img_width/tile_width;
	tile_rows = metadata.img_height/tile_height;
	SDL_Log("Exit setSDL_Rects");
}

void setSDL_Tileset_Rects()
{
	srcTileRect.x = 0;
	srcTileRect.y = 0;
	srcTileRect.w = tile_width * tile_cols;
	srcTileRect.h = tile_height * tile_rows;

	//w and h need to be same as srcTileRect to prevent warping of image due to stretching
	dstTileRect.x = 0;
	dstTileRect.y = tile_height * map_rows + tile_height;
	dstTileRect.w = tile_width * tile_cols;
	dstTileRect.h = tile_height * tile_rows;
}

//init the global SDL_RECTs
void initSDL_Rects()
{
	SDL_Log("Enter initSDL_Rects\n");


	//tile being moved by mouse
	active_tex_rect.x = 0;
	active_tex_rect.y = 0;
	active_tex_rect.w = tile_width;
	active_tex_rect.h = tile_height;

	//current tile being rendered on map
	active_map_tex_rect.x = 0;
	active_map_tex_rect.y = 0;
	active_map_tex_rect.w = tile_width;
	active_map_tex_rect.h = tile_height;

	//draw image and tile grid
    //SDL_RenderCopy(mainRenderer, pngTexture, &srcTileRect, &dstTileRect);

	//draw tile gfx
	//w and h need to be less than width and height of image to prevent warping due to stretching
	srcTileRect.x = 0;
	srcTileRect.y = 0;
	srcTileRect.w = tile_width * tile_cols;
	srcTileRect.h = tile_height * tile_rows;

	//w and h need to be same as srcTileRect to prevent warping of image due to stretching
	dstTileRect.x = 0;
	dstTileRect.y = tile_height * map_rows + tile_height;
	dstTileRect.w = tile_width * tile_cols;
	dstTileRect.h = tile_height * tile_rows;

	dstTileMap.x = 0;
	dstTileMap.y = 0;
	dstTileMap.w = tile_width;
	dstTileMap.h = tile_height;

	dstTileMapPlaced.x = 0;
	dstTileMapPlaced.y = 0;
	dstTileMapPlaced.w = tile_width;
	dstTileMapPlaced.h = tile_height;

	SDL_Log("Exit initSDL_Rects\n");
}
