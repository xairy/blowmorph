#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <set>
#include <map>
#include <vector>
#include <string>

#define GL3_PROTOTYPES 1
#include <GL3/gl3.h>
#include <SDL/SDL.h>

class SDLUtils {
private:
    static size_t initCount;
    
public:

    // a simple function that prints a message,
    // the error code returned by SDL, and quits
    // the application
    static void die(const char *msg) {
        fprintf(stderr, "%s: %s\n", msg, SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    
    static void init() {
        // initialize SDL's Video subsystem
	    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		    sdlError("Unable to initialize SDL");
	    }
    
        initCount++;
    }
    
    static void close() {
        initCount--;
        
        if (initCount == 0) {
            SDL_Quit();
        }    
    }
};
size_t SDLUtils::initCount = 0;

class RenderWindowListener {
public:
    virtual RenderWindowListener() { };
    virtual void onEvent(SDL_Event& event) = 0;
};

class RenderWindow {
private:
    SDL_Window* sdlWindow;
    SDL_GLContext sdlGLContext;
    
    size_t width;
    size_t height;
    
    bool vsync;
    
    RenderWindowListener* listener;
    
public:
    RenderWindow(const char* title, size_t width, size_t height, bool vsync) {
        this->width = width;
        this->height = height;
        this->vsync = vsync;
    
        SDLUtils::init();
	    
	    // request an opengl 3.2 context.
	    // SDL doesn't have the ability to choose
	    // which profile at this time of writing,
	    // but it should default to the core profile
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	    // turn on double buffering with
	    // a 24bit Z buffer. you may need to change
	    // this to 16 or 32 for your system
	    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	    
	    this->sdlWindow = SDL_CreateWindow(
	        title,
	        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	        width, height,
	        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	    );
	    
	    if (!this->sdlWindow) {
	        sdlError("Unable to create a window.");
	    }
	    
	    // create our opengl context and attach it to our window
	    this->sdlGLContext = SDL_GL_CreateContext(this->sdlWindow);

	    // this makes our buffer swap syncronized
	    // with the monitor's vertical refresh
	    if (this->vsync) {
		    SDL_GL_SetSwapInterval(1);
	    }
    }
    
    void setListener(RenderWindowListener* listener) {
        this->listener = listener;
    }
    
    void swapBuffers() {
        SDL_GL_SwapWindow(this->sdlWindow);
    }
    
    void pumpEvents() {
        SDL_Event event;
        
        while (SDL_PollEvent(&event)) {
            if (listener) {
                listener->onEvent(event);
            }             
        }
    }
    
    void close() {
        // delete our opengl context
	    SDL_GL_DeleteContext(rw->sdlGLContext);
	    // destroy our window
	    SDL_DestroyWindow(rw->sdlWindow);
	    // and shutdown SDL
	    SDLUtils::quit();
    }
};

class GameWindow : public RenderWindow, RenderWindowListener {
public:
    GameWindow(const char* title, size_t width, size_t height, bool vsync) : RenderWindow(title, width, height, vsync) {
        
    }
    
    
};

int main(int argc, char** argv) {
    bool running = true;
    GameWindow window;
    
    while (running) {
        window->pumpEvents();
        window->swapBuffers();
    }
    
    return EXIT_SUCCESS;
}
