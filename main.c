#include <simple2d.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define GROUND_Y 500
#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 40
#define MAX_SPIKES 2
#define MAX_LIVES 5
#define MAX_COINS 10
#define MAX_ENEMIES 2
#define ENEMY_LEFT_BOUNDARY 200
#define ENEMY_RIGHT_BOUNDARY 600

typedef struct {
    int data[MAX_COINS];
    int top;
} Stack;

void init_stack(Stack *stack) {
    stack->top = -1;
}

int is_empty(Stack *stack) {
    return stack->top == -1;
}

int is_full(Stack *stack) {
    return stack->top == MAX_COINS - 1;
}

void push(Stack *stack, int value) {
    if (!is_full(stack)) {
        stack->data[++stack->top] = value;
    }
}

int pop(Stack *stack) {
    if (!is_empty(stack)) {
        return stack->data[stack->top--];
    }
    return -1;
}

Stack coin_stack; 
S2D_Sound *coin_sound;
S2D_Sound *shock_sound;

typedef struct {
    float x, y;
    float w, h;
    int is_collected;
} GameObject;

GameObject player;
GameObject spikes[MAX_SPIKES];
GameObject coins[MAX_COINS];
GameObject enemies[MAX_ENEMIES];
float enemy_speed[MAX_ENEMIES];
GameObject flag;

int player_speed = 5;
int jump_power = 20;
int is_jumping = 0;
int lives = MAX_LIVES;
int current_level = 1;
int game_over = 0;
int score = 0;
int invincible_timer = 0;

S2D_Text *score_text;
S2D_Text *game_over_text;

void reset_player() {
    player.x = 100;
    player.y = GROUND_Y - PLAYER_HEIGHT;
    invincible_timer = 60;
}

void load_level(int level) {
    char filename[20];
    sprintf(filename, "level%d.txt", level);
    FILE *file = fopen(filename, "r");

    if (!file) {
        printf("Failed to load level %d\n", level);
        return;
    }

    int spike_count = 0, coin_count = 0, enemy_count = 0;
    while (!feof(file)) {
        char object_type[10];
        float x, y;
        if (fscanf(file, "%s %f %f", object_type, &x, &y) != 3) break;

        if (strcmp(object_type, "coin") == 0 && coin_count < MAX_COINS) {
            coins[coin_count] = (GameObject){x, y, 20, 20, 0};
            coin_count++;
        } else if (strcmp(object_type, "spike") == 0 && spike_count < MAX_SPIKES) {
            spikes[spike_count] = (GameObject){x, y, 40, 40, 0};
            spike_count++;
        } else if (strcmp(object_type, "enemy") == 0 && enemy_count < MAX_ENEMIES) {
            enemies[enemy_count] = (GameObject){x, y, 40, 40, 0};
            enemy_speed[enemy_count] = 2;
            enemy_count++;
        } else if (strcmp(object_type, "flag") == 0) {
            flag = (GameObject){x, y, 40, 40, 0};
        }
    }

    fclose(file);
}

int check_collision(GameObject a, GameObject b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x &&
            a.y < b.y + b.h && a.y + a.h > b.y);
}

void update() {
    if (game_over) return;

    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT] && player.x > 0) player.x -= player_speed;
    if (keys[SDL_SCANCODE_RIGHT]) player.x += player_speed;

    if (keys[SDL_SCANCODE_SPACE] && player.y == GROUND_Y - PLAYER_HEIGHT && is_jumping == 0) {
        is_jumping = jump_power;
    }

    if (keys[SDL_SCANCODE_U]) {
        int last_collected = pop(&coin_stack);
        if (last_collected != -1) {
            coins[last_collected].is_collected = 0;
            score -= 10;
        }
    }

    if (is_jumping > 0) {
        player.y -= is_jumping;
        is_jumping--;
    } else if (player.y < GROUND_Y - PLAYER_HEIGHT) {
        player.y += 8;
        if (player.y > GROUND_Y - PLAYER_HEIGHT) player.y = GROUND_Y - PLAYER_HEIGHT;
    }

    if (invincible_timer > 0) invincible_timer--;

    for (int i = 0; i < MAX_SPIKES; i++) {
        if (check_collision(player, spikes[i]) && invincible_timer == 0) {
            lives--;
            reset_player();
            S2D_PlaySound(shock_sound);
            if (lives <= 0) game_over = 1;
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].x += enemy_speed[i];

        if (enemies[i].x <= ENEMY_LEFT_BOUNDARY || enemies[i].x + enemies[i].w >= ENEMY_RIGHT_BOUNDARY) {
            enemy_speed[i] *= -1;
        }

        if (check_collision(player, enemies[i]) && invincible_timer == 0) {
            lives--;
            reset_player();
            S2D_PlaySound(shock_sound);
            if (lives <= 0) game_over = 1;
        }
    }

    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].is_collected && check_collision(player, coins[i])) {
            coins[i].is_collected = 1;
            score += 10;
            push(&coin_stack, i); 
            S2D_PlaySound(coin_sound);
        }
    }

    if (check_collision(player, flag)) {
        game_over = 1;
    }

    if (player.x > WINDOW_WIDTH) {
        if (current_level == 1) {
            current_level = 2;
            load_level(current_level);
            reset_player();
        } else {
            game_over = 1;
        }
    }

    char score_buf[50];
    snprintf(score_buf, sizeof(score_buf), "Score: %d", score);
    S2D_FreeText(score_text);
    score_text = S2D_CreateText("arial.ttf", score_buf, 30);
    score_text->x = 10;
    score_text->y = 10;
}

