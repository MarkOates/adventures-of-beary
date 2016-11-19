#pragma once




#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <vector>




int random_int(int min, int max);




class TileAtlas
{
private:
   class TileIndexInfo
   {
   public:
      int index_num;

      ALLEGRO_BITMAP *bitmap_source;
      ALLEGRO_BITMAP *sub_bitmap;

      int u1, v1, u2, v2;

      TileIndexInfo();
   };

public:
   std::vector<TileIndexInfo> tile_index;
   ALLEGRO_BITMAP *bitmap;

   void clear();
   static ALLEGRO_BITMAP *build_tile_atlas(int tile_w, int tile_h, std::vector<TileIndexInfo> &tile_index);
   void load(ALLEGRO_BITMAP *tileset, int tile_width, int tile_height, int spacing=0);
   void draw_tile_to_atlas(ALLEGRO_BITMAP *tile, int tile_num, ALLEGRO_COLOR color=al_map_rgba_f(1, 1, 1, 1));
   bool get_tile_uv(int index_num, int *u1, int *v1, int *u2, int *v2);
};




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
   void set_tile_uv(int tile_x, int tile_y, int u1, int v1, int u2, int v2);

   ALLEGRO_VERTEX_BUFFER *vertex_buffer;
   std::vector<ALLEGRO_VERTEX> vtx;
   TileAtlas *tile_atlas;
   int width;
   int height;
   std::vector<int> tiles;

   bool use_vtx;

public:

   TileMap();

   void use_tile_atlas(TileAtlas *atlas);

   void resize(int w, int h, int tile_w, int tile_h);

   int get_num_tiles();

   bool set_contiguous_tile_num(int contiguous_tile_num, int tile_index);

   int get_tile(int tile_x, int tile_y);

   bool set_tile(int tile_x, int tile_y, int tile_index);

   void draw(int camera_x, int camera_y);

   void random_fill();

   int get_width();

   int get_height();
};




