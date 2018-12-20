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
#include <time.h> 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <random>
#include <cmath>
#include <queue> 
#include <iterator>
#include <SDL_mixer.h>



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

void drawLine(float vertices[], ShaderProgram program) {
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 2);
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

class Snake {

public:
	float x;
	float y;
	float velY;
	float velX;
	float locX;
	float locY;
	float preX;
	float preY;
	glm::mat4 snakeMatrix = glm::mat4(1.0f);
	int direction = -1;
	int score = 0;
	bool alive = true;
	queue<pair<float, float>> locations;

	Snake(float x, float y) {
		this->x = x;
		this->y = y;
		this->locX = this->x;
		this->locY = this->y;
		locations.push(make_pair(x,y));
	}

	void draw(ShaderProgram &program) {
		program.SetModelMatrix(snakeMatrix);
		queue<pair<float, float>> tmp_q = locations; //copy the original queue to the temporary queue
		while (!tmp_q.empty())
		{
			pair<float,float> item = tmp_q.front();
			x = item.first;
			y = item.second;
			tmp_q.pop();
			float vertices[6] = { x, y+.006, x - 0.0955f, y + .1f, x - 0.0955f,  y + .006 };
			float vertices2[6] = { x,  y + .006, x - 0.0955f, y + .1f, x , y + .1f };
			drawTriangle(vertices, program);
			drawTriangle(vertices2, program);
		}
		
		
		
		//float vertices[6] = { 0.1f, -1.0f, 0.0f, -0.8f, -0.1f, -1.0f };


	}

	void moveRight(){
		snakeMatrix = glm::translate(snakeMatrix, glm::vec3(.10f, .00f, .00f));
		direction = 0;
	}

	void moveLeft() {
		snakeMatrix = glm::translate(snakeMatrix, glm::vec3(-.10f, .00f, .00f));
		direction = 1;
	}
	void moveDown() {
		snakeMatrix = glm::translate(snakeMatrix, glm::vec3(.00f, -.10f, .00f));
		direction = 2;
	}
	
	void moveUp() {
		snakeMatrix = glm::translate(snakeMatrix, glm::vec3(.00f, .10f, .00f));
		direction = 3;
	}

	void move() {
		switch(direction)
		{
		case -1:
			break;
		case 0:
			snakeMatrix = glm::translate(snakeMatrix, glm::vec3(.10f, .00f, .00f));
			break;
		case 1:
			snakeMatrix = glm::translate(snakeMatrix, glm::vec3(-.10f, .00f, .00f));
			break;
		case 2:
			snakeMatrix = glm::translate(snakeMatrix, glm::vec3(.00f, -.10f, .00f));
			break;
		case 3:
			snakeMatrix = glm::translate(snakeMatrix, glm::vec3(.00f, .10f, .00f));
			break;
		}
	}

	void eat(float x, float y) {
		locations.push(make_pair(x, y));
	}
	
};

class Tail {
public:
	float x;
	float y;
	float velY;
	float velX;
	float locX;
	float locY;
	glm::mat4 tailMatrix = glm::mat4(1.0f);
	queue<pair<float, float>> locations;
	Tail() {
		//locations.push(make_pair(0.0, 0.0));
	}
	void draw(ShaderProgram &program) {
		program.SetModelMatrix(tailMatrix);
		queue<pair<float, float>> tmp_q = locations; //copy the original queue to the temporary queue
		while (!tmp_q.empty())
		{
			pair<float, float> item = tmp_q.front();
			x = item.first;
			y = item.second;

			tmp_q.pop();
			float vertices[6] = { x, y + .006, x - 0.0955f, y + .1f, x - 0.0955f,  y + .006 };
			float vertices2[6] = { x,  y + .006, x - 0.0955f, y + .1f, x , y + .1f };
			drawTriangle(vertices, program);
			drawTriangle(vertices2, program);
		}
	}

	void addTail(float x, float y, int score) {
		locations.push(make_pair(x, y));
		if (locations.size() > score) {
			locations.pop();
		}
	}


};

