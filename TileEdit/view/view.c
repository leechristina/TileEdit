#include "view.h"

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

bool loadMedia(struct Tilemap* tilemap_data)
{
	//fprintf(stdout, "enter loadMedia file: %s tilemap_data->metadata.filename: %s\n", file, tilemap_data->metadata.filename);
	fprintf(stdout, "enter loadMedia tilemap_data->metadata.filename: %s\n", tilemap_data->metadata.filename);
	bool success = true;

	pngTexture = IMG_LoadTexture(mainRenderer, tilemap_data->metadata.filename);
	if( pngTexture == NULL )
	{
		printf( "Failed to load PNG image %s!\n", tilemap_data->metadata.filename );
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