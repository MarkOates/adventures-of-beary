float random_float(float min, float max)
{
    return ((float) rand()/RAND_MAX)*(max-min) + min;
}

int random_int(int min, int max)
{
    return rand()%(max-min+1) + min;
}

#include <algorithm> 
template<class T>
T random(std::vector<T> list)
{
	if (list.empty()) return T();
	std::random_shuffle(list.begin(), list.end());
	return list[0];
}

#include <algorithm> 
int random_int(std::vector<int> list)
{
	if (list.empty()) return 0;

	std::random_shuffle(list.begin(), list.end());
	return list[0];
}

bool random_bool()
{
    return (rand()%2 == 1);
}




#define ABS(v) ((v < 0) ? (v*-1) : v)

void write_outline_center(int x, int y, ALLEGRO_FONT *f, ALLEGRO_COLOR color, const char *format, ...);
void write_outline(int x, int y, ALLEGRO_FONT *f, ALLEGRO_COLOR color, const char *format, ...);

ALLEGRO_BITMAP *_load_bitmap(std::string address, bool show_if_error=true)
{
	ALLEGRO_BITMAP *b;

	b = al_load_bitmap(address.c_str());
	if (b) al_convert_mask_to_alpha(b, al_map_rgb(255,0, 255));
	if (!b && show_if_error) std::cout << "could not load \"" << address << "\"" << std::endl;
	return b;
}

float BLEND_ALPHA = 1.0;

void set_multiply_blender(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	BLEND_ALPHA = a / 255.0;
}

void set_trans_blender(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	BLEND_ALPHA = a / 255.0;
}

void draw_trans_sprite(ALLEGRO_BITMAP *target, ALLEGRO_BITMAP *src, int x, int y)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_tinted_bitmap(src, al_map_rgba_f(BLEND_ALPHA, BLEND_ALPHA, BLEND_ALPHA, BLEND_ALPHA), x, y, 0);

	al_restore_state(&prev);	
}

void fblend_rect_trans(ALLEGRO_BITMAP *target, float x, float y, float x2, float y2, ALLEGRO_COLOR primary_color, unsigned char _factor)
{
	float factor = _factor / 255.0;

	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_filled_rectangle(x, y, x2, y2, al_map_rgba_f(primary_color.r * factor, primary_color.g * factor, primary_color.b * factor, primary_color.a * factor));

	al_restore_state(&prev);	
}

#define DRAW_MODE_TRANS 1
void drawing_mode(int mode, ALLEGRO_BITMAP *pattern, int x, int y)
{
	// do nothing
}

void solid_mode()
{
	// also do nothing
}


template<class T>
std::string tostring(T val)
{
	std::ostringstream s;
	s << val;
	return s.str();
}


int text_length(ALLEGRO_FONT *f, std::string message)
{
	return al_get_text_width(f, message.c_str());
}

ALLEGRO_FONT *alfont_load_font(std::string filename)
{
	return al_load_font(filename.c_str(), 15, 0);
}

void alfont_destroy_font(ALLEGRO_FONT *font)
{
	al_destroy_font(font);
}

int alfont_text_length(ALLEGRO_FONT *f, std::string text)
{
	return text_length(f, text);
}

ALLEGRO_SAMPLE *load_sample(std::string filename)
{
	return al_load_sample(filename.c_str());
}

void destroy_sample(ALLEGRO_SAMPLE *sample)
{
	al_destroy_sample(sample);
}


ALLEGRO_KEYBOARD_STATE keyboard_state;
bool key[ALLEGRO_KEY_MAX];
void poll_keyboard()
{
	al_get_keyboard_state(&keyboard_state);
	for (int i=0; i<ALLEGRO_KEY_MAX; i++)
	{
		key[i] = al_key_down(&keyboard_state, i);
	}
}

bool keyboard_needs_poll()
{
	return true;
}

void clear_keybuf()
{
	poll_keyboard();
}

void readkey()
{
	ALLEGRO_KEYBOARD_STATE new_state;

	while(true)
	{
		al_get_keyboard_state(&new_state);

		for (int i=0; i<ALLEGRO_KEY_MAX; i++)
		{
			if (!key[i] && al_key_down(&new_state, i)) return;
		}
	}
}


void alfont_set_font_size(ALLEGRO_FONT *font, int size)
{
	// do nothing
}


void draw_sprite_h_flip(ALLEGRO_BITMAP *target, ALLEGRO_BITMAP *src, int x, int y)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_bitmap(src, x, y, ALLEGRO_FLIP_HORIZONTAL);

	al_restore_state(&prev);
}


