#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include "Engine.hpp"

const int WIDTH = 1024;
const int HEIGHT = 1024;
const int NUM_AGENTS = 10000;
const float PI = M_PI;

void setTrail(float *grid, int x, int y, float value) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
        grid[y * WIDTH + x] = value;
}

float getTrail(const float *grid, int x, int y) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
        return grid[y * WIDTH + x];
    return -10.0f;
}

struct Agent {
    float x, y;
    float angle;

    float speed = 2.0f;
    float turnAngle = PI / 10.0f;
    float sensorAngleOffset = PI / 3.0f;
    float sensorDistance = 20.0f;

    void senseAndSteer(const float* readGrid) {
        float sensorAngleLeft  = angle + sensorAngleOffset;
        float sensorAngleFwd   = angle;
        float sensorAngleRight = angle - sensorAngleOffset;

        int leftX  = static_cast<int>(x + sensorDistance * cos(sensorAngleLeft));
        int leftY  = static_cast<int>(y + sensorDistance * sin(sensorAngleLeft));
        int fwdX   = static_cast<int>(x + sensorDistance * cos(sensorAngleFwd));
        int fwdY   = static_cast<int>(y + sensorDistance * sin(sensorAngleFwd));
        int rightX = static_cast<int>(x + sensorDistance * cos(sensorAngleRight));
        int rightY = static_cast<int>(y + sensorDistance * sin(sensorAngleRight));

        float weightL = getTrail(readGrid, leftX,  leftY);
        float weightF = getTrail(readGrid, fwdX,   fwdY);
        float weightR = getTrail(readGrid, rightX, rightY);

        if (weightF > weightL && weightF > weightR) {
            // Keep going straight
        } else if (weightF < weightL && weightF < weightR) {
            // Forward is weakest — random turn
            angle += (rand() % 2 == 0) ? turnAngle : -turnAngle;
        } else if (weightL > weightR) {
            angle += turnAngle;
        } else if (weightR > weightL) {
            angle -= turnAngle;
        } else {
            // Tie-break
            angle += (rand() % 2 == 0) ? turnAngle : -turnAngle;
        }
    }
};

int main() {
    RenderEngine engine(WIDTH, HEIGHT);
    if (!engine.init()) {
        std::cerr << "Failed to initialize graphics engine!" << std::endl;
        return -1;
    }

    // Two grids instead of one
    float* readGrid  = new float[WIDTH * HEIGHT]();
    float* writeGrid = new float[WIDTH * HEIGHT]();

    Agent* agents = new Agent[NUM_AGENTS]();

    // Random positions, angles, speeds
    for (int i = 0; i < NUM_AGENTS; i++) {
        agents[i].x     = (float)(rand() % WIDTH);
        agents[i].y     = (float)(rand() % HEIGHT);
        agents[i].angle = (float)rand() / RAND_MAX * 2.0f * PI;
        agents[i].speed = 1.0f + (float)rand() / RAND_MAX * 2.0f;
    }

    while (!engine.shouldClose()) {

        // Step A: Diffuse + Decay readGrid -> writeGrid
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                float sum = 0.0f;
                for (int ny = -1; ny <= 1; ny++)
                    for (int nx = -1; nx <= 1; nx++) {
                        float val = getTrail(readGrid, x + nx, y + ny);
                        sum += (val < 0) ? 0 : val;
                    }
                writeGrid[y * WIDTH + x] = (sum / 9.0f) * 0.99f;
            }
        }

        // Step B: Sense, steer, move, deposit
        for (int i = 0; i < NUM_AGENTS; ++i) {
            agents[i].senseAndSteer(readGrid);

            agents[i].x += cos(agents[i].angle) * agents[i].speed;
            agents[i].y += sin(agents[i].angle) * agents[i].speed;

            if (agents[i].x <= 0)          { agents[i].x = 0;          agents[i].angle = PI - agents[i].angle; }
            else if (agents[i].x >= WIDTH)  { agents[i].x = WIDTH - 1;  agents[i].angle = PI - agents[i].angle; }
            if (agents[i].y <= 0)          { agents[i].y = 0;          agents[i].angle = -agents[i].angle; }
            else if (agents[i].y >= HEIGHT) { agents[i].y = HEIGHT - 1; agents[i].angle = -agents[i].angle; }

            setTrail(writeGrid, (int)agents[i].x, (int)agents[i].y, 1.0f);
        }

        // Step C: Render
        engine.renderFrame(writeGrid);

        // Step D: Swap
        float* temp = readGrid;
        readGrid    = writeGrid;
        writeGrid   = temp;
    }

    delete[] readGrid;
    delete[] writeGrid;
    delete[] agents;
    engine.shutdown();
    return 0;
}