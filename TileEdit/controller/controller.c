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

int onClickTileSet(SDL_Event event, struct Metadata metadata)
{
	printf("clicked mouse in tile area!\n");
	SDL_ShowCursor(SDL_DISABLE);
	int position =  get_position(event, metadata);
	holding_tex = true;

	return position;
}

void onClickTileMap(SDL_Event event, struct Tilemap tilemap_data, bool* mousedown, bool* mouse_pointer, int curr_tilemap, int position)
{
	printf("clicked map area with tile!\n");
	int index = get_map_tile_clicked(event.button.x, event.button.y, tilemap_data.metadata);
	printf("index: %d\n", index);
	//printf("position: %d\n", position);
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
	*mousedown = true;
	*mouse_pointer = false;
}

void onClickCollisionMap(SDL_Event event, struct Tilemap tilemap_data)
{
	int index = get_map_tile_clicked(event.button.x, event.button.y, tilemap_data.metadata);
	SDL_Log("click collision start no tile -- index: %d collisionmap[index]: %d", index, tilemap_data.collisionmap[index]);
	tilemap_data.collisionmap[index] = !tilemap_data.collisionmap[index];
	SDL_Log("index: %d collisionmap[index]: %d", index, tilemap_data.collisionmap[index]);
}

void onPressP(bool* mouse_pointer)
{
	SDL_Log("p pressed!");
	if (!*mouse_pointer)
	{
		SDL_ShowCursor(SDL_ENABLE);
		*mouse_pointer = true;
	}
	else
	{
		SDL_ShowCursor(SDL_DISABLE);
		*mouse_pointer = false;
	}
}

int onPressE(bool showCollision, int curr_tilemap)
{
	SDL_Log("e pressed");
	SDL_Log(" showCollision: %d curr_tilemap = %d\n", showCollision, curr_tilemap);
	if (showCollision)
	{
		//editCollision = true;
		curr_tilemap = COLLISION;
	}	
	else
	{
		curr_tilemap = 0;
	}
	SDL_Log(" showCollision: %d curr_tilemap = %d\n", showCollision, curr_tilemap);

	return curr_tilemap;
}

//double the width by adding blank space to the right of any existing map	
void onPressX(struct Tilemap* tilemap_data)
{
	int m_cols = *tilemap_data->metadata.map_cols;
	int m_rows = *tilemap_data->metadata.map_rows;

	SDL_Log("Increase x");
	//printTileMap(tilemap_data->tilemap);
	//printTileMap(tilemap_data->tilemap1);
	printTileMapBool(tilemap_data->collisionmap);
	//cpy current to temp
	int *tilemap_tmp = calloc(m_cols * m_rows, sizeof(int)); 
	memcpy(tilemap_tmp, tilemap_data->tilemap, sizeof(int) * m_cols * m_rows);
	int *tilemap1_tmp = calloc(m_cols * m_rows, sizeof(int));
	memcpy(tilemap1_tmp, tilemap_data->tilemap1, sizeof(int) * m_cols * m_rows);
	bool *tilemap_collision_tmp = calloc(m_cols * m_rows, sizeof(bool));
	memcpy(tilemap_collision_tmp, tilemap_data->collisionmap, sizeof(bool) * m_cols * m_rows);
    printTileMapBool(tilemap_collision_tmp);

	//record larger column size
	*tilemap_data->metadata.map_cols *= 2;
	tilemap_data->metadata.endx = *tilemap_data->metadata.map_cols + tilemap_data->metadata.startx; 
	//increase size of tilemaps
	tilemap_data->tilemap = calloc((m_cols * 2) * *tilemap_data->metadata.map_rows, sizeof(int)); 
	tilemap_data->tilemap1 = calloc((m_cols * 2) * *tilemap_data->metadata.map_rows, sizeof(int));
	tilemap_data->collisionmap = calloc((m_cols * 2) * *tilemap_data->metadata.map_rows, sizeof(bool));
	init_arr(tilemap_data->tilemap, -1, m_cols * 2 * m_rows); 
	init_arr(tilemap_data->tilemap1, -1, m_cols * 2 * m_rows); 
	init_arr_bool(tilemap_data->collisionmap, false, m_cols * 2 * m_rows); 
	//printTileMap(tilemap_data->tilemap);
	//printTileMap(tilemap_data->tilemap1);
	printTileMapBool(tilemap_data->collisionmap);
	//move saved tilemap data back
	//void blockcpy(void* dest, void* src, int d_rows, int d_cols, int s_rows, int s_cols)
	blockcpy(tilemap_data->tilemap, tilemap_tmp, (int)m_rows, (int)m_cols * 2, (int)m_rows, (int)m_cols);
	blockcpy(tilemap_data->tilemap1, tilemap1_tmp, (int)m_rows, (int)m_cols * 2, (int)m_rows, (int)m_cols);
	blockcpybool(tilemap_data->collisionmap, tilemap_collision_tmp, (int)m_rows, (int)m_cols * 2, (int)m_rows, (int)m_cols);
	
	//printTileMap(tilemap_data->tilemap);
	//printTileMap(tilemap_data->tilemap1);
	printTileMapBool(tilemap_data->collisionmap);
}

int get_position(SDL_Event event, struct Metadata metadata)
{
	int position = get_tile_clicked(event, metadata);
	printf("position: %d\n", position);
	//get active_tex_rect
	get_position_tex(position, metadata);
	printf("active_tex_rect.x: %d\n", active_tex_rect.x);
	printf("active_tex_rect.y: %d\n", active_tex_rect.y);
	printf("active_tex_rect.w: %d\n", active_tex_rect.w);
	printf("active_tex_rect.h: %d\n\n", active_tex_rect.h);

	return position;
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

