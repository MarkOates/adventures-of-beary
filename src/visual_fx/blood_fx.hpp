
///////// blood

class blood_drop_class
{
public:
    float_coordinate velocity;
    float_coordinate map_pos;
    int counter;
    bool on_ground;
    //bool size

    blood_drop_class()
    {
        velocity.x=0;
        velocity.y=0;
        map_pos.y=0;
        map_pos.y=0;
        counter = 600;
        on_ground = false;
    }


    void set_coordinate(int xx, int yy)
    {
        map_pos.x = xx;
        map_pos.y = yy;
        on_ground = false;
    }
    void set_velocity(float xx, float yy)
    {
        velocity.x = xx;
        velocity.y = yy;//yy;
        counter = 500; //(int)((rand() * ((velocity.x+velocity.y)/2*100)) + 300);
        on_ground = false;
    }

    void update_vertical_velocity(void)
    {
        if (!on_ground) velocity.y += GRAVITY;
    }
    void update_horizontal_velocity(void)
    {
        if (velocity.x > 0.0f)
        {
           velocity.x -= 0.02f;
           if (velocity.x < 0.0f) velocity.x = 0;
        }
        else if (velocity.x < 0.0f)
        {
           velocity.x += 0.02f;
           if (velocity.x > 0.0f) velocity.x = 0;
        }
    }

    bool at_platform()
    {
        if (is_platform_block(block_type_at((int)(map_pos.x), (int)(map_pos.y)+1))) return true;
        else return false;
    }

    void test_vertical_map_collisions()
    {
        bool char_collides_next = false;
        bool char_collides_now = false;
        if (velocity.y > 0.0f) // falling
        {
            if (is_platform_block(block_type_at((int)(map_pos.x), (int)(map_pos.y+velocity.y)+1)))
            {
                char_collides_next = true;
            }
            if (is_platform_block(block_type_at((int)(map_pos.x), (int)map_pos.y)))
            {
                char_collides_now = true;
            }
        }
        if ((!char_collides_now) && (char_collides_next))
        {
            velocity.y = 0.0f;
            map_pos.y = block_top_edge(((int)(map_pos.y+velocity.y+8)+1)/16) - 1;
            on_ground = true;
        }
        else if (velocity.y < 0.0f) // jumping
        {
            if (is_ceiling_block(block_type_at((int)(map_pos.x), (int)(map_pos.y+velocity.y)-1)))
            {
                velocity.y = 0.0f;
                map_pos.y = block_bottom_edge(((int)(map_pos.y+velocity.y-8)-1)/16) + 1;
            }
        }
    }

    void update_pos()
    {
        if (!at_platform()) on_ground = false;

        update_vertical_velocity();
        test_vertical_map_collisions();
        map_pos.y += velocity.y;
        update_horizontal_velocity();
        map_pos.x += velocity.x;
    }

};

vector<blood_drop_class> blood_drops;

void make_drops(int drops, float xx, float yy)
{
    if (drops < 0) drops = 0;

    int k=0;
    bool there_are_still_drops = false;
    for (k=0; k<drops; k++)
    {
        blood_drop_class gimp_blood_drops_class;
        
        gimp_blood_drops_class.set_coordinate((int)xx, (int)yy);
        gimp_blood_drops_class.set_velocity((rand()%(500))/100.0f - 2.5f, (rand()%(400))/100.0f - 4.0f);
        blood_drops.push_back(gimp_blood_drops_class);
    }
}

void make_drops2(int drops, float xx, float yy)
{
    if (drops < 0) drops = 0;

    int k=0;
    bool there_are_still_drops = false;
    for (k=0; k<drops; k++)
    {
        blood_drop_class gimp_blood_drops_class;
        
        gimp_blood_drops_class.set_coordinate((int)xx, (int)yy);
        gimp_blood_drops_class.set_velocity((rand()%(400))/100.0f - 2.0f, (rand()%(500))/100.0f - 5.0f);
        blood_drops.push_back(gimp_blood_drops_class);
    }
}

void update_blood_drops()
{
    int k=0;

    /// update the counters
    bool there_are_still_drops = false;
    for (k=blood_drops.size()-1; k>=0; k--)
    {
        blood_drops[k].update_pos();

        blood_drops[k].counter--;
        if (blood_drops[k].counter <= 0)
        {
            blood_drops[k].counter = 0;
        }
        else there_are_still_drops = true;
    }

    if (!there_are_still_drops && (blood_drops.size() != 0)) blood_drops.pop_back();
}

