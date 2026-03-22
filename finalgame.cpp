#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

const int WIDTH = 82, HEIGHT = 29, MAX_OBSTACLES = 50;

class Entity {
public:
    virtual void update() = 0;
    virtual void draw(char screen[HEIGHT][WIDTH]) = 0;
    virtual ~Entity() {}
};

class Cat : public Entity {
    int x = WIDTH / 2, y = HEIGHT - 5;
    const char* shape[3] = {
        "  /\\_/\\  ",
        " ( o.o ) ",
        "  > ^ <  "
    };

public:
    void update() override {}
    void draw(char screen[HEIGHT][WIDTH]) override {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 9; ++j)
                if (y + i < HEIGHT && x + j < WIDTH)
                    screen[y + i][x + j] = shape[i][j];
    }

    void move(int dx, int dy) {
        if (x + dx >= 0 && x + dx + 9 < WIDTH) x += dx;
        if (y + dy >= 0 && y + dy + 3 < HEIGHT) y += dy;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getWidth() const { return 7; }
    int getHeight() const { return 2; }
};

class Obstacle : public Entity {
    int dx = 0, dy = 0;
public:
    int x = 0, y = 0;
    bool active = true;

    void setup(const Cat& cat) {
        static int edge = 0;
        edge = (edge + 1) % 4;

        switch (edge) {
            case 0: x = WIDTH / 2; y = 0; break;
            case 1: x = WIDTH / 2; y = HEIGHT - 1; break;
            case 2: x = 0; y = HEIGHT / 2; break;
            case 3: x = WIDTH - 1; y = HEIGHT / 2; break;
        }

        dx = (cat.getX() > x) - (cat.getX() < x);
        dy = (cat.getY() > y) - (cat.getY() < y);
    }

    void update() override {
        x += dx; y += dy;
        active = (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT);
    }

    void draw(char screen[HEIGHT][WIDTH]) override {
        if (active) screen[y][x] = 'O';
    }

    bool isActive() const { return active; }

    bool collidesWith(const Cat& cat) const {
        return x >= cat.getX() && x < cat.getX() + cat.getWidth()
            && y >= cat.getY() && y < cat.getY() + cat.getHeight();
    }
};

class Game {
    Cat cat;
    Obstacle obstacles[MAX_OBSTACLES];
    int count = 0, score = 0, lives = 3, timer = 0;

    void handleInput() {
        if (GetAsyncKeyState(VK_UP))    cat.move(0, -1);
        if (GetAsyncKeyState(VK_DOWN))  cat.move(0, 1);
        if (GetAsyncKeyState(VK_LEFT))  cat.move(-1, 0);
        if (GetAsyncKeyState(VK_RIGHT)) cat.move(1, 0);
        if (GetAsyncKeyState('X')) {
            system("cls");
            cout << "?? You exited the game.\nFinal Score: " << score << "\n";
            exit(0);
        }
    }

    void drawUI(char screen[HEIGHT][WIDTH]) {
        string msg = "Press X to Exit | Arrow Keys to Dodge!";
        for (int i = 0; i < msg.size(); ++i) screen[2][10 + i] = msg[i];

        string livesMsg = "LIVES: " + to_string(lives);
        for (int i = 0; i < livesMsg.size(); ++i) screen[3][5 + i] = livesMsg[i];

        for (int i = 0; i < WIDTH; ++i) screen[HEIGHT - 4][i] = '=';
    }

    void clearScreen(char screen[HEIGHT][WIDTH]) {
        for (int r = 0; r < HEIGHT; ++r)
            for (int c = 0; c < WIDTH; ++c)
                screen[r][c] = ' ';
    }
    public:
    void run() {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD written;

        while (true) {
            handleInput();
            cat.update();

            for (int i = 0; i < count; ++i) obstacles[i].update();

            for (int i = 0; i < count; ++i) {
                if (!obstacles[i].collidesWith(cat) && !obstacles[i].isActive())
                    score++;
            }

            for (int i = 0; i < count; )
                if (!obstacles[i].isActive())
                    obstacles[i] = obstacles[--count];
                else i++;

            if (++timer > 15 && count < MAX_OBSTACLES) {
                obstacles[count++].setup(cat);
                timer = 0;
            }

            for (int i = 0; i < count; ++i) {
                if (obstacles[i].collidesWith(cat)) {
                    if (--lives <= 0) {
                        system("cls");
                        cout << "?? Game Over! You were hit.\nFinal Score: " << score << "\n";
                        return;
                    }
                    count = 0;
                    break;
                }
            }

            char screen[HEIGHT][WIDTH];
            clearScreen(screen);
            drawUI(screen);
            cat.draw(screen);
            for (int i = 0; i < count; ++i) obstacles[i].draw(screen);

            for (int r = 0; r < HEIGHT; ++r)
                WriteConsoleOutputCharacterA(hOut, screen[r], WIDTH, {0, (SHORT)r}, &written);

            Sleep(100);
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}