class Apple {
public:
	float x;
	float y;
	float velY;
	float r = 0.1f;
	float velX;
	glm::mat4 AppleMatrix = glm::mat4(1.0f);
	int direction = -1;

	Apple() {
		float randX = -(rand() % 15)/10.0;
		//this->x = -((1.5) / 2) - .049;
		float randY = rand() % 18;
		randY = (randY - 9) / 10;
		this->x = randX;
		this->y = randY;
	}

	void draw(ShaderProgram &program) {
		
		program.SetModelMatrix(AppleMatrix);
		program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);
		if (this->r > 1.0f) {
			this->r = 0.0f;
		}
		r += 0.1f;
		program.SetColor(r, 0.0f, 0.0f, 1.0f);
		//float vertices[6] = { 0.1f, -1.0f, 0.0f, -0.8f, -0.1f, -1.0f };
		float vertices[6] = { x, y+.01, x - 0.09f, y + .1f, x - .09f, y+.01 };
		float vertices2[6] = { x, y+.01, x - 0.09f, y + .1f, x , y + .1f };
		drawTriangle(vertices, program);
		drawTriangle(vertices2, program);

	}

	void newPlace() {
		float randX = -(rand() % 15) / 10.0;
		float randY = rand() % 18;
		randY = (randY-9)/10;
		this->x = randX;
		this->y = randY;
	}
};

bool Eaten(Snake& player, Apple& treat) {
	if ((abs(player.locX - treat.x) - .09 < 0) && (abs(player.locY - treat.y) - .09 < 0)) {
			
			return true;
	}
	
	return false;
}

bool TailCollision(Tail& playerTail, Apple& treat) {
	queue<pair<float, float>> tmp_q = playerTail.locations; //copy the original queue to the temporary queue
	while (!tmp_q.empty())
	{
		pair<float, float> item = tmp_q.front();
		float x = item.first;
		float y = item.second;
		if ((abs(x - treat.x) - .09 < 0) && (abs(y - treat.y) - .09 < 0)) {
			return true;
		}

		tmp_q.pop();
		

	}
	return false;
}

bool headCollision(Snake& player, Tail& playerTail) {
	if (player.locX<-1.4 || player.locX>0.1 || player.locY<-1.0 ||player.locY > .9) {
		return true;
	}
	queue<pair<float, float>> tmp_q = playerTail.locations; //copy the original queue to the temporary queue
	while (!tmp_q.empty())
	{
		pair<float, float> item = tmp_q.front();
		float x = item.first;
		float y = item.second;
		if ((abs(x - player.locX) - .09 < 0) && (abs(y - player.locY) - .09 < 0)) {
			return true;
		}
		tmp_q.pop();
	}
	return false;
}

void drawScore(ShaderProgram &program, Snake player) {

}

