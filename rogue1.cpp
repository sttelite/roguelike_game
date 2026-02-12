// roguelike : Endless roguelike game with monsters and some progress

#define COLOR_RESET  "\033[0m"
#define COLOR_WALL   "\033[90m"
#define COLOR_PLAYER "\033[93m"

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <vector>

void setCursorPosition(int x, int y) {
    COORD cursorPosition;
    cursorPosition.X = x;
    cursorPosition.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

struct Vec2 {
    int x, y;

    bool operator==(const Vec2& other) {
        return x == other.x && y == other.y;
    }
};

struct Entity {
    Vec2 pos;
    char symbol;
    std::string color;
    int hp; 

    Entity(int x, int y, char s, std::string c) :
        pos({ x, y }), symbol(s), color(c), hp(100) {}
};

class GameMap {
private:
    int width, height;
    Entity player;
    std::vector<char> data;

public:
    GameMap(int w, int h) : width(w), height(h), data(w * h, '#'), player(0, 0, '!', COLOR_PLAYER) {}

    char getCell(int x, int y) const {
        return data[y * width + x];
    }
    
    void setCell(int x, int y, char val) {
        data[y * width + x] = val;
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

void render(const GameMap& map) {
    setCursorPosition(0, 0);

    std::string frame = "";

    for (int y = 0; y < map.getHeight(); y++) {
        for (int x = 0; x < map.getWidth(); x++) {
            char tile = map.getCell(x, y);
            if (tile == '#') frame += COLOR_WALL + std::string("#") + COLOR_RESET;
            else if (tile == '@') frame += COLOR_PLAYER + std::string("@") + COLOR_RESET;
            else frame += tile;
        }
        frame += '\n';
    }

    std::cout << frame << std::flush;
}


class Engine {
private:
    bool running;
    GameMap Map;

public:
    Engine() : Map(40, 25), running(true) {}

    void run(){
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hConsole, &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &cursorInfo);

        while (running) {
            render(Map);
            Sleep(20);
        }

    }
};


int main()
{
    Engine game;
    game.run();
}
