# R4 Game Engine in C++
This is a game engine written in C++ that uses the Windows console to display graphics. It allows you to create and update a console window with simple ASCII characters.

# Installation
This game engine is currently only supported on Windows. If you try to run it on any other operating system, it will print an error message and exit.

# Usage
To use this game engine, create a class that inherits from the **`r4GameEngine`** class and implement the **`OnUserCreate()`** and **`OnUserUpdate(double fElapsedTime)`** methods.

``` c++
class MyGame : public r4GameEngine {
public:
    bool OnUserCreate() {
        // initialization code
        return true;
    }

    bool OnUserUpdate(double fElapsedTime) {
        // update code
        return true;
    }
};

int main() {
    MyGame game;
    game.ConstructConsole(80, 60, 8, 8);
    game.Start();
}
```

The **`ConstructConsole()`** method initializes the console window with the given dimensions and font size. The **`Start()`** method starts the game loop, which calls the **`OnUserUpdate()`** method on each frame.

The Pixel struct represents a single pixel on the screen. You can draw pixels, lines, and shapes by calling the **`DrawPixel()`**, **`DrawLine()`**, and **`DrawPixels()`** methods, respectively.

``` c++
struct Pixel {
    int x, y;
    char c;
    short col;

    Pixel(const int& x = 0, const int& y = 0, const char& c = '#', const short& col = 0x000F) :
        x(x), y(y), c(c), col(col) {}
};
```

Usage of the Pixel struct with **`DrawPixel()`**, **`DrawLine()`**, and **`DrawPixels()`** methods.

``` c++
Pixel p(10, 10, '#', FG_RED | BG_GREEN);
game.DrawPixel(p);

Pixel p1(10, 20, '#', FG_BLUE | BG_YELLOW);
Pixel p2(20, 10, '#', FG_GREEN | BG_BLUE);
game.DrawLine(std::make_pair(p1, p2));

std::vector<Pixel> shape = {
    Pixel(30, 10, '#', FG_RED | BG_GREEN),
    Pixel(35, 15, '#', FG_BLUE | BG_YELLOW),
    Pixel(40, 10, '#', FG_GREEN | BG_BLUE)
};
game.DrawPixels(shape);
```
The **`ClearScreen()`** method fills the screen with the given character and color.

``` c++
game.ClearScreen(' ', FG_BLACK | BG_WHITE);
```
The **`m_bShowFPS`** variable controls whether the frames per second (FPS) is displayed in the top-left corner of the screen.

# License
This game engine is released under the MIT License.
