#include<iostream>
#include"raylib.h"
#include<fstream>
#include<cstdlib>
using namespace std;

//----Constants 
const int window_height = 900;
const int window_width = 1000;
const int star_count = 200;
const int max_boss_lasers = 20;
const int max_lasers = 60;
const int max_enemies = 50;
const int explosion_frames_number = 5;
const int max_explosions = 20;

// --- GAME STATES & TRANSITIONS --- 
const int state_title = 0;
const int state_instructions = 1;
const int State_paused = 2;
const int state_game_over = 3;
const int state_game_won = 4;

const int Transition_none = 0;
const int Transition_at_gamestart = 1;
const int Transition_to_instructions = 2;
const int Transition_to_title = 3;
const int Transition_to_resume = 4;
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

// Variables needed for Wave System
int enemies_to_Kill = 0, enemies_killed = 0, enemies_spawned_count = 0;

int current_game_state = state_title, menu_selection = 0;
float fadeAlpha = 0.0f;
bool isFading_out = false;
int pendingTransition = Transition_none;


// --- OBJECTS & ASSETS --- 
star stars[star_count];
Texture2D playerTexture, enemyTexture, laserTexture, explosionTexture, assistTexture,
bossTexture, bossLaserTexture;
Sound shootSound, explosionSound;


// --- PROTOTYPE DECLERATION --- 
void loadAssets(void);
void unloadAllAssets(void);
void loadHighScore(void);
void saveHighScore(void);
void resetGame(Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser
    lasers[], BossLaser bossLasers[]);
void startLevel(int lvl);
void updateStars(void);

void handlePlayerInput(Spaceship& ship, Spaceship& assistShip, Laser lasers[]);
void updateEnemyLogic(Spaceship& ship, Enemy enemies[]);
void updateGameLogic(float dt, Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser lasers[], BossLaser bossLasers[], Explosion explosions[]);
void handleTransitions(Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser lasers[], BossLaser bossLasers[]);

void handleLevelTransition(Spaceship& ship, Boss& bigBoss, Enemy enemies[], Laser lasers[], BossLaser bossLasers[], Explosion explosions[]);
void updateExplosions(float dt, Explosion explosions[]);
void DrawGame(Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser
    lasers[], BossLaser bossLasers[], Explosion explosions[]);

void DrawTransition(Spaceship& ship);
void DrawGameplay(Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser lasers[], BossLaser bossLasers[], Explosion explosions[]);
void DrawMenu(void);


// Helper function to spawn explosion
void SpawnExplosion(float x, float y, Explosion explosions[]) {
    for (int k = 0; k < max_explosions; k++) {
        if (!explosions[k].active) {
            explosions[k].active = true;
            explosions[k].x = x;
            explosions[k].y = y;
            explosions[k].currentFrame = 0;
            explosions[k].frameTimer = 0.0f;
            break;
        }
    }
}


