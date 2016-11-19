
#define USE_KEY_START_ANGLE    -90.0f
class use_key_animation_class
{
public:
    float x; float y;
    float center_x; float center_y;
    int item_type;

    float distance;
    float angle;
    float angle_speed;

    bool active;

    use_key_animation_class(float center_xx, float center_yy, bool silver_key=false)
    {
        center_x = center_xx;
        center_y = center_yy;

        active = true;
        angle = USE_KEY_START_ANGLE;
        distance = 40.0f;
        angle_speed = 7.0f;
        if (silver_key) item_type = silver_key_const;
        else item_type = key_const;
    }

   bool update()
   {
       if (active)
       {
          angle += angle_speed;

          x = cos(get_radians(angle)) * distance + center_x;
          y = sin(get_radians(angle)) * distance + center_y;

          //particle_effect_manager.add_cluster(x, y, 0, 0, 20.0f, 3.0f, 3, 20, 50);

          if (angle > (USE_KEY_START_ANGLE+360.0f))
          {
             active = false;
             play_use_key_sound();
             particle_effect_manager.add_cluster(x, y, 0, 0, 30.0f, 4.0f, 70, 30, 100);
             // open_door;
          }
       }

       return active;
   }

   void draw()
   {
       if (active) draw_item_centered(item_type, (int)(x-camera.x), (int)(y-camera.y));
   }
};



class use_key_animation_manager_class
{
public:
   vector<use_key_animation_class> key_object;


   use_key_animation_manager_class()
   {
       clear_all();
   }

   void add_key(float x, float y, bool silver_key=false)
   {
       key_object.push_back(use_key_animation_class(x, y, silver_key));
   }

   void update()
   {
       for (int i=0; i<key_object.size(); i++)
       {
           if (!key_object[i].update()) key_object.erase(key_object.begin() + i);
       }
   }

   void draw()
   {
       for (int i=0; i<key_object.size(); i++) key_object[i].draw();
   }

   void clear_all()
   {
       for (int i=key_object.size()-1; i>=0; i--) key_object.pop_back();
   }

   bool active()
   {
       if (key_object.size() == 0) return false;
       return true;
   }

};


