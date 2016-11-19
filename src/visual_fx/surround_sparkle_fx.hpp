

#define SOLVE_DOOR_START_ANGLE    -90.0f

#define START_RADIUS  100
#define END_RADIUS     0

#define SOLVE_DOOR_ANIMATION_DURATION  150
#define SOLVE_DOOR_WAIT_A_SEC_DURATION  50


class solved_door_animation_class
{
public:
    float start_x; float start_y;
    float end_x; float end_y;

    int distance_counter;

    float angle;
    float angle_speed;

    bool active;

    int wait_a_sec_duration;
    bool poped_the_door;

    solved_door_animation_class(float start_xx, float start_yy, float end_xx, float end_yy, int duration=SOLVE_DOOR_ANIMATION_DURATION)
    {
        start_x = start_xx;
        start_y = start_yy;

        end_x = end_xx;
        end_y = end_yy;

        active = true;
        poped_the_door = false;

        angle = SOLVE_DOOR_START_ANGLE;

        distance_counter = duration;

        wait_a_sec_duration = SOLVE_DOOR_WAIT_A_SEC_DURATION;

        //distance = 40.0f;
        angle_speed = 7.0f;
    }

    float get_center_x()
    {
       return (start_x - end_x)*get_distance_normal() + end_x;
    }

    float get_center_y()
    {
       return (start_y - end_y)*get_distance_normal() + end_y;
    }

    float get_distance_normal()
    {
       //return ((float)distance_counter / (float)SOLVE_DOOR_ANIMATION_DURATION)*-1.0f + 1.0f;
       return ((float)distance_counter / (float)SOLVE_DOOR_ANIMATION_DURATION);
    }

   bool update()
   {
       if (active)
       {
          if (poped_the_door)
          {
              wait_a_sec_duration--;

              if (wait_a_sec_duration < 0)
              {
                 wait_a_sec_duration = 0;
                 active = false;
              }
          }
          else
          {
             angle += angle_speed;
             distance_counter--;

             if (distance_counter < 0)
             {
                distance_counter = 0;
                //active = false;
                poped_the_door = true;
                play_use_key_sound();
                particle_effect_manager.add_cluster(end_x, end_y, 0, -2.0f, 30.0f, 4.0f, 100, 30, 100);
                //door_drawing_manager.update_doors(&current_level);
             }


             float distance_normal = get_distance_normal();

             float final_radius = (START_RADIUS - END_RADIUS)*distance_normal + END_RADIUS;
             float center_x = (start_x - end_x)*distance_normal + end_x;
             float center_y = (start_y - end_y)*distance_normal + end_y;

             float particle_x = cos(get_radians(angle)) * final_radius + center_x;
             float particle_y = sin(get_radians(angle)) * final_radius + center_y;

             particle_effect_manager.add_cluster(particle_x, particle_y, 0, 0, 0.0f, 4.0f, 2, 20, 50);
          }
       }

       return active;
   }

   void draw()
   {
       //if (active) draw_item_centered(item_type, (int)(x-camera.x), (int)(y-camera.y));
   }
};





class solved_door_animation_manager_class
{
public:
    vector<solved_door_animation_class> animation;

    solved_door_animation_manager_class()
    {
        clear_all();
    }

    void start_effect(float start_xx, float start_yy, float end_xx, float end_yy, int duration=SOLVE_DOOR_ANIMATION_DURATION)
    {
        animation.push_back(solved_door_animation_class(start_xx, start_yy, end_xx, end_yy, duration));
    }

    void clear_all()
    {
        for (int i=animation.size()-1; i>=0; i--) animation.pop_back();
    }

    void update()
    {
        for (int i=animation.size()-1; i>=0; i--) animation[i].update();
    }

    bool active()
    {
        for (int i=animation.size()-1; i>=0; i--)
        {
            if (animation[i].active) return true;
        }

        return false;
    }

    float get_x()
    {
        if (animation.size() > 0) return animation[0].get_center_x();
        return -1.0f;
    }

    float get_y()
    {
        if (animation.size() > 0) return animation[0].get_center_y();
        return -1.0f;
    }
};



