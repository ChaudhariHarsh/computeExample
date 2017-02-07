#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include "camera.h"
#include "ShaderProgram.hpp"
#include "utils.h"

using namespace std;

GLuint VAO, VBO;
GLuint computeShader,computeProgram;
SDL_Event e;
SDL_Window* window = nullptr;
bool quit = false;

int main()
{
#pragma region SDL_INIT
	
	//int start = NULL;
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

#pragma region CAMERA_CODE

	Camera cam;
	cam.SetPosition(glm::vec3(0, 0, 35));
	int m = 4;
#pragma endregion CAMERA_CODE

#pragma region SHADERS
	unique_ptr<ShaderProgram> viewShader(new ShaderProgram());
	viewShader->initFromFiles("render.vert","render.frag");
	viewShader->addAttribute("pos");
	viewShader->addUniform("view");
	viewShader->addUniform("projection");

	//now setup compute shader
	ifstream file("compute.cs");
	if (!file.good())
	{
		throw std::runtime_error("Failed to open file: compute.cs");
	}
	stringstream stream;
	const char *source;
	int length;

	stream << file.rdbuf();
	file.close();
	string cs_shader_src = stream.str();
	computeShader = glCreateShader(GL_COMPUTE_SHADER);
	source = cs_shader_src.c_str();
	length = cs_shader_src.length();
	glShaderSource(computeShader, 1, &source, &length);
	glCompileShader(computeShader);
	//check_program_compile_status(computeShader);
	computeProgram = glCreateProgram();
	glAttachShader(computeProgram, computeShader);
	glLinkProgram(computeProgram);
	GLint status;
    glGetProgramiv(computeProgram, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(computeProgram, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(computeProgram, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;
    }
    //check_program_link_status(acceleration_program);
	//By this point out compute shader must be created.

#pragma endregion SHADERS

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);	//clear screen
	glClear(GL_COLOR_BUFFER_BIT);
	
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			glClear(GL_COLOR_BUFFER_BIT);
			switch (e.type)
			{
			case SDL_QUIT:	//if X windowkey is pressed then quit
				quit = true;
			
			case SDL_KEYDOWN :	//if ESC is pressed then quit
				
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;

				

				case SDLK_e:
					std::cout << "E pressed \n";
					break;

				}
				break;
				
			}
		}
		//glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(window);
	}
	//Exit
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
    SDL_Quit();

}