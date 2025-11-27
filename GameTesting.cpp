#include <iostream>
#include <raylib.h>
#include <cstdlib>
#include <fstream>
using namespace std;

// --- STRUCTS ---
struct Star {
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

int main() {
    int windowWidth = 900;
    int windowHeight = 900;
    InitWindow(windowWidth, windowHeight, "C9----Space Shooter (Final Version)");
    SetTargetFPS(60);
    InitAudioDevice(); // Audio system on karna zaroori hai

    // --- LOAD ASSETS ---
    // Make sure ye 7 files aapke project folder mein mojood hon!
    Texture2D playerTexture = LoadTexture("player.png");
    Texture2D enemyTexture = LoadTexture("enemy.png");
    Texture2D laserTexture = LoadTexture("laser.png");
    Texture2D explosionTexture = LoadTexture("explosion.png");
    Texture2D assistTexture = LoadTexture("assist.png");
    Texture2D bossTexture = LoadTexture("boss.png");
    Texture2D bossLaserTexture = LoadTexture("boss_laser.png");

    Sound shootSound = LoadSound("ahh-shoot.mp3");
    Sound explosionSound = LoadSound("explosion2.mp3");

    // --- EXPLOSION SETUP ---
    const int EXPLOSION_NUM_FRAMES = 5;
    float frameWidth = (float)(explosionTexture.width / EXPLOSION_NUM_FRAMES);
    float frameHeight = (float)explosionTexture.height;

    const int MAX_EXPLOSIONS = 50;
    Explosion explosions[MAX_EXPLOSIONS];
    for (int i = 0; i < MAX_EXPLOSIONS; i++) {
        explosions[i].active = false;
        explosions[i].currentFrame = 0;
        explosions[i].frameTimer = 0.0f;
    }

    // --- GAME STATE ---
    int score = 0;
    int highScore = 0;

    int level = 11; // GAME STARTS AT LEVEL 1

    int lives = 3;
    int nextLevelScore = 100;

    bool gameRunning = false;
    bool inTransition = false;
    bool gameWon = false;
    float transitionTimer = 0.0f;

    // --- STARS ---
    const int STAR_COUNT = 200;
    Star stars[STAR_COUNT];
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].x = GetRandomValue(0, windowWidth);
        stars[i].y = GetRandomValue(0, windowHeight);
        stars[i].speed = GetRandomValue(50, 200) / 100.0f;
        stars[i].size = GetRandomValue(1, 3);
        int brightness = GetRandomValue(150, 255);
        stars[i].color = Color{ (unsigned char)brightness, (unsigned char)brightness, (unsigned char)brightness, 255 };
    }

    // --- PLAYER ---
    Spaceship ship;
    ship.width = 60;
    ship.height = 80;
    ship.x = windowWidth / 2 - ship.width / 2;
    ship.y = windowHeight - ship.height - 20;
    ship.speed = 9.0f;

    // --- ASSISTANT ---
    Spaceship assistShip;
    assistShip.width = 60;
    assistShip.height = 80;
    assistShip.x = -100;
    assistShip.y = 0 ;
    assistShip.speed = 9.0f;
    bool assistActive = false;

    // --- BOSS SETUP ---
    Boss bigBoss;
    bigBoss.width = (float)bossTexture.width;
    bigBoss.height = (float)bossTexture.height;
    bigBoss.x = windowWidth / 2 - bigBoss.width / 2;
    bigBoss.y = -300;
    bigBoss.speed = 3.0f;
    bigBoss.maxHp = 500;
    bigBoss.hp = bigBoss.maxHp;
    bigBoss.active = false;
    bigBoss.entering = false;
    bigBoss.moveDir = 1;
    bigBoss.shootTimer = 0.0f;

    // --- BOSS LASERS ---
    const int MAX_BOSS_LASERS = 20;
    BossLaser bossLasers[MAX_BOSS_LASERS];
    for (int i = 0; i < MAX_BOSS_LASERS; i++) {
        bossLasers[i].active = false;
        bossLasers[i].speed = 7.0f;
        bossLasers[i].width = 20;
        bossLasers[i].height = 50;
    }

    // --- LASERS ---
    const int MAX_LASERS = 60;
    Laser lasers[MAX_LASERS];
    for (int i = 0; i < MAX_LASERS; i++) {
        lasers[i].active = false;
        lasers[i].speed = 12.0f;
    }

    float shootCooldown = 0.2f;
    float shootTimer = 0.0f;
    float assistShootTimer = 0.0f;

    // --- ENEMIES ---
    const int MAX_ENEMIES = 50;
    Enemy enemies[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
        enemies[i].width = 50;
        enemies[i].height = 50;
        enemies[i].speed = 1.0f;
        enemies[i].hp = 1;
        enemies[i].maxHp = 1;
    }

    // --- LOAD SAVE ---
    ifstream infile("savegame.dat", ios::binary);
    if (infile.is_open()) {
        infile.read((char*)&highScore, sizeof(highScore));
        infile.close();
    }

    // --- MAIN LOOP ---
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (score > highScore) highScore = score;

        // Stars Update
        for (int i = 0; i < STAR_COUNT; i++) {
            stars[i].y += stars[i].speed;
            if (stars[i].y > windowHeight) {
                stars[i].y = 0;
                stars[i].x = GetRandomValue(0, windowWidth);
            }
            if (GetRandomValue(0, 20) == 0) {
                int brightness = GetRandomValue(150, 255);
                stars[i].color = Color{ (unsigned char)brightness, (unsigned char)brightness, (unsigned char)brightness, 255 };
            }
        }

        if (!gameRunning) {
            // MENU
            if (IsKeyPressed(KEY_ENTER)) {
                gameRunning = true;
                gameWon = false;
                score = 0;
                lives = 3;
                level = 1; // Start Normal Game
                nextLevelScore = 100;

                inTransition = false;
                assistActive = false;

                // Reset everything
                bigBoss.active = false;
                bigBoss.hp = bigBoss.maxHp;
                bigBoss.y = -300;

                for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
                for (int i = 0; i < MAX_LASERS; i++) lasers[i].active = false;
                for (int i = 0; i < MAX_BOSS_LASERS; i++) bossLasers[i].active = false;

                PlaySound(shootSound);
            }
        }
        else if (inTransition) {
            // TRANSITION
            transitionTimer -= dt;
            float targetX = windowWidth / 2 - ship.width / 2;
            if (ship.x < targetX) ship.x += 2.0f;
            if (ship.x > targetX) ship.x -= 2.0f;
            if (transitionTimer <= 0.0f) {
                inTransition = false;
                if (level == 11) {
                    bigBoss.active = true;
                    bigBoss.entering = true;
                    bigBoss.y = -bigBoss.height;
                    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
                }
            }
        }
        else {
            // *** GAMEPLAY ***

            // Player Move
            if (IsKeyDown(KEY_LEFT)) ship.x -= ship.speed;
            if (IsKeyDown(KEY_RIGHT)) ship.x += ship.speed;
            if (ship.x < 0) ship.x = 0;
            if (ship.x > windowWidth - ship.width) ship.x = windowWidth - ship.width;

            // Assistant
            if (IsKeyPressed(KEY_H)) assistActive = !assistActive;
            if (assistActive) {
                assistShip.x = ship.x + 80;
                assistShip.y = ship.y;
                if (assistShip.x > windowWidth - assistShip.width) {
                assistShip.x = windowWidth - assistShip.width;
				ship.x = assistShip.x - 80;
            }
        }

            // Shooting
            shootTimer -= dt;
            if (IsKeyDown(KEY_SPACE) && shootTimer <= 0.0f) {
                bool fired = false;
                for (int i = 0; i < MAX_LASERS; i++) {
                    if (!lasers[i].active) {
                        lasers[i].active = true;
                        lasers[i].x = ship.x + ship.width / 2 - 10;
                        lasers[i].y = ship.y;
                        fired = true;
                        break;
                    }
                }
                if (assistActive) {
                    for (int i = 0; i < MAX_LASERS; i++) {
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

            // Update Lasers
            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    lasers[i].y -= lasers[i].speed;
                    if (lasers[i].y < 0) lasers[i].active = false;
                }
            }

            // --- BOSS LOGIC ---
            if (level == 11 && bigBoss.active) {
                if (bigBoss.entering) {
                    bigBoss.y += 1.0f;
                    if (bigBoss.y >= 50) {
                        bigBoss.entering = false;
                    }
                }
                else {
                    // Move
                    bigBoss.x += bigBoss.speed * bigBoss.moveDir;
                    if (bigBoss.x <= 0) bigBoss.moveDir = 1;
                    if (bigBoss.x + bigBoss.width >= windowWidth) bigBoss.moveDir = -1;

                    // Shoot
                    bigBoss.shootTimer -= dt;
                    if (bigBoss.shootTimer <= 0.0f) {
                        for (int i = 0; i < MAX_BOSS_LASERS; i++) {
                            if (!bossLasers[i].active) {
                                bossLasers[i].active = true;
                                bossLasers[i].x = bigBoss.x + bigBoss.width / 2 - 10; // Center
                                bossLasers[i].y = bigBoss.y + bigBoss.height;
                                break;
                            }
                        }
                        bigBoss.shootTimer = 0.8f;
                    }
                }

                // Boss Lasers Update
                for (int i = 0; i < MAX_BOSS_LASERS; i++) {
                    if (bossLasers[i].active) {
                        bossLasers[i].y += bossLasers[i].speed;

                        Rectangle bLaserRec = { bossLasers[i].x, bossLasers[i].y, bossLasers[i].width, bossLasers[i].height };
                        Rectangle playerRec = { ship.x, ship.y, ship.width, ship.height };

                        if (CheckCollisionRecs(bLaserRec, playerRec)) {
                            bossLasers[i].active = false;
                            lives--;
                            if (lives <= 0) gameRunning = false;
                        }
                        if (bossLasers[i].y > windowHeight) bossLasers[i].active = false;
                    }
                }

                // Boss Body Collision
                float pad = 40.0f;
                Rectangle bossHitbox = {
                    bigBoss.x + pad,
                    bigBoss.y + pad,
                    bigBoss.width - (pad * 2),
                    bigBoss.height - (pad * 2)
                };
                Rectangle playerRec = { ship.x, ship.y, ship.width, ship.height };

                if (CheckCollisionRecs(bossHitbox, playerRec)) {
                    lives = 0;
                    gameRunning = false;
                }
            }

            // --- SPAWN ENEMIES ---
            int maxEnemiesOnScreen = (level == 11) ? 0 : (5 + level * 2);
            

            for (int i = 0; i < maxEnemiesOnScreen; i++) {
                if (!enemies[i].active) {
                    // Spawn Chance
                    if (GetRandomValue(0, 100) < (2 + level)) {
                        enemies[i].active = true;
                        enemies[i].x = GetRandomValue(0, windowWidth - (int)enemies[i].width);
                        enemies[i].y = -enemies[i].height; // Start ABOVE screen
                        enemies[i].speed = 1.0f + ((level - 1) * 0.25f);
                        if (level <= 3) { enemies[i].hp = 1; enemies[i].maxHp = 1; }
                        else { enemies[i].hp = 2; enemies[i].maxHp = 2; }
                    }
                }
            }

            // Update Enemies
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    enemies[i].y += enemies[i].speed; // Move Down

                    Rectangle enemyRec = { enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height };
                    Rectangle playerRec = { ship.x, ship.y, ship.width, ship.height };

                    if (CheckCollisionRecs(enemyRec, playerRec)) {
                        enemies[i].active = false;
                        lives--;
                        if (lives <= 0) gameRunning = false;
                    }

                    if (enemies[i].y > windowHeight) {
                        enemies[i].active = false;
                    }
                }
            }

            // Update Explosions
            for (int i = 0; i < MAX_EXPLOSIONS; i++) {
                if (explosions[i].active) {
                    explosions[i].frameTimer += dt;
                    if (explosions[i].frameTimer >= 0.05f) {
                        explosions[i].frameTimer = 0.0f;
                        explosions[i].currentFrame++;
                        if (explosions[i].currentFrame >= EXPLOSION_NUM_FRAMES) {
                            explosions[i].active = false;
                        }
                    }
                }
            }

            // Collision Logic
            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    Rectangle laserRec = { lasers[i].x, lasers[i].y, 20, 60 };

                    // 1. BOSS COLLISION
                    if (level == 11 && bigBoss.active) {
                        float pad = 40.0f;
                        Rectangle bossHitbox = {
                            bigBoss.x + pad,
                            bigBoss.y + pad,
                            bigBoss.width - (pad * 2),
                            bigBoss.height - (pad * 2)
                        };

                        if (CheckCollisionRecs(laserRec, bossHitbox)) {
                            lasers[i].active = false;
                            bigBoss.hp--;

                            // Explosion
                            for (int k = 0; k < MAX_EXPLOSIONS; k++) {
                                if (!explosions[k].active) {
                                    explosions[k].active = true;
                                    explosions[k].x = lasers[i].x - 15;
                                    explosions[k].y = lasers[i].y - 15;
                                    explosions[k].currentFrame = 0;
                                    explosions[k].frameTimer = 0.0f;
                                    break;
                                }
                            }

                            if (bigBoss.hp <= 0) {
                                bigBoss.active = false;
                                PlaySound(explosionSound);
                                // Victory Explosion
                                for (int k = 0; k < MAX_EXPLOSIONS; k++) {
                                    if (!explosions[k].active) {
                                        explosions[k].active = true;
                                        explosions[k].x = bigBoss.x + bigBoss.width / 2 - 25;
                                        explosions[k].y = bigBoss.y + bigBoss.height / 2 - 25;
                                        explosions[k].currentFrame = 0;
                                        break;
                                    }
                                }
                                gameWon = true;
                                gameRunning = false;
                            }
                        }
                    }

                    // 2. Enemy Collision
                    for (int j = 0; j < MAX_ENEMIES; j++) {
                        if (enemies[j].active) {
                            Rectangle enemyRec = { enemies[j].x, enemies[j].y, enemies[j].width, enemies[j].height };

                            if (CheckCollisionRecs(laserRec, enemyRec)) {
                                lasers[i].active = false;
                                enemies[j].hp--;
                                enemies[j].y -= 15;

                                if (enemies[j].hp <= 0) {
                                    enemies[j].active = false;
                                    PlaySound(explosionSound);
                                    for (int k = 0; k < MAX_EXPLOSIONS; k++) {
                                        if (!explosions[k].active) {
                                            explosions[k].active = true;
                                            explosions[k].x = enemies[j].x;
                                            explosions[k].y = enemies[j].y;
                                            explosions[k].currentFrame = 0;
                                            break;
                                        }
                                    }
                                    score += 10;

                                    if (level < 11 && score >= nextLevelScore) {
                                        if (level == 10) {
                                            level++;
                                            nextLevelScore += 9999;
                                            inTransition = true;
                                            transitionTimer = 3.0f;
                                            for (int k = 0; k < MAX_ENEMIES; k++) enemies[k].active = false;
                                            for (int k = 0; k < MAX_LASERS; k++) lasers[k].active = false;
                                        }
                                        else {
                                            level++;
                                            nextLevelScore += 100;
                                            inTransition = true;
                                            transitionTimer = 3.0f;
                                            for (int k = 0; k < MAX_ENEMIES; k++) enemies[k].active = false;
                                            for (int k = 0; k < MAX_LASERS; k++) lasers[k].active = false;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ofstream outfile("savegame.dat", ios::binary);
            outfile.write((char*)&highScore, sizeof(highScore));
            outfile.close();
        }

        // --- DRAWING ---
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < STAR_COUNT; i++)
            DrawCircle(stars[i].x, stars[i].y, stars[i].size, stars[i].color);

        if (!gameRunning) {
            if (gameWon) {
                DrawText("CONGRATULATIONS!", 200, 300, 50, GOLD);
                DrawText("YOU SAVED THE GALAXY!", 230, 360, 30, GREEN);
                DrawText(TextFormat("FINAL SCORE: %d", score), 320, 450, 30, WHITE);
                DrawText("PRESS [ENTER] TO PLAY AGAIN", 280, 600, 20, DARKGRAY);
            }
            else if (lives <= 0 && score > 0) {
                DrawText("GAME OVER", 300, 300, 60, RED);
                DrawText(TextFormat("SCORE: %d", score), 350, 400, 20, WHITE);
                DrawText(TextFormat("HIGH: %d", highScore), 350, 450, 20, GOLD);
                DrawText("PRESS [ENTER] TO RESTART", 300, 550, 20, GRAY);
            }
            else {
                // TITLE SCREEN
                DrawText("SYSTEM DEFENDER", 260, 300, 40, SKYBLUE);
                DrawText("PRESS [ENTER] TO START", 280, 400, 25, GREEN);
                DrawText(TextFormat("HIGH SCORE: %d", highScore), 350, 500, 20, GOLD);
                DrawText("[SPACE] FIRE | [H] CALL WINGMAN", 280, 550, 20, DARKGRAY);
            }
        }
        else if (inTransition) {
            if (level == 11) {
                DrawText("WARNING!", 350, 250, 50, RED);
                DrawText("BOSS DETECTED", 300, 320, 30, RED);
            }
            else {
                DrawText("SECTOR CLEARED", 280, 300, 40, GREEN);
                DrawText(TextFormat("NEXT LEVEL: %d", level), 320, 400, 30, YELLOW);
            }
            Rectangle playerSource = { 0.0f, 0.0f, (float)playerTexture.width, (float)playerTexture.height };
            Rectangle playerDest = { ship.x, ship.y, ship.width, ship.height };
            DrawTexturePro(playerTexture, playerSource, playerDest, { 0,0 }, 0.0f, WHITE);
        }
        else {
            // GAMEPLAY DRAW

            Rectangle laserSource = { 0.0f, 0.0f, (float)laserTexture.width, (float)laserTexture.height };
            for (int i = 0; i < MAX_LASERS; i++) {
                if (lasers[i].active) {
                    Rectangle laserDest = { lasers[i].x, lasers[i].y, 20, 60 };
                    DrawTexturePro(laserTexture, laserSource, laserDest, { 0,0 }, 0.0f, WHITE);
                }
            }

            if (level == 11 && bigBoss.active) {
                Rectangle bossSource = { 0.0f, 0.0f, (float)bossTexture.width, (float)bossTexture.height };
                Rectangle bossDest = { bigBoss.x, bigBoss.y, bigBoss.width, bigBoss.height };
                Color bossTint = WHITE;
                if (bigBoss.hp < bigBoss.maxHp && GetRandomValue(0, 10) > 8) bossTint = RED;
                DrawTexturePro(bossTexture, bossSource, bossDest, { 0,0 }, 0.0f, bossTint);

                // DRAW BOSS LASERS
                Rectangle bLaserSource = { 0.0f, 0.0f, (float)bossLaserTexture.width, (float)bossLaserTexture.height };
                for (int i = 0; i < MAX_BOSS_LASERS; i++) {
                    if (bossLasers[i].active) {
                        Rectangle bLaserDest = { bossLasers[i].x, bossLasers[i].y, bossLasers[i].width, bossLasers[i].height };
                        DrawTexturePro(bossLaserTexture, bLaserSource, bLaserDest, { 0,0 }, 180.0f, WHITE);
                    }
                }

                float hpPercent = (float)bigBoss.hp / (float)bigBoss.maxHp;
                DrawRectangle(200, 50, 500, 20, GRAY);
                DrawRectangle(200, 50, (int)(500 * hpPercent), 20, RED);
                DrawRectangleLines(200, 50, 500, 20, WHITE);
                DrawText("BOSS HEALTH", 400, 25, 20, RED);
            }

            // Draw Enemies
            Rectangle enemySource = { 0.0f, 0.0f, (float)enemyTexture.width, (float)enemyTexture.height };
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    Rectangle enemyDest = { enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height };
                    Color tint = WHITE;
                    if (enemies[i].maxHp > 1 && enemies[i].hp < enemies[i].maxHp) tint = RED;

                    if (enemyTexture.id > 0) {
                        DrawTexturePro(enemyTexture, enemySource, enemyDest, { 0,0 }, 0.0f, tint);
                    }
                    else {
                        DrawRectangleRec(enemyDest, RED);
                    }
                }
            }

            Rectangle playerSource = { 0.0f, 0.0f, (float)playerTexture.width, (float)playerTexture.height };
            Rectangle playerDest = { ship.x, ship.y, ship.width, ship.height };
            DrawTexturePro(playerTexture, playerSource, playerDest, { 0,0 }, 0.0f, WHITE);

            if (assistActive) {
                Rectangle assistSource = { 0.0f, 0.0f, (float)assistTexture.width, (float)assistTexture.height };
                Rectangle assistDest = { assistShip.x, assistShip.y, assistShip.width, assistShip.height };
                DrawTexturePro(assistTexture, assistSource, assistDest, { 0,0 }, 0.0f, WHITE);
            }

            for (int i = 0; i < MAX_EXPLOSIONS; i++) {
                if (explosions[i].active) {
                    Rectangle explSource = { explosions[i].currentFrame * frameWidth, 0.0f, frameWidth, frameHeight };
                    Rectangle explDest = { explosions[i].x, explosions[i].y, 50, 50 };
                    DrawTexturePro(explosionTexture, explSource, explDest, { 0,0 }, 0.0f, WHITE);
                }
            }

            DrawText(TextFormat("SCORE: %d", score), 20, 20, 20, GREEN);
            DrawText(TextFormat("HIGH: %d", highScore), 20, 50, 20, GOLD);
            if (level == 11) DrawText("LEVEL: IMPOSSIBLE", 20, 80, 20, RED);
            else {
                DrawText(TextFormat("LEVEL: %d", level), 20, 80, 20, YELLOW);
                DrawText(TextFormat("TARGET: %d", nextLevelScore), 200, 20, 20, SKYBLUE);
            }

            DrawText(TextFormat("LIVES: %d", lives), 20, 110, 20, RED);

            if (!assistActive) DrawText("[H] CALL WINGMAN", 700, 20, 20, DARKGRAY);
            else DrawText("WINGMAN ACTIVE", 700, 20, 20, GREEN);
        }

        EndDrawing();
    }

    UnloadTexture(playerTexture);
    UnloadTexture(enemyTexture);
    UnloadTexture(laserTexture);
    UnloadTexture(explosionTexture);
    UnloadTexture(assistTexture);
    UnloadTexture(bossTexture);
    UnloadTexture(bossLaserTexture);

    UnloadSound(shootSound);
    UnloadSound(explosionSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}