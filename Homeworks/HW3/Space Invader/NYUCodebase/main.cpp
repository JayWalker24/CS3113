#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace std;


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




void drawTriangle(float vertices[], ShaderProgram program) {
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(program.positionAttribute);
}

class Player;
class Bullet;
class Enemy;
void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
	float character_size = 1.0 / 16.0f;

	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (size_t i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];

		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});

		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + character_size,
			texture_x + character_size, texture_y,
			texture_x + character_size, texture_y + character_size,
			texture_x + character_size, texture_y,
			texture_x, texture_y + character_size,
		});
	}


	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glUseProgram(program.programID);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, (int)vertexData.size() / 2);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

class SheetSprite {
public:
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) {
		this->textureID = textureID;
		this->u = u;
		this->v = v;
		this->width = width;
		this->height = height;
		this->size = size;
	}

	SheetSprite(){}
	void SheetSprite::Draw(ShaderProgram &program, float vertices[]) {
		glBindTexture(GL_TEXTURE_2D, textureID);
		GLfloat texCoords[] = {
			u, v + height,
			u + width, v,
			u, v,
			u + width, v,
			u, v + height,
			u + width, v + height
		};
		float aspect = width / height;
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);
	}
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};


class Bullet {
public:
	float x;
	float y;
	float velY;
	float velX;
	float width = 0.025f;
	float height = 0.05f;
	bool alive = true;

	Bullet(float x, float y) {
		this->x = x;
		this->y = y;

	}

	void draw(ShaderProgram &program) {
		float puck[] = {
			x,y,
			x + width,y + height,
			x,y + height
		};
		float puck2[] = {
			x,y,
			x + width,y + height,
			x + width,y
		};
		drawTriangle(puck, program);
		drawTriangle(puck2, program);
	}
	
	void moveUp() {
		y += .0125f;
	}


};


class Player {

public:
	float x;
	float y;
	float velY;
	float velX;
	vector<Bullet*> bullets;
	

	Player() {
		this->x = 0.1f;
		this->y = -1.0f;
	}

	void draw(ShaderProgram &program) {
		for (Bullet* p : bullets) {
			if (p->alive) {
				p->draw(program);
			}
		}
		/*
		make vertices entirely relative to a single x and y cordinate which I can change

	
		*/

		//float vertices[6] = { 0.1f, -1.0f, 0.0f, -0.8f, -0.1f, -1.0f };
		float vertices[6] = { x, y, x-0.1f, y+.2f, x-.2f, y};
		drawTriangle(vertices, program);
	}

	void goRight() {
		x += .05f;
	}

	void goLeft() {
		x -= .05f;
	}

	void Shoot() {
		Bullet* pow = new Bullet(this->x-.1125f,-.80f);
		bullets.push_back(pow);
	}

	void update() {
		for (Bullet* b : bullets) {
			if (b) {
				b->moveUp();
			}
		}
	}




};


class Enemy {
public:
	float x;
	float y;
	float width = .10f;
	float height = .10f;
	bool alive = true;
	bool moveRight = true;
	SheetSprite prac;

	Enemy(float x, float y, SheetSprite prac) {
		this->x = x;
		this->y = y;
		this->prac = prac;
	}

	void draw(ShaderProgram &program) {
		/*
		float puck[] = {
			x,y,
			x + width,y + height,
			x,y + height
		};
		float puck2[] = {
			x,y,
			x + width,y + height,
			x + width,y
		};
		drawTriangle(puck, program);
		drawTriangle(puck2, program);
		*/

		float vertices[] = { x,y,
			x + width,y + height,
			x,y + height,
			x,y,
			x + width,y + height,
			x + width,y
		};

		prac.Draw(program, vertices);
		
	}

	bool collision(Player me) {
		for (Bullet* b : me.bullets) {
			if (b->alive && this->alive) {
				if (b->x > this->x
					&&b->x< this->x+this->width
					&&b->y>this->y) {

					this->alive = false;
					b->alive = false;
					return true;
				}

			}

		}
		return false;
	}

	void move() {
		if (moveRight) {
			if (x > 1.0f) {
				y -= .05f;
				moveRight = false;
			}
			else {
				x += .05f;
			}
		}
		else {
			if (x < -1.0f) {
				y -= .05f;
				moveRight = true;
			}
			else {
				x -= .001f;
			}
		}
		
	}



};