void draw_blood_drops(ALLEGRO_BITMAP *b=buffer)
{
    int k=0;

    int size;

    for (k=blood_drops.size()-1; k>=0; k--)
    {
        if (blood_drops[k].counter > 490)
        {
            size = 5;
        }
        else if (blood_drops[k].counter > 480)
        {
            size = 2;
        }
        else if (blood_drops[k].counter > 470)
        {
            size = 1;
        }
        else if (blood_drops[k].counter > 460)
        {
            size = 1;
        }
        else if (blood_drops[k].counter > 450)
        {
            size = 0;
        }
        else size = 0;

        if (blood_drops[k].counter != 0)
        {
            if (!blood_drops[k].on_ground)
            {
               if (ABS((int)blood_drops[k].velocity.y) >= 3.0f)
               {
                   ellipsefill(b, ((int)blood_drops[k].map_pos.x-(int)camera_x), ((int)blood_drops[k].map_pos.y-(int)camera_y),
                                  (((size+1)+1))+size,
                                  (((size+1)+1))+2+size,
                                  makecol(160, 0, 0));

                   // type 2
                   //rectfill(b, (int)(blood_drops[k].map_pos.x-camera_x)-(rand()%(size+1)+(rand()%1)),
                   //            (int)(blood_drops[k].map_pos.y-camera_y)-3-(rand()%(size+1)+(rand()%1)),
                   //            (int)(blood_drops[k].map_pos.x-camera_x)+1,
                   //            (int)(blood_drops[k].map_pos.y-camera_y)+2+(rand()%(size+1)-(rand()%1)), makecol(180, 0, 0));

               }
               else if (ABS((int)blood_drops[k].velocity.y) >= 1.0f)
               {
                   ellipsefill(b, ((int)blood_drops[k].map_pos.x-(int)camera_x), ((int)blood_drops[k].map_pos.y-(int)camera_y),
                                  (((size+1)+(1))+1+size),
                                  (((size+1)+(1))+0+size),
                                  makecol(160, 0, 0));
                   //rectfill(b, (int)(blood_drops[k].map_pos.x-camera_x)-(rand()%(size+1)+(rand()%1)),
                   //            (int)(blood_drops[k].map_pos.y-camera_y)-1-(rand()%(size+1)+(rand()%1)),
                   //            (int)(blood_drops[k].map_pos.x-camera_x)+1,
                   //            (int)(blood_drops[k].map_pos.y-camera_y)+2, makecol(190, 0, 0));
                   //type 2
                   //rectfill(b, (int)(blood_drops[k].map_pos.x-camera_x)-(rand()%(size+1)+(rand()%1)),
                   //            (int)(blood_drops[k].map_pos.y-camera_y)-1-(rand()%(size+1)+(rand()%1)),
                   //            (int)(blood_drops[k].map_pos.x-camera_x)+1,
                   //            (int)(blood_drops[k].map_pos.y-camera_y)+2, makecol(190, 0, 0));

               }
               else
               {
                   putpixel(b, ((int)blood_drops[k].map_pos.x-(int)camera_x), ((int)blood_drops[k].map_pos.y-(int)camera_y), makecol(120, 0, 0));
                   putpixel(b, ((int)blood_drops[k].map_pos.x-(int)camera_x)+1, ((int)blood_drops[k].map_pos.y-(int)camera_y)+1, makecol(120, 0, 0));
                   putpixel(b, ((int)blood_drops[k].map_pos.x-(int)camera_x), ((int)blood_drops[k].map_pos.y-(int)camera_y)+1, makecol(175, 0, 0));
                   putpixel(b, ((int)blood_drops[k].map_pos.x-(int)camera_x)+1, ((int)blood_drops[k].map_pos.y-(int)camera_y), makecol(200, 128, 128));
               }
            }
            else
            {
                //type 1
                hline(b, ((int)blood_drops[k].map_pos.x-(int)camera_x)-3, ((int)blood_drops[k].map_pos.y-(int)camera_y), ((int)blood_drops[k].map_pos.x-(int)camera_x)+3, makecol(120, 0, 0));
                hline(b, ((int)blood_drops[k].map_pos.x-(int)camera_x)-2, ((int)blood_drops[k].map_pos.y-(int)camera_y)-1, ((int)blood_drops[k].map_pos.x-(int)camera_x)+1, makecol(160, 0, 0));
                putpixel(b, ((int)blood_drops[k].map_pos.x-(int)camera_x)+2, ((int)blood_drops[k].map_pos.y-(int)camera_y)-1, makecol(200, 132, 132));

                // type 2
                //putpixel(b, (int)(blood_drops[k].map_pos.x-camera_x)-1, (int)(blood_drops[k].map_pos.y-camera_y), makecol(180, 0, 0));
                //putpixel(b, (int)(blood_drops[k].map_pos.x-camera_x), (int)(blood_drops[k].map_pos.y-camera_y), makecol(180, 0, 0));
                //putpixel(b, (int)(blood_drops[k].map_pos.x-camera_x)+1, (int)(blood_drops[k].map_pos.y-camera_y), makecol(255, 132, 132));
            }
        }
    }
}

void empty_the_blood_drop_vector()
{
    int k=0;
    for (k=blood_drops.size()-1; k>=0; k--) {blood_drops.pop_back();}
}

