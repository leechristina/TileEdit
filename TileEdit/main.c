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
c: show or hide collision map  
e: edit collision map layer when collision map is showing (click tiles to toggle collision on or off)  
x: double width of tilemap  
y: double height of tilemap (not yet implemented)  
left arrow <-: go left 5 tiles (or by tiles left if less than 5)  
right arrow ->: go right 5 tiles (or by tiles left if less than 5)   
up arrow ↑: go up 5 tiles or by tiles left if less than 5 (not yet implemented)  
down arrow ↓: go down 5 tiles or by tiles left if less than 5 (not yet implemented) 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h> //capatibility for older c
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "controller.h"



int main( int argc, char* args[] )
{
	bool showCollision = false;
	// editCollision = false;

	
	map_size = map_rows * map_cols;

	SDL_Log("tiles_start: %d pixels map size: %d pixels", tiles_start, map_size);
    
	struct Tilemap tilemap_data = {
		//.metadata.filename = file,
		.metadata.filename = calloc(300, sizeof(char)),
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
                                	tilemap_data.metadata.startx += 5;
									tilemap_data.metadata.endx += 5;
									SDL_Log("after startx: %d endx: %d", tilemap_data.metadata.startx, tilemap_data.metadata.endx);
									//else if curr start = 0 do nothing
									break;

								case SDLK_RIGHT:
									//TODO: Debug
									SDL_Log("go right");
									SDL_Log("before startx: %d endx: %d", tilemap_data.metadata.startx, tilemap_data.metadata.endx);
									uint32_t end_disp = tilemap_data.metadata.endx;
                                    tilemap_data.metadata.startx -= 5;
									tilemap_data.metadata.endx -= 5;
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


