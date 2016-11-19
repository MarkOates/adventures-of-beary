
ALLEGRO_BITMAP *get_random_sparkle()
{
    int num = rand()%4;

    switch(num)
    {
       case 0: return sparkle_particle_0; break;
       case 1: return sparkle_particle_1; break;
       case 2: return sparkle_particle_2; break;
       case 3: return sparkle_particle_3; break;
       default: return sparkle_particle_0; break;
    }

    return NULL;
}


class sparkle_effect_class
{
public:
    float x;
    float y;
    float velocity_x;
    float velocity_y;

    bool on_switch;

    int lifespan;

    bool active;

    ALLEGRO_BITMAP *img;

    // ---

    sparkle_effect_class(float xx, float yy, float velocity_xx, float velocity_yy, int _lifespan, bool _on_switch=true)
    {
       x = xx;
       y = yy;
       velocity_x = velocity_xx;
       velocity_y = velocity_yy;

       lifespan = _lifespan;

       on_switch = _on_switch;
       active = true;
    }

    void set_image(ALLEGRO_BITMAP *b)
    {
        img = b;
    }

    void update_vertical_velocity()
    {
        bool char_collides_next = false;
        bool char_collides_now = false;

        if (velocity_y > 0.0f) // falling
        {
            //if (block_type_at((int)(x), (int)(y+velocity_y)+1) == BLOCK_DESTRUCTABLE) return;
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
            //if (velocity_y < -1.0f) play_ball_bounce_sound(x - camera.x);
        }
        else if (velocity_y < 0.0f) // jumping
        {
            //if (block_type_at((int)(x), (int)(y+velocity_y)-1) == BLOCK_DESTRUCTABLE) return;
            if (is_ceiling_block(block_type_at((int)(x), (int)(y+velocity_y)-1)))
            {
                velocity_y *= -0.8f;
                y = block_bottom_edge(((int)(y+velocity_y-8)-1)/16) + 1;
                //play_ball_bounce_sound(x - camera.x);
            }
        }
    }

    bool update()
    {
       if (active)
       {

          velocity_y += GRAVITY/2;
          update_vertical_velocity();

          x += velocity_x;
          y += velocity_y;
          on_switch = !on_switch;

          lifespan -= 1;
          if (lifespan < 0)
          {
            lifespan = 0;
            active = false;
          }
       }

       return active;
    }

    void draw()
    {
       if (active && img) draw_sprite(buffer, img, (int)(x-camera.x), (int)(y-camera.y));
    }
};




class sparkle_effect_manager_class
{
public:
    vector<sparkle_effect_class> sparkle;

    sparkle_effect_manager_class()
    {
        clear_all();
    }

    void clear_all()
    {
        for (int i=sparkle.size()-1; i>=0; i--) sparkle.pop_back();
    }

    void add_sparkle(float x, float y, float velocity_x, float velocity_y, int lifespan, bool on_switch=true)
    {
        sparkle.push_back(sparkle_effect_class(x, y, velocity_x, velocity_y, lifespan, on_switch));
    }

    void add_cluster(float x, float y, float velocity_x, float velocity_y, float padding, float velocity_padding, int density=10, int lifespan_min=10, int lifespan_max=30)
    {
        float particle_x;
        float particle_y;
        float particle_velocity_x;
        float particle_velocity_y;
        bool particle_on;
        int lifespan;

        for (int i=0; i<density; i++)
        {
            particle_x = random_float(0, padding) - padding/2 + x;
            particle_y = random_float(0, padding) - padding/2 + y;

            particle_velocity_x = random_float(0, velocity_padding) - velocity_padding/2 + velocity_x;
            particle_velocity_y = random_float(0, velocity_padding) - velocity_padding/2 + velocity_y - random_float(-1.0, 1.5f);

            particle_on = random_bool();

            lifespan = random_int(lifespan_min, lifespan_max);

            add_sparkle(particle_x, particle_y, particle_velocity_x, particle_velocity_y, lifespan, particle_on);
            sparkle[sparkle.size()-1].set_image(get_random_sparkle());
        }

    }

    void draw()
    {
        for (int i=0; i<sparkle.size(); i++) sparkle[i].draw();
    }

    void update()
    {
        for (int i=0; i<sparkle.size(); i++) { if (!sparkle[i].update()) sparkle.erase(sparkle.begin() + i); }
    }
};


