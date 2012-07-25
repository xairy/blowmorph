#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <SDL/SDL.h>
#include <glm/glm.hpp>

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>

void raise_error(const char* error) {
  fprintf(stderr, "%s\n", error);
}

class Shader {
public:
  Shader() : _state(SHADER_CREATED) { }
  ~Shader() { }

  bool Init(GLenum shader_type, const std::string& file) {
    assert(_state == SHADER_CREATED);

    std::ifstream file_fstream(file.c_str());
    std::stringstream file_sstream;
    file_sstream << file_fstream.rdbuf();
    file_fstream.close();

    std::string source = file_sstream.str();

    _shader = glCreateShader(shader_type);
    const char* source_c_str = source.c_str();

    glShaderSource(_shader, 1, &source_c_str, NULL);
    glCompileShader(_shader);

    GLint status;
    glGetShaderiv(_shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
      GLint log_length;
      glGetShaderiv(_shader, GL_INFO_LOG_LENGTH, &log_length);

      GLchar* log = new GLchar[log_length + 1];
      glGetShaderInfoLog(_shader, log_length, NULL, log);

      const char* shader_type_name = NULL;
      switch(shader_type) {
        case GL_VERTEX_SHADER:
          shader_type_name = "vertex";
          break;
        case GL_GEOMETRY_SHADER:
          shader_type_name = "geometry";
          break;
        case GL_FRAGMENT_SHADER:
          shader_type_name = "fragment";
          break;
      }

      std::string error_description = std::string("Compile failure in ") +
        std::string(shader_type_name) + std::string(" shader\n") + std::string(log);
      raise_error(error_description.c_str());

      delete[] log;
      return false;
    }

    _state = SHADER_INITIALIZED;
    return true;
  }

  GLuint GetId() const {
    return _shader;
  }

private:
  enum {
    SHADER_CREATED,
    SHADER_INITIALIZED
  } _state;

  GLuint _shader;
};

class Program {
public:
  Program() : _state(PROGRAM_CREATED) { }
  ~Program() { }

  bool Init(const std::vector<Shader*>& shaders) {
    assert(_state == PROGRAM_CREATED);

    _program = glCreateProgram();

    for(size_t i = 0; i < shaders.size(); i++) {
      glAttachShader(_program, shaders[i]->GetId());
    }

    glLinkProgram(_program);

    GLint status;
    glGetProgramiv(_program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
      GLint log_length;
      glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &log_length);

      GLchar* log = new GLchar[log_length + 1];
      glGetProgramInfoLog(_program, log_length, NULL, log);

      std::string error_description = std::string("Linker failure:\n") + std::string(log);
      raise_error(error_description.c_str());

      delete[] log;
    }

    for(size_t i = 0; i < shaders.size(); i++) {
      glDetachShader(_program, shaders[i]->GetId());
    }

    if(status == GL_TRUE) {
      _state = PROGRAM_INITIALIZED;
    }
    return status == GL_TRUE;
  }

  GLuint GetId() const {
    return _program;
  }

private:
  enum {
    PROGRAM_CREATED,
    PROGRAM_INITIALIZED
  } _state;

  GLuint _program;
};

#define ARRAY_COUNT( array ) (sizeof( array ) / (sizeof( array[0] ) *\
  (sizeof( array ) != sizeof(void*) || sizeof( array[0] ) <= sizeof(void*))))

GLuint theProgram;

GLuint offsetUniform;
GLuint perspectiveMatrixUnif;

float perspectiveMatrix[16];
const float fFrustumScale = 1.0f;

float fzNear = 1.2f;
float fzFar = 3.0f;

void InitializeProgram()
{
  // TODO: fix the shaders' paths.
  Shader vertex_shader, fragment_shader;
  bool rv = vertex_shader.Init(GL_VERTEX_SHADER, "..\\src\\client\\vertex_shader.vert");
  assert(rv == true);
  rv = fragment_shader.Init(GL_FRAGMENT_SHADER, "..\\src\\client\\fragment_shader.frag");
  assert(rv == true);

  std::vector<Shader*> shaders;
  shaders.push_back(&vertex_shader);
  shaders.push_back(&fragment_shader);

  Program program;
  rv = program.Init(shaders);
  assert(rv == true);

  theProgram = program.GetId();

	offsetUniform = glGetUniformLocation(theProgram, "offset");

	perspectiveMatrixUnif = glGetUniformLocation(theProgram, "perspectiveMatrix");

	memset(perspectiveMatrix, 0, sizeof(float) * 16);

	perspectiveMatrix[0] = fFrustumScale;
	perspectiveMatrix[5] = fFrustumScale;
	perspectiveMatrix[10] = (fzFar + fzNear) / (fzNear - fzFar);
	perspectiveMatrix[14] = (2 * fzFar * fzNear) / (fzNear - fzFar);
	perspectiveMatrix[11] = -1.0f;

	glUseProgram(theProgram);
	glUniformMatrix4fv(perspectiveMatrixUnif, 1, GL_FALSE, perspectiveMatrix);
	glUseProgram(0);
}

