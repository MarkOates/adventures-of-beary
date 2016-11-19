ALLEGRO_BITMAP *poof = NULL;

class poof_class
{
public:
    int counter;
    int_coordinate map_pos;

    void set_coordinates(int xx, int yy)
    {
        counter = 20;
        map_pos.x = xx;
        map_pos.y = yy;
    }
};

vector<poof_class> poofs;

void update_poofs()
{
    int k=0;
    bool there_are_still_poofs = false;
    for (k=poofs.size()-1; k>=0; k--)
    {
        poofs[k].counter--;
        if (poofs[k].counter <= 0)
        {
            poofs[k].counter = 0;
        }
        else there_are_still_poofs = true;
    }

    if (!there_are_still_poofs && (poofs.size() != 0)) poofs.pop_back();
}

void draw_poofs(ALLEGRO_BITMAP *b=buffer)
{
    int k=0;
    for (k=0; k<poofs.size(); k++)
    {
        if (poofs[k].counter != 0)
        {
            draw_sprite(b, poof, poofs[k].map_pos.x-(al_get_bitmap_width(poof)/2)-(int)camera_x, poofs[k].map_pos.y-(al_get_bitmap_height(poof)/2)-(int)camera_y);
        }
    }
}



void empty_the_poofs_vector()
{
    int k=0;
    for (k=poofs.size()-1; k>=0; k--) {poofs.pop_back();}
}