int text_height(ALLEGRO_FONT *font)
{
	return al_get_font_line_height(font);
}

int alfont_text_height(ALLEGRO_FONT *font)
{
	return text_height(font);
}

void fblend_trans(ALLEGRO_BITMAP *sprite, ALLEGRO_BITMAP *target, int x, int y, unsigned char factor)
{
	float alpha = factor / 255.0;

	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_tinted_bitmap(sprite, al_map_rgba_f(alpha, alpha, alpha, alpha), x, y, 0);

	al_restore_state(&prev);
}


ALLEGRO_JOYSTICK_STATE joy[1];
ALLEGRO_JOYSTICK *joystick = NULL;
void poll_joystick()
{
	if (!joystick) return;

	al_get_joystick_state(joystick, &joy[0]);
}
int get_joy_num_buttons()
{
	if (!joystick) return 0;
	
	return al_get_joystick_num_buttons(joystick);
}


void textprintf(ALLEGRO_BITMAP *target, ALLEGRO_FONT *font, int x, int y, ALLEGRO_COLOR color, const char *format, ...)
{
   char argsbuff[256];
   va_list args;
   va_start(args, format);
   vsprintf(argsbuff, format, args);
   va_end(args);

	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_text(font, color, x, y, 0, argsbuff);

	al_restore_state(&prev);
}


void alfont_textprintf(ALLEGRO_BITMAP *target, ALLEGRO_FONT *font, int x, int y, ALLEGRO_COLOR color, std::string text)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_text(font, color, x, y, 0, text.c_str());

	al_restore_state(&prev);
}

void alfont_textprintf_aa(ALLEGRO_BITMAP *target, ALLEGRO_FONT *font, int x, int y, ALLEGRO_COLOR color, std::string text)
{
	alfont_textprintf(target, font, x, y, color, text);
}

void alfont_textprintf_centre(ALLEGRO_BITMAP *target, ALLEGRO_FONT *font, int x, int y, ALLEGRO_COLOR color, std::string text)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	y -= 2;

	al_draw_text(font, color, x, y, ALLEGRO_ALIGN_CENTER, text.c_str());

	al_restore_state(&prev);
}

void alfont_textprintf_centre(ALLEGRO_BITMAP *target, ALLEGRO_FONT *font, int x, int y, ALLEGRO_COLOR color, std::string text, int val)
{
	std::stringstream ss;
	if (text == "0%i") ss << "0";
	ss << val;

	alfont_textprintf_centre(target, font, x, y, color, ss.str());
}


void alfont_textprintf_centre_aa(ALLEGRO_BITMAP *target, ALLEGRO_FONT *font, int x, int y, ALLEGRO_COLOR color, std::string text)
{
	alfont_textprintf_centre(target, font, x, y, color, text);
}

void alfont_textprintf_centre_aa(ALLEGRO_BITMAP *target, ALLEGRO_FONT *font, int x, int y, ALLEGRO_COLOR color, std::string text, int val)
{
	std::stringstream ss;
	if (text == "0%i") ss << "0";
	ss << val;

	alfont_textprintf_centre(target, font, x, y, color, ss.str());
}

   
void ellipsefill(ALLEGRO_BITMAP *target, int cx, int cy, int rx, int ry, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_filled_ellipse(cx, cy, rx, ry, color);

	al_restore_state(&prev);
}


#include <algorithm>
std::string take_screenshot(std::string filename)
{
	ALLEGRO_STATE previous_state;
	al_store_state(&previous_state, ALLEGRO_STATE_TARGET_BITMAP);

	ALLEGRO_DISPLAY *display = al_get_current_display();
	ALLEGRO_BITMAP *target = al_create_bitmap(al_get_display_width(display), al_get_display_height(display));
	al_set_target_bitmap(target);

	al_draw_bitmap(al_get_backbuffer(display), 0, 0, 0);

	al_restore_state(&previous_state);

	al_save_bitmap(filename.c_str(), target);
	al_destroy_bitmap(target);

	return filename;
}


void rect(ALLEGRO_BITMAP *target, int x, int y, int x2, int y2, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_rectangle(x+0.5, y+0.5, x2+0.5, y2+0.5, color, 1.0);

	al_restore_state(&prev);
}



ALLEGRO_BITMAP *create_bitmap(int w, int h)
{
	return al_create_bitmap(w, h);
}



void destroy_font(ALLEGRO_FONT *font)
{
	al_destroy_font(font);
}



