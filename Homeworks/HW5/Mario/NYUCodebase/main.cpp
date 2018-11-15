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
#include <stdlib.h>  
#include <time.h> 

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

float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}


void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
	float character_size = 1.1 / 16.0f;

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

class Wall {
public:
	float x;
	float y;
	float hieght = 0.2f;
	float width = 0.2f;
	glm::mat4 wallMatrix = glm::mat4(1.0f);


	Wall(float x, float y, float hieght, float width) {
		this->x = x;
		this->y = y;
		this->hieght = hieght;
		this->width = width;
	}

	void draw(ShaderProgram &program) {
		glUseProgram(program.programID);
		//program.SetModelMatrix(wallMatrix);
		//float vertices[6] = { 0.1f, -1.0f, 0.0f, -0.8f, -0.1f, -1.0f };
		//float vertices[6] = { x, y, x - 0.1f, y + .2f, x - .2f, y };
		float verticesTop[6] = { x, y, x - width, y + hieght, x - width, y };
		program.SetModelMatrix(wallMatrix);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesTop);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(program.positionAttribute);

		float verticesBot[6] = { x, y, x - width, y + hieght, x, y + hieght };
		program.SetModelMatrix(wallMatrix);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesBot);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(program.positionAttribute);
	}

	void wallCollision(float x1, float y1, bool gameState) {
		float bufferHieght = .2f;
		float bufferWidth = .2f;
		if ((x > x1&&x < x1 + bufferWidth) && (y>y1&&y<y1 + bufferHieght)) {
			gameState = true;
		}
		else {
			gameState = false;
		}
	}
	


};

class Player {
	private:
	public:
		float x;
		float y;
		float velY = .00f;
		float velX = .00f;
		float accX = .000f;
		float accY = .000f;
		bool touchingBot = true;
		bool touchingTop = false;
		bool touchingRight = false;
		bool touchingLeft = false;
		glm::mat4 playerMatrix = glm::mat4(1.0f);


		
	Player() {
			this->x = 0.1f;
			this->y = -1.0f;
			//glm::scale(playerMatrix, glm::vec3(.001f, .001f, 0.0f));
		}
	void draw(ShaderProgram &program) {

		//float vertices[6] = { 0.1f, -1.0f, 0.0f, -0.8f, -0.1f, -1.0f };
		float vertices[6] = { x, y, x - 0.1f, y + .2f, x - .2f, y };
		program.SetModelMatrix(playerMatrix);
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(program.positionAttribute);
	}

