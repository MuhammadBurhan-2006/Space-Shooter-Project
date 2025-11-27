#include <iostream>
#include <raylib.h>
#include <cstdlib>
#include <fstream>

struct Star {
    float x;
    float y;
    float speed;
    int size;
    Color color;
};

struct Spaceship {
    float x, y;
    float width, height;
    float speed;
    Color bodyColor;
    Color cockpitColor;
    Color thrusterColor;
};

struct Laser {
    float x, y;
    float speed;
    bool active;
};

struct Enemy {
    float x, y;
    float width, height;
    float speed;
    bool active;
    Color color;
};

int main() {
    int windowWidth = 900;
    int windowHeight = 900;
    InitWindow(windowWidth, windowHeight, "C9----Space Shooter Project");
    SetTargetFPS(60);

    int score = 0;
    int level = 1;
    int lives = 3;

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

    Spaceship ship;
    ship.width = 60;
    ship.height = 80;
    ship.x = windowWidth / 2 - ship.width / 2;
    ship.y = windowHeight - ship.height - 20;
    ship.speed = 7.0f;
    ship.bodyColor = BLUE;
    ship.cockpitColor = SKYBLUE;
    ship.thrusterColor = ORANGE;

    Spaceship assistShip = ship;
    bool assistActive = false;

    const int MAX_LASERS = 60;
    Laser lasers[MAX_LASERS];
    for (int i = 0; i < MAX_LASERS; i++) {
        lasers[i].active = false;
        lasers[i].speed = 10.0f;
    }

    float shootCooldown = 0.2f;
    float shootTimer = 0.0f;
    float assistShootTimer = 0.0f;

    const int MAX_ENEMIES = 50;
    Enemy enemies[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].active = false;
        enemies[i].width = 50;
        enemies[i].height = 50;
        enemies[i].speed = 1.0f + level * 0.5f;
        enemies[i].color = RED;
    }

    std::ifstream infile("savegame.dat", std::ios::binary);
    if (infile.is_open()) {
        infile.read((char*)&score, sizeof(score));
        infile.read((char*)&level, sizeof(level));
        infile.read((char*)&lives, sizeof(lives));
        infile.close();
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        for (int i = 0; i < STAR_COUNT; i++) {
            stars[i].y += stars[i].speed;
            if (stars[i].y > windowHeight) {
                stars[i].y = 0;
                stars[i].x = GetRandomValue(0, windowWidth);
            }
            int brightnessChange = GetRandomValue(-5, 5);
            int newBrightness = stars[i].color.r + brightnessChange;
            if (newBrightness < 150) newBrightness = 150;
            if (newBrightness > 255) newBrightness = 255;
            stars[i].color = Color{ (unsigned char)newBrightness, (unsigned char)newBrightness, (unsigned char)newBrightness, 255 };
        }

        if (IsKeyDown(KEY_LEFT)) ship.x -= ship.speed;
        if (IsKeyDown(KEY_RIGHT)) ship.x += ship.speed;
        if (ship.x < 0) ship.x = 0;
        if (ship.x > windowWidth - ship.width) ship.x = windowWidth - ship.width;

        if (assistActive) {
            if (IsKeyDown(KEY_A)) assistShip.x -= assistShip.speed;
            if (IsKeyDown(KEY_D)) assistShip.x += assistShip.speed;
            if (assistShip.x < 0) assistShip.x = 0;
            if (assistShip.x > windowWidth - assistShip.width) assistShip.x = windowWidth - assistShip.width;
        }

        shootTimer -= dt;
        assistShootTimer -= dt;

        if (IsKeyDown(KEY_SPACE) && shootTimer <= 0.0f) {
            for (int i = 0; i < MAX_LASERS; i++) {
                if (!lasers[i].active) {
                    lasers[i].active = true;
                    lasers[i].x = ship.x + ship.width / 2 - 2;
                    lasers[i].y = ship.y;
                    shootTimer = shootCooldown;
                    break;
                }
            }
        }

        if (assistActive && IsKeyDown(KEY_F) && assistShootTimer <= 0.0f) {
            for (int i = 0; i < MAX_LASERS; i++) {
                if (!lasers[i].active) {
                    lasers[i].active = true;
                    lasers[i].x = assistShip.x + assistShip.width / 2 - 2;
                    lasers[i].y = assistShip.y;
                    assistShootTimer = shootCooldown;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_LASERS; i++) {
            if (lasers[i].active) {
                lasers[i].y -= lasers[i].speed;
                if (lasers[i].y < 0) lasers[i].active = false;
            }
        }

        for (int i = 0; i < level && i < MAX_ENEMIES; i++) {
            if (!enemies[i].active) {
                enemies[i].active = true;
                enemies[i].x = GetRandomValue(0, windowWidth - (int)enemies[i].width);
                enemies[i].y = -enemies[i].height;
                enemies[i].speed = 1.0f + level * 0.5f;
            }
        }

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                enemies[i].y += enemies[i].speed;
                if (enemies[i].y > windowHeight) {
                    enemies[i].active = false;
                    lives--;
                    if (lives <= 0) {
                        score = 0;
                        level = 1;
                        lives = 3;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_LASERS; i++) {
            if (lasers[i].active) {
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (enemies[j].active) {
                        Rectangle laserRec = { lasers[i].x, lasers[i].y, 4, 15 };
                        Rectangle enemyRec = { enemies[j].x, enemies[j].y, enemies[j].width, enemies[j].height };
                        if (CheckCollisionRecs(laserRec, enemyRec)) {
                            lasers[i].active = false;
                            enemies[j].active = false;
                            score += 10;
                            if (score % 100 == 0) level++;
                        }
                    }
                }
            }
        }

        if (IsKeyPressed(KEY_H)) {
            assistActive = !assistActive;
            if (assistActive) {
                assistShip.x = windowWidth / 2 - assistShip.width / 2;
                assistShip.y = windowHeight - assistShip.height - 20;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < STAR_COUNT; i++) 
            DrawCircle(stars[i].x, stars[i].y, stars[i].size, stars[i].color);
        for (int i = 0; i < MAX_LASERS; i++) 
            if (lasers[i].active) 
                DrawRectangle(lasers[i].x, lasers[i].y, 4, 30, YELLOW);
        for (int i = 0; i < MAX_ENEMIES; i++) 
            if (enemies[i].active) 
                DrawRectangle(enemies[i].x, enemies[i].y, enemies[i].width, enemies[i].height, enemies[i].color);

        DrawTriangle({ ship.x + ship.width / 2, ship.y }, { ship.x, ship.y + ship.height }, { ship.x + ship.width, ship.y + ship.height }, ship.bodyColor);
        DrawCircle(ship.x + ship.width / 2, ship.y + ship.height / 3, ship.width / 6, ship.cockpitColor);
        DrawTriangle({ ship.x + ship.width / 2, ship.y + ship.height + 10 }, { ship.x + ship.width / 4, ship.y + ship.height }, { ship.x + 3 * ship.width / 4, ship.y + ship.height }, ship.thrusterColor);

        if (assistActive) {
            DrawTriangle({ assistShip.x + assistShip.width / 2, assistShip.y }, { assistShip.x, assistShip.y + assistShip.height }, { assistShip.x + assistShip.width, assistShip.y + assistShip.height }, assistShip.bodyColor);
            DrawCircle(assistShip.x + assistShip.width / 2, assistShip.y + assistShip.height / 3, assistShip.width / 6, assistShip.cockpitColor);
            DrawTriangle({ assistShip.x + assistShip.width / 2, assistShip.y + assistShip.height + 10 }, { assistShip.x + assistShip.width / 4, assistShip.y + assistShip.height }, { assistShip.x + 3 * assistShip.width / 4, assistShip.y + assistShip.height }, assistShip.thrusterColor);
        }

        DrawText(TextFormat("Score: %d  Level: %d  Lives: %d", score, level, lives), 10, 10, 20, WHITE);
        EndDrawing();

        std::ofstream outfile("savegame.dat", std::ios::binary);
        outfile.write((char*)&score, sizeof(score));
        outfile.write((char*)&level, sizeof(level));
        outfile.write((char*)&lives, sizeof(lives));
        outfile.close();
    }

    CloseWindow();
    return 0;
}