void drawBoard(ShaderProgram &program) {
	//float vertices[6] = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
	float vertices[6] = { 0.0f, -.9f, -1.5f, 0.9f, -1.5f, -.9f };
	float vertices2[6] = { 0.0f, -.9f, -1.5f, 0.9f, 0.0f, .9f };
	
	program.SetColor(0.0f,0.0f,0.0f,1.0f);
	drawTriangle(vertices, program);
	drawTriangle(vertices2, program);
	program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	float lineVectice[6] = { 0.0f, -.9f, -1.5f, -.89f, -1.5f, -.9f };
	float lineVectice2[6] = { 0.0f, -.9f,-1.5f, -.89f, 0.0f, -.89f };
	//float line1 = 0.0f, line2 = -.9f, line3 = -1.5f, line4 = -.89f, line5 = 0.0f, line6 = -.89f;
	//draw the horizontal lines
	for (int i = 0; i < 20; i++) {
		drawTriangle(lineVectice, program);
		drawTriangle(lineVectice2, program);
		lineVectice[1] += .1;
		lineVectice[3] += .1;
		lineVectice[5] += .1;
		lineVectice2[1] += .1;
		lineVectice2[3] += .1;
		lineVectice2[5] += .1;
	}

	float lineVecticeH[6] = { -1.5f,0.9f,-1.5f,-.9f,-1.49,-.9f };
	float lineVecticeH2[6] = { -1.5f,0.91f,-1.49f,.91f,-1.49,-.9f};

	for (int i = 0; i < 16; i++) {
		drawTriangle(lineVecticeH, program);
		drawTriangle(lineVecticeH2, program);
		lineVecticeH[0] += .1;
		lineVecticeH[2] += .1;
		lineVecticeH[4] += .1;
		lineVecticeH2[0] += .1;
		lineVecticeH2[2] += .1;
		lineVecticeH2[4] += .1;
	}
	
	

}


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Snek", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 680, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    SDL_Event event;
    bool done = false;
	glViewport(0, 0, 1280, 680);


	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Chunk *someSound;
	someSound = Mix_LoadWAV("gameJump.wav");
	Mix_VolumeChunk(someSound, 20);


	Mix_Music *music;
	music = Mix_LoadMUS("hitmiss.mp3");
	//Mix_PlayMusic(music, -1);
	/*
	
	
	//program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	GLuint texture1 = LoadTexture(RESOURCE_FOLDER"emoji.png");

	
	ShaderProgram program2;
	program2.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	GLuint texture2 = LoadTexture(RESOURCE_FOLDER"emoji2.png");
	GLuint texture3 = LoadTexture(RESOURCE_FOLDER"emoji3.png");
	
	

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
	
	*/		

	ShaderProgram programTextured;
	programTextured.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	ShaderProgram program;
	program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");

	glm::mat4 projectionMatrix = glm::mat4(1.0f);//creatse the 3 trasnformation matrices
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	glm::mat4 viewMatrix = glm::mat4(1.0f);
	glm::mat4 scoreMatrix = glm::mat4(1.0f);
	glm::mat4 scoreMatrix2 = glm::mat4(1.0f);
	glm::mat4 levelMatrix = glm::mat4(1.0f);
	glm::mat4 gameMode = glm::mat4(1.0f);
	glm::mat4 gameMode2 = glm::mat4(1.0f);
	glm::mat4 preMatrix = glm::mat4(1.0f);
	preMatrix = glm::translate(modelMatrix, glm::vec3(-.2f, 0.7f, 0.0f));
	scoreMatrix = glm::translate(modelMatrix, glm::vec3(.20f, 0.8f, 0.0f));
	scoreMatrix2 = glm::translate(modelMatrix, glm::vec3(.20f, 0.7f, 0.0f));
	levelMatrix = glm::translate(modelMatrix, glm::vec3(.30f, 0.6f, 0.0f));
	gameMode = glm::translate(modelMatrix, glm::vec3(.20f, 0.4f, 0.0f));
	gameMode2 = glm::translate(modelMatrix, glm::vec3(.20f, 0.3f, 0.0f));
	projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);

	ShaderProgram bulletProgram;	
	bulletProgram.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	
	GLuint font = LoadTexture(RESOURCE_FOLDER"font1.png");
	GLuint space = LoadTexture(RESOURCE_FOLDER"sheet.png");
	Snake SnakePlayer(-((1.5) / 2) - .049, 0.0f);
	Snake SnakePlayer2(-((1.5) / 2) - .149, 0.0f);
	Tail SnakeTail;
	Tail SnakeTail2;
	Apple Treat;
	srand(time(NULL));
	time_t timer;
	struct tm y2k = { 0 };
	int seconds;

	int GameState = 0;

	y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
	y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

	time(&timer);
	int secondCounter = seconds = int(difftime(timer, mktime(&y2k)));

    while (!done) {
		string score = "Player 1 Score:";
		string score2 = "Player 2 Score:";

		switch (GameState) {


		case 0:
			while (SDL_PollEvent(&event)) {
				// dont know what it does but crashed the prgram w/o it
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_1) {
						GameState = 1;
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_2) {
						GameState = 2;
					}
				}
			}
			glClearColor(1.0f, .5f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);


			programTextured.SetModelMatrix(preMatrix);
			programTextured.SetProjectionMatrix(projectionMatrix);
			programTextured.SetViewMatrix(viewMatrix);
			glm::mat4 menuMatrix = glm::mat4(1.0f);
			glm::mat4 menuMatrix2 = glm::mat4(1.0f);
			menuMatrix = glm::translate(preMatrix, glm::vec3(-1.00f, -.50f, 0.0f));
			menuMatrix2 = glm::translate(preMatrix, glm::vec3(-1.00f, -.8f, 0.0f));

			DrawText(programTextured, font, "Snek", .1f, .00f);
			programTextured.SetModelMatrix(menuMatrix);
			DrawText(programTextured, font, "Press 1 for Single Player", .1f, .00f);

			programTextured.SetModelMatrix(menuMatrix2);
			DrawText(programTextured, font, "Press 2 for Double Player", .1f, .00f);

			SDL_GL_SwapWindow(displayWindow);
			break;


		case 1:
			time(&timer);
			seconds = int(difftime(timer, mktime(&y2k)));
			if (headCollision(SnakePlayer, SnakeTail)) {
				SnakePlayer.alive = false;
			}
			if (secondCounter != seconds) {
				//SnakePlayer.move();
				//Treat.newPlace();
				secondCounter = seconds;
			}

			if (TailCollision(SnakeTail, Treat)) {
				Treat.newPlace();
			}

			if (Eaten(SnakePlayer, Treat)) {
				SnakePlayer.score++;
				//SnakePlayer.eat(Treat.x, Treat.y);
				Mix_PlayChannel(-1, someSound, 0);
				Treat.newPlace();
			}



			//check whatever current input i'm getting
			while (SDL_PollEvent(&event)) {
				// dont know what it does but crashed the prgram w/o it
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN&&SnakePlayer.alive) {
					SnakeTail.addTail(SnakePlayer.locX, SnakePlayer.locY, SnakePlayer.score);
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
						//modelMatrix = glm::translate(modelMatrix, glm::vec3(.10f, 0.0f, 0.0f));
						//TODO LEARN CORRECT WAY TO MOVE OBJECTS INSTEAD OF USING CORDINATES
						//right is 0
						if (SnakePlayer.direction == 1 && SnakePlayer.score>0) {
							break;
						}
						else {
							SnakePlayer.moveRight();
							SnakePlayer.locX += .1;
						}



					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						//modelMatrix = glm::translate(modelMatrix, glm::vec3(.10f, 0.0f, 0.0f));
						//TODO LEARN CORRECT WAY TO MOVE OBJECTS INSTEAD OF USING CORDINATES
						//left is 1
						if (SnakePlayer.direction == 0 && SnakePlayer.score>0) {
							break;
						}
						else {
							SnakePlayer.moveLeft();
							SnakePlayer.locX += -.1;
						}

					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
						//down is 2
						if (SnakePlayer.direction == 3 && SnakePlayer.score>0) {
							break;

						}
						else {
							SnakePlayer.moveDown();
							SnakePlayer.locY -= .1;
						}

					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
						//up is 3
						if (SnakePlayer.direction == 2 && SnakePlayer.score>0) {
							break;
						}
						else {
							SnakePlayer.moveUp();
							SnakePlayer.locY += .1;
						}

					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						GameState = 3;
					}
				}
				else if (event.type == SDL_KEYDOWN && !SnakePlayer.alive) {
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						GameState = 3;
					}
				}


			}
			glClearColor(1.0f, .5f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			//


			program.SetModelMatrix(modelMatrix);
			program.SetProjectionMatrix(projectionMatrix);
			program.SetViewMatrix(viewMatrix);

			drawBoard(program);
			SnakePlayer.draw(program);
			SnakeTail.draw(program);
			Treat.draw(program);

			//DrawText(program, font, to_string(elapsed), 5, .5);
			//glUseProgram(programTextured.programID);
			//SnakePlayer.move();

			programTextured.SetModelMatrix(scoreMatrix);
			programTextured.SetProjectionMatrix(projectionMatrix);
			programTextured.SetViewMatrix(viewMatrix);
			
			score.append(to_string(SnakePlayer.score));
			DrawText(programTextured, font, score, .08f, .00f);

			//programTextured.SetModelMatrix(levelMatrix);
			//DrawText(programTextured, font, "Level: Easy", .1f, .00f);

			programTextured.SetModelMatrix(gameMode);
			if (SnakePlayer.alive) {
				DrawText(programTextured, font, "State: Alive", .08f, .00f);
			}
			else {
				DrawText(programTextured, font, "State: Dead", .08f, .00f);
			}
			//DrawText(programTextured, font,to_string(SnakePlayer.locY), .1f, .00f);
			/*
			float vertices[] = { 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glDisableVertexAttribArray(program.positionAttribute);
			*/

			SDL_GL_SwapWindow(displayWindow);
			break;


		case 2:
			
			time(&timer);
			seconds = int(difftime(timer, mktime(&y2k)));
			if (headCollision(SnakePlayer, SnakeTail)) {
				SnakePlayer.alive = false;
			}
			if (headCollision(SnakePlayer2, SnakeTail2)) {
				SnakePlayer2.alive = false;
			}

			if (headCollision(SnakePlayer, SnakeTail2)) {
				SnakePlayer.alive = false;
			}
			if (headCollision(SnakePlayer2, SnakeTail)) {
				SnakePlayer2.alive = false;
			}
			if (secondCounter != seconds) {
				//SnakePlayer.move();
				//Treat.newPlace();
				secondCounter = seconds;
			}

			if (TailCollision(SnakeTail, Treat)) {
				Treat.newPlace();
			}

			if (TailCollision(SnakeTail2, Treat)) {
				Treat.newPlace();
			}



			if (Eaten(SnakePlayer, Treat)) {
				SnakePlayer.score++;
				//SnakePlayer.eat(Treat.x, Treat.y);
				Mix_PlayChannel(-1, someSound, 0);
				Treat.newPlace();
			}

			if (Eaten(SnakePlayer2, Treat)) {
				SnakePlayer2.score++;
				//SnakePlayer.eat(Treat.x, Treat.y);
				Mix_PlayChannel(-1, someSound, 0);
				Treat.newPlace();
			}



			//check whatever current input i'm getting
			while (SDL_PollEvent(&event)) {
				// dont know what it does but crashed the prgram w/o it
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN&&SnakePlayer.alive&&SnakePlayer2.alive) {
					
					if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
						SnakeTail.addTail(SnakePlayer.locX, SnakePlayer.locY, SnakePlayer.score);
						//modelMatrix = glm::translate(modelMatrix, glm::vec3(.10f, 0.0f, 0.0f));
						//TODO LEARN CORRECT WAY TO MOVE OBJECTS INSTEAD OF USING CORDINATES
						//right is 0
						if (SnakePlayer.direction == 1 && SnakePlayer.score>0) {
							break;
						}
						else {
							SnakePlayer.moveRight();
							SnakePlayer.locX += .1;
						}



					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
						SnakeTail.addTail(SnakePlayer.locX, SnakePlayer.locY, SnakePlayer.score);
						//modelMatrix = glm::translate(modelMatrix, glm::vec3(.10f, 0.0f, 0.0f));
						//TODO LEARN CORRECT WAY TO MOVE OBJECTS INSTEAD OF USING CORDINATES
						//left is 1
						if (SnakePlayer.direction == 0 && SnakePlayer.score>0) {
							break;
						}
						else {
							SnakePlayer.moveLeft();
							SnakePlayer.locX += -.1;
						}

					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
						SnakeTail.addTail(SnakePlayer.locX, SnakePlayer.locY, SnakePlayer.score);
						//down is 2
						if (SnakePlayer.direction == 3 && SnakePlayer.score>0) {
							break;

						}
						else {
							SnakePlayer.moveDown();
							SnakePlayer.locY -= .1;
						}

					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
						SnakeTail.addTail(SnakePlayer.locX, SnakePlayer.locY, SnakePlayer.score);
						//up is 3
						if (SnakePlayer.direction == 2 && SnakePlayer.score>0) {
							break;
						}
						else {
							SnakePlayer.moveUp();
							SnakePlayer.locY += .1;
						}

					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_W) {
						SnakeTail2.addTail(SnakePlayer2.locX, SnakePlayer2.locY, SnakePlayer2.score);

						if (SnakePlayer2.direction == 2 && SnakePlayer2.score>0) {
							break;
						}
						else {
							SnakePlayer2.moveUp();
							SnakePlayer2.locY += .1;
						}
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_S) {
						SnakeTail2.addTail(SnakePlayer2.locX, SnakePlayer2.locY, SnakePlayer2.score);

						if (SnakePlayer2.direction == 3 && SnakePlayer2.score>0) {
							break;

						}
						else {
							SnakePlayer2.moveDown();
							SnakePlayer2.locY -= .1;
						}
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_A) {
						SnakeTail2.addTail(SnakePlayer2.locX, SnakePlayer2.locY, SnakePlayer2.score);

						if (SnakePlayer2.direction == 0 && SnakePlayer2.score>0) {
							break;
						}
						else {
							SnakePlayer2.moveLeft();
							SnakePlayer2.locX += -.1;
						}
					}
					else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
						SnakeTail2.addTail(SnakePlayer2.locX, SnakePlayer2.locY, SnakePlayer2.score);

						if (SnakePlayer2.direction == 1 && SnakePlayer2.score>0) {
							break;
						}
						else {
							SnakePlayer2.moveRight();
							SnakePlayer2.locX += .1;
						}
					}
				}
				else if (event.type == SDL_KEYDOWN && (SnakePlayer.alive||SnakePlayer2.alive)) {
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						GameState = 3;
					}
				}
			}
			glClearColor(1.0f, .5f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			//


			program.SetModelMatrix(modelMatrix);
			program.SetProjectionMatrix(projectionMatrix);
			program.SetViewMatrix(viewMatrix);

			drawBoard(program);
			SnakePlayer.draw(program);
			SnakeTail.draw(program);

			program.SetColor(0.0f, 1.0f, 0.0f, 1.0f);
			SnakePlayer2.draw(program);
			SnakeTail2.draw(program);
			Treat.draw(program);

			//DrawText(program, font, to_string(elapsed), 5, .5);
			//glUseProgram(programTextured.programID);
			//SnakePlayer.move();

			programTextured.SetModelMatrix(scoreMatrix);
			programTextured.SetProjectionMatrix(projectionMatrix);
			programTextured.SetViewMatrix(viewMatrix);

			programTextured.SetModelMatrix(gameMode);
			if (SnakePlayer.alive) {
				DrawText(programTextured, font, "Player 1: Alive", .08f, .00f);
			}
			else {
				DrawText(programTextured, font, "Player 1: Dead", .08f, .00f);
			}

			programTextured.SetModelMatrix(gameMode2);
			if (SnakePlayer2.alive) {
				DrawText(programTextured, font, "Player 2: Alive", .08f, .00f);
			}
			else {
				DrawText(programTextured, font, "Player 2: Dead", .08f, .00f);
			}

			programTextured.SetModelMatrix(scoreMatrix);
			score.append(to_string(SnakePlayer.score));
			DrawText(programTextured, font, score, .08f, .00f);

			programTextured.SetModelMatrix(scoreMatrix2);
			score2.append(to_string(SnakePlayer2.score));
			DrawText(programTextured, font, score2, .08f, .00f);

			/*
			

			

			
			
			
			*/
			
			//DrawText(programTextured, font,to_string(SnakePlayer.locY), .1f, .00f);
			/*
			float vertices[] = { 0.5f, -0.5f, 0.0f, 0.5f, -0.5f, -0.5f };
			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			glDisableVertexAttribArray(program.positionAttribute);
			*/

			SDL_GL_SwapWindow(displayWindow);
			break;

		case 3:
			string score = "Score:";
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
				else if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
						done = true;
					}
				}
			}
			glClearColor(1.0f, .5f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);


			programTextured.SetModelMatrix(preMatrix);
			programTextured.SetProjectionMatrix(projectionMatrix);
			programTextured.SetViewMatrix(viewMatrix);
			
			programTextured.SetModelMatrix(menuMatrix);
			DrawText(programTextured, font, "Are you sure you want to quit?", .1f, .00f);

			

			SDL_GL_SwapWindow(displayWindow);
			break;

			}


			
			
		

    }
    
    SDL_Quit();
    return 0;
}