const int numberOfVertices = 36;

#define RIGHT_EXTENT 0.8f
#define LEFT_EXTENT -RIGHT_EXTENT
#define TOP_EXTENT 0.20f
#define MIDDLE_EXTENT 0.0f
#define BOTTOM_EXTENT -TOP_EXTENT
#define FRONT_EXTENT -1.25f
#define REAR_EXTENT -1.75f

#define GREEN_COLOR 0.75f, 0.75f, 1.0f, 1.0f
#define BLUE_COLOR 	0.0f, 0.5f, 0.0f, 1.0f
#define RED_COLOR 1.0f, 0.0f, 0.0f, 1.0f
#define GREY_COLOR 0.8f, 0.8f, 0.8f, 1.0f
#define BROWN_COLOR 0.5f, 0.5f, 0.0f, 1.0f

const float vertexData[] = {
	//Object 1 positions
	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,

	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	LEFT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	MIDDLE_EXTENT,	FRONT_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	LEFT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,
	LEFT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	TOP_EXTENT,		REAR_EXTENT,
	RIGHT_EXTENT,	BOTTOM_EXTENT,	REAR_EXTENT,

	//Object 2 positions
	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,

	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	RIGHT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
					
	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,
	MIDDLE_EXTENT,	LEFT_EXTENT,	FRONT_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,
					
	BOTTOM_EXTENT,	RIGHT_EXTENT,	REAR_EXTENT,
	TOP_EXTENT,		RIGHT_EXTENT,	REAR_EXTENT,
	TOP_EXTENT,		LEFT_EXTENT,	REAR_EXTENT,
	BOTTOM_EXTENT,	LEFT_EXTENT,	REAR_EXTENT,

	//Object 1 colors
	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,

	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,

	RED_COLOR,
	RED_COLOR,
	RED_COLOR,

	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,

	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,

	//Object 2 colors
	RED_COLOR,
	RED_COLOR,
	RED_COLOR,
	RED_COLOR,

	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,
	BROWN_COLOR,

	BLUE_COLOR,
	BLUE_COLOR,
	BLUE_COLOR,

	GREEN_COLOR,
	GREEN_COLOR,
	GREEN_COLOR,

	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,
	GREY_COLOR,
};

const GLshort indexData[] =
{
	0, 2, 1,
	3, 2, 0,

	4, 5, 6,
	6, 7, 4,

	8, 9, 10,
	11, 13, 12,

	14, 16, 15,
	17, 16, 14,
};

GLuint vertexBufferObject;
GLuint indexBufferObject;

GLuint vao;

void InitializeVertexBuffer()
{
	glGenBuffers(1, &vertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &indexBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//Called after the window and OpenGL are initialized. Called exactly once, before the main loop.
void init()
{
	InitializeProgram();
	InitializeVertexBuffer();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	size_t colorDataOffset = sizeof(float) * 3 * numberOfVertices;
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)colorDataOffset);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);

	glBindVertexArray(0);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

  glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
}

float offset_x = 0.0f;
float offset_y = 0.0f;
float offset_z = 0.0f;

//Called to update the display.
//You should call glutSwapBuffers after all of your rendering to display what you rendered.
//If you need continuous updates of the screen, call glutPostRedisplay() at the end of the function.
void display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(theProgram);

	glBindVertexArray(vao);

	glUniform3f(offsetUniform, offset_x, offset_y, offset_z);
	glDrawElements(GL_TRIANGLES, ARRAY_COUNT(indexData), GL_UNSIGNED_SHORT, 0);

	glUniform3f(offsetUniform, offset_x, offset_y, -1.0f);
	glDrawElementsBaseVertex(GL_TRIANGLES, ARRAY_COUNT(indexData),
		GL_UNSIGNED_SHORT, 0, numberOfVertices / 2);

	glBindVertexArray(0);
	glUseProgram(0);
}

