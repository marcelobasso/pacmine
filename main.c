#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAMANHO_MAPA 600
#define MAX_PATH_SIZE 20

#define MAX_COLUNAS 30
#define MAX_LINHAS 20

#define ALTURA_MAPA 600
#define LARGURA_MAPA 900
#define ALTURA_MENU 50
#define SCREEN_WIDTH LARGURA_MAPA
#define SCREEN_HEIGHT (ALTURA_MAPA + ALTURA_MENU)
#define MAP_SIZE_HEIGHT (ALTURA_MAPA / ARESTA)
#define MAP_SIZE_WIDTH (LARGURA_MAPA / ARESTA)

#define MAX_TOUPEIRAS 200
//***
#define ARESTA 30
#define PASSO 4
#define PASSO_TOUPEIRAS 2 
#define QTD_VIDAS 3
//***
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

//Adicionei a struct Des para marcar o deslocamento

typedef struct Player {
    POS pos, posInicial;
    int vidas;
} PLAYER;

typedef struct Toupeira {
    POS pos, posInicial, des;
    int estado; // viva ou morta
} TOUPEIRA;

void desenhaMapa(int max_linhas, int max_colunas, PLAYER *player, TOUPEIRA *toupeiras, char mapa[MAX_LINHAS][MAX_COLUNAS], int primeira_execucao, int *qnt_toupeiras);
void carregaMapa(char mapa[MAX_LINHAS][MAX_COLUNAS], int fase);

//*********************************************************************************
//verifica se o player vai sair da tela 
//ainda vai ser melhoras para nao poder passar por obstaculos
int podeMover(PLAYER *player, int desX, int desY) {
    int i;

    if (player->pos.x == 0 && desX == -1) {
        i = 0;
    }
    else if(player->pos.y==0 && desY ==-1){
        i = 0;
    }
    else if(player->pos.x==LARGURA_MAPA-ARESTA && desX ==1){
        i = 0;
    }
    else if(player->pos.y== ALTURA_MAPA-ARESTA && desY ==1){
        i = 0;
    }
    else{
        i = 1;
    }
    return i;
}
//move o player
void move(PLAYER *player, int desX, int desY){
    player->pos.x = player->pos.x + desX * PASSO;
    player->pos.y = player->pos.y + desY * PASSO;
}

//verifica se o inimigo vai sair dos limites da tela
//ainda vai ser melhoras para nao poder passar por obstaculos
int inimigoPodeMover(TOUPEIRA *toupeira, TOUPEIRA *toupeiras, int *qnt_toupeiras){
    int pode_mover = 0, colisao = 0;
    int count = 0;

    // for (count = 0; count < *qnt_toupeiras && !colisao; count++) {
    //     if (CheckCollisionRecs((Rectangle) { toupeira->pos.x, toupeira->pos.y, ARESTA, ARESTA}, (Rectangle) { toupeiras[count].pos.x, toupeiras[count].pos.y, ARESTA, ARESTA })) {
    //         colisao = 1;
    //     }
    // }

    if (toupeira->pos.x + toupeira->des.x > 0 && toupeira->pos.y + toupeira->des.y > 0) {
        if (toupeira->pos.x + toupeira->des.x < LARGURA_MAPA - ARESTA && toupeira->pos.y + toupeira->des.y < ALTURA_MAPA - ARESTA) {
            pode_mover = 1;
        }
    }

    // inverte movimento
    if (!pode_mover || colisao) {
        if (toupeira->pos.x + toupeira->des.x > 0 || toupeira->pos.x + toupeira->des.x < LARGURA_MAPA - ARESTA) {
            toupeira->des.x *= -1;
        } 
        
        if (toupeira->pos.y + toupeira->des.y > 0 || toupeira->pos.y + toupeira->des.y < ALTURA_MAPA - ARESTA) {
            toupeira->des.y *= -1;
        }
    }

    return pode_mover;
}
//move as toupeiras
void inimigoMove(TOUPEIRA *toupeira){
    toupeira->pos.x += toupeira->des.x * PASSO_TOUPEIRAS;
    toupeira->pos.y += toupeira->des.y * PASSO_TOUPEIRAS;
}

