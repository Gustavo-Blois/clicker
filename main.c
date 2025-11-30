#include <raylib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>

#include <fcntl.h>

#include <errno.h>


// STOVE: STOVE
// CTBD: cutting board
// FLOR: floor
// DLVR: deliver (onde entregamos os pedidos)
// TRSH: trash (descartar itens feitos por engano)
typedef enum block{
WALL,
BRED,
BBLUE,
BYELL,
FLOR,
DLVR,
TRSH
} block;

typedef struct pos_st {
    int x;
    int y;
} Pos;

typedef struct player_st{
    Pos pos;
    short int color;
} Player;

typedef struct customer_st{
    Pos pos;
} Customer;

#define LEVEL_SIZE_Y 12
#define LEVEL_SIZE_X 10
#define N_THREADS LEVEL_SIZE_X
#define screenHeight 500
#define screenWidth 500

#define N_ACTIVE_ORDERS 5

#define n_pedidos 10

#include "boards.h"
const char *game_over_message = "O JOGO ACABOU";
int GAME_OVER = 0;

float tempo_spawn_cliente = 4.0;
float tempo_espera_cliente = 15.0;


int n_pedidos_feitos = 0;
int n_erros = 0;
int score = 0;

typedef struct node {
	int v;
	struct node *next;
	pthread_t client_thread;
} order_list;


order_list* list_push_back(int e, order_list** l) {
	order_list* node = *l;
	if (!node) {
		*l = malloc(sizeof(order_list));
		(*l)->v = e;
		(*l)->next = NULL;

		return *l;
	}

	while (node->next)
		node = node->next;

	node->next = malloc(sizeof(order_list));
	if (node->next) {
		node->next->v = e;
		node->next->next = NULL;
	}

	return node->next;
}

void free_list(order_list** l) {
	order_list* node = *l;
	order_list* prev = NULL;

	while (node) {
		if (prev)
			free(prev);

		prev = node;
		node = node->next;
	}

	if (prev)
		free(prev);

	*l = NULL;
}

void list_remove(order_list* it, order_list** l) {
	order_list* node = *l;
	order_list* prev = NULL;
	if (!it)
		return;

	while (node) {
		if (node == it) {
			if (*l == node)
				*l = node->next;

			if (prev)
				prev->next = node->next;

			free(node);
			return;
		}

		prev = node;
		node = node->next;
	}
}


void render(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1, order_list **orders, sem_t* sem){

   // Vector2 offset = {screenWidth%LEVEL_SIZE_X,screenHeight%LEVEL_SIZE_Y};
    BeginDrawing();
    ClearBackground(GRAY);

    if (GAME_OVER == 1) {
        ClearBackground(RED);
        DrawText(game_over_message, screenWidth/2, screenHeight/2, 20, WHITE);
    } else{
    Vector2 sizeofsquare = {
        screenWidth/LEVEL_SIZE_X ,
        screenHeight/LEVEL_SIZE_Y ,
    };
for (int y = 0; y < LEVEL_SIZE_Y; y++) {
    for (int x = 0; x < LEVEL_SIZE_X; x++) {
        switch (board[y][x]) {
            case FLOR:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    (Color){50, 50, 50, 255}
                );
                break;
            case BYELL:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    YELLOW
                );
                break;
            case BRED:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    RED
                );
                break;
            case BBLUE:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    BLUE
                );
                break;
            case DLVR:
                DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    (Color){50, 255, 50, 255}
                );
				break;
			case TRSH:
				DrawRectangleV(
                    (Vector2){ sizeofsquare.x * x, sizeofsquare.y * y },
                    sizeofsquare,
                    (Color){0, 255, 255, 255}
                );
        }
    }
}
	sem_wait(sem);

	int index = 0;
    for(order_list *node = *orders; node; node = node->next) {
		switch(node->v) {
			case -1:
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare, LIGHTGRAY);
				break;
			case(0b001):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,BLUE);
				break;
			case(0b010):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,YELLOW);
				break;
			case(0b011):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,GREEN);
				break;
			case(0b100):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,RED);
				break;
			case(0b101):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,PURPLE);
				break;
			case(0b110):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,ORANGE);
				break;
			case(0b111):
				DrawRectangleV((Vector2){sizeofsquare.x*index,0},sizeofsquare,WHITE);
				break;
		}

		index += 1;
    }

	sem_post(sem);

    for (int i = 0; i < screenWidth/LEVEL_SIZE_X + 1; i++){
        DrawLineV((Vector2){i*sizeofsquare.x, 0}, (Vector2){i*sizeofsquare.x, screenHeight}, LIGHTGRAY);
    }

    for (int i = 0; i < screenHeight/LEVEL_SIZE_Y + 1; i++){
        DrawLineV((Vector2){0, i*sizeofsquare.y}, (Vector2){screenWidth, i*sizeofsquare.y}, LIGHTGRAY);
    }

    switch(p1->color){
        case(0b000):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,BLACK);
            break;
        case(0b001):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,BLUE);
            break;
        case(0b010):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,YELLOW);
            break;
        case(0b011):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,GREEN);
            break;
        case(0b100):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,RED);
            break;
        case(0b101):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,PURPLE);
            break;
        case(0b110):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,ORANGE);
            break;
        case(0b111):
            DrawRectangleV((Vector2){sizeofsquare.x*p1->pos.x,sizeofsquare.y*p1->pos.y},sizeofsquare,WHITE);
            break;
    }



}
    EndDrawing();

}

