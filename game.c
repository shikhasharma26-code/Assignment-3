#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <mmsystem.h> // Required for PlaySound

// --- Game Constants ---
#define MAP_SIZE 7
#define WALL '#'
#define FLOOR ' '
#define PLAYER 'P'
#define GOLD '$'
#define MONSTER 'M'
#define EXIT 'E'

// --- Game State Variables ---
char map[MAP_SIZE][MAP_SIZE];
int player_x, player_y;
int player_health = 5;
int turns = 0;
int score = 0;
int game_status = 0; // 0=playing, 1=won, -1=lost

// --- Console Helper Function ---

/* Function to remove flicker by resetting the cursor position */
void clear_screen_fast() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(h, pos);
}

// --- Game Logic Functions ---

/**
 * Initializes the game map and places entities.
 */
void initialize_game() {
    // 1. Fill map with walls
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            map[i][j] = WALL;
        }
    }

    // 2. Create a basic path (simple cross pattern)
    for (int i = 0; i < MAP_SIZE; i++) {
        map[i][MAP_SIZE / 2] = FLOOR; // Vertical path
        map[MAP_SIZE / 2][i] = FLOOR; // Horizontal path
    }

    // 3. Randomly place some floor tiles
    for (int k = 0; k < 10; k++) {
        map[rand() % (MAP_SIZE - 2) + 1][rand() % (MAP_SIZE - 2) + 1] = FLOOR;
    }

    // 4. Place Entities
    // Player start (center)
    player_x = MAP_SIZE / 2;
    player_y = MAP_SIZE / 2;
    map[player_y][player_x] = PLAYER;

    // Place Exit (E)
    map[MAP_SIZE - 2][MAP_SIZE - 2] = EXIT;

    // Place Gold ($)
    map[1][1] = GOLD;

    // Place Monster (M)
    map[1][MAP_SIZE - 2] = MONSTER;
    
    // Reset state
    player_health = 5;
    turns = 0;
    score = 0;
    game_status = 0;
}

/**
 * Prints the current state of the map and HUD (Heads-Up Display).
 */
void draw_game() {
    clear_screen_fast();

    printf("======================================\n");
    printf("|          THE C-DUNGEON             |\n");
    printf("======================================\n");
    printf(" Health: %d | Turns: %d | Gold Score: %d \n", player_health, turns, score);
    printf("======================================\n");

    // Draw the 7x7 map
    for (int i = 0; i < MAP_SIZE; i++) {
        printf("| ");
        for (int j = 0; j < MAP_SIZE; j++) {
            printf("%c ", map[i][j]);
        }
        printf("|\n");
    }
    printf("======================================\n");
    printf("Controls: WASD or Arrow Keys to move.\n");
    printf("Collect '$' to score, avoid 'M'. Find 'E' to win.\n");
}

/**
 * Handles the game logic when the player moves to a new tile.
 * @param new_x The new column index.
 * @param new_y The new row index.
 */
void check_interaction(int new_x, int new_y) {
    char target = map[new_y][new_x];

    if (target == GOLD) {
        printf("You found gold! (+10 Score)\n");
        score += 10;
        map[new_y][new_x] = FLOOR; // Gold is collected
        PlaySound(TEXT("ding.wav"), NULL, SND_ASYNC);
    } 
    else if (target == MONSTER) {
        printf("A monster attacked you! (-1 Health)\n");
        player_health -= 1;
        map[new_y][new_x] = FLOOR; // Monster is defeated/runs away
        PlaySound(TEXT("scream.wav"), NULL, SND_ASYNC);
    } 
    else if (target == EXIT) {
        printf("You found the dungeon exit!\n");
        game_status = 1; // Win
    }
    
    // Check for game over condition
    if (player_health <= 0) {
        game_status = -1; // Loss
    }
}

/**
 * Processes player movement based on keyboard input.
 * @param dx Change in X (column).
 * @param dy Change in Y (row).
 */
void move_player(int dx, int dy) {
    int new_x = player_x + dx;
    int new_y = player_y + dy;

    // Check boundary conditions
    if (new_x < 0 || new_x >= MAP_SIZE || new_y < 0 || new_y >= MAP_SIZE) {
        printf("Cannot move out of bounds.\n");
        return;
    }

    // Check for wall
    if (map[new_y][new_x] == WALL) {
        printf("That's a wall!\n");
        return;
    }

    // Interaction with the tile
    check_interaction(new_x, new_y);

    // If the game status hasn't changed (not won/lost), proceed with movement
    if (game_status == 0) {
        // Clear old position
        map[player_y][player_x] = FLOOR;

        // Update position
        player_x = new_x;
        player_y = new_y;

        // Draw player at new position
        map[player_y][player_x] = PLAYER;
        
        // Increment turn counter
        turns++;
        
        // Play step sound (optional, assuming step.wav exists)
        // PlaySound(TEXT("step.wav"), NULL, SND_ASYNC);
    }
}

/**
 * Main function to handle user input.
 */
void handle_input() {
    int dx = 0, dy = 0;
    int key = getch();

    // Check for arrow keys (which are two key presses: 0xE0 then the actual code)
    if (key == 0 || key == 0xE0) {
        key = getch(); // Get the actual arrow key code
    }

    switch (key) {
        // W or Up Arrow
        case 'w':
        case 'W':
        case 72: // Up Arrow
            dy = -1;
            break;

        // S or Down Arrow
        case 's':
        case 'S':
        case 80: // Down Arrow
            dy = 1;
            break;

        // A or Left Arrow
        case 'a':
        case 'A':
        case 75: // Left Arrow
            dx = -1;
            break;

        // D or Right Arrow
        case 'd':
        case 'D':
        case 77: // Right Arrow
            dx = 1;
            break;

        // Q (Quit)
        case 'q':
        case 'Q':
            game_status = -1; // End game
            break;
            
        default:
            // Do nothing for other keys
            return;
    }

    // If a valid move key was pressed, execute the move
    if (dx != 0 || dy != 0) {
        move_player(dx, dy);
    }
}


int main() {
    // Set console properties
    system("color 1F"); // Blue background, bright white text
    srand(time(NULL));

    initialize_game();

    // --- Main Game Loop ---
    while (game_status == 0) {
        draw_game();
        
        // Wait for player input
        while (!_kbhit()) {
            Sleep(10); // Sleep briefly while waiting for input
        }

        handle_input();
    }
    
    // --- Game Over Screen ---
    clear_screen_fast();
    printf("\n\n\n\n");
    if (game_status == 1) {
        system("color 2F"); // Green on Win
        printf("            *** VICTORY! ***\n");
        PlaySound(TEXT("win.wav"), NULL, SND_ASYNC);
    } else {
        system("color 4F"); // Red on Loss
        printf("            *** GAME OVER! ***\n");
        PlaySound(TEXT("lose.wav"), NULL, SND_ASYNC);
    }
    
    printf("    You survived %d turns and collected %d gold.\n\n", turns, score);
    printf("           Press any key to exit.\n");

    getch(); // Wait for final input before closing
    return 0;
}