//*********************************************************************************
int main() {
    srand(time(NULL));
    PLAYER player;
    TOUPEIRA toupeiras[MAX_TOUPEIRAS];
    char mapa[MAX_LINHAS][MAX_COLUNAS];
    int i, cont, encerra = 0, jogo_iniciado = 0;
    int qnt_toupeiras = 0;

    carregaMapa(mapa, 1);
    iniciaToupeiras(toupeiras, mapa, &qnt_toupeiras);    

    InitWindow(LARGURA_MAPA, ALTURA_MAPA, "PACMINE - UFRGS");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(RAYWHITE);
        
        //*********************************************************************************
        //movimento do jogador
        if (IsKeyDown(KEY_RIGHT)) {
            if (podeMover(&player, 1, 0) == 1) {
                move(&player, 1, 0);
            }
        }

        if (IsKeyDown(KEY_LEFT)) {
            if (podeMover(&player, -1, 0) == 1) {
                move(&player, -1, 0);
            }
        }

        if (IsKeyDown(KEY_UP)) {
            if (podeMover(&player, 0, -1) == 1) {
                move(&player, 0, -1);
            }
        }

        if (IsKeyDown(KEY_DOWN)) {
            if (podeMover(&player, 0, 1) == 1) {
                move(&player, 0, 1);
            }
        }
        
        // movimento das toupeiras
        for (i = 0; i < qnt_toupeiras; i++) {
            if (cont < 1) {
                toupeiras[i].des.x = rand() % 3 - 1;
                if (toupeiras[i].des.x == 0) {
                    do {
                        toupeiras[i].des.y = rand() % 3 - 1;
                    } while(toupeiras[i].des.y == 0);
                }

                cont++;
            }

            cont++;

            if (cont == 60) {
                cont = 0;
            }
        }

        // move as toupeiras
        for (i = 0; i < qnt_toupeiras; i++) {
            if (inimigoPodeMover(&toupeiras[i], toupeiras, &qnt_toupeiras)) {
                inimigoMove(&toupeiras[i]);
            }
        }
        
        //tira a vida do jogador caso ele encostar em uma toupeira
        for (i = 0; i < qnt_toupeiras; i++) {
            if (CheckCollisionRecs((Rectangle) { player.pos.x, player.pos.y, ARESTA, ARESTA }, (Rectangle) {toupeiras[i].pos.x, toupeiras[i].pos.y, ARESTA, ARESTA})) {
                resetPosicoes(&player, toupeiras, qnt_toupeiras);
                player.vidas--;

                if (player.vidas == 0) {
                    encerra = 1;
                }
            }
        }
        //*********************************************************************************

        // inicia mapa
        desenhaMapa(MAX_LINHAS, MAX_COLUNAS, &player, toupeiras, mapa, jogo_iniciado, &qnt_toupeiras);
        jogo_iniciado = 1;

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

void resetPosicoes(PLAYER *player, TOUPEIRA *toupeiras, int qnt_toupeiras) {
    int i = 0;

    for (i = 0; i < qnt_toupeiras; i++) {
        toupeiras[i].pos.x = toupeiras[i].posInicial.x;
        toupeiras[i].pos.y = toupeiras[i].posInicial.y;
    }
    player->pos.x = player->posInicial.x;
    player->pos.y = player->posInicial.y;
}

void iniciaToupeiras(TOUPEIRA *toupeiras, char mapa[MAX_LINHAS][MAX_COLUNAS], int *qnt_toupeiras) {
    int l, c;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            if (mapa[l][c] == 'T') {
                toupeiras[*qnt_toupeiras].posInicial.x = c * ARESTA;
                toupeiras[*qnt_toupeiras].posInicial.y = l * ARESTA; 
                toupeiras[*qnt_toupeiras].pos.x = c * ARESTA;
                toupeiras[*qnt_toupeiras].pos.y = l * ARESTA;
                *qnt_toupeiras += 1;
            }
        }

    }
}

void desenhaMapa(int max_linhas, int max_colunas, PLAYER *player, TOUPEIRA *toupeiras, char mapa[MAX_LINHAS][MAX_COLUNAS], int jogo_iniciado, int *qnt_toupeiras) {
    Color cor_bloco;
    Rectangle bloco;
    int l, c;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {

            cor_bloco = RAYWHITE;

            switch (mapa[l][c]) {
                case PAREDE_INDESTRUTIVEL:
                    cor_bloco = BLACK;
                    break;

                case POS_INICIAL:
                    if (!jogo_iniciado) {
                        player->posInicial.x = c * ARESTA;
                        player->posInicial.y = l * ARESTA;
                        player->pos.x = player->posInicial.x;
                        player->pos.y = player->posInicial.y;
                        player->vidas = QTD_VIDAS;
                    }

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
                x: c * ARESTA, 
                y: l * ARESTA,
                width: ARESTA,
                height: ARESTA
            };

            DrawRectangleRec(bloco, cor_bloco);

        }
    }

    // desenha jogador
    DrawRectangle(player->pos.x, player->pos.y, ARESTA, ARESTA, GREEN);

    // desenha toupeiras
    for (l = 0; l < *qnt_toupeiras; l++) {
        DrawRectangle(toupeiras[l].pos.x, toupeiras[l].pos.y, ARESTA, ARESTA, RED);
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