#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANHO_MAPA 600
#define MAX_PATH_SIZE 20

#define MAX_COLUNAS 30
#define MAX_LINHAS 20

#define ALTURA_MAPA 600
#define LARGURA_MAPA 900
#define TAMANHO_TILE 30
#define ALTURA_MENU 50
#define SCREEN_WIDTH LARGURA_MAPA
#define SCREEN_HEIGHT (ALTURA_MAPA + ALTURA_MENU)
#define MAP_SIZE_HEIGHT (ALTURA_MAPA / TAMANHO_TILE)
#define MAP_SIZE_WIDTH (LARGURA_MAPA / TAMANHO_TILE)

#define N_TOUPEIRAS 200

// define elementos do mapa
#define PAREDE_INDESTRUTIVEL '#'
#define POS_INICIAL 'J'
#define POS_INICIAL_TOUPEIRA 'T'
#define OURO 'O'
#define ESMERALDA 'E'
#define AREA_SOTERRADA 'S'
#define POWER_UP 'A'
#define LIVRE ' '

typedef struct Pos {
    int x, y;
} POS;

typedef struct Player {
    POS pos;
    int vidas;
} PLAYER;

typedef struct Toupeira {
    POS pos;
    int estado; // viva ou morta
} TOUPEIRA;

void desenhaMapa(int max_linhas, int max_colunas, PLAYER *player, TOUPEIRA *toupeiras, char mapa[MAX_LINHAS][MAX_COLUNAS]);
void carregaMapa(char mapa[MAX_LINHAS][MAX_COLUNAS], int fase);

int main() {
    PLAYER player;
    TOUPEIRA toupeiras[N_TOUPEIRAS];
    char mapa[MAX_LINHAS][MAX_COLUNAS];

    carregaMapa(mapa, 1);

    InitWindow(LARGURA_MAPA, ALTURA_MAPA, "PACMINE - UFRGS");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        BeginDrawing();

            ClearBackground(RAYWHITE);
            DrawText("PACMINE", 10, 10, 20, DARKGRAY);

            // inicia mapa
            desenhaMapa(MAX_LINHAS, MAX_COLUNAS, &player, toupeiras, mapa);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

void desenhaMapa(int max_linhas, int max_colunas, PLAYER *player, TOUPEIRA *toupeiras, char mapa[MAX_LINHAS][MAX_COLUNAS]) {
    Color cor_bloco;
    Rectangle bloco;
    int l, c;
    int toup_count = 0;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            cor_bloco = RAYWHITE;
            toup_count = 0;

            switch (mapa[l][c]) {
                case PAREDE_INDESTRUTIVEL:
                    cor_bloco = BLACK;
                    break;

                case POS_INICIAL:
                    player->pos.x = c * TAMANHO_TILE;
                    player->pos.y = l * TAMANHO_TILE;
                    player->vidas = 3;

                    cor_bloco = GREEN;
                    break;

                case POS_INICIAL_TOUPEIRA:
                    toupeiras[toup_count].pos.x = c * TAMANHO_TILE;
                    toupeiras[toup_count].pos.y = l * TAMANHO_TILE;
                    toup_count++;

                    cor_bloco = RED;
                    break;

                case OURO:
                    cor_bloco = GOLD;
                    break;

                case ESMERALDA:
                    cor_bloco = LIME;
                    break;
                
                case AREA_SOTERRADA:
                    cor_bloco = BROWN;
                    break;
                
                case POWER_UP:
                    cor_bloco = PINK;
                    break;
            }

            bloco = (Rectangle) { 
                x: c * TAMANHO_TILE, 
                y: l * TAMANHO_TILE,
                width: TAMANHO_TILE,
                height: TAMANHO_TILE
            };

            DrawRectangleRec(bloco, cor_bloco);
        }
    }

}

void carregaMapa(char mapa[MAX_LINHAS][MAX_COLUNAS], int fase) {
    char caminho_mapa[MAX_PATH_SIZE];
    char linha_mapa[TAMANHO_MAPA];
    FILE *arquivo_mapa;
    int linha = 0;

    sprintf(caminho_mapa, "./maps/mapa%d.txt", fase);
    arquivo_mapa = fopen(caminho_mapa, "r");

    if (NULL == arquivo_mapa) {
        printf("file can't be opened \n");
    }

    while (!feof(arquivo_mapa)) {
        fgets(linha_mapa, 32, arquivo_mapa); // ?
        strcpy(mapa[linha], linha_mapa);
        linha++;
    }


    fclose(arquivo_mapa);
}