void move(vector<Enemy*> guys) {
	bool reachedRightEdge = false;
	bool reachedLeftEdge = false;
	for (Enemy* p : guys) {
		if (p->x > 1.0f&&p->alive) {
			reachedRightEdge = true;
		}
		else if(p->x < -1.0f&&p->alive){
			reachedLeftEdge = true;
		}
	}
	for (Enemy* p : guys) {
		if (p->moveRight&&p->alive) {
			p->x += .001f;
		}
		else {
			p->x -= .001f;
		}
	}

	if (reachedRightEdge) {
		for (Enemy* p : guys) {
			if (p->alive) {
				p->moveRight = false;
				p->y -= .01f;
			}
			
		}
	}
	else if (reachedLeftEdge) {
		for (Enemy* p : guys) {
			if (p->alive) {
				p->moveRight = true;
				p->y -= .01f;
			}
		}
	}

}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 680, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    SDL_Event event;
    bool done = false;


	/*
	
	
	//program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	GLuint texture1 = LoadTexture(RESOURCE_FOLDER"emoji.png");

	glViewport(0, 0, 640, 360);
	ShaderProgram program2;
	program2.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint texture2 = LoadTexture(RESOURCE_FOLDER"emoji2.png");
	GLuint texture3 = LoadTexture(RESOURCE_FOLDER"emoji3.png");
	*/		

	ShaderProgram programTextured;
	programTextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	glm::mat4 projectionMatrix = glm::mat4(1.0f);//creatse the 3 trasnformation matrices
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);



	ShaderProgram bulletProgram;	
	bulletProgram.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	GLuint font = LoadTexture(RESOURCE_FOLDER"font1.png");
	GLuint space = LoadTexture(RESOURCE_FOLDER"sheet.png");
	

	glm::mat4  modelMatrix2 = glm::translate(modelMatrix, glm::vec3(-.80f, 0.2f, .10f));
	
	Player me;
	vector<Enemy*> enemies;
	SheetSprite prac = SheetSprite(space, 425.0f / 1024.0f, 468.0f / 1024.0f, 93.0f / 1024.0f, 84.0f /
		1024.0f, 0.2f);
	float y = .85f;
	for (int j = 0; j < 5; j++) {
		float x = -.95f;
		for (int k = 0; k < 5; k++) {
			Enemy* billy = new Enemy(x, y,prac);
			enemies.push_back(billy);
			x += 0.3f;
		}
		
		y -= .2f;
	}
	
	bool MainMenu = true;
    while (!done) {

		if (MainMenu) {
			while (SDL_PollEvent(&event)) {
				// dont know what it does but crashed the prgram w/o it
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
						//modelMatrix = glm::translate(modelMatrix, glm::vec3(.10f, 0.0f, 0.0f));
						//TODO LEARN CORRECT WAY TO MOVE OBJECTS INSTEAD OF USING CORDINATES
						MainMenu = false;
					}
				}

			}

			glClear(GL_COLOR_BUFFER_BIT);

			programTextured.SetModelMatrix(modelMatrix2);
			programTextured.SetProjectionMatrix(projectionMatrix);
			programTextured.SetViewMatrix(viewMatrix);
			

			DrawText(programTextured, font, "Press Enter", .125f, .00f);

			SDL_GL_SwapWindow(displayWindow);
		}
		else {
			me.update();
			move(enemies);
			for (Enemy* e : enemies) {
				if (e) {
					e->collision(me);
				}
			}

			
			while (SDL_PollEvent(&event)) {
				// dont know what it does but crashed the prgram w/o it
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
						//modelMatrix = glm::translate(modelMatrix, glm::vec3(.10f, 0.0f, 0.0f));
						//TODO LEARN CORRECT WAY TO MOVE OBJECTS INSTEAD OF USING CORDINATES
						if (me.x < 1.0f) {
							me.goRight();
						}
						
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						//modelMatrix = glm::translate(modelMatrix, glm::vec3(.10f, 0.0f, 0.0f));
						//TODO LEARN CORRECT WAY TO MOVE OBJECTS INSTEAD OF USING CORDINATES
						if (me.x-.2 > -1) {
							me.goLeft();
						}
						
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						me.Shoot();

					}


				}

			}
			glClear(GL_COLOR_BUFFER_BIT);

			program.SetModelMatrix(modelMatrix);
			program.SetProjectionMatrix(projectionMatrix);
			program.SetViewMatrix(viewMatrix);

			me.draw(program);


			programTextured.SetModelMatrix(modelMatrix);
			programTextured.SetProjectionMatrix(projectionMatrix);
			programTextured.SetViewMatrix(viewMatrix);

			for (Enemy* e : enemies) {
				if (e->alive) {
					e->draw(programTextured);
				}
			}
			


			SDL_GL_SwapWindow(displayWindow);
		}

    }
    
    SDL_Quit();
    return 0;
}
