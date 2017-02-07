#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>

#include "camera.h"
#include "ShaderProgram.hpp"
#include "utils.h"

using namespace std;
void render(unique_ptr<ShaderProgram>&, Camera&);
void update(GLuint, GLuint);

GLuint posBuffer, velBuffer;
GLuint computeShader,computeProgram;
GLuint integrationShader, integrationProgram;
glm::mat4 model, view;
glm::mat4 projection = glm::perspective(45.0f, 800.0f / 600.0f, 1.0f, 100.0f);

SDL_Event e;
SDL_Window* window = nullptr;
bool quit = false;

const int numParticles = 1000000;

int main()
{
#pragma region SDL_INIT
	
	//int start = NULL;
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


	window = SDL_CreateWindow("GLSL Compute example", 200, 30, 1024, 768, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cout << "Sorry, but GLEW failed to load.";
		return 1;
	}
    glEnable(GL_DEBUG_OUTPUT);
#pragma endregion SDL_INIT

#pragma region CAMERA_CODE

	Camera cam;
	cam.SetPosition(glm::vec3(0, 0, 35));
	glm::mat4 proj = glm::perspective(45.0f, 800.0f / 600.0f, 1.0f, 100.0f);	//projection matrix

#pragma endregion CAMERA_CODE

#pragma region SHADERS
	//Render Shader
	unique_ptr<ShaderProgram> viewShader(new ShaderProgram());
	viewShader->initFromFiles("shaders/render.vert","shaders/render.frag");
	viewShader->addAttribute("pos");
	viewShader->addUniform("MVP");
	//viewShader->addUniform("projection");

	//Velocity compute shader
	ifstream file("shaders/compute.cs");
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

    //Position integration compute shader
    file.open("shaders/integrate.cs");
	if (!file.good())
	{
		throw std::runtime_error("Failed to open file: integrate.cs");
	}
	stringstream stream2;
	//const char *source;

	stream2 << file.rdbuf();
	file.close();
	string integration_shader_src = stream2.str();
	integrationShader = glCreateShader(GL_COMPUTE_SHADER);
	source = integration_shader_src.c_str();
	length = integration_shader_src.length();
	glShaderSource(integrationShader, 1, &source, &length);
	glCompileShader(integrationShader);
	//check_program_compile_status(computeShader);
	integrationProgram = glCreateProgram();
	glAttachShader(integrationProgram, integrationShader);
	glLinkProgram(integrationProgram);
	glGetProgramiv(integrationProgram, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(integrationProgram, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(integrationProgram, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;
    }

#pragma endregion SHADERS

#pragma region BUFFERS
    //Generate particles randomly
    vector<glm::vec3> positionData(numParticles);
    vector<glm::vec3> velocityData(numParticles);
    for(int i=0;i<positionData.size();++i)
    {
    	positionData[i] = glm::gaussRand(glm::vec3(0,0,0), glm::vec3(1, 0.2, 1));
    	velocityData[i] = glm::vec3(0);
    }
    //make buffers and upload to device
    glGenBuffers(1,&posBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3)*numParticles, positionData.data(), GL_STATIC_DRAW);

    glGenBuffers(1,&velBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3)*numParticles, velocityData.data(), GL_STATIC_DRAW);

    //bind buffers in binding table
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velBuffer);


    //Now attach buffers to render shader
    glVertexAttribPointer(viewShader->attribute("pos"), 3, GL_FLOAT, GL_FALSE, 0, 0);
#pragma endregion BUFFERS

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	//clear screen
	glClear(GL_COLOR_BUFFER_BIT);
	
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			//glClear(GL_COLOR_BUFFER_BIT);
			switch (e.type)
			{
			case SDL_QUIT:	//if X windowkey is pressed then quit
				quit = true;

			case SDL_KEYDOWN:	//if ESC is pressed then quit
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;

				case SDLK_w:
					cam.move(FORWARD);
					std::cout << "W pressed \n";
					break;

				case SDLK_s:
					cam.move(BACK);
					std::cout << "S pressed \n";
					break;

				case SDLK_a:
					cam.move(LEFT);
					std::cout << "A pressed \n";
					break;

				case SDLK_d:
					cam.move(RIGHT);
					std::cout << "D pressed \n";
					break;

				case SDLK_UP:
					cam.move(UP);
					std::cout << "PgUP pressed \n";
					break;

				case SDLK_DOWN:
					cam.move(DOWN);
					std::cout << "PgDn pressed \n";
					break;

				case SDLK_LEFT:
					cam.move(ROT_LEFT);
					break;

				case SDLK_RIGHT:
					cam.move(ROT_RIGHT);
					break;


				case SDLK_r:
					cam.Reset();
					std::cout << "R pressed \n";
					break;

				case SDLK_e:
					std::cout << "E pressed \n";
					break;

				}
				break;

			case SDL_MOUSEMOTION:
				cam.rotate();
				std::cout << "mouse moved by x=" << e.motion.xrel << " y=" << e.motion.yrel << "\n";
				break;
			}

		}
		glClear(GL_COLOR_BUFFER_BIT);
		
        update(computeProgram, integrationProgram);
        render(viewShader, cam);

		SDL_GL_SwapWindow(window);
		
	}
	//Exit
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

//renders on screen
void render(unique_ptr<ShaderProgram> &program, Camera &cam)
{
	GLfloat time = SDL_GetTicks();
	program->use();
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glEnableVertexAttribArray(program->attribute("pos"));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	cam.calcMatrices();
	view = cam.getViewMatrix();
	model = glm::rotate(glm::mat4(1), time*0.002f, glm::vec3(0, -1, 0));//	//calculate on the fly
	glm::mat4 MVP = projection*view*model;
	glUniformMatrix4fv(0, 1, false, glm::value_ptr(MVP));
	glDrawArrays(GL_POINTS, 0, numParticles);
}

//Dispatches compute
void update(GLuint computeProgram, GLuint integrationProgram)
{
	glUseProgram(computeProgram);
	float dt = 1.0f/60.0f;
	glUniform1f(0, dt);
	glDispatchCompute(numParticles/256,1,1);

	glUseProgram(integrationProgram);
	glUniform1f(0, dt);
	glDispatchCompute(numParticles/256,1,1);

	//Don't know if explicit synchronisation is necessary,
	//but doing it just to be safe.
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

}
