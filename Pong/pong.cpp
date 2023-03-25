#define _USE_MATH_DEFINES
#include <cmath>
#include <ctime>
#include "r4GameEngine.hpp"

class Game : public r4GameEngine {
    int
        nScore0, nScore1;
    float
        fPaddle0X, fPaddle0Y,
        fPaddle1X, fPaddle1Y,
        fBallX, fBallY, fBallA,
        fBallVelX, fBallVelY;
    const float
        fPaddleSize = 16.0f,
        fPaddleVel = 32.0f,
        fBallVel = 50.0f;

    bool OnUserCreate() {
        // Code ran when loaded
        // Set app name
        m_sAppName = L"R4WLYX's Game Engine - Pong";

        // Set rand() seed
        srand((unsigned int)time(NULL));

        // Set initial score
        nScore0 = 0;
        nScore1 = 0;

        // Sets paddles' initial position
        fPaddle0X = 8;
        fPaddle0Y = ScreenHeight() / 2.0f;

        fPaddle1X = ScreenWidth() - 9;
        fPaddle1Y = ScreenHeight() / 2.0f;

        // Sets ball's initial position
        fBallX = ScreenWidth() / 2.0f;
        fBallY = ScreenHeight() / 2.0f;

        // Sets ball's initial angle
        fBallA = rand()%20 - 10 + 90;

        // Sets ball's initial velocity
        fBallVelX = sinf(fBallA * M_PI / 180)*fBallVel;
        fBallVelY = cosf(fBallA * M_PI / 180)*fBallVel;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return true;
    }