int init_player(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1){
    p1->color = 0b000;
    for(int i = 1; i < LEVEL_SIZE_Y; i++){
        for(int j = 0; j < LEVEL_SIZE_X; j++){
            if (board[i][j] == FLOR) {
                p1->pos.x = j;
                p1-> pos.y =i;
                return 0;
            }
        }
    }
    return -1;
}

void update_player_color(Player *p1, block color){
    switch(color){
        case(BRED):
            p1->color = (p1->color & 0b111) | 0b100;
            break;
        case(BYELL):
            p1->color = (p1->color & 0b111) | 0b010;
            break;
        case(BBLUE):
            p1->color = (p1->color & 0b111) | 0b001;
            break;
		case(TRSH):
			p1->color = 0b000;
			break;
    }
}


order_list* get_next_matching_color(order_list* orders, int color) {
	order_list* node = orders;
	while(node) {
		if (node->v == color)
			return node;
		node = node->next;
	}

	return NULL;
}


void deliver(Player *p1, sem_t* orders_buf_semaphore, order_list* orders) {
	if (p1->color == 0)
		return;

	sem_wait(orders_buf_semaphore);

	order_list* it = get_next_matching_color(orders, p1->color);
	if (it) {
		it->v = -1;
		p1->color = 0b000;
	}

	sem_post(orders_buf_semaphore);
}

void moveUp(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X], Player *p1, sem_t* sem, order_list* orders){
    if (p1->pos.y < LEVEL_SIZE_Y){
        if (board[p1->pos.y - 1][p1->pos.x] == FLOR){
            p1->pos.y -= 1;
        }
        else if (board[p1->pos.y - 1][p1->pos.x] == DLVR){
            deliver(p1, sem, orders);
        }
        else if (board[p1->pos.y - 1][p1->pos.x] != WALL){
            update_player_color(p1, board[p1->pos.y - 1][p1->pos.x]);
        }
    }
}

void moveDown(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1, sem_t* sem, order_list* orders){
    if (p1->pos.y > 0){
        if (board[p1->pos.y + 1][p1->pos.x] == FLOR){
            p1->pos.y += 1;
        }
        else if (board[p1->pos.y + 1][p1->pos.x] == DLVR){
            deliver(p1, sem, orders);
        }
        else if (board[p1->pos.y + 1][p1->pos.x] != WALL){
            update_player_color(p1, board[p1->pos.y + 1][p1->pos.x]);
        }
    }
}

void moveLeft(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1, sem_t* sem, order_list* orders){
    if (p1->pos.x > 0){
        if (board[p1->pos.y][p1->pos.x - 1] == FLOR){
            p1->pos.x -= 1;
        }
        else if (board[p1->pos.y][p1->pos.x -1] == DLVR){
            deliver(p1, sem, orders);
        }
        else if (board[p1->pos.y ][p1->pos.x-1] != WALL){
            update_player_color(p1, board[p1->pos.y][p1->pos.x -1]);
        }
    }
}

void moveRight(block board[LEVEL_SIZE_Y][LEVEL_SIZE_X],Player *p1, sem_t* sem, order_list* orders){
    if (p1->pos.x < LEVEL_SIZE_X){
        if (board[p1->pos.y][p1->pos.x + 1] == FLOR){
            p1->pos.x += 1;
        }
        else if (board[p1->pos.y][p1->pos.x +1] == DLVR){
            deliver(p1, sem, orders);
        }
        else if (board[p1->pos.y ][p1->pos.x+1] != WALL){
            update_player_color(p1, board[p1->pos.y][p1->pos.x +1]);
        }
    }
}

bool any_open_thread(order_list* orders){
    order_list* node = orders;
	int i = 0;
	while(node) {
		node = node->next;
		i++;
	}

	return i < N_ACTIVE_ORDERS;
}


void* consumidor(void* args);