	void drawTextured(ShaderProgram &programTextured, GLuint texturePic) {
		glUseProgram(programTextured.programID);
		float vertices[6] = { x, y, x - 0.1f, y + .2f, x - .2f, y };
		programTextured.SetModelMatrix(playerMatrix);
		//set up the textured shader
		glBindTexture(GL_TEXTURE_2D, texturePic);
		//set up the text to be used
		glVertexAttribPointer(programTextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(programTextured.positionAttribute);
		//set up the shape to be drawn
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(programTextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(programTextured.texCoordAttribute);

		//set up the amount of text that is used
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//draw the two triang;es
		glDisableVertexAttribArray(programTextured.positionAttribute);
		glDisableVertexAttribArray(programTextured.texCoordAttribute);
	}

	void moveRight() {
		//playerMatrix = glm::translate(playerMatrix, glm::vec3(velX, 0.0f, 0.0f));
		accX += .000001f;
	}

	void moveLeft() {
		//playerMatrix = glm::translate(playerMatrix, glm::vec3(-velX, 0.0f, 0.0f));
		accX -= .000001f;
	}

	void moveUp() {
		touchingBot = false;
		//playerMatrix = glm::translate(playerMatrix, glm::vec3(0.0f, velY, 0.0f));
		accY = .0002f;
		
	}

	void move() {
		this->x += velX;
		this->y += velY;
		playerMatrix = glm::translate(playerMatrix, glm::vec3(velX, velY, 0.0f));
	}

	void updatePosition() {
		if (this->y < -1.0f) {
			velY = 0.0f;
			accY = 0.0f;
			this->y = -1.0f;
			touchingBot = true;
		}
		if (velX > 0) {
			accX -= .0000005;
		}
		if (velX < 0) {
			accX += .0000005;
		}
		if (!touchingBot) {
			accY -= .000001;
		}
		velX = lerp(velX, 0.0f, .1f);
		velY = lerp(velY, 0.0f, .1f);
		velX += accX;
		velY += accY;
		
	}


	/*
	
	
	*/
	bool checkCollision(vector<Wall*> blocks) {
		
		
		for (Wall* b : blocks) {
			if (collision(b)) {
				this->accX = 0;
				this->velX = -this->velX;
				return true;
			}
		}
		return false;
	}


	bool collision(Wall* b) {
		if (this->x > b->x-.3f) {
			if (this->y > b->y - .4f) {
				if (this->x > b->x - .1f) {
					if (this->y > b->y - .2f) {
						return true;
					}
				}
			}
				
	
		}
		return false;
	}

};



void checkCollision(Player& me, Wall* block, bool& GameFlag) {
	//the player has an x and y cordinate
	//the block has an x and y cordinate
	//if the bottom of the player equals the top of the block
	//stop all vertical velocity and acceleration

	/*
	if (me.x > block->x&&
		me.x < block->x + block->width/2&&
		me.y + .2f > block->y-block->hieght&&
		me.y + .2f < block->y - block->hieght+.005f) {
		me.velY = 0;
		me.accY = 0;
		me.touchingBot = true;
	}

	
	
	*/
	if ((me.y > block->y&&
		me.x > block->x)&&(true)){
		GameFlag = false;
	}
	
	
	
	
}

/*
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

*/



int main(int argc, char *argv[])
{
	srand(time(NULL));
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

	ShaderProgram program2;
	program2.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint texture2 = LoadTexture(RESOURCE_FOLDER"emoji2.png");
	GLuint texture3 = LoadTexture(RESOURCE_FOLDER"emoji3.png");
	*/		

	glViewport(0, 0, 1280, 720);
	ShaderProgram programTextured;
	programTextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");


	GLuint font = LoadTexture(RESOURCE_FOLDER"font1.png");
	GLuint space = LoadTexture(RESOURCE_FOLDER"sheet.png");
	GLuint emojiTexture = LoadTexture(RESOURCE_FOLDER"emoji.png");
	
	//glm::mat4 projectionMatrix = glm::mat4(1.0f);//creatse the 3 trasnformation matrices
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	glm::mat4 projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);


	
	glm::translate(viewMatrix, glm::vec3(1.0f, 0.0f, 0.0f));
	
	Player me;
	programTextured.SetModelMatrix(modelMatrix);
	programTextured.SetProjectionMatrix(projectionMatrix);
	programTextured.SetViewMatrix(viewMatrix);

	program.SetModelMatrix(modelMatrix);
	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	

	float lastFrameTicks = 0.0f;
	vector<Wall*> blocks;
	for (float i = 0.5; i < 30.0; i += .4) {
		float randHeight = (rand() % 100);
		randHeight /= 100;
		randHeight -= 1.0f;
		Wall* block = new Wall(i,randHeight,.2f, .2f);
		blocks.push_back(block);
	}
	bool gameState = true;
    while (!done) {
			
			float ticks = (float)SDL_GetTicks() / 1000.0f;
			float elapsed = ticks - lastFrameTicks;
			lastFrameTicks = ticks;

			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			if (keys[SDL_SCANCODE_LEFT]) {
				// go left!
				me.moveLeft();
			}
			else if (keys[SDL_SCANCODE_RIGHT]) {
				me.moveRight();
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
						//me.moveRight();
						
						
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						//modelMatrix = glm::translate(modelMatrix, glm::vec3(-.10f, 0.0f, 0.0f));
						//TODO LEARN CORRECT WAY TO MOVE OBJECTS INSTEAD OF USING CORDINATES
						//me.moveLeft();
						
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
						me.moveUp();
					}


				}

			}
			glClear(GL_COLOR_BUFFER_BIT);


			/*
			
			programTextured.SetModelMatrix(modelMatrix);
			programTextured.SetProjectionMatrix(projectionMatrix);
			programTextured.SetViewMatrix(viewMatrix);
			//set up the textured shader
			glBindTexture(GL_TEXTURE_2D, emojiTexture);
			//set up the text to be used
			float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			glVertexAttribPointer(programTextured.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(programTextured.positionAttribute);
			//set up the shape to be drawn
			float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			glVertexAttribPointer(programTextured.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
			glEnableVertexAttribArray(programTextured.texCoordAttribute);

			//set up the amount of text that is used
			glDrawArrays(GL_TRIANGLES, 0, 6);
			//draw the two triang;es
			glDisableVertexAttribArray(programTextured.positionAttribute);
			glDisableVertexAttribArray(programTextured.texCoordAttribute);
			//disable the two triangles

			*/
			

			//Wall* ptr = new Wall(0.0f, 0.0f, .2f, .2f);

			for (Wall* p : blocks) {
				checkCollision(me, p, gameState);
			}

			//checkCollision(me, ptr);
			if (gameState) {
				DrawText(programTextured, font, "Don't touch the white blocks", .105f, .00f);
				me.updatePosition();
				viewMatrix = glm::translate(viewMatrix, glm::vec3(-me.velX * 2, -me.velY, 0.0f));
				program.SetViewMatrix(viewMatrix);
				me.move();
			}
			else {
				DrawText(programTextured, font, " You Lost", .105f, .00f);
			}
			
			
			

			//me.drawTextured(programTextured, emojiTexture);
			me.draw(program);


			/*
			string debugMode;
			debugMode.append(to_string(ptr->x));
			debugMode.append(" ");
			debugMode.append(to_string(ptr->x));
			//DrawText(programTextured, font, debugMode, .105f, .00f);
			*/
			
			
			
			for (Wall* p : blocks) {
				p->draw(program);
			}
			
			//glBindTexture(GL_TEXTURE_2D, space);

			/*
			float triangle4[] = { 0.25f,-0.25f,0.0f,0.25f,-0.25f,-0.25f };
			glUseProgram(program.programID);//loads the second shader and says to use it
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, triangle4);
			glEnableVertexAttribArray(program.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glDisableVertexAttribArray(program.positionAttribute);
			*/
			


			SDL_GL_SwapWindow(displayWindow);
		

    }
    
    SDL_Quit();
    return 0;
}