void render() {
    S2D_DrawQuad(0, 0, 0, 0, 0, 0,
                 WINDOW_WIDTH, 0, 0, 0, 0, 0,
                 WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, 0, 0,
                 0, WINDOW_HEIGHT, 0, 0, 0, 0);

    S2D_DrawQuad(0, GROUND_Y, 0.2, 0.8, 0.2, 1,
                 WINDOW_WIDTH, GROUND_Y, 0.2, 0.8, 0.2, 1,
                 WINDOW_WIDTH, WINDOW_HEIGHT, 0.2, 0.8, 0.2, 1,
                 0, WINDOW_HEIGHT, 0.2, 0.8, 0.2, 1);

    S2D_DrawQuad(player.x, player.y, 1, 0, 0, 1,
                 player.x + player.w, player.y, 1, 0, 0, 1,
                 player.x + player.w, player.y + player.h, 1, 0, 0, 1,
                 player.x, player.y + player.h, 1, 0, 0, 1);

    for (int i = 0; i < MAX_COINS; i++) {
        if (!coins[i].is_collected) {
            S2D_DrawCircle(coins[i].x, coins[i].y, 10, 20, 1, 1, 0, 1);
        }
    }

    for (int i = 0; i < MAX_SPIKES; i++) {
        S2D_DrawTriangle(
            spikes[i].x, spikes[i].y + spikes[i].h, 1, 1, 0, 1,
            spikes[i].x + spikes[i].w / 2, spikes[i].y, 1, 1, 0, 1,
            spikes[i].x + spikes[i].w, spikes[i].y + spikes[i].h, 1, 1, 0, 1
        );
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].w == 0 || enemies[i].h == 0) continue;
        S2D_DrawCircle(enemies[i].x + enemies[i].w / 2, enemies[i].y + enemies[i].h / 2, 20, 20, 1, 0, 0, 1);
    }

    if (!game_over) {
        S2D_DrawQuad(flag.x, flag.y, 0, 1, 0, 1,
                     flag.x + flag.w, flag.y, 0, 1, 0, 1,
                     flag.x + flag.w, flag.y + flag.h, 0, 1, 0, 1,
                     flag.x, flag.y + flag.h, 0, 1, 0, 1);
    }

    for (int i = 0; i < MAX_LIVES; i++) {
        float red = (i < lives) ? 1.0f : 1.0f;
        float green = (i < lives) ? 0.0f : 1.0f;
        float blue = (i < lives) ? 0.0f : 1.0f;
        S2D_DrawCircle(WINDOW_WIDTH - 20 - i * 20, 20, 8, 20, red, green, blue, 1);
    }

    S2D_DrawText(score_text);

    if (game_over) {
        S2D_DrawText(game_over_text);
    }
}

int main() {
    player.w = PLAYER_WIDTH;
    player.h = PLAYER_HEIGHT;
    reset_player();
    load_level(current_level);

    init_stack(&coin_stack); 
    
    coin_sound = S2D_CreateSound("coin.wav");
    shock_sound = S2D_CreateSound("shock.wav");

    score_text = S2D_CreateText("arial.ttf", "Score: 0", 20);
    score_text->x = 20;
    score_text->y = 20;
    score_text->color.r = 1;
    score_text->color.g = 1;
    score_text->color.b = 1;
    score_text->color.a = 1;

    game_over_text = S2D_CreateText("arial.ttf", "Game Over", 40);
    game_over_text->x = WINDOW_WIDTH / 2 - 100;
    game_over_text->y = WINDOW_HEIGHT / 2 - 20;
    game_over_text->color.r = 1;
    game_over_text->color.g = 0;
    game_over_text->color.b = 0;
    game_over_text->color.a = 1;

    S2D_Window *window = S2D_CreateWindow(
        "Mario", WINDOW_WIDTH, WINDOW_HEIGHT, update, render, 0
    );

    S2D_Show(window);

    S2D_FreeText(score_text);
    S2D_FreeText(game_over_text);
    S2D_FreeSound(coin_sound);
    S2D_FreeSound(shock_sound);
    S2D_FreeWindow(window);

    return 0;
}