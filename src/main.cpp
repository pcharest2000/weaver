//#include "mainWindow.h"
#include "mainWindow.h"
#include "weaveImage.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

#include <memory>
#include <sdlgui/button.h>
#include <sdlgui/checkbox.h>
#include <sdlgui/colorwheel.h>
#include <sdlgui/combobox.h>
#include <sdlgui/dropdownbox.h>
#include <sdlgui/entypo.h>
#include <sdlgui/formhelper.h>
#include <sdlgui/graph.h>
#include <sdlgui/imagepanel.h>
#include <sdlgui/imageview.h>
#include <sdlgui/label.h>
#include <sdlgui/layout.h>
#include <sdlgui/messagedialog.h>
#include <sdlgui/popupbutton.h>
#include <sdlgui/progressbar.h>
#include <sdlgui/screen.h>
#include <sdlgui/slider.h>
#include <sdlgui/switchbox.h>
#include <sdlgui/tabwidget.h>
#include <sdlgui/textbox.h>
#include <sdlgui/toolbutton.h>
#include <sdlgui/vscrollpanel.h>
#include <sdlgui/window.h>

using namespace sdlgui;
auto main() -> int {
  SDL_Window *window; // Declare a pointer to an SDL_Window
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // int winWidth = 1024;
  // int winHeight = 768;

  int winWidth = 800;
  int winHeight = 800;
  // Create an application window with the following settings:
  window = SDL_CreateWindow(
      "An SDL2 window",        //    const char* title
      SDL_WINDOWPOS_UNDEFINED, //    int x: initial x position
      SDL_WINDOWPOS_UNDEFINED, //    int y: initial y position
      winWidth,                //    int w: width, in pixels
      winHeight,               //    int h: height, in pixels
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_SHOWN |
          SDL_RENDERER_PRESENTVSYNC //   Uint32 flags: window options, see docs
  );

  // Check that the window was successfully made
  if (window == NULL) {
    // In the event that the window could not be made...
    std::cout << "Could not create window: " << SDL_GetError() << '\n';
    SDL_Quit();
    return 1;
  }

  auto context = SDL_GL_CreateContext(window);

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  WeaveImage weaImage(renderer, winWidth, "assets/david1300s.jpg", 220, 5000,
                      14);
//The gui
  MainWindow *screen = new MainWindow(window, &weaImage, winWidth, winHeight);


  SDL_Event event;
  for (;;) {

    while (SDL_PollEvent(&event) != 0) {
      if (event.type == SDL_QUIT) {
        std::cout << "Exit message" << std::endl;
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        return 0;
      }
      screen->onEvent(event);
    }
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderClear(renderer);
    weaImage.render();
    // Render the gui
    screen->drawAll();
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  return 0;
}
