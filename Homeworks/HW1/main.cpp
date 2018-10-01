#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif


GLuint LoadTexture(const char *filePath) {
	int w, h,comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image==NULL) {
		std::cout << "Unable to load image. Make sure that path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return retTexture;
}


SDL_Window* displayWindow;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    SDL_Event event;
    bool done = false;

	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	//program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	GLuint texture1 = LoadTexture(RESOURCE_FOLDER"emoji.png");

	ShaderProgram program2;
	program2.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint texture2 = LoadTexture(RESOURCE_FOLDER"emoji2.png");
	GLuint texture3 = LoadTexture(RESOURCE_FOLDER"emoji3.png");


	ShaderProgram program3;
	program3.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	glm::mat4 projectionMatrix = glm::mat4(1.0f);//creatse the 3 trasnformation matrices
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);

	glm::mat4 projectionMatrix2 = glm::mat4(1.0f);//creatse the 3 trasnformation matrices for 2nd
	glm::mat4 modelMatrix2 = glm::mat4(1.0f);
	glm::mat4 viewMatrix2 = glm::mat4(1.0f);

	glm::mat4 projectionMatrix3 = glm::mat4(1.0f);//creatse the 3 trasnformation matrices for 3rd
	glm::mat4 modelMatrix3 = glm::mat4(1.0f);
	glm::mat4 viewMatrix3 = glm::mat4(1.0f);

	glm::mat4 projectionMatrix4 = glm::mat4(1.0f);//creatse the 3 trasnformation matrices for 3rd
	glm::mat4 modelMatrix4 = glm::mat4(1.0f);
	glm::mat4 viewMatrix4 = glm::mat4(1.0f);

	modelMatrix2 = glm::translate(modelMatrix2, glm::vec3(.60f, 0.0f, .10f));
	modelMatrix3 = glm::translate(modelMatrix3, glm::vec3(-.60f, 0.0f, .10f));
	modelMatrix4 = glm::translate(modelMatrix4, glm::vec3(0.0f, -.60f, .10f));

    while (!done) {
        while (SDL_PollEvent(&event)) {
			// dont know what it does but crashed the prgram w/o it
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

		//rotates the original object
		float angle = 1.0f*(3.14f / 180.0f);
		modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));//rotates the triangle 1D
		
		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);


		glBindTexture(GL_TEXTURE_2D, texture1);

		//float vertices[] = {0.25f,-0.25f,0.0f,0.25f,-0.25f,-0.25f};
		float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};

		
		glUseProgram(program.programID);//loads the first shader and says to use it
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		//cordinates for the texture
		float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		//loads the shape
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
		
		
		//trying to draw second object
		/*
		
		Try to draw multiple shapes with the same shaders
		
		*/

		program2.SetModelMatrix(modelMatrix2);
		program2.SetProjectionMatrix(projectionMatrix2);
		program2.SetViewMatrix(viewMatrix2);

		glBindTexture(GL_TEXTURE_2D, texture2);



		//float vertices[] = {0.25f,-0.25f,0.0f,0.25f,-0.25f,-0.25f};
		float vertices2[] = { -.25, -.25, .25, -.25, .25, .25, -.25, -.25, .25, .25, -.25, .25 };


		glUseProgram(program2.programID);//loads the first shader and says to use it
		glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(program2.positionAttribute);

		//cordinates for the texture
		float texCoords2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program2.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords2);
		glEnableVertexAttribArray(program2.texCoordAttribute);

		//loads the shape
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program2.positionAttribute);
		glDisableVertexAttribArray(program2.texCoordAttribute);

		/*
		glUseProgram(program2.programID);//loads the second shader and says to use it
		glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, triangle2);
		glEnableVertexAttribArray(program2.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(program2.positionAttribute);
		
		*/

		
		//trying to draw 3rd object


		program2.SetModelMatrix(modelMatrix3);
		program2.SetProjectionMatrix(projectionMatrix3);
		program2.SetViewMatrix(viewMatrix3);

		glBindTexture(GL_TEXTURE_2D, texture3);



		float vertices3[] = { -.25, -.25, .25, -.25, .25, .25, -.25, -.25, .25, .25, -.25, .25 };


		glUseProgram(program2.programID);//loads the first shader and says to use it
		glVertexAttribPointer(program2.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
		glEnableVertexAttribArray(program2.positionAttribute);

		//cordinates for the texture
		float texCoords3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program2.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords3);
		glEnableVertexAttribArray(program2.texCoordAttribute);

		//loads the shape
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program2.positionAttribute);
		glDisableVertexAttribArray(program2.texCoordAttribute);


		//trying to draw final 4th object

		program3.SetModelMatrix(modelMatrix4);
		program3.SetProjectionMatrix(projectionMatrix4);
		program3.SetViewMatrix(viewMatrix4);

		float triangle4[] = { 0.25f,-0.25f,0.0f,0.25f,-0.25f,-0.25f };
		glUseProgram(program3.programID);//loads the second shader and says to use it
		glVertexAttribPointer(program3.positionAttribute, 2, GL_FLOAT, false, 0, triangle4);
		glEnableVertexAttribArray(program3.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(program3.positionAttribute);




		//displays final window
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