typedef struct {
	sem_t* semaphore;
	order_list* iterator;
	order_list** active_orders;
} consumer_args;

typedef struct {
	sem_t* semaphore;
	pthread_t* active_customers;
	order_list** active_orders;
} producer_args;


void *produtor(void *args) {

	sem_t* sem_buf_orders = ((producer_args*) args)->semaphore;

	pthread_t* active_customers =
		((producer_args*) args)->active_customers;

	order_list** active_orders =
		((producer_args*) args)->active_orders;

	time_t initial_time = time(NULL);
	time_t current_time = time(NULL);

	// Cria clientes em tempos
	for (; n_pedidos_feitos < n_pedidos; ) {

		if (GAME_OVER)
			break;

		// exclusao mutua do buffer de pedidos
		sem_wait(sem_buf_orders);

		order_list* it = NULL;

		if (any_open_thread(*active_orders)) {

			int next_order = (rand() % 7) + 1;
			it = list_push_back(next_order, active_orders);
			
			consumer_args c_args = {
				.semaphore = sem_buf_orders,
				.active_orders = active_orders,
				.iterator = it
			};

			pthread_create(&it->client_thread, NULL, &consumidor, &c_args);
		}

		// todo: checa erro
		sem_post(sem_buf_orders);

		if (it)
			n_pedidos_feitos += 1;

		while (difftime(current_time, initial_time) < tempo_spawn_cliente)
			current_time = time(NULL);

		initial_time = current_time;
	}
}


void vai_embora(sem_t* buf_sem, order_list** active_orders, order_list* order_it, bool feliz) {

	int pontos_ganhos = 10;
	sem_wait(buf_sem);

	if (feliz)
		score += pontos_ganhos;
	else {
		score -= pontos_ganhos;

		n_erros += 1;
		if (n_erros == 2)
			GAME_OVER = 1;
	}

	list_remove(order_it, active_orders);

	sem_post(buf_sem);
}


void* consumidor(void* args) {

	sem_t* orders_buf_sem = ((consumer_args*) args)->semaphore;
	order_list** active_orders = ((consumer_args*) args)->active_orders;
	order_list* it = ((consumer_args*) args)->iterator;

	time_t initial_time = time(NULL);
	time_t current_time = time(NULL);

	printf("Cliente criado!\n");

	while(difftime(current_time, initial_time) < tempo_espera_cliente) {
		if (GAME_OVER) {
			list_remove(it, active_orders);
			return NULL;
		}

		if (it->v < 0) {
			vai_embora(orders_buf_sem, active_orders, it, true);

			printf("Cliente foi embora satisfeito :D\n");
			return NULL;
		}

		current_time = time(NULL);
	}

	vai_embora(orders_buf_sem, active_orders, it, false);
	printf("Cliente foi embora não satisfeito :(\n");

	return NULL;
}


int main(){
    srand(time(NULL));
    InitWindow(500,500,"clicker");
    SetTargetFPS(60);
    Player p1;
    if(init_player(map2, &p1) < 0){
        printf("Could not position player\n");
        return -1;
    }

    time_t initial_time = time(NULL);

	pthread_t prod;
	sem_t* orders_buf_sem;
	order_list* active_orders;
	pthread_t active_customers[N_ACTIVE_ORDERS];

	orders_buf_sem = sem_open("orderbuf", O_CREAT | O_EXCL, 0644, 1);
	if (orders_buf_sem == SEM_FAILED) {
		sem_unlink("orderbuf");
	}

	orders_buf_sem = sem_open("orderbuf", O_CREAT, 0644, 1);

	active_orders = NULL;

	producer_args prod_args = {
		.semaphore = orders_buf_sem,
		.active_orders = &active_orders,
		.active_customers = active_customers
	};

	pthread_create(&prod, NULL, &produtor, &prod_args);

    while(!WindowShouldClose()) {
        render(map2, &p1, &active_orders, orders_buf_sem);
        time_t current_time = time(NULL);

        
        if (IsKeyPressed('Q')) {
			sem_close(orders_buf_sem);
			sem_unlink("orderbuf");

			CloseWindow();
            break;
        }

		if (GAME_OVER){continue;}

        if (IsKeyPressed('W')){
            moveUp(map2,&p1, orders_buf_sem, active_orders);
        }
        if (IsKeyPressed('S')){
            moveDown(map2,&p1, orders_buf_sem, active_orders);
        }
        if (IsKeyPressed('A')){
            moveLeft(map2,&p1, orders_buf_sem, active_orders);
        }
        if (IsKeyPressed('D')){
            moveRight(map2,&p1, orders_buf_sem, active_orders);
        }
    }

    return 0;
}
