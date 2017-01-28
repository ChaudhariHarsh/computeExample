#include<SDL2/SDL.h>
#include<GL/glew.h>
#include<GL/glu.h>
#include<iostream>

using namespace std;

GLuint VAO, VBO;
SDL_Event e;
SDL_Window* window = nullptr;


int main()
{
#pragma region SDL_INIT
	
	Uint32 start = NULL;
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


	window = SDL_CreateWindow("GLSL Compute example", 200, 30, 1024, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cout << "Sorry, but GLEW failed to load.";
		return 1;
	}

#pragma endregion SDL_INIT

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);	//clear screen
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapWindow(window);

}