void reshape(int w, int h) {
  perspectiveMatrix[0] = fFrustumScale / (w / (float)h);
  perspectiveMatrix[5] = fFrustumScale;

  glUseProgram(theProgram);
  glUniformMatrix4fv(perspectiveMatrixUnif, 1, GL_FALSE, perspectiveMatrix);
  glUseProgram(0);

  glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void update_znear_zfar() {
  perspectiveMatrix[10] = (fzFar + fzNear) / (fzNear - fzFar);
	perspectiveMatrix[14] = (2 * fzFar * fzNear) / (fzNear - fzFar);

  glUseProgram(theProgram);
  glUniformMatrix4fv(perspectiveMatrixUnif, 1, GL_FALSE, perspectiveMatrix);
  glUseProgram(0);
}

bool bDepthClampingActive = false;

class Application {
public:
  Application() : _is_running(false) { }
  ~Application() { }

  bool Execute() {
    if(!Initialize()) {
      return false;
    }

    _is_running = true;

    while(_is_running) {
      PumpEvents();
      Loop();
      Render();
    }

    Destroy();

    return true;
  }

private:
  bool Initialize() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
      raise_error("Unable to initialize SDL!");
      return false;
    }

    // Request opengl 3.2 context.
    // SDL doesn't have the ability to choose which profile at this
    // time of writing, but it should default to the core profile.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Turn on double buffering with a 24bit Z buffer.
    // You may need to change this to 16 or 32 for your system.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create our window centered at 512x512 resolution
    _sdl_window = SDL_CreateWindow("Blowmorph Demo", SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(_sdl_window == NULL) {
      raise_error("Unable to create window!");
      return false;
    }

    // Create our opengl context and attach it to our window.
    _sdl_context = SDL_GL_CreateContext(_sdl_window);
    if(_sdl_context == NULL) {
      raise_error("Unable to create context!");
      return false;
    }

    // This makes our buffer swap syncronized with the monitor's vertical refresh.
    SDL_GL_SetSwapInterval(1);

    // Set up glew.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if(GLEW_OK != err) {
      raise_error("Unable to init GLEW!");
      return false;
    }

    // From tutorial.
    init();

    return true;
  }

  void ProcessEvent(SDL_Event* event) {
    switch(event->type) {
      case SDL_KEYDOWN:
        switch(event->key.keysym.sym){
          case SDLK_LEFT:
            offset_x -= 0.05f;
            break;
          case SDLK_RIGHT:
            offset_x += 0.05f;
            break;
          case SDLK_UP:
            offset_y += 0.05f;
            break;
          case SDLK_DOWN:
            offset_y -= 0.05f;
            break;
          case SDLK_F5:
            offset_z += 0.05f;
            break;
          case SDLK_F6:
            offset_z -= 0.05f;
            break;
          case SDLK_F1:
            fzNear += 0.02f;
            update_znear_zfar();
            break;
          case SDLK_F2:
            fzNear -= 0.02f;
            update_znear_zfar();
            break;
          case SDLK_F3:
            fzFar += 0.02f;
            update_znear_zfar();
            break;
          case SDLK_F4:
            fzFar -= 0.02f;
            update_znear_zfar();
            break;
          case SDLK_SPACE:
            if(bDepthClampingActive) {
              glDisable(GL_DEPTH_CLAMP);
            } else {
              glEnable(GL_DEPTH_CLAMP);
            }

            bDepthClampingActive = !bDepthClampingActive;
            break;
          default:
            _is_running = false;
            break;
        }
        break;
      case SDL_QUIT:
        _is_running = false;
        break;
      case SDL_WINDOWEVENT:
        switch (event->window.event) {
          case SDL_WINDOWEVENT_RESIZED:
            reshape(event->window.data1, event->window.data2);
            break;
        }
        break;
    }
  }

  void PumpEvents() {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
      ProcessEvent(&event);
    }
  }

  void Loop() {

  }

  void Render() {
    // From tutorial.
    display();

    SwapBuffers();
  }

  void Destroy() {
    SDL_GL_DeleteContext(_sdl_context);
    SDL_DestroyWindow(_sdl_window);
    SDL_Quit();
  }

  void SwapBuffers() {
    SDL_GL_SwapWindow(_sdl_window);
  }

private:
  SDL_Window* _sdl_window;
  SDL_GLContext _sdl_context;

  bool _is_running;
};

int main(int argc, char** argv) {
  Application app;
  if(!app.Execute()) {
    //_CrtDumpMemoryLeaks();
    return EXIT_FAILURE;
  }
  //_CrtDumpMemoryLeaks();
  return EXIT_SUCCESS;
}
