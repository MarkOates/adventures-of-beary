                                                         


// bug
//   -- if your in the menus, you can still go in and out of doors
//   -- when there are two platform blocks on top of each other
//      you can not jump onto the bottom one.
//   -- you cannot abort the game with ESC while choosing a level
//   -- frame animations that loop don't show the last frame
//   -- 



#define version_number "Beta v.5-wip"
#define COPYRIGHT_TEXT  "Copyright 2016 - Mark Oates - " version_number

#include <stdlib.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <math.h>


#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>


std::string new_level_name = "central_world_map";


#define SCREEN_W (16*25) /*400 * 3  1200*/
#define SCREEN_H (16*14.0625) /*240 * 3  672*/

#define NEW_DISPLAY_FLAGS 0
//#define NEW_DISPLAY_FLAGS ALLEGRO_FULLSCREEN_WINDOW
#define DISPLAY_ADAPTER 0
//#define NEW_DISPLAY_FLAGS (ALLEGRO_WINDOWED | ALLEGRO_NOFRAME)
//#define SCREEN_SCALE_X (4.8 /*4.8*/)
//#define SCREEN_SCALE_Y (4.8 /*4.8*/)
//#define SCREEN_SCALE_X (2.5 /*4.8*/)
//#define SCREEN_SCALE_Y (2.5 /*4.8*/)
#define SCREEN_SCALE_X (4 /*4.8*/)
#define SCREEN_SCALE_Y (4 /*4.8*/)


ALLEGRO_COLOR foreground_color = al_map_rgba_f(0,0,0,0);
ALLEGRO_COLOR background_color = al_map_rgba_f(0,0,0,1.0);


#define BLACK makecol(0,0,0)
#define WHITE makecol(255,255,255)
#define RED makecol(255,0,0)
#define GREEN makecol(0,255,0)
#define BLUE makecol(0,0,255)
#define YELLOW makecol(255, 255, 0)
#define ORANGE makecol(255, 128, 0)
#define VIOLET makecol(128, 0, 128)
#define DGREEN makecol(0, 128, 0)
#define DBLUE makecol(0, 0, 128)
#define HAPPY_YELLOW makecol(255,229,0)
//#define COLOR_BACKGROUND makecol(212,242,250)
#define COLOR_BACKGROUND makecol(0, 0, 16)



#include "helpers.h"



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
///  GAME  ///////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


float camera_x;
float camera_y;


#define POWER_UP_BOUNDING_BOX_SIZE  24

ALLEGRO_FONT *font_med;
ALLEGRO_FONT *font_small;
ALLEGRO_FONT *font_x_small;


float GAMEPLAY_FRAME_DURATION_DEFAULT = (1.0/60.0);
float GAMEPLAY_FRAME_DURATION = GAMEPLAY_FRAME_DURATION_DEFAULT;
#define GAMEPLAY_FRAME_DURATION_PER_SEC (GAMEPLAY_FRAME_DURATION * 60.0)



int mouse_x = 0;
int mouse_y = 0;
bool mouse_b1 = false;
bool mouse_b2 = false;


ALLEGRO_BITMAP *projection_subbitmap = NULL;


ALLEGRO_FONT *font = NULL;
ALLEGRO_FONT *game_font = NULL;

void poll_mouse()
{
	ALLEGRO_MOUSE_STATE mouse_state;
	al_get_mouse_state(&mouse_state);
	mouse_x = mouse_state.x;
	mouse_y = mouse_state.y;
	mouse_b1 = al_mouse_button_down(&mouse_state, 1);
	mouse_b2 = al_mouse_button_down(&mouse_state, 2);
}



// this fancy function will parse an interger value out of a string.
// if the string starts with a 't' then the value is a tile coordinate,
// so the value is multiplied by TILE_WIDTH to parse the value into
// world coordinates.
#define TILE_WIDTH 16
int atoi_t(const char *ch)
{
	if (strlen(ch) == 0) return 0;
	if (ch[0] != 't') return atoi(ch);
	std::string str(ch);
	return atoi(str.substr(1).c_str()) * TILE_WIDTH;
}

// an interesting technique.  These are in screen-widths ('c' for column) and screen-heights ('r' for row)
// 'screens' are 25 x 14 tiles each
float atof_rct(const char *ch)
{
	if (strlen(ch) == 0) return 0;
	std::string str(ch);
	if (ch[0] == 'r') return atof(str.substr(1).c_str()) * TILE_WIDTH * 14 + TILE_WIDTH / 2;
	if (ch[0] == 'c') return atof(str.substr(1).c_str()) * TILE_WIDTH * 25 + TILE_WIDTH / 2;
	if (ch[0] == 't') return atof(str.substr(1).c_str()) * TILE_WIDTH;
	return atoi(ch);
}


float atof_t(const char *ch)
{
	if (strlen(ch) == 0) return 0;
	if (ch[0] != 't') return atof(ch);
	std::string str(ch);
	return atof(str.substr(1).c_str()) * TILE_WIDTH;
}


std::vector<std::string> explode(const std::string &delimiter, const std::string &str);

ALLEGRO_COLOR atocol(const char *ch)
{
	ALLEGRO_COLOR result;
	std::string str = ch;
	std::vector<std::string> parts = explode(" ", str);
	if (parts.size() != 4)
	{
		std::cout << "error parsing color from string \"" << ch << "\"." << std::endl;
		return al_map_rgba_f(0, 0, 0, 0);
	}
	result.r = atof(parts[0].c_str());
	result.g = atof(parts[1].c_str());
	result.b = atof(parts[2].c_str());
	result.a = atof(parts[3].c_str());
	return result;
}

std::string color_to_string(ALLEGRO_COLOR col)
{
	std::stringstream ss;
	ss << col.r << " " << col.g << " " << col.b << " " << col.a;
	return ss.str();
}

void play_attack_sound();




#define ROCK_LIST \
ADD_ROCK(iso_rock, 1)\
ADD_ROCK(iso_rock, 2)\
ADD_ROCK(iso_rock, 3)\
ADD_ROCK(iso_rock, 4)\
ADD_ROCK(iso_rock, 5)\
ADD_ROCK(iso_rock, 6)\
ADD_ROCK(iso_rock, 7)\
ADD_ROCK(iso_rock, 8)\
ADD_ROCK(iso_rock, 9)\
ADD_ROCK(iso_rock, 10)



#define JEWEL_LIST \
JEWEL_TYPE(small_jewel)\
JEWEL_TYPE(medium_jewel)\
JEWEL_TYPE(large_jewel)\
JEWEL_TYPE(diamond)



#define BEAR_LIST \
ADD_BEAR(yellow, YELLOW, 1)\
ADD_BEAR(brown, BROWN, 2)\
ADD_BEAR(blue, BLUE, 3)\
ADD_BEAR(purple, PURPLE, 4)





using namespace std;

bool hide_hud = false;
//bool F_show_congrats_banner = false;

#define MIN_NUMBER_OF_KILLS_FOR_A_CHAIN 2
#define MIN_NUMBER_OF_SIMULTANIOUS_KILLS 2


///// program flags
bool F_disable_edits=false;

bool F_draw_map_layer_1=true;
bool F_draw_map_layer_2=true;
bool F_draw_map_layer_3=true;
bool F_draw_boundarys=false;
bool F_debug_mode=false;
bool F_map_maker_mode=false;
bool F_draw_backgrounds=true;

bool no_fades_no_rests = true;
void do_logic(void);


ALLEGRO_FONT *new_font = NULL;


ALLEGRO_BITMAP *stat_life_meter_heart_full = NULL;
ALLEGRO_BITMAP *stat_life_meter_heart_empty = NULL;
ALLEGRO_BITMAP *stat_life_meter_heart_half = NULL;

ALLEGRO_BITMAP *life_icon = NULL;



ALLEGRO_BITMAP *choose_a_level = NULL;

ALLEGRO_BITMAP *holding_gun = NULL;


ALLEGRO_BITMAP *paused_banner = NULL;

ALLEGRO_BITMAP *bear1 = NULL;
ALLEGRO_BITMAP *bear2 = NULL;
ALLEGRO_BITMAP *bear3 = NULL;
ALLEGRO_BITMAP *bear4 = NULL;
ALLEGRO_BITMAP *bear_duck = NULL;
ALLEGRO_BITMAP *bear = NULL;
ALLEGRO_BITMAP *bear_climb1 = NULL;
ALLEGRO_BITMAP *bear_climb2 = NULL;
ALLEGRO_BITMAP *bear_in_air = NULL;
ALLEGRO_BITMAP *bear_downthrusting = NULL;

ALLEGRO_BITMAP *bad_bear_pic1 = NULL;
ALLEGRO_BITMAP *bad_bear_pic2 = NULL;
ALLEGRO_BITMAP *bad_bear2_pic1 = NULL;
ALLEGRO_BITMAP *bad_bear2_pic2 = NULL;
ALLEGRO_BITMAP *blue_bear_pic1 = NULL;
ALLEGRO_BITMAP *blue_bear_pic2 = NULL;
ALLEGRO_BITMAP *blue_bear_jump_pic = NULL;
ALLEGRO_BITMAP *purple_bear_pic1 = NULL;
ALLEGRO_BITMAP *purple_bear_pic2 = NULL;
ALLEGRO_BITMAP *medusa_head_bmp = NULL;

//ALLEGRO_SAMPLE *one_up_sample = NULL;
ALLEGRO_SAMPLE *pickup_key_sample = NULL;
ALLEGRO_SAMPLE *use_key_sample = NULL;
ALLEGRO_SAMPLE *pickup_coin_sample = NULL;
ALLEGRO_SAMPLE *door_locked_sample = NULL;
ALLEGRO_SAMPLE *door_open_sample = NULL;
ALLEGRO_SAMPLE *door_close_sample = NULL;

ALLEGRO_BITMAP *sparkle_particle_0 = NULL;
ALLEGRO_BITMAP *sparkle_particle_1 = NULL;
ALLEGRO_BITMAP *sparkle_particle_2 = NULL;
ALLEGRO_BITMAP *sparkle_particle_3 = NULL;

ALLEGRO_SAMPLE *ball_bounce_sample = NULL;

ALLEGRO_BITMAP *checkpoint = NULL;

#define ADD_ROCK(x, y) ALLEGRO_SAMPLE* x##y##_sample;
//#include "rock_list.hpp"
	ROCK_LIST
#undef ADD_ROCK





ALLEGRO_BITMAP *trigger_on = NULL;
ALLEGRO_BITMAP *trigger_off = NULL;


ALLEGRO_SAMPLE *cha_ching = NULL;
ALLEGRO_SAMPLE *jump_sample = NULL;

ALLEGRO_BITMAP *title_graphic = NULL;


ALLEGRO_SAMPLE *sword_strike = NULL;

ALLEGRO_SAMPLE *menu_move = NULL;
ALLEGRO_SAMPLE *menu_select = NULL;
ALLEGRO_SAMPLE *pause_sample = NULL;
ALLEGRO_SAMPLE *secret_found_sample = NULL;
ALLEGRO_SAMPLE *unpause_sample = NULL;

ALLEGRO_SAMPLE *power_up_sample_heart = NULL;


bool screen_is_black = false;



char *arg_string;
char buf[256];
char *ptrbuf;

bool abort_game=false;

bool keyboard_available=false;
bool mouse_available=false;
bool joystick_available=false;

ALLEGRO_BITMAP *buffer = NULL;
ALLEGRO_BITMAP *page1 = NULL;
ALLEGRO_BITMAP *page2 = NULL;
ALLEGRO_BITMAP *empty = NULL;


ALLEGRO_BITMAP *bullet_img = NULL;

int key_for_button_a = ALLEGRO_KEY_A;
int key_for_button_b = ALLEGRO_KEY_SPACE;
int key_for_button_x = ALLEGRO_KEY_X;
int key_for_button_y = ALLEGRO_KEY_LCTRL;
int key_for_button_up = ALLEGRO_KEY_UP;
int key_for_button_down = ALLEGRO_KEY_DOWN;
int key_for_button_left = ALLEGRO_KEY_LEFT;
int key_for_button_right = ALLEGRO_KEY_RIGHT;

bool button_a_release = false;
bool button_b_release = false;
bool button_x_release = false;
bool button_y_release = false;

bool release_a = false;
bool release_b = false;
bool release_x = false;
bool release_y = false;



enum generic_controller_buttons
{
	BUTTON_Y, // use weapon
	BUTTON_B, // jump
	BUTTON_X, // use (or activate) special item
	BUTTON_A, // run (optional usage, could double-tap right or left)

	BUTTON_START, // pause (items_menu, map, restart)

	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
};



#define PI 3.14159265f

float get_degrees(float radians)
{
   return (radians * (180 / PI));
}

float get_radians(float degrees)
{
   return (degrees * (PI / 180));
}




float distance(float x1, float y1, float x2, float y2)
{
      return sqrt( ((x1 - x2) * (x1 - x2)) + ((y1 - y2) * (y1 - y2)) ) ;
}




/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//////// Tile Map ///////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////



#define BEARY_TILE_LAYER_BACKGROUND -1
#define BEARY_TILE_LAYER_0 0
#define BEARY_TILE_LAYER_1 1
#define BEARY_TILE_LAYER_2 2
#define BEARY_TILE_LAYER_BOUNDARY 3


class TileMapBase
{
public:
	virtual int get_tile(int tile_x, int tile_y, int tile_layer) = 0;
	virtual bool set_tile(int tile_x, int tile_y, int tile_layer, int tile) = 0;
	virtual bool draw_layer(int tile_layer, float camera_x, float camera_y) = 0;
};



/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//////// Tile Map ///////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////



class TileIndexInfo
{
public:
	int index_num;

	ALLEGRO_BITMAP *bitmap_source;
	ALLEGRO_BITMAP *sub_bitmap;

	int u1;
	int v1;
	int u2;
	int v2;

	TileIndexInfo()
		: index_num(-1)
		, bitmap_source(NULL)
		, sub_bitmap(NULL)
		, u1(0)
		, v1(0)
		, u2(0)
		, v2(0)
	{}
};






ALLEGRO_BITMAP *build_tile_atlas(int tile_w, int tile_h, std::vector<TileIndexInfo> &tile_index)
	// this function is the superior tile atlas generator
{
	//al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR | ALLEGRO_MIPMAP);

	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	ALLEGRO_BITMAP *target = al_create_bitmap(1024, 1024);
	al_set_target_bitmap(target);

	//al_clear_to_color(color::transparent);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));


	int step_x = tile_w + 2;
	int step_y = tile_h + 2;

	int num_rows = al_get_bitmap_height(target) / step_x;
	int num_cols = al_get_bitmap_width(target) / step_y;


	// step 1:
	// draw each tile onto the new tile_atlas bitmap, each with an extra padding of 1px

	for (int y=0; y<num_rows; y++)
		for (int x=0; x<num_cols; x++)
		{
			int index_num = x + y*num_cols;
			if (index_num >= (int)tile_index.size()) goto tile_drawing_finished;

			int draw_x = x*step_x + 1;
			int draw_y = y*step_y + 1;

			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x, draw_y, 0);
		}

	tile_drawing_finished:


	// step 2:
	// "extrude" the pixels on the edges of each tile

	for (int y=0; y<num_rows; y++)
		for (int x=0; x<num_cols; x++)
		{
			int index_num = x + y*num_cols;
			if (index_num >= (int)tile_index.size()) goto edge_extruding_finished;

			int draw_x = x*step_x + 1;
			int draw_y = y*step_y + 1;


			// draw the top row of pixels
			al_set_clipping_rectangle(draw_x, draw_y-1, tile_w, 1);
			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x, draw_y-1, 0);

			// draw the bottom row of pixels
			al_set_clipping_rectangle(draw_x, draw_y+tile_h, tile_w, 1);
			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x, draw_y+1, 0);

			// draw the left row of pixels
			al_set_clipping_rectangle(draw_x-1, draw_y, 1, tile_h);
			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x-1, draw_y, 0);

			// draw the right row of pixels
			al_set_clipping_rectangle(draw_x+tile_w, draw_y, 1, tile_h);
			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x+1, draw_y, 0);

		
			// draw just the top-left pixel
			al_set_clipping_rectangle(draw_x-1, draw_y-1, 1, 1);
			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x-1, draw_y-1, 0);
		
			// draw just the bottom-left pixel
			al_set_clipping_rectangle(draw_x-1, draw_y+tile_h, 1, 1);
			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x-1, draw_y+1, 0);
		
			// draw just the top-right pixel
			al_set_clipping_rectangle(draw_x+tile_w, draw_y-1, 1, 1);
			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x+1, draw_y-1, 0);
		
			// draw just the top-right pixel
			al_set_clipping_rectangle(draw_x+tile_w, draw_y+tile_h, 1, 1);
			al_draw_bitmap(tile_index[index_num].sub_bitmap, draw_x+1, draw_y+1, 0);
	}

	edge_extruding_finished:

	al_set_clipping_rectangle(0, 0, al_get_bitmap_width(target), al_get_bitmap_height(target));


	//
	// wrap it all up

	al_restore_state(&prev);


	// 
	// to solve an odd bug, the bitmap will be cloned so that OPENGL will correctly create the mipmaps
	ALLEGRO_BITMAP *cloned = al_clone_bitmap(target);
	al_destroy_bitmap(target);
	target = cloned;


	return target;
}




class TileAtlas
{
public:
	std::vector<TileIndexInfo> tile_index;
	ALLEGRO_BITMAP *bitmap;

	void clear()
	{
		for (unsigned i=0; i<tile_index.size(); i++) al_destroy_bitmap(tile_index[i].sub_bitmap);
		if (bitmap) al_destroy_bitmap(bitmap);
		bitmap = NULL;
		tile_index.clear();
	}

	// load will *copy* the bitmap that you pass into it.  You probably would want to al_destroy_bitmap after loading; 
	void load(ALLEGRO_BITMAP *tileset, int tile_width, int tile_height, int spacing=0)
	{
		if (!tileset)
		{
			std::cout << "[TileAtlas::load()] ERROR: the ALLEGRO_BITMAP provided is NULL" << std::endl;
		}

		// clear the existing contents of this tile atlas (if any)
		clear();

		bitmap = al_clone_bitmap(tileset);

		int tile_step_x = tile_width + spacing;
		int tile_step_y = tile_height + spacing;

		int num_columns = al_get_bitmap_width(bitmap) / tile_step_x;
		int num_rows = al_get_bitmap_height(bitmap) / tile_step_y;
		tile_index.resize(num_columns * num_rows);

		for (unsigned index_num=0; index_num<tile_index.size(); index_num++)
		{
			int x1 = (index_num % num_columns) * tile_step_x + spacing;
			int y1 = (index_num / num_columns) * tile_step_y + spacing;
			int x2 = x1 + tile_width;
			int y2 = y1 + tile_height;

			tile_index[index_num].index_num = index_num;
			tile_index[index_num].bitmap_source = NULL;
			tile_index[index_num].sub_bitmap = al_create_sub_bitmap(bitmap, x1, y1, x2-x1, y2-y1);
			tile_index[index_num].u1 = x1;
			tile_index[index_num].v1 = y1;
			tile_index[index_num].u2 = x2;
			tile_index[index_num].v2 = y2;
		}
	}

	void draw_tile_to_atlas(ALLEGRO_BITMAP *tile, int tile_num, ALLEGRO_COLOR color=al_map_rgba_f(1, 1, 1, 1))
	{
		// ok, this is a serious bit of a hack.  It will only work when the tiles are spaced with spaces of 1 px.
		if (!tile) return;

				std::cout << " - Drawing to tile " << tile_num << " - " << std::endl;

			int tile_width = 16;
			int tile_height = 16;
			int spacing = 1;

			if (!bitmap)
			{
				std::cout << "[TileAtlas::draw_to_tile_atlas()] ERROR: the ALLEGRO_BITMAP provided is NULL" << std::endl;
				return;
			}

			int tile_step_x = tile_width + spacing;
			int tile_step_y = tile_height + spacing;

			int num_columns = al_get_bitmap_width(bitmap) / tile_step_x;
			int num_rows = al_get_bitmap_height(bitmap) / tile_step_y;

			for (int index_num=0; index_num<(int)tile_index.size(); index_num++)
			{
				if (index_num != tile_num) continue;

				int x1 = (index_num % num_columns) * tile_step_x + spacing;
				int y1 = (index_num / num_columns) * tile_step_y + spacing;
				int x2 = x1 + tile_width;
				int y2 = y1 + tile_height;

					ALLEGRO_STATE previous_state;
					al_store_state(&previous_state, ALLEGRO_STATE_TARGET_BITMAP);
					al_set_target_bitmap(tile_index[index_num].sub_bitmap);

					al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
					al_draw_tinted_bitmap(tile, color, 0, 0, 0);

					al_restore_state(&previous_state);
/*
				tile_index[index_num].index_num = index_num;
				tile_index[index_num].bitmap_source = NULL;
				tile_index[index_num].sub_bitmap = al_create_sub_bitmap(bitmap, x1, y1, x2-x1, y2-y1);
				tile_index[index_num].u1 = x1;
				tile_index[index_num].v1 = y1;
				tile_index[index_num].u2 = x2;
				tile_index[index_num].v2 = y2;
*/
			}
	}

	bool get_tile_uv(int index_num, int *u1, int *v1, int *u2, int *v2)
	{
		if (index_num < 0 || index_num >= (int)tile_index.size()) return false;

		*u1 = tile_index[index_num].u1;
		*v1 = tile_index[index_num].v1;
		*u2 = tile_index[index_num].u2;
		*v2 = tile_index[index_num].v2;

		return true;
	}
};


TileAtlas *new_tile_atlas = NULL;


/*
typedef struct
{
	float x, y, z;
	float texture_u, texture_v;
	float normal_u, normal_v;
	float height_u, height_v;
	float height;
	ALLEGRO_COLOR color;
} ALLEGRO_TILE_VERTEX;




ALLEGRO_VERTEX_ELEMENT elems[] = {
	{ALLEGRO_PRIM_POSITION, ALLEGRO_PRIM_FLOAT_3, offsetof(ALLEGRO_VERTEX_WITH_NORMAL, x)},
	{ALLEGRO_PRIM_TEX_COORD, ALLEGRO_PRIM_FLOAT_2, offsetof(ALLEGRO_VERTEX_WITH_NORMAL, texture_u)},
	{ALLEGRO_PRIM_USER_ATTR, ALLEGRO_PRIM_FLOAT_2, offsetof(ALLEGRO_VERTEX_WITH_NORMAL, normal_u)},
	{ALLEGRO_PRIM_USER_ATTR, ALLEGRO_PRIM_FLOAT_2, offsetof(ALLEGRO_VERTEX_WITH_NORMAL, height_u)},
	{ALLEGRO_PRIM_COLOR_ATTR, 0, offsetof(ALLEGRO_VERTEX_WITH_NORMAL, color)},
	{ALLEGRO_PRIM_USER_ATTR, ALLEGRO_PRIM_FLOAT_3, offsetof(ALLEGRO_VERTEX_WITH_NORMAL, nx)},
	{0, 0, 0}
};

ALLEGRO_VERTEX_DECLARATION *vertex_declaration = al_create_vertex_decl(elems, sizeof(ALLEGRO_VERTEX_WITH_NORMAL));
*/



class TileMap
{
private:
	void set_tile_uv(int tile_x, int tile_y, int u1, int v1, int u2, int v2)
	{
		int index_start = (tile_x * 6) + tile_y * (width*6);
		int &i = index_start;

		//ALLEGRO_VERTEX *vbuff = (ALLEGRO_VERTEX *)al_lock_vertex_buffer(vertex_buffer, index_start, 6, ALLEGRO_LOCK_WRITEONLY);
		ALLEGRO_VERTEX *vbuff = (ALLEGRO_VERTEX *)al_lock_vertex_buffer(vertex_buffer, index_start, 6, ALLEGRO_LOCK_WRITEONLY);
		if (!vbuff) std::cout << "could not lock vertex buffer" << std::endl;

		if (use_vtx) vtx[i+0].u = u1;
		if (use_vtx) vtx[i+0].v = v1;
		vbuff[0].u = u1;
		vbuff[0].v = v1;

		if (use_vtx) vtx[i+1].u = u1;
		if (use_vtx) vtx[i+1].v = v2;
		vbuff[1].u = u1;
		vbuff[1].v = v2;

		if (use_vtx) vtx[i+2].u = u2;
		if (use_vtx) vtx[i+2].v = v2;
		vbuff[2].u = u2;
		vbuff[2].v = v2;

		if (use_vtx) vtx[i+3].u = u2;
		if (use_vtx) vtx[i+3].v = v2;
		vbuff[3].u = u2;
		vbuff[3].v = v2;

		if (use_vtx) vtx[i+4].u = u2;
		if (use_vtx) vtx[i+4].v = v1;
		vbuff[4].u = u2;
		vbuff[4].v = v1;

		if (use_vtx) vtx[i+5].u = u1;
		if (use_vtx) vtx[i+5].v = v1;
		vbuff[5].u = u1;
		vbuff[5].v = v1;

		al_unlock_vertex_buffer(vertex_buffer);
	}

	ALLEGRO_VERTEX_BUFFER *vertex_buffer;
	std::vector<ALLEGRO_VERTEX> vtx;
	TileAtlas *tile_atlas;
	int width;
	int height;
	std::vector<int> tiles;

	bool use_vtx;

public:

	TileMap()
		: vertex_buffer(NULL)
		, vtx()
		, tile_atlas(NULL)
		, width(0)
		, height(0)
		, tiles()
		, use_vtx(false) // setting this to false will skip the creation of the ALLEGRO_VERTEX mesh
	{}

	void use_tile_atlas(TileAtlas *atlas)
	{
		if (tile_atlas != NULL) std::cout << "Warning: swapping new atlas; potentially unsafe operation" << std::endl;
		tile_atlas = atlas;
	}

	void resize(int w, int h, int tile_w, int tile_h)
	{
		// set the width and height of our map
		width = w;
		height = h;

		// resize and clear the tiles
		tiles.assign(width * height, 0);

		// resize the vtx vector
		vtx.clear();
		if (use_vtx) vtx.resize(width*height*6);

		// create a vertex_buffer
		if (vertex_buffer) al_destroy_vertex_buffer(vertex_buffer);
		vertex_buffer = al_create_vertex_buffer(NULL, NULL, width*height*6, ALLEGRO_PRIM_BUFFER_STATIC);
		if (!vertex_buffer) std::cout << "There was an error creating the vertex buffer" << std::endl;

		// lock the buffer before writing to it
		ALLEGRO_VERTEX *vbuff_begin = (ALLEGRO_VERTEX *)al_lock_vertex_buffer(vertex_buffer, 0, al_get_vertex_buffer_size(vertex_buffer), ALLEGRO_LOCK_WRITEONLY);
		if (!vbuff_begin) std::cout << "There was an error locking the vertex buffer" << std::endl;

		// place the vertexes in the mesh
		ALLEGRO_VERTEX *vbuff = vbuff_begin;
		int v = 0;
		int num_vertexes = width*height*6;
		for (; v<num_vertexes; v+=6, vbuff+=6)
		{
			long tile_num = v / 6;

			int x1 = (tile_num % width);
			int y1 = (tile_num / width);
			int x2 = x1 + 1;
			int y2 = y1 + 1;

			if (use_vtx) vtx[v+0].x = x1;
			if (use_vtx) vtx[v+0].y = y1;
			vbuff[0].x = x1;
			vbuff[0].y = y1;

			if (use_vtx) vtx[v+1].x = x1;
			if (use_vtx) vtx[v+1].y = y2;
			vbuff[1].x = x1;
			vbuff[1].y = y2;

			if (use_vtx) vtx[v+2].x = x2;
			if (use_vtx) vtx[v+2].y = y2;
			vbuff[2].x = x2;
			vbuff[2].y = y2;

			if (use_vtx) vtx[v+3].x = x2;
			if (use_vtx) vtx[v+3].y = y2;
			vbuff[3].x = x2;
			vbuff[3].y = y2;

			if (use_vtx) vtx[v+4].x = x2;
			if (use_vtx) vtx[v+4].y = y1;
			vbuff[4].x = x2;
			vbuff[4].y = y1;

			if (use_vtx) vtx[v+5].x = x1;
			if (use_vtx) vtx[v+5].y = y1;
			vbuff[5].x = x1;
			vbuff[5].y = y1;
		}

		// "scale" the vertexes to the tile_w and tile_h
		// and set other default values
		vbuff = vbuff_begin;
		v = 0;
		for (; v<num_vertexes; v++, vbuff++)
		{
			if (use_vtx) vtx[v].x *= tile_w;
			if (use_vtx) vtx[v].y *= tile_h;
			if (use_vtx) vtx[v].z = 0;
			if (use_vtx) vtx[v].color = al_map_rgba_f(1, 1, 1, 1);//color::mix(color::white, random_color(), 0.5);//color::transparent;
			vbuff[0].x *= tile_w;
			vbuff[0].y *= tile_h;
			vbuff[0].z = 0;
			vbuff[0].color = al_map_rgba_f(1, 1, 1, 1);//color::mix(color::white, random_color(), 0.5);
		}

		// unlock our buffer
		al_unlock_vertex_buffer(vertex_buffer);
	}

	int get_num_tiles()
	{
		return width * height;
	}

	bool set_contiguous_tile_num(int contiguous_tile_num, int tile_index)
	{
		int tile_x = contiguous_tile_num % width;
		int tile_y = contiguous_tile_num / width;
		return set_tile(tile_x, tile_y, tile_index);
	}

	int get_tile(int tile_x, int tile_y)
	{
		if (tile_x < 0 || (tile_x >= width)) return -1;
		if (tile_y < 0 || (tile_y >= height)) return -1;

		return tiles[tile_x % width + tile_y * width];
	}

	bool set_tile(int tile_x, int tile_y, int tile_index)
		// if the tile is set to a negative number, then the tiles[tile_index] will be set to that number, but
		// the image will be the bitmap at index 0
	{
		if (tile_x < 0 || (tile_x >= width)) return false;
		if (tile_y < 0 || (tile_y >= height)) return false;

		tiles[tile_x + tile_y * width] = tile_index;

		if (!tile_atlas) return false;
		if (tile_index >= (int)tile_atlas->tile_index.size()) return false;


		// texture the appropriate vetexes in the mesh

		if (tile_index < 0) tile_index = 0; // IF the tile is < 0, the graphic will be set to 0

		int u1, v1, u2, v2;
		tile_atlas->get_tile_uv(tile_index, &u1, &v1, &u2, &v2);

		set_tile_uv(tile_x, tile_y, u1, v1, u2, v2);

		return true;
	}

	void draw(int camera_x, int camera_y)
	{
		ALLEGRO_TRANSFORM prev, transform;
		al_copy_transform(&prev, al_get_current_transform());
		al_identity_transform(&transform);
		al_translate_transform(&transform, -camera_x, -camera_y);
		al_use_transform(&transform);

		//al_draw_prim(&vtx[0], NULL, tile_atlas->bitmap, 0, vtx.size(), ALLEGRO_PRIM_TRIANGLE_LIST);
		al_draw_vertex_buffer(vertex_buffer, tile_atlas->bitmap, 0, al_get_vertex_buffer_size(vertex_buffer), ALLEGRO_PRIM_TRIANGLE_LIST);

		al_use_transform(&prev);
	}

	void random_fill()
	{
		for (unsigned i=0; i<(width*height); i++)
			set_contiguous_tile_num(i, random_int(0, (int)tile_atlas->tile_index.size()));
	}

	int get_width()
	{
		return width;
	}

	int get_height()
	{
		return height;
	}
};




/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//////// DOORS //////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////



#define DOOR_TYPE_NORMAL           1
#define DOOR_TYPE_LOCKED           2
#define DOOR_TYPE_UNLOCKED         3
#define DOOR_TYPE_LOCKED_SILVER    4
#define DOOR_TYPE_UNLOCKED_SILVER  5
#define DOOR_TYPE_LOCKED_SECRET    6
#define DOOR_TYPE_UNLOCKED_SECRET  7
#define DOOR_TYPE_EXIT_ONLY        8
#define DOOR_TYPE_WALKOFF			  9	



ALLEGRO_BITMAP *door_normal;
ALLEGRO_BITMAP *door_locked;
ALLEGRO_BITMAP *door_unlocked;
ALLEGRO_BITMAP *door_locked_silver;
ALLEGRO_BITMAP *door_unlocked_silver;
ALLEGRO_BITMAP *door_locked_secret;
ALLEGRO_BITMAP *door_unlocked_secret;
ALLEGRO_BITMAP *door_exit_only;

void load_locked_door_graphics()
{
    door_normal = _load_bitmap("images/doors/door_normal.gif");
    door_locked = _load_bitmap("images/doors/door_locked.gif");
    door_unlocked = _load_bitmap("images/doors/door_unlocked.gif");
    door_locked_silver = _load_bitmap("images/doors/door_locked_silver.gif");
    door_unlocked_silver = _load_bitmap("images/doors/door_unlocked_silver.gif");
    door_locked_secret = _load_bitmap("images/doors/door_locked_secret.gif");
    door_unlocked_secret = _load_bitmap("images/doors/door_unlocked_secret.gif");
    door_exit_only = _load_bitmap("images/doors/door_exit_only.gif");
}

void destroy_locked_door_graphics()
{
    al_destroy_bitmap(door_normal);
    al_destroy_bitmap(door_locked);
    al_destroy_bitmap(door_unlocked);
    al_destroy_bitmap(door_locked_silver);
    al_destroy_bitmap(door_unlocked_silver);
    al_destroy_bitmap(door_locked_secret);
    al_destroy_bitmap(door_unlocked_secret);
    al_destroy_bitmap(door_exit_only);
}




class new_door_struct
{
public:
	static int last_unique_id;
	int unique_id; // this is used internally and should never be modified

	// placement
	int world_num;
	int level_num;
	int map_num;
	int x, y;

	// type and label
	std::string label;
	int type;

	// destination:
	int destination_world_num;
	std::string destination_label;
};

int new_door_struct::last_unique_id = 0;



#define VAR_DUMP(parent_class, identifier) std::cout << "" #identifier " = " << parent_class . identifier << std::endl;
void inspect(const new_door_struct &door)
{
	std::cout << "=== new_door_struct ===" << std::endl;
	VAR_DUMP(door, unique_id);
	VAR_DUMP(door, world_num);
	VAR_DUMP(door, level_num);
	VAR_DUMP(door, map_num);
	VAR_DUMP(door, x);
	VAR_DUMP(door, y);
	VAR_DUMP(door, label);
	VAR_DUMP(door, type);
	VAR_DUMP(door, destination_world_num);
	VAR_DUMP(door, destination_label);
}


class Doors
{
private:
	ALLEGRO_BITMAP *crazy_ass_door;
	std::vector<new_door_struct> doors;
	static Doors *instance;
	Doors()
		: doors()
		, crazy_ass_door(al_load_bitmap("images/doors/crazy_ass_door_low2.png"))
	{}
	static Doors *get_instance()
	{
		if (!instance) instance = new Doors();
		return instance;
	}
	static void _clear()
	{
		get_instance()->doors.clear();
		new_door_struct::last_unique_id = 1;
		al_destroy_bitmap(get_instance()->crazy_ass_door);
		get_instance()->crazy_ass_door = al_load_bitmap("images/doors/crazy_ass_door_low2.png");
	}

public:
	static int get_door_type_from_identifier(const char *identifier)
	{
		if (strcmp(identifier, "locked silver") == 0) return DOOR_TYPE_LOCKED_SILVER;
		if (strcmp(identifier, "unlocked silver") == 0) return DOOR_TYPE_UNLOCKED_SILVER;
		if (strcmp(identifier, "locked secret") == 0) return DOOR_TYPE_LOCKED_SECRET;
		if (strcmp(identifier, "unlocked secret") == 0) return DOOR_TYPE_UNLOCKED_SECRET;
		if (strcmp(identifier, "exit only") == 0) return DOOR_TYPE_EXIT_ONLY;
		if (strcmp(identifier, "normal") == 0) return DOOR_TYPE_NORMAL;
		if (strcmp(identifier, "locked") == 0) return DOOR_TYPE_LOCKED;
		if (strcmp(identifier, "unlocked") == 0) return DOOR_TYPE_UNLOCKED;
		if (strcmp(identifier, "walkoff") == 0) return DOOR_TYPE_WALKOFF;

		return 0;
	}
	static ALLEGRO_BITMAP *get_door_bitmap(int type)
	{
		switch(type)
		{
			case DOOR_TYPE_NORMAL: return door_normal; break;
			case DOOR_TYPE_LOCKED: return door_locked; break;
			case DOOR_TYPE_UNLOCKED: return door_unlocked; break;
			case DOOR_TYPE_LOCKED_SILVER: return door_locked_silver; break;
			case DOOR_TYPE_UNLOCKED_SILVER: return door_unlocked_silver; break;
			case DOOR_TYPE_LOCKED_SECRET: return door_locked_secret; break;
			case DOOR_TYPE_UNLOCKED_SECRET: return door_unlocked_secret; break;
			case DOOR_TYPE_EXIT_ONLY: return door_exit_only; break;
			case DOOR_TYPE_WALKOFF: return door_normal; break;
		}

		return NULL;
	}
	static bool get_destination_door(const new_door_struct &door, new_door_struct *destination_door)
	{
		std::vector<new_door_struct> &doors = get_instance()->doors;

		inspect(door);

		int world_num = door.destination_world_num;
		std::string label = door.destination_label;
		int unique_id = door.unique_id;

		for (unsigned i=0; i<doors.size(); i++)
		{
			if (doors[i].world_num != world_num) continue;
			if (doors[i].unique_id == unique_id) continue; // it shouldn't be the same door
			if (doors[i].label != label) continue;

			*destination_door = doors[i];
			return true;
		}

		return false;
	}
	static void clear()
	{
		_clear();
	}
	static void load(std::string filename)
	{
		_clear();
		append_doors_file(filename);
	}
	static void add_door(int world_num, int level_num, int map_num, float x, float y, std::string label, int type, int destination_world_num, std::string destination_label)
	{
		new_door_struct new_door;

		new_door.unique_id = new_door_struct::last_unique_id++;
		new_door.world_num = world_num;
		new_door.level_num = level_num;
		new_door.map_num = map_num;
		new_door.x = x;
		new_door.y = y;
		new_door.label = label;
		new_door.type = type;
		new_door.destination_world_num = destination_world_num;
		new_door.destination_label = destination_label;

		get_instance()->doors.push_back(new_door);
	}
	static bool append_doors_file(std::string filename) // don't use this unless you know what you're doing
	{
		std::vector<new_door_struct> &doors = get_instance()->doors;

		ALLEGRO_CONFIG *config = al_load_config_file(filename.c_str());
		if (!config)
		{
			std::cout << "Could not load doors file \"" << filename << "\"" << std::endl;
			return false;
		}

		ALLEGRO_CONFIG_SECTION *section;
		const char *current_section = al_get_first_config_section(config, &section);

		if (current_section == NULL)
		{
			std::cout << "it appears that the .doors file \"" << filename << "\" is empty; skipping loading." << std::endl;
			return true;
		}

		do
		{
			if (strcmp(current_section, "") == 0)
			{
				continue;
				std::cout << "blank door section found (skipping section)" << std::endl;
			}

			new_door_struct new_door;

			// this hairy macro just validates that each key/value is accounted for, and outputs a warning if a value was not found.
			// the macro then assigns the value to the destination variable, and allows it to be "post-processed" with a function
			// before assignment, e.g. atoi() or atof(), etc.
			#define get_config_val(config, section, key, post_proc) { const char *r = al_get_config_value(config, section, #key); if (!r) { std::cout << "Doors::load() error: key \"" << #key << "\" not found under section \"" << section << "\"" << std::endl; } else { new_door . key = post_proc (r); } } 

			new_door.unique_id = new_door_struct::last_unique_id++;
			get_config_val(config, current_section, world_num, atoi);
			get_config_val(config, current_section, level_num, atoi);
			get_config_val(config, current_section, map_num, atoi);
			get_config_val(config, current_section, x, atof_t);
			get_config_val(config, current_section, y, atof_t);
			get_config_val(config, current_section, label, );
			get_config_val(config, current_section, type, atoi);
			get_config_val(config, current_section, destination_world_num, atoi);
			get_config_val(config, current_section, destination_label, );

			#undef get_config_val

			doors.push_back(new_door);
			std::cout << "door section added: " << current_section << " (" << new_door.world_num << " - " << new_door.level_num << " - " << new_door.map_num << std::endl;

		} while(current_section = al_get_next_config_section(&section));

		std::cout << "doors file loaded successfully. " << doors.size() << " door(s) created." << std::endl;

		al_destroy_config(config);
		return true;
	}
	static bool get_collided_door(int world_num, int level_num, int map_num, float x, float y, new_door_struct *collided_door)
	{
		std::vector<new_door_struct> &doors = get_instance()->doors;

		for (unsigned i=0; i<doors.size(); i++)
		{
			if (doors[i].world_num != world_num) continue;
			if (doors[i].level_num != level_num) continue;
			if (doors[i].map_num != map_num) continue;

			int door_x1 = doors[i].x - 16;
			int door_y1 = doors[i].y - 16*3;
			int door_x2 = doors[i].x + 16;
			int door_y2 = doors[i].y;

			if (x < door_x1) continue;
			if (y < door_y1) continue;
			if (x > door_x2) continue;
			if (y > door_y2) continue;

			*collided_door = doors[i];
			return true;
		}
		return false;
	}

	static void draw(int world_num, int level_num, int map_num)
	{
		std::vector<new_door_struct> &doors = get_instance()->doors;
		for (unsigned i=0; i<doors.size(); i++)
		{
			if (doors[i].world_num != world_num) continue;
			if (doors[i].level_num != level_num) continue;
			if (doors[i].map_num != map_num) continue;

			//ALLEGRO_BITMAP *door_bmp = get_door_bitmap(doors[i].type);
			ALLEGRO_BITMAP *door_bmp = get_instance()->crazy_ass_door;
			if (door_bmp) al_draw_bitmap(door_bmp, doors[i].x-al_get_bitmap_width(door_bmp)/2 - camera_x, doors[i].y-al_get_bitmap_height(door_bmp) - camera_y, 0);

			if (false)
			{
				al_draw_rectangle(doors[i].x-16 - camera_x, doors[i].y-48 - camera_y, doors[i].x+16 - camera_x, doors[i].y - camera_y, al_color_name("yellow"), 3.0);
				std::stringstream ss;
				ss << doors[i].map_num << doors[i].destination_label;
				al_draw_text(font_small, al_color_name("yellow"), doors[i].x - 16 - camera_x + 3, doors[i].y-48 + 3 - camera_y, 0, ss.str().c_str());
			}
		}
	}
};

Doors *Doors::instance = NULL;





/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////




void wait_for_press_any_button()
{
    bool get_me_out = false;

    button_a_release = false;
    button_b_release = false;
    button_x_release = false;
    button_y_release = false;

    bool not_pushin_0 = false;
    bool not_pushin_1 = false;
    bool not_pushin_2 = false;
    bool not_pushin_3 = false;

    bool not_pushin_a = false;
    bool not_pushin_b = false;
    bool not_pushin_x = false;
    bool not_pushin_y = false;

    while (!get_me_out)
    {
		poll_keyboard();

      // varify releases
      if (!key[key_for_button_a]) not_pushin_a = true;
      if (!key[key_for_button_b]) not_pushin_b = true;
      if (!key[key_for_button_x]) not_pushin_x = true;
      if (!key[key_for_button_y]) not_pushin_y = true;


      // varify button presses
      if ((key[key_for_button_a] && not_pushin_a)) get_me_out = true;
      if ((key[key_for_button_b] && not_pushin_b)) get_me_out = true;
      if ((key[key_for_button_x] && not_pushin_x)) get_me_out = true;
      if ((key[key_for_button_y] && not_pushin_y)) get_me_out = true;


               poll_joystick();

               int c;
               for (c=0; c<get_joy_num_buttons(); c++)
               {
        	        if (joy[0].button[c])
                    {
                        switch (c)
                        {
                            case 0:
                               if (not_pushin_0) get_me_out = true;
                            break;
                            case 1:
                               if (not_pushin_1) get_me_out = true;
                            break;
                            case 2:
                               if (not_pushin_2) get_me_out = true;
                            break;
                            case 3:
                               if (not_pushin_3) get_me_out = true;
                            break;
                        }
                    }
        	        else if (!joy[0].button[c])
                    {
                        switch (c)
                        {
                            case 0:
                               not_pushin_0 = true;
                            break;
                            case 1:
                               not_pushin_1 = true;
                            break;
                            case 2:
                               not_pushin_2 = true;
                            break;
                            case 3:
                               not_pushin_3 = true;
                            break;
                        }
                    }
               }
    }
}






string make_absolute(string filename)
{
	// takes the filename relative to the executable
	// and produces a full filename

	ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_set_path_filename(path, filename.c_str());
	std::string full_filenam = al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP);
	al_destroy_path(path);

    return full_filenam;
}




const char *full_filename(const char *address)
{
	return make_absolute(address).c_str();
    //return replace_filename(buf, arg_string, address, sizeof(buf));
}


#define load_bitmap(x) _load_bitmap(x)





void my_error_message2(std::string mess, ...)
{
   char buf[512];
std::cout << "my_error_message2(): " << mess << std::endl;
     clear_keybuf();
     readkey();
}




int ten_mili_seconds = 0;

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


#include "motion.h"


class MusicTrack
{
public:
	int index_id;
	std::string filename;
	ALLEGRO_SAMPLE *sample;
	ALLEGRO_SAMPLE_INSTANCE *instance;
	float gain;

	void destroy()
	{
		al_destroy_sample_instance(instance);
		al_destroy_sample(sample);
	}
};



class MultitrackMusic
{
private:
	MusicTrack *_get_track_by_index_id(int index_id)
	{
		for (unsigned i=0; i<tracks.size(); i++)
		{
			if (tracks[i].index_id == index_id) return &tracks[i];
		}
		return NULL;
	}

public:
	Motion automation;
	ALLEGRO_VOICE *voice;
	ALLEGRO_MIXER *mixer;
	std::vector<MusicTrack> tracks;

	int current_playing_index_id;
	float tempo;
	float master_gain;
	float playhead;

	MultitrackMusic()
		: automation()
		, voice(NULL)
		, mixer(NULL)
		, tracks()
		, current_playing_index_id(-1)
		, tempo(120.0)
		, master_gain(1.0)
		, playhead(0.0)
	{
		voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
		if (!voice) std::cout << "Could not create voice" << std::endl;

		mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
		if (!mixer) std::cout << "Could not create voice" << std::endl;

		if (!al_attach_mixer_to_voice(mixer, voice))
		{
			std::cout << "[MultitrackMusic] error: could not attach mixer to voice" << std::endl;
		}

		al_set_mixer_playing(mixer, true);
		al_set_voice_playing(voice, true);
	}

	void clear()
	{
		automation.clear_all();
		for (unsigned i=0; i<tracks.size(); i++) tracks[i].destroy();
		tracks.clear();
		current_playing_index_id = -1;
		playhead = 0.0;
	}

	~MultitrackMusic()
	{
		clear();
		al_destroy_mixer(mixer);
	}

	int get_current_playing_index_id()
	{
		return current_playing_index_id;
	}

	void add_music_layer(int index_id, std::string filename)
	{
		MusicTrack new_music_track;

		new_music_track.index_id = index_id;
		new_music_track.filename = filename;
		new_music_track.sample = al_load_sample(filename.c_str());
		if (!new_music_track.sample) std::cout << "[MultitrackMuic::add_music_track error] sample file \"" << filename << "\" could not load" << std::endl;
		new_music_track.instance = al_create_sample_instance(new_music_track.sample);
		if (!new_music_track.instance) std::cout << "[MultitrackMuic::add_music_track error] could not create sample instance" << std::endl;
		al_attach_sample_instance_to_mixer(new_music_track.instance, mixer);
		al_set_sample_instance_playmode(new_music_track.instance, ALLEGRO_PLAYMODE_LOOP);
		new_music_track.gain = 0.0;

		tracks.push_back(new_music_track);
	}

	void update()
	{
		automation.update(al_get_time());

		// update the parameters
		for (unsigned i=0; i<tracks.size(); i++)
		{
			al_set_sample_instance_gain(tracks[i].instance, tracks[i].gain);
		// perhaps stop sample instances if their gain is 0.0 here, so that /ALL/ the samples aren't playing at the same time
		}
	}

	bool play(int index_id=0)
	{
		return false;
		return crossfade_to(index_id, 0);
	}

	bool fade_in(int index_id, float fade_duration=5)
	{
		return false;
		MusicTrack *track = _get_track_by_index_id(index_id);
		if (!track) return false;
		automation.cmove_to(&track->gain, 1.0, fade_duration, interpolator::fastIn);
		return true;
	}

	bool fade_out(int index_id, float fade_duration=5)
	{
		return false;
		MusicTrack *track = _get_track_by_index_id(index_id);
		if (!track) return false;
		automation.cmove_to(&track->gain, 0.0, fade_duration, interpolator::slowIn);
		return true;
	}

	void quick_dip()
	{
		// alert! pause() will not stop crossfades
		for (unsigned i=0; i<tracks.size(); i++)
		{
			float prev_gain = al_get_sample_instance_gain(tracks[i].instance);
			al_set_sample_instance_gain(tracks[i].instance, prev_gain * 0.001);
		}
	}

	void quick_boost()
	{
		for (unsigned i=0; i<tracks.size(); i++)
		{
			float prev_gain = al_get_sample_instance_gain(tracks[i].instance);
			al_set_sample_instance_gain(tracks[i].instance, prev_gain * 1000.0);
		}
	}

	bool crossfade_to(int index_id, float fade_duration=6.0)
	{
		return false;
		//return true; // silencing the thing
		if (current_playing_index_id == index_id)
		{
			std::cout << "skipping crossfade from " << current_playing_index_id << " to " << index_id << "." << std::endl;
			return true; // no point in crossfading between the same two tracks
		}

		// grab the tracks in question
		MusicTrack *track_to_fade_out = _get_track_by_index_id(current_playing_index_id);
		MusicTrack *track_to_fade_in = _get_track_by_index_id(index_id);

		std::cout << "crossfading to " << index_id << std::endl;
		current_playing_index_id = index_id;

		unsigned int play_position = 0;
		if (track_to_fade_out)
		{
			// syncronize the play position if crossfading between two tracks
			play_position = al_get_sample_instance_position(track_to_fade_out->instance);
		}

		if (track_to_fade_in)
		{
			al_set_sample_instance_position(track_to_fade_in->instance, play_position);
			al_set_sample_instance_playing(track_to_fade_in->instance, true);
		}

		// initiate the fade
		if (track_to_fade_out) fade_out(track_to_fade_out->index_id, fade_duration);
		fade_in(track_to_fade_in->index_id, fade_duration);
	}
};

MultitrackMusic *multitrack_music = NULL;




/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////



int initialize_input_devices(void)
{
	al_install_joystick();
	if (!al_is_joystick_installed()) std::cout << "Joystick not installed" << std::endl;
	else
	{
		joystick_available = true;
		if (al_get_num_joysticks() == 0)
		{
			std::cout << "No joysticks attached." << std::endl;
		}
		else
		{
			joystick = al_get_joystick(0);
		}
	}

	al_install_keyboard();
	keyboard_available = true;

	al_install_mouse();
	mouse_available = true;

    return 0;
}


int initialize_graphics(void)
{
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_image_addon();

	font = al_create_builtin_font();

	al_set_new_display_flags(NEW_DISPLAY_FLAGS);
	al_set_new_display_adapter(DISPLAY_ADAPTER);
	ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W*SCREEN_SCALE_X, SCREEN_H*SCREEN_SCALE_Y);
	buffer = al_get_backbuffer(display);

	projection_subbitmap = al_create_sub_bitmap(al_get_backbuffer(display), 0, 0, al_get_bitmap_width(al_get_backbuffer(display)), al_get_bitmap_height(al_get_backbuffer(display)));


	// setup the transform for the "normal" projection, which is a scale
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_scale_transform(&t, SCREEN_SCALE_X, SCREEN_SCALE_Y);
	al_orthographic_transform(&t, 0, 0, -1.0, al_get_bitmap_width(buffer),
                          al_get_bitmap_height(buffer), 1.0);
	al_set_target_bitmap(buffer);
	al_use_projection_transform(&t);


	// setup the projection transform for the "projection"
	ALLEGRO_TRANSFORM proj_t;
	al_identity_transform(&proj_t);
	float aspect_ratio = (float)al_get_bitmap_height(projection_subbitmap) / al_get_bitmap_width(projection_subbitmap);
	al_perspective_transform(&proj_t, 1, aspect_ratio, 1, -1, -aspect_ratio, 1000);
	al_set_target_bitmap(projection_subbitmap);
	al_use_projection_transform(&proj_t);



	al_set_target_bitmap(buffer);


    clear_to_color(buffer, makecol(0,0,0));
    return 0;
}




int initialize_sound(void)
{
	al_install_audio();
	al_init_acodec_addon();
	al_reserve_samples(32);
	return 0;
}



#define GOOD 1
#define BAD 2
#define NEUTRAL 3


void show_notification(std::string line1, std::string line2, int warning_level=-1)
{
	ALLEGRO_COLOR color = al_color_name("lightslategray");

	if (warning_level == GOOD) color = al_color_name("aquamarine");
	if (warning_level == BAD) color = al_color_name("firebrick");
	if (warning_level == NEUTRAL) color = al_color_name("orange");


	ALLEGRO_BITMAP *screen = al_get_backbuffer(al_get_current_display());

	ALLEGRO_COLOR back_color = al_map_rgba_f(color.r*0.35+0.1, color.g*0.35+0.1, color.b*0.35+0.1, 1);
	ALLEGRO_COLOR line1_color = al_map_rgba_f(color.r*0.65+0.3, color.g*0.65+0.3, color.b*0.65+0.3, 1);
	ALLEGRO_COLOR line2_color = al_color_name("white");

	int width_of_box = max(text_length(font_small, line1.c_str()), text_length(font_small, line2.c_str())) + 40;

	rectfill(screen, 100, 100, SCREEN_W-100, SCREEN_H-100, back_color);
	textprintf_centre(screen, font_small, SCREEN_W/2, SCREEN_H/2-text_height(font_small)/2-7, line1_color, line1);
	textprintf_centre(screen, font_small, SCREEN_W/2, SCREEN_H/2-text_height(font_small)/2+5, line2_color, line2);

	al_flip_display();
	readkey();
}


void show_loading_notice(void)
{
	ALLEGRO_BITMAP *font_bmp = al_load_bitmap("dat/xsmall.bmp");
	al_convert_mask_to_alpha(font_bmp, makecol(255, 0, 255));
	int ranges[] = {32, 126};
	ALLEGRO_FONT *loading_font = al_grab_font_from_bitmap(font_bmp, 1, ranges);

	float rect_width = 70;
	float rect_height = 20;
	al_draw_filled_rectangle(SCREEN_W/2-rect_width/2, SCREEN_H/2-rect_height/2, SCREEN_W/2+rect_width/2, SCREEN_H/2+rect_height/2, al_map_rgba_f(0.0, 0.0, 0.0, 0.8));
	al_draw_text(loading_font, al_map_rgba_f(0.4, 0.4, 0.4, 0.4), SCREEN_W/2, SCREEN_H/2 - al_get_font_line_height(loading_font)/2, ALLEGRO_ALIGN_CENTRE, "- Loading -");
	al_flip_display();

	al_destroy_font(loading_font);
	al_destroy_bitmap(font_bmp);
}


bool key_shift()
{
    if (key[ALLEGRO_KEY_RSHIFT] || key[ALLEGRO_KEY_LSHIFT]) return true;
    return false;
}    



// PREGAME ///////////////////////////////
int initialize(void)
{
	al_init();
   srand(time(NULL));


	ALLEGRO_PATH *resource_path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_change_directory(al_path_cstr(resource_path, ALLEGRO_NATIVE_PATH_SEP));
	al_destroy_path(resource_path);	


	initialize_graphics();
	show_loading_notice();

    if (initialize_sound() != 0)
    {
        allegro_message("unable to initialze sound");
        return 1;
    }    


	initialize_input_devices();





    return 0;
}

void draw_PIXEL_LAYER_to_buffer();
//void update_buffer(void);

//////////////////////////////////////////////////////////



void cleanup(void)
{
   destroy_font(font_med);
   destroy_font(font_small);
   destroy_font(font_x_small);
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////


bool button_x = false;
bool button_y = false;
bool button_a = false;
bool button_b = false;

bool button_up = false;
bool button_down = false;
bool button_left = false;
bool button_right = false;

//bool release_x = true;
//bool release_y = true;
//bool release_a = true;
//bool release_b = true;

bool release_up = true;
bool release_down = true;
bool release_left = true;
bool release_right = true;


void clear_input()
{
     button_x = false;
     button_y = false;
     button_a = false;
     button_b = false;

     button_up = false;
     button_down = false;
     button_left = false;
     button_right = false;
}


////////////// KEYBOARD INPUT
int check_keyboard()
{
	poll_keyboard();

	if (key[key_for_button_up]) button_up = true;
	if (key[key_for_button_down]) button_down = true;
	if (key[key_for_button_left]) button_left = true;
	if (key[key_for_button_right]) button_right = true;
     
    if (key[key_for_button_a] || key[ALLEGRO_KEY_ENTER] || key[ALLEGRO_KEY_4]) button_a = true;
    if (key[key_for_button_b] || key[ALLEGRO_KEY_2]) button_b = true;
    if (key[key_for_button_x] || key[ALLEGRO_KEY_3]) button_x = true;
    if (key[key_for_button_y] || key[ALLEGRO_KEY_1]) button_y = true;

	if (key_shift() && key[ALLEGRO_KEY_ESCAPE]) abort_game = true;	// press esc to quit

    return 0;
}

////////////// JOYSTICK INPUT
void check_joystick()
{
     poll_joystick();
     int c;

     //for (c=0; c<joy[0].num_buttons; c++)
     for (c=0; c<get_joy_num_buttons(); c++)
     {
	    if (joy[0].button[c])
        {
            switch (c)
            {
                   case 0:
                       button_y = true;
                       break;
                   case 1:
                       button_b = true;
                       break;
                   case 2:
                       button_x = true;
                       break;
                   case 3:
                       button_a = true;
                       break;
            }
        }
     }

     if (joy[0].stick[0].axis[0] < 0) button_left = true;
	 if (joy[0].stick[0].axis[0] > 0) button_right = true;
	 if (joy[0].stick[0].axis[1] < 0) button_up = true;
	 if (joy[0].stick[0].axis[1] > 0) button_down = true;

     if (joy[0].stick[1].axis[0] < 0) button_left = true;
	 if (joy[0].stick[1].axis[0] > 0) button_right = true;
	 if (joy[0].stick[1].axis[1] < 0) button_up = true;
	 if (joy[0].stick[1].axis[1] > 0) button_down = true;

     if (joy[0].stick[2].axis[0] < 0) button_left = true;
	 if (joy[0].stick[2].axis[0] > 0) button_right = true;
	 if (joy[0].stick[2].axis[1] < 0) button_up = true;
	 if (joy[0].stick[2].axis[1] > 0) button_down = true;

     //if (joy[0].stick[0].axis[0].d1) button_left = true;
	 //if (joy[0].stick[0].axis[0].d2) button_right = true;
	 //if (joy[0].stick[0].axis[1].d1) button_up = true;
	 //if (joy[0].stick[0].axis[1].d2) button_down = true;
}

void draw_mouse_cursor(ALLEGRO_COLOR col=makecol(255,255,255))
{
     vline(buffer, mouse_x, mouse_y-4, mouse_y+4, col);
     hline(buffer, mouse_x-4, mouse_y, mouse_x+4, col);
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

float get_player_center();
float get_player_middle();



int2 world_to_tile_coordinates(float world_x, float world_y);
int block_type_at_tile_coord(float tile_x, float tile_y);
int block_type_at(float world_x, float world_y);
//int graphic_block_type_at(int x, int y);
int block_left_edge(int horiz);
int block_right_edge(int horiz);
int block_top_edge(int vertic);
int block_bottom_edge(int vertic);

/* TODO: depreciate these functions */
// YOU WERE HERE //
bool is_platform_block(int type);
bool is_ceiling_block(int type);
bool is_right_wall_block(int type);
bool is_left_wall_block(int type);
bool is_passable_platform_block(int type); // SPECIFICALLY HERE: make this not needed because only fire collisions when passing through boundaries

int get_on_a_ramp_judge();
//void get_off_a_ramp_judge();

//void restart_level(void);
int load_level(const char *address, int map_num=-1);

//float point_on_ramp_y(int ramp_index_num, float xx);

//int fill_with_boundary(int xx, int yy, int xx2, int yy2, int boundary_tile);

int get_map_width();
int get_map_height();

int get_num_of_ramps();

#define TILE_SIZE 16

#define BLOCK_BOUNDRY_EMPTY 0
#define BLOCK_BOUNDRY_0     BOUNDRY_EMPTY
#define BLOCK_BOUNDRY_1     1
#define BLOCK_BOUNDRY_2     2
#define BLOCK_BOUNDRY_3     3
#define BLOCK_BOUNDRY_4     4
#define BLOCK_BOUNDRY_5     5
#define BLOCK_BOUNDRY_6     6
#define BLOCK_BOUNDRY_7     7
#define BLOCK_BOUNDRY_8     8
#define BLOCK_BOUNDRY_9     9
#define BLOCK_BOUNDRY_10    10
#define BLOCK_BOUNDRY_11    11
#define BLOCK_BOUNDRY_12    12
#define BLOCK_BOUNDRY_13    13
#define BLOCK_BOUNDRY_14    14
#define BLOCK_BOUNDRY_15    15
#define BLOCK_BOUNDRY_16    16


#define BLOCK_EMPTY         0

#define BLOCK_LADDER        17
#define BLOCK_DEATH         18
#define BLOCK_DESTRUCTABLE  19
#define BLOCK_DESTRUCTED	 20 // this is a block that was destructable1 and has been destructed
#define BLOCK_LADDER_TOP    21 
#define BLOCK_LADDER_PLATFORM	22 

#define BLOCK_GOAL_1        100


#define BLOCK_SWITCH_STICK  200000
#define BLOCK_SWITCH_BLOCK  (32+1)
//#define BLOCK_SWITCH_BLOCK  300000


#define BLOCK_UNLOCK_CAMERA (100000-1)
#define BLOCK_CAMERA 100000


#define BLOCK_TRIGGER       400000


bool is_ladder_block(int block_num)
{
	if (block_num == BLOCK_LADDER) return true;
	if (block_num == BLOCK_LADDER_TOP) return true;
	if (block_num == BLOCK_LADDER_PLATFORM) return true;
	return false;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

#define MOVING_UP        8
#define MOVING_DOWN      2
#define MOVING_LEFT      4
#define MOVING_RIGHT     6
#define MOVING_STILL     5

#define ENVIRONMENT_DRAG_AIR 0.0f
#define ENVIRONMENT_DRAG_FLOOR 0.075f   // .075 normal .025f ice
#define ENVIRONMENT_DRAG_ICE 0.000f   // .075 normal .025f ice
#define GRAVITY 0.3f // was 0.15
#define TERMINAL_VELOCITY 6 //TILE_SIZE




//////


#define DIRECTORY_TILES "images/tiles/"



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



#define DIALOGUE_TEXT "text"
#define DIALOGUE_SUPER_ITEM "super_item"


class dialogue_struct
{
public:
	float x;
	float y;
	float width;
	std::string text;
	std::string style;
	void draw()
	{
		if (style == DIALOGUE_TEXT)
		{
			int padding = 12;
			int height = 60;
			al_draw_filled_rectangle(x, y, x+width, y+height, al_color_name("darkblue"));
			al_draw_rectangle(x, y, x+width, y+height, al_color_name("white"), 2.0);

			al_draw_multiline_text(game_font, al_color_name("white"),
				x+padding, y+padding, width-padding*2,
				0, ALLEGRO_ALIGN_LEFT, text.c_str());
		}
		else if (style == DIALOGUE_SUPER_ITEM)
		{
			al_draw_filled_rectangle(SCREEN_W/2-100, SCREEN_H/2-15, SCREEN_W/2+100, SCREEN_H/2+15, al_color_name("black"));
			write_outline_center(SCREEN_W/2, SCREEN_H/2-5, game_font, al_color_name("white"), text.c_str());
		}
	}
};


class Dialogues
{
private:
	static Dialogues *instance;
	Dialogues()
		: dialogues()
	{}
	static Dialogues *get_instance()
	{
		if (!instance) instance = new Dialogues();
		return instance;
	}

public:
	std::vector<dialogue_struct> dialogues;

	static void clear()
	{
		std::vector<dialogue_struct> &dialogues = Dialogues::get_instance()->dialogues;
		dialogues.clear();
	}
	static void spawn(std::string text, std::string style=DIALOGUE_TEXT, float x=SCREEN_W/2-120, float y=SCREEN_H/2-15, float width=240)
	{
		std::vector<dialogue_struct> &dialogues = Dialogues::get_instance()->dialogues;

		clear();		

		dialogue_struct new_dialogue;
		new_dialogue.x = x;
		new_dialogue.y = y;
		new_dialogue.width = width;
		new_dialogue.style = style;
		new_dialogue.text = text;
		dialogues.push_back(new_dialogue);
	}
	static void update()
	{
		std::vector<dialogue_struct> &dialogues = Dialogues::get_instance()->dialogues;

		if (((button_a && button_a_release) || (button_b && button_b_release))
			&& !dialogues.empty())
				dialogues.pop_back();
	}
	static void draw()
	{
		std::vector<dialogue_struct> &dialogues = Dialogues::get_instance()->dialogues;

		for (unsigned i=0; i<dialogues.size(); i++)
			dialogues[i].draw();
	}
	static bool dialogue_is_open()
	{
		std::vector<dialogue_struct> &dialogues = Dialogues::get_instance()->dialogues;
		return !dialogues.empty();
	}
};

Dialogues *Dialogues::instance = NULL;




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




class animation_sequence
{
public:
	struct frame
	{
		float duration;
		ALLEGRO_BITMAP *bmp;
	};

	std::string name;
	std::vector<frame> frames;
	float playhead;
	ALLEGRO_BITMAP *current_frame;
	ALLEGRO_COLOR color;
	int x, y;
	float align_x, align_y;
	bool looped;

	animation_sequence(std::string name)
		: name(name)
		, frames()
		, playhead(0)
		, current_frame(NULL)
		, color(al_color_name("white"))
		, x(0)
		, y(0)
		, align_x(0.5)
		, align_y(0.5)
		, looped(false)
	{}

	float get_duration()
	{
		float duration = 0;
		for (unsigned i=0; i<frames.size(); i++)
			duration += frames[i].duration;
		return duration;
	}

	void update()
	{
		if (frames.empty()) return;

		playhead += GAMEPLAY_FRAME_DURATION;
		current_frame = NULL;

		if (looped)
		{
			// prevent the playhead from getting too large
			float animation_total_duration = get_duration();
			while (playhead > animation_total_duration) playhead -= animation_total_duration;
		}

		float duration_cursor = 0;
		for (unsigned i=0; i<frames.size(); i++)
		{
			current_frame = frames[i].bmp;
			duration_cursor += frames[i].duration;
			if (playhead < duration_cursor) return;
		}
	}

	bool finished()
	{
		float duration = get_duration();

		if (playhead > duration) return true;
		return false;
	}

	void add_frame(ALLEGRO_BITMAP *bmp, float duration)
	{
		frame f;
		f.bmp = bmp;
		f.duration = duration;

		frames.push_back(f);
	}
};


class VisualFX
{
private:
	static VisualFX *instance;
	std::vector<animation_sequence> sequences;
	std::vector<animation_sequence> sequence_factory;
	VisualFX() {}
	static VisualFX *get_instance()
	{
		if (!instance) instance = new VisualFX();
		return instance;
	}

public:

	static animation_sequence *get_sequence_by_name(std::string name)
	{
		VisualFX *inst = get_instance();

		for (int i=0; i<inst->sequence_factory.size(); i++)
			if (inst->sequence_factory[i].name == name) return &inst->sequence_factory[i];

		return NULL;
	}

	static void clear()
	{
		VisualFX *inst = get_instance();
		inst->sequences.clear();
	}

	static void spawn(float x, float y, float align_x, float align_y, std::string sequence_name, bool looped=false, ALLEGRO_COLOR color=al_color_name("white"))
	{
		VisualFX *inst = get_instance();
		animation_sequence *sequence = get_sequence_by_name(sequence_name);

		if (sequence)
		{
			// once found, copy the sequence from the factory into an active sequence

			animation_sequence new_sequence = *sequence;

 			new_sequence.x = x;
			new_sequence.y = y;
			new_sequence.align_x = align_x;
			new_sequence.align_y = align_y;
			new_sequence.looped = looped;
			new_sequence.color = color;

			inst->sequences.push_back(new_sequence);
		}
		else
		{
			std::cout << "Could not find animation sequence \"" << sequence_name << "\"" << std::endl;

		}	
	}

	static animation_sequence *get_last_created_sequence()
	{
		VisualFX *inst = get_instance();
		if (inst->sequences.empty()) return NULL;
		return &inst->sequences.back();
	}

	static void update()
	{
		VisualFX *inst = get_instance();
		for (unsigned i=0; i<inst->sequences.size(); i++)
		{
			inst->sequences[i].update();
			if (inst->sequences[i].finished())
			{
				inst->sequences.erase(inst->sequences.begin() + i--);
			}
		}
	}

	static void build_animation_sequence_from_folder(std::string folder_name, float framerate=0.1)
	{
		VisualFX *inst = get_instance();

		animation_sequence constructed_sequence(folder_name);
		
		ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
		al_append_path_component(path, "images");
		al_append_path_component(path, "effects");
		al_append_path_component(path, folder_name.c_str());

		ALLEGRO_FS_ENTRY *fs_entry = al_create_fs_entry(al_path_cstr(path, ALLEGRO_NATIVE_PATH_SEP));

		if (al_open_directory(fs_entry))
		{
			ALLEGRO_FS_ENTRY *entry = NULL;
			while(entry = al_read_directory(fs_entry))
			{
				constructed_sequence.add_frame(_load_bitmap(al_get_fs_entry_name(entry)), framerate);

				al_destroy_fs_entry(entry);
				entry = NULL;
			}
			al_close_directory(fs_entry);

			// set the first frame
			if (!constructed_sequence.frames.empty())
				constructed_sequence.current_frame = constructed_sequence.frames[0].bmp;

			inst->sequence_factory.push_back(constructed_sequence);
			std::cout << "animation sequence \"" << folder_name << "\" created with "
				<< constructed_sequence.frames.size() << " frame(s)" << std::endl;
		}
		else
		{
			std::cout << "could not load animation sequence \"" << folder_name << "\"" << std::endl;
		}

		al_destroy_path(path);
	}

	static void draw_all()
	{
		VisualFX *inst = get_instance();
		for (unsigned i=0; i<inst->sequences.size(); i++)
		{
			ALLEGRO_BITMAP *bmp = inst->sequences[i].current_frame;
			if (bmp)
			{
				float x = inst->sequences[i].x - (int)camera_x - al_get_bitmap_width(bmp)*inst->sequences[i].align_x;
				float y = inst->sequences[i].y - (int)camera_y - al_get_bitmap_height(bmp)*inst->sequences[i].align_y;
				al_draw_tinted_bitmap(bmp, inst->sequences[i].color, x, y, 0);
			}
			else std::cout << "!no_bmp";
		}
	}
};

VisualFX *VisualFX::instance = NULL;




/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
////// ITEMS ////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////




#define ITEM_LIST \
ADD_ITEM(undefined,      0, false) \
ADD_ITEM(heart,          1, false) \
ADD_ITEM(key,            2, false) \
ADD_ITEM(silver_key,     3, false) \
ADD_ITEM(small_jewel,    4, false) \
ADD_ITEM(medium_jewel,   5, false) \
ADD_ITEM(large_jewel,    6, false) \
ADD_ITEM(diamond,        7, false) \
ADD_ITEM(gun,            8, true) \
ADD_ITEM(jump_shoes,     9, true) \
ADD_ITEM(running_shoes,  10, true) \
ADD_ITEM(stick,          11, true) \
ADD_ITEM(shovel,         12, true) \
ADD_ITEM(angel_wings,    13, true) \
ADD_ITEM(jetpack,        14, true) \
ADD_ITEM(canister_with_a_heart, 15, true) \
ADD_ITEM(bow_and_arrows, 16, true)



ALLEGRO_BITMAP *shovel_icon = NULL;


enum item_t
{
#define ADD_ITEM(xx, yy, zz)    xx##_const = yy,
	ITEM_LIST
#undef ADD_ITEM
};


#define ADD_ITEM(xx, yy, zz)     ALLEGRO_BITMAP* xx##_img;
	ITEM_LIST
#undef ADD_ITEM


ALLEGRO_BITMAP *get_item_image(item_t item)
{
	switch(item)
	{
#define ADD_ITEM(xx, yy, zz) case xx##_const: return xx##_img; break;
	ITEM_LIST
#undef ADD_ITEM
	}

	return NULL;
}



std::string get_item_name(item_t item)
{
	std::string result = "unknown item";
	switch(item)
	{
	#define ADD_ITEM(xx, yy, zz) case xx##_const: result = "" #xx; break;
		ITEM_LIST
	#undef ADD_ITEM
	}
	std::replace(result.begin(), result.end(), '_', ' ');
	return result;
}



bool is_item_worth_celebrating(item_t item)
{
	switch(item)
	{
	#define ADD_ITEM(xx, yy, zz) case xx##_const: return zz; break;
		ITEM_LIST
	#undef ADD_ITEM
	}
	return false;
}



item_t str_to_item_type(std::string str)
{
#define ADD_ITEM(xx, yy, zz) if (str == #xx) { return xx##_const; }
	ITEM_LIST
#undef ADD_ITEM
	return undefined_const;
}

ALLEGRO_BITMAP *get_jewel_image(int item_type, float map_x, float map_y);



#define SPAWNED_ITEM_LIFESPAN 7.0


class item_struct
{
private:
	static int last_unique_id;

public:
	int world_num;
	int level_num;
	int map_num;

	ALLEGRO_BITMAP *image;
	int unique_id;
	item_t type;
	float x;
	float y;
	float align_x;
	float align_y;

	bool alive;

	// lifespan and behavior
	bool temporary;
	bool with_physics;
	float lifespan;
	float velocity_x;
	float velocity_y;
	float drag;

	bool collides(float xx, float yy, float ww, float hh)
	{
		int hsize = 22; // effectively giving the item a bounding box of (44, 44)
		if (xx < x-hsize) return false;
		if (yy < y-hsize) return false;
		if (xx > x+hsize) return false;
		if (yy > y+hsize) return false;
		return true;
	}
	virtual void update()
	{
		if (!alive) return;

		// update lifespan
		if (temporary)
		{
			lifespan -= GAMEPLAY_FRAME_DURATION;
			if (lifespan <= 0)
			{
				lifespan = 0.0;
				alive = false;
				image = NULL;
				VisualFX::spawn(x, y, 0.5, 0.5, "explosion");
				return;
			}
		}

		// update with (or without) physics
		if (with_physics)
		{
			update_with_physics();
		}
		else
		{
			velocity_x *= (1.0 - drag);
			velocity_y *= (1.0 - drag);
			x += velocity_x;
			y += velocity_y;
		}

		// set the appropriate image
		image = get_item_image(type);

		if ((type == small_jewel_const) || (type == medium_jewel_const) || (type == large_jewel_const))
		{
			image = get_jewel_image(type, x, y);
		}
	}

	bool is_in_numbspawn()
	{
		if (temporary)
		{
			float ITEM_NUMBSPAWN_DURATION = 0.6;
			if (lifespan > (SPAWNED_ITEM_LIFESPAN-ITEM_NUMBSPAWN_DURATION)) return true;
			else return false;
		}
		return false;
	}

	static int get_unique_id()
	{
		return item_struct::last_unique_id++;
	}

	//
	// all the physicisy stuff
	//  -- with physics! :D
	//

    void test_vertical_map_collisions()
    {
        bool char_collides_next = false;
        bool char_collides_now = false;

        if (velocity_y > 0.0f) // falling
        {
            if (block_type_at((int)(x), (int)(y+velocity_y)+1) == BLOCK_DESTRUCTABLE) return;
            if (is_platform_block(block_type_at((int)(x), (int)(y+velocity_y)+1)))
            {
                char_collides_next = true;
            }
            if (is_platform_block(block_type_at((int)(x), (int)y)))
            {
                char_collides_now = true;
            }
        }
        if ((!char_collides_now) && (char_collides_next))
        {
            velocity_y *= -1.0f; // reflect
            velocity_y *= 0.4; // dampen
            y = block_top_edge(((int)(y+velocity_y+8)+1)/16) - 1;
            if (velocity_y < -1.0f)
				{
					// play a item bounce on wall sound here
					//play_ball_bounce_sound(x - camera.x);
				}
        }
        else if (velocity_y < 0.0f) // jumping
        {
            if (block_type_at((int)(x), (int)(y+velocity_y)-1) == BLOCK_DESTRUCTABLE) return;
            if (is_ceiling_block(block_type_at((int)(x), (int)(y+velocity_y)-1)))
            {
					velocity_y *= -1.0f; // reflect
					velocity_y *= 0.4; // dampen
                y = block_bottom_edge(((int)(y+velocity_y-8)-1)/16) + 1;
					// play a item bounce on wall sound here
               // play_ball_bounce_sound(x - camera.x);
            }
        }
    }

    void test_horizontal_map_collisions()
    {
        bool char_collides_next = false;
        bool char_collides_now = false;

        if (velocity_x > 0.0f) // falling
        {
            if (block_type_at((int)(x+velocity_x)+1, (int)(y)) == BLOCK_DESTRUCTABLE) return;
            if (is_left_wall_block(block_type_at((int)(x+velocity_x)+1, (int)(y))))
            {
                char_collides_next = true;
            }
            if (is_left_wall_block(block_type_at((int)(x), (int)y)))
            {
                char_collides_now = true;
            }
        }
        if ((!char_collides_now) && (char_collides_next))
        {
            velocity_x *= -1.0f;
				 velocity_y *= 0.4f;
            x = block_left_edge(((int)(x+velocity_x+8)+1)/16) - 1;
				// play a item bounce on wall sound here
            //play_ball_bounce_sound(x - camera.x);
        }

        else if (velocity_x < 0.0f) // jumping
        {
            if (block_type_at((int)(x+velocity_x)-1, (int)(y)) == BLOCK_DESTRUCTABLE) return;
            if (is_right_wall_block(block_type_at((int)(x+velocity_x)-1, (int)(y))))
            {
                velocity_x *= -1.0f;
					 velocity_x *= 0.4f;
                x = block_right_edge(((int)(x+velocity_x-8)-1)/16) + 1;
					// play a item bounce on wall sound here
                //play_ball_bounce_sound(x - camera.x);
            }
        }
    }

    bool outside_screen()
    {
         if ((int)x-(int)camera_x < 0-64) return true;
         if ((int)x-(int)camera_x > SCREEN_W+64) return true;
         if ((int)y-(int)camera_y < 0-64) return true;
         if ((int)y-(int)camera_y > SCREEN_H+64) return true;
         return false;
    }

    void ricochet()
    {
        velocity_x *= -1.0f;
        velocity_y *= -1.0f;
    }

    void update_with_physics()
    {
        if (alive)
        {
				velocity_x *= (1.0 - drag);
				velocity_y *= (1.0 - drag);
           //if (velocity_x > 0) velocity_x -= 0.03f;
           //else if (velocity_x < 0) velocity_x += 0.03f;

           velocity_y += GRAVITY;
           if (velocity_y > TERMINAL_VELOCITY) velocity_y = TERMINAL_VELOCITY;

           test_vertical_map_collisions();
           y += velocity_y;
           test_horizontal_map_collisions();
           x += velocity_x;

           float min_motion_range = 0.05f;
           if ((fabs(velocity_x) < min_motion_range) && (fabs(velocity_y) < min_motion_range))
				{
					velocity_x = 0;
					velocity_y = 0;
				}
        }

        if (outside_screen()) alive = false;
    }

};

int item_struct::last_unique_id = 0;



class Items
{
private:
	static Items *instance;
	static Items *get_instance()
	{
		if (!instance) instance = new Items();
		return instance;
	}
	std::vector<item_struct> items;
public:
	static void clear()
	{
		Items::get_instance()->items.clear();
	}
	static void load(std::string filename)
	{
		std::cout << "Items::load() is depreciated." << std::endl;
		//Items::clear();
		//Items::append_items_file(filename);
	}
	static void spawn_item(int world_num, int level_num, int map_num, float x, float y, float align_x, float align_y, item_t type, float velocity_x, float velocity_y)
	{
		item_struct *new_item = add_item(world_num, level_num, map_num, x, y, align_x, align_y, type);

		if (new_item)
		{
			new_item->with_physics = true;
			new_item->temporary = true;
			new_item->lifespan = SPAWNED_ITEM_LIFESPAN;
			new_item->velocity_x = velocity_x;
			new_item->velocity_y = velocity_y;
			new_item->drag = 0.05;
		}
	}
	static item_struct *add_item(int world_num, int level_num, int map_num, float x, float y, float align_x, float align_y, item_t type)
	{
		item_struct new_item;

		new_item.unique_id = item_struct::get_unique_id();
		new_item.image = NULL;
		new_item.alive = true;
		new_item.world_num = world_num;
		new_item.level_num = level_num;
		new_item.map_num = map_num;
		new_item.type = type;
		new_item.x = x;
		new_item.y = y;
		new_item.align_x = align_x;
		new_item.align_y = align_y;

		// behavior
		new_item.with_physics = false;
		new_item.temporary = false;
		new_item.lifespan = 0.0;
		new_item.velocity_x = 0.0;
		new_item.velocity_y = 0.0;
		new_item.drag = 1.0;

		get_instance()->items.push_back(new_item);
		return &get_instance()->items.back();
	}
/*
	static bool append_items_file(std::string filename)
	{
		std::vector<item_struct> &items = get_instance()->items;

		ALLEGRO_CONFIG *config = al_load_config_file(filename.c_str());
		if (!config)
		{
			std::cout << "Could not load items file \"" << filename << "\"" << std::endl;
			return false;
		}

		ALLEGRO_CONFIG_SECTION *section;
		const char *current_section = al_get_first_config_section(config, &section);

		if (current_section == NULL)
		{
			std::cout << "it appears that the .items file \"" << filename << "\" is empty; skipping loading." << std::endl;
			return true;
		}

		do
		{
			if (strcmp(current_section, "") == 0)
			{
				std::cout << "skipping global section in item file" << std::endl;
				continue;
			}

			std::cout << "item section found: " << current_section << std::endl;

			item_struct new_item;

			// this hairy macro just validates that each key/value is accounted for, and outputs a warning if a value was not found.
			// the macro then assigns the value to the destination variable, and allows it to be "post-processed" with a function
			// before assignment, e.g. atoi() or atof(), etc.
			#define get_config_val(config, section, key, post_proc) { const char *r = al_get_config_value(config, section, #key); if (!r) { std::cout << "Items::load() error: key \"" << #key << "\" not found under section \"" << section << "\"" << std::endl; } else { new_item . key = post_proc (r); } } 

			new_item.unique_id = item_struct::get_unique_id();
			new_item.image = NULL;
			new_item.alive = true;
			get_config_val(config, current_section, world_num, atoi);
			get_config_val(config, current_section, level_num, atoi);
			get_config_val(config, current_section, map_num, atoi);
			get_config_val(config, current_section, type, str_to_item_type); // mkay, might want to change this in the future
			get_config_val(config, current_section, x, atof_t);
			get_config_val(config, current_section, y, atof_t);
			get_config_val(config, current_section, align_x, atof_t);
			get_config_val(config, current_section, align_y, atof_t);

			#undef get_config_val

			items.push_back(new_item);
		} while (current_section = al_get_next_config_section(&section));

		std::cout << "items file loaded successfully. " << items.size() << " item(s) created." << std::endl;

		al_destroy_config(config);
		return true;
	}
*/
	static void update()
	{
		std::vector<item_struct> &items = Items::get_instance()->items;
		for (unsigned i=0; i<items.size(); i++)
		{
			if (!items[i].alive) continue;

			items[i].update();
		}
	}
	static void draw(int world_num, int level_num, int map_num)
	{
		std::vector<item_struct> &items = Items::get_instance()->items;
		for (unsigned i=0; i<items.size(); i++)
		{
			if (!items[i].alive) continue;

			if (items[i].world_num != world_num) continue;
			if (items[i].level_num != level_num) continue;
			if (items[i].map_num != map_num) continue;

			item_struct &item = items[i];

			bool strobing_off = false;
			if (item.temporary)
			{
				if (item.lifespan < 1.0) strobing_off = (fmod(item.lifespan, 0.2) < 0.1);
				else if (item.lifespan < 2.0) strobing_off = (fmod(item.lifespan, 0.5) < 0.1);
				else if (item.lifespan < 4.0) strobing_off = (fmod(item.lifespan, 1.0) < 0.1);
			}

			if (item.image && !strobing_off)
			{
				int w = al_get_bitmap_width(item.image);
				int h = al_get_bitmap_height(item.image);
				al_draw_bitmap(item.image, (int)(item.x-w*item.align_x-camera_x), (int)(item.y-h*item.align_y-camera_y), 0);
			}
		}
	}
	static bool get_collided_item(int world_num, int level_num, int map_num, float x, float y, item_struct *collided_item)
	{
		std::vector<item_struct> &items = Items::get_instance()->items;
		for (unsigned i=0; i<items.size(); i++)
		{
			if (!items[i].alive || items[i].is_in_numbspawn()) continue;

			if (items[i].world_num != world_num) continue;
			if (items[i].level_num != level_num) continue;
			if (items[i].map_num != map_num) continue;

			if (items[i].collides(x, y, 0, 0))
			{
				*collided_item = items[i];
				return true;
			}
		}
		return false;
	}
	static bool remove_item_by_id(int unique_id)
	{
		std::vector<item_struct> &items = Items::get_instance()->items;
		for (unsigned i=0; i<items.size(); i++)
		{
			if (items[i].unique_id == unique_id)
			{
				items.erase(items.begin()+i);
				return true;
			}
		}
		return false;
	}
};

Items *Items::instance = NULL;


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//////// INVENTORY //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


// DONT FORGET the blowing bubbles item that trapps enemies and makes them float up

// ALSO DONT FORGET viles of bright pink liquid that get mixed up, changing your form (and control scheme) into different things

class Inventory
{
private:
	static Inventory *instance;
   std::vector<item_t> items;
	Inventory()
		: items()
	{}
	static Inventory *get_instance()
	{
		if (!instance) instance = new Inventory();
		return instance;
	}
public:
   static void add_item(item_t item)
   {
      get_instance()->items.push_back(item);
   }

   static bool remove_item(item_t item)
   {
		std::vector<item_t> &items = get_instance()->items;
      // find the item and remove it
      // return true if it existed and was removed,
      // otherwise return false
      for (unsigned i=0; i<items.size(); i++)
      {
         if (items[i] == item)
         {
            items.erase(items.begin()+i);
            return true;
         }
      }
      return false;
   }

   static int count_items_of_type(item_t type)
   {
		std::vector<item_t> &items = get_instance()->items;
      int count = 0;
      for (unsigned i=0; i<items.size(); i++)
      {
         if (items[i] == type) count++;
      }
      return count;
   }

	static void draw_inventory_screen()
	{
		
	}
};




/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
//////// CHECKPOINTS ////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


class checkpoint_struct
{
public:
	static int last_unique_id;
	int unique_id;

	int world_num;
	int level_num;
	int map_num;
	float x;
	float y;
};

int checkpoint_struct::last_unique_id = 1;


class Checkpoints
{
private:
	static Checkpoints *instance;
	std::vector<checkpoint_struct> checkpoints;
	int last_triggered_checkpoint_id;
	Checkpoints()
		: checkpoints()
		, last_triggered_checkpoint_id(-1)
	{
	}
	static Checkpoints *get_instance()
	{
		if (!instance) instance = new Checkpoints();
		return instance;
	}
public:

	static ALLEGRO_BITMAP *get_checkpoint_bitmap()
	{
		return checkpoint;
	}

	static void clear()
	{
		Checkpoints *inst = get_instance();
		checkpoint_struct::last_unique_id = 1;
		inst->last_triggered_checkpoint_id = -1;
		inst->checkpoints.clear();
	}

	static void load(std::string filename)
	{
		clear();
		append_checkpoints_file(filename);
	}

	static bool get_last_visited_checkpoint(checkpoint_struct *checkpoint)
	{
		int last_triggered_checkpoint_id = get_instance()->last_triggered_checkpoint_id;
		std::vector<checkpoint_struct> &checkpoints = get_instance()->checkpoints;

		if (last_triggered_checkpoint_id == -1) return false;
		for (unsigned i=0; i<checkpoints.size(); i++)
		{
			if (checkpoints[i].unique_id == last_triggered_checkpoint_id)
			{
				*checkpoint = checkpoints[i];
				return true;
			}
		}
		return false;
	}

	static void trigger_checkpoint(int checkpoint_id)
	{
		Checkpoints *inst = Checkpoints::get_instance();
		std::vector<checkpoint_struct> &checkpoints = get_instance()->checkpoints;

		for (unsigned i=0; i<checkpoints.size(); i++)
		{
			if (checkpoints[i].unique_id == checkpoint_id)
			{
				inst->last_triggered_checkpoint_id = checkpoint_id;
				VisualFX::spawn(checkpoints[i].x, checkpoints[i].y, 0.5, 1.0, "explosion");
				return;
			}
		}
	}

	static void add_checkpoint(int world_num, int level_num, int map_num, float x, float y)
	{
		checkpoint_struct new_checkpoint;
		new_checkpoint.world_num = world_num;
		new_checkpoint.level_num = level_num;
		new_checkpoint.map_num = map_num;
		new_checkpoint.x = x;
		new_checkpoint.y = y;
		Checkpoints::get_instance()->checkpoints.push_back(new_checkpoint);
	}

	static bool append_checkpoints_file(std::string filename)
	{
		std::vector<checkpoint_struct> &checkpoints = get_instance()->checkpoints;

		ALLEGRO_CONFIG *config = al_load_config_file(filename.c_str());
		if (!config)
		{
			std::cout << "Could not load checkpoints file \"" << filename << "\"" << std::endl;
			return false;
		}

		ALLEGRO_CONFIG_SECTION *section;
		const char *current_section = al_get_first_config_section(config, &section);

		if (current_section == NULL)
		{
			std::cout << "it appears that the .checkpoints file \"" << filename << "\" is empty; skipping loading." << std::endl;
			return true;
		}

		do
		{
			if (strcmp(current_section, "") == 0)
			{
				std::cout << "skipping global section in checkpoint file" << std::endl;
				continue;
			}

			std::cout << "checkpoint section found: " << current_section << std::endl;

			checkpoint_struct new_checkpoint;

			// this hairy macro just validates that each key/value is accounted for, and outputs a warning if a value was not found.
			// the macro then assigns the value to the destination variable, and allows it to be "post-processed" with a function
			// before assignment, e.g. atoi() or atof(), etc.
			#define get_config_val(config, section, key, post_proc) { const char *r = al_get_config_value(config, section, #key); if (!r) { std::cout << "Checkpoints::load() error: key \"" << #key << "\" not found under section \"" << section << "\"" << std::endl; } else { new_checkpoint . key = post_proc (r); } } 

			new_checkpoint.unique_id = checkpoint_struct::last_unique_id++;
			get_config_val(config, current_section, world_num, atoi);
			get_config_val(config, current_section, level_num, atoi);
			get_config_val(config, current_section, map_num, atoi);
			get_config_val(config, current_section, x, atof_t);
			get_config_val(config, current_section, y, atof_t);

			#undef get_config_val

			checkpoints.push_back(new_checkpoint);
		} while (current_section = al_get_next_config_section(&section));

		std::cout << "checkpoints file loaded successfully. " << checkpoints.size() << " checkpoint(s) created." << std::endl;

		al_destroy_config(config);
		return true;
	}

	static bool get_collided_checkpoint(int world_num, int level_num, int map_num, int x, int y, checkpoint_struct *collided_checkpoint)
	{
		std::vector<checkpoint_struct> &checkpoints = get_instance()->checkpoints;

		for (unsigned i=0; i<checkpoints.size(); i++)
		{
			if (checkpoints[i].world_num != world_num) continue;
			if (checkpoints[i].level_num != level_num) continue;
			if (checkpoints[i].map_num != map_num) continue;

			int checkpoint_x1 = checkpoints[i].x - 8;
			int checkpoint_y1 = checkpoints[i].y - 16*3;
			int checkpoint_x2 = checkpoints[i].x + 8;
			int checkpoint_y2 = checkpoints[i].y;

			if (x < checkpoint_x1) continue;
			if (y < checkpoint_y1) continue;
			if (x > checkpoint_x2) continue;
			if (y > checkpoint_y2) continue;

			if (collided_checkpoint != NULL) *collided_checkpoint = checkpoints[i];
			return true;
		}
		return false;
	}

	static void draw(int world_num, int level_num, int map_num)
	{
		Checkpoints *inst = get_instance();
		std::vector<checkpoint_struct> &checkpoints = inst->checkpoints;

		for (unsigned i=0; i<checkpoints.size(); i++)
		{
			if (checkpoints[i].world_num != world_num) continue;
			if (checkpoints[i].level_num != level_num) continue;
			if (checkpoints[i].map_num != map_num) continue;

			ALLEGRO_BITMAP *checkpoint_bmp = get_checkpoint_bitmap();
			if (checkpoint_bmp)
			{
				if (checkpoints[i].unique_id == inst->last_triggered_checkpoint_id)
					al_draw_bitmap(life_icon, checkpoints[i].x-al_get_bitmap_width(life_icon)/2 - camera_x, checkpoints[i].y-47-camera_y, 0); 

				al_draw_bitmap(checkpoint_bmp, checkpoints[i].x-al_get_bitmap_width(checkpoint_bmp)/2 - camera_x, checkpoints[i].y-al_get_bitmap_height(checkpoint_bmp) - camera_y, 0);
			}

			//al_draw_rectangle(checkpoints[i].x-8 - camera_x, checkpoints[i].y-48 - camera_y,
			//	checkpoints[i].x+8 - camera_x, checkpoints[i].y - camera_y, al_color_name("aquamarine"), 3.0);
		}
	}
};

Checkpoints *Checkpoints::instance = NULL;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



ALLEGRO_SAMPLE *rock_destruct_sound = NULL;

void play_rock_destroy_sound()
{
	if (!rock_destruct_sound)
	{
		std::cout << "could not play rock destruct sound" << std::endl;
		return;
	}
	al_play_sample(rock_destruct_sound, 1.0, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

int previous_rock_sample_number = 1;
int second_previous_rock_sample_number = 0;
void play_random_rock_sample(int vol, int pan, int freq)
{
    int i = random_int(1, 10);
    while ((i == previous_rock_sample_number) || (i == second_previous_rock_sample_number)) i = random_int(1, 10);

    second_previous_rock_sample_number = previous_rock_sample_number;
    previous_rock_sample_number = i;

    ALLEGRO_SAMPLE *s;

    switch(i)
    {

    #define ADD_ROCK(x, y)    case y:  s = x##y##_sample; break;
    //#include "rock_list.hpp"
		ROCK_LIST
    #undef ADD_ROCK
        default:
                s = NULL;
                break;
    }

    if (s) play_sample(s, vol, pan, freq, false);
}








////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



bool start_new_level=false;




#define DRAWING_TO_LAYER_1     9999
#define DRAWING_TO_LAYER_2     9998
#define DRAWING_TO_LAYER_3     9997
#define DRAWING_TO_BOUNDRY     9996
#define DRAWING_TO_ENVIRONMENT 9995

unsigned int drawing_pen_int1 = BLOCK_BOUNDRY_1;
unsigned int drawing_pen_int2 = 0;
unsigned short int drawing_destination = DRAWING_TO_BOUNDRY;
unsigned int pen_from_tile_index = 0;
unsigned int pen_of_boundary = 0;

int current_chain=0;  // the number of bears you killed in the latest chain
int chain_award=0;  // the number of bears you killed in the latest chain
int chain_time_start=0;  // the amount of time you stayed in the air during a chain
int chain_time_finish=0;  //
int chain_award_counter=0;  // the counter that tells you wether or not to display the chain award.
int killed_em_all_counter=0;
int finished_round_counter=0;
int total_enemies_killed=0;

#define FINISHED_ROUND_DURATION 500

int clock_time_suspended_at=0;

bool first_time_through_fade_in = true;

string gimpString;

////////////////////
////////////////////
////////////////////






#define STATE_FINISH_ROUND 299999 /* this is depreciated */

#define GAME_STATE_LIST \
ADD_GAME_STATE(STATE_NOT_SET) \
ADD_GAME_STATE(STATE_TITLE_SCREEN) \
ADD_GAME_STATE(STATE_WAITING_FOR_LEVEL_START) \
ADD_GAME_STATE(STATE_IN_LEVEL) \
ADD_GAME_STATE(STATE_PAUSE) \
ADD_GAME_STATE(STATE_DIALOGUE) \
ADD_GAME_STATE(STATE_TIME_UP)
//ADD_GAME_STATE(STATE_CHOOSING_LEVEL)


enum game_state_t
{
#define ADD_GAME_STATE(state_val) state_val,
	GAME_STATE_LIST
#undef ADD_GAME_STATE
};


std::string get_game_state_str(game_state_t type)
{
	switch(type)
	{
	#define ADD_GAME_STATE(state_val) case state_val: return #state_val; break;
		GAME_STATE_LIST
	#undef ADD_GAME_STATE
	};
	return "[STATE_UNDEFINED]";
};






//int _game_state = STATE_TITLE_SCREEN;//STATE_WAITING_FOR_LEVEL_START;
int _game_state = STATE_NOT_SET;//STATE_WAITING_FOR_LEVEL_START;

void set_game_state(int state);

int get_game_state()
{
	return _game_state;
}


int num_of_enemies_left = 0;

int player_score = 0;
//int player_lives = 3;
int player_coins = 0;

//int ellapsed_time = 0;

int time_when_completed = 0;
//int time_left_in_round = 0;
int time_to_kill_all_the_bears=-1;
//int level_timer;

void assign_points(int p)
{
    player_score += p;
}


float point_on_a_line_y(int top_x, int top_y, int bottom_x, int bottom_y, float xx)
{
    return ((((float)(top_y - bottom_y))/((float)(top_x - bottom_x))*(xx-bottom_x))+(bottom_y));
}

////////////////////
////////////////////

//char *level_to_start = "Old_Level_3.lev";
#define DEFAULT_BACKGROUND "hill_3.bmp"

string title_screen_level = "title_screen_level.lv2";
string level_to_start = title_screen_level;//"levels and maps/Old Level 3.lv2";
//char *level_to_start = "Level_2.lev";

////////////////////

int pre_game_counter=0;
int simultanious_kill=0;
int award_counter=200;
int total_points_in_the_fer=0;


//int go_banner_counter=0;



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// HELP MESSAGE
//#include "help_message.hpp"

#define HELP_MESSAGE_DURATION   300


int help_message_counter = 0;
string help_message_text = "";
ALLEGRO_COLOR help_message_color = WHITE;


void start_help_message(string mesg, ALLEGRO_COLOR color=WHITE)
{
    help_message_text = mesg;
    help_message_color = color;
    help_message_counter = HELP_MESSAGE_DURATION;
}    


void draw_help_message()
{
    if (help_message_counter > 0)
    {
        textprintf_centre(buffer, font_small, SCREEN_W/2, SCREEN_H-text_height(font_small)-10, help_message_color, help_message_text.c_str());
    }    
}


void update_help_message()
{
    help_message_counter--;
    if (help_message_counter < 0) help_message_counter = 0;
}    


void clear_help_message()
{
    help_message_text = "";
    help_message_counter = 0;
}        

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////






////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// CHANGE LOCKED DOOR GRAPHICS
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



#define JEWEL_TYPE(xx)    ALLEGRO_BITMAP* xx##_1; ALLEGRO_BITMAP* xx##_2; ALLEGRO_BITMAP* xx##_3;
//#include "jewel_list.hpp"
	JEWEL_LIST
#undef JEWEL_TYPE



void load_jewel_shimmers()
{  
#define JEWEL_TYPE(xx)    xx##_1 = _load_bitmap("images/items/" #xx "_1.gif"); \
                           xx##_2 = _load_bitmap("images/items/" #xx "_2.gif"); \
                           xx##_3 = _load_bitmap("images/items/" #xx "_3.gif");
//#include "jewel_list.hpp"
	JEWEL_LIST
#undef JEWEL_TYPE
}

void destroy_jewel_shimmers()
{
#define JEWEL_TYPE(xx)    al_destroy_bitmap(xx##_1); \
                          al_destroy_bitmap(xx##_2); \
                          al_destroy_bitmap(xx##_3);
//#include "jewel_list.hpp"
	JEWEL_LIST
#undef JEWEL_TYPE
}



bool is_a_jewel(int item_type)
{
#define JEWEL_TYPE(xx)    if (item_type == xx##_const) return true;
//#include "jewel_list.hpp"
	JEWEL_LIST
#undef JEWEL_TYPE
    return false;
}


float jewel_shimmer_x = 0.0f;
float jewel_shimmer_y = 0.0f;
float jewel_padding = 20.0f;
float jewel_shimmer_spacing = SCREEN_W*1.5f;

void update_jewel_shimmer()
{
    jewel_shimmer_spacing = SCREEN_W*1.5f;

    jewel_shimmer_x += (GAMEPLAY_FRAME_DURATION * 3 * 60);
    jewel_shimmer_y += (GAMEPLAY_FRAME_DURATION * 0.6f * 60);

    while (jewel_shimmer_x > jewel_shimmer_spacing) jewel_shimmer_x -= jewel_shimmer_spacing;
    while (jewel_shimmer_y > jewel_shimmer_spacing) jewel_shimmer_y -= jewel_shimmer_spacing;
}

bool within_x_range(float x, float xx1, float xx2, float padding=0.0f)
{
    if (x+padding < xx1) return false;
    if (x-padding > xx2) return false;
    return true;
}


ALLEGRO_BITMAP *get_jewel_image(int item_type, float map_x, float map_y)
{
     float x_spot = map_x;
     float y_spot = map_y;

     while (x_spot > jewel_shimmer_spacing) x_spot -= jewel_shimmer_spacing;
     while (y_spot > jewel_shimmer_spacing) y_spot -= jewel_shimmer_spacing;

     ALLEGRO_BITMAP *b;

     switch (item_type)
     {
#define JEWEL_TYPE(xx)    case xx##_const: \
    if (within_x_range(x_spot, jewel_shimmer_x+jewel_padding*0, jewel_shimmer_x+jewel_padding*1, 0)) b = xx##_1;  \
    else if (within_x_range(x_spot, jewel_shimmer_x+jewel_padding*1, jewel_shimmer_x+jewel_padding*2, 0)) b = xx##_2;  \
    else if (within_x_range(x_spot, jewel_shimmer_x+jewel_padding*2, jewel_shimmer_x+jewel_padding*3, 0)) b = xx##_3;  \
    else b = xx##_img;   \
    break;

//#include "jewel_list.hpp"
	JEWEL_LIST
#undef JEWEL_TYPE
     }

	return b;
}


void draw_jewel_centered(int item_type, float map_x, float map_y)
{
	ALLEGRO_BITMAP *b = get_jewel_image(item_type, map_x, map_y);
	if (b) draw_sprite(buffer, b, (int)map_x-al_get_bitmap_width(b)/2-(int)camera_x, (int)map_y-al_get_bitmap_height(b)/2-(int)camera_y);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool is_trigger_block(int tile_num);
void attempt_to_turn_on_trigger(int trigger_num);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//#include "sprites.hpp"

#define DEFAULT_STRIKE_DISTANCE        30 
#define DEFAULT_MAX_WALK_SPEED         1.5f
//#define DEFAULT_MAX_RUN_SPEED          3.0f
#define DEFAULT_MAX_RUN_SPEED          DEFAULT_MAX_WALK_SPEED
#define DEFAULT_MAX_CLIMB_SPEED        1.0f
//#define DEFAULT_ACCEL_RATE             0.15f
#define DEFAULT_ACCEL_RATE             0.13f
//#define DEFAULT_JUMP_VELOCITY          6.5f
#define DEFAULT_JUMP_VELOCITY          6.0f






class player_damage_zone
{
public:
	int map_num;
	float x, y, w, h;
	float damage;
	int type;
	float lifespan;

	bool collides(int xx, int yy)
	{
		if (xx < x) return false;
		if (yy < y) return false;
		if (xx > x+w) return false;
		if (yy > y+h) return false;
		return true;
	}
	bool collides(int xx1, int yy1, int xx2, int yy2)
	{
		int x1 = x;
		int y1 = y;
		int x2 = x+w;
		int y2 = y+h;

		if (x1 > xx2) return false;
		if (y1 > yy2) return false;
		if (x2 < xx1) return false;
		if (y2 < yy1) return false;
		return true;
	}
};




#define PLAYER_SWIPE 1
#define PLAYER_DOWNTHRUST 2


class DamageZones
{
private:
	static DamageZones *instance;
	std::vector<player_damage_zone> damage_zones;

	DamageZones()
	{
		damage_zones.reserve(100); // just to keep it from rezising or anything
	}

	static DamageZones *get_instance()
	{
		if (!instance) instance = new DamageZones();
		return instance;
	}

public:

	static const std::vector<player_damage_zone> &get_damage_zones()
	{	
		DamageZones *inst = get_instance();
		return inst->damage_zones;
	}
	
	static player_damage_zone *spawn(float x, float y, float w, float h, int map_num, float damage, int type, float lifespan)
	{
		DamageZones *inst = get_instance();

		player_damage_zone dz;
		dz.x = x;
		dz.y = y;
		dz.w = w;
		dz.h = h;
		dz.map_num = map_num;
		dz.damage = damage;
		dz.type = type;
		dz.lifespan = lifespan;

		inst->damage_zones.push_back(dz);
		return &inst->damage_zones.back();
	}

	static void draw()
	{
		DamageZones *inst = get_instance();
		for (unsigned i=0; i<inst->damage_zones.size(); i++)
		{
			al_draw_rectangle(inst->damage_zones[i].x-camera_x,
				inst->damage_zones[i].y-camera_y,
				inst->damage_zones[i].x + inst->damage_zones[i].w - camera_x,
				inst->damage_zones[i].y + inst->damage_zones[i].h - camera_y,
				al_color_name("red"), 1.0);
		}
	}

	static bool kill(player_damage_zone *zone)
	{
		DamageZones *inst = get_instance();
		for (unsigned i=0; i<inst->damage_zones.size(); i++)
			if ((&inst->damage_zones[i]) == zone)
			{
				inst->damage_zones.erase(inst->damage_zones.begin() + i--);
				return true;
			}

		return false;
	}

	static bool dead(player_damage_zone *zone)
	{
		DamageZones *inst = get_instance();
		for (unsigned i=0; i<inst->damage_zones.size(); i++)
			if ((&inst->damage_zones[i]) == zone)
			{
				if (inst->damage_zones[i].lifespan <= 0) return true;
				return false;
			}

		return false;
	}

	static void update()
	{
		DamageZones *inst = get_instance();
		for (unsigned i=0; i<inst->damage_zones.size(); i++)
		{
			inst->damage_zones[i].lifespan -= GAMEPLAY_FRAME_DURATION;

			if ((inst->damage_zones[i].type != PLAYER_SWIPE) && (inst->damage_zones[i].lifespan <= 0.0))
				inst->damage_zones.erase(inst->damage_zones.begin() + i--);
		}
	}

	static bool collides(int map_num, int x, int y, int w, int h, player_damage_zone *collided_zone)
	{
		DamageZones *inst = get_instance();
		for (unsigned i=0; i<inst->damage_zones.size(); i++)
		{
			if (inst->damage_zones[i].map_num != map_num) continue;
			if (inst->damage_zones[i].collides(x, y, x+w, y+h))
			{
				if (collided_zone) *collided_zone = inst->damage_zones[i];
				return true;
			}
		}
		return false;
	}

	static bool collides(int map_num, int x, int y, player_damage_zone *collided_zone)
	{
		DamageZones *inst = get_instance();
		for (unsigned i=0; i<inst->damage_zones.size(); i++)
		{
			if (inst->damage_zones[i].map_num != map_num) continue;
			if (inst->damage_zones[i].collides(x, y))
			{
				if (collided_zone) *collided_zone = inst->damage_zones[i];
				return true;
			}
		}
		return false;
	}

	static void clear()
	{
		DamageZones *inst = get_instance();
		inst->damage_zones.clear();
	}
};

DamageZones *DamageZones::instance = NULL;



//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////




class CameraRegion
{
public:
	std::string label;
	float x1, y1, x2, y2;
	bool player_inside;

	CameraRegion()
		: x1(0)
		, y1(0)
		, x2(0)
		, y2(0)
		, player_inside(false)
	{}
	CameraRegion(float x1, float y1, float x2, float y2)
		: label("")
		, x1(x1)
		, y1(y1)
		, x2(x2)
		, y2(y2)
		, player_inside(false)
	{}
	bool collides(int xx, int yy)
	{
		if (xx < x1) return false;
		if (yy < y1) return false;
		if (yy > y2) return false;
		if (xx > x2) return false;
		return true;
	}
};



class CameraRegions
{
private:
	static std::vector<CameraRegion> camera_regions;
	static CameraRegions *instance;
	static CameraRegions *get_instance()
	{
		if (!instance) instance = new CameraRegions();
		return instance;
	}

	CameraRegion *topmost_collided_region;
	float from_camera_x;
	float from_camera_y;
	float moving_region_counter;

	CameraRegions()
		: topmost_collided_region(NULL)
		, from_camera_x(0)
		, from_camera_y(0)
		, moving_region_counter(0)
	{
	}


public:
	static void clear()
	{
		std::vector<CameraRegion> &camera_regions = get_instance()->camera_regions;
		camera_regions.clear();
		get_instance()->topmost_collided_region = NULL;
	}
/*
	static void load(std::string filename)
	{
		clear();
		append_camera_regions_file(filename);
	}
*/
	static void add_camera_region(float x1, float y1, float x2, float y2)
	{
		CameraRegion new_camera_region;
		new_camera_region.x1 = x1;
		new_camera_region.y1 = y1;
		new_camera_region.x2 = x2;
		new_camera_region.y2 = y2;
		camera_regions.push_back(new_camera_region);
	}
/*
	static bool append_camera_regions_file(std::string filename) // don't use this unless you know what you're doing
	{
		std::vector<CameraRegion> &camera_regions = get_instance()->camera_regions;

		ALLEGRO_CONFIG *config = al_load_config_file(filename.c_str());
		if (!config)
		{
			std::cout << "Could not load camera_regions file \"" << filename << "\"" << std::endl;
			return false;
		}

		ALLEGRO_CONFIG_SECTION *section;
		const char *current_section = al_get_first_config_section(config, &section);

		if (current_section == NULL)
		{
			std::cout << "it appears that the .camera_regions file \"" << filename << "\" is empty; skipping loading." << std::endl;
			return true;
		}

		do
		{
			if (strcmp(current_section, "") == 0)
			{
				continue;
				std::cout << "blank camera_regions section found (skipping section)" << std::endl;
			}

			// new camera region found!
			CameraRegion new_camera_region;
			new_camera_region.label = current_section;

			// just check it manually, k
			{
				float x = 0;
				float y = 0;
				float width = 0;
				float height = 0;
				int num_x_y_width_height_found = 0;

				{
					const char *val = al_get_config_value(config, current_section, "x");
					if (val) { num_x_y_width_height_found++; x = atof_rct(val); }
				}
				{
					const char *val = al_get_config_value(config, current_section, "y");
					if (val) { num_x_y_width_height_found++; y = atof_rct(val); }
				}
				{
					const char *val = al_get_config_value(config, current_section, "width");
					if (val) { num_x_y_width_height_found++; width = atof_rct(val); }
				}
				{
					const char *val = al_get_config_value(config, current_section, "height");
					if (val) { num_x_y_width_height_found++; height = atof_rct(val); }
				}

				if (num_x_y_width_height_found==4)
				{
					std::cout << "alternative (x, y, width, height) key/value pairs found! :D)" << std::endl;
					new_camera_region.x1 = x;
					new_camera_region.y1 = y;
					new_camera_region.x2 = x+width;
					new_camera_region.y2 = y+height;
				}
				else if (num_x_y_width_height_found != 0)
				{
					std::cout << "[CameraRegions::load() error:] incomplete number (" << num_x_y_width_height_found << ") of key/value pairs found for x, y, width, and height." << std::endl;
				}
				else
				{
					//
					// There is no "alternative" (x, y, width, height) key/value pairs.  Now search for (x1, y1, x2, y2) values.
					//

					// this hairy macro just validates that each key/value is accounted for, and outputs a warning if a value was not found.
					// the macro then assigns the value to the destination variable, and allows it to be "post-processed" with a function
					// before assignment, e.g. atoi() or atof(), etc.
					#define get_config_val(config, section, key, post_proc) { const char *r = al_get_config_value(config, section, #key); if (!r) { std::cout << "CameraRegions::load() error: key \"" << #key << "\" not found under section \"" << section << "\"" << std::endl; } else { new_camera_region . key = post_proc (r); } } 

					//new_camera_region.unique_id = new_camera_region::last_unique_id++;
					//get_config_val(config, current_section, world_num, atoi_t);
					//get_config_val(config, current_section, level_num, atoi_t);
					//get_config_val(config, current_section, map_num, atoi_t);
					//get_config_val(config, current_section, type, atoi_t);
					get_config_val(config, current_section, x1, atof_rct);
					get_config_val(config, current_section, y1, atof_rct);
					get_config_val(config, current_section, x2, atof_rct);
					get_config_val(config, current_section, y2, atof_rct);
				}
			}

			#undef get_config_val

			camera_regions.push_back(new_camera_region);
		} while(current_section = al_get_next_config_section(&section));

		std::cout << "camera_regions file loaded successfully. " << camera_regions.size() << " camera_region(s) created." << std::endl;

		al_destroy_config(config);
		return true;
	}
*/
	static bool update(float target_x, float target_y, float *camera_x, float *camera_y, float screen_w, float screen_h)
		// returns true if a new camera region has just been entered
	{
		CameraRegions *inst = CameraRegions::get_instance();
		bool new_topmost_collision_occured = false;


		inst->moving_region_counter -= GAMEPLAY_FRAME_DURATION;
		if (inst->moving_region_counter <= 0) inst->moving_region_counter = 0;

		CameraRegion *topmost_collided_region_now = NULL;


		// center the camera on the target by default
		float destination_camera_x = target_x - screen_w/2;
		float destination_camera_y = target_y - screen_h/2;


		// find out which camera regions have been entered or exited in this frame
		for (CameraRegion &region : inst->camera_regions)
		{
			bool player_collides_now = region.collides(target_x, target_y);
			bool player_collided_prior = region.player_inside;

			// locate the top-most camera region
			// elements are assumed to be sorted in z-order
			if (player_collides_now) topmost_collided_region_now = &region;

			if (player_collides_now && !player_collided_prior)
			{
				// this camera region has /just/ been entered
			}
			else if (player_collided_prior && !player_collides_now)
			{
				// this camera region has /just/ been exited
			}
		}

		// a new topmost camera region was /just/ entered
		if (inst->topmost_collided_region != topmost_collided_region_now)
		{
			new_topmost_collision_occured = true;
			inst->topmost_collided_region = topmost_collided_region_now;
			inst->moving_region_counter = 1.0;
			inst->from_camera_x = *camera_x;
			inst->from_camera_y = *camera_y;
		}

		// this will hard limit the camera position to the edges of the topmost camera region
		if (topmost_collided_region_now)
		{
			CameraRegion &region = *topmost_collided_region_now;

			// limit the bounds of the camera's position
			if (destination_camera_x < region.x1) destination_camera_x = region.x1;
			if (destination_camera_y < region.y1) destination_camera_y = region.y1;
			if (destination_camera_x+screen_w > region.x2) destination_camera_x = region.x2 - screen_w;
			if (destination_camera_y+screen_h > region.y2) destination_camera_y = region.y2 - screen_h;
		}

		// calculate and set the position of the camera (between the two target points)
		float vcounter = interpolator::doubleFastOut(inst->moving_region_counter);
		float invvcounter = 1.0 - vcounter;
		*camera_x = inst->from_camera_x * vcounter + destination_camera_x * invvcounter;
		*camera_y = inst->from_camera_y * vcounter + destination_camera_y * invvcounter;

		return new_topmost_collision_occured;
	}
	static bool get_topmost_collided_region(CameraRegion *region)
	{
		CameraRegions *inst = CameraRegions::get_instance();

		region = NULL;
		if (!inst->topmost_collided_region) return false;

		*region = *inst->topmost_collided_region;
		return true;
	}
	static void draw(float camera_x, float camera_y)
	{
		return;
		std::vector<CameraRegion> &camera_regions = get_instance()->camera_regions;
		for (unsigned i=0; i<camera_regions.size(); i++)
		{
			CameraRegion &region = camera_regions[i];
			al_draw_rounded_rectangle(region.x1 - camera_x, region.y1 - camera_y,
				region.x2 - camera_x, region.y2 - camera_y, 5, 5, al_color_name("black"), 3.0);
		}
	}
};

CameraRegions *CameraRegions::instance = NULL;
std::vector<CameraRegion> CameraRegions::camera_regions;



//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////




#define ENEMY_LIST \
PROC_ENEMY(ENEMY_TYPE_UNDEF, undef) \
PROC_ENEMY(ENEMY_TYPE_YELLOW_BEAR, yellow) \
PROC_ENEMY(ENEMY_TYPE_BLUE_BEAR, blue) \
PROC_ENEMY(ENEMY_TYPE_RED_BEAR, red) \
PROC_ENEMY(ENEMY_TYPE_PURPLE_BEAR, purple)



enum enemy_t
{
#define PROC_ENEMY(x, y) x,
	ENEMY_LIST
#undef PROC_ENEMY
};



class EnemyBase;
void update_enemy_position(EnemyBase *enemy_ptr);


class EnemyBase
{
public:
	int unique_id;
	static int last_unique_id;

	int world_num;
	int level_num;
	int map_num;
	int origin_x, origin_y;

	int type;
	float x, y;
	int w, h;
	float velocity_x, velocity_y;
	bool facing_left;
	int health, max_health;
	float post_pain_counter;

	int touch_damage;

	ALLEGRO_BITMAP *current_bitmap;
	float bitmap_origin_alignment_x, bitmap_origin_alignment_y;
	bool bitmap_flip_h, bitmap_flip_v;

	bool collides_with_tile_boundaries;
	bool respawns_offscreen;
	bool on_ground;
	bool fazed_from_damage;

	EnemyBase(int world_num, int level_num, int map_num, int origin_x, int origin_y)
		: unique_id(last_unique_id++)
		, world_num(world_num)
		, level_num(level_num)
		, map_num(map_num)
		, origin_x(origin_x)
		, origin_y(origin_y)
		, type(ENEMY_TYPE_UNDEF)
		, x(origin_x)
		, y(origin_y)
		, facing_left(true)
		, w(16)
		, h(32)
		, velocity_x(0)
		, velocity_y(0)
		, health(1)
		, max_health(1)
		, post_pain_counter(0)
		, touch_damage(1)
		, current_bitmap(NULL)
		, bitmap_origin_alignment_x(0.5)
		, bitmap_origin_alignment_y(1.0)
		, bitmap_flip_h(true)
		, bitmap_flip_v(false)
		, collides_with_tile_boundaries(true)
		, respawns_offscreen(false)
		, on_ground(false)
		, fazed_from_damage(false)
	{}

	// state and drawing
	bool is_dead()
	{
		return (health <= 0);
	}
	bool collides(int x, int y)
	{
		if (x < this->x) return false;
		if (y < this->y) return false;
		if (x > (this->x+this->w)) return false;
		if (y > (this->y+this->h)) return false;
		return true;
	}
	bool collides(int x, int y, int w, int h)
	{
		if ((x+w) < this->x) return false;
		if ((y+h) < this->y) return false;
		if (x > (this->x+this->w)) return false;
		if (y > (this->y+this->h)) return false;
		return true;
	}
	void update()
	{
		// update the post-pain counter
		if (post_pain_counter > 0.0)
		{
			post_pain_counter -= GAMEPLAY_FRAME_DURATION;
			if (post_pain_counter <= 0)
			{
				post_pain_counter = 0;
				fazed_from_damage = false;
				on_recovery();
			}
		}

		// check for damage from attacks
		player_damage_zone collided_zone;
		if (DamageZones::collides(map_num, x, y, w, h, &collided_zone))
			on_collides_with_damage_zone(collided_zone);

		update_physics();

		// fire an on_update event
		on_update();
	}
	virtual void update_physics()
	{
		// update dynamic movement
		// these lines might need to be placed somewhere else
		velocity_y += GRAVITY * GAMEPLAY_FRAME_DURATION_PER_SEC;

		// update the position (this is the complex part that interacts with the map)
		// TODO: this function only test collisions on the current map
		update_enemy_position(this);
	}
	virtual void draw()
	{
		ALLEGRO_COLOR color = al_color_name("white");
		if (fazed_from_damage) color = ((fmod(post_pain_counter, 0.1) < 0.05) ? al_color_name("red") : al_color_name("yellow"));
		if (is_dead())
		{
			color = al_color_name("gray");
			return;
		}

		if (current_bitmap)
		{
			int flip_flags = 0;
			if (bitmap_flip_h) flip_flags = flip_flags | ALLEGRO_FLIP_HORIZONTAL;
			if (bitmap_flip_v) flip_flags = flip_flags | ALLEGRO_FLIP_VERTICAL;
			al_draw_tinted_bitmap(current_bitmap, color, (int)(x + w*bitmap_origin_alignment_x - camera_x - al_get_bitmap_width(current_bitmap)*bitmap_origin_alignment_x),
				(int)(y + h*bitmap_origin_alignment_y - camera_y - al_get_bitmap_height(current_bitmap)*bitmap_origin_alignment_y)+1, flip_flags);
		}

		if (F_debug_mode)
		{
			al_draw_rectangle(x - camera_x, y - camera_y, x+w - camera_x, y+h - camera_y, color, 2.0);
			al_draw_text(font_x_small, color, x+w/2 - camera_x, y+h/2 - camera_y, ALLEGRO_ALIGN_CENTRE, (facing_left ? "<<" : ">>"));
		}
	}

	// events
	virtual void on_update() {}
	virtual void on_reposition() {}
	virtual void on_spawn() {}
	virtual void on_bottom_collides_with_ground_tiles(std::vector<int2> tiles) {}
	virtual void on_collides_with_ceiling_tile(int tile_x, int tile_y) {}
	virtual void on_collides_with_wall_tile(int tile_x, int tile_y) {}
	virtual void on_about_to_walk_off_ground() {}
	virtual void on_collides_with_damage_zone(const player_damage_zone &damage_zone)
	{
		if (fazed_from_damage || is_dead()) return;

		if ((health-damage_zone.damage) <= 0)
		{
			health = 0;
			on_death();
		}
		else
		{
			health -= damage_zone.damage;

			fazed_from_damage = true;
			post_pain_counter = 1;
		  	on_damage();
		}
	}
	virtual void on_collides_with_enemy() {}
	virtual void on_collides_with_player() {}
	virtual void on_damage() {}
	virtual void on_recovery() {}
	virtual void on_death()
	{
		VisualFX::spawn(x+w/2, y+h/2, 0.5, 0.5, "explosion");
	//static bool spawn_item(int world_num, int level_num, int map_num, float x, float y, float align_x, float align_y, item_t type, float velocity_x, float velocity_y)
		Items::spawn_item(get_current_world_index(), get_current_level_index(), get_current_map_index(),
			x+w/2, y+h/2, 0.5, 0.5, random<item_t>({heart_const, small_jewel_const, medium_jewel_const}),
			//x+w/2, y+h/2, 0.5, 0.5, heart_const,
			1.5, -4.0
			);
		play_enemy_dead_sound();
	}
	virtual void on_enters_screen() {}
	virtual void on_leaves_screen() {}
	virtual void on_target_locked() {}
	virtual void on_target_lost() {}
};

int EnemyBase::last_unique_id = 1;





class MedusaHeadEnemy : public EnemyBase
{
public:
	float movement_counter;
	MedusaHeadEnemy(int world_num, int level_num, int map_num, int origin_x, int origin_y, bool facing_left=true)
		: EnemyBase(world_num, level_num, map_num, origin_x, origin_y)
		, movement_counter(0)
	{
		max_health = 3;
		health = 3;

		w = 16;
		h = 16;

		bitmap_flip_h = facing_left;
		current_bitmap = medusa_head_bmp;
		bitmap_origin_alignment_x = 0.5;
		bitmap_origin_alignment_y = 0.5;
	}
	void update_physics() override
	{
		// physics do not apply to this enemy
		// so overriding this default
	}
	void on_reposition() override
	{
		if (fazed_from_damage) return;

		if (bitmap_flip_h) x -= (GAMEPLAY_FRAME_DURATION_PER_SEC * 1);
		else x += (GAMEPLAY_FRAME_DURATION_PER_SEC * 1);

		y = sin(movement_counter * 2) * 50 + origin_y;
	}
	void on_update() override
	{
		if (!fazed_from_damage) movement_counter += GAMEPLAY_FRAME_DURATION;

		if (x < camera_x) x = camera_x + (SCREEN_W + 50);
		on_reposition();
	}
};



class YellowBearEnemy : public EnemyBase
{
private:
	float default_velocity;
	float animation_counter;
	static ALLEGRO_BITMAP *frame1;
	static ALLEGRO_BITMAP *frame2;
	static ALLEGRO_BITMAP *frame3;
	static ALLEGRO_BITMAP *frame4;

public:
	YellowBearEnemy(int world_num, int level_num, int map_num, int origin_x, int origin_y, bool facing_left=true)
		: EnemyBase(world_num, level_num, map_num, origin_x, origin_y)
		, default_velocity(0.5)
		, animation_counter(random_float(0, 1))
	{
		if (!frame1 && !frame2 && !frame3)
		{
			frame1 = al_load_bitmap("images/enemies/yellow_bear/yellow_bear_walk_cycle00.png");
			frame2 = al_load_bitmap("images/enemies/yellow_bear/yellow_bear_walk_cycle01.png");
			frame3 = al_load_bitmap("images/enemies/yellow_bear/yellow_bear_walk_cycle02.png");
			frame4 = al_load_bitmap("images/enemies/yellow_bear/yellow_bear_walk_cycle03.png");
		}

		h = 29;
		current_bitmap = frame1;
		origin_y -= h;

		if (facing_left)
		{
			bitmap_flip_h = true;
			velocity_x = -default_velocity;
		}
		else
		{
			bitmap_flip_h = false;
			velocity_x = default_velocity;
		}
	}
	void turn_around()
	{
		facing_left = !facing_left;
		bitmap_flip_h = !bitmap_flip_h;

		if (facing_left) velocity_x = -default_velocity;
		else velocity_x = default_velocity;
	}
	void on_bottom_collides_with_ground_tiles(std::vector<int2> tiles) override
	{
		if (tiles.empty()) return;

		float tile_top_edge = block_top_edge(tiles[0].y);
		y = tile_top_edge - h - 0.0001;
		velocity_y = 0.0f;
	
		if (!on_ground)
		{
			on_ground = true;

			for (int i=0; i<tiles.size(); i++)
			{
				VisualFX::spawn(tiles[i].x*16+8, tiles[i].y*16, 0.5, 1.0, "sparkflare");
				//VisualFX::spawn(tiles[i].x*16+8, tiles[i].y*16+8, 0.5, 0.5, "explosion");
			}
		}
	}
	void on_update() override
	{
		animation_counter -= GAMEPLAY_FRAME_DURATION;
		while (animation_counter < 0) animation_counter += 1.0;

		float playhead = 1.0 - animation_counter;

		if (playhead < 0.25) current_bitmap = frame1;
		else if (playhead < 0.5) current_bitmap = frame2;
		else if (playhead < 0.75) current_bitmap = frame3;
		else current_bitmap = frame4;

		//if (on_ground && distance(player.map_pos.x+player.size.y/2, player.map_pos.y+player.size.x/2, x+width/2, y+height/2) < 48)
		if (on_ground && distance(get_player_center(), get_player_middle(), x+w/2, y+h/2) < 48)
		{
			// jump!
			velocity_y -= 8.0;
			on_ground = false;
		}
	}
	void on_collides_with_ceiling_tile(int tile_x, int tile_y) override
	{
	}
	void on_collides_with_wall_tile(int tile_x, int tile_y) override
	{
		turn_around();
	}
	void on_about_to_walk_off_ground() override
	{
		turn_around();
	}
};

ALLEGRO_BITMAP *YellowBearEnemy::frame1 = NULL;
ALLEGRO_BITMAP *YellowBearEnemy::frame2 = NULL;
ALLEGRO_BITMAP *YellowBearEnemy::frame3 = NULL;
ALLEGRO_BITMAP *YellowBearEnemy::frame4 = NULL;




class EnemySpawner
{
public:
	int world_num;
	int level_num;
	int map_num;

	float x;
	float y;
	float w;
	float h;
	std::string identifier;

	bool player_over;

	int spawns_left;

	EnemySpawner(int world_num, int level_num, int map_num, float x, float y, float w, float h, std::string identifier)
		: world_num(world_num)
		, level_num(level_num)
		, map_num(map_num)
		, x(x)
		, y(y)
		, w(w)
		, h(h)
		, identifier(identifier)
		, player_over(false)
		, spawns_left(1)
	{}
};




class Enemies
{
private:
	static Enemies *instance;
	std::vector<EnemySpawner> enemy_spawners;
	std::vector<EnemyBase *> enemies;
	static Enemies *get_instance()
	{
		if (!instance) instance = new Enemies();
		return instance;
	}
public:
	static void clear()
	{
		std::vector<EnemyBase *> &enemies = Enemies::get_instance()->enemies;
		for (unsigned i=0; i<enemies.size(); i++) delete enemies[i];
		enemies.clear();
	}
	static void add_enemy_spawner(int world_num, int level_num, int map_num, int origin_x, int origin_y, int origin_w, int origin_h, std::string identifier)
	{
		std::vector<EnemySpawner> &enemy_spawners = Enemies::get_instance()->enemy_spawners;

		enemy_spawners.push_back(EnemySpawner(world_num, level_num, map_num, origin_x, origin_y, origin_w, origin_h, identifier));
	}
	static EnemyBase *add_enemy(int world_num, int level_num, int map_num, int origin_x, int origin_y, std::string identifier)
	{
		std::vector<EnemyBase *> &enemies = Enemies::get_instance()->enemies;
		EnemyBase *new_enemy = NULL;

		if (identifier == "yellow_bear")
		{
			new_enemy = new YellowBearEnemy(world_num, level_num, map_num, origin_x, origin_y);
			enemies.push_back(new_enemy);
		}
		else if (identifier == "medusa_head")
		{
			new_enemy = new MedusaHeadEnemy(world_num, level_num, map_num, origin_x, origin_y);
			enemies.push_back(new_enemy);
		}
		else
		{
			std::cout << "[Enemies::add_enemy() error] enemy type \"" << identifier << "\" not found." << std::endl;
		}

		return new_enemy;
	}
	static void draw(int world_num, int level_num, int map_num)
	{
		std::vector<EnemyBase *> &enemies = Enemies::get_instance()->enemies;
		for (unsigned i=0; i<enemies.size(); i++)
		{
			if (enemies[i]->world_num != world_num) continue;
			if (enemies[i]->level_num != level_num) continue;
			if (enemies[i]->map_num != map_num) continue;

			enemies[i]->draw();
		}
	}
	static bool collides(int world_num, int level_num, int map_num, int x, int y, int w, int h, EnemyBase **collided_enemy)
	{
		std::vector<EnemyBase *> &enemies = Enemies::get_instance()->enemies;
		for (unsigned i=0; i<enemies.size(); i++)
		{
			if (enemies[i]->world_num != world_num) continue;
			if (enemies[i]->level_num != level_num) continue;
			if (enemies[i]->map_num != map_num) continue;

			if (enemies[i]->collides(x, y, w, h))
			{
				*collided_enemy = enemies[i];
				return true;
			}
		}
		return false;
	}
	static bool collides(int world_num, int level_num, int map_num, int x, int y, EnemyBase **collided_enemy)
	{
		return collides(world_num, level_num, map_num, x, y, 0, 0, collided_enemy);
	}
	static void update_enemy_spawners(int world_num, int level_num, int map_num, float player_x, float player_y)
	{
		// update the enemy spawners

		std::vector<EnemySpawner> &enemy_spawners = Enemies::get_instance()->enemy_spawners;
		float spawner_range = 16*9;

		for(EnemySpawner &e : enemy_spawners)
		{
			if (e.spawns_left <= 0) continue;

			bool over_before = e.player_over;
			bool over_now = (distance(player_x, player_y, e.x+e.w/2, e.y+e.h/2) < (spawner_range));

			if (over_now && !over_before)
			{
				// just entered
				e.player_over = true;
				add_enemy(e.world_num, e.level_num, e.map_num, e.x, e.y, e.identifier);
				e.spawns_left--;
				VisualFX::spawn(e.x, e.y, 0.5, 0.5, "explosion");
			}
			else if (over_before && !over_now)
			{
				// lust left
				e.player_over = false;
			}
		};
	}
	static void update(int world_num, int level_num, int map_num)
	{
		// update the existing enemies

		std::vector<EnemyBase *> &enemies = Enemies::get_instance()->enemies;
		player_damage_zone collided_zone;
		for (unsigned i=0; i<enemies.size(); i++)
		{
			if (enemies[i]->is_dead()) continue;

			if (enemies[i]->world_num != world_num) continue;
			if (enemies[i]->level_num != level_num) continue;
			if (enemies[i]->map_num != map_num) continue;

			enemies[i]->update();
			
			// for now:
			//enemies[i]->on_reposition();
		}
	}
};

Enemies *Enemies::instance = NULL;



#include <algorithm>

bool is_top_edge_block(int type);
bool is_bottom_edge_block(int type);
bool is_left_edge_block(int type);
bool is_right_edge_block(int type);


class TileAndObjectCollision
{
public:
	float x;
	float y;
	float w;
	float h;
	float velocity_x;
	float velocity_y;

	bool collides(const TileAndObjectCollision &other)
	{
		if (other.x > (x+w)) return false;
		if (other.y > (y+h)) return false;
		if (other.x+other.w < x) return false;
		if (other.y+other.h < y) return false;
		return true;
	}
	float get_left() { return x; }
	float get_right() { return x+w; }
	float get_bottom() { return y+h; }
	float get_top() { return y; }

	void set_left(float x) { this->x = x; }
	void set_right(float x) { this->x = x - w; }
	void set_bottom(float y) { this->y = y - h; }
	void set_top(float y) { this->y = y; }

	static bool on_enter_new_tiles_moving_downward(TileAndObjectCollision &obj, std::vector<int2> tiles)
	{
		bool collided_at_surface_edge = false;
		for (int2 &t : tiles)
		{
			if (is_top_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				obj.velocity_y = 0;
				obj.set_bottom(block_top_edge(t.y) - 0.0001);
				collided_at_surface_edge = true;
			}
		}
		return collided_at_surface_edge;
	}

	static bool on_enter_new_tiles_moving_upward(TileAndObjectCollision &obj, std::vector<int2> tiles)
	{
		bool collided_at_surface_edge = false;
		for (int2 &t : tiles)
		{
			if (is_bottom_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				obj.velocity_y = 0;
				obj.set_top(block_bottom_edge(t.y) + 0.0001);
				collided_at_surface_edge = true;
			}
		}
		return collided_at_surface_edge;
	}

	static bool on_enter_new_tiles_moving_right(TileAndObjectCollision &obj, std::vector<int2> tiles)
	{
		bool collided_at_surface_edge = false;
		for (int2 &t : tiles)
		{
			if (is_left_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				obj.velocity_x = 0;
				obj.set_right(block_left_edge(t.x) - 0.0001);
				collided_at_surface_edge = true;
			}
		}
		return collided_at_surface_edge;
	}

	static bool on_enter_new_tiles_moving_left(TileAndObjectCollision &obj, std::vector<int2> tiles)
	{
		bool collided_at_surface_edge = false;
		for (int2 &t : tiles)
		{
			if (is_right_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				obj.velocity_x = 0;
				obj.set_left(block_right_edge(t.x) + 0.0001);
				collided_at_surface_edge = true;
			}
		}
		return collided_at_surface_edge;
	}


public:

	enum AXIS
	{
		X_AXIS,
		Y_AXIS,
	};

	static std::vector<int2> test_tile_map_collision_axis(AXIS axis, TileAndObjectCollision element, float timestep = GAMEPLAY_FRAME_DURATION_PER_SEC)
	{
		float velocity_amt = element.velocity_x * timestep;
		float now_x = element.x;
		float next_x = element.x + velocity_amt;
		float next_y = element.y;
		float size_of_body = element.w;
		float length_of_edge = element.h;
		float cursor_step_amt = 16;






		if (axis == Y_AXIS)
		{
			velocity_amt = element.velocity_y * timestep;
			now_x = element.y;
			next_x = element.y + velocity_amt;
			next_y = element.x;
			size_of_body = element.h;
			length_of_edge = element.w;
		}







		// if moving right, test along the right edge
		if (velocity_amt > 0.0) // moving right
		{
			now_x += size_of_body;
			next_x += size_of_body;
		}

		int column_x_now = now_x / 16;
		int column_x_next = next_x / 16;
		std::vector<int2> collided_tiles;

		if (column_x_now != column_x_next)
		{

			//std::cout << "PASS" << std::endl;
			// the player has passed from one row of tiles into the next
			// collect the tiles along the "next" left edge
			int num_passes = ceil(length_of_edge / 16);
			float step_cursor = 0;

			collided_tiles.reserve(num_passes);
			for (unsigned i=0; i<num_passes; i++)
			{
				if (i==(num_passes-1)) step_cursor = length_of_edge;

				if (axis == X_AXIS) collided_tiles.push_back(world_to_tile_coordinates(next_x, next_y + step_cursor));
				else if (axis == Y_AXIS) collided_tiles.push_back(world_to_tile_coordinates(next_y + step_cursor, next_x));

				step_cursor += cursor_step_amt;
			}
		}

		return collided_tiles;
	}
/*
	static std::vector<int2> test_tile_map_collision_horizontal(TileAndObjectCollision element, float timestep = GAMEPLAY_FRAME_DURATION_PER_SEC)
	{
		float now_x = element.x;
		float next_x = element.x + element.velocity_x * timestep;

		// if moving right, test along the right edge
		if (element.velocity > 0.0) // moving right
		{
			now_x += element.w;
			next_x += element.w;
		}

		int column_x_now = now_x / 16;
		int column_x_next = next_x / 16;

		if (column_x_now != column_x_next)
		{
			// the player has passed from one row of tiles into the next
			// collect the tiles along the "next" left edge
			int num_passes = ceil(element.h / 16);
			float step_cursor = 0;

			collided_tiles.reserve(num_passes);
			for (unsigned i=0; i<num_passes; i++)
			{
				if (i==num_passes) step_cursor = element.h;
				collided_tiles.push_back(world_to_tile_coordinates(next_x + step_cursor, next_y));
				step_cursor += 16;
			}
		}

		return collided_tiles;
	}
*/

	// new map collision detections
	static void test_enemy_map_collision_vertical(EnemyBase *enemy_ptr, float timestep = GAMEPLAY_FRAME_DURATION_PER_SEC)
	{
		if (enemy_ptr == NULL) return;

		TileAndObjectCollision element;	

		element.x = enemy_ptr->x;
		element.y = enemy_ptr->y;
		element.w = enemy_ptr->w;
		element.h = enemy_ptr->h;
		element.velocity_x = enemy_ptr->velocity_x;
		element.velocity_y = enemy_ptr->velocity_y;

		float step_velocity_x = element.velocity_x*timestep;
		float step_velocity_y = element.velocity_y*timestep;
		bool lands_on_ground = false;
		//float step_velocity_y = enemy_ptr.velocity_y*timestep;
		//float step_velocity_y = enemy_ptr.velocity_y*timestep;


		if (step_velocity_y > 0.0f) // falling
		{
			float dur = 0; // eeks, potential floating point errors (I am trusting in you computer to not fail me...)
			std::vector<int2> collided_tiles;

			float now_x = element.x;
			float now_y = element.y + element.h; // + 1 puts it on the bottom edge
			float next_x = now_x;
			float next_y = now_y + step_velocity_y;

			int num_passes = ceil(element.w / 16.0);

			for (int i=0; i<=num_passes; i++)
			{
				bool char_collides_next = false;
				bool char_collides_now = false;

				if (is_top_edge_block(block_type_at(now_x + dur, now_y)))
					char_collides_now = true;

				if (is_top_edge_block(block_type_at(next_x + dur, next_y)))
					char_collides_next = true;

				if (!char_collides_now && char_collides_next)
				{
					// the player has just "entered" into this tile
					int2 collided_tile = world_to_tile_coordinates(next_x + dur, next_y);
					collided_tiles.push_back(collided_tile);
				}

				dur += 16;
				dur = std::min(dur, element.w);
			};

			// calculate what to do if there was a new collision
			//enemy.on_bottom_collides_with_ground_tiles(collided_tiles);
			lands_on_ground = on_enter_new_tiles_moving_downward(element, collided_tiles);

		}
		else if (step_velocity_y < 0.0f) // jumping
		{
			//int dur=-1;
			float dur = 0.0;
			std::vector<int2> collided_tiles;

			float now_x = element.x;
			float now_y = element.y; // + 1 puts it on the bottom edge
			float next_x = now_x;
			float next_y = now_y + step_velocity_y;

			int num_passes = ceil(element.w / 16.0);
			bool collides = false;

			for (int i=0; i<=num_passes; i++)
			{
				if (is_bottom_edge_block(block_type_at(next_x + dur, next_y)))
				 {
					int2 collided_tile = world_to_tile_coordinates(next_x + dur, next_y);
					collided_tiles.push_back(collided_tile);
				 }

				dur += 16;
				dur = std::min(dur, element.w);
			}

			//if (collides)
			on_enter_new_tiles_moving_upward(element, collided_tiles);
		}


		/*
		// here just do the assignment for now,
		// in theory, this function will eventually act directly on the object
		*/
		enemy_ptr->x = element.x;
		enemy_ptr->y = element.y;
		enemy_ptr->w = element.w;
		enemy_ptr->h = element.h;
		enemy_ptr->velocity_x = element.velocity_x;
		enemy_ptr->velocity_y = element.velocity_y;
		if (lands_on_ground && !enemy_ptr->on_ground) enemy_ptr->on_ground = true;
	}




	// new map collision detections
	static void test_enemy_map_collision_horizontal(EnemyBase *enemy_ptr, float timestep)
	{
		if (enemy_ptr == NULL) return;

		bool collided_at_wall = false;

		TileAndObjectCollision element;	

		element.x = enemy_ptr->x;
		element.y = enemy_ptr->y;
		element.w = enemy_ptr->w;
		element.h = enemy_ptr->h;
		element.velocity_x = enemy_ptr->velocity_x;
		element.velocity_y = enemy_ptr->velocity_y;

		float step_velocity_x = element.velocity_x*timestep;
		float step_velocity_y = element.velocity_y*timestep;




		if (step_velocity_x > 0.0f) // moving right
		{
			float dur = 0; // eeks, potential floating point errors (I am trusting in you computer to not fail me...)
			std::vector<int2> collided_tiles;

			float now_x = element.x + element.w;
			float now_y = element.y; // + 1 puts it on the bottom edge
			float next_x = now_x + step_velocity_x;
			float next_y = now_y;

			int num_passes = ceil(element.h / 16.0);

			for (int i=0; i<=num_passes; i++)
			{
				bool char_collides_next = false;
				bool char_collides_now = false;

				if (is_left_edge_block(block_type_at(now_x, now_y + dur)))
					char_collides_now = true;

				if (is_left_edge_block(block_type_at(next_x, next_y + dur)))
					char_collides_next = true;

				if (!char_collides_now && char_collides_next)
				{
					// the player has just "entered" into this tile
					int2 collided_tile = world_to_tile_coordinates(next_x, next_y + dur);
					collided_tiles.push_back(collided_tile);
				}

				dur += 16;
				dur = std::min(dur, element.h);
			};

			// calculate what to do if there was a new collision
			//enemy.on_bottom_collides_with_ground_tiles(collided_tiles);
			//lands_on_ground = on_enter_new_tiles_moving_downward(element, collided_tiles);
			collided_at_wall = on_enter_new_tiles_moving_right(element, collided_tiles);

		}
		else if (step_velocity_x < 0.0f) // moving left
		{
			//int dur=-1;
			float dur = 0.0;
			std::vector<int2> collided_tiles;

			float now_x = element.x;
			float now_y = element.y;
			float next_x = now_x + step_velocity_x;
			float next_y = now_y;

			int num_passes = ceil(element.h / 16.0);
			bool collides = false;

			for (int i=0; i<=num_passes; i++)
			{
				if (is_right_edge_block(block_type_at(next_x, next_y + dur)))
				 {
					int2 collided_tile = world_to_tile_coordinates(next_x, next_y + dur);
					collided_tiles.push_back(collided_tile);
				 }

				dur += 16;
				dur = std::min(dur, element.h);
			}

			//if (collides)
			collided_at_wall = (on_enter_new_tiles_moving_left(element, collided_tiles) || collided_at_wall);
			//if (collided) enemy.turn_around(); 
		}


		/*
		// here just do the assignment for now,
		// in theory, this function will eventually act directly on the object
		*/
		enemy_ptr->x = element.x;
		enemy_ptr->y = element.y;
		enemy_ptr->w = element.w;
		enemy_ptr->h = element.h;
		enemy_ptr->velocity_x = element.velocity_x;
		enemy_ptr->velocity_y = element.velocity_y;
		if (collided_at_wall)
		{
			//enemy_ptr->turn_around();
			enemy_ptr->facing_left = !enemy_ptr->facing_left;
			enemy_ptr->bitmap_flip_h = !enemy_ptr->bitmap_flip_h;

			if (enemy_ptr->facing_left) enemy_ptr->velocity_x = -0.5;
			else enemy_ptr->velocity_x = 0.5;
			//if (enemy_ptr->facing_left) enemy_ptr->velocity_x = -enemy_ptr->default_velocity;
			//else enemy_ptr->velocity_x = enemy_ptr->default_velocity;
		}
	}
};




// new map collision detections
void test_enemy_map_collision_vertical(EnemyBase *enemy_ptr, float timestep = GAMEPLAY_FRAME_DURATION_PER_SEC)
{
	if (enemy_ptr == NULL) return;

	EnemyBase &enemy = *enemy_ptr;
	float element_x = enemy.x;
	float element_y = enemy.y;
	float element_w = enemy.w;
	float element_h = enemy.h;
	float step_velocity_x = enemy.velocity_x*timestep;
	float step_velocity_y = enemy.velocity_y*timestep;


	if (step_velocity_y > 0.0f) // falling
	{
		bool char_collides_next = false;
		bool char_collides_now = false;
		int dur=-1;
		while(dur<(int)element_w+1)
		{
			 if (is_platform_block(block_type_at((int)(element_x)+dur, (int)(element_y+step_velocity_y)+(int)element_h+1)))
			 {
				  char_collides_next = true;
			 }
			 if (is_platform_block(block_type_at((int)(element_x)+dur, (int)(element_y)+(int)element_h)))
			 {
				  char_collides_now = true;
			 }
			 dur++;
		}
		if ((!char_collides_now) && (char_collides_next))
		{
			 enemy.velocity_y = 0.0f;
			 enemy.y = block_top_edge(((int)(element_y+step_velocity_y+8)+(int)element_h+1)/16) - (int)(element_h) - 1;
			 //if (pic == jump_pic) pic = gimp_pic;
			 enemy.on_ground = true;
			 enemy.on_bottom_collides_with_ground_tiles({int2(0, 0)} /* do this part today */);
		}
	}
	else if (step_velocity_y < 0.0f) // jumping
	{
		int dur=-1;
		bool collides = false;
		while(dur<(int)element_w+1)
		{
			 if (is_ceiling_block(block_type_at((int)(element_x)+dur, (int)(element_y+step_velocity_y)-1)))
			 {
				  dur = (int)element_w+1;
				  enemy.velocity_y = 0.0f;
				  enemy.y = block_bottom_edge(((int)(element_y+step_velocity_y-8)-1)/16) + 1;
					collides = true;
			 }
			 else dur++;
		}

		if (collides) enemy.on_collides_with_ceiling_tile(0, 0 /* do this part today */);
	}
}


// new map collision detections
void test_enemy_map_collision_horizontal(EnemyBase *enemy_ptr, float timestep)
{
	if (enemy_ptr == NULL) return;

	EnemyBase &enemy = *enemy_ptr;
	float element_x = enemy.x;
	float element_y = enemy.y;
	float element_w = enemy.w;
	float element_h = enemy.h;
	float step_velocity_x = enemy.velocity_x * GAMEPLAY_FRAME_DURATION_PER_SEC;
	float setp_velocity_y = enemy.velocity_y * GAMEPLAY_FRAME_DURATION_PER_SEC;


	if (step_velocity_x > 0.0f)
	{
		bool collided = false;
		int y_step = 16;
		int y_step_cursor=0;
		bool last_check = false;


		while(y_step_cursor<(int)element_h+1)
		{
			 if (is_right_wall_block(block_type_at((int)(element_x+step_velocity_x)+(int)element_w+1, (int)(element_y)+y_step_cursor)))
			 {
				  y_step_cursor = (int)element_h+1;
				  enemy.velocity_x = 0.0f;
				  enemy.x = block_left_edge(((int)(element_x+step_velocity_x+8)+(int)element_w+1)/16) - ((int)element_w) - 1;
					collided = true;
			 }
			 else y_step_cursor+=16;
			 if (y_step_cursor > (int)element_h)
			 {
				  if (is_right_wall_block(block_type_at((int)(element_x+step_velocity_x)+(int)element_w+1, (int)(element_y)+(int)element_h)))
				  {
						y_step_cursor = (int)element_h+1;
						enemy.velocity_x = 0.0f;
						enemy.x = block_left_edge(((int)(element_x+step_velocity_x+8)+(int)element_w+1)/16) - ((int)element_w) - 1;
						collided = true;
				  }
			 }
		}

		if (collided) enemy.on_collides_with_wall_tile(0, 0 /* do this part today */);
	}
	else if (step_velocity_x < 0.0f)
	{
		bool collided = false;
		int dur=0;
		while(dur<(int)element_h+1)
		{
			 if (is_left_wall_block(block_type_at((int)(element_x+step_velocity_x)-1, (int)(element_y)+dur)))
			 {
				  dur = (int)element_h+1;
				  enemy.velocity_x = 0.0f;
				  enemy.x = block_right_edge(((int)(element_x+step_velocity_x-8)-1)/16) + 1;
					collided = true;
				  //turn_around();
			 }
			 else dur+=16;
			 if (dur > (int)element_h)
			 {
				  if (is_left_wall_block(block_type_at((int)(element_x+step_velocity_x)-1, (int)(element_y)+(int)element_h)))
				  {
						dur = (int)element_h+1;
						enemy.velocity_x = 0.0f;
						enemy.x = block_right_edge(((int)(element_x+step_velocity_x-8)-1)/16) + 1;
						collided = true;
						//turn_around();
				  }
			 }
		}

		if (collided) enemy.on_collides_with_wall_tile(0, 0 /* do this part today */);
	}
}



bool enemy_at_platform_right_edge(EnemyBase *enemy_ptr)
{
	if (!enemy_ptr) return false;
	EnemyBase &enemy = *enemy_ptr;


	if (is_platform_block(block_type_at((int)(enemy.x)+(int)enemy.w, (int)(enemy.y)+(int)enemy.h+1)))
	return true;
	else return false;
}



bool enemy_at_platform_left_edge(EnemyBase *enemy_ptr)
{
	if (!enemy_ptr) return false;
	EnemyBase &enemy = *enemy_ptr;

	if (is_platform_block(block_type_at((int)(enemy.x), (int)(enemy.y)+(int)enemy.h+1)))
	return true;
	else return false;
}



void test_if_enemy_will_fall_off(EnemyBase *enemy_ptr)
{
	if (!enemy_ptr) return;
	EnemyBase &enemy = *enemy_ptr;

	if (enemy.on_ground)
	{
		if (enemy.facing_left)
		{
			if (!enemy_at_platform_left_edge(enemy_ptr))
			{
				enemy.on_about_to_walk_off_ground();
				//enemy.on_ccollide
				 //if (will_jump_off_edge) jump(jump_x_velocity_boost);
				 //else turn_around();
			}
		}
		else
		{
			if (!enemy_at_platform_right_edge(enemy_ptr))
			{
				enemy.on_about_to_walk_off_ground();
				// if (will_jump_off_edge) jump(jump_x_velocity_boost);
				 //else turn_around();
			}
		}
	}
}


/*
void update_enemy_position(EnemyBase *enemy_ptr)
{
	if (!enemy_ptr) return;
	EnemyBase &enemy = *enemy_ptr;

	float timestep = GAMEPLAY_FRAME_DURATION_PER_SEC;

	// vertical tests
	TileAndObjectCollision::test_enemy_map_collision_vertical(enemy_ptr, timestep);
	enemy.y += enemy.velocity_y * timestep;

	// horizontal tests
	TileAndObjectCollision::test_enemy_map_collision_horizontal(enemy_ptr, timestep);
	test_if_enemy_will_fall_off(enemy_ptr);
	enemy.x += enemy.velocity_x * timestep;
}
*/


void update_enemy_position(EnemyBase *enemy_ptr)
{
	if (!enemy_ptr) return;
//	EnemyBase &enemy = *enemy_ptr;

	// plug our values into a collision object
	TileAndObjectCollision element;	

	element.x = enemy_ptr->x;
	element.y = enemy_ptr->y;
	element.w = enemy_ptr->w;
	element.h = enemy_ptr->h;
	element.velocity_x = enemy_ptr->velocity_x;
	element.velocity_y = enemy_ptr->velocity_y;






	float timestep = GAMEPLAY_FRAME_DURATION_PER_SEC;
	std::vector<int2> collided_tiles;

	// vertical tests
//	TileAndObjectCollision::test_enemy_map_collision_vertical(enemy_ptr, timestep);
	collided_tiles = TileAndObjectCollision::test_tile_map_collision_axis(TileAndObjectCollision::Y_AXIS, element, timestep);
	bool collides_on_ground = false;
	for (int2 &t : collided_tiles)
	{
		if (element.velocity_y < 0.0)
		{
			if (is_bottom_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				element.velocity_y = 0;
				element.set_top(block_bottom_edge(t.y) + 0.0001);
			}
		}
		else if (element.velocity_y > 0.0)
		{
			if (is_top_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				element.velocity_y = 0;
				element.set_bottom(block_top_edge(t.y) - 0.0001);
				collides_on_ground = true;
			}
		}
	}
	element.y += element.velocity_y * timestep;

	// horizontal tests
	collided_tiles = TileAndObjectCollision::test_tile_map_collision_axis(TileAndObjectCollision::X_AXIS, element, timestep);
	//TileAndObjectCollision::test_enemy_map_collision_horizontal(enemy_ptr, timestep);
	//test_if_enemy_will_fall_off(enemy_ptr); // dont' avoid walking off clifs
	bool collided_at_wall = false;
	for (int2 &t : collided_tiles)
	{
		if (element.velocity_x < 0.0)
		{
			if (is_right_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				element.velocity_x = 0;
				element.set_left(block_right_edge(t.x) + 0.0001);
				collided_at_wall = true;
			}
		}
		else if (element.velocity_x > 0.0)
		{
			if (is_left_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				element.velocity_x = 0;
				element.set_right(block_left_edge(t.x) - 0.0001);
				collided_at_wall = true;
			}
		}
	}
	element.x += element.velocity_x * timestep;



	// plug it back into the original enemy

	enemy_ptr->x = element.x;
	enemy_ptr->y = element.y;
	enemy_ptr->w = element.w;
	enemy_ptr->h = element.h;
	enemy_ptr->velocity_x = element.velocity_x;
	enemy_ptr->velocity_y = element.velocity_y;

	//std::cout << enemy_ptr->velocity_y << std::endl;

	if (collides_on_ground)
	{
		enemy_ptr->on_ground = true;
	}
	if (collided_at_wall)
	{
		//enemy_ptr->turn_around();
		enemy_ptr->facing_left = !enemy_ptr->facing_left;
		enemy_ptr->bitmap_flip_h = !enemy_ptr->bitmap_flip_h;

		if (enemy_ptr->facing_left) enemy_ptr->velocity_x = -0.5;
		else enemy_ptr->velocity_x = 0.5;
		//if (enemy_ptr->facing_left) enemy_ptr->velocity_x = -enemy_ptr->default_velocity;
		//else enemy_ptr->velocity_x = enemy_ptr->default_velocity;
	}


}



//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////



class character_class;
void NEW_update_player_position(character_class *enemy_ptr);




class character_class
{
public: // this it's high security bank software!!

    float2 map_pos;   // character's location on the current map.
    int2 size;         // character's width and height
    float2 velocity;    // character's velocity

    // speed and drag variables
    float walk_max_speed;   // the maximum walking speed the character can move while walking
    float run_max_speed;    // the maximum speed the character can move while running
    float max_speed;        // the current maximum speed
    float max_climb_speed;
    float jump_velocity;    // the initial force of the jump
    float variable_jump_velocity_decay;   // if you release the jump button before
                                          // the peak of the jump, this value ditermines
                                          // how much velocity you will loose
    float accel_rate;        // how fast the character speeds up when walking or running
    float environment_drag;
    float air_drag;
    float floor_drag;

    // state variables

    bool is_ducking;
	 bool is_downthrusting;

    bool has_gun;
    int gun_ammo;

    bool facing_right;      // if the character is facing right
    bool on_ground;         // if the character is on the ground
                               // used for gravity and jumping
    bool released_jump;     // if the character has released his jump
    int on_ramp;           // if the character is on a ramp
    bool on_ladder;
    int moving_direction;   // the direction the character is trying to move

    // energy stuff
    int energy;       // the current ammount of energy.. shouldn't be more than max_energy
    int max_energy;     // the maximum energy the player can have.

    // animation
    int anim_counter;
    int anim_rate;

    //bool post_pain;
    int post_pain_counter;
    int bounce_from_pain;

    bool player_alive;

    float strength;
    
    int keys;
    int silver_keys;

    int refire_delay;

    /// functions
    /////////////////////

    character_class(void)
    {
        init();
    }

    void init(void)
    {
        map_pos.x = 20.0f;
        map_pos.y = 20.0f;
        size.x = 14;
        size.y = 30-4;
        velocity.x = 0.0f;
        velocity.y = 0.0f;
        
        keys = 0;
        silver_keys = 0;

        walk_max_speed = DEFAULT_MAX_WALK_SPEED;
        run_max_speed = DEFAULT_MAX_RUN_SPEED; // 3.0f
        max_speed = walk_max_speed;
        max_climb_speed = DEFAULT_MAX_CLIMB_SPEED;
        jump_velocity = DEFAULT_JUMP_VELOCITY;
        variable_jump_velocity_decay = 0.15f;
        accel_rate = DEFAULT_ACCEL_RATE;  //0.75f - on crack!   //0.15f normal  // on ice = 0.05f;
        floor_drag = ENVIRONMENT_DRAG_FLOOR;
        air_drag = ENVIRONMENT_DRAG_AIR;
			strike_distance = DEFAULT_STRIKE_DISTANCE;
        environment_drag = 0.0f;

        facing_right = true;
        on_ground = false;
        released_jump = true;
        is_ducking = false;
			is_downthrusting = false;

        on_ramp = -1;
        on_ladder = false;
        moving_direction = MOVING_STILL;

        strength = 1.0f;
        energy = 6;
        max_energy = 6;
        anim_counter = 0;
        anim_rate = 0;

        has_gun = false;
        gun_ammo = 0;

        //post_pain = false;
        post_pain_counter = 0;    // 50
        player_alive = true;
        bounce_from_pain = true;

        refire_delay = 0;

			DamageZones::kill(swipe_damage_zone);
			swipe_damage_zone = NULL;
    }

    void reset_for_new_level()
    {
//        map_pos.x = 20.0f;
//        map_pos.y = 20.0f;
//        size.x = 14;
//        size.y = 37;
        velocity.x = 0.0f;
        velocity.y = 0.0f;
        
        keys = 0;
        silver_keys = 0;

        walk_max_speed = DEFAULT_MAX_WALK_SPEED;
        run_max_speed = DEFAULT_MAX_RUN_SPEED; // 3.0f
        max_speed = walk_max_speed;
        max_climb_speed = DEFAULT_MAX_CLIMB_SPEED;
        jump_velocity = DEFAULT_JUMP_VELOCITY;
        variable_jump_velocity_decay = 0.15f;
        accel_rate = DEFAULT_ACCEL_RATE;  //0.75f - on crack!   //0.15f normal  // on ice = 0.05f;
        floor_drag = ENVIRONMENT_DRAG_FLOOR;
        air_drag = ENVIRONMENT_DRAG_AIR;
        environment_drag = 0.0f;

        facing_right = true;
        on_ground = false;
        released_jump = true;
        is_ducking = false;
			is_downthrusting = false;

        on_ramp = -1;
        on_ladder = false;
        moving_direction = MOVING_STILL;

        strength = 1.0f;
        energy = 6;
        max_energy = 6;
        anim_counter = 0;
        anim_rate = 0;

        has_gun = false;
        gun_ammo = 0;

        //post_pain = false;
        post_pain_counter = 0;    // 50
        player_alive = true;
        bounce_from_pain = true;

        refire_delay = 0;

			DamageZones::kill(swipe_damage_zone);
			swipe_damage_zone = NULL;
    }

	player_damage_zone *swipe_damage_zone;
	int strike_distance;

	void attack()
	{
		play_attack_sound();
		int current_map = get_current_map_index();

		DamageZones::kill(swipe_damage_zone);
		swipe_damage_zone = DamageZones::spawn(map_pos.x + size.x/2 - ((!facing_right) ? strike_distance : 0), map_pos.y + 5, strike_distance, size.y/3*2, current_map, 1, PLAYER_SWIPE, GAMEPLAY_FRAME_DURATION/2);
		//static void spawn(float x, float y, float w, float h, int map_num, float damage, int type, float lifespan)
	}

	void update_attacked_damage_zones()
	{
		if (swipe_damage_zone)
		{
			swipe_damage_zone->x = map_pos.x + size.x/2;
			if (!facing_right) swipe_damage_zone->x -= strike_distance;
			swipe_damage_zone->y = map_pos.y + 5;
			if (DamageZones::dead(swipe_damage_zone))
			{
				DamageZones::kill(swipe_damage_zone);
				swipe_damage_zone = NULL;
			}
		}
	}

    // find points
    float get_right_edge() {return map_pos.x+size.x;}
    float get_left_edge() {return map_pos.x;}
    float get_top_edge() {return map_pos.y;}
    float get_bottom_edge() {return map_pos.y+size.y;}
    float get_middle() {return map_pos.y+(size.y/2);}
    float get_center() {return map_pos.x+(size.x/2);}
    int get_right_edge_int() {return (int)(map_pos.x+size.x);}
    int get_left_edge_int() {return (int)(map_pos.x);}
    int get_top_edge_int() {return (int)(map_pos.y);}
    int get_bottom_edge_int() {return (int)(map_pos.y+size.y);}
    int get_middle_int() {return (int)(map_pos.y+(size.y/2));}
    int get_center_int() {return (int)(map_pos.x+(size.x/2));}

    // set points
    void set_right_edge(float f) {map_pos.x = f-size.x;}
    void set_left_edge(float f) {map_pos.x = f;}
    void set_top_edge(float f) {map_pos.y = f;}
    void set_bottom_edge(float f) {map_pos.y = f-size.y;}
    void set_center(float f) {map_pos.x = f-(size.x/2);}
    void set_middle(float f) {map_pos.y = f-(size.y/2);}

    // pushing functions
    void push_horizontal(float f) {velocity.x += f;}
    void push_vertical(float f) {velocity.y += f;}
    void update_environment_drag()
    {
        if (!on_ground) environment_drag = air_drag;
        else environment_drag = air_drag+floor_drag;
    }
    void update_horizontal_velocity(void)
    {
        if (velocity.x > 0.0f)
        {
           velocity.x -= environment_drag;
				if (moving_direction != MOVING_RIGHT) velocity.x *= 0.01;
           if (velocity.x < 0.0f) velocity.x = 0;
        }
        else if (velocity.x < 0.0f)
        {
           velocity.x += environment_drag;
				if (moving_direction != MOVING_LEFT) velocity.x *= 0.01;
           if (velocity.x > 0.0f) velocity.x = 0;
        }
    }
    void update_vertical_velocity(void)
    {
        if (!on_ground && !on_ladder)
        {
           if (!(velocity.y > TERMINAL_VELOCITY))
           {
               velocity.y += GRAVITY;
           }
           else velocity.y -= environment_drag;
        }
    }

    // character movements
    void run(void) {
        max_speed = run_max_speed;
        }
    void walk(void) {
        max_speed = walk_max_speed;
        }
    void duck(void)
        {
			if (on_ground)
			{
			  if (!is_ducking) {size.y = 30-4-12; map_pos.y+=12;}
			  is_ducking = true;
			}
			else
			{
				is_downthrusting = true;
			}
        }
    void stand(void)
        {
        if (is_ducking) {size.y += 12; map_pos.y-=12;}
        is_ducking = false;
        }
    void move_right(void) {
        moving_direction = MOVING_RIGHT;
        facing_right = true;
        }
    void move_left(void) {
        moving_direction = MOVING_LEFT;
        facing_right = false;
        }
    void move_nowhere(void) {
        moving_direction = MOVING_STILL;
        }
    void jump(void)
    {
       if (on_ground && (released_jump == true))
       {
          push_vertical(jump_velocity*(-1));
          on_ground = false;
/////
          get_off_ramp();
          released_jump = false;
       }
    }
    void jump_off_ladder(void)
    {
        on_ladder = false;
        push_vertical(jump_velocity*(-1));
        on_ground = false;
        released_jump = false;
    }

    void put_on_running_shoes()
    {
        walk_max_speed = DEFAULT_MAX_RUN_SPEED + 0.5f;
        run_max_speed = walk_max_speed + 1.0f;
        accel_rate = DEFAULT_ACCEL_RATE * 1.5f;
    }

    void take_off_running_shoes()
    {
        walk_max_speed = DEFAULT_MAX_WALK_SPEED;
        run_max_speed = DEFAULT_MAX_RUN_SPEED; // 3.0f
        accel_rate = DEFAULT_ACCEL_RATE;
    }

    void pickup_gun()
    {
        has_gun = true;
    }

    void put_on_jumping_shoes()
    {
        jump_velocity = DEFAULT_JUMP_VELOCITY*1.5f;
    }

    void take_off_jumping_shoes()
    {
        jump_velocity = DEFAULT_JUMP_VELOCITY;
    }

    void get_on_ladder(int ladder_x)
    {
        on_ladder = true;
        map_pos.x = ladder_x - size.x/2 + 1;
        velocity.x = 0.0f;
        velocity.y = 0.0f;
        bear = bear_climb1;
    }
    void get_off_ladder()
    {
        on_ladder = false;
        velocity.x = 0.0f;
        velocity.y = 0.0f;
    }
    void release_jump(void)
    {
       if (velocity.y < 0.0f) push_vertical(variable_jump_velocity_decay);
       if (on_ground) released_jump = true;
    }
    int get_on_ramp(int ramp_index) // max_num_of_ramps = current_map.ramp.size();
    {
       if (ramp_index < 0) return 1;

       velocity.y = 0.0f;
       on_ramp = ramp_index;
       on_ground = true;
	    is_downthrusting = false;
       return 0;
    }
    void get_off_ramp()
    {
       on_ramp = -1;
    }
    float take_and_react_to_damage(float damage)
    {
			if (post_pain_counter > 0)
			{
				return energy;
			}

        energy -= damage;
        if (post_pain_counter == 0)
        {
            post_pain_counter = 70;
        }

        if (bounce_from_pain)
        {
            if (facing_right) velocity.x = -2.0f;
            else velocity.x = 2.0f;
        }
        return energy;
    }
    float energy_up(float e)
    {
        energy += e;
        if (energy >= max_energy) energy = max_energy;
        
        return energy;   
    }    
    bool at_platform()
    {
        int dur=0;
        for (dur=0; dur<size.x+1; dur++)
        {
            if (is_platform_block(block_type_at((int)(map_pos.x)+dur, (int)(map_pos.y)+size.y+1)))
            return true;
        }
        return false;
    }

	 // don't go off the map
	void prevent_from_going_off_map()
	{
		if (map_pos.x <= 0)
		{
			map_pos.x = 0;
			velocity.x = 0;
		}
		if (get_right_edge() >= ((get_map_width()*TILE_SIZE)-1))
		{
			set_right_edge((get_map_width()*TILE_SIZE)-1);
			velocity.x = 0;
		}
	}

    bool ditect_collision(int x1, int y1, int x2, int y2)
    {
        if ((map_pos.x > x2) == true) return false;
        if ((map_pos.x+size.x < x1) == true) return false;
        if ((map_pos.y+size.y < y1) == true) return false;
        if ((map_pos.y > y2) == true) return false;
        else return true;
    }

    bool ditect_collision_center(int x1, int y1, int x2, int y2)
    {
        if (get_center_int() > x2) return false;
        if (get_center_int() < x1) return false;
        if (map_pos.y+size.y < y1) return false;
        if (map_pos.y > y2) return false;
        else return true;
    }

    bool ditect_collision_center_post_v(int x1, int y1, int x2, int y2)
    {
        if (get_center_int()+velocity.x > x2) return false;
        if (get_center_int()+velocity.x < x1) return false;
        if (map_pos.y+size.y+velocity.y < y1) return false;
        if (map_pos.y+velocity.y > y2) return false;
        else return true;
    }

    int pickup_key()
    {
        keys++;
        return keys;
    }

    int pickup_silver_key()
    {
        silver_keys++;
        return keys;
    }
    
    int use_key()
    {
        keys--;
        if (keys < 0) {keys=0; return -1;}
        return keys;
    }   

    int use_silver_key()
    {
        silver_keys--;
        if (silver_keys < 0) {silver_keys=0; return -1;}
        return silver_keys;
    }

    int get_keys()
    {
        return keys;
    }     

    int get_silver_keys()
    {
        return silver_keys;
    }


    // map collision detections
    int test_map_collision_vertical()
    {
        // return values:
        // 0; no collision
        // 1: collided with a floor
        // 2: collided with a ceiling
        // 3: at the end of a ladder

        #define TRIGGER_Y_OFFSET  6

        if (velocity.y > 0.0f) // falling
        {
            bool char_collides_next = false;
            bool char_collides_now = false;
            int dur=0;
            while(dur<size.x)
            {
                if (is_platform_block(block_type_at((int)(map_pos.x)+dur, (int)(map_pos.y+velocity.y)+size.y+1)))
                {
                    char_collides_next = true;
                }
                if (is_platform_block(block_type_at((int)(map_pos.x)+dur, (int)(map_pos.y)+size.y)))
                {
                    char_collides_now = true;
                }

                dur++;
            }
            if (on_ladder)
            {
                //if (is_ladder_block(block_type_at(get_center_int(), (int)(get_bottom_edge()+velocity.y+1))) == BLOCK_LADDER)
                if (is_ladder_block(block_type_at(get_center_int(), (int)(get_bottom_edge()+velocity.y+1))))
                {
                    char_collides_next=false;
                }
            }

            if (((!char_collides_now) && (char_collides_next)))
            {

                int pos_to_set_to = block_top_edge(((int)(map_pos.y+velocity.y+TILE_SIZE/2)+size.y+1)/TILE_SIZE) - (size.y) - 1;

                int block_player_lands_on_left = block_type_at((int)get_left_edge(), (((int)(map_pos.y+velocity.y+TILE_SIZE/2)+size.y+1+3)));
                int block_player_lands_on_center = block_type_at(get_center_int(), (((int)(map_pos.y+velocity.y+TILE_SIZE/2)+size.y+1+3)));
                int block_player_lands_on_middle = block_type_at((int)get_right_edge(), (((int)(map_pos.y+velocity.y+TILE_SIZE/2)+size.y+1+3)));

                if (is_trigger_block(block_player_lands_on_left) || is_trigger_block(block_player_lands_on_center) || is_trigger_block(block_player_lands_on_middle))
                {
                    attempt_to_turn_on_trigger(block_player_lands_on_left - BLOCK_TRIGGER);
                    attempt_to_turn_on_trigger(block_player_lands_on_center - BLOCK_TRIGGER);
                    attempt_to_turn_on_trigger(block_player_lands_on_middle - BLOCK_TRIGGER);
                    //attempt_to_turn_on_trigger(3);
                    //pos_to_set_to -= TRIGGER_Y_OFFSET;
                }

                map_pos.y = pos_to_set_to;
                on_ground = true;
						is_downthrusting = false;
                velocity.y = 0.0f;
                return 1;
            }
        }
        else if (velocity.y < 0.0f) // jumping (or moving up)
        {
            int dur=0;
            while(dur<size.x)
            {
                if (is_ceiling_block(block_type_at((int)(map_pos.x+1)+dur, (int)(map_pos.y+velocity.y))))
                {
                    //if (on_ladder && ((block_type_at(get_center_int(), (int)(map_pos.y+velocity.y)-1) == BLOCK_LADDER)))
                    if (on_ladder && is_ladder_block(((block_type_at(get_center_int(), (int)(map_pos.y+velocity.y)-1)))))
                    {
                        dur = size.x+1;
                        //if (block_type_at(get_center_int(), (int)(map_pos.y+velocity.y)-1) == BLOCK_LADDER)
                    }
                    else
                    {
                        dur = size.x+1;
                        velocity.y = 0.0f;
                        map_pos.y = block_bottom_edge(((int)(map_pos.y+velocity.y-TILE_SIZE/2)-1)/TILE_SIZE);
                        return 2;
                    }
                }
                else dur++;
            }
            if (on_ladder)
            {
                //if (block_type_at(get_center_int(), (int)(get_middle()+velocity.y)) != BLOCK_LADDER)
                if (!is_ladder_block(block_type_at(get_center_int(), (int)(get_middle()+velocity.y))))
                {
							float top_edge = block_top_edge((int)(get_middle() / 16));
							set_middle(top_edge);
                    velocity.y = 0.0f;
                    return 3;
                }
            }
        }
        else if (on_ramp != -1)
        {
            bool going_down_on_ramp = false;
            bool going_up_on_ramp = false;  // not doin this one yet
 //           float point_to_test_y = point_on_ramp_y(on_ramp, get_center_int());

            if (velocity.x < 0.0f)
            {
                going_down_on_ramp = true;
            }
        }
        return 0;
    }
    int test_map_collision_horizontal()
    {
        // return values:
        // 0: no collision
        // 1: collided at the left side of the player
        // 2: collided at the right side of the player

        if (velocity.x > 0.0f)
        {
            int dur=0;
            while(dur<size.y)
            {
                if (is_right_wall_block(block_type_at((int)(map_pos.x+velocity.x)+size.x+1, (int)(map_pos.y)+dur)))
                {
                    dur = size.y+1;
                    velocity.x = 0.0f;
                    map_pos.x = block_left_edge(((int)(map_pos.x+velocity.x+TILE_SIZE/2)+size.x+1)/TILE_SIZE) - (size.x) -1;
                    return 1;
                }
                else dur+=16;
                if (dur > size.y)
                {
                    if (is_right_wall_block(block_type_at((int)(map_pos.x+velocity.x)+size.x+1, (int)(map_pos.y)+size.y)))
                    {
                        dur = size.y+1;
                        velocity.x = 0.0f;
                        map_pos.x = block_left_edge(((int)(map_pos.x+velocity.x+TILE_SIZE/2)+size.x+1)/TILE_SIZE) - (size.x) -1;
                        return 1;
                    }
                }
            }
        }
        else if (velocity.x < 0.0f)
        {
            int dur=0;
            while(dur<size.y)
            {
                if (is_left_wall_block(block_type_at((int)(map_pos.x+velocity.x)-1, (int)(map_pos.y)+dur)))
                {
                    dur = size.y+1;
                    velocity.x = 0.0f;
                    map_pos.x = block_right_edge(((int)(map_pos.x+velocity.x-TILE_SIZE/2)-1)/TILE_SIZE);// + 1;
                    return 2;
                }
                else dur+=16;
                if (dur > size.y)
                {
                    if (is_left_wall_block(block_type_at((int)(map_pos.x+velocity.x)-1, (int)(map_pos.y)+size.y)))
                    {
                        dur = size.y+1;
                        velocity.x = 0.0f;
                        map_pos.x = block_right_edge(((int)(map_pos.x+velocity.x-TILE_SIZE/2)-1)/TILE_SIZE);// + 1;
                        return 2;
                    }
                }
            }
        }
        return 0;
    } // end test_map_collision_horizontal()

    void update_velocity_from_player_movement()
    {
        if (!is_ducking || !on_ground)
        {
            switch (moving_direction)
            {
                case MOVING_RIGHT:
                     if (velocity.x >= max_speed)
                     {

                     }
                     else
                     {
                         push_horizontal(accel_rate);
                         if (velocity.x > max_speed) velocity.x = max_speed;
                     }
                     break;
                     /////////////////////////////////////
                case MOVING_LEFT:
                     if (velocity.x <= max_speed*(-1))
                     {

                     }
                     else
                     {
                         push_horizontal(accel_rate*(-1));
                         if (velocity.x < max_speed*(-1)) velocity.x = max_speed*(-1);
                     }
                     break;
            }
        }
    }

    bool ramp_test()
    {
        int bypass_block_collisions = false;
        if (on_ramp != -1)
        {
            //bypass_block_collisions = true;
            //on_ground=true;
            //map_pos.x += velocity.x;
            //set_bottom_edge(point_on_ramp_y(on_ramp, get_center_int()));
        }
        return bypass_block_collisions;
    }


    bool test_on_death_block()
    {
         if (((block_type_at(get_center_int(), get_middle_int()) == BLOCK_DEATH)))
         {
            return true;
         }

         return false;
    }

    ////////////
    void update_pos(void)
    {

			//
			// apply forces to player_character
			//

        update_environment_drag();

        update_horizontal_velocity();
        update_vertical_velocity();
        update_velocity_from_player_movement();


			//NEW_update_player_position(character_class *enemy_ptr);
			NEW_update_player_position(this);




/*
        // get_off_a_ramp_judge();


        //if (ramp_test() == false)
        //{

        test_map_collision_horizontal();
        map_pos.x += velocity.x;

        int vert_return_val = test_map_collision_vertical();
			if (vert_return_val == 3) // at the end of a ladder (moving up)
			{
				// set bear's feet on ground at tile above
				float floor_edge = block_bottom_edge((int)(get_middle()-8) / 16);
				velocity.y = 0;
				set_bottom_edge(floor_edge - 1);
				on_ground = true;
				velocity.y = 0;
				on_ladder = false;
				is_downthrusting = false;
			}






        //get_on_a_ramp_judge();



        if ((velocity.y == 0.0f) && (on_ramp == -1))
        {
            if (at_platform()) {
                if (!on_ladder)
					{
						on_ground = true;
						is_downthrusting = false;
					}
                //environment_drag = FLOOR_STONE_DRAG + AIR_DRAG;
            }
            else {
                on_ground = false;
                //environment_drag = AIR_DRAG;
            }
        }
        else if (on_ramp != -1)
        {
            on_ground = true;
            is_downthrusting = false;
        }

        if (on_ramp != -1)
        {
           if ((vert_return_val == 1) || (vert_return_val == 2));
           //else set_bottom_edge(point_on_ramp_y(on_ramp, get_center_int()));
           else set_bottom_edge(32);  // YIKES! dirty placeholder hack
        }
        else map_pos.y += velocity.y;
        //}

*/


        if (test_on_death_block() == true)
        {
            energy = 0;
            player_alive = false;
        }

        prevent_from_going_off_map();

			update_attacked_damage_zones();
    }

    void update_state(void)
    {
        update_pos();
        if (post_pain_counter > 0) post_pain_counter--;
    }



    ////////////
    void draw_bounding_box(ALLEGRO_BITMAP *b = buffer)
    {
			
        rect(b, get_left_edge_int()-(int)camera_x, get_top_edge_int()-(int)camera_y, get_right_edge_int()-(int)camera_x, get_bottom_edge_int()-(int)camera_y, makecol(125, 255, 255));
        if (facing_right) textprintf_centre(b, font_med, get_center_int()-(int)camera_x, get_middle_int()-(text_height(font_med)/2)-2-(int)camera_y, makecol(125, 255, 255), ">>");
        else textprintf_centre(b, font_med, get_center_int()-(int)camera_x, get_middle_int()-(text_height(font_med)/2)-2-(int)camera_y, makecol(125, 255, 255), "<<");
        putpixel(b, get_center_int()-(int)camera_x, get_top_edge_int()-(int)camera_y, makecol(100-75, 223-75, 223-75));
        putpixel(b, get_center_int()-(int)camera_x, get_bottom_edge_int()-(int)camera_y, makecol(100-75, 223-75, 223-75));
        putpixel(b, get_left_edge_int()-(int)camera_x, get_middle_int()-(int)camera_y, makecol(100-75, 223-75, 223-75));
        putpixel(b, get_right_edge_int()-(int)camera_x, get_middle_int()-(int)camera_y, makecol(100-75, 223-75, 223-75));
    }
};

character_class player;


void NEW_update_player_position(character_class *enemy_ptr)
{
	if (!enemy_ptr) return;
//	EnemyBase &enemy = *enemy_ptr;

	// plug our values into a collision object
	TileAndObjectCollision element;	

	element.x = enemy_ptr->map_pos.x;
	element.y = enemy_ptr->map_pos.y;
	element.w = enemy_ptr->size.x;
	element.h = enemy_ptr->size.y;
	element.velocity_x = enemy_ptr->velocity.x;
	element.velocity_y = enemy_ptr->velocity.y;






	float timestep = GAMEPLAY_FRAME_DURATION_PER_SEC;
	std::vector<int2> collided_tiles;

	// vertical tests
//	TileAndObjectCollision::test_enemy_map_collision_vertical(enemy_ptr, timestep);
	collided_tiles = TileAndObjectCollision::test_tile_map_collision_axis(TileAndObjectCollision::Y_AXIS, element, timestep);
	bool collides_on_ground = false;
	for (int2 &t : collided_tiles)
	{
		if (element.velocity_y < 0.0)
		{
			if (is_bottom_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				element.velocity_y = 0;
				element.set_top(block_bottom_edge(t.y) + 0.0001);
			}
		}
		else if (element.velocity_y > 0.0)
		{
			if (is_top_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				element.velocity_y = 0;
				element.set_bottom(block_top_edge(t.y) - 0.0001);
				collides_on_ground = true;
			}
		}
	}
	element.y += element.velocity_y * timestep;

	// horizontal tests
	collided_tiles = TileAndObjectCollision::test_tile_map_collision_axis(TileAndObjectCollision::X_AXIS, element, timestep);
	//TileAndObjectCollision::test_enemy_map_collision_horizontal(enemy_ptr, timestep);
	//test_if_enemy_will_fall_off(enemy_ptr); // dont' avoid walking off clifs
	bool collided_at_wall = false;
	for (int2 &t : collided_tiles)
	{
		if (element.velocity_x < 0.0)
		{
			if (is_right_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				element.velocity_x = 0;
				element.set_left(block_right_edge(t.x) + 0.0001);
				collided_at_wall = true;
			}
		}
		else if (element.velocity_x > 0.0)
		{
			if (is_left_edge_block(block_type_at_tile_coord(t.x, t.y)))
			{
				element.velocity_x = 0;
				element.set_right(block_left_edge(t.x) - 0.0001);
				collided_at_wall = true;
			}
		}
	}
	element.x += element.velocity_x * timestep;



	// plug it back into the original enemy

	enemy_ptr->map_pos.x = element.x;
	enemy_ptr->map_pos.y = element.y;
	enemy_ptr->size.x = element.w;
	enemy_ptr->size.y = element.h;
	enemy_ptr->velocity.x = element.velocity_x;
	enemy_ptr->velocity.y = element.velocity_y;

	//std::cout << enemy_ptr->velocity_y << std::endl;

	if (collides_on_ground)
	{
		enemy_ptr->on_ground = true;
	}
/*
	if (collided_at_wall)
	{
		//enemy_ptr->turn_around();
		enemy_ptr->facing_left = !enemy_ptr->facing_left;
		enemy_ptr->bitmap_flip_h = !enemy_ptr->bitmap_flip_h;

		if (enemy_ptr->facing_left) enemy_ptr->velocity_x = -0.5;
		else enemy_ptr->velocity_x = 0.5;
		//if (enemy_ptr->facing_left) enemy_ptr->velocity_x = -enemy_ptr->default_velocity;
		//else enemy_ptr->velocity_x = enemy_ptr->default_velocity;
	}
*/

}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

float get_player_center()
{
	return player.map_pos.x+player.size.y/2;
}

float get_player_middle()
{
	return player.map_pos.y+player.size.x/2;
}
 

////////////////////////////////////////////////////////////////////////////////


bool is_switch_stick(int tile_num);
void play_switch_sound();
//bool swap_switch_stick(int index_num);
void set_switch_stick(int index_num, bool on);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//#include "map.hpp"



#define SECRET_SOLUTION_NONE             0
#define SECRET_SOLUTION_KILL_BEARS       1
#define SECRET_SOLUTION_COLLECT_JEWELS   2
#define SECRET_SOLUTION_TRIGGER_ALL      3
#define SECRET_SOLUTION_TRIGGER_ORDERED  4
//#define SECRET_SOLUTION_FER_OF           1000
//#define SECRET_SOLUTION_CHAIN_OF         2000




struct background_struct
{
    string img_filename;
    ALLEGRO_BITMAP *img;
    int index_num;
    
    bool loop_x;
    bool loop_y;    
    int x_offset;
    int y_offset;
    float x_speed;
    float y_speed;
    float x_disp;    // this one gets incremented with the speed;
    float y_disp;    // and this one too;
    
    float scroll_rate;
};    



struct destructable_block_struct
{
    std::string block_address;
    std::string shard_1_address;
	 std::string shard_2_address;
    std::string shard_3_address;
};

vector<background_struct> current_map_background;
void current_map__update_backgrounds()
{
  for (int k=0; k<current_map_background.size(); k++)
  {
		current_map_background[k].x_disp += (GAMEPLAY_FRAME_DURATION * 60 * current_map_background[k].x_speed);
		current_map_background[k].y_disp += (GAMEPLAY_FRAME_DURATION * 60 * current_map_background[k].y_speed);
		//background[k].x_disp = background[k].x_disp % (float)background[k].img->w;
		//background[k].y_disp = background[k].x_disp % (float)background[k].img->h;
  }    
}    



class NewBearyMap;
void __post_process_map(NewBearyMap *map);






class NewBearyMap : public TileMapBase
{
private:
	TileMap *new_tile_map_LAYER_BACKGROUND;
	TileMap *new_tile_map_LAYER_0;
	TileMap *new_tile_map_LAYER_1;
	TileMap *new_tile_map_LAYER_2;
	TileMap *new_tile_map_LAYER_BOUNDARY;

public:
	TileAtlas *new_tile_atlas;
	std::string title;
	std::string style;

	NewBearyMap()
		: new_tile_map_LAYER_BACKGROUND(new TileMap())
		, new_tile_map_LAYER_0(new TileMap())
		, new_tile_map_LAYER_1(new TileMap())
		, new_tile_map_LAYER_2(new TileMap())
		, new_tile_map_LAYER_BOUNDARY(new TileMap())
		, new_tile_atlas(new TileAtlas())
		, title("Untitled")
		, style("")
	{}

	int get_tile(int tile_x, int tile_y, int tile_layer) override
	{
		if (tile_layer == BEARY_TILE_LAYER_BACKGROUND) return new_tile_map_LAYER_BACKGROUND->get_tile(tile_x, tile_y);
		if (tile_layer == BEARY_TILE_LAYER_0) return new_tile_map_LAYER_0->get_tile(tile_x, tile_y);
		if (tile_layer == BEARY_TILE_LAYER_1) return new_tile_map_LAYER_1->get_tile(tile_x, tile_y);
		if (tile_layer == BEARY_TILE_LAYER_2) return new_tile_map_LAYER_2->get_tile(tile_x, tile_y);
		if (tile_layer == BEARY_TILE_LAYER_BOUNDARY) return new_tile_map_LAYER_BOUNDARY->get_tile(tile_x, tile_y);
      return 0;
	}
	bool set_tile(int tile_x, int tile_y, int tile_layer, int tile) override
	{
		if (tile_layer == BEARY_TILE_LAYER_BACKGROUND) return new_tile_map_LAYER_BACKGROUND->set_tile(tile_x, tile_y, tile);
		if (tile_layer == BEARY_TILE_LAYER_0) return new_tile_map_LAYER_0->set_tile(tile_x, tile_y, tile);
		if (tile_layer == BEARY_TILE_LAYER_1) return new_tile_map_LAYER_1->set_tile(tile_x, tile_y, tile);
		if (tile_layer == BEARY_TILE_LAYER_2) return new_tile_map_LAYER_2->set_tile(tile_x, tile_y, tile);
		if (tile_layer == BEARY_TILE_LAYER_BOUNDARY) return new_tile_map_LAYER_BOUNDARY->set_tile(tile_x, tile_y, tile);
      return 0;
	}
	int get_width()
	{
		return new_tile_map_LAYER_0->get_width();
	}
	int get_height()
	{
		return new_tile_map_LAYER_0->get_height();
	}
	bool draw_layer(int tile_layer, float camera_x, float float_y)
	{
		if (tile_layer == BEARY_TILE_LAYER_BACKGROUND) new_tile_map_LAYER_BACKGROUND->draw(camera_x, camera_y);
		if (tile_layer == BEARY_TILE_LAYER_0) new_tile_map_LAYER_0->draw(camera_x, camera_y);
		if (tile_layer == BEARY_TILE_LAYER_1) new_tile_map_LAYER_1->draw(camera_x, camera_y);
		if (tile_layer == BEARY_TILE_LAYER_2) new_tile_map_LAYER_2->draw(camera_x, camera_y);
		if (tile_layer == BEARY_TILE_LAYER_BOUNDARY) new_tile_map_LAYER_BOUNDARY->draw(camera_x, camera_y);
		return true;
	}
	void use_tile_atlas(TileAtlas *at)
	{
		new_tile_map_LAYER_BACKGROUND->use_tile_atlas(at);
		new_tile_map_LAYER_0->use_tile_atlas(at);
		new_tile_map_LAYER_1->use_tile_atlas(at);
		new_tile_map_LAYER_2->use_tile_atlas(at);
		new_tile_map_LAYER_BOUNDARY->use_tile_atlas(at);
	}
	void resize(int width, int height)
	{
		new_tile_map_LAYER_BACKGROUND->resize(width, height, 16, 16);
		new_tile_map_LAYER_0->resize(width, height, 16, 16);
		new_tile_map_LAYER_1->resize(width, height, 16, 16);
		new_tile_map_LAYER_2->resize(width, height, 16, 16);
		new_tile_map_LAYER_BOUNDARY->resize(width, height, 16, 16);
	}
};


NewBearyMap *current_map = NULL;





int get_map_width()
{
   return current_map->get_width();
}
int get_map_height()
{
   return current_map->get_height();
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////







////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#define NUM_SWITCH_BLOCKS 10
/*
switch_block_struct switch_block[NUM_SWITCH_BLOCKS];
struct switch_block_struct
{
};
*/


class switch_struct
{
public:
    ALLEGRO_BITMAP *switch_on;
    ALLEGRO_BITMAP *switch_off;
    ALLEGRO_BITMAP *block_on;
    ALLEGRO_BITMAP *block_off;
    int index_num;
    bool active;

	switch_struct()
		: switch_on(NULL)
		, switch_off(NULL)
		, block_on(NULL)
		, block_off(NULL)
		, index_num(-1)
		, active(false)
	{}
};

vector<switch_struct> level_class__switch_stick;





void write_outline_center_alfont_aa_to_destination(ALLEGRO_BITMAP *destination, int x, int y, ALLEGRO_FONT *f, int size, ALLEGRO_COLOR color, ALLEGRO_COLOR bgcolor, std::string buf, int num)
{
	y -= 1;
	x += 1;

	std::stringstream ss;
	ss << num;
	if (buf == "%d") buf = ss.str();
 
    alfont_textprintf_centre(destination, f, x+1, y, bgcolor, buf);
    alfont_textprintf_centre(destination, f, x-1, y, bgcolor, buf);
    alfont_textprintf_centre(destination, f, x, y+1, bgcolor, buf);
    alfont_textprintf_centre(destination, f, x, y-1, bgcolor, buf);
    alfont_textprintf_centre_aa(destination, f, x, y, color, buf);
}





int get_number_offset(int num)
{
    //if (num == 1) return -1;
    if (num == 7) return 1;
    if (num == 9) return 1;
    return 0;
}




void convert_to_transparent(ALLEGRO_BITMAP *b)
{
   int h; int k;

   bool swap_trigger = true;

   for (k=0; k<al_get_bitmap_height(b); k++)
   {
      if (swap_trigger)
      {
         for (h=0; h<al_get_bitmap_width(b); h+=2)
         {
            putpixel(b, h, k, makecol(255, 0, 255));
         }
      }
      else
      {
         for (h=1; h<al_get_bitmap_width(b); h+=2)
         {
            putpixel(b, h, k, makecol(255, 0, 255));
         }
      }

      swap_trigger = !swap_trigger;
   }
}





ALLEGRO_BITMAP *create_on_switch(string address, int num)
{
    ALLEGRO_BITMAP *original_image = load_bitmap(make_absolute(address).c_str());
	return original_image;

// do this later:
/*
    ALLEGRO_BITMAP *final_bitmap = create_bitmap(al_get_bitmap_width(original_image), al_get_bitmap_height(original_image)+16+8);
    clear_to_color(final_bitmap, makecol(255, 0, 255));

    ALLEGRO_BITMAP *on_img = load_bitmap(make_absolute("images/on.gif").c_str());
	al_convert_mask_to_alpha(on_img, makecol(255, 0, 255));

    ALLEGRO_BITMAP *on_switch = load_bitmap(make_absolute("images/switch_on.gif").c_str());
	al_convert_mask_to_alpha(on_switch, makecol(255, 0, 255));

    if (!original_image || !final_bitmap || !on_img || !on_switch) return NULL;

    // --- start makin!

	

    draw_sprite(final_bitmap, original_image, 0, 16+8);
    draw_sprite(final_bitmap, on_img, al_get_bitmap_width(final_bitmap)/2-al_get_bitmap_width(on_img)/2, 0);
    draw_sprite(final_bitmap, on_switch, 0, 8);
    write_outline_center_alfont_aa_to_destination(final_bitmap, al_get_bitmap_width(final_bitmap)/2+get_number_offset(num), al_get_bitmap_height(final_bitmap)-14, new_font, 12, makecol(255,255,255), makecol(128,128,128), "%d", num);


    // --- clean up

    al_destroy_bitmap(original_image);
    al_destroy_bitmap(on_img);
    al_destroy_bitmap(on_switch);
	al_convert_mask_to_alpha(final_bitmap, makecol(255, 0, 255));

    return final_bitmap;
*/
    return NULL;
}



ALLEGRO_BITMAP *create_off_switch(string address, int num)
{
    ALLEGRO_BITMAP *original_image = load_bitmap(make_absolute(address).c_str());
	return original_image;

/*
    ALLEGRO_BITMAP *original_image = load_bitmap(make_absolute(address).c_str());

    ALLEGRO_BITMAP *final_bitmap = create_bitmap(al_get_bitmap_width(original_image), al_get_bitmap_height(original_image)+16+8);
    clear_to_color(final_bitmap, makecol(255, 0, 255));

    ALLEGRO_BITMAP *off_img = load_bitmap(make_absolute("images/off.gif").c_str());
	al_convert_mask_to_alpha(off_img, makecol(255, 0, 255));

    ALLEGRO_BITMAP *off_switch = load_bitmap(make_absolute("images/switch_off.gif").c_str());
	al_convert_mask_to_alpha(off_img, makecol(255, 0, 255));

    if (!original_image || !final_bitmap || !off_img || !off_switch) return NULL;

    // --- start makin!


    draw_sprite(final_bitmap, original_image, 0, 16+8);
    draw_sprite(final_bitmap, off_img, al_get_bitmap_width(final_bitmap)/2-al_get_bitmap_width(off_img)/2, 0);
    draw_sprite(final_bitmap, off_switch, 0, 8);

    // draw switch here


    write_outline_center_alfont_aa_to_destination(final_bitmap, al_get_bitmap_width(final_bitmap)/2+get_number_offset(num), al_get_bitmap_height(final_bitmap)-14, new_font, 12, makecol(255,255,255), makecol(128,128,128), "%d", num);
//    write_outline_center_alfont_aa_to_destination(final_bitmap, final_bitmap->w/2+get_number_offset(num), final_bitmap->h-14, new_font, 12, makecol(0,0,0), makecol(128,128,128), "%d", num);


    // --- clean up

    al_destroy_bitmap(original_image);
    al_destroy_bitmap(off_img);
    al_destroy_bitmap(off_switch);
	al_convert_mask_to_alpha(final_bitmap, makecol(255, 0, 255));

    return final_bitmap;
*/
}


ALLEGRO_BITMAP *create_on_block(string address, int num)
{
    ALLEGRO_BITMAP *final_bitmap = load_bitmap(make_absolute(address).c_str());
	return final_bitmap;

/*
    if (!final_bitmap) return NULL;

    // --- start makin!

    // draw switch here
    // write text here

    write_outline_center_alfont_aa_to_destination(final_bitmap, al_get_bitmap_width(final_bitmap)/2+get_number_offset(num), al_get_bitmap_height(final_bitmap)-14, new_font, 12, makecol(255,255,255), makecol(128,128,128), "%d", num);


    //convert_to_transparent(final_bitmap);

    // --- clean up

    return final_bitmap;
*/
}

ALLEGRO_BITMAP *create_off_block(string address, int num)
{
    ALLEGRO_BITMAP *final_bitmap = load_bitmap(make_absolute(address).c_str());
	return final_bitmap;

/*
    if (!final_bitmap) return NULL;

    // --- start makin!

    // draw switch here
    // write text here

    write_outline_center_alfont_aa_to_destination(final_bitmap, al_get_bitmap_width(final_bitmap)/2+get_number_offset(num), al_get_bitmap_height(final_bitmap)-14, new_font, 12, makecol(255,255,255), makecol(128,128,128), "%d", num);
//    write_outline_center_alfont_aa_to_destination(final_bitmap, final_bitmap->w/2+get_number_offset(num), final_bitmap->h-14, new_font, 12, makecol(0,0,0), makecol(128,128,128), "%d", num);

    convert_to_transparent(final_bitmap);
	al_convert_mask_to_alpha(final_bitmap, makecol(255, 0, 255));


    // --- clean up

    return final_bitmap;
*/
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




ALLEGRO_BITMAP *destructable_block = NULL;
ALLEGRO_BITMAP *destructable_block_shard_1 = NULL;
ALLEGRO_BITMAP *destructable_block_shard_2 = NULL;
ALLEGRO_BITMAP *destructable_block_shard_3 = NULL;




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////






void destroy_switch_block_group(switch_struct *s)
{
    al_destroy_bitmap(s->switch_on);
    al_destroy_bitmap(s->switch_off);
    al_destroy_bitmap(s->block_on);
    al_destroy_bitmap(s->block_off);
}



void create_switch_block_group(switch_struct *s, int num)
{
    destroy_switch_block_group(s);

	  std::string switch_block_address = "images/tiles/stone12.bmp";

	 s->switch_on = create_on_switch(switch_block_address, num);
	 s->switch_off = create_off_switch(switch_block_address, num);
	 s->block_on = create_on_block(switch_block_address, num);
	 s->block_off = create_off_block(switch_block_address, num);

	 if (!s->switch_on || !s->switch_off || !s->block_on || !s->block_off) { allegro_message("BAM!"); abort_game = true; }
}



bool is_switch_active(int switch_index_num)
{
	if (switch_index_num < 0 || switch_index_num >= level_class__switch_stick.size()) return false;

	return level_class__switch_stick[switch_index_num].active;
}



class switch_stick_instance_struct
{
public:
	float width;
	float height;
	float x;
	float y;
	bool player_now_over;

	int switch_num;

	switch_stick_instance_struct(int switch_num, float x, float y)
		: switch_num(switch_num)
		, width(16)
		, height(24)
		, x(x-width/2)
		, y(y-height)
		, player_now_over(false)
	{}

	bool collides(float xx, float yy)
	{
		if (xx < x) return false;
		if (yy < y) return false;
		if (xx > x+width) return false;
		if (yy > y+height) return false;
		return true;
	}

};



class Switches
{
private:
	static Switches *instance;
	static Switches *get_instance()
	{
		if (!instance) instance = new Switches();
		return instance;
	}

	std::vector<switch_stick_instance_struct> switches;

	ALLEGRO_SAMPLE *switch_blocks_on_sound;
	ALLEGRO_SAMPLE *switch_blocks_off_sound;
	ALLEGRO_SAMPLE *switch_sound;

	ALLEGRO_BITMAP *switch_stick_on_bitmap;
	ALLEGRO_BITMAP *switch_stick_off_bitmap;

	Switches()
		: switches()
		, switch_blocks_on_sound(NULL)
		, switch_blocks_off_sound(NULL)
		, switch_sound(NULL)
		, switch_stick_on_bitmap(NULL)
		, switch_stick_off_bitmap(NULL)
	{
		switch_blocks_on_sound = al_load_sample("sounds/new/switch_on_clean.wav");
		switch_blocks_off_sound = al_load_sample("sounds/new/switch_off_clean.wav");
		switch_sound = al_load_sample("sounds/new/switch_sound.wav");

		switch_stick_on_bitmap = _load_bitmap("images/switch_stick_on.png");
		switch_stick_off_bitmap = _load_bitmap("images/switch_stick_off.png");
	}

	~Switches()
	{
		al_destroy_sample(switch_blocks_on_sound);
		al_destroy_sample(switch_blocks_off_sound);
		al_destroy_sample(switch_sound);

		al_destroy_bitmap(switch_stick_on_bitmap);
		al_destroy_bitmap(switch_stick_off_bitmap);
	}

	static void play_switch_blocks_on_sound()
	{
		if (!Switches::get_instance()->switch_blocks_on_sound) return;
		al_play_sample(Switches::get_instance()->switch_blocks_on_sound, 0.8, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	}

	static void play_switch_blocks_off_sound()
	{
		if (!Switches::get_instance()->switch_blocks_off_sound) return;
		al_play_sample(Switches::get_instance()->switch_blocks_off_sound, 0.8, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	}

	static void play_switch_sound()
	{
		if (!Switches::get_instance()->switch_sound) return;
		//al_play_sample(Switches::get_instance()->switch_sound, 1.0, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	}

public:
	static void clear()
	{
		std::vector<switch_stick_instance_struct> &switches = Switches::get_instance()->switches;
		switches.clear();
	}

	static bool toggle_switch_stick(int index_num)
	{
		if (index_num < 0 || index_num >= level_class__switch_stick.size())
		{
			std::stringstream ss;
			ss << "switch stick " << index_num << " out of bounds";
			start_help_message(ss.str().c_str());
			std::cout << ss.str() << std::endl;
			return false;
		}

		std::cout << " -- swapping stick " << index_num << std::endl;

		static ALLEGRO_BITMAP *on_bmp = _load_bitmap("images/on.gif");
		static ALLEGRO_BITMAP *off_bmp = _load_bitmap("images/censored_poof.gif");

		if (!current_map->new_tile_atlas)
		{
			std::cout << " -- AGH no atlas, can't swap" << std::endl;
		}
		else
		{
			if (level_class__switch_stick[index_num].active)
				current_map->new_tile_atlas->draw_tile_to_atlas(level_class__switch_stick[index_num].block_off, 60+index_num, al_map_rgba_f(0.3, 0.3, 0.3, 0.3)); //+60 because the tile atlas's switch blocks start at 60
			else
				current_map->new_tile_atlas->draw_tile_to_atlas(level_class__switch_stick[index_num].block_on, 60+index_num);
		}

		level_class__switch_stick[index_num].active = !level_class__switch_stick[index_num].active;

		Switches::play_switch_sound();
		if (!level_class__switch_stick[index_num].active) Switches::play_switch_blocks_off_sound();
		else Switches::play_switch_blocks_on_sound();

		std::cout << " -- swapped successfully to " << level_class__switch_stick[index_num].active << std::endl;
		std::cout << " -- checkingswapped successfully to " << is_switch_active(index_num) << std::endl;

		VisualFX::spawn(player.map_pos.x, player.map_pos.y, 0.5, 0.5, "explosion");


		return true;
	}

	static void add_switch_stick(float x, float y, int num)
	{
		std::vector<switch_stick_instance_struct> &switches = Switches::get_instance()->switches;

		switches.push_back(switch_stick_instance_struct(num, x, y));
	}
	static void update()
	{
		std::vector<switch_stick_instance_struct> &switches = Switches::get_instance()->switches;

		for (unsigned i=0; i<switches.size(); i++)
		{
			bool collides_now = switches[i].collides(player.map_pos.x + player.size.x/2, player.map_pos.y + player.size.y/2);
			if (collides_now && !switches[i].player_now_over)
			{
				// just entered!
				bool swapped_successfully = Switches::toggle_switch_stick(switches[i].switch_num);
				if (swapped_successfully)
					play_switch_sound();
				else
				{
					// play_switch_broken_sound();
				}
			}

			switches[i].player_now_over = collides_now;
		}
	}
	static void draw()
	{
		Switches *inst = Switches::get_instance();
		std::vector<switch_stick_instance_struct> &switches = Switches::get_instance()->switches;

		for (unsigned i=0; i<switches.size(); i++)
		{
			switch_stick_instance_struct &s = switches[i];
			bool on = is_switch_active(s.switch_num);

			ALLEGRO_BITMAP *stick_bmp = NULL;
			if (on) stick_bmp = inst->switch_stick_on_bitmap;
			else stick_bmp = inst->switch_stick_off_bitmap;

			// draw the bitmap
			if (stick_bmp) al_draw_bitmap(stick_bmp,
				s.x+s.width/2 - al_get_bitmap_width(stick_bmp)/2 - camera_x, s.y+s.height - al_get_bitmap_height(stick_bmp) - camera_y, 0);

			// draw the number
			if (s.switch_num != 0)
				al_draw_text(game_font, al_color_name("white"), s.x+s.width/2 - camera_x, s.y+s.height-17 - camera_y, ALLEGRO_ALIGN_CENTRE, tostring(s.switch_num).c_str());

			bool debug = false;
			if (debug)
			{
				al_draw_rectangle(s.x - camera_x, s.y - camera_y, s.x+s.width - camera_x, s.y+s.height - camera_y, al_color_name("dodgerblue"), 2.0);
				al_draw_text(game_font, al_color_name("dodgerblue"), s.x+2 - camera_x, s.y+2 - camera_y, 0, tostring(s.switch_num).c_str());
				al_draw_text(game_font, al_color_name("dodgerblue"), s.x+2 - camera_x, s.y+2+16 - camera_y, 0, tostring(on ? "ON" : "OFF").c_str());
			}
		}
	}
};

Switches *Switches::instance = NULL;


void set_switch_stick(int index_num, bool on)
{
	if (is_switch_active(index_num) != on) Switches::toggle_switch_stick(index_num);
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////





class StyleDrafter
{
public:
	static bool any_surrounding_tile_does_not_match(TileMapBase &map, int tile_x, int tile_y, std::vector<int> boundary_type_to_match)
	{
		for (int y=tile_y-1; y<=tile_y+1; y++)
			for (int x=tile_x-1; x<=tile_x+1; x++)
			{
				if (std::find(boundary_type_to_match.begin(), boundary_type_to_match.end(), map.get_tile(x, y, BEARY_TILE_LAYER_BOUNDARY)) != boundary_type_to_match.end()) return true;
				//int this_tile = 
				//if (map.get_tile(x, y, BEARY_TILE_LAYER_BOUNDARY) != boundary_type_to_match) return true;
			}

		return false;
	}
	static bool below_is_ladder(TileMapBase &map, int tile_x, int tile_y)
	{
		int x = tile_x;
		int y = tile_y + 1;

		if (is_ladder_block(map.get_tile(x, y, BEARY_TILE_LAYER_BOUNDARY))) return true;

		return false;
	}
	static bool above_is_ladder(TileMapBase &map, int tile_x, int tile_y)
	{
		int x = tile_x;
		int y = tile_y - 1;

		if (is_ladder_block(map.get_tile(x, y, BEARY_TILE_LAYER_BOUNDARY))) return true;

		return false;
	}
	static bool matches(TileMapBase &map, int tile_x, int tile_y, std::vector<int> tile_to_match, int LAYER=BEARY_TILE_LAYER_BOUNDARY)
	{
		if (std::find(tile_to_match.begin(), tile_to_match.end(), map.get_tile(tile_x, tile_y, LAYER)) != tile_to_match.end()) return true;
		return false;
	}
	static bool above_matches(TileMapBase &map, int tile_x, int tile_y, std::vector<int> boundary_type_to_match)
	{
		int x = tile_x;
		int y = tile_y - 1;

		if (std::find(boundary_type_to_match.begin(), boundary_type_to_match.end(), map.get_tile(x, y, BEARY_TILE_LAYER_BOUNDARY)) != boundary_type_to_match.end()) return true;
		//if (is_ladder_block(map.get_tile(x, y, BEARY_TILE_LAYER_BOUNDARY))) return true;

		return false;
	}
	static bool right_matches(TileMapBase &map, int tile_x, int tile_y, std::vector<int> boundary_type_to_match)
	{
		int x = tile_x + 1;
		int y = tile_y;

		if (std::find(boundary_type_to_match.begin(), boundary_type_to_match.end(), map.get_tile(x, y, BEARY_TILE_LAYER_BOUNDARY)) != boundary_type_to_match.end()) return true;
		//if (is_ladder_block(map.get_tile(x, y, BEARY_TILE_LAYER_BOUNDARY))) return true;

		return false;
	}
};




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////





void __post_process_map(NewBearyMap *m)
{
	if (!m) return;
	if (m->style.empty()) return;

	std::cout << "entering into post process." << std::endl;

	NewBearyMap &map = (*m);

	::background_color = al_map_rgba_f(0,0,0,1.0);
	::foreground_color = al_map_rgba_f(0,0,0,0);


	//
	// switches
	//

	for (unsigned y=0; y<current_map->get_height(); y++)
		for (unsigned x=0; x<current_map->get_width(); x++)
		{
			for (int i=0; i<(int)level_class__switch_stick.size(); i++)
				if (StyleDrafter::matches(map, x, y, {BLOCK_SWITCH_BLOCK+i}))
					map.set_tile(x, y, BEARY_TILE_LAYER_1, 60+i);
		}


	//
	// graphic style
	//

	if (map.style == "cavern")
	{
		for (unsigned y=0; y<current_map->get_height(); y++)
			for (unsigned x=0; x<current_map->get_width(); x++)
			{

				// 
				// patterened background tiles
				//

				int N=-64;
				//int N=1;
				bool this_matches = StyleDrafter::matches(map, x, y, {56+N}, BEARY_TILE_LAYER_BACKGROUND);

				bool left = StyleDrafter::matches(map, x-1, y, {56+N}, BEARY_TILE_LAYER_BACKGROUND);
				bool right = StyleDrafter::matches(map, x+1, y, {56+N}, BEARY_TILE_LAYER_BACKGROUND);
				bool above = StyleDrafter::matches(map, x, y-1, {56+N}, BEARY_TILE_LAYER_BACKGROUND);
				bool below = StyleDrafter::matches(map, x, y+1, {56+N}, BEARY_TILE_LAYER_BACKGROUND);
				bool above_left = StyleDrafter::matches(map, x-1, y-1, {56+N}, BEARY_TILE_LAYER_BACKGROUND);
				bool above_right = StyleDrafter::matches(map, x+1, y-1, {56+N}, BEARY_TILE_LAYER_BACKGROUND);
				bool below_left = StyleDrafter::matches(map, x-1, y+1, {56+N}, BEARY_TILE_LAYER_BACKGROUND);
				bool below_right = StyleDrafter::matches(map, x+1, y+1, {56+N}, BEARY_TILE_LAYER_BACKGROUND);

			// all the matching things
			{
				int tile_result = -1;
				if (this_matches) tile_result = random_int({112, 121, 122, 123, 132});

				if ((!above_left && !above && !above_right) && (!left && !this_matches && right) && (below_left && below && below_right)) tile_result = 100;
				if ((!above_left && !above && !above_right) && (!left && this_matches && right) && (below_left && below && below_right)) tile_result = 101;
				if ((!above) && (left && this_matches && right) && (below)) tile_result = 102;
				if ((!above_left && !above && !above_right) && (left && this_matches && !right) && (below_left && below && below_right)) tile_result = 103;
				if ((!above_left && !above && !above_right) && (left && !this_matches && !right) && (below_left && below && !below_right)) tile_result = 104;

				if ((!above_left && !above && above_right) && (!left && this_matches && right) && (below && below_right)) tile_result = 110;
				if ((!above_left && above && above_right) && (left && this_matches && right) && (below_left && below && below_right)) tile_result = 111;
				if ((above_left && above && !above_right) && (left && this_matches && right) && (below_left && below && below_right)) tile_result = 113;
				if ((above_left && !above && !above_right) && (left && this_matches && !right) && (below_left && below)) tile_result = 114;

				if ((above) && (!left && this_matches && right) && (below)) tile_result = 120;
				if ((above_left && above) && (left && this_matches && !right) && (below)) tile_result = 124;

				if ((above && above_right) && (!left && this_matches && right) && (!below_left && !below)) tile_result = 130;
				if ((above_left && above && above_right) && (left && this_matches && right) && (!below_left && below && below_right)) tile_result = 131;
				if ((above_left && above && above_right) && (left && this_matches && right) && (below_left && below && !below_right)) tile_result = 133;
				if ((above_left && above) && (left && this_matches && !right) && (below_left && !below && !below_right)) tile_result = 134;

				if ((!above_left && above && above_right) && (!left && !this_matches && right) && (!below_left && !below && !below_right)) tile_result = 140;
				if ((above_left && above && above_right) && (!left && this_matches && right) && (!below_left && !below && !below_right)) tile_result = 141;
				if ((above_left && above && above_right) && (left && this_matches && right) && (!below)) tile_result = 142;
				if ((above_left && above && above_right) && (left && this_matches && !right) && (!below_left && !below && !below_right)) tile_result = 143;
				if ((above_left && above && !above_right) && (left && !this_matches && !right) && (!below_left && !below && !below_right)) tile_result = 144;

				if ((!above_left && !above && !above_right) && (!left && this_matches && right) && (!below_left && below && below_right)) tile_result = 150;
				if ((!above_left && !above && !above_right) && (left && this_matches && !right) && (below_left && below && !below_right)) tile_result = 151;
				if ((!above_left && above && above_right) && (!left && this_matches && right) && (!below_left && !below && !below_right)) tile_result = 160;
				if ((above_left && above && !above_right) && (left && this_matches && !right) && (!below_left && !below && !below_right)) tile_result = 161;

				if ((!above_left && !above && !above_right) && (!left && this_matches && !right) && (!below_left && below && !below_right)) tile_result = 152;
				if ((above) && (!left && this_matches && !right) && (!below_left && !below && !below_right)) tile_result = 153;
				if ((!above_left && !above && !above_right) && (left && this_matches && !right) && (!below_left && !below && !below_right)) tile_result = 162;
				if ((!above) && (!left && this_matches && right) && (!below)) tile_result = 163;

				if ((above) && (!left && this_matches && !right) && (below)) tile_result = 154;
				if ((!above) && (left && this_matches && right) && (!below)) tile_result = 164;

				if ((!above_left && above && above_right) && (left && this_matches && right) && (!below_left && below && below_right)) tile_result = 180;
				if ((above_left && above && above_right) && (left && this_matches && right) && (!below_left && below && !below_right)) tile_result = 181;

				if (tile_result != -1) map.set_tile(x, y, BEARY_TILE_LAYER_0, tile_result);
			}


				//
				// test for solids foreground tiles
				//

				bool this_solid = StyleDrafter::matches(map, x, y, {11, 12 });

				bool left_solid = StyleDrafter::matches(map, x-1, y, {11, 12});
				bool right_solid = StyleDrafter::matches(map, x+1, y, {11, 12});
				bool above_solid = StyleDrafter::matches(map, x, y-1, {11, 12});
				bool below_solid = StyleDrafter::matches(map, x, y+1, {11, 12});
				bool above_left_solid = StyleDrafter::matches(map, x-1, y-1, {11, 12});
				bool above_right_solid = StyleDrafter::matches(map, x+1, y-1, {11, 12});
				bool below_left_solid = StyleDrafter::matches(map, x-1, y+1, {11, 12});
				bool below_right_solid = StyleDrafter::matches(map, x+1, y+1, {11, 12});

				if (this_solid)
				{
					map.set_tile(x, y, BEARY_TILE_LAYER_1, 1);

					if (!below_solid)
					{
						if (left_solid && right_solid) map.set_tile(x, y, BEARY_TILE_LAYER_1, random_int({95, 96}));
						else if (!left_solid && right_solid) map.set_tile(x, y, BEARY_TILE_LAYER_1, 94);
						else if (left_solid && !right_solid) map.set_tile(x, y, BEARY_TILE_LAYER_1, 97);
						else if (!left_solid && !right_solid) map.set_tile(x, y, BEARY_TILE_LAYER_1, 90);
					}

				}
				else if (!this_solid)
				{
					// if to the right, left
					if (left_solid && right_solid)
					{
						if (above_solid || (!above_solid && !above_left_solid && !above_right_solid)) map.set_tile(x, y, BEARY_TILE_LAYER_1, 99);
						else if (!above_left_solid && above_right_solid) map.set_tile(x, y, BEARY_TILE_LAYER_1, 98);
						else if (above_left_solid && !above_right_solid) map.set_tile(x, y, BEARY_TILE_LAYER_1, 93);
					}
					else if (left_solid && !right_solid && (!above_left_solid || above_solid)) map.set_tile(x, y, BEARY_TILE_LAYER_1, 98);
					else if (!left_solid && right_solid && (!above_right_solid || above_solid)) map.set_tile(x, y, BEARY_TILE_LAYER_1, 93);

				}


				bool is_platform = StyleDrafter::matches(map, x, y, {1, 5, 8, 9, 11, 14, 15, 16}); // is a platform block 
				bool below_is_platform = StyleDrafter::matches(map, x, y+1, {1, 5, 8, 9, 11, 14, 15, 16}); // is a platform block 

				// platforms
				if (is_platform && !above_solid)
				{
					// add some platform topper! :D
					map.set_tile(x, y, BEARY_TILE_LAYER_2, 11);
				}
				if (!this_solid && below_is_platform)
				{
					// add some grass on the top!
					map.set_tile(x, y, BEARY_TILE_LAYER_2, random_int({80, 81, 82}));
				}


				//
				// ladders and destructables
				//

				if (StyleDrafter::matches(map, x, y, {BLOCK_DESTRUCTABLE}))
				{
					map.set_tile(x, y, BEARY_TILE_LAYER_1, random_int({70, 71, 72}));
					//continue;
				}

				if (StyleDrafter::matches(map, x, y, {BLOCK_LADDER, BLOCK_LADDER_TOP, BLOCK_LADDER_PLATFORM}))
				{
					map.set_tile(x, y, BEARY_TILE_LAYER_1, 17);
					//continue;
				}
			}
	}

	if (map.style == "grassland" || map.style=="dungeon")
	{
		//
		// set the background color and process the tiles
		//

		float opa = 0.05;
		foreground_color = al_map_rgba_f(230.0/255*opa,213.0/255*opa,18.0/255*opa,opa);
		background_color = al_map_rgb(213, 242, 255);

		//map.background_color = makecol(0, 0, 0);
		//map.background_color = makecol(232,234,255);
		//map.add_background("clouds3.png", true, true);

		for (unsigned y=0; y<current_map->get_height(); y++)
			for (unsigned x=0; x<current_map->get_width(); x++)
			{
				// destructable
				if (StyleDrafter::matches(map, x, y, {BLOCK_DESTRUCTABLE}))
				{
					map.set_tile(x, y, BEARY_TILE_LAYER_1, random_int({70, 71, 72}));
				}
				// solid blocks
				if (StyleDrafter::matches(map, x, y, {11, 12}))
				{
					// draw them as blocks
					if (StyleDrafter::any_surrounding_tile_does_not_match(map, x, y, {11, 12}))
						map.set_tile(x, y, StyleDrafter::matches(map, x, y, {12}) ? BEARY_TILE_LAYER_2 : BEARY_TILE_LAYER_0, 1);
					else
						map.set_tile(x, y, BEARY_TILE_LAYER_0, random_bool() ? (random_bool() ? 2 : 3) : 4);

					// put some grass on top
					if (!StyleDrafter::above_matches(map, x, y, {11, 12}))
						map.set_tile(x, y, BEARY_TILE_LAYER_1, 10);
				}
				if (StyleDrafter::matches(map, x, y, {BLOCK_LADDER}))
				{
					if (!StyleDrafter::above_is_ladder(map, x, y))
					{
						map.set_tile(x, y, BEARY_TILE_LAYER_1, 19); // a ladder top
					}
					else if (!StyleDrafter::below_is_ladder(map, x, y))
					{
						map.set_tile(x, y, BEARY_TILE_LAYER_1, 18); // a ladder bottom
					}
					else
					{
						map.set_tile(x, y, BEARY_TILE_LAYER_1, 17); // a ladder
					}
				}
				if (StyleDrafter::matches(map, x, y, {BLOCK_DEATH})) // death block
				{
					map.set_tile(x, y, BEARY_TILE_LAYER_1, 41); // for now

					if (StyleDrafter::right_matches(map, x, y, {18}))
					{
						VisualFX::spawn(x*16+16, y*16, 0.5, 1.0, "fireloop", true, al_map_rgb(230, 87, 18));
						animation_sequence *last_seq = VisualFX::get_last_created_sequence();
						last_seq->playhead = random_float(0, 1) * last_seq->get_duration();
						last_seq->update();
					}
				}
			}

	}
	else
	{
		std::cout << "[__post_process_map() error] map.style \"" << map.style << "\" does not exist." << std::endl;
	}



	if (map.style == "dungeon")
	{
		background_color = al_map_rgba_f(0.15, 0.18, 0.2, 1.0);
		foreground_color = al_map_rgba_f(0.1, 0.15, 0.2, 0.5);
	}

	std::cout << "post process completed." << std::endl;
}






////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


int draw_tile(ALLEGRO_BITMAP *b, int tile_index_num, int xx, int yy)
{
	//if ((tile_index_num < 0) || (tile_index_num > map_graphics.size()-1)) return 1;
	al_draw_text(game_font, al_color_name("white"), xx, yy, 0, tostring(tile_index_num).c_str());
	//draw_sprite(b, map_graphics[tile_index_num].bmp, xx, yy);
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// CAMERA

class camera_class
{
public:
    float x; // the left
    float y; // the top


    float bottom() { return y + SCREEN_H; }
    float right() { return x + SCREEN_W; }

    void prevent_scrolling_off_map()
    {
        if (x < 0) x = 0;
        if (y < 0) y = 0;

        if ((x+SCREEN_W) > current_map->get_width()*TILE_SIZE)
        {
            x = current_map->get_width()*TILE_SIZE - SCREEN_W;
            //if (x < 0) center(current_map.center());
        }
        if ((y+SCREEN_H) > current_map->get_height()*TILE_SIZE)
        {
            y = current_map->get_height()*TILE_SIZE - SCREEN_H;
            //if (y < 0) middle(current_map.middle());
        }
    }

};



camera_class camera;





#include <sstream>
using namespace std;

template< class Type >
std::string ToString( Type value )
{
   std::stringstream s;
   s << value;
   return s.str();
}



void play_ball_bounce_sound(float screen_pos_x)
{
    float pan;
    pan = (float)screen_pos_x/SCREEN_W;

    pan = (pan) * 255;
    play_sample(ball_bounce_sample, 64, (int)pan, 900+(rand()%201), false);
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




bool is_trigger_block(int tile_num)
{                                                                            
    if ((tile_num >= BLOCK_TRIGGER) && (tile_num < BLOCK_TRIGGER+9999))
    {
        return true;
    }

    return false;
}


class trigger_class
{
public:
    int index_num;
    int map_num;
    int tile_x;
    int tile_y;
    bool active;

    trigger_class(int _index_num, int _map_num, int tile_xx, int tile_yy)
    {
        tile_x = tile_xx;
        tile_y = tile_yy;
        index_num = _index_num;
        map_num = _map_num;
        active = false;
    }

    void turn_on() { active = true; }

    void turn_off() { active = false; }

    void draw(int map_num_to_draw)
    {
        if (map_num_to_draw == map_num)
        {
            if (active) { if (trigger_on) draw_sprite(buffer, trigger_on, tile_x*16 - (int)camera_x, tile_y*16 - (int)camera_y - 10); }
            else { if (trigger_off) draw_sprite(buffer, trigger_off, tile_x*16 - (int)camera_x, tile_y*16 - (int)camera_y - 10); }
        }
    }
};



#include <algorithm>

bool operator< (const trigger_class &tc1, const trigger_class &tc2)
{
   return tc1.index_num < tc2.index_num;
}



class trigger_manger_class
{
public:
   vector<trigger_class> trigger;
   vector<int> push_order;

   trigger_manger_class()
   {
      clear_all();
   }

   void clear_all()
   {
      for (int i=trigger.size()-1; i>=0; i--) trigger.pop_back();
      clear_push_order();
   }

   void clear_push_order()
   {
      for (int i=push_order.size()-1; i>=0; i--) push_order.pop_back();
   }

   void add_trigger_to_list(int index_num, int map_num, int tile_xx, int tile_yy)
   {
      bool number_already_used = false;
      for (int i=0; i<trigger.size(); i++)
      {
         if ((trigger[i].index_num == index_num) && (trigger[i].map_num == map_num)) number_already_used = true;
      }

      if (!number_already_used)
      {
         trigger.push_back(trigger_class(index_num, map_num, tile_xx, tile_yy));
      }
   }



   void make_all_inactive()
   {
      for (int i=0; i<trigger.size(); i++) trigger[i].turn_off();
   }

   void make_all_active()
   {
      for (int i=0; i<trigger.size(); i++) trigger[i].turn_on();
   }

   void turn_on(int index_num)
   {
      for (int i=0; i<trigger.size(); i++)
      {
         if ((trigger[i].index_num == index_num) && (!trigger[i].active))
         {
             trigger[i].turn_on();
             push_order.push_back(index_num);
         }
      }
   }

   void turn_off(int index_num)
   {
      for (int i=0; i<trigger.size(); i++)
      {
         if ((trigger[i].index_num == index_num)  && (trigger[i].active)) trigger[i].turn_off();
      }
   }


   bool are_all_active()
   {
      for (int i=0; i<trigger.size(); i++) { if (!trigger[i].active) return false; }
      return true;
   }

   // put this in the main layer thing
   void draw()
   {
      for (int i=0; i<trigger.size(); i++) trigger[i].draw(get_current_map_index());
   }


   bool first_pushed_is_not_least()
   {
      for (int p=0; p<push_order.size(); p++)
      {
          for (int i=0; i<trigger.size(); i++)
          {
              if ((trigger[i].index_num < push_order[p]) && (!trigger[i].active)) return true;
          }
      }

      return false;
   }

   bool triggers_activated_in_order()
   {
      if (first_pushed_is_not_least()) return false;

      if (push_order.size() == 1) return true;

      for (int i=1; i<push_order.size(); i++)
      {
         if (push_order[i-1] > push_order[i]) return false;
      }
      return true;
   }

};


trigger_manger_class trigger_manager;


void attempt_to_turn_on_trigger(int index_num)
{
    trigger_manager.turn_on(index_num);
}


void test_for_incorrect_order()
{
    if (!trigger_manager.triggers_activated_in_order())
    {
       trigger_manager.make_all_inactive();
       trigger_manager.clear_push_order();
       // play_error_sound();
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#define DEFAULT_BULLET_X_VELOCITY   4.0f
#define DEFAULT_BULLET_Y_VELOCITY   -1.75f


float f_abs(float f)
{
   if (f < 0.0f) return f * -1.0f;
   return f;
}


class gun_bullet_class
{
public:
    float x;
    float y;

    float velocity_x;
    float velocity_y;

    bool active;

    gun_bullet_class(float xx, float yy, float velocity_xx, float velocity_yy, bool shooting_right)
    {
        x = xx;
        y = yy;
        velocity_x = DEFAULT_BULLET_X_VELOCITY;
        if (!shooting_right) velocity_x *= -1;
        velocity_x += velocity_xx;
        velocity_y = DEFAULT_BULLET_Y_VELOCITY + velocity_yy;

        active = true;
    }

    bool outside_screen()
    {
         if ((int)x-(int)camera.x < 0-64) return true;
         if ((int)x-(int)camera.x > SCREEN_W+64) return true;
         if ((int)y-(int)camera.y < 0-64) return true;
         if ((int)y-(int)camera.y > SCREEN_H+64) return true;
         return false;
    }



    void test_vertical_map_collisions()
    {
        bool char_collides_next = false;
        bool char_collides_now = false;

        if (velocity_y > 0.0f) // falling
        {
            if (block_type_at((int)(x), (int)(y+velocity_y)+1) == BLOCK_DESTRUCTABLE) return;
            if (is_platform_block(block_type_at((int)(x), (int)(y+velocity_y)+1)))
            {
                char_collides_next = true;
            }
            if (is_platform_block(block_type_at((int)(x), (int)y)))
            {
                char_collides_now = true;
            }
        }
        if ((!char_collides_now) && (char_collides_next))
        {
            velocity_y *= -0.8f;
            y = block_top_edge(((int)(y+velocity_y+8)+1)/16) - 1;
            if (velocity_y < -1.0f) play_ball_bounce_sound(x - camera.x);
        }
        else if (velocity_y < 0.0f) // jumping
        {
            if (block_type_at((int)(x), (int)(y+velocity_y)-1) == BLOCK_DESTRUCTABLE) return;
            if (is_ceiling_block(block_type_at((int)(x), (int)(y+velocity_y)-1)))
            {
                velocity_y *= -0.8f;
                y = block_bottom_edge(((int)(y+velocity_y-8)-1)/16) + 1;
                play_ball_bounce_sound(x - camera.x);
            }
        }
    }

    void test_horizontal_map_collisions()
    {
        bool char_collides_next = false;
        bool char_collides_now = false;

        if (velocity_x > 0.0f) // falling
        {
            if (block_type_at((int)(x+velocity_x)+1, (int)(y)) == BLOCK_DESTRUCTABLE) return;
            if (is_left_wall_block(block_type_at((int)(x+velocity_x)+1, (int)(y))))
            {
                char_collides_next = true;
            }
            if (is_left_wall_block(block_type_at((int)(x), (int)y)))
            {
                char_collides_now = true;
            }
        }
        if ((!char_collides_now) && (char_collides_next))
        {
            velocity_x *= -1.0f;
            x = block_left_edge(((int)(x+velocity_x+8)+1)/16) - 1;
            play_ball_bounce_sound(x - camera.x);
        }

        else if (velocity_x < 0.0f) // jumping
        {
            if (block_type_at((int)(x+velocity_x)-1, (int)(y)) == BLOCK_DESTRUCTABLE) return;
            if (is_right_wall_block(block_type_at((int)(x+velocity_x)-1, (int)(y))))
            {
                velocity_x *= -1.0f;
                x = block_right_edge(((int)(x+velocity_x-8)-1)/16) + 1;
                play_ball_bounce_sound(x - camera.x);
            }
        }
    }


    void ricochet()
    {
        velocity_x *= -1.0f;
        velocity_y *= -1.0f;
    }

    void update()
    {
        if (active)
        {
           if (velocity_x > 0) velocity_x -= 0.03f;
           else if (velocity_x < 0) velocity_x += 0.03f;
           velocity_y += GRAVITY;
           if (velocity_y > TERMINAL_VELOCITY) velocity_y = TERMINAL_VELOCITY;

           test_vertical_map_collisions();
           y += velocity_y;
           test_horizontal_map_collisions();
           x += velocity_x;

           float min_motion_range = 0.05f;
           if ((f_abs(velocity_x) < min_motion_range) && (f_abs(velocity_y) < min_motion_range)) active = false;
        }

        if (outside_screen()) active = false;
    }

    void kill()
    {
        active = false;
    }

    void draw()
    {
        if (active) draw_sprite(buffer, bullet_img, (int)x-al_get_bitmap_width(bullet_img)/2-(int)camera.x, (int)y-al_get_bitmap_height(bullet_img)/2-(int)camera.y);
    }
};


class gun_bullet_manager_class
{
public:
    vector<gun_bullet_class> bullet;

    void add_bullet(float x, float y, float velocity_x, float velocity_y, bool shooting_right)
    {
        bullet.push_back(gun_bullet_class(x, y, velocity_x, velocity_y, shooting_right));
    }

    void clear_all()
    {
        bullet.empty();
    }

    void update()
    {
        for (int i=0; i<bullet.size(); i++)
        {
            bullet[i].update();
            if (!bullet[i].active) bullet.erase(bullet.begin()+i);
        }
    }

    void draw()
    {
        for (int i=0; i<bullet.size(); i++) bullet[i].draw();
    }
};


gun_bullet_manager_class bullet_manager;




void play_use_key_sound();
void draw_item_centered(int item_type, int x, int y);

void play_secret_found_sound();
void start_door_found_animation(float start_xx, float start_yy, float end_xx, float end_yy);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "visual_fx/sparkle_fx.hpp"

sparkle_effect_manager_class particle_effect_manager;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "visual_fx/block_destruction_fx.hpp"

block_destruction_class block_destruction_manager;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "visual_fx/use_key_fx.hpp"

use_key_animation_manager_class use_key_animation_manager;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include "visual_fx/surround_sparkle_fx.hpp"

solved_door_animation_manager_class solved_door_animation_manager;

void start_door_found_animation(float start_xx, float start_yy, float end_xx, float end_yy)
{
    solved_door_animation_manager.clear_all();
    solved_door_animation_manager.start_effect(start_xx, start_yy, end_xx, end_yy);
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


bool is_switch_block(int tile_num)
{                                                                            
    if ((tile_num >= (BLOCK_SWITCH_BLOCK)) && (tile_num < (BLOCK_SWITCH_BLOCK+8)))
    {
        return true;
    }

    return false;
}

bool is_switch_stick(int tile_num)
{
    if ((tile_num >= BLOCK_SWITCH_STICK) && (tile_num < BLOCK_SWITCH_STICK+8))
    {
        return true;
    }

    return false;
}





bool player_on_switch_block = false;

void check_switch_behind_player()
{
     int block_below_block_behind_player = block_type_at(player.get_center_int(), player.get_middle_int()+16);
     int block_behind_player = block_type_at(player.get_center_int(), player.get_middle_int()+16+16);

     bool good_to_go = false;
     int switch_num = -1;

     if (is_switch_stick(block_behind_player)) { good_to_go = true; switch_num = block_behind_player-BLOCK_SWITCH_STICK; }
     if (is_switch_stick(block_below_block_behind_player)) { good_to_go = true; switch_num = block_behind_player-BLOCK_SWITCH_STICK; }

     if (good_to_go)
     {
         if (!player_on_switch_block)
         {
            player_on_switch_block = true;
            play_switch_sound();
            //swap_switch_stick(switch_num);
            //my_error_message2("got here %d", switch_num);
         }
     }
     else
     {
         player_on_switch_block = false;
     }
}





/*
void draw_switch_blocks_and_switches(int tile_num, int tile_h, int tile_k, int xx, int yy)
{
   switch_block_struct *sbs;
   int block_number = tile_num;

   if (is_switch_block(tile_num))
   {
       block_number -= BLOCK_SWITCH_BLOCK;
   }
   else if (is_switch_stick(tile_num))
   {
       block_number -= BLOCK_SWITCH;
   }



   switch (block_number)
   {
       case 0:
            sbs = &switch_block[0];
            break;
       case 1:
            sbs = &switch_block[1];
            break;
       case 2:
            sbs = &switch_block[2];
            break;
       case 3:
            sbs = &switch_block[3];
            break;
       case 4:
            sbs = &switch_block[4];
            break;
       case 5:
            sbs = &switch_block[5];
            break;
       case 6:
            sbs = &switch_block[6];
            break;
       case 7:
            sbs = &switch_block[7];
            break;
       case 8:
            sbs = &switch_block[8];
            break;
       default:
            return;
            break;
   }



   if (is_switch_block(tile_num))
   {
      if (is_switch_active(tile_num))
      {
         if (sbs->block_on) draw_sprite(buffer, sbs->block_on, (tile_h*TILE_SIZE)-xx, (tile_k*TILE_SIZE)-yy);
      }
      else
      {
         if (sbs->block_off) draw_sprite(buffer, sbs->block_off, (tile_h*TILE_SIZE)-xx, (tile_k*TILE_SIZE)-yy);
      }
   }



   if (is_switch_stick(tile_num))
   {
      if (is_switch_active(tile_num+100000))
      {
         if (sbs->switch_on) draw_sprite(buffer, sbs->switch_on, (tile_h*TILE_SIZE)-xx, (tile_k*TILE_SIZE)-yy-16-8);
      }
      else
      {
         if (sbs->switch_off) draw_sprite(buffer, sbs->switch_off, (tile_h*TILE_SIZE)-xx, (tile_k*TILE_SIZE)-yy-16-8);
      }
   }

}
*/



int limit(int n, int lower, int upper) {
  return std::max(lower, std::min(n, upper));
}

void draw_map_layer_SPECIAL_TILES(int layer, int xx=0, int yy=0)
{
     layer--;
     if (layer < 0) layer = 0;
     else if (layer > 2) layer = 2;

     int h=0;
     int k=0;
     //int the_number_of_tiles_in_the_maps_index = current_map.num_of_tiles();
     bool done_with_loop = false;


			//al_lock_bitmap(atlas, ALLEGRO_LOCK_READWRITE, ALLEGRO_PIXEL_FORMAT_ANY);

		int padding = -1;

		int minx = xx / 16 + padding;
		int maxx = (xx + SCREEN_W) / 16 - padding;
		int miny = yy / 16 + padding;
		int maxy = (yy + SCREEN_H) / 16 - padding;


		// limit
		minx = limit(minx, 0, current_map->get_width());
		maxx = limit(maxx, 0, current_map->get_width());
		miny = limit(miny, 0, current_map->get_height());
		maxy = limit(maxy, 0, current_map->get_height());


     for (k=miny; k<=maxy; k++)
     {
         for (h=minx; h<=maxx; h++)
         {
             if (layer == 2)
             {
                 //if (current_map.tile[h][k].boundary == BLOCK_DESTRUCTABLE) draw_sprite(buffer, destructable_block, (h*TILE_SIZE)-xx, (k*TILE_SIZE)-yy);
                 if (current_map->get_tile(h, k, BEARY_TILE_LAYER_BOUNDARY) == BLOCK_DESTRUCTABLE) draw_sprite(buffer, destructable_block, (h*TILE_SIZE)-xx, (k*TILE_SIZE)-yy);
             }

             if (layer == 1)
             {
                 //draw_switch_blocks_and_switches(current_map.tile[h][k].boundary, h, k, xx, yy);
                 //draw_switch_blocks_and_switches(current_map->get_tile(h, k, BEARY_TILE_LAYER_BOUNDARY), h, k, xx, yy);
             }
         }
     }

		//al_unlock_bitmap(atlas);
}


void draw_boundary_block(ALLEGRO_BITMAP *b, int x, int y, int boundary_block, ALLEGRO_COLOR col=YELLOW)
{
      switch (boundary_block)
      {
           case BLOCK_BOUNDRY_1:
                hline(b, x, y, x+15, col);  // top
                break;
           case BLOCK_BOUNDRY_2:
                vline(b, x+15, y, y+15, col);  // right
                break;
           case BLOCK_BOUNDRY_3:
                hline(b, x, y+15, x+15, col);  // bottom
                break;
           case BLOCK_BOUNDRY_4:
                vline(b, x, y, y+15, col);  // left
                break;
           case BLOCK_BOUNDRY_5:
                hline(b, x, y, x+15, col);  // top
                vline(b, x+15, y, y+15, col);  // right
                break;
           case BLOCK_BOUNDRY_6:
                vline(b, x+15, y, y+15, col);  // right
                hline(b, x, y+15, x+15, col);  // bottom
                break;
           case BLOCK_BOUNDRY_7:
                hline(b, x, y+15, x+15, col);  // bottom
                vline(b, x, y, y+15, col);  // left
                break;
           case BLOCK_BOUNDRY_8:
                hline(b, x, y, x+15, col);  // top
                vline(b, x, y, y+15, col);  // left
                break;
           case BLOCK_BOUNDRY_9:
                hline(b, x, y, x+15, col);  // top
                hline(b, x, y+15, x+15, col);  // bottom
                break;
           case BLOCK_BOUNDRY_10:
                vline(b, x+15, y, y+15, col);  // right
                vline(b, x, y, y+15, col);  // left
                break;
           case BLOCK_BOUNDRY_11:
                hline(b, x, y, x+15, col);  // top
                vline(b, x+15, y, y+15, col);  // right
                hline(b, x, y+15, x+15, col);  // bottom
                vline(b, x, y, y+15, col);  // left
                break;
           case BLOCK_BOUNDRY_12:
                break;
           case BLOCK_BOUNDRY_13:
                vline(b, x+15, y, y+15, col);  // right
                hline(b, x, y+15, x+15, col);  // bottom
                vline(b, x, y, y+15, col);  // left
                break;
           case BLOCK_BOUNDRY_14:
                hline(b, x, y, x+15, col);  // top
                hline(b, x, y+15, x+15, col);  // bottom
                vline(b, x, y, y+15, col);  // left
                break;
           case BLOCK_BOUNDRY_15:
                hline(b, x, y, x+15, col);  // top
                vline(b, x+15, y, y+15, col);  // right
                vline(b, x, y, y+15, col);  // left
                break;
           case BLOCK_BOUNDRY_16:
                hline(b, x, y, x+15, col);  // top
                vline(b, x+15, y, y+15, col);  // right
                hline(b, x, y+15, x+15, col);  // bottom
                break;
           case BLOCK_LADDER_TOP:
           case BLOCK_LADDER_PLATFORM:
                hline(b, x, y+2, x+15, makecol(255,128,0));
                hline(b, x, y+3, x+15, makecol(255,128,0));
           case BLOCK_LADDER:
                hline(b, x, y, x+15, makecol(255,128,0));
                hline(b, x, y+4, x+15, makecol(255,128,0));
                hline(b, x, y+8, x+15, makecol(255,128,0));
                hline(b, x, y+12, x+15, makecol(255,128,0));
                break;
           case BLOCK_UNLOCK_CAMERA:
                line(b, x+0, y+0, x+15, y+15, makecol(128,255,128));
                line(b, x+15, y+0, x, y+15, makecol(128,255,128));
                break;
           case BLOCK_GOAL_1:
                hline(b, x, y, x+15, RED);  // top
                vline(b, x+15, y, y+15, RED);  // right
                hline(b, x, y+15, x+15, RED);  // bottom
                vline(b, x, y, y+15, RED);  // left
                break;
           case BLOCK_DEATH:
                line(b, x+0, y+0, x+15, y+15, makecol(255,0,0));
                line(b, x+15, y+0, x, y+15, makecol(255,0,0));
                break;
           case BLOCK_DESTRUCTABLE:
                hline(b, x, y, x+15, makecol(255,0,128));  // top
                vline(b, x+15, y, y+15, makecol(255,0,128));  // right
                hline(b, x, y+15, x+15, makecol(255,0,128));  // bottom
                vline(b, x, y, y+15, makecol(255,0,128));  // left

                hline(b, x, y+4, x+15, makecol(255,0,128));
                hline(b, x, y+12, x+15, makecol(255,0,128));
                break;
           default:
                if ((boundary_block >= BLOCK_SWITCH_STICK) && (boundary_block < BLOCK_SWITCH_STICK+8))
                {
                   hline(b, x, y, x+15, makecol(255, 128, 0));  // top
                   vline(b, x+15, y, y+15, makecol(255, 128, 0));  // right
                   hline(b, x, y+15, x+15, makecol(255, 128, 0));  // bottom
                   vline(b, x, y, y+15, makecol(255, 128, 0));  // left
                   textprintf(b, font_x_small, x+2, y+1, makecol(255,255,255), "%d", boundary_block-BLOCK_SWITCH_STICK);
                }
                else if ((boundary_block >= BLOCK_SWITCH_BLOCK) && (boundary_block < BLOCK_SWITCH_BLOCK+8))
                {
                   hline(b, x, y, x+15, makecol(255, 64, 0));  // top
                   vline(b, x+15, y, y+15, makecol(255, 64, 0));  // right
                   hline(b, x+5, y+15, x+15, makecol(255, 64, 0));  // bottom
                   vline(b, x, y, y+15-5, makecol(255, 64, 0));  // left
                   line(b, x, y+10, x+5, y+15, makecol(255, 64, 0));  // bottom left diagonal
                   //textprintf(b, font_x_small, x+2, y+1, makecol(255,255,255), "%d", boundary_block-BLOCK_CAMERA);

                }
                else if ((boundary_block >= BLOCK_TRIGGER) && (boundary_block < BLOCK_TRIGGER+99999))
                {
                   hline(b, x, y, x+15, makecol(64, 255, 0));  // top
                   vline(b, x+15, y, y+15, makecol(64, 255, 0));  // right
                   hline(b, x, y+15, x+15, makecol(64, 255, 0));  // bottom
                   vline(b, x, y, y+15, makecol(64, 255, 0));  // left

                   vline(b, x+8, y-6, y, makecol(64, 255, 0)); // stem
                   hline(b, x+3+1, y-6, x+15-3, makecol(64, 255, 0));  // top

                   textprintf(b, font_x_small, x+2, y+1, makecol(255,255,255), "%d", boundary_block-BLOCK_TRIGGER);
                }
                break;
      }

}


void draw_map_boundarys(ALLEGRO_BITMAP *b, int xx, int yy)
{
     int h=0;
     int k=0;

     for (k=0; k<current_map->get_height(); k++)
     {
         for (h=0; h<current_map->get_width(); h++)
         {
              //draw_boundary_block(b, h*TILE_SIZE-xx, k*TILE_SIZE-yy, current_map.tile[h][k].boundary);
              draw_boundary_block(b, h*TILE_SIZE-xx, k*TILE_SIZE-yy, current_map->get_tile(h, k, BEARY_TILE_LAYER_BOUNDARY));
         }
     }
}


/*
void draw_map(void)
{
}

void draw_map_front_layer(void)
{
}
*/



////////////////////////////////////////////////////////////////////////////////
// TESTING

int2 world_to_tile_coordinates(float world_x, float world_y)
{
     return int2(world_x/16, world_y/16);
}

int block_type_at(float world_x, float world_y)
{
	int2 tile_coords = world_to_tile_coordinates(world_x, world_y);
	return current_map->get_tile(tile_coords.x, tile_coords.y, BEARY_TILE_LAYER_BOUNDARY);
}

int block_type_at_tile_coord(float tile_x, float tile_y)
{
	return current_map->get_tile(tile_x, tile_y, BEARY_TILE_LAYER_BOUNDARY);
}

/*
int graphic_block_type_at(int x, int y)
{
     return current_map->get_tile(x/16, y/16, BEARY_TILE_LAYER_BOUNDARY);
}
*/

int block_left_edge(int tile_x)
{
     return (tile_x*16);
}

int block_right_edge(int tile_x)
{
     return ((tile_x+1)*16);
}

int block_top_edge(int tile_y)
{
     return (tile_y*16);
}

int block_bottom_edge(int tile_y)
{
     return ((tile_y+1)*16);
}




bool is_platform_block(int type)
{
    if (type == BLOCK_BOUNDRY_1) return true;
    if (type == BLOCK_BOUNDRY_5) return true;
    if (type == BLOCK_BOUNDRY_8) return true;
    if (type == BLOCK_BOUNDRY_9) return true;
    if (type == BLOCK_BOUNDRY_11) return true;
    if (type == BLOCK_BOUNDRY_14) return true;
    if (type == BLOCK_BOUNDRY_15) return true;
    if (type == BLOCK_BOUNDRY_16) return true;

    if (type == BLOCK_LADDER_TOP) return true;
    if (type == BLOCK_LADDER_PLATFORM) return true;

    if (type == BLOCK_DESTRUCTABLE) return true;
    if (is_switch_block(type) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    //if ((type >= BLOCK_SWITCH_BLOCK) && (type < BLOCK_SWITCH_BLOCK+10) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    if ((type >= BLOCK_TRIGGER) && (type < BLOCK_TRIGGER+8)) return true;
    if (is_switch_stick(type)) return true;
    else return false;
}

bool is_passable_platform_block(int type)
{
    if (type == BLOCK_BOUNDRY_1) return true;
    if (type == BLOCK_BOUNDRY_5) return true;
    if (type == BLOCK_BOUNDRY_8) return true;
    if (type == BLOCK_BOUNDRY_9) return true;
    //if (type == BLOCK_BOUNDRY_11) return true;
    if (type == BLOCK_BOUNDRY_14) return true;
    if (type == BLOCK_BOUNDRY_15) return true;
    if (type == BLOCK_BOUNDRY_16) return true;

    if (type == BLOCK_LADDER_TOP) return true;
    if (type == BLOCK_LADDER_PLATFORM) return true;

    //if ((type >= BLOCK_SWITCH_BLOCK) && (type < BLOCK_SWITCH_BLOCK+10) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    if ((type >= BLOCK_TRIGGER) && (type < BLOCK_TRIGGER+8)) return true;
    if (is_switch_stick(type)) return true;
    else return false;
}

bool is_ceiling_block(int type)
{
    if (type == BLOCK_BOUNDRY_3) return true;
    if (type == BLOCK_BOUNDRY_6) return true;
    if (type == BLOCK_BOUNDRY_7) return true;
    if (type == BLOCK_BOUNDRY_9) return true;
    if (type == BLOCK_BOUNDRY_11) return true;
    if (type == BLOCK_BOUNDRY_13) return true;
    if (type == BLOCK_BOUNDRY_14) return true;
    if (type == BLOCK_BOUNDRY_16) return true;
    if (type == BLOCK_DESTRUCTABLE) return true;
    if (is_switch_block(type) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    //if ((type >= BLOCK_SWITCH_BLOCK) && (type < BLOCK_SWITCH_BLOCK+10) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    if ((type >= BLOCK_TRIGGER) && (type < BLOCK_TRIGGER+8)) return true;
    if (is_switch_stick(type)) return true;
    else return false;
}

bool is_right_wall_block(int type)
{
    if (type == BLOCK_BOUNDRY_4) return true;
    if (type == BLOCK_BOUNDRY_7) return true;
    if (type == BLOCK_BOUNDRY_8) return true;
    if (type == BLOCK_BOUNDRY_10) return true;
    if (type == BLOCK_BOUNDRY_11) return true;
    if (type == BLOCK_BOUNDRY_13) return true;
    if (type == BLOCK_BOUNDRY_14) return true;
    if (type == BLOCK_BOUNDRY_15) return true;
    if (type == BLOCK_DESTRUCTABLE) return true;
    if (is_switch_block(type) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    //if ((type >= BLOCK_SWITCH_BLOCK) && (type < BLOCK_SWITCH_BLOCK+10) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    if ((type >= BLOCK_TRIGGER) && (type < BLOCK_TRIGGER+8)) return true;
    if (is_switch_stick(type)) return true;
    else return false;
}

bool is_left_wall_block(int type)
{
    if (type == BLOCK_BOUNDRY_2) return true;
    if (type == BLOCK_BOUNDRY_5) return true;
    if (type == BLOCK_BOUNDRY_6) return true;
    if (type == BLOCK_BOUNDRY_10) return true;
    if (type == BLOCK_BOUNDRY_11) return true;
    if (type == BLOCK_BOUNDRY_13) return true;
    if (type == BLOCK_BOUNDRY_15) return true;
    if (type == BLOCK_BOUNDRY_16) return true;
    if (type == BLOCK_DESTRUCTABLE) return true;
    if (is_switch_block(type) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    //if ((type >= BLOCK_SWITCH_BLOCK) && (type < BLOCK_SWITCH_BLOCK+10) && is_switch_active(type-BLOCK_SWITCH_BLOCK)) return true;
    if ((type >= BLOCK_TRIGGER) && (type < BLOCK_TRIGGER+8)) return true;
    if (is_switch_stick(type)) return true;
    else return false;
}


bool is_top_edge_block(int type)
{
	return is_platform_block(type);
}

bool is_bottom_edge_block(int type)
{
	return is_ceiling_block(type);
}

bool is_left_edge_block(int type)
{
	return is_left_wall_block(type);
}

bool is_right_edge_block(int type)
{
	return is_right_wall_block(type);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


class LevelTitle
{
private:
	static LevelTitle *instance;
	ALLEGRO_FONT *font;
	std::string text;
	float x, y;
	float timer;

	LevelTitle()
		//: font(al_load_font("fonts/arialbd.ttf", 15, 0))
		: font(al_load_font("fonts/Kemco Smooth Bold.ttf", -8, 0))

		, text("unknown")
		, x(20)
		, y(SCREEN_H - 20)
		, timer(0)
	{}

	static LevelTitle *get_instance()
	{
		if (!instance) instance = new LevelTitle();
		return instance;
	}

public:
	static void spawn(std::string text)
	{
		LevelTitle *inst = LevelTitle::get_instance();
		inst->text = text;
		inst->timer = 1;
	}

	static void clear()
	{
		LevelTitle *inst = LevelTitle::get_instance();
		inst->timer = 0;
	}

	static void update_and_draw()
	{
		LevelTitle *inst = LevelTitle::get_instance();

		inst->timer -= 0.002;
		if (inst->timer <= 0.001) return;

		float opacity = 1;
		if (inst->timer > 0.9) opacity = (1.0 - inst->timer) * 10;
		if (inst->timer < 0.1) opacity = inst->timer * 10;

		ALLEGRO_COLOR font_color = al_map_rgba_f(opacity, opacity, opacity, opacity);
		write_outline(inst->x, inst->y, inst->font, font_color, inst->text.c_str());
		//al_draw_text(inst->font, al_map_rgba_f(opacity, opacity, opacity, opacity), inst->x, inst->y, 0, inst->text.c_str());
	}
};

LevelTitle *LevelTitle::instance = NULL;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////





class level_record_struct
{
public:
	int index;
	std::string identifier;


	level_record_struct(int index, std::string identifier)
		: index(index)
		, identifier(identifier)
	{}

	level_record_struct()
		: index(-1)
		, identifier("unidentified")
	{
	}

	std::string get_title()
	{
		std::string result = identifier;
		std::replace(result.begin(), result.end(), '_', ' ');
		return result;
	}

	std::string get_doors_file()
	{
		std::stringstream ss;
		ss << "NEWlevels/" << identifier << "/" << identifier << ".doors";
		return ss.str();
	}
	std::string get_checkpoints_file()
	{
		std::stringstream ss;
		ss << "NEWlevels/" << identifier << "/" << identifier << ".checkpoints";
		return ss.str();
	}
	std::string get_camera_regions_file(int map_num)
	{
		std::stringstream ss;
		ss << "NEWlevels/" << identifier << "/" << identifier << "." << map_num << ".camera_regions";
		return ss.str();
	}
	std::string get_tile_map_file(int map_num)
	{
		std::stringstream ss;
		ss << "NEWlevels/" << identifier << "/" << identifier << "." << map_num << ".tile_map";
		return ss.str();
	}
	std::string get_items_file()
	{
		std::stringstream ss;
		ss << "NEWlevels/" << identifier << "/" << identifier << ".items";
		return ss.str();
	}
	std::string get_enemies_file()
	{
		std::stringstream ss;
		ss << "NEWlevels/" << identifier << "/" << identifier << ".enemies";
		return ss.str();
	}
	std::string get_tmx_file(int map_num)
	{
		std::stringstream ss;
		ss << "NEWlevels/" << identifier << "/" << identifier << "." << map_num << ".tmx";
		return ss.str();
	}
};


void load_music_HACK(std::string style)
{
	if (style == "grassland")
	{
		multitrack_music->clear();
		multitrack_music->add_music_layer(0, "music/open_world/beary-open_world_music-03.wav");
		multitrack_music->play(0);
	}
	else if (style == "dungeon")
	{
		multitrack_music->clear();
		multitrack_music->add_music_layer(0, "music/deep_dungeon/layer0.ogg");
		multitrack_music->add_music_layer(1, "music/deep_dungeon/layer1.ogg");
		multitrack_music->add_music_layer(2, "music/deep_dungeon/layer2.ogg");
		multitrack_music->add_music_layer(3, "music/deep_dungeon/layer3.ogg");
		multitrack_music->add_music_layer(4, "music/deep_dungeon/layer4.ogg");
		multitrack_music->add_music_layer(5, "music/deep_dungeon/layer5.ogg");
		multitrack_music->add_music_layer(6, "music/deep_dungeon/layer6.ogg");
		multitrack_music->add_music_layer(7, "music/deep_dungeon/layer7.ogg");
		multitrack_music->add_music_layer(8, "music/deep_dungeon/layer8.ogg");
		multitrack_music->play(0);
	}
}


//bool load_tile_map_file(std::string filename);


#include <tmx.h>

class TMXLoader
{
private:
	static tmx_layer *get_layer_by_name(tmx_map *m, std::string name)
	{
		tmx_layer *found_layer = m->ly_head;
		if (!m || !found_layer)
		{
			std::cout << "This tmx map does not have a ly_head (layer_head)" << std::endl;
			return NULL;
		}	

		do
		{
			//if (found_layer->name == name.c_str())  // <- this won't work, yo ;)
			if (strcmp(found_layer->name, name.c_str()) == 0)
				return found_layer;
		} while (found_layer = found_layer->next);

		return NULL;
	}

public:
	static bool load_tmx_file(std::string filename, int as_level_num, int as_map_num)
	{
		tmx_map *m = tmx_load(filename.c_str()); // crashes when there's an empty layer
		if (!m)
		{
			std::cout << "There was an error when loading the tmx file by tmx_load()" << std::endl;
			tmx_perror("tmxlib reported the following error");
         return false;
		}

		//
		// parse the map
		//

		// properties

		std::string style = "";
		std::string title = "-Untitled-";
		if (m->properties)
		{
			tmx_property *prop = m->properties;
			do
			{
				if (strcmp(prop->name, "title") == 0) title = prop->value;
				if (strcmp(prop->name, "style") == 0) style = prop->value;
			} while(prop = prop->next);
		}

		ALLEGRO_BITMAP *tile_atlas_bmp = al_load_bitmap("images/beary_tileset_03.png");
		current_map->new_tile_atlas->load(tile_atlas_bmp, 16, 16, 1);
		al_destroy_bitmap(tile_atlas_bmp);
		current_map->title = title;
		current_map->style = style;
		current_map->resize(m->width, m->height);
		current_map->use_tile_atlas(current_map->new_tile_atlas);


		//
		// parse the tile layers
		//

		tmx_layer *tile_layer = NULL;

		// boundary

		tile_layer = TMXLoader::get_layer_by_name(m, "boundary");
		if (!tile_layer)
		{
			std::cout << "ERROR: could not find the \"boundary\" tile layer." << std::endl;
		}
		else if (tile_layer->type != L_LAYER)
		{
			std::cout << "ERROR: layer \"boundary\" found, but is not a tile type (of type L_LAYER), skipping." << std::endl;
		}
		else
		{
			for (unsigned y=0; y<m->height; y++)
				for (unsigned x=0; x<m->width; x++)
				{
					int tile = tile_layer->content.gids[x + y*m->width];
					current_map->set_tile(x, y, BEARY_TILE_LAYER_BOUNDARY, tile);
				}
		}

		// background

		tile_layer = TMXLoader::get_layer_by_name(m, "background");
		if (!tile_layer)
		{
			std::cout << "ERROR: could not find the \"background\" tile layer." << std::endl;
		}
		else if (tile_layer->type != L_LAYER)
		{
			std::cout << "ERROR: layer \"background\" found, but is not a tile type (of type L_LAYER), skipping." << std::endl;
		}
		else
		{
			for (unsigned y=0; y<m->height; y++)
				for (unsigned x=0; x<m->width; x++)
				{
					int tile = tile_layer->content.gids[x + y*m->width];
					//current_map->set_tile(x, y, BEARY_TILE_LAYER_0, std::max(0, tile - 65));
					current_map->set_tile(x, y, BEARY_TILE_LAYER_BACKGROUND, tile - 65);
				}
		}

		// layer[0]

		tile_layer = TMXLoader::get_layer_by_name(m, "layer[0]");
		if (!tile_layer)
		{
			std::cout << "error: could not find the \"layer[0]\" tile layer." << std::endl;
		}
		else if (tile_layer->type != L_LAYER)
		{
			std::cout << "error: layer \"layer[0]\" found, but is not a tile type (of type l_layer), skipping." << std::endl;
		}
		else
		{
			for (unsigned y=0; y<m->height; y++)
				for (unsigned x=0; x<m->width; x++)
				{
					int tile = tile_layer->content.gids[x + y*m->width];
					//current_map->set_tile(x, y, BEARY_TILE_LAYER_0, std::max(0, tile - 65));
					current_map->set_tile(x, y, BEARY_TILE_LAYER_0, tile - 65);
				}
		}

		// layer[1]

		tile_layer = TMXLoader::get_layer_by_name(m, "layer[1]");
		if (!tile_layer)
		{
			std::cout << "ERROR: could not find the \"layer[1]\" tile layer." << std::endl;
		}
		else if (tile_layer->type != L_LAYER)
		{
			std::cout << "ERROR: layer \"layer[1]\" found, but is not a tile type (of type L_LAYER), skipping." << std::endl;
		}
		else
		{
			for (unsigned y=0; y<m->height; y++)
				for (unsigned x=0; x<m->width; x++)
				{
					int tile = tile_layer->content.gids[x + y*m->width];
					//current_map->set_tile(x, y, BEARY_TILE_LAYER_1, std::max(0, tile - 65));
					current_map->set_tile(x, y, BEARY_TILE_LAYER_1, tile - 65);
				}
		}

		// layer[2]

		tile_layer = TMXLoader::get_layer_by_name(m, "layer[2]");
		if (!tile_layer)
		{
			std::cout << "ERROR: could not find the \"layer[2]\" tile layer." << std::endl;
		}
		else if (tile_layer->type != L_LAYER)
		{
			std::cout << "ERROR: layer \"layer[2]\" found, but is not a tile type (of type L_LAYER), skipping." << std::endl;
		}
		else
		{
			for (unsigned y=0; y<m->height; y++)
				for (unsigned x=0; x<m->width; x++)
				{
					int tile = tile_layer->content.gids[x + y*m->width];
					//current_map->set_tile(x, y, BEARY_TILE_LAYER_2, std::max(0, tile - 65));
					current_map->set_tile(x, y, BEARY_TILE_LAYER_2, tile - 65);
				}
		}


		//
		// parse the camera region layer
		//

		tmx_layer *camera_region_layer = TMXLoader::get_layer_by_name(m, "CameraRegions");
		if (camera_region_layer && (camera_region_layer->type == L_OBJGR))
		{
			tmx_object *obj = camera_region_layer->content.objgr->head;
			while (obj)
			{
				if (obj->shape == S_SQUARE)
				{
					CameraRegions::add_camera_region(obj->x, obj->y, obj->x + obj->width, obj->y + obj->height);
				}
				else
				{
					std::cout << "warning: non-square object found in CameraRegions layer" << std::endl;
				}
				obj = obj->next;
			};

		}
		else
		{
			std::cout << "ERROR: could not find the CameraRegions tile layer." << std::endl;
		}


		//
		// parse the Objects layer (checkpoints, doors, enemies)
		//

		tmx_layer *object_layer = TMXLoader::get_layer_by_name(m, "Objects");
		if (object_layer && (object_layer->type == L_OBJGR))
		{
			tmx_object *obj = object_layer->content.objgr->head;
			while (obj)
			{
				if (!obj->name)
				{
					std::cout << "warning: unnamed object found in \"Objects\" layer; skipping." << std::endl;
				}
				else if (strcmp(obj->name, "switch_stick") == 0)
				{
					int switch_num = -1;

					if (obj->properties)
					{
						tmx_property *prop = obj->properties;
						do
						{
							if (strcmp(prop->name, "switch_num") == 0) switch_num = atoi(prop->value);
						} while(prop = prop->next);
					}

					if (switch_num == -1) std::cout << "missing \"switch_num\" property in switch_stick" << std::endl;
					Switches::add_switch_stick(obj->x+obj->width/2, obj->y+obj->height, switch_num);
					std::cout << "switch_stick added" << std::endl;
				}
				else if (strcmp(obj->name, "checkpoint") == 0)
				{
					Checkpoints::add_checkpoint(1, as_level_num, as_map_num, obj->x, obj->y);
					//std::cout << "checkpoint added" << std::endl;
				}
				else if (strcmp(obj->name, "player_spawn") == 0)
				{
					//Checkpoints::add_checkpoint(1, as_level_num, as_map_num, obj->x, obj->y);
					//std::cout << "checkpoint added" << std::endl;
					player.map_pos.x = obj->x+obj->width/2;
					player.map_pos.y = obj->y+obj->height - player.size.y-4;
					player.velocity.x = 0;
					player.velocity.y = 0;
				}
				else if (strcmp(obj->name, "item") == 0)
				{
					std::string type = obj->type;

/*
					if (obj->properties)
					{
						tmx_property *prop = obj->properties;
						do
						{
							if (strcmp(prop->name, "type") == 0) type = prop->value;
						} while(prop = prop->next);
					}
*/

					//static bool add_item(int world_num, int level_num, int map_num, float x, float y, float align_x, float align_y, item_t type)
					Items::add_item(1, as_level_num, as_map_num, obj->x+obj->width/2, obj->y+obj->height/2, 0.5, 0.5, str_to_item_type(type));
					//Enemies::add_enemy(1, as_level_num, as_map_num, obj->x, obj->y-48, type);
					//std::cout << "enemy \"" << type << "\" added" << std::endl;
				}
				else if (strcmp(obj->name, "enemy") == 0)
				{
					std::string type = obj->type;
					//Enemies::add_enemy(1, level_index, map_index, 200, 100, "medusa head");

/*
					if (obj->properties)
					{
						tmx_property *prop = obj->properties;
						do
						{
							if (strcmp(prop->name, "type") == 0) type = prop->value;
						} while(prop = prop->next);
					}
*/
					Enemies::add_enemy_spawner(1, as_level_num, as_map_num, obj->x, obj->y, obj->width, obj->height, type);
					//std::cout << "enemy \"" << type << "\" added" << std::endl;
				}
				else if (strcmp(obj->name, "door") == 0)
				{
					// skip the door adding... this is done globally when loading the world
				}
				else
				{
					std::cout << "unrecognized object \"" << obj->name << "\"" << std::endl;
				}

				obj = obj->next;
			};
				
		}
		else
		{
			std::cout << "ERROR: could not find the Objects tile layer." << std::endl;
		}
		

		//
		// post-process (if necessairy)
		//
		
		if (!current_map->style.empty()) __post_process_map(current_map);

		//
		// cleanup
		//

		// delete the tmx parsed map
		tmx_map_free(m);

      return true;
	}
	static bool append_doors_from_tmx_file(std::string filename, int as_level_num, int as_map_num)
	{
		tmx_map *m = tmx_load(filename.c_str()); // crashes when there's an empty layer
		if (!m)
		{
			tmx_perror("error");
			return false;
		}


		//
		// parse the Objects layer (checkpoints, doors, enemies)
		//

		tmx_layer *object_layer = TMXLoader::get_layer_by_name(m, "Objects");
		if (object_layer && (object_layer->type == L_OBJGR))
		{
			tmx_object *obj = object_layer->content.objgr->head;
			while (obj)
			{
				std::cout << "AA";
				if (!obj->name)
				{
					// make sure to check if it's blank
				}
				else if (strcmp(obj->name, "door") == 0) // extract only the objects named "door"
				{
					std::cout << "BB";
					std::string label = "?";
					int type = DOOR_TYPE_NORMAL;
					int destination_world_num = 1;
					std::string destination_label = "?";

					if (obj->properties)
					{
						std::cout << "CC";
						tmx_property *prop = obj->properties;
						do
						{
							std::cout << "DD";
							if (strcmp(prop->name, "label") == 0) label = prop->value;
							else if (strcmp(prop->name, "type") == 0) type = Doors::get_door_type_from_identifier(prop->value);
							else if (strcmp(prop->name, "destination_world_num") == 0) destination_world_num = atoi(prop->value);
							else if (strcmp(prop->name, "destination_label") == 0) destination_label = prop->value;
						} while(prop = prop->next);
					}
					else
					{
					}

					// coordinate of the new door will be at the bottom center of the object, or at (x, y) if the 
					Doors::add_door(1, as_level_num, as_map_num, obj->x+obj->width/2, obj->y+obj->height, label, type, destination_world_num, destination_label);
					//std::cout << "door added" << std::endl;
				}

				obj = obj->next;
			};
		}
		

		//
		// cleanup
		//

		// delete the tmx parsed map
		tmx_map_free(m);

      return true;
	}
};



class World
{
private:
	static World *instance;
	static World *get_instance()
	{
		if (!instance) instance = new World();
		return instance;
	}
	std::vector<level_record_struct> levels;
	int current_level;
	int current_map;
	World()
		: levels()
		, current_level(-1)
		, current_map(-1)
	{}
	
public:

	static bool get_level_record_struct_by_index_id(int index_id, level_record_struct *level_record)
	{
		std::vector<level_record_struct> &levels = World::get_instance()->levels;
		for (unsigned i=0; i<levels.size(); i++)
		{
			if (levels[i].index == index_id)
			{
				*level_record = levels[i];
				return true;
			}
		}
		return false;
	}
	static int get_current_level_index()
	{
		return World::get_instance()->current_level;
	}
	static int get_current_map_index()
	{
		return World::get_instance()->current_map;
	}
	static void add_level(int index, std::string identifier)
	{
		World *inst = get_instance();
		level_record_struct new_level(index, identifier);
		inst->levels.push_back(new_level);
		inst->current_level = -1;
		inst->current_map = -1;
	}
	static void clear()
	{
		World *inst = World::get_instance();
		inst->levels.clear();
	}
	static void load(std::string level_filename)
	{
		// 1) clear the current stuff
		clear();

		// 1) load the level file (which has a list of each level and level number)
		//add_level(0, "deep_dungeon");
		//add_level(0, "dungeon_1");
		add_level(0, "switch_stick_class");
		//add_level(0, "style_test");
		//add_level(0, "central_core");
		add_level(1, "central_world_map");
		//add_level(2, "switch_stick_class");
		//add_level(1, "dungeon_1");
		//add_level(2, "deep_dungeon");

		// 2) load the doors and checkpoints from each level
		std::vector<level_record_struct> &levels = World::get_instance()->levels;
		Checkpoints::clear();
		Doors::clear();
		Switches::clear();
		Items::clear();
		for (unsigned i=0; i<levels.size(); i++)
		{
			level_record_struct level_to_load_from;
			get_level_record_struct_by_index_id(i, &level_to_load_from);

			int map_index_num_to_test = 0;

			do
			{
				std::cout << "TESTING \"" << map_index_num_to_test << "\" in level \"" << i << "\", AKA \"" << level_to_load_from.identifier << "\")" << std::endl;
				std::string tmx_map_filename = level_to_load_from.get_tmx_file(map_index_num_to_test);
				if (al_filename_exists(tmx_map_filename.c_str()))
				{
					std::cout << "found! located tmx file (for map \"" << map_index_num_to_test << "\" in level \"" << i << "\")" << std::endl;
					TMXLoader::append_doors_from_tmx_file(tmx_map_filename, i, map_index_num_to_test);
					std::cout << " ... loaded" << std::endl;
				}
			} while (map_index_num_to_test++ > 20); // we're just going to try 20 for now

		//	Checkpoints::append_checkpoints_file(levels[i].get_checkpoints_file());
	//		Doors::append_doors_file(levels[i].get_doors_file());
	//		Items::append_items_file(levels[i].get_items_file());
		}
	}
	static void refresh_world_and_current_map()
	{
		int current_level = World::get_current_level_index();
		int current_map = World::get_current_map_index();
		World::load("actual_filename_would_go_here");
		World::load_map(current_level, current_map);
		//World::load_map(World::get_current_level_index(), World::get_current_map_index());
	}
	static bool load_map(int level_index, int map_index)
	{
		World *inst = World::get_instance();

		//TODO validate the map and world indexes are legit
		inst->current_map = map_index;
		inst->current_level = level_index;

		// Clear all the old stuff
		VisualFX::clear();
		Switches::clear();
		CameraRegions::clear();
		DamageZones::clear();
		Enemies::clear();
		Dialogues::clear();
		//Items::clear(); //TODO: actually, this should only be cleared if this is a new level, maybe even "clear progress" if exiting a level or something

		std::vector<level_record_struct> &levels = inst->levels;
		level_record_struct level_to_load_from;
		if (!get_level_record_struct_by_index_id(level_index, &level_to_load_from))
		{
			std::cout << "error loading map" << std::endl;
			return false;
		}

		if (al_filename_exists(level_to_load_from.get_tmx_file(map_index).c_str()))
		{
			// load the tmx file
			if (!TMXLoader::load_tmx_file(level_to_load_from.get_tmx_file(map_index), level_index, map_index))
			{
				std::cout << "There was an error loading the tmx file" << std::endl;
			}
		}
		else
		{
			std::cout << "ERROR: there was not a map.0.tmx file \"" << level_to_load_from.get_tmx_file(map_index).c_str() << "\"" << std::endl;
			//show_notification("Old legacy map loading no longer suppported.", "Use TMX files", GOOD);

/*
			load_tile_map_file(level_to_load_from.get_tile_map_file(map_index));

			// load the enemies
			Enemies::add_enemy(1, level_index, map_index, 200, 100, "medusa head");
			Enemies::add_enemy(1, level_index, map_index, 204, 102, "yellow bear");
			Enemies::add_enemy(1, level_index, map_index, 304, 72, "yellow bear");
*/

			// load the camera regions
//			CameraRegions::load(level_to_load_from.get_camera_regions_file(map_index));

			// load the items
		}

		// load the music
		load_music_HACK(::current_map->style);

		// spawn the title text
		LevelTitle::spawn(::current_map->title);

		return false;
	}
};

World *World::instance = NULL;



int get_current_map_index()
{
	return World::get_current_map_index();
}
int get_current_level_index()
{
	return World::get_current_level_index();
}
int get_current_world_index()
{
	return 1;
}




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void write_outline(int x, int y, ALLEGRO_FONT *f, ALLEGRO_COLOR color, const char *format, ...)
{
   char buf[512];

   va_list ap;
   va_start(ap, format);
   //uvszprintf(buf, sizeof(buf), format, ap);
   vsprintf(buf, format, ap);
   va_end(ap);

    ALLEGRO_COLOR outline_color = al_map_rgba_f(0.0, 0.0, 0.0, 0.4);//makecol(0,0,0);
    ALLEGRO_COLOR text_color = color;


    textprintf(buffer, f, x+1, y, outline_color, buf);
    textprintf(buffer, f, x-1, y, outline_color, buf);
    textprintf(buffer, f, x, y+1, outline_color, buf);
    textprintf(buffer, f, x, y-1, outline_color, buf);
    textprintf(buffer, f, x, y, text_color, buf);
}


void write_outline_center(int x, int y, ALLEGRO_FONT *f, ALLEGRO_COLOR color, const char *format, ...)
{
   char buf[512];

   va_list ap;
   va_start(ap, format);
   //uvszprintf(buf, sizeof(buf), format, ap);
   vsprintf(buf, format, ap);
   va_end(ap);

    ALLEGRO_COLOR outline_color = al_map_rgba_f(0.0, 0.0, 0.0, 0.4);//makecol(0,0,0);
    ALLEGRO_COLOR text_color = color;


    textprintf_centre(buffer, f, x+1, y, outline_color, buf);
    textprintf_centre(buffer, f, x-1, y, outline_color, buf);
    textprintf_centre(buffer, f, x, y+1, outline_color, buf);
    textprintf_centre(buffer, f, x, y-1, outline_color, buf);
    textprintf_centre(buffer, f, x, y, text_color, buf);
}


void write_outline_alfont_aa(int x, int y, ALLEGRO_FONT *f, int size, ALLEGRO_COLOR color, const char *format, ...)
{
   char buf[512];

   va_list ap;
   va_start(ap, format);
   //uvszprintf(buf, sizeof(buf), format, ap);
   vsprintf(buf, format, ap);
   va_end(ap);
   
   alfont_set_font_size(f, size);

    ALLEGRO_COLOR outline_color = makecol(0,0,0);
    ALLEGRO_COLOR text_color = color;

    alfont_textprintf(buffer, f, x+1, y, outline_color, buf);
    alfont_textprintf(buffer, f, x-1, y, outline_color, buf);
    alfont_textprintf(buffer, f, x, y+1, outline_color, buf);
    alfont_textprintf(buffer, f, x, y-1, outline_color, buf);
    alfont_textprintf_aa(buffer, f, x, y, text_color, buf);
}

int get_min(int t)
{
    return (int)(((float)t/(float)100)/(float)60);
}    
int get_sec(int t)
{
	return (t - ((int)(((float)t/(float)100)/(float)60))*6000)/100;
}    
int get_hun_sec(int t)
{
    return t%100;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void set_game_state(int new_state)
{
	if (_game_state == new_state) return;

	switch(new_state)
	{
		case STATE_TITLE_SCREEN:
			multitrack_music->clear();
			multitrack_music->add_music_layer(0, "music/title_screen/beary_theme-07-loopable.wav");
			multitrack_music->play(0);
		break;
		// syncronize the play position if crossfading between two tracks
	}

	std::cout << "setting game state to " << get_game_state_str(game_state_t(new_state)) << "." << std::endl;
	_game_state = new_state;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#define PICKUP_ITEM_DURATION 2.0
#define PICKUP_ITEM_GRAVITY 3.4f


struct picked_up_item_struct
{
    float x_pos;
    float y_pos;
    float counter;
    int item_type;
};

vector<picked_up_item_struct> picked_up_item;



void begin_drawing_picked_up_item(int item_type, float x_pos, float y_pos)
{
    picked_up_item_struct puis;
    puis.counter = PICKUP_ITEM_DURATION;
    puis.item_type = item_type;
    puis.x_pos = x_pos;
    puis.y_pos = y_pos;
    
    puis.y_pos -= 20;
   
	picked_up_item.clear(); 
    picked_up_item.push_back(puis);
}    

void update_picked_up_items()
{
    for (int k=0; k<picked_up_item.size(); k++)
    {
        picked_up_item[k].counter -= GAMEPLAY_FRAME_DURATION;
			if (picked_up_item[k].counter < 0)
			{
				picked_up_item.erase(picked_up_item.begin() + k--);
			}
    }    
}    


void draw_picked_up_items()
{
    int padding = 12;
    int item_spacing = 16;
    int x_cursor = 0;

    ALLEGRO_BITMAP *img;




    for (int k=0; k<picked_up_item.size(); k++)
    {
			float y_offset = 0;
			if (picked_up_item[k].counter > 1.8) y_offset = 6 - (2.0 - picked_up_item[k].counter) * 30;

        if (!(picked_up_item[k].counter < 0))
        {
            switch (picked_up_item[k].item_type)
            {

#define ADD_ITEM(xx, yy, zz)     case xx##_const: img = xx##_img; break;
//#include "item_list.hpp"
	ITEM_LIST
#undef ADD_ITEM
            }

            if (img) draw_sprite(buffer, img,
                                 player.get_center_int() - al_get_bitmap_width(img)/2 - (int)camera.x,
                                 player.get_top_edge_int() - padding + x_cursor - al_get_bitmap_height(img)/2 - (int)camera.y + (int)y_offset);
            x_cursor -= item_spacing;
        }    
    }    
}    




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void _draw_ms_background(ALLEGRO_BITMAP *buffer, ALLEGRO_BITMAP *bg, int x_off, int y_off, float rate)
{
    int x_point = x_off - (int)(camera_x*rate);
    int y_point = y_off - (int)(camera_y*rate);

    masked_blit(bg, buffer, 0, 0, x_point, y_point, al_get_bitmap_width(bg), al_get_bitmap_height(bg));
}


void _draw_ms_background_loop_x(ALLEGRO_BITMAP *b, ALLEGRO_BITMAP *bg, int x_off, int y_off, float rate)
{
    x_off = -x_off + (int)(camera_x*rate);
    int y_point = y_off - (int)(camera_y*rate);

    int covered_width = -x_off;
    
    while (covered_width > 0) covered_width -= al_get_bitmap_width(bg);
    while (covered_width < -al_get_bitmap_width(bg)) covered_width += al_get_bitmap_width(bg);
       
    while (covered_width < SCREEN_W)
    {
        masked_blit(bg, buffer, 0, 0, covered_width, y_point, al_get_bitmap_width(bg), al_get_bitmap_height(bg));
        covered_width += al_get_bitmap_width(bg);
    }    
}

void _draw_ms_background_loop_y(ALLEGRO_BITMAP *b, ALLEGRO_BITMAP *bg, int x_off, int y_off, float rate)
{
    y_off = -y_off + (int)(camera_y*rate);
    int x_point = x_off - (int)(camera_x*rate);

    int covered_height = -y_off;
    
    while (covered_height > 0) covered_height -= al_get_bitmap_height(bg);
    while (covered_height < -al_get_bitmap_height(bg)) covered_height += al_get_bitmap_height(bg);
       
    while (covered_height < SCREEN_H)
    {
        masked_blit(bg, buffer, 0, 0, x_point, covered_height, al_get_bitmap_width(bg), al_get_bitmap_height(bg));
        covered_height += al_get_bitmap_height(bg);
    }    
}

void _draw_ms_background_loop_x_y(ALLEGRO_BITMAP *b, ALLEGRO_BITMAP *bg, int x_off, int y_off, float rate)
{
    x_off = -x_off + (int)(camera_x*rate);
    y_off = -y_off + (int)(camera_y*rate);

    int covered_width = -x_off;
    int covered_height = -y_off;
    
    while (covered_width > 0) covered_width -= al_get_bitmap_width(bg);
    while (covered_width < -al_get_bitmap_width(bg)) covered_width += al_get_bitmap_width(bg);
    while (covered_height > 0) covered_height -= al_get_bitmap_height(bg);
    while (covered_height < -al_get_bitmap_height(bg)) covered_height += al_get_bitmap_height(bg);

    int orig_covered_height = covered_height;
       
    while (covered_width < SCREEN_W)
    {
        while (covered_height < SCREEN_H)
        {
            masked_blit(bg, buffer, 0, 0, covered_width, covered_height, al_get_bitmap_width(bg), al_get_bitmap_height(bg));
            covered_height += al_get_bitmap_height(bg);
        }    
        covered_width += al_get_bitmap_width(bg);
        covered_height = orig_covered_height;
    }    
}

void draw_ms_background(ALLEGRO_BITMAP *b, ALLEGRO_BITMAP *bg, int x_off, int y_off, float rate, bool loop_x=false, bool loop_y=false)
{
    if (loop_x && loop_y) _draw_ms_background_loop_x_y(b, bg, x_off, y_off, rate);
    else if (loop_x) _draw_ms_background_loop_x(b, bg, x_off, y_off, rate);
    else if (loop_y) _draw_ms_background_loop_y(b, bg, x_off, y_off, rate);
    else _draw_ms_background(b, bg, x_off, y_off, rate);
}    

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <vector>
std::vector<std::string> explode(const std::string &delimiter, const std::string &str)
{
	// note: this function skips multiple delimiters, e.g. it will not return return empty tokens
    std::vector<std::string> arr;

    int strleng = str.length();
    int delleng = delimiter.length();
    if (delleng==0)
        return arr;//no change

    int i=0;
    int k=0;
    while(i<strleng)
    {
        int j=0;
        while (i+j<strleng && j<delleng && str[i+j]==delimiter[j])
            j++;
        if (j==delleng)//found delimiter
        {
            arr.push_back(str.substr(k, i-k));
            i+=delleng;
            k=i;
        }
        else
        {
            i++;
        }
    }
    arr.push_back(str.substr(k, i-k));
    return arr;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void draw_stat_box(int x, int y, int w, ALLEGRO_COLOR col = al_map_rgba_f(0, 0, 0, 0.6))
{
	int h = 9;
	int hw = w/2;
	al_draw_filled_rectangle(x-hw+2, y+2, x-hw+w-2, y+h-2, col);

	al_draw_rectangle(x-hw+0.5, y+0.5, x-hw+w-0.5, y+h-0.5, col, 1.0);
}


void draw_stat_font(int x, int y, int num)
{
	//alfont_textprintf_centre_aa(buffer, new_font, 244+10+6+al_get_bitmap_width(small_jewel_img)+3+9-10+(int)(xx)-1+13, 10+2-3+y_offset, WHITE, "x%d", player_coins);
	//al_draw_text(game_font, al_color_name("firebrick"), x, y+1, ALLEGRO_ALIGN_CENTRE, tostring(num).c_str());
	//al_draw_text(game_font, WHITE, x, y, ALLEGRO_ALIGN_CENTRE, tostring(num).c_str());
	write_outline_center(x, y, game_font, WHITE, tostring(num).c_str());
}


void draw_player_stats_keys__half_aa(float xx=0.0f)
{
    int y_offset = 0;

    int space_fixer = 0;
    if (player_coins > 9) space_fixer = 4;
    set_multiply_blender(255,255,255,255-128);

    if (player.get_keys() > 0) xx -= 15;
    if (player.get_silver_keys() > 0) xx -= 15;

    //draw_trans_sprite(buffer, stat_bground, 244+(int)(xx)-1+13, 10+y_offset);
		draw_stat_box(244+(int)(xx)-1+13+20, 10+y_offset, (player_coins > 99) ? 50 : 40);
    if (small_jewel_img)
    {
       draw_sprite(buffer, small_jewel_img, 244+18-space_fixer-10+(int)(xx)-1+13, 7+3+y_offset+1);

       draw_stat_font(244+10+6+al_get_bitmap_width(small_jewel_img)+3+9-10+(int)(xx)-1+13, 10+2-3+y_offset+2, player_coins);
    }
    xx += 15;
    y_offset += 17;

    if (player.get_keys() > 0)
    {
        /// draw numer of keys left
        int space_fixer = 0;
        if (player.get_keys() > 9) space_fixer = 4;
        set_multiply_blender(255,255,255,255-128);
        //draw_trans_sprite(buffer, stat_bground, 244+(int)(xx)-1+13, 10+y_offset);
			draw_stat_box(244+(int)(xx)-1+13+20, 10+y_offset, 40);
        if (key_img)
        {
           draw_sprite(buffer, key_img, 244+18-space_fixer-10+(int)(xx)-1+13, 7+y_offset);
           draw_stat_font(244+10+6+al_get_bitmap_width(key_img)+3+9-10+(int)(xx)-1+13, 10+2-3+y_offset+2, player.get_keys());
        }

        xx += 15;
        y_offset += 17;
    }

    if (player.get_silver_keys() > 0)
    {
        /// draw numer of keys left
        int space_fixer = 0;
        if (player.get_silver_keys() > 9) space_fixer = 4;
        set_multiply_blender(255,255,255,255-128);
        //draw_trans_sprite(buffer, stat_bground, 244+(int)(xx)-1+13, 10+y_offset);
			draw_stat_box(244+(int)(xx)-1+13+20, 10+y_offset, 40);

        if (silver_key_img)
        {
           draw_sprite(buffer, silver_key_img, 244+18-space_fixer-10+(int)(xx)-1+13, 7+y_offset);
           alfont_textprintf_centre(buffer, new_font, 244+10+6+al_get_bitmap_width(silver_key_img)+3+(int)(xx)-1+13, 10+2-3+y_offset, BLACK, "x%d", player.get_silver_keys());
           alfont_textprintf_centre(buffer, new_font, 244+10+6+al_get_bitmap_width(silver_key_img)+3-1+9-10+(int)(xx)-1+13, 10+2-3+y_offset, BLACK, "x%d", player.get_silver_keys());
           alfont_textprintf_centre(buffer, new_font, 244+10+6+al_get_bitmap_width(silver_key_img)+3+9-10+(int)(xx)-1+13, 10+1+2-3+y_offset, BLACK, "x%d", player.get_silver_keys());
           alfont_textprintf_centre(buffer, new_font, 244+10+6+al_get_bitmap_width(silver_key_img)+3+9-10+(int)(xx)-1+13, 10-1+2-3+y_offset, BLACK, "x%d", player.get_silver_keys());
           alfont_textprintf_centre_aa(buffer, new_font, 244+10+6+al_get_bitmap_width(silver_key_img)+3+9-10+(int)(xx)-1+13, 10+2-3+y_offset, WHITE, "x%d", player.get_silver_keys());
        }
    }
}




void draw_hearts(int x, int y, int energy, int max_energy)
{
	int heart_spacing = 10;
	int num_hearts = max_energy/2;
	int num_full_hearts = energy/2;
	bool has_half_heart = ((energy % 2) == 1);

	int num_extra_hearts = num_hearts - 3;

	draw_stat_box(x+num_extra_hearts*heart_spacing/2, y, 50 + num_extra_hearts*heart_spacing);

	x -= 15;

	for (int i=0; i<num_hearts; i++)
	{
		if (i < num_full_hearts) al_draw_bitmap(stat_life_meter_heart_full, x+heart_spacing*i, y, 0);
		else if ((i==num_full_hearts) && has_half_heart) al_draw_bitmap(stat_life_meter_heart_half, x+heart_spacing*i, y, 0);
		else al_draw_bitmap(stat_life_meter_heart_empty, x+heart_spacing*i, y, 0);
	}
}



void draw_weapon(int x, int y, int energy, int max_energy)
{
	draw_stat_box(x, y, 60);

	x -= 20;

	al_draw_bitmap(shovel_icon, x-4, y-4, 0);
	write_outline_center(x+18+7, y+1, font_x_small, WHITE, "shovel");
}



void draw_hud(float xx=0.0f)
{
	draw_hearts(18+25, 11, player.energy, player.max_energy);



	// draw keys

	draw_player_stats_keys__half_aa(SCREEN_W-310);



	draw_weapon(SCREEN_W-45, SCREEN_H-20, player.energy, player.max_energy);

}


void draw_pen_info()
{
    rectfill(buffer, SCREEN_W-TILE_SIZE*2-6-10, SCREEN_H-TILE_SIZE-6, SCREEN_W-2, SCREEN_H-2, makecol(0,0,0));
    if (drawing_destination == DRAWING_TO_BOUNDRY)
    {
       draw_boundary_block(buffer, SCREEN_W-TILE_SIZE*2-11, SCREEN_H-TILE_SIZE-6+2, drawing_pen_int1, makecol(255,255,0));
       textprintf(buffer, font_x_small, SCREEN_W-TILE_SIZE*2-6+TILE_SIZE/2, SCREEN_H-TILE_SIZE-6-8, makecol(255,255,255), "%d", drawing_pen_int1);
    }
    else
    {
       draw_tile(buffer, pen_from_tile_index, SCREEN_W-TILE_SIZE*2-11, SCREEN_H-TILE_SIZE-6+2);
       textprintf(buffer, font_x_small, SCREEN_W-TILE_SIZE*2-6+TILE_SIZE/2, SCREEN_H-TILE_SIZE-6-8, makecol(255,255,255), "%d", pen_from_tile_index);
    }

    textprintf(buffer, font_x_small, SCREEN_W-50, SCREEN_H-8, makecol(255,255,255), "%d", drawing_destination);
}
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void draw_debug_info()
{
    rectfill(buffer, 1, 1, 90, 13, makecol(0,0,0));
    textprintf(buffer, font_x_small, 2, 4, makecol(255,128,128), "camera |");
    textprintf(buffer, font_x_small, 35, 1, makecol(255,128,128), "-x=%f", camera.x);
    textprintf(buffer, font_x_small, 35, 7, makecol(255,128,128), "-y=%f", camera.y);

    rectfill(buffer, 1+100, 1, 90+100, 13, makecol(0,0,0));
    textprintf(buffer, font_x_small, 2+100, 4, makecol(128,255,128), "player |");
    textprintf(buffer, font_x_small, 36+96, 1, makecol(128,255,128), "-x=%f", player.map_pos.x);
    textprintf(buffer, font_x_small, 36+96, 7, makecol(128,255,128), "-y=%f", player.map_pos.y);

    textprintf(buffer, font_x_small, 2, 12+12, makecol(128,128,96), "Current world index: %d", get_current_world_index());
    textprintf(buffer, font_x_small, 2, 12+19, makecol(128,128,96), "Current level index: %d", get_current_level_index());
    textprintf(buffer, font_x_small, 2, 12+26, makecol(128,128,96), "Current map index: %d", get_current_map_index());
    ALLEGRO_COLOR dum_col = makecol(128, 128, 255);


    if (player.on_ground) textprintf(buffer, font_x_small, SCREEN_W-50, SCREEN_H-10, WHITE, "ON_GROUND");
    textprintf(buffer, font_x_small, SCREEN_W-50, SCREEN_H-18, WHITE, "Ramp: %d", player.on_ramp);


    // show camera lock information
    int spacer = 0;

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void draw_player_bear()
{
        if ((player.post_pain_counter == 0) || (get_game_state() == STATE_TIME_UP) || (player.post_pain_counter%6 <= 2))
        {
				int y = (int)player.map_pos.y + player.size.y - al_get_bitmap_height(bear) - (int)camera.y + 1;
				int x = (int)player.map_pos.x + player.size.x/2 - al_get_bitmap_width(bear)/2 - (int)camera_x;

            if (player.facing_right)
            {
               draw_sprite(buffer, bear, x, y);
               if (player.has_gun) draw_sprite(buffer, holding_gun, x, y+14);
            }
            else
            {
               draw_sprite_h_flip(buffer, bear, x, y);
               if (player.has_gun) draw_sprite_h_flip(buffer, holding_gun, x, y+14);
            }
        }
}





void draw_map_maker_and_debug_stuff(void)
{
	CameraRegions::draw(camera_x, camera_y);

    if (F_debug_mode)
    {
        draw_debug_info();
        draw_boundary_block(buffer, ((mouse_x+(int)camera.x)/TILE_SIZE)*TILE_SIZE-(int)camera.x, ((mouse_y+(int)camera.y)/TILE_SIZE)*TILE_SIZE-(int)camera.y, BLOCK_BOUNDRY_11, makecol(128,128,255));
        textprintf(buffer, font_x_small, 2, SCREEN_H-text_height(font_x_small), makecol(128,128,128), "Debug Mode");
        //textprintf(buffer, font_x_small, 2, SCREEN_H-text_height(font_x_small), makecol(128,128,128), "Map Edit Mode (F4-Save)");
    }
    else if (F_map_maker_mode)
    {
        draw_boundary_block(buffer, ((mouse_x+(int)camera.x)/TILE_SIZE)*TILE_SIZE-(int)camera.x, ((mouse_y+(int)camera.y)/TILE_SIZE)*TILE_SIZE-(int)camera.y, BLOCK_BOUNDRY_11, makecol(255,0,0));
        textprintf(buffer, font_x_small, 2, SCREEN_H-text_height(font_x_small), makecol(128,128,128), "Map Edit Mode (F4-Save)");
        draw_pen_info();
    }

    if (F_map_maker_mode || F_debug_mode)
    {
        rectfill(buffer, SCREEN_W-100, 1, SCREEN_W-15, 9, makecol(0,0,0));
        textprintf(buffer, font_x_small, SCREEN_W-100+2+2, 2, makecol(128,128,255), "Mouse + (%d,%d)", (mouse_x+(int)camera.x)/TILE_SIZE, (mouse_y+(int)camera.y)/TILE_SIZE);

        draw_mouse_cursor();
    }
    if (F_debug_mode || (F_draw_boundarys || ((drawing_destination == DRAWING_TO_BOUNDRY) && (F_map_maker_mode))))
    {
       draw_map_boundarys(buffer, (int)(camera.x), (int)(camera.y));
       //draw_ramps();
    }

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define MAX_BLOOD_DROPS 7



bool point_box_collision(float x, float y, float xx1, float yy1, float xx2, float yy2, float padding)
{
    if (x+padding < xx1) return false;
    if (x-padding > xx2) return false;
    if (y+padding < yy1) return false;
    if (y-padding > yy2) return false;
    return true;
}



#define BULLET_STRENGTH 1

void test_enemy_bullet_collisions()
{
    if (bullet_manager.bullet.size() == 0) return;

    int simultanious_kill_counter=0;
    int points_in_fer=0;

    for (int i=0; i<bullet_manager.bullet.size(); i++)
    {
      if (bullet_manager.bullet[i].active)
      {
     }
    }


}




void test_destructable_block_bullet_collision()
{
    if (bullet_manager.bullet.size() == 0) return;

    for (int i=0; i<bullet_manager.bullet.size(); i++)
    {
       if (block_type_at((int)bullet_manager.bullet[i].x, (int)bullet_manager.bullet[i].y) == BLOCK_DESTRUCTABLE)
       {
           current_map->set_tile((int)bullet_manager.bullet[i].x/16, (int)bullet_manager.bullet[i].y/16, BEARY_TILE_LAYER_BOUNDARY, BLOCK_DESTRUCTED);
           block_destruction_manager.add_particles(bullet_manager.bullet[i].x, bullet_manager.bullet[i].y, bullet_manager.bullet[i].velocity_x, bullet_manager.bullet[i].velocity_y);

           bullet_manager.bullet[i].active = false;

           // spawn destruction effect
           // pickup_item_sound();
       }
    }
}



void play_secret_found_sound()
{
    play_sample(secret_found_sample, 200, 128, 1000, false);
}

void play_menu_move_sound()
{
    play_sample(menu_move, 200, 128, 1000, false);
    //play_sample(menu_move, 200, 128, 900+(rand()%201), false);
}

void play_attack_sound()
{
    play_sample(sword_strike, 200, 128, 1000, false);
} 

void play_menu_select_sound()
{
    play_sample(menu_select, 200, 128, 1000, false); 
}

void play_pause_sound()
{
    play_sample(pause_sample, 200, 128, 1000, false); 
}    

void play_unpause_sound()
{
    play_sample(unpause_sample, 200, 128, 1000, false); 
}    

void play_heart_sound()
{
    play_sample(power_up_sample_heart, 64, 128, 1000, false);
}

void play_switch_sound()
{
    play_sample(menu_move, 200, 128, 900+(rand()%201), false);
}

void play_key_sound()
{
    play_sample(pickup_key_sample, 64, 128, 1000, false);
}

void play_use_key_sound()
{
    play_sample(use_key_sample, 196, 128, 1000, false);
}

void play_coin_sound()
{
    play_sample(pickup_coin_sample, 64, 128, 1000, false);
}

void play_locked_door_sound()
{
    play_sample(door_locked_sample, 64, 128, 1000, false);
}

void play_open_door_sound()
{
    play_sample(door_open_sample, 64, 128, 1000, false);
}


int adding_coins_counter = 0;
int coins_to_be_added = 0;
bool add_coin = true;

void pickup_coins(int num=1)
{
    coins_to_be_added += num;
}



#define ADDING_COINS_SPEED 3
void adding_coins_update()
{
    if ((coins_to_be_added > 0) && (add_coin))
    {
       player_coins++;
       coins_to_be_added--;
       play_coin_sound();
       add_coin = false;
    }


    adding_coins_counter -= 1;
    if (adding_coins_counter < -1) adding_coins_counter = -1;
    if ((adding_coins_counter < 0) && (!add_coin))
    {
        adding_coins_counter = ADDING_COINS_SPEED;
        add_coin = true;
    }
}




void player_collect_item(item_struct &collected_item)
{

		if (collected_item.type == heart_const)
		{
			play_heart_sound();
			collected_item.alive = false;
			player_score += 100;
			player.energy_up(1);
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
		}
		else if (collected_item.type == key_const)
		{
			play_key_sound();
			collected_item.alive = false;
			player_score += 100;
			player.pickup_key();
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
		}
		else if (collected_item.type == silver_key_const)
		{
			play_key_sound();
			player.pickup_silver_key();

			player_score += 100;
			collected_item.alive = false;
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
		}
		else if (collected_item.type == small_jewel_const)
		{
			player_score += 100;
			pickup_coins(1);
			collected_item.alive = false;
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
		}
		else if (collected_item.type == medium_jewel_const)
		{
			player_score += 100;
			pickup_coins(5);
			collected_item.alive = false;
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
		}
		else if (collected_item.type == large_jewel_const)
		{
			player_score += 100;
			pickup_coins(10);
			collected_item.alive = false;
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
		}
		else if (collected_item.type == canister_with_a_heart_const)
		{
			player_score += 100;

			player.max_energy += 2;
			player.energy = player.max_energy;

			collected_item.alive = false;
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
		}
		else if (collected_item.type == jump_shoes_const)
		{
			player_score += 100;

			player.put_on_jumping_shoes();

			collected_item.alive = false;
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
		}
		else if (collected_item.type == gun_const)
		{
			player.pickup_gun();

			player_score += 100;
			collected_item.alive = false;
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
			//player.pickup_key();
		}
		else if (collected_item.type == running_shoes_const)
		{
			player.put_on_running_shoes();

			player_score += 100;
			collected_item.alive = false;
			begin_drawing_picked_up_item(collected_item.type, collected_item.x, collected_item.y);
			//player.pickup_key();
		}

		VisualFX::spawn(collected_item.x,
			collected_item.y,
			0.5, 0.5, "sparkle");
}


void update_player_animation()
{

    if (player.on_ladder)
    {
        player.anim_rate = 10;
        if (player.velocity.y == 0.0f) player.anim_counter = player.anim_rate-1;

        if (player.anim_counter > player.anim_rate)
        {
            if (bear == bear_climb1) bear = bear_climb2;
            else if (bear == bear_climb2) bear = bear_climb1;
            player.anim_counter = 0;
        }
    }
	else if (player.is_downthrusting)
	{
		bear = bear_downthrusting;
	}
    else
    {
        if (player.velocity.x != 0.0f)
        {
            if (player.anim_counter > player.anim_rate)
            {
                if ((bear == bear_downthrusting) || (bear == bear_in_air) || (bear == bear_duck)) bear = bear1;
                else if (bear == bear1) bear = bear2;
                else if (bear == bear2) bear = bear3;
                else if (bear == bear3) bear = bear4;
                else if (bear == bear4) bear = bear1;
                player.anim_counter = 0;
            }
        }
        else if (player.velocity.x == 0.0f)
        {
            bear = bear1;
            player.anim_counter = player.anim_rate; // the bear is ready for the next frame of animation when walking
        }

        if (ABS(player.velocity.x) < 1.5f) player.anim_rate = 10;
        else if (ABS(player.velocity.x) < 2.75f) player.anim_rate = 7;
        else player.anim_rate = 5;

        if (!player.on_ground)
        {
            bear = bear_in_air;
            player.anim_counter = player.anim_rate-1;
        }
        if (player.is_ducking)
        {
            bear = bear_duck;
            player.anim_counter = player.anim_rate-1;
        }

    }

    player.anim_counter++;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

bool release_plus = false;
bool release_minus = false;



int go_in_door(const new_door_struct &door)
{
	new_door_struct destination_door;

	if (!Doors::get_destination_door(door, &destination_door))
	{
		// could not find the door
		std::cout << "door not found";
		std::stringstream error_message;
		error_message << "Destination door (World " << door.destination_world_num
			<< ", door \"" << door.destination_label << "\") not found." << std::endl; 
		start_help_message(error_message.str());
		return -1;
	}


	// door is found. Enter into the door
	
	play_open_door_sound();
	show_loading_notice();



	World::load_map(destination_door.level_num, destination_door.map_num);


			 player.set_center(destination_door.x);
			 player.set_bottom_edge(destination_door.y-0.5);
			player.on_ground = true;
			player.is_downthrusting = false;
			player.velocity.x = 0;
			player.velocity.y = 0;

			clear_input();

	return 0;
}





bool release_f1 = false;
bool release_f2 = false;
bool release_f6 = false;
bool release_f7 = false;
bool release_f8 = false;
bool release_f9 = false;
bool release_f10 = false;
bool release_f11 = false;




int stupid_play_counter = 100;
void fake_play_game()
{
    if (pre_game_counter <= 40)
    {  player.move_right();
       stupid_play_counter--;
       if (stupid_play_counter < 0) {stupid_play_counter = (rand() % 200); player.jump();}
       if (stupid_play_counter <= 100 -20) player.release_jump();
       if ((player.velocity.x == 0.0f) && (pre_game_counter != 40)) player.jump();
    }
}


#define MENU_TITLE 25583
#define MENU_PAUSE 34820
#define MENU_CHOOSE_LEVEL 448156





////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////








int menu_focus=1;
int menu_number=MENU_TITLE;

bool button_down_release=false;
bool button_up_release=false;

void figure_out_title_screen_user_input()
{
    if (button_down && button_down_release) {menu_focus++; button_down_release=false; play_menu_move_sound();}
    else if (!button_down) button_down_release=true;

    if (button_up && button_up_release) {menu_focus--; button_up_release=false; play_menu_move_sound();}
    else if (!button_up) button_up_release=true;

    if (menu_number == MENU_TITLE)
    {
        if (menu_focus == 5) menu_focus = 1;
        if (menu_focus == 0) menu_focus = 4;
    }

    if ((button_a && button_a_release) || (button_b && button_b_release))
    {
       button_a_release=false;
       button_b_release=false;

       if ((menu_number == MENU_TITLE) && (menu_focus == 1))
       {
			  play_menu_select_sound();

           /////////////// START THE GAME RIGHT HERE ////////////////
           /////////////// START THE GAME RIGHT HERE ////////////////
           /////////////// START THE GAME RIGHT HERE ////////////////


				World::load("actual_filename_would_go_here");
				World::load_map(0, 0);
				set_game_state(STATE_IN_LEVEL);
				//player.map_pos.x = 645;
				//player.map_pos.y = 145;


       }
       else if ((menu_number == MENU_TITLE) && (menu_focus == 4))
       {
           abort_game=true;
       }
    }

    if (!button_a) button_a_release = true;
    if (!button_b) button_b_release = true;
}

int pre_pause_gamestate = 0;




void figure_out_pause_screen_user_input(void)
{
    if (button_down && button_down_release) {menu_focus++; button_down_release=false; play_menu_move_sound();}
    else if (!button_down) button_down_release=true;

    if (button_up && button_up_release) {menu_focus--; button_up_release=false; play_menu_move_sound();}
    else if (!button_up) button_up_release=true;

    if (menu_number == MENU_PAUSE)
    {
        if (menu_focus == 4) menu_focus = 1;
        else if (menu_focus == 0) menu_focus = (4)-1;
    }


    if ((button_a && button_a_release) || (button_b && button_b_release))
    {
       button_a_release=false;
       button_b_release=false;

       if (menu_number == MENU_PAUSE)
       {
           if (menu_focus == 1)  // unpause
           {
                 play_unpause_sound();
					GAMEPLAY_FRAME_DURATION = GAMEPLAY_FRAME_DURATION_DEFAULT;
                 if (pre_game_counter > 0)
                 {
                     pre_pause_gamestate = get_game_state();
                     //game_state == STATE_WAITING_FOR_LEVEL_START;
                 }
                 else
                 {
                     set_game_state(pre_pause_gamestate);
                     ten_mili_seconds = clock_time_suspended_at;
                 }        
           }
           else if (menu_focus == 2)  // restart level
           {
					// blank for now
           }
           else if (menu_focus == 3)  // exit to title
           {
              play_menu_select_sound();
              play_unpause_sound();
					GAMEPLAY_FRAME_DURATION = GAMEPLAY_FRAME_DURATION_DEFAULT;

              show_loading_notice();


				//TODO clear all the level stuff here

				set_game_state(STATE_TITLE_SCREEN);



              menu_number = MENU_TITLE;
              menu_focus = 1;
           }

       }
    }

    if (!button_a) button_a_release = true;
    if (!button_b) button_b_release = true;

}

char *char_ptr(const char *c)
{
    string st;
    st = c;
    char *cc = strdup(st.c_str());
    return cc;
}    


void update_award_timers(void)
{
        int k=0;

        k=0;
        award_counter--;
        if (award_counter < 0) award_counter = 0;

        chain_award_counter--;
        if (chain_award_counter < 0) chain_award_counter = 0;
}

void update_pre_game(void)
{
    pre_game_counter--;
    if (pre_game_counter < 0) pre_game_counter = 0;

    if (pre_game_counter > 0) player.move_nowhere();

    if ((pre_game_counter == 0) && (get_game_state() == STATE_WAITING_FOR_LEVEL_START))
    {
        set_game_state(STATE_IN_LEVEL);
        ten_mili_seconds = 0;
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////


void load_game(void)
{
	std::cout << "========== LOADING GAME =========" << std::endl;;



	current_map = new NewBearyMap();



	multitrack_music = new MultitrackMusic();  // create the music!!
	multitrack_music->add_music_layer(0, "music/deep_dungeon/layer0.ogg");
	multitrack_music->add_music_layer(1, "music/deep_dungeon/layer1.ogg");
	multitrack_music->add_music_layer(2, "music/deep_dungeon/layer2.ogg");
	multitrack_music->add_music_layer(3, "music/deep_dungeon/layer3.ogg");
	multitrack_music->add_music_layer(4, "music/deep_dungeon/layer4.ogg");
	multitrack_music->add_music_layer(5, "music/deep_dungeon/layer5.ogg");
	multitrack_music->add_music_layer(6, "music/deep_dungeon/layer6.ogg");
	multitrack_music->add_music_layer(7, "music/deep_dungeon/layer7.ogg");
	multitrack_music->add_music_layer(8, "music/deep_dungeon/layer8.ogg");

	//multitrack_music->start(1);



	VisualFX::build_animation_sequence_from_folder("sparkflare");
	VisualFX::build_animation_sequence_from_folder("fireloop");
	VisualFX::build_animation_sequence_from_folder("explosion");
	VisualFX::build_animation_sequence_from_folder("sparkle");

    destructable_block = _load_bitmap("images/tiles/stone_d_red_broken.bmp");
    destructable_block_shard_1 = _load_bitmap("images/tiles/stone_d_red_shard_1.bmp");
    destructable_block_shard_2 = _load_bitmap("images/tiles/stone_d_red_shard_2.bmp");
    destructable_block_shard_3 = _load_bitmap("images/tiles/stone_d_red_shard_3.bmp");




	switch_struct ss;
	for (unsigned i=0; i<NUM_SWITCH_BLOCKS; i++)
	{
		level_class__switch_stick.push_back(ss);
		create_switch_block_group(&level_class__switch_stick[i], i);
	}

	set_switch_stick(0, 0);
	set_switch_stick(1, 1);
	set_switch_stick(2, 0);
	set_switch_stick(3, 0);

	



    load_locked_door_graphics();
    door_open_sample = load_sample(full_filename("sounds/door_open.ogg"));
    door_locked_sample = load_sample(full_filename("sounds/door_locked.ogg"));
	rock_destruct_sound = al_load_sample("sounds/new/AOL_edit_ROCK.wav");




	shovel_icon = _load_bitmap("images/items/shovel.png");

    bullet_img = _load_bitmap("images/bullett.gif");

    holding_gun = _load_bitmap("images/holding_gun.gif");





	checkpoint = _load_bitmap("images/checkpoint.png");





	paused_banner = _load_bitmap("images/ops/paused_banner.png");


    bear1 = _load_bitmap("images/bears/beary_stand.png");
    bear2 = _load_bitmap("images/bears/beary_walk1.png");
    bear3 = _load_bitmap("images/bears/beary_stand.png");
    bear4 = _load_bitmap("images/bears/beary_walk2.png");
    bear_duck = _load_bitmap("images/bears/beary6-duck.gif");
    bear_climb1 = _load_bitmap("images/bears/beary_climb1.gif");
    bear_climb2 = _load_bitmap("images/bears/beary_climb2.gif");
    bear_in_air = _load_bitmap("images/bears/beary_jump.png");
    bear_downthrusting = _load_bitmap("images/bears/beary_downthrusting.png");

    bear = bear1;



    sparkle_particle_0 = _load_bitmap("images/effects/sparkle_particle_0.gif");
    sparkle_particle_1 = _load_bitmap("images/effects/sparkle_particle_1.gif");
    sparkle_particle_2 = _load_bitmap("images/effects/sparkle_particle_2.gif");
    sparkle_particle_3 = _load_bitmap("images/effects/sparkle_particle_3.gif");




    bad_bear_pic1 = _load_bitmap("images/bears/yellow_bear_1.gif");
    bad_bear_pic2 = _load_bitmap("images/bears/yellow_bear_2.gif");
    blue_bear_pic1 = _load_bitmap("images/bears/blue_bear_1.gif");
    blue_bear_pic2 = _load_bitmap("images/bears/blue_bear_2.gif");
    blue_bear_jump_pic = _load_bitmap("images/bears/blue_bear_jump.gif");
    purple_bear_pic1 = _load_bitmap("images/bears/purple_bear_1.gif");
    purple_bear_pic2 = _load_bitmap("images/bears/purple_bear_2.gif");
    bad_bear2_pic1 = _load_bitmap("images/bears/brown_bear_1.gif");
    bad_bear2_pic2 = _load_bitmap("images/bears/brown_bear_2.gif");

    medusa_head_bmp = _load_bitmap("images/bears/medusa_head.png");



#define ADD_ITEM(xx, yy, zz) { xx##_img = _load_bitmap("images/items/" #xx ".gif", false); \
								if (!xx##_img) xx##_img = _load_bitmap("images/items/" #xx ".png", false); \
								if (!xx##_img) std::cout << "Could not load \"" #xx "\" as .gif or .png " << std::endl; }
//#include "item_list.hpp"
	ITEM_LIST
#undef ADD_ITEM


    load_jewel_shimmers();


	stat_life_meter_heart_full = _load_bitmap("images/ops/life_meter_heart_full.png");
	stat_life_meter_heart_empty = _load_bitmap("images/ops/life_meter_heart_empty.png");
	stat_life_meter_heart_half = _load_bitmap("images/ops/life_meter_heart_half.png");

    life_icon = _load_bitmap("images/ops/life_icon.gif");

    title_graphic = _load_bitmap("images/ops/adventures_of_beary_title_2.png");




	ALLEGRO_BITMAP *small7 = al_load_bitmap("dat/small7.bmp");
	al_convert_mask_to_alpha(small7, makecol(255, 0, 255));
	int ranges[] = {32, 126};
	font_small = al_grab_font_from_bitmap(small7, 1, ranges);

	ALLEGRO_BITMAP *helv15 = al_load_bitmap("dat/helv15.bmp");
	al_convert_mask_to_alpha(helv15, makecol(255, 0, 255));
	font_med = al_grab_font_from_bitmap(helv15, 1, ranges);

	ALLEGRO_BITMAP *xsmall = al_load_bitmap("dat/xsmall.bmp");
	al_convert_mask_to_alpha(xsmall, makecol(255, 0, 255));
	font_x_small = al_grab_font_from_bitmap(xsmall, 1, ranges);

	empty = al_load_bitmap("dat/empty.bmp");

	if ((!empty) || (!font_med) || (!font_small) || (!font_x_small))
	{
	std::cout << "error loading data" << std::endl;
	}




#define ADD_ROCK(x, y) x##y##_sample = load_sample(full_filename("sounds/" #x "_" #y ".ogg"));
	ROCK_LIST
#undef ADD_ROCK




    // trigger

    trigger_on = _load_bitmap("images/trigger_on.gif");
    trigger_off = _load_bitmap("images/trigger_off.gif");


    // sounds

	enemy_hit_sound = load_sample(full_filename("sounds/new/AOL_Sword_Hit.wav"));
	enemy_dead_sound = load_sample(full_filename("sounds/new/AOL_Kill.wav"));
	sword_clang_sound = load_sample(full_filename("sounds/new/AOL_Sword_Clang.wav"));
	item_worth_celebrating_sound = load_sample(full_filename("sounds/new/pickup_inventory_item-01.ogg"));


    sword_strike = load_sample(full_filename("sounds/new/AOL_sword.wav"));
 
    menu_move = load_sample(full_filename("sounds/new/beary_menu_sound-01.ogg"));
    menu_select = load_sample(full_filename("sounds/new/beary_menu_choose_sound-01.ogg"));
    pause_sample = load_sample(full_filename("sounds/new/pause-01.ogg"));
    unpause_sample = load_sample(full_filename("sounds/new/unpause-01.ogg"));

    secret_found_sample = load_sample(full_filename("sounds/enchant.ogg"));

    pickup_key_sample = load_sample(full_filename("sounds/pickup_key.ogg"));
    use_key_sample = load_sample(full_filename("sounds/use_key.ogg"));
    pickup_coin_sample = load_sample(full_filename("sounds/pickup_coin.ogg"));
    ball_bounce_sample = load_sample(full_filename("sounds/ball_bounce.ogg"));
    power_up_sample_heart = load_sample(full_filename("sounds/crappy_heart_sound.ogg"));


    // fonts

	 //game_font = al_load_font("fonts/Kemco Smooth Bold.ttf", -8, 0);
	 game_font = al_load_font("fonts/Chrono Trigger.ttf", -12, 0);
    new_font = alfont_load_font(full_filename("fonts/arialbd.ttf"));

    if (new_font == NULL)
    {
       my_error_message2("Cannot load the font");
    }
    alfont_set_font_size(new_font, 18);




    // game prep

}

void destroy_game(void)
{
    al_destroy_bitmap(paused_banner);


    al_destroy_bitmap(bear1);
    al_destroy_bitmap(bear2);
    al_destroy_bitmap(bear3);
    al_destroy_bitmap(bear4);
    al_destroy_bitmap(bear_duck);
    al_destroy_bitmap(bear_climb1);
    al_destroy_bitmap(bear_climb2);
    al_destroy_bitmap(bear_in_air);
    al_destroy_bitmap(bear_downthrusting);



    al_destroy_bitmap(bad_bear_pic1);
    al_destroy_bitmap(bad_bear_pic2);
    al_destroy_bitmap(blue_bear_pic1);
    al_destroy_bitmap(blue_bear_pic2);
    al_destroy_bitmap(blue_bear_jump_pic);
    al_destroy_bitmap(purple_bear_pic1);
    al_destroy_bitmap(purple_bear_pic2);
    al_destroy_bitmap(bad_bear2_pic1);
    al_destroy_bitmap(bad_bear2_pic2);

    destroy_jewel_shimmers();




#define ADD_ITEM(xx, yy, zz)   al_destroy_bitmap( xx##_img );
	ITEM_LIST
#undef ADD_ITEM


#define ADD_ROCK(x, y) destroy_sample( x##y##_sample );
	ROCK_LIST
#undef ADD_ROCK

	al_destroy_bitmap(stat_life_meter_heart_full);
	al_destroy_bitmap(stat_life_meter_heart_empty);
	al_destroy_bitmap(stat_life_meter_heart_half);


    al_destroy_bitmap(title_graphic);
    al_destroy_bitmap(checkpoint);

    destroy_sample(sword_strike);

    destroy_sample(menu_move);
    destroy_sample(menu_select);
    destroy_sample(pause_sample);
    destroy_sample(unpause_sample);


    destroy_locked_door_graphics();

    destroy_sample(pickup_key_sample);
    destroy_sample(pickup_coin_sample);
    destroy_sample(door_locked_sample);
    destroy_sample(door_open_sample);
    destroy_sample(door_close_sample);
    destroy_sample(cha_ching);
    destroy_sample(power_up_sample_heart);
    destroy_sample(jump_sample);

    alfont_destroy_font(new_font);
    alfont_destroy_font(game_font);
}







////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




void draw_menu_option(int x, int y, const char *text, ALLEGRO_BITMAP *bmp, int selected, int disabled)
{
	ALLEGRO_COLOR color = al_map_rgba_f(1, 1, 1, 1);

	if (!selected) color = al_map_rgba_f(0.5, 0.5, 0.5, 0.5);
	if (selected && disabled) color = al_map_rgba_f(0.2, 0.2, 0.2, 1.0);

	//al_draw_tinted_bitmap(bmp, color, x - al_get_bitmap_width(bmp)/2, y - al_get_bitmap_height(bmp)/2, 0);

	std::string final_string = ((selected) ? std::string("- ") + std::string(text) + std::string(" -") : text);

	al_draw_text(game_font, (selected) ? al_color_name("green") : al_color_name("black"), x, y+1, ALLEGRO_ALIGN_CENTRE, final_string.c_str());
	al_draw_text(game_font, (selected) ? al_color_name("yellow") : al_color_name("white"), x, y, ALLEGRO_ALIGN_CENTRE, final_string.c_str());
}



void draw_title_screen()
{
	al_clear_to_color(al_color_html("131040"));
	al_draw_rectangle(5.5, 5.5, SCREEN_W-5.5, SCREEN_H-5.5, al_color_name("black"), 3.0);

	draw_sprite(buffer, title_graphic, SCREEN_W/2-al_get_bitmap_width(title_graphic)/2, 22-12);

	int x = SCREEN_W/2;
	int y = 140;
	int spacing = 13;
	draw_menu_option(x, y+spacing*0, "Continue Game", NULL, menu_focus==1, false);
	draw_menu_option(x, y+spacing*1, "Load Game", NULL, menu_focus==2, true);
	draw_menu_option(x, y+spacing*2, "Options", NULL, menu_focus==3, true);
	draw_menu_option(x, y+spacing*3, "Exit", NULL, menu_focus==4, false);


	write_outline_center(SCREEN_W/2, SCREEN_H-15, font_x_small, WHITE, COPYRIGHT_TEXT);
}



// pause menu item color
ALLEGRO_COLOR pmit(int menu_focus, int this_menu_num)
{
    if (menu_focus == this_menu_num) return YELLOW;
    return WHITE;
}    

void draw_pause_menu(void)
{
	al_draw_rectangle(3, 3, SCREEN_W-3, SCREEN_H-3, al_map_rgba_f(0.0, 0.0, 0.0, 0.5), 6.0);

	fblend_rect_trans(buffer, 10, 10, SCREEN_W-10, SCREEN_H-10, BLACK, 255/2);

	draw_sprite(buffer, paused_banner, (SCREEN_W/2)-(al_get_bitmap_width(paused_banner)/2), (SCREEN_H/3)-5);


	int x = SCREEN_W/2;
	int y = 100;
	int spacing = 16;
	draw_menu_option(x, y+spacing*0, "back to game", NULL, menu_focus==1, false);
	draw_menu_option(x, y+spacing*1, "restart this level", NULL, menu_focus==2, true);
	draw_menu_option(x, y+spacing*2, "exit to title screen", NULL, menu_focus==3, true);

}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


//DOORS
void check_function_behind_player(int block_behind_player)
{
    /// judgement of the block behind the player

	bool door_activated = (button_up && release_up);


		new_door_struct collided_door;
		if (Doors::get_collided_door(1, get_current_level_index(), get_current_map_index(), player.map_pos.x+player.size.x/2, player.map_pos.y+player.size.y/2, &collided_door))
		{
			// the player is colliding with a door right now!!
			if (door_activated)
			{
				// go in door!
				go_in_door(collided_door);
				release_up = false;
			}
		}


		Switches::update();


		item_struct collided_item;
		if (Items::get_collided_item(1, get_current_level_index(), get_current_map_index(), player.map_pos.x+player.size.x/2, player.map_pos.y+player.size.y/2, &collided_item))
		{
			// the player is colliding with an item right now!!
			// collect the item!
			player_collect_item(collided_item);

			// remove that item from the Items
			Items::remove_item_by_id(collided_item.unique_id);



			if (is_item_worth_celebrating(collided_item.type))
			{
				Dialogues::spawn(get_item_name(collided_item.type).c_str(), DIALOGUE_SUPER_ITEM);
				Dialogues::draw();
				al_flip_display();
				multitrack_music->quick_dip();
				play_item_worth_celebrating_sound();
				al_rest(7);
				multitrack_music->quick_boost();
				Dialogues::clear();
			}
		}


		checkpoint_struct collided_checkpoint;
		if (Checkpoints::get_collided_checkpoint(1, get_current_level_index(), get_current_map_index(), player.map_pos.x+player.size.x/2, player.map_pos.y+player.size.y/2, &collided_checkpoint))
		{
			// the player is colliding with a checkpoint right now!!
			// if the checkpoint is not the most recently activated checkpoint, activate it!
			checkpoint_struct last_visited_checkpoint;
			bool visited_checkpoint_exists = Checkpoints::get_last_visited_checkpoint(&last_visited_checkpoint);
			if ((collided_checkpoint.unique_id != last_visited_checkpoint.unique_id) || !visited_checkpoint_exists)
			{
				// activate this checkpoint!
				Checkpoints::trigger_checkpoint(collided_checkpoint.unique_id);
			}
		}


}



int check_user_input()
{
     if (keyboard_needs_poll()) poll_keyboard();
     if (keyboard_available) check_keyboard();
     if (joystick_available) check_joystick();
     if (mouse_available) poll_mouse();

    if (key[ALLEGRO_KEY_F12]) // f12 takes a screenshot
    {
			take_screenshot("screenshot");
    }

   if (F_debug_mode)
	{ 
    if (key[ALLEGRO_KEY_PAD_9]) player.keys++;
    if (key[ALLEGRO_KEY_PAD_3]) player.use_key();
    if (key[ALLEGRO_KEY_PAD_7]) player.silver_keys++;
    if (key[ALLEGRO_KEY_PAD_1]) player.use_silver_key();
	}

	return 0;
}



void draw_item_centered(int item_type, int x, int y)
{
     ALLEGRO_BITMAP *b;

           switch (item_type)
           {

#define ADD_ITEM(xx, yy, zz)   case xx##_const: b = xx##_img; break;
//#include "item_list.hpp"
	ITEM_LIST
#undef ADD_ITEM

           }

           if (b)
           {
               draw_sprite(buffer, b, x-al_get_bitmap_width(b)/2, y-al_get_bitmap_height(b)/2);
           }
}




void draw_map_backgrounds()
{
    for (int k=0; k<current_map_background.size(); k++)
    {
        if (current_map_background[k].img)
        {

              draw_ms_background(buffer,
                                 current_map_background[k].img,
                                 current_map_background[k].x_offset+(int)current_map_background[k].x_disp,
                                 current_map_background[k].y_offset+(int)current_map_background[k].y_disp,
                                 current_map_background[k].scroll_rate,
                                 current_map_background[k].loop_x,
                                 current_map_background[k].loop_y
                                 );
        }    
    }        
}    





void do_logic(void)
{
	multitrack_music->update();

	switch (get_game_state())
	{
	case STATE_TITLE_SCREEN:
	   figure_out_title_screen_user_input();
		break;
	default:
		{
		  use_key_animation_manager.update();

		  solved_door_animation_manager.update();
		  if (solved_door_animation_manager.active())
		  {
				camera.x = solved_door_animation_manager.get_x()-SCREEN_W/2;
				camera.y = solved_door_animation_manager.get_y()-SCREEN_H/2;
				camera.prevent_scrolling_off_map();
				camera_x = camera.x;
				camera_y = camera.y;

				particle_effect_manager.update();
		  }

		  if (!use_key_animation_manager.active())
		  //if (!use_key_animation_manager.active() && !solved_door_animation_manager.active())
		  {

				update_jewel_shimmer();
			Items::update();
			Enemies::update_enemy_spawners(1, get_current_level_index(), get_current_map_index(), player.map_pos.x+player.size.x/2, player.map_pos.y+player.size.y/2);
			Enemies::update(1, get_current_level_index(), get_current_map_index());
			VisualFX::update();
			Dialogues::update();




			 if (key[ALLEGRO_KEY_PAD_0]) player.has_gun = true;
			 current_map__update_backgrounds();
			 update_help_message();
			 //test_player_power_up_collisions();
			 update_picked_up_items();


			if (!F_disable_edits)
			{
				 if (key[ALLEGRO_KEY_F5])
				{
					World::refresh_world_and_current_map();
				}
				 if (key[ALLEGRO_KEY_F6])
				{
					//save_tile_map_file(current_map, "saved_map.bmap3");
				}
				if (key[ALLEGRO_KEY_8])
				{
					std::string script = "Ember: I hope that it finds its place in you.  And I hope that you might one day be able to give it to another.";
					Dialogues::spawn(script, DIALOGUE_TEXT);
				}
				
				 if (key[ALLEGRO_KEY_F1] && release_f1) {F_debug_mode=!F_debug_mode; F_map_maker_mode=false; release_f1=false;}
				 else if (!key[ALLEGRO_KEY_F1]) release_f1=true;

				 if (key[ALLEGRO_KEY_F2] && release_f2) {F_map_maker_mode=!F_map_maker_mode; F_debug_mode=false; release_f2=false;}
				 else if (!key[ALLEGRO_KEY_F2]) release_f2=true;
			}


			 // graphic options and toggles
			 if (key[ALLEGRO_KEY_F6] && release_f6)
			 {
					std::cout << "swapping" << std::endl;
					for (unsigned i=0; i<level_class__switch_stick.size(); i++) Switches::toggle_switch_stick(i);
					std::cout << "...done swapping" << std::endl;

/*
				  F_draw_backgrounds =! F_draw_backgrounds;
				  if (F_draw_backgrounds) start_help_message("Background On");
				  else start_help_message("Background Off");
*/
				  release_f6=false;
			 }
			 else if (!key[ALLEGRO_KEY_F6]) release_f6=true;




			// check if the damage zones collide with a destructable block

			const std::vector<player_damage_zone> &damage_zones = DamageZones::get_damage_zones();
			for (int i=0; i<damage_zones.size(); i++)
			{
				const player_damage_zone &zone = damage_zones[i];
				std::vector<int2> collided_blocks = get_tile_coordinates_within(zone.x, zone.y, zone.x+zone.w, zone.y+zone.h);
				for (unsigned b=0; b<collided_blocks.size(); b++)
				{
					int tile_x = collided_blocks[b].x;
					int tile_y = collided_blocks[b].y;
	
					switch (current_map->get_tile(collided_blocks[b].x, collided_blocks[b].y, BEARY_TILE_LAYER_BOUNDARY))
					{
						case BLOCK_DESTRUCTABLE:
							// there was a collision between a player damage zone and a destructable block
							// spawn some particles
							VisualFX::spawn(tile_x * 16 + 8, tile_y * 16 + 8, 0.5, 0.5, "explosion");
							block_destruction_manager.add_particles(tile_x * 16 + 8, tile_y * 16 + 8, 0, 0);
							play_rock_destroy_sound();
							// swap out the boundary block and graphic :)
							current_map->set_tile(collided_blocks[b].x, collided_blocks[b].y, BEARY_TILE_LAYER_BOUNDARY, BLOCK_DESTRUCTED);
							current_map->set_tile(collided_blocks[b].x, collided_blocks[b].y, BEARY_TILE_LAYER_1, 0);
							break;
						case BLOCK_BOUNDRY_11:
							// the player's weapon hit a wall
							if (zone.type == PLAYER_SWIPE)
								play_sword_clang_sound();
							break;
					}
				}
			}

			DamageZones::update();



			 adding_coins_update();

			 // gameplay

			 int block_behind_player = block_type_at(player.get_center_int(), player.get_middle_int());

			 if (!button_up) release_up = true;
			 if (!button_a) release_a = true;
			 if (!button_b) release_b = true;

			 /// user control of the player during the game
			 if ((get_game_state() == STATE_IN_LEVEL) || ((get_game_state() == STATE_FINISH_ROUND) && (finished_round_counter > 0)))
			 {
				  check_function_behind_player(block_behind_player);

				  check_switch_behind_player();


				  if (button_y)
				  {
						if (release_y) player.attack();
						//player.run();
						if (player.has_gun && release_y)
						{
							 float y_force = 0.0f;
							 if (button_up) y_force = -2.0f;
							 if (button_down) y_force = 2.0f;
							 bullet_manager.add_bullet(player.get_center(), player.get_middle(), player.velocity.x, y_force, player.facing_right);
							 play_menu_move_sound();
						}
						release_y = false;
				  }
				  else if (!button_y)
				  {
						 //player.walk();
						 release_y = true;
				  }




				  //if (button_b && release_b) player.jump();

				  if (button_right) player.move_right();
				  if (button_left) player.move_left();
				  else if (!button_right && !button_left) player.move_nowhere();

				  bullet_manager.update();
				  block_destruction_manager.update();
				  particle_effect_manager.update();

				  test_enemy_bullet_collisions();
				  test_destructable_block_bullet_collision();

				  if (!player.on_ladder)
				  {
					  if (button_b && !player.on_ladder) {player.jump();}
					  else if (!button_b && !player.on_ladder) {player.release_jump();}

					  if (button_down) player.duck();
					  else player.stand();
				  }

				  //if (!player.on_ladder && (button_up || button_down) && (block_behind_player == BLOCK_LADDER))
				  if (!player.on_ladder && (button_up || button_down) && (is_ladder_block(block_behind_player)))
				  {
						// player gets on the ladder right here
					  int center_of_ladder_x = ((player.get_center_int()/TILE_SIZE)*TILE_SIZE)+(TILE_SIZE/2-1);
					  player.get_on_ladder(center_of_ladder_x);
				  }


				  if (player.on_ladder)
				  {
						if (button_down)
						{
							player.velocity.y = player.max_climb_speed;
							player.velocity.x = 0.0f;
							if (player.on_ground) player.get_off_ladder();
						}
						else if (button_up)
						{
							player.velocity.y = player.max_climb_speed*(-1);
							player.velocity.x = 0.0f;
						}
						else
						{
							player.velocity.x = 0.0f;
							player.velocity.y = 0.0f;
						}

						// if the block below him isn't a ladder.
						//if (block_behind_player != BLOCK_LADDER) player.get_off_ladder();
						if (!is_ladder_block(block_behind_player)) player.get_off_ladder();

						// if the user pushes the jump button.
						if (button_b && (release_b==true)) {player.jump_off_ladder(); release_b=false;}
				  }
			 }/// END user control of the player during the game
			

			 if (button_x && release_x)
			 {
				  switch (get_game_state())
				  {
						case STATE_IN_LEVEL:
							  play_pause_sound();
								GAMEPLAY_FRAME_DURATION = 0.0;
							  pre_pause_gamestate = get_game_state();
							  set_game_state(STATE_PAUSE);
							  menu_number = MENU_PAUSE;
							  menu_focus = 1;
							  clock_time_suspended_at = ten_mili_seconds;
						break;
						case STATE_PAUSE:
							  play_unpause_sound();
								GAMEPLAY_FRAME_DURATION = GAMEPLAY_FRAME_DURATION_DEFAULT;
								set_game_state(pre_pause_gamestate);
								ten_mili_seconds = clock_time_suspended_at;
						break;
						case STATE_WAITING_FOR_LEVEL_START:
							  //pre_pause_gamestate = game_state;
							  //game_state = STATE_PAUSE;
							  //menu_number = MENU_PAUSE;
							  //menu_focus = 1;
						break;
						case STATE_FINISH_ROUND:
							  play_pause_sound();
							  if (finished_round_counter <= 0)
							  {
									pre_pause_gamestate = get_game_state();
									set_game_state(STATE_PAUSE);
									menu_number = MENU_PAUSE;
									menu_focus = 1;
							  }    
						break;
				  } 
					  
				  release_x = false;
			 } // end if (button_x && release_x)
			 
			 
			 if (!button_x) release_x = true;


			 // update-clocks

			 if ((get_game_state() == STATE_FINISH_ROUND) && (finished_round_counter <=0))
			 {
				  player.move_nowhere();
			 }    

			 if (player.energy <= 0)
			 {
				  // you lost !  dieded !
					checkpoint_struct last_checkpoint;
					if (Checkpoints::get_last_visited_checkpoint(&last_checkpoint))
					{
						checkpoint_struct destination_door = last_checkpoint;
						std::cout << "DEAD... need to respawn" << std::endl;
					}
					else
					{
						std::cout << "YOU GOTTA RESPAWN!" << std::endl;
					}
			 }

			//test_player_damage_zone_collisions();

			//OLD_test_player_enemy_collisions();





			// new test_player_enemy_collisions
			// in this case, only one collision per frame
			EnemyBase *collided_enemy = NULL;
			if (Enemies::collides(1, get_current_level_index(), get_current_map_index(), player.map_pos.x, player.map_pos.y, player.size.x, player.size.y, &collided_enemy)
				&& !collided_enemy->is_dead())
			{
				if (!collided_enemy->fazed_from_damage)
				{
					// if it's a downstrike, apply damage to the enemy and bounce the player up
					if (player.is_downthrusting)
					{
						// we're just going to hack this damage zone that only appears for this purpose :/
						player_damage_zone downthrust_damage_zone;
						downthrust_damage_zone.x=0;
						downthrust_damage_zone.y=0;
						downthrust_damage_zone.w=0;
						downthrust_damage_zone.h=0;
						downthrust_damage_zone.damage=1;
						downthrust_damage_zone.type=1;
						downthrust_damage_zone.lifespan=0;

						collided_enemy->on_collides_with_damage_zone(downthrust_damage_zone);
						if (player.on_ladder) {player.get_off_ladder();}
						player.velocity.y = -5.5;
					}
					else
					{
						player.take_and_react_to_damage(collided_enemy->touch_damage);
						collided_enemy->on_collides_with_player();
					}
				}
			}


			 if ((get_game_state() != STATE_TIME_UP) && (get_game_state() != STATE_PAUSE))
			 {
				  update_pre_game();

				  update_award_timers();

				  player.update_state();

				  update_player_animation();

				  if (CameraRegions::update(player.map_pos.x+player.size.x/2, player.map_pos.y+player.size.y, &camera_x, &camera_y, SCREEN_W, SCREEN_H))
					{
						int new_id = (multitrack_music->get_current_playing_index_id()+1) % multitrack_music->tracks.size();
						multitrack_music->crossfade_to(new_id);
					}
				  camera.x = camera_x;
					camera.y = camera_y;

			 }
			 if (get_game_state() == STATE_PAUSE)
			 {
				  figure_out_pause_screen_user_input();
			 }


			 if (get_game_state() != STATE_PAUSE)
			 {
				  update_help_message();
			 }    

			 if (get_game_state() == STATE_FINISH_ROUND) finished_round_counter--;
			 if (finished_round_counter < 0)
			{
				finished_round_counter = 0;
			}


			}
			break;
	  }// end of game state switch statement
	}
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void draw_PIXEL_LAYER_to_buffer(void)
{
	switch(get_game_state())
	{
		case STATE_NOT_SET:
		{
			break;
		}
		case STATE_TITLE_SCREEN:
		{
			clear_to_color(buffer, COLOR_BACKGROUND);
			draw_title_screen();
			break;
		}
		default:
		{
			clear_to_color(buffer, background_color);



			if (F_draw_backgrounds) draw_map_backgrounds();




			//draw_map();
			current_map->draw_layer(BEARY_TILE_LAYER_BACKGROUND, camera.x, camera.y);
			current_map->draw_layer(BEARY_TILE_LAYER_0, camera.x, camera.y);
			current_map->draw_layer(BEARY_TILE_LAYER_1, camera.x, camera.y);

			Doors::draw(1, get_current_level_index(), get_current_map_index());
			Switches::draw();
			Checkpoints::draw(1, get_current_level_index(), get_current_map_index());
			Items::draw(1, get_current_level_index(), get_current_map_index());
			Enemies::draw(1, get_current_level_index(), get_current_map_index());

			trigger_manager.draw();



			bullet_manager.draw();
			particle_effect_manager.draw();
			use_key_animation_manager.draw();


			draw_player_bear();

			//draw_map_front_layer();
			current_map->draw_layer(BEARY_TILE_LAYER_2, camera.x, camera.y);

			VisualFX::draw_all();
			Dialogues::draw();




			DamageZones::draw();





			if (F_debug_mode) player.draw_bounding_box();

			block_destruction_manager.draw();
			draw_picked_up_items();

			if (!F_map_maker_mode && !F_debug_mode && (get_game_state() != STATE_TITLE_SCREEN))
			{
				if (!hide_hud) draw_hud();
			}

			draw_map_maker_and_debug_stuff();




			draw_help_message();

			if (get_game_state() == STATE_PAUSE)
			{
				draw_pause_menu();
			}


			LevelTitle::update_and_draw();

			// here's where we draw the hue overlay on top of the whole thing
			al_draw_filled_rectangle(0, 0, SCREEN_W, SCREEN_H, foreground_color);

		}
		break;
	} // end of case (game_state)
}



void draw_PROJECTION_LAYER_to_buffer()
{
  int ind[] = 
  {
    0, 1, 2,
    0, 1, 3,
    0, 2, 3,
    1, 2, 3
  };
 

	float size = 1;

  ALLEGRO_VERTEX vtx[4];
  vtx[0].x = -size;
  vtx[0].y = -size;
  vtx[0].z = -size;
  vtx[0].color = al_map_rgb_f(0, 0, 1);

  vtx[1].x = 0;
  vtx[1].y = -size;
  vtx[1].z = size;
  vtx[1].color = al_map_rgb_f(0, 1, 0);

  vtx[2].x = size;
  vtx[2].y = -size;
  vtx[2].z = -size;
  vtx[2].color = al_map_rgb_f(1, 0, 1);

  vtx[3].x = 0;
  vtx[3].y = size;
  vtx[3].z = 0;
  vtx[3].color = al_map_rgb_f(1, 1, 1);


	ALLEGRO_TRANSFORM t, prev;
	al_copy_transform(&prev, al_get_current_transform());
	al_identity_transform(&t);
	al_rotate_transform_3d(&t, 0, 1, 0, al_get_time()*0.2);

	//float aspect_ratio = (float)al_get_bitmap_height(projection_subbitmap) / al_get_bitmap_width(projection_subbitmap);
	//al_perspective_transform(&proj_t, 1, aspect_ratio, 1, -1, -aspect_ratio, 1000);
	//std::cout << "aspect_ratio " << aspect_ratio << std::endl;

	al_translate_transform_3d(&t, 0, 0, -256);
	al_translate_transform_3d(&t, (float)camera_x * SCREEN_SCALE_X, (float)camera_y * SCREEN_SCALE_Y, 0);

	al_use_transform(&t);

	al_draw_indexed_prim(vtx, NULL, NULL, ind, 12, ALLEGRO_PRIM_TRIANGLE_LIST);

	al_use_transform(&prev);
}




void game_loop()
{
	ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
	ALLEGRO_TIMER *timer = al_create_timer(1/60.0);
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_display_event_source(al_get_current_display()));

	al_start_timer(timer);

   while (!abort_game)
   {
		ALLEGRO_EVENT current_event;

		al_wait_for_event(event_queue, &current_event);
      switch(current_event.type) 
		{
		case ALLEGRO_EVENT_TIMER:
			while(!al_is_event_queue_empty(event_queue))
				al_drop_next_event(event_queue);

			ten_mili_seconds += 6000;

			check_user_input();
			do_logic();
			clear_input();

						// setup the render settings
						al_set_target_bitmap(buffer);
						al_set_render_state(ALLEGRO_DEPTH_TEST, 0);
						//al_set_render_state(ALLEGRO_WRITE_MASK, ALLEGRO_MASK_DEPTH | ALLEGRO_MASK_RGBA);
						//al_clear_depth_buffer(1000);

			draw_PIXEL_LAYER_to_buffer();

						// setup the render settings
						al_set_target_bitmap(projection_subbitmap);
						al_set_render_state(ALLEGRO_DEPTH_TEST, 1);
						al_set_render_state(ALLEGRO_WRITE_MASK, ALLEGRO_MASK_DEPTH | ALLEGRO_MASK_RGBA);
						al_clear_depth_buffer(1000);

			draw_PROJECTION_LAYER_to_buffer();
/*
						// setup the render settings
						al_set_target_bitmap(buffer);
						al_set_render_state(ALLEGRO_DEPTH_TEST, 0);
						al_set_render_state(ALLEGRO_WRITE_MASK, ALLEGRO_MASK_DEPTH | ALLEGRO_MASK_RGBA);
						al_clear_depth_buffer(1000);
*/
			al_flip_display();
			break;
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			abort_game = true;
			break;
       }

   }

	al_destroy_timer(timer);
	al_destroy_event_queue(event_queue);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{
   arg_string = argv[0];
   if (initialize() == 1) allegro_message("initialization failed");

   load_game();
   set_game_state(STATE_TITLE_SCREEN);


   game_loop();

   cleanup();
   destroy_game();

   return 0;
}

