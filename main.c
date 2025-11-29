#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>


// STOVE: STOVE
// CTBD: cutting board
// FLOR: floor
// DLVR: deliver (onde entregamos os pedidos)
typedef enum block{
WALL,
STOVE,
CTBD,
FLOR,
DLVR,
} block;

typedef struct pos_st {
    int x;
    int y;
} Pos;

typedef struct player_st{
    Pos pos;
} Player;

#define LEVEL_SIZE 10
#define screenHeight 500
#define screenWidth 500

#include "boards.h"

void render(block board[LEVEL_SIZE][LEVEL_SIZE], Player *p1){

    Vector2 offset = {screenWidth%LEVEL_SIZE,screenHeight%LEVEL_SIZE};
    BeginDrawing();
    ClearBackground(GRAY);

    
    Vector2 sizeofsquare = {
        screenWidth/LEVEL_SIZE ,
        screenHeight/LEVEL_SIZE ,
    };

    for(int i = 0; i < LEVEL_SIZE; i++){
        for(int j = 0; j < LEVEL_SIZE; j++){
			switch(board[i][j]) {
				case FLOR:
					DrawRectangleV((Vector2){sizeofsquare.x*j,sizeofsquare.y*i},sizeofsquare,(Color){50, 50, 50, 255});
					break;
				case WALL:
					DrawRectangleV((Vector2){sizeofsquare.x*j,sizeofsquare.y*i},sizeofsquare,(Color){150, 150, 150, 255});
					break;
				case CTBD:
					DrawRectangleV((Vector2){sizeofsquare.x*j,sizeofsquare.y*i},sizeofsquare,(Color){255, 255, 255, 255});
					break;
				case STOVE:
					DrawRectangleV((Vector2){sizeofsquare.x*j,sizeofsquare.y*i},sizeofsquare,(Color){50, 50, 255, 255});
					break;
				case DLVR:
					DrawRectangleV((Vector2){sizeofsquare.x*j,sizeofsquare.y*i},sizeofsquare,(Color){50, 255, 50, 255});
					break;
			}
        }
    }
    for (int i = 0; i < screenWidth/LEVEL_SIZE + 1; i++){
        DrawLineV((Vector2){i*sizeofsquare.x, 0}, (Vector2){i*sizeofsquare.x, screenHeight}, LIGHTGRAY);
    }

    for (int i = 0; i < screenHeight/LEVEL_SIZE + 1; i++){
        DrawLineV((Vector2){0, i*sizeofsquare.y}, (Vector2){screenWidth, i*sizeofsquare.y}, LIGHTGRAY);
    }

    DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,RED);



    EndDrawing();

}

int position_player(block board[LEVEL_SIZE][LEVEL_SIZE],Player *p1){
    for(int i = 0; i < LEVEL_SIZE; i++){
        for(int j = 0; j < LEVEL_SIZE; j++){
            if (board[i][j] == FLOR) {
                p1->pos.x = i;
                p1-> pos.y = j;
                return 0;
            }
        }
    }
    return -1;
}

void moveUp(block board[LEVEL_SIZE][LEVEL_SIZE],Player *p1){
    if (p1->pos.y < LEVEL_SIZE){
        if (board[p1->pos.y - 1][p1->pos.x] == FLOR){
            p1->pos.y -= 1;
        }
    }
}

void moveDown(block board[LEVEL_SIZE][LEVEL_SIZE],Player *p1){
    if (p1->pos.y < LEVEL_SIZE){
        if (board[p1->pos.y + 1][p1->pos.x] == FLOR){
            p1->pos.y += 1;
        }
    }
}

void moveLeft(block board[LEVEL_SIZE][LEVEL_SIZE],Player *p1){
    if (p1->pos.x < LEVEL_SIZE){
        if (board[p1->pos.y][p1->pos.x - 1] == FLOR){
            p1->pos.x -= 1;
        }
    }
}

void moveRight(block board[LEVEL_SIZE][LEVEL_SIZE],Player *p1){
    if (p1->pos.x < LEVEL_SIZE){
        if (board[p1->pos.y][p1->pos.x + 1] == FLOR){
            p1->pos.x += 1;
        }
    }
}

int main(){
    srand(time(NULL));
    InitWindow(500,500,"clicker");
    SetTargetFPS(60);
    Player p1;
    if(position_player(map2, &p1) < 0){
        printf("Could not position player\n");
        return -1;
    }
    while(!WindowShouldClose()){
        render(map2,&p1);
        if (IsKeyPressed('Q')){
            CloseWindow();
            break;
        }
        if (IsKeyPressed('W')){
            moveUp(map2,&p1);
        }
        if (IsKeyPressed('S')){
            moveDown(map2,&p1);
        }
        if (IsKeyPressed('A')){
            moveLeft(map2,&p1);
        }
        if (IsKeyPressed('D')){
            moveRight(map2,&p1);
        }
    }

    return 0;
}
