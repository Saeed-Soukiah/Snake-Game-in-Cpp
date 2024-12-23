/*
    Name: Saeed Soukiah
    Date: 24/12/2024
    Retro Snake Game using Raylib
    This C++ program implements a classic snake game using the Raylib library. 
    It features:
    snake movement, food generation, collision detection, and speed control to increase difficulty over time.
*/

#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

// Global variables for game settings and states
static bool allowMove = false;       // Controls if the snake can move in the current frame
Color green = { 173, 204, 96, 255 }; // Background color
Color darkGreen = { 43, 51, 24, 255 }; // Snake and border color

int cellSize = 30;                  // Size of each grid cell
int cellCount = 25;                 // Number of cells along each dimension of the grid
int offset = 75;                    // Offset for the grid from the window edges

const double SPEED_UP_INTERVAL = 10.0; // Time interval for increasing game speed (in seconds)
const float SPEED_MULTIPLIER = 0.9f;   // Factor by which game speed increases

float gameSpeed = 0.2f;                // Initial speed of the game (lower is faster)
double lastSpeedUpTime = 0;           // Tracks the last time speed was increased

double lastUpdateTime = 0;           // Tracks the last time the game logic was updated

// Function to check if a given element exists in a deque
bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

// Function to check if a specific time interval has elapsed
bool EventTriggered(double interval)
{
    double currentTime = GetTime(); // Get the current time in seconds
    if (currentTime - lastUpdateTime >= interval)
    {
        lastUpdateTime = currentTime; // Update the last update time
        return true;
    }
    return false;
}

// Function to gradually increase the game speed
void SpeedUpGame()
{
    double currentTime = GetTime(); // Get the current time
    if (currentTime - lastSpeedUpTime >= SPEED_UP_INTERVAL)
    {
        gameSpeed *= SPEED_MULTIPLIER; // Increase the game speed
        lastSpeedUpTime = currentTime; // Update the last speed-up time
    }
}

// Snake class to handle the snake's behavior and state
class Snake
{
public:
    deque<Vector2> body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} }; // Initial snake body
    Vector2 direction = { 1, 0 }; // Initial direction of movement
    bool addSegment = false;     // Whether to add a new segment to the snake

    // Draws the snake on the screen
    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{ offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize };
            DrawRectangleRounded(segment, 0.5, 6, darkGreen);
        }
    }

    // Updates the snake's position and handles growth
    void Update()
    {
        body.push_front(Vector2Add(body[0], direction)); // Add a new head in the direction of movement
        if (!addSegment)
        {
            body.pop_back(); // Remove the tail segment if no new segment is to be added
        }
        else
        {
            addSegment = false;
        }
    }

    // Resets the snake to its initial state
    void Reset()
    {
        body = { Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9} };
        direction = { 1, 0 };
    }
};

// Food class to manage food generation and rendering
class Food
{
public:
    Vector2 position;         // Current position of the food
    Texture2D texture;        // Texture for rendering the food

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody); // Generate initial food position
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    // Draws the food on the screen
    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    // Generates a random position within the grid
    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{ x, y };
    }

    // Ensures the food position does not overlap with the snake
    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

// Game class to handle the overall game logic and state
class Game
{
public:
    Snake snake = Snake();          // Snake instance
    Food food = Food(snake.body);   // Food instance
    bool running = true;            // Indicates if the game is running
    int score = 0;                  // Current score
    Sound eatSound;                 // Sound effect for eating food
    Sound wallSound;                // Sound effect for collisions with walls

    Game()
    {
        InitAudioDevice();          // Initialize the audio system
        eatSound = LoadSound("Sounds/eat.mp3");
        wallSound = LoadSound("Sounds/wall.mp3");
    }

    ~Game()
    {
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();         // Close the audio system
    }

    // Draws the game elements on the screen
    void Draw()
    {
        food.Draw();
        snake.Draw();
        DrawText(TextFormat("Score: %i", score), offset, offset - 40, 20, darkGreen);
    }

    // Updates the game logic
    void Update()
    {
        if (running)
        {
            snake.Update();
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
        SpeedUpGame(); // Adjust game speed over time
    }

    // Checks if the snake has eaten the food
    void CheckCollisionWithFood()
    {
        if (Vector2Equals(snake.body[0], food.position))
        {
            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;
            score++;
            PlaySound(eatSound);
        }
    }

    // Checks for collisions with the edges of the grid
    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1 || snake.body[0].y == cellCount || snake.body[0].y == -1)
        {
            GameOver();
        }
    }

    // Resets the game when the snake collides with itself or a wall
    void GameOver()
    {
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body);
        running = false;
        score = 0;
        gameSpeed = 0.2f; // Reset speed
        lastSpeedUpTime = GetTime();
        PlaySound(wallSound);
    }

    // Checks for collisions between the snake's head and its body
    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
    }
};

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Retro Snake");
    SetTargetFPS(60);

    Game game = Game();

    while (!WindowShouldClose())
    {
        BeginDrawing();

        if (EventTriggered(gameSpeed))
        {
            allowMove = true;
            game.Update();
        }

        // Handle user input for snake direction
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove)
        {
            game.snake.direction = { 0, -1 };
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove)
        {
            game.snake.direction = { 0, 1 };
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove)
        {
            game.snake.direction = { -1, 0 };
            game.running = true;
            allowMove = false;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove)
        {
            game.snake.direction = { 1, 0 };
            game.running = true;
            allowMove = false;
        }

        // Drawing
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10 }, 5, darkGreen);
        DrawText("Retro Snake", offset - 5, 20, 40, darkGreen);
        game.Draw();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