void play_sample(ALLEGRO_SAMPLE *spl, int vol, int pan, int freq, int loop)
{
	al_play_sample(spl, vol/255.0, (pan-128)/128.0, freq/1000.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}


void rectfill(ALLEGRO_BITMAP *target, float x1, float y1, float x2, float y2, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_filled_rectangle(x1, y1, x2+1, y2+1, color);

	al_restore_state(&prev);
}


void putpixel(ALLEGRO_BITMAP *target, int x, int y, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_pixel(x, y, color);

	al_restore_state(&prev);
}


void textprintf_centre(ALLEGRO_BITMAP *target, ALLEGRO_FONT *font, float x, float y, ALLEGRO_COLOR color, std::string text, std::string str2="")
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_textf(font, color, x, y, ALLEGRO_ALIGN_CENTER, text.c_str(), str2.c_str());

	al_restore_state(&prev);
}



ALLEGRO_SAMPLE *enemy_hit_sound = NULL;
ALLEGRO_SAMPLE *enemy_dead_sound = NULL;
ALLEGRO_SAMPLE *sword_clang_sound = NULL;
ALLEGRO_SAMPLE *item_worth_celebrating_sound = NULL;

void play_enemy_hit_sound()
{
	al_play_sample(enemy_hit_sound, 1.0, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

void play_item_worth_celebrating_sound()
{
	al_play_sample(item_worth_celebrating_sound, 1.0, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

void play_enemy_dead_sound()
{
	al_play_sample(enemy_dead_sound, 1.0, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}

void play_sword_clang_sound()
{
	al_play_sample(sword_clang_sound, 1.0, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
}




void vline(ALLEGRO_BITMAP *target, int x, int y, int y2, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_line(x + 0.5, y+0.5, x + 0.5, y2+0.5, color, 1);

	al_restore_state(&prev);
}


void hline(ALLEGRO_BITMAP *target, int x, int y, int x2, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_line(x + 0.5, y+0.5, x2 + 0.5, y+0.5, color, 1);

	al_restore_state(&prev);
}

void line(ALLEGRO_BITMAP *target, int x, int y, int x2, int y2, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_line(x + 0.5, y+0.5, x2 + 0.5, y2+0.5, color, 1);

	al_restore_state(&prev);
}



void clear_to_color(ALLEGRO_BITMAP *target, ALLEGRO_COLOR color)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_clear_to_color(color);

	al_restore_state(&prev);
}



ALLEGRO_COLOR makecol(unsigned char r, unsigned char g, unsigned char b)
{
	return al_map_rgb(r, g, b);
}



void allegro_message(std::string msg, std::string msg2="")
{
	std::cout << "allegro_message() \"" << msg << "\", \"" << msg2 << "\"" << std::endl;
}



// consequently, any time blit is used in this program, it's equivelent to a flip
void blit(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *target, int sx, int sy, int dx, int dy, int sw, int sh)
{
	al_flip_display();
}

void masked_blit(ALLEGRO_BITMAP *source, ALLEGRO_BITMAP *target, int sx, int sy, int dx, int dy, int w, int h)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_bitmap_region(source, sx, sy, w, h, dx, dy, 0);

	al_restore_state(&prev);
}


void draw_sprite_FAST(ALLEGRO_BITMAP *src, int x, int y)
{
	al_draw_bitmap(src, x, y, 0);
}



void draw_sprite(ALLEGRO_BITMAP *target, ALLEGRO_BITMAP *src, int x, int y)
{
	ALLEGRO_STATE prev;
	al_store_state(&prev, ALLEGRO_STATE_TARGET_BITMAP);
	al_set_target_bitmap(target);

	al_draw_bitmap(src, x, y, 0);

	al_restore_state(&prev);
}


void show_video_bitmap(ALLEGRO_BITMAP *buff)
{
	al_flip_display();
}



struct float2
{
   float x;
   float y;
};

class int2
{
public:
	int x;
	int y;
	int2() : x(0), y(0) {}
	int2(int x, int y) : x(x), y(y) {}
	bool operator==(const int2 &other)
	{
		if (x != other.x) return false;
		if (y != other.y) return false;
		return true;
	}
};







//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int get_current_map_index();
int get_current_level_index();
int get_current_world_index();



std::vector<int2> get_tile_coordinates_within(float x1, float y1, float x2, float y2)
{
	std::vector<int2> result;
	int left = x1/16;
	int top = y1/16;
	int right = x2/16;
	int bottom = y2/16;

	result.reserve((bottom-top) * (right-left));

	for (int y=top; y<=bottom; y++)
		for (int x=left; x<=right; x++)
		{
			result.push_back(int2(x, y));
		}

	return result;
}