int main() {
    InitWindow(window_width, window_height, "C9----Space Shooter (Group Project)");
    SetTargetFPS(60);
    InitAudioDevice();
    loadAssets();
    loadHighScore();

    // Initialization Logic 
    for (int i = 0; i < star_count; i++) {
        stars[i].x = (float)GetRandomValue(0, window_width);
        stars[i].y = (float)GetRandomValue(0, window_height);
        stars[i].speed = (float)GetRandomValue(50, 200) / 100.0f;
        stars[i].size = GetRandomValue(1, 3);
        stars[i].color = WHITE;
    }

    Spaceship ship = { window_width / 2 - 30.0f, window_height - 100.0f, 60.0f, 80.0f, 9.0f };
    Spaceship assistShip = { -100.0f, 0.0f, 60.0f, 80.0f, 9.0f };

    Boss bigBoss;
    bigBoss.width = (float)bossTexture.width > 0 ? (float)bossTexture.width : 100;
    bigBoss.height = (float)bossTexture.height > 0 ? (float)bossTexture.height : 100;
    bigBoss.x = window_width / 2 - bigBoss.width / 2;
    bigBoss.y = -300;
    bigBoss.speed = 3.0f;
    bigBoss.maxHp = 3500;
    bigBoss.hp = bigBoss.maxHp;
    bigBoss.active = false;
    bigBoss.entering = false;
    bigBoss.moveDir = 1;
    bigBoss.shootTimer = 0.0f;

    BossLaser bossLasers[max_boss_lasers];
    for (int i = 0; i < max_boss_lasers; i++) {
        bossLasers[i].active = false; bossLasers[i].speed = 7.0f;
        bossLasers[i].width = 20; bossLasers[i].height = 50;
    }
    Laser lasers[max_lasers];

    for (int i = 0; i < max_lasers; i++) {
        lasers[i].active = false; lasers[i].speed = 12.0f;
    }


    Enemy enemies[max_enemies];
    for (int i = 0; i < max_enemies; i++) {
        enemies[i].active = false;
        enemies[i].width = 50;
        enemies[i].height = 50;
        enemies[i].speed = 1.0f;
        enemies[i].hp = 1;
        enemies[i].maxHp = 1;
    }

    Explosion explosions[max_explosions];
    for (int i = 0; i < max_explosions; i++) {
        explosions[i].active = false;
        explosions[i].currentFrame = 0;
        explosions[i].frameTimer = 0.0f;
    }
    // -------------------------------------------
        // MAIN GAME LOOP 
    while (!WindowShouldClose() && !exitGameRequest) {
        float dt = GetFrameTime();

        updateStars();
        if (score > highScore) highScore = score;
        handleTransitions(ship, assistShip, bigBoss, enemies, lasers, bossLasers);

        if (pendingTransition == Transition_none) {
            if (!gameRunning || current_game_state == State_paused || current_game_state ==
                state_game_over || current_game_state == state_game_won) {
                handleMenuInput();
            }
           
            if (gameRunning && current_game_state != State_paused) {
                if (inTransition) {
                    
                    handleLevelTransition(ship, bigBoss, enemies, lasers, bossLasers, explosions);
                }
                else {
                    handlePlayerInput(ship, assistShip, lasers);
                    updateGameLogic(dt, ship, assistShip, bigBoss, enemies, lasers, bossLasers, explosions);
                }
            }
        }
        DrawGame(ship, assistShip, bigBoss, enemies, lasers, bossLasers, explosions);
        if (!gameRunning && score > 0) saveHighScore();
    }

    unloadAllAssets();
    CloseAudioDevice();
    CloseWindow();
    return 0;
} 


void loadAssets(void) {
    playerTexture = LoadTexture("player.png");
    enemyTexture = LoadTexture("enemy.png");
    laserTexture = LoadTexture("laser.png");
    explosionTexture = LoadTexture("explosion.png");
    assistTexture = LoadTexture("assist.png");
    bossTexture = LoadTexture("boss.png");
    bossLaserTexture = LoadTexture("boss_laser.png");
    shootSound = LoadSound("shoot.mp3");
    explosionSound = LoadSound("explosion2.mp3");
    if (explosionTexture.width > 0) {
        frameWidth = (float)(explosionTexture.width / explosion_frames_number);
    }
    else {
        frameWidth = 20.0f;
    }
}


void unloadAllAssets(void) {
    UnloadTexture(playerTexture);
    UnloadTexture(enemyTexture);
    UnloadTexture(laserTexture);
    UnloadTexture(explosionTexture);
    UnloadTexture(assistTexture);
    UnloadTexture(bossTexture);
    UnloadTexture(bossLaserTexture);
    UnloadSound(shootSound);
    UnloadSound(explosionSound);
}

void loadHighScore(void) {
    ifstream infile("savegame.dat");
    if (infile.is_open()) { infile >> highScore; infile.close(); }
}

void saveHighScore(void) {
    ofstream outfile("savegame.dat");
    if (outfile.is_open()) { outfile << highScore; outfile.close(); }
}

void startLevel(int lvl) {
    level = lvl;
    enemies_to_Kill = 5 + (level * 5);
    enemies_killed = 0;
    enemies_spawned_count = 0;
}

void resetGame(Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser
    lasers[], BossLaser bossLasers[]) {
    gameRunning = true; gameWon = false; score = 0; lives = 3;
    startLevel(1);
    inTransition = false; assistActive = false; shootTimer = 0.0f; transitionTimer = 0.0f;
    bigBoss.active = false; bigBoss.hp = bigBoss.maxHp; bigBoss.y = -300;
    for (int i = 0; i < max_enemies; i++) enemies[i].active = false;
    for (int i = 0; i < max_lasers; i++) lasers[i].active = false;
    for (int i = 0; i < max_boss_lasers; i++) bossLasers[i].active = false;
    PlaySound(shootSound);

}


