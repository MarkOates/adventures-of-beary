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

			if (!bitmap)
			{
				std::cout << "[TileAtlas::draw_to_tile_atlas()] ERROR: the ALLEGRO_BITMAP provided is NULL" << std::endl;
				return;
			}

			for (int index_num=0; index_num<(int)tile_index.size(); index_num++)
			{
				if (index_num != tile_num) continue;

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

