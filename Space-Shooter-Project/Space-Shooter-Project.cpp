#include<iostream>
#include"raylib.h"
#include<fstream>
#include<cstdlib>
using namespace std;

	//----Constans 
	const int window_height = 1000;
	const int window_width = 1000;
	const int star_count = 200;
	const int max_boss_lasers = 20;
	const int max_lasers = 50;
	const int max_enemies = 60;
	const int explosion_frames_number = 5;
	const int max_explosions = 10;
	// --- GAME STATES & TRANSITIONS --- 
	const int state_title = 0;
	const int state_instructions = 1;
	const int State_paused = 2;
	const int state_game_over = 3;
	const int state_ganme_won = 4;

	const int Transition_none = 0;
	const int Transition_at_gamestart = 1;
	const int Transition_to_instructions = 2;
	const int Transition_to_title = 3;
	const int TTransition_to_resume = 4;
	const int Transition_quit_to_title = 5;
	//--Structs
	struct star {
		float x, y;
		float speed;
		int size;
		Color color;
	};
	struct Spaceship {
		float x, y;
		float width, height;
		float speed;
	};
	struct Laser {
		float x, y;
		float speed;
		bool active;
	};
	struct BossLaser {
		float x, y;
		float speed;
		bool active;
		float width, height;
	};
	struct Enemy {
		float x, y;
		float width, height;
		float speed;
		bool active;
		int hp;
		int maxHp;
	};
	struct Boss {
		float x, y;
		float width, height;
		float speed;
		bool active;
		int hp;
		int maxHp;
		bool entering;
		int moveDir;
		float shootTimer;
	};
	struct Explosion {
		float x, y;
		bool active;
		int currentFrame;
		float frameTimer;
	};
	// --- GLOBAL VARIABLES --- 
	int score = 0, highScore = 0, level = 1, lives = 3;
	bool gameRunning = false, inTransition = false, gameWon = false, exitGameRequest = false;
	float transitionTimer = 0.0f, shootCooldown = 0.2f, shootTimer = 0.0f, frameWidth;
	bool assistActive = false;
	int enemies_to_Kill = 0, enemies_killed = 0, enemies_spawned_count = 0;
	int current_ame_state = state_title, menu_selection = 0;
	float fadeAlpha = 0.0f;
	bool isFading_out = false;
	int pendingTransition = Transition_none;
	// --- OBJECTS & ASSETS --- 
	star stars[star_count];
	Texture2D playerTexture, enemyTexture, laserTexture, explosionTexture, assistTexture,
		bossTexture, bossLaserTexture;
	Sound shootSound, explosionSound;

int main() {

}