void updateStars(void) {
    for (int i = 0; i < star_count; i++) {
        stars[i].y += stars[i].speed;
        if (stars[i].y > window_height) {
            stars[i].y = 0; stars[i].x = (float)GetRandomValue(0, window_width);
        }
        if (GetRandomValue(0, 20) == 0) {
            int brightness = GetRandomValue(150, 255);
            stars[i].color = Color{ (unsigned char)brightness, (unsigned char)brightness, (unsigned
            char)brightness, 255 };
        }
    }
}


void handlePlayerInput(Spaceship& ship, Spaceship& assistShip, Laser lasers[]) {
    float dt = GetFrameTime();

    // Movement Logic
    if (IsKeyDown(KEY_LEFT)) ship.x -= ship.speed;
    if (IsKeyDown(KEY_RIGHT)) ship.x += ship.speed;

    // Boundary Checks (using our new window_width)
    if (ship.x < 0) ship.x = 0;
    if (ship.x > window_width - ship.width) ship.x = window_width - ship.width;

    // Assistant Ship Logic
    if (IsKeyPressed(KEY_H)) assistActive = !assistActive;

    if (assistActive) {
        assistShip.x = ship.x + 80;
        assistShip.y = ship.y;

        // Keep assist ship within screen bounds
        if (assistShip.x > window_width - assistShip.width) {
            assistShip.x = window_width - assistShip.width;
            ship.x = assistShip.x - 80;
        }
    }

    // Shooting Logic
    shootTimer -= dt;
    if (IsKeyDown(KEY_SPACE) && shootTimer <= 0.0f) {
        bool fired = false;

        // Player Laser
        for (int i = 0; i < max_lasers; i++) {
            if (!lasers[i].active) {
                lasers[i].active = true;
                lasers[i].x = ship.x + ship.width / 2 - 10;
                lasers[i].y = ship.y;
                fired = true;
                break;
            }
        }

        // Assist Laser
        if (assistActive) {
            for (int i = 0; i < max_lasers; i++) {
                if (!lasers[i].active) {
                    lasers[i].active = true;
                    lasers[i].x = assistShip.x + assistShip.width / 2 - 10;
                    lasers[i].y = assistShip.y;
                    fired = true;
                    break;
                }
            }
        }

        if (fired) {
            shootTimer = shootCooldown;
            PlaySound(shootSound);
        }
    }
}

void handlePlayerInput(Spaceship& ship, Spaceship& assistShip, Laser lasers[]) {
    float dt = GetFrameTime();

    // Movement Logic
    if (IsKeyDown(KEY_LEFT)) ship.x -= ship.speed;
    if (IsKeyDown(KEY_RIGHT)) ship.x += ship.speed;

    // Boundary Checks (using our new window_width)
    if (ship.x < 0) ship.x = 0;
    if (ship.x > window_width - ship.width) ship.x = window_width - ship.width;

    // Assistant Ship Logic
    if (IsKeyPressed(KEY_H)) assistActive = !assistActive;

    if (assistActive) {
        assistShip.x = ship.x + 80;
        assistShip.y = ship.y;

        // Keep assist ship within screen bounds
        if (assistShip.x > window_width - assistShip.width) {
            assistShip.x = window_width - assistShip.width;
            ship.x = assistShip.x - 80;
        }
    }

    // Shooting Logic
    shootTimer -= dt;
    if (IsKeyDown(KEY_SPACE) && shootTimer <= 0.0f) {
        bool fired = false;

        // Player Laser
        for (int i = 0; i < max_lasers; i++) {
            if (!lasers[i].active) {
                lasers[i].active = true;
                lasers[i].x = ship.x + ship.width / 2 - 10;
                lasers[i].y = ship.y;
                fired = true;
                break;
            }
        }

        // Assist Laser
        if (assistActive) {
            for (int i = 0; i < max_lasers; i++) {
                if (!lasers[i].active) {
                    lasers[i].active = true;
                    lasers[i].x = assistShip.x + assistShip.width / 2 - 10;
                    lasers[i].y = assistShip.y;
                    fired = true;
                    break;
                }
            }
        }

        if (fired) {
            shootTimer = shootCooldown;
            PlaySound(shootSound);
        }
    }
}


