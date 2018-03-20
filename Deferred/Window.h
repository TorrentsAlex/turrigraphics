#ifndef TURRI_WINDOW
#define TURRI_WINDOW


#include <iostream>
#include <SDL2/SDL.h>
#include <GL/glew.h>


#include <glm/glm.hpp>
#include <string>
//This class manages the window input/output using the SDL library

//Create flags for the management of the window properties
enum class WindowFlags { INVISIBLE = 0x1, FULLSCREEN = 0x2, BORDERLESS = 0x3 };

using namespace std;

class Window 	{
public:
	Window();
	~Window();

	int create(string windowName, int screenWidth, int screenHeight, unsigned int currentFlags);

	//int getScreenWidth() { return _screenWidth; }
	//int getScreenHeight() { return _screenHeight; }
	void swapBuffer();

	void setScreenSize(int resolutionX, int resolutionY);
	void setFullScreen();
	void setWindowedScreen();
	glm::vec2 getNativeResolution();

	SDL_Window* getWindow();
private:
	SDL_DisplayMode wSDLDisplayMode;
	//TTF_Font* g_font;
	SDL_Color windowBackGroundColor;
	int _windowHeight, _windowWidth;
	int _windowHeightFullScreen, _windowWidthFullScreen;

	SDL_Window * _sdlWindow;
	int _screenHeight;
	int _screenWidth;

};
#endif
