#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

void drawTriangle(float vertices[], ShaderProgram program);
void drawBorder(ShaderProgram program);
class Puck {
public:

	float x;
	float y;
	float width;
	float hieght;
	bool right;
	bool up;
	Puck() {
		x = 0.0f;
		y = 0.0f;
		width = .03f;
		hieght = .03f;
		right = true;
		up = true;
	}

	void Draw(ShaderProgram &program) {
		float puck[] = {
			x,y,
			x + width,y + hieght,
			x,y + hieght
		};
		float puck2[] = {
			x,y,
			x + width,y + hieght,
			x + width,y
		};
		drawTriangle(puck, program);
		drawTriangle(puck2, program);

	}

	float getX() {return x; }
	float getY() {return y; }
	void setX(float num) { x = num; }
	void setY(float num) { y = num; }
	bool goingRight() {
		return right;
	}

	bool goingUp() {
		return up;
	}

	void goUp() {
		up = true;
	}

	void goDown() {
		up = false;
	}

	void goLeft() {
		right = false;
	}

	void goRight() {
		right = true;
	}

	bool collide(float x1, float y1) {
		float bufferHieght = .4f;
		float bufferWidth = .02f;
		if ((x > x1&&x < x1 + bufferWidth) &&(y>y1&&y<y1+bufferHieght) ) {
			return true;
		}
		else {
			return false;
		}
	}

};


class Guard {
public:
	float x;
	float y;
	float width;
	float hieght;
	bool right;

	Guard() {

	}
	float getX() { return x; }
	float getY() { return y; }
	void setX(float num) { x = num; }
	void setY(float num) { y = num; }

};
int main(int argc, char *argv[])
{

	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif


	//glViewport(0, 0, 640, 360);
	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	glm::mat4 projectionMatrix = glm::mat4(1.0f);//this line creates an identitiy matrix
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-.50f, 0.0f, 0.0f));





	SDL_Event event;
	bool done = false;
	float y = 0.0;
	float lastFrameTicks = 0.0f;
	Puck p;
	bool lost = false;


	while (!done) {
		
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					y += -0.09f;
				}
				else if(event.key.keysym.scancode == SDL_SCANCODE_UP){
					y += 0.09f;
				}

			}
		}
		glClear(GL_COLOR_BUFFER_BIT);

		program.SetModelMatrix(modelMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		program.SetViewMatrix(viewMatrix);
		glUseProgram(program.programID);

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		float vertices[] = { -.20f, -.20f+y, -.18f, -.20f+y, -.20f, .20f+y};
		drawTriangle(vertices, program);
		float vertices1[] = { -.18f,-.20f+y,-.18f,.20f+y,-.20f,.20f+y};
		drawTriangle(vertices1, program);
		float vertices2[] = { 1.2f, p.getY()-.1f, 1.22f, p.getY() - .1f, 1.2f, p.getY()+.4f - .1f };
		drawTriangle(vertices2, program);
		float vertices3[] = { 1.22f, p.getY() - .1f,1.22f, p.getY() - .1f + .4f,1.2f, p.getY() + .4f - .1f };
		drawTriangle(vertices3, program);

		p.Draw(program);

		drawBorder(program);

		bool rebound1 = p.collide(-.21f, -.20f + y);
		bool rebound2 = p.collide(1.18f, p.getY() - .1f);
		if (!lost) {
			if (p.goingRight()) {
				if (rebound2) {
					p.goLeft();
				}
				if (p.getX() < 1.45) {
					p.setX(p.getX() + elapsed*.75);
				}


			}
			else {
				if (rebound1) {
					p.goRight();
				}
				if (p.getX() > -.50) {
					p.setX(p.getX() - elapsed*.75);
				}
				else {
					lost = true;
				}

			}

			if (p.goingUp()) {
				if (p.getY() < .955) {
					p.setY(p.getY() + elapsed*.75);
				}
				else {
					p.goDown();
				}
			}
			else {
				if (p.getY() > -.955) {
					p.setY(p.getY() - elapsed*.75);
				}
				else {
					p.goUp();
				}
			}


		}
		

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();

    return 0;
}



void drawTriangle(float vertices[], ShaderProgram program) {
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(program.positionAttribute);
}


void drawBorder(ShaderProgram program) {
	float x = 0.45f;
	float y = -1.0f;

	while (y < 1) {
		float borderTop[] = {
			0.0f + x,0.0f + y,
			.05f + x,0.0f + y,
			0.0f + x,.05f + y };
		drawTriangle(borderTop, program);

		float borderBottom[] = {
			0.05f + x,0.05f + y,
			0.0f + x,.05f + y,
			0.05f + x,0.0f + y };
		drawTriangle(borderBottom, program);
		y += .1f;
	}
}


