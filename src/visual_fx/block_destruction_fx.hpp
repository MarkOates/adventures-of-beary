

class block_destruction_particle_class
{
public:
    float x;
    float y;
    float velocity_x;
    float velocity_y;

    int counter;
    bool active;

    int type;  // 1 2 or 3

    block_destruction_particle_class(float xx, float yy, float velocity_xx, float velocity_yy, int particle_type, int _counter=50)
    {
        active = true;
        counter = _counter;

        x = xx;
        y = yy;
        velocity_x = velocity_xx;
        velocity_y = velocity_yy;

        type = particle_type;
    }

    void test_vertical_map_collisions()
    {
        bool char_collides_next = false;
        bool char_collides_now = false;

        if (velocity_y > 0.0f) // falling
        {
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
            if (velocity_y > 0.25f) play_random_rock_sample(64, (int)((float)(x-camera.x)/SCREEN_W*255), random_int(800, 1000));
            velocity_y *= -0.4f;
            y = block_top_edge(((int)(y+velocity_y+8)+1)/16) - 1;
        }
        else if (velocity_y < 0.0f) // jumping
        {
            if (is_ceiling_block(block_type_at((int)(x), (int)(y+velocity_y)-1)))
            {
                play_random_rock_sample(64, (int)((float)(x-camera.x)/SCREEN_W*255), random_int(800, 1000));
                velocity_y *= -0.4f;
                y = block_bottom_edge(((int)(y+velocity_y-8)-1)/16) + 1;
            }
        }
    }

    void test_horizontal_map_collisions()
    {
        bool char_collides_next = false;
        bool char_collides_now = false;

        if (velocity_x > 0.0f) // falling
        {
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
            velocity_x *= -0.4f;
            x = block_left_edge(((int)(x+velocity_x+8)+1)/16) - 1;
            play_random_rock_sample(64, (int)((float)(x-camera.x)/SCREEN_W*255), random_int(800, 1000));
        }

        else if (velocity_x < 0.0f) // jumping
        {
            if (is_right_wall_block(block_type_at((int)(x+velocity_x)-1, (int)(y))))
            {
                velocity_x *= -0.4f;
                x = block_right_edge(((int)(x+velocity_x-8)-1)/16) + 1;
                play_random_rock_sample(64, (int)((float)(x-camera.x)/SCREEN_W*255), random_int(800, 1000));
            }
        }
    }


    void update()
    {
        counter -= 1;
        if (counter < 0) { counter = 0; active = false; }

        if (active)
        {
            velocity_y += GRAVITY;
            test_vertical_map_collisions();
            y += velocity_y;

            //void apply_drag
            test_horizontal_map_collisions();
            x += velocity_x;
        }
    }

    void draw()
    {
         if (!active) return;

         ALLEGRO_BITMAP *b;

         switch(type)
         {
             case 1:
                  b = destructable_block_shard_1;
                  break;
             case 2:
                  b = destructable_block_shard_2;
                  break;
             case 3:
                  b = destructable_block_shard_3;
                  break;
         }

         draw_sprite(buffer, b, (int)x-al_get_bitmap_width(b)/2-(int)camera.x, (int)y-al_get_bitmap_height(b)/2-(int)camera.y);
    }
};


class block_destruction_class
{
public:
    vector<block_destruction_particle_class> particle;

    block_destruction_class() { clear_all(); }

    void add_particles(float xx, float yy, float velocity_xx=0, float velocity_yy=0)
    {
         velocity_xx /= 2;
         velocity_yy /= 2;

         particle.push_back(block_destruction_particle_class(xx, yy, velocity_xx-1, velocity_yy-3, 1, 30 + rand()%40));
         particle.push_back(block_destruction_particle_class(xx, yy, velocity_xx-1, velocity_yy-1, 2, 20 + rand()%40));
         particle.push_back(block_destruction_particle_class(xx, yy, velocity_xx+1, velocity_yy+2, 3, 20 + rand()%40));
    }

    void clear_all()
    {
        for (int i=particle.size()-1; i>=0; i--) particle.pop_back();
    }

    void update()
    {
        for (int i=0; i<particle.size(); i++) { particle[i].update(); if (!particle[i].active) particle.erase(particle.begin()+i); }
    }

    void draw()
    {
        for (int i=0; i<particle.size(); i++) particle[i].draw();
    }
};