void handleTransitions(Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser lasers[], BossLaser bossLasers[]) {
    if (pendingTransition == Transition_none) return;

    float fadeSpeed = 0.05f;

    if (isFading_out) {
        fadeAlpha += fadeSpeed;
        if (fadeAlpha >= 1.0f) {
            fadeAlpha = 1.0f;
            isFading_out = false;

            switch (pendingTransition) {
            case Transition_at_gamestart:

                resetGame(ship, assistShip, bigBoss, enemies, lasers, bossLasers);


                current_game_state = state_title;
                gameRunning = true;
                break;

            case Transition_to_instructions:
                current_game_state = state_instructions;
                break;

            case Transition_to_title:
                current_game_state = state_title;
                menu_selection = 0;
                gameRunning = false;
                break;

                
            case Transition_to_resume:
                break;

            case Transition_quit_to_title:
                gameRunning = false;
                current_game_state = state_title;
                menu_selection = 0;
                break;
            }
        }
    }
    else {
        fadeAlpha -= fadeSpeed;
        if (fadeAlpha <= 0.0f) {
            fadeAlpha = 0.0f;
            pendingTransition = Transition_none;
        }
    }
}

void updateGameLogic(float dt, Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser lasers[], BossLaser bossLasers[], Explosion explosions[]) {
    // Laser Movement
    for (int i = 0; i < max_lasers; i++) {
        if (lasers[i].active) {
            lasers[i].y -= lasers[i].speed;
            if (lasers[i].y < 0) lasers[i].active = false;
        }
    }

    //  Boss Logic
    if (level == 11 && bigBoss.active) {
        if (bigBoss.entering) {
            bigBoss.y += 1.0f;
            if (bigBoss.y >= 50) bigBoss.entering = false;
        }
        else {
            bigBoss.x += bigBoss.speed * bigBoss.moveDir;

            // Boundary checks
            if (bigBoss.x <= 0) bigBoss.moveDir = 1;
            if (bigBoss.x + bigBoss.width >= window_width) bigBoss.moveDir = -1;

            bigBoss.shootTimer -= dt;
            if (bigBoss.shootTimer <= 0.0f) {
                for (int i = 0; i < max_boss_lasers; i++) {
                    if (!bossLasers[i].active) {
                        bossLasers[i].active = true;
                        bossLasers[i].x = bigBoss.x + bigBoss.width / 2 - 10;
                        bossLasers[i].y = bigBoss.y + bigBoss.height;
                        break;
                    }
                }
                bigBoss.shootTimer = 0.8f;
            }
        }

        // BOSS COLLISIONS 

        Rectangle playerRect = { ship.x, ship.y, ship.width, ship.height };
        Rectangle bossRect = { bigBoss.x, bigBoss.y, bigBoss.width, bigBoss.height };

        //  Boss Body vs Player
        if (CheckCollisionRecs(playerRect, bossRect)) {
            lives--;
            PlaySound(explosionSound);
            ship.x = window_width / 2 - ship.width / 2; // Reset player position
            ship.y = window_height - 100;
            if (lives <= 0) {
                current_game_state = state_game_over;
                gameRunning = false;
            }
        }

        //  Boss Lasers vs Player
        for (int i = 0; i < max_boss_lasers; i++) {
            if (bossLasers[i].active) {
                // Moving Boss Lasers
                bossLasers[i].y += bossLasers[i].speed;
                if (bossLasers[i].y > window_height) bossLasers[i].active = false;

                // Check Collision
                Rectangle laserRect = { bossLasers[i].x, bossLasers[i].y, bossLasers[i].width, bossLasers[i].height };
                if (CheckCollisionRecs(playerRect, laserRect)) {
                    bossLasers[i].active = false;
                    lives--;
                    PlaySound(explosionSound);
                    if (lives <= 0) {
                        current_game_state = state_game_over;
                        gameRunning = false;
                    }
                }
            }
        }
    }

    updateEnemyLogic(ship, enemies);
    updateExplosions(dt, explosions);

    //  COLLISION LOGIC (Player Lasers vs Enemies/Boss)

    Rectangle playerRect = { ship.x, ship.y, ship.width, ship.height }; // Re-define for scope safety

    for (int i = 0; i < max_lasers; i++) {
        if (!lasers[i].active) continue;

        Rectangle laserRect = { lasers[i].x, lasers[i].y, 20, 60 }; // Hardcoded size based on your draw logic

        // Laser vs Boss
        if (level == 11 && bigBoss.active) {
            Rectangle bossRect = { bigBoss.x, bigBoss.y, bigBoss.width, bigBoss.height };
            if (CheckCollisionRecs(laserRect, bossRect)) {
                lasers[i].active = false;
                bigBoss.hp -= 10; // Damage the boss

                // --- BOSS HIT ANIMATION (KEPT) ---
                SpawnExplosion(lasers[i].x - 15, lasers[i].y, explosions);
                // ------------------------------------

                if (bigBoss.hp <= 0) {
                    bigBoss.active = false;
                    score += 1000;
                    current_game_state = state_game_won; // VICTORY!
                    gameRunning = false;
                }
                continue;
            }
        }

        //  Laser vs Enemies
        for (int j = 0; j < max_enemies; j++) {
            if (enemies[j].active) {
                Rectangle enemyRect = { enemies[j].x, enemies[j].y, enemies[j].width, enemies[j].height };
                if (CheckCollisionRecs(laserRect, enemyRect)) {
                    // Collision Happened
                    lasers[i].active = false;
                    enemies[j].active = false;
                    enemies_killed++;

                    // --- SCORE IS 10 (KEPT) ---
                    score += 10;
                    // --------------------------------

                    PlaySound(explosionSound);

                    // Spawn Explosion
                    SpawnExplosion(enemies[j].x, enemies[j].y, explosions);
                    break;
                }
            }
        }
    }

    //  Enemy vs Player Body Collision
    for (int j = 0; j < max_enemies; j++) {
        if (enemies[j].active) {
            Rectangle enemyRect = { enemies[j].x, enemies[j].y, enemies[j].width, enemies[j].height };
            if (CheckCollisionRecs(playerRect, enemyRect)) {
                enemies[j].active = false;
                lives--;
                PlaySound(explosionSound);
                // Trigger explosion on player
                SpawnExplosion(ship.x, ship.y, explosions);

                if (lives <= 0) {
                    current_game_state = state_game_over;
                    gameRunning = false;
                }
            }
        }
    }

    //  Level Progression
    if (enemies_killed >= enemies_to_Kill && level != 11) {
        if (!inTransition) {
            inTransition = true;
            transitionTimer = 3.0f;
            level++;
            startLevel(level);

            // If we just hit level 11, setup boss
            if (level == 11) {
                bigBoss.active = true;
                bigBoss.hp = bigBoss.maxHp;
                bigBoss.entering = true;
                bigBoss.y = -300;
            }
        }
    }
}



