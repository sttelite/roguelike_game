// roguelike : Endless roguelike game

#define COLOR_RESET  "\033[0m"
#define COLOR_WALL   "\033[90m"
#define COLOR_FLOOR  "\033[37m"
#define COLOR_PLAYER "\033[93m"
#define COLOR_STAIRS "\033[96m"

#define NOMINMAX

#include <iostream>
#include <windows.h>
#include <conio.h>
#include <random>
#include <vector>
#include <algorithm>
#include <string>

void setCursorPosition(int x, int y) {
	COORD cursorPosition;
	cursorPosition.X = x;
	cursorPosition.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
}

int getRandomInt(int min, int max) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(min, max);

	return distrib(gen);
}

struct Vec2 {
	int x, y;

	bool operator==(const Vec2& other) {
		return x == other.x && y == other.y;
	}
};

struct Room {
	int x, y, w, h;

	Vec2 center() const {
		return { x + w / 2, y + h / 2 };
	}

	bool intersects(const Room& other) const {
		return (x - 1 <= other.x + other.w && x + w + 1 >= other.x &&
			y - 1 <= other.y + other.h && y + h + 1 >= other.y);
	}
};


struct Entity {
	Vec2 pos;
	char symbol;
	std::string color;
	int hp;

	Entity(int x, int y, char s, std::string c) :
		pos({ x, y }), symbol(s), color(c), hp(100) {
	}
};

class GameMap {
private:
	int width, height;
	std::vector<char> data;

public:
	GameMap(int w, int h) : width(w), height(h), data(w* h, '#') {}

	char getCell(int x, int y) const {
		if (x < 0 || x >= width || y < 0 || y >= height) return '#';
		return data[y * width + x];
	}

	void setCell(int x, int y, char val) {
		if (x >= 0 && x < width && y >= 0 && y < height)
			data[y * width + x] = val;
	}

	void fill(char symbol) {
		for (auto& c : data) {
			c = symbol;
		}
	}

	int getWidth() const { return width; }
	int getHeight() const { return height; }
};

class Engine {
private:
	bool running;
	GameMap Map;
	Entity player;
	int currentLevel = 0;

	void digCorridorH(int x1, int x2, int y) {
		for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) {
			Map.setCell(x, y, '.');
		}
	}

	void digCorridorV(int y1, int y2, int x) {
		for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) {
			Map.setCell(x, y, '.');
		}
	}

	void generateLevel() {
		Map.fill('#');
		std::vector<Room> rooms;

		const int MAX_ROOMS = 10;
		const int MIN_ROOM_SIZE = 4;
		const int MAX_ROOM_SIZE = 8;

		for (int i = 0; i < 30; ++i) {
			int w = getRandomInt(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
			int h = getRandomInt(MIN_ROOM_SIZE, MAX_ROOM_SIZE);
			int x = getRandomInt(1, Map.getWidth() - w - 1);
			int y = getRandomInt(1, Map.getHeight() - h - 1);

			Room newRoom = { x, y, w, h };

			bool failed = false;
			for (const Room& otherRoom : rooms) {
				if (newRoom.intersects(otherRoom)) {
					failed = true;
					break;
				}
			}

			if (!failed) {
				for (int ry = y; ry < y + h; ++ry) {
					for (int rx = x; rx < x + w; ++rx) {
						Map.setCell(rx, ry, '.');
					}
				}

				if (rooms.empty()) {
					player.pos = newRoom.center();
				}
				else {
					Vec2 prevCenter = rooms.back().center();
					Vec2 newCenter = newRoom.center();

					if (getRandomInt(0, 1) == 1) {
						digCorridorH(prevCenter.x, newCenter.x, prevCenter.y);
						digCorridorV(prevCenter.y, newCenter.y, newCenter.x);
					}
					else {
						digCorridorV(prevCenter.y, newCenter.y, prevCenter.x);
						digCorridorH(prevCenter.x, newCenter.x, newCenter.y);
					}
				}

				rooms.push_back(newRoom);
				if (rooms.size() == MAX_ROOMS) break;
			}
		}

		Vec2 lastRoomCenter = rooms.back().center();
		Map.setCell(lastRoomCenter.x, lastRoomCenter.y, '>');
	}



public:
	Engine() : Map(40, 25), running(true), player(5, 5, '@', COLOR_PLAYER), currentLevel(1) {
		generateLevel();
	}

	void handleInput() {
		int dx = 0, dy = 0;
		char key = _getch();

		switch (key) {
		case 'w': dy = -1; break;
		case 's': dy = 1; break;
		case 'a': dx = -1; break;
		case 'd': dx = 1; break;
		case 27: running = false; break;
		}

		int newX = player.pos.x + dx;
		int newY = player.pos.y + dy;
		char targetCell = Map.getCell(newX, newY);
		if (targetCell == '.' || targetCell == '>') {
			player.pos.x = newX;
			player.pos.y = newY;
			if (targetCell == '>') {
				++currentLevel;
				generateLevel();
			}
		}

	}

	void render() {
		setCursorPosition(0, 0);
		std::string frame = "=== LEVEL " + std::to_string(currentLevel) + " ===\n";

		for (int y = 0; y < Map.getHeight(); y++) {
			for (int x = 0; x < Map.getWidth(); x++) {
				if (player.pos.x == x && player.pos.y == y) {
					frame += player.color + player.symbol + COLOR_RESET;
				}
				else {
					char tile = Map.getCell(x, y);
					if (tile == '#') frame += COLOR_WALL + std::string("#") + COLOR_RESET;
					else if (tile == '.') frame += COLOR_FLOOR + std::string(".") + COLOR_RESET;
					else if (tile == '>') frame += COLOR_STAIRS + std::string(">") + COLOR_RESET;
					else frame += tile;
				}
			}
			frame += '\n';
		}

		std::cout << frame << std::flush;
	}



	void run() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(hConsole, &cursorInfo);
		cursorInfo.bVisible = FALSE;
		SetConsoleCursorInfo(hConsole, &cursorInfo);

		render();

		while (running) {
			handleInput();
			if (running) {
				render();
			}
		}

	}
};


int main() {
	Engine game;
	game.run();
	return 0;
}