    bool OnUserUpdate(double elapsedTime) { 
        // Code ran every frame
        // Set rand() seed
        srand((unsigned int)time(NULL));

        // Clear screen
        ClearScreen();

        // Render everything
        // Draw pong!
        DrawTexts(ScreenWidth()/2, ScreenHeight()/2-6, {
            ":::::::::     ::::::::    ::::    :::    ::::::::    :::",
            ":::::::::     ::::::::    ::::    :::    ::::::::    :::",
            ":+:    :+:   :+:    :+:   :+:+:   :+:   :+:    :+:   :+:",
            ":+:    :+:   :+:    :+:   :+:+:   :+:   :+:    :+:   :+:",
            "+:+    +:+   +:+    +:+   :+:+:+  +:+   +:+          +:+",
            "+:+    +:+   +:+    +:+   :+:+:+  +:+   +:+          +:+",
            "+#++:++#+    +#+    +:+   +#+ +:+ +#+   :#:          +#+",
            "+#++:++#+    +#+    +:+   +#+ +:+ +#+   :#:          +#+",
            "+#+          +#+    +#+   +#+  +#+#+#   +#+   +#+#   +#+",
            "+#+          +#+    +#+   +#+  +#+#+#   +#+   +#+#   +#+",
            "#+#          #+#    #+#   #+#   #+#+#   #+#    #+#      ",
            "#+#          #+#    #+#   #+#   #+#+#   #+#    #+#      ",
            "###           ########    ###    ####    ########    ###",
            "###           ########    ###    ####    ########    ###"
        }, CENTER, FG_DARK_GREY);

        // Draw border and paddles
        DrawLines({
            {Pixel(0, 0, '*'), Pixel(ScreenWidth()-1, 0)},
            {Pixel(ScreenWidth()-1, 0, '*'), Pixel(ScreenWidth()-1, ScreenHeight()-1)},
            {Pixel(ScreenWidth()-1, ScreenHeight()-1, '*'), Pixel(0, ScreenHeight()-1)},
            {Pixel(0, ScreenHeight()-1, '*'), Pixel(0, 0)},
            {Pixel(fPaddle0X, fPaddle0Y - fPaddleSize/2, '#', FG_RED), Pixel(fPaddle0X, fPaddle0Y + fPaddleSize/2)},
            {Pixel(fPaddle0X-1, fPaddle0Y - fPaddleSize/2, '%', FG_RED), Pixel(fPaddle0X-1, fPaddle0Y + fPaddleSize/2)},
            {Pixel(fPaddle1X, fPaddle1Y - fPaddleSize/2, '#', FG_BLUE), Pixel(fPaddle1X, fPaddle1Y + fPaddleSize/2)},
            {Pixel(fPaddle1X+1, fPaddle1Y - fPaddleSize/2, '%', FG_BLUE), Pixel(fPaddle1X+1, fPaddle1Y + fPaddleSize/2)}
        });

        // Draw ball
        DrawPixel(Pixel(fBallX, fBallY, '@'));

        // Display score
        DrawText(ScreenWidth()/4, 10, std::to_string(nScore0), CENTER);
        DrawText(ScreenWidth()*3/4, 10, std::to_string(nScore1), CENTER);

        // Move paddle 0
        if (GetAsyncKeyState('W') & 0x8000)
            fPaddle0Y -= fPaddleVel*elapsedTime;
        if (GetAsyncKeyState('S') & 0x8000)
            fPaddle0Y += fPaddleVel*elapsedTime;

        // Move paddle 1
        if (GetAsyncKeyState(VK_UP) & 0x8000)
            fPaddle1Y -= fPaddleVel*elapsedTime;
        if (GetAsyncKeyState(VK_DOWN) & 0x8000)
            fPaddle1Y += fPaddleVel*elapsedTime;

        // Move ball
        fBallX += fBallVelX*elapsedTime;
        fBallY += fBallVelY*elapsedTime;

        // Check for collisions
        // Check if paddle 0 is out of bounds
        if (fPaddle0Y - fPaddleSize/2 <= 1)
            fPaddle0Y = fPaddleSize/2+1;
        else if (fPaddle0Y + fPaddleSize/2 >= ScreenHeight()-2)
            fPaddle0Y = ScreenHeight() - fPaddleSize/2-2;
        
        // Check if paddle 1 is out of bounds
        if (fPaddle1Y - fPaddleSize/2 <= 1)
            fPaddle1Y = fPaddleSize/2+1;
        else if (fPaddle1Y + fPaddleSize/2 >= ScreenHeight()-2)
            fPaddle1Y = ScreenHeight() - fPaddleSize/2-2;

        // Check if ball hits top or bottom
        if (fBallY <= 1) {
            fBallVelY = -fBallVelY;
            fBallY = 2;
        } else if (fBallY >= ScreenHeight()-2) {
            fBallVelY = -fBallVelY;
            fBallY = ScreenHeight()-3;
        }

        // Check if ball hits a paddle        
        if (fBallX <= fPaddle0X &&
            fBallX >= fPaddle0X-2.0f &&
            fBallY < fPaddle0Y + fPaddleSize/2.0f &&
            fBallY > fPaddle0Y - fPaddleSize/2.0f) { 
            fBallVelX = -fBallVelX;
            fBallX = fPaddle0X+1;
        } else if (fBallX >= fPaddle1X &&
            fBallX <= fPaddle1X+2.0f &&
            fBallY < fPaddle1Y + fPaddleSize/2.0f &&
            fBallY > fPaddle1Y - fPaddleSize/2.0f) {
            fBallVelX = -fBallVelX;
            fBallX = fPaddle1X-1;
        }

        // Check if anyone scored
        if (fBallX <= 0) {
            fPaddle0Y = ScreenHeight() / 2.0f;
            fPaddle1Y = ScreenHeight() / 2.0f;
            fBallX = ScreenWidth() / 2.0f;
            fBallY = ScreenHeight() / 2.0f;
            fBallA = rand()%20 - 10 + 90;
            fBallVelX = sinf(fBallA * M_PI / 180)*fBallVel;
            fBallVelY = cosf(fBallA * M_PI / 180)*fBallVel;
            nScore1++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        } else if (fBallX >= ScreenWidth()-1) {
            fPaddle0Y = ScreenHeight() / 2.0f;
            fPaddle1Y = ScreenHeight() / 2.0f;
            fBallX = ScreenWidth() / 2.0f;
            fBallY = ScreenHeight() / 2.0f;
            fBallA = rand()%20 - 10 + 270;
            fBallVelX = sinf(fBallA * M_PI / 180)*fBallVel;
            fBallVelY = cosf(fBallA * M_PI / 180)*fBallVel;
            nScore0++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        return true;
    }
};

int main() {
    // Construct the console and run the game
    Game game;
    game.ConstructConsole(160, 80, 8, 8, true);
    game.Start();
}