void updateEnemyLogic(Spaceship& ship, Enemy enemies[]) {
    
    int max_enemies_on_screen = (level == 11) ? 0 : (5 + level * 2);
    if (max_enemies_on_screen > max_enemies) max_enemies_on_screen = max_enemies;

    // Spawning Logic
    if (enemies_spawned_count < enemies_to_Kill) {
        for (int i = 0; i < max_enemies_on_screen; i++) {
            if (!enemies[i].active) {
                // Difficulty increases with level
                if (GetRandomValue(0, 100) < (2 + level)) {
                    enemies[i].active = true;
                    enemies[i].width = 50;
                    enemies[i].height = 50;

                    enemies[i].x = (float)GetRandomValue(0, window_width - 50);
                    enemies[i].y = -100.0f;

                    // Speed increases slightly every level
                    enemies[i].speed = 1.0f + ((level - 1) * 0.1f);
               
                    enemies_spawned_count++;
                    if (enemies_spawned_count >= enemies_to_Kill) break;
                }
            }
        }
    }

    // Movement & Collision Logic
    for (int i = 0; i < max_enemies; i++) {
        if (enemies[i].active) {
            enemies[i].y += enemies[i].speed;
            Rectangle enemyRect = { enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height };
            Rectangle playerRect = { ship.x, ship.y, ship.width, ship.height };

            // Player Collision
            if (CheckCollisionRecs(playerRect, enemyRect)) {
                enemies[i].active = false;
                lives--;
                enemies_spawned_count--; // Respawn replacement
                PlaySound(explosionSound);
                if (lives <= 0) {
                    current_game_state = state_game_over;
                    gameRunning = false;
                }
            }

            // Boundary Check (Respawn if missed)
            if (enemies[i].y > window_height) {
                enemies[i].y = -enemies[i].height;
                enemies[i].x = (float)GetRandomValue(0, window_width - 50);
                // Reset HP based on level
                if (level <= 6) enemies[i].hp = 1;
                else enemies[i].hp = 2;
            }
        }
    }
}

