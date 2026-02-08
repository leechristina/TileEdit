#include "controller.h"

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

//get index of map tile clicked
int get_map_tile_clicked(int mouseX, int mouseY, struct Metadata metadata)
{
    int col = mouseX/metadata.tile.width;
    int row = mouseY/metadata.tile.height;
    int tile_index = row*map_cols+col;

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

bool inTileArea(SDL_Event e, struct Metadata metadata)
{
	return e.button.x >= 0 && e.button.x < metadata.tile.width*tile_cols && e.button.y >= tiles_start 
	&& e.button.y < tiles_start + metadata.tile.height*tile_rows;
}

bool inMapArea(int mouseX, int mouseY, struct Metadata metadata)
{
	return mouseX >= 0 && 
		mouseX < metadata.tile.width*map_cols && 
	    mouseY >= 0 && 
		mouseY < metadata.tile.height*map_rows;
}