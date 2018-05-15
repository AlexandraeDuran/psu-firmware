/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "window.h"
#include "texture.h"

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>
#include <map>

namespace eez {
namespace platform {
namespace simulator {
namespace imgui {

/// Implementation of the Window.
class WindowImpl {
public:
    WindowImpl(WindowDefinition *window_definition);
    ~WindowImpl();

    bool init();

    bool pollEvent();

    void beginUpdate();

    void addImage(int x, int y, int w, int h, const char *image);
    void addOnOffImage(int x, int y, int w, int h, bool value, const char *on_image, const char *off_image);
    void addText(int x, int y, int w, int h, const char *text);
    bool addButton(int x, int y, int w, int h, const char *normal_image, const char *pressed_image);
    void addUserWidget(UserWidget *user_widget);

    void endUpdate();

    void getMouseData(MouseData *mouseData);
    void getMouseWheelData(int *x, int *y);

private:
    Texture *getTexture(const char *path);

    WindowDefinition *window_definition;
    SDL_Window *sdl_window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    typedef std::map<std::string, Texture *> TextureMap;
    TextureMap textures;

    MouseData mouseData;

    int xMouseWheel;
    int yMouseWheel;

    bool pointInRect(int px, int py, int x, int y, int w, int h);
};

std::string getFullPath(std::string category, std::string path);

}
}
}
} // namespace eez::platform::simulator::imgui