void handleLevelTransition(Spaceship& ship, Boss& bigBoss, Enemy enemies[], Laser lasers[], BossLaser bossLasers[], Explosion explosions[]) {
    float dt = GetFrameTime();
    transitionTimer -= dt;

    // Calculate center target based on new window_width
    float targetX = window_width / 2 - ship.width / 2;

    // Smoothly move ship to center
    if (ship.x < targetX) ship.x += 2.0f;
    if (ship.x > targetX) ship.x -= 2.0f;

    
    if (transitionTimer <= 0.0f) {
        inTransition = false;

        
        for (int i = 0; i < max_lasers; i++) lasers[i].active = false;

       
        for (int i = 0; i < max_boss_lasers; i++) bossLasers[i].active = false;

        
        for (int i = 0; i < max_explosions; i++) {
            explosions[i].active = false;
            explosions[i].currentFrame = 0; 
        }
      

        shootTimer = 0.5f;

        // Activate Boss if level 11
        if (level == 11) {
            bigBoss.active = true;
            bigBoss.entering = true;
            bigBoss.y = -bigBoss.height;
            for (int i = 0; i < max_enemies; i++) enemies[i].active = false;
        }
    }
}


void updateExplosions(float dt, Explosion explosions[]) {
    for (int i = 0; i < max_explosions; i++) {
        if (explosions[i].active) {
            explosions[i].frameTimer += dt;

            if (explosions[i].frameTimer >= 0.05f) {
                explosions[i].frameTimer = 0.0f;
                explosions[i].currentFrame++;

                if (explosions[i].currentFrame >= explosion_frames_number) {
                    explosions[i].active = false;
                    explosions[i].currentFrame = 0;
                }
            }
        }
    }
}






void DrawGame(Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser
    lasers[], BossLaser bossLasers[], Explosion explosions[]) {
    BeginDrawing();
    ClearBackground(BLACK);
    for (int i = 0; i < star_count; i++) DrawCircle(stars[i].x, stars[i].y, stars[i].size, stars[i].color);

    if (!gameRunning || current_game_state == State_paused || current_game_state ==
        state_game_over || current_game_state == state_game_won) DrawMenu();
    else if (inTransition) DrawTransition(ship);
    else DrawGameplay(ship, assistShip, bigBoss, enemies, lasers, bossLasers, explosions);

    if (pendingTransition != Transition_none) DrawRectangle(0, 0, window_width,
        window_height, Fade(BLACK, fadeAlpha));
    EndDrawing();
}


void DrawMenu(void) {
    if (current_game_state == state_title) {
        Rectangle rec = { window_width / 2.0f, window_height / 2.0f, 600, 500 };
        Vector2 origin = { 300, 250 };
        DrawRectanglePro(rec, origin, -5.0f, Fade(DARKGRAY, 0.9f));
        DrawText("SPACE SHOOTER", 100, 100, 50, MAGENTA);
        DrawText("MAIN MENU", 600, 100, 40, MAGENTA);

        int startY = 300, spacing = 50;
        Color c0 = (menu_selection == 0) ? GREEN : MAGENTA;
        DrawText("START NEW GAME", window_width / 2 - 150, startY, 30, c0);
        if (menu_selection == 0) DrawText(">", window_width / 2 - 180, startY, 30, GREEN);

        Color c1 = (menu_selection == 1) ? GREEN : MAGENTA;
        DrawText("INSTRUCTIONS", window_width / 2 - 150, startY + spacing, 30, c1);
        if (menu_selection == 1) DrawText(">", window_width / 2 - 180, startY + spacing, 30, GREEN);

        DrawText(TextFormat("STATS (HIGH: %d)", highScore), window_width / 2 - 150, startY +
            spacing * 2, 30, GRAY);

       
        Color c2 = (menu_selection == 2) ? GREEN : MAGENTA; 
        DrawText("QUIT GAME", window_width / 2 - 150, startY + spacing * 3, 30, c2);
        if (menu_selection == 2) DrawText(">", window_width / 2 - 180, startY + spacing * 3, 30, GREEN); 

        DrawText("Use UP/DOWN Arrows to Move, ENTER to Select", 250, 800, 20, LIGHTGRAY);
    }
    else if (current_game_state == state_instructions) {
        DrawRectangle(100, 100, window_width - 200, window_height - 200,
            Fade(DARKGRAY, 0.9f));
        DrawRectangleLines(100, 100, window_width - 200, window_height - 200, WHITE);

        DrawText("SYSTEM DEFENDER: INSTRUCTIONS", 150, 150, 30, SKYBLUE);
        DrawText("MOVEMENT:", 150, 250, 20, YELLOW);
        DrawText("  LEFT/RIGHT ARROWS: Move your ship horizontally.", 150, 280, 20, WHITE);
        DrawText("COMBAT:", 150, 350, 20, YELLOW);
        DrawText("  SPACE: Fire player and wingman lasers.", 150, 380, 20, WHITE);
        DrawText("  H: Call/Dismiss Wingman (Assist Ship).", 150, 410, 20, WHITE);
        DrawText("GOALS:", 150, 480, 20, YELLOW);
        DrawText("  - Destroy enemies to increase SCORE and advance LEVELS.", 150, 510, 20, WHITE);
        DrawText("  - Survive 10 levels to face the IMPOSSIBLE BOSS.", 150, 540, 20, WHITE);
        DrawText("  - Enemies in Level 7+ require 2 HITS to destroy!", 150, 570, 20, RED);

        DrawText("Press [ENTER] to return", 150, 720, 20, GREEN);
    }
    //  STATE PAUSED 
    else if (current_game_state == State_paused) {
        DrawRectangle(300, 200, 300, 400, Fade(BLACK, 0.8f));
        DrawRectangleLines(300, 200, 300, 400, WHITE);

        DrawText("PAUSE MENU", 340, 250, 30, SKYBLUE);
        int startY = 330;
        int spacing = 50;
        Color itemColor;

        itemColor = (menu_selection == 0) ? GREEN : WHITE;
        DrawText("RESUME", 380, startY, 20, itemColor);
        if (menu_selection == 0) DrawText(">", 360, startY, 20, GREEN);

        itemColor = (menu_selection == 1) ? GREEN : WHITE;
        DrawText("RESTART GAME", 380, startY + spacing, 20, itemColor);
        if (menu_selection == 1) DrawText(">", 360, startY + spacing, 20, GREEN);

        itemColor = (menu_selection == 2) ? GREEN : WHITE;
        DrawText("QUIT TO TITLE", 380, startY + spacing * 2, 20, itemColor);
        if (menu_selection == 2) DrawText(">", 360, startY + spacing * 2, 20, GREEN);
    }
    //  STATE GAME OVER OR VICTORY 
    else if (current_game_state == state_game_over) {
        DrawText("GAME OVER", 300, 300, 60, RED);
        DrawText(TextFormat("SCORE: %d", score), 350, 400, 20, WHITE);
        DrawText("PRESS [ENTER]", 250, 550, 20, GRAY);
    }
    else if (current_game_state == state_game_won) {
        DrawText("CONGRATULATIONS!", 200, 300, 50, GOLD);
        DrawText("YOU SAVED THE GALAXY!", 230, 360, 30, GREEN);
        DrawText(TextFormat("FINAL SCORE: %d", score), 320, 450, 30, WHITE);
        DrawText("PRESS [ENTER] TO RETURN TO TITLE", 250, 600, 20, DARKGRAY);
    }
}

void DrawTransition(Spaceship& ship) {
    if (level == 11) {
        DrawText("WARNING!", 350, 250, 50, RED);
        DrawText("BOSS DETECTED", 300, 320, 30, RED);
    }
    else {
        DrawText("SECTOR CLEARED", 280, 300, 40, GREEN);
        DrawText(TextFormat("NEXT LEVEL: % d", level), 320, 400, 30, YELLOW);
    }
    Rectangle playerSource = { 0.0f, 0.0f, (float)playerTexture.width, (float)playerTexture.height
    };
    Rectangle playerDest = { ship.x, ship.y, ship.width, ship.height };
    DrawTexturePro(playerTexture, playerSource, playerDest, { 0,0 }, 0.0f, WHITE);
}


void DrawGameplay(Spaceship& ship, Spaceship& assistShip, Boss& bigBoss, Enemy enemies[], Laser lasers[], BossLaser bossLasers[], Explosion explosions[]) {
    // Lasers
    Rectangle laserSource = { 0.0f, 0.0f, (float)laserTexture.width, (float)laserTexture.height };
    for (int i = 0; i < max_lasers; i++) {
        if (lasers[i].active) {
            Rectangle laserDest = { lasers[i].x, lasers[i].y, 20, 60 };
            DrawTexturePro(laserTexture, laserSource, laserDest, { 0,0 }, 0.0f, WHITE);
        }
    }

    // Boss & Health Bar (NEW FEATURE)
    if (level == 11 && bigBoss.active) {
        Rectangle bossSource = { 0.0f, 0.0f, (float)bossTexture.width, (float)bossTexture.height };
        Rectangle bossDest = { bigBoss.x, bigBoss.y, bigBoss.width, bigBoss.height };

        // Flash Red when hit
        Color bossTint = WHITE;
        if (bigBoss.hp < bigBoss.maxHp && GetRandomValue(0, 10) > 8) bossTint = RED;
        DrawTexturePro(bossTexture, bossSource, bossDest, { 0,0 }, 0.0f, bossTint);

        // Boss Lasers (Rotated 180 degrees)
        Rectangle bLaserSource = { 0.0f, 0.0f, (float)bossLaserTexture.width, (float)bossLaserTexture.height };
        for (int i = 0; i < max_boss_lasers; i++) {
            if (bossLasers[i].active) {
                Rectangle bLaserDest = { bossLasers[i].x, bossLasers[i].y, bossLasers[i].width, bossLasers[i].height };
                DrawTexturePro(bossLaserTexture, bLaserSource, bLaserDest, { 0,0 }, 180.0f, WHITE);
            }
        }

        // Draw Health Bar
        float hpPercent = (float)bigBoss.hp / (float)bigBoss.maxHp;
        DrawRectangle(200, 50, 500, 20, GRAY);
        DrawRectangle(200, 50, (int)(500 * hpPercent), 20, RED);
        DrawRectangleLines(200, 50, 500, 20, WHITE);
        DrawText("BOSS HEALTH", 400, 25, 20, RED);
    }

    // Enemies
    Rectangle enemySource = { 0.0f, 0.0f, (float)enemyTexture.width, (float)enemyTexture.height };
    for (int i = 0; i < max_enemies; i++) {
        if (enemies[i].active) {
            Rectangle enemyDest = { enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height };
            
            Color tint = WHITE;
            if (enemies[i].maxHp > 1 && enemies[i].hp < enemies[i].maxHp) tint = RED;
            DrawTexturePro(enemyTexture, enemySource, enemyDest, { 0,0 }, 0.0f, tint);
        }
    }

    // Player
    Rectangle playerSource = { 0.0f, 0.0f, (float)playerTexture.width, (float)playerTexture.height };
    Rectangle playerDest = { ship.x, ship.y, ship.width, ship.height };
    DrawTexturePro(playerTexture, playerSource, playerDest, { 0,0 }, 0.0f, WHITE);

    // Assist Ship
    if (assistActive) {
        Rectangle assistSource = { 0.0f, 0.0f, (float)assistTexture.width, (float)assistTexture.height };
        Rectangle assistDest = { assistShip.x, assistShip.y, assistShip.width, assistShip.height };
        DrawTexturePro(assistTexture, assistSource, assistDest, { 0,0 }, 0.0f, WHITE);
    }

    // Explosions
    for (int i = 0; i < max_explosions; i++) {
        if (explosions[i].active) {
            Rectangle explSource = { explosions[i].currentFrame * frameWidth, 0.0f, frameWidth, (float)explosionTexture.height };
            Rectangle explDest = { explosions[i].x, explosions[i].y, 50, 50 };
            DrawTexturePro(explosionTexture, explSource, explDest, { 0,0 }, 0.0f, WHITE);
        }
    }

    // IMPROVED HUD (Using Code 1 variables)
    DrawText(TextFormat("SCORE: %d", score), 20, 20, 20, GREEN);
    DrawText(TextFormat("HIGH: %d", highScore), 20, 50, 20, GOLD);
    DrawText(TextFormat("LIVES: %d", lives), 20, 110, 20, RED);

    if (level == 11) {
        DrawText("LEVEL: IMPOSSIBLE", 20, 80, 20, RED);
    }
    else {
        DrawText(TextFormat("LEVEL: %d", level), 20, 80, 20, YELLOW);
        int remaining = enemies_to_Kill - enemies_killed;
        if (remaining < 0) remaining = 0;
        DrawText(TextFormat("ENEMIES LEFT: %d", remaining), 200, 20, 20, SKYBLUE);
    }

    if (!assistActive) DrawText("[H] CALL WINGMAN", 700, 20, 20, DARKGREEN);
    else DrawText("WINGMAN ACTIVE", 700, 20, 20, GREEN);
}