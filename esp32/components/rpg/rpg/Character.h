#pragma once

#include <string>

namespace rpg
{

class Viewport;

class Character
{
  public:
    Character() : Character{"", 0, 0}
    {
    }

    Character(const char *name) : Character{name, 0, 0}
    {
    }

    Character(const char *name, int width, int height)
        : name{name}, width{width}, height{height}
    {
        rendered_scene_x = 0;
        rendered_scene_y = 0;
        scene_x = 0;
        scene_y = 0;
    }

    const std::string get_name() const
    {
        return name;
    }

    int get_height() const
    {
        return height;
    }

    int get_rendered_scene_x() const
    {
        return rendered_scene_x;
    }

    int get_rendered_scene_y() const
    {
        return rendered_scene_y;
    }

    int get_scene_x() const
    {
        return scene_x;
    }

    int get_scene_y() const
    {
        return scene_y;
    }

    int get_width() const
    {
        return width;
    }

    void move(int new_scene_x, int new_scene_y)
    {
        scene_x = new_scene_x;
        scene_y = new_scene_y;
    }

    virtual void render(Viewport &viewport) = 0;

  private:
    const std::string name;
    const int width;
    const int height;
    int rendered_scene_x;
    int rendered_scene_y;
    int scene_x;
    int scene_y;
};

} // namespace rpg