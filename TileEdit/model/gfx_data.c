#include "gfx_data.h"

const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT =  900;

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

SDL_Window* mainWindow = NULL;
SDL_Renderer* mainRenderer = NULL;
SDL_Texture* pngTexture = NULL;

struct TextData textTexture;
TTF_Font* loadedTTFFont = NULL;

bool holding_tex = false;
bool mousedown = false;
//const int FOLDER_SIZE = 11;
char folder[FOLDER_SIZE] = "tilesets/";
//char file[FILE_SIZE] = "tilesets/wood_tileset.png";

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

bool str_replace (char* str, int size, char src, char dest)
{
	for (int i =0; i < size; ++i)
	{
		if (str[i] == src)
		{
			str[i] = dest;
		}
	}
}

bool readConfigFile(struct Tilemap* tilemap_data)
{
	SDL_Log("Enter readConfigFile");
	FILE* fileh;
	SDL_Log("folder: %s", folder);
	char* configFile = "config/config.dat";
	
    fileh = fopen(configFile, "r");
	if (fileh == NULL) {
        printf("Error opening %s!", configFile);
        return false;
    }
    int items_written = 0;
	
	char fname_sizec[NUM] = "00000";
	char tilewc[NUM] = "00000";
	char tilehc[NUM] = "00000";
	uint8_t fname_size = 0;

	int tempsize = FILE_SIZE;

    fgets(tilemap_data->metadata.filename, tempsize, fileh);
	str_replace(tilemap_data->metadata.filename, tempsize, '\n', '\0');
	//tilemap_data->metadata.filename[fname_size]='\0';
	fname_size = strlen(tilemap_data->metadata.filename);
	SDL_Log("tilemap_data->metadata.filename: %s", tilemap_data->metadata.filename);
    
	char * temp = calloc(FOLDER_SIZE + fname_size + 2, sizeof(char));
	strcpy(temp, folder);
	strcat(temp, tilemap_data->metadata.filename);
	strcpy (tilemap_data->metadata.filename, temp);
	
	tempsize = NUM;
	fgets(tilewc, tempsize, fileh);
	str_replace(tilewc, tempsize, '\n', '\0');

	tilemap_data->metadata.tile.width = atoi(tilewc);
	fgets(tilehc, tempsize, fileh);
	str_replace(tilehc, tempsize, '\n', '\0');
	tilemap_data->metadata.tile.height = atoi(tilehc);
	
	tile_width = tilemap_data->metadata.tile.width;
	tile_height = tilemap_data->metadata.tile.height;
	
	fclose(fileh);

	SDL_Log("End readConfigFile() fname_size: %d, tilemap_data->metadata.filename: %s", fname_size, tilemap_data->metadata.filename);
	SDL_Log("tilewc: %s tilehc: %s", tilewc, tilehc);
	SDL_Log("tilemap_data->metadata.tile.width: %d tilemap_data->metadata.tile.height: %d", tilemap_data->metadata.tile.width, tilemap_data->metadata.tile.height);

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
	tilemap_data->metadata.filename[fname_size] = '\0';
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
	free(tilemap_data->metadata.filename);
	tilemap_data->metadata.filename = NULL;
	free(tilemap_data->tilemap);
	tilemap_data->tilemap = NULL;
	free(tilemap_data->tilemap1);
	tilemap_data->tilemap1 = NULL;	

	//tilemap_data itself does not need to be freed or set to NULL since it is a pointer to a statically allocated struct
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
