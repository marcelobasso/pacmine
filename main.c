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
#define PASSO 5
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

// define menu
#define NOJO_JOGO 'N'
#define CARREGAR_JOGO 'C'
#define SALVAR_JOGO 'S'
#define SAIR 'S'
#define VOLTAR 'V'

typedef struct Pos {
    int x, y;
} POS;

//Adicionei a struct Des para marcar o deslocamento

typedef struct Player {
    POS pos, posInicial, des;
    int vidas;
} PLAYER;

typedef struct Toupeira {
    POS pos, posInicial, des;
    int estado; // viva ou morta
    int id;
} TOUPEIRA;

typedef struct {
    char mapa[PATH_MAX], mapas[10][PATH_MAX];
    char opcao;
} JOGO;

int dentroDosLimites(POS posicao, POS deslocamento, int lim_x, int lim_y) {
    int dentro = 1;

    if (posicao.x == 0 && deslocamento.x == -1) {
        dentro = 0;
    }
    else if (posicao.y == 0 && deslocamento.y == -1) {
        dentro = 0;
    }
    else if (posicao.x == lim_x - ARESTA && deslocamento.x == 1){
        dentro = 0;
    }
    else if (posicao.y == lim_y - ARESTA && deslocamento.y == 1) {
        dentro = 0;
    }

    return dentro;
}

void move(POS *pos, POS des, int passo) {
    pos->x += des.x * passo;
    pos->y += des.y * passo;
}

//*********************************************************************************
//verifica se o player vai sair da tela
//ainda vai ser melhoras para nao poder passar por obstaculos
int podeMover(PLAYER player) {
    return dentroDosLimites(player.pos, player.des, LARGURA_MAPA, ALTURA_MAPA);    
}

//verifica se o inimigo vai sair dos limites da tela
//ainda vai ser melhoras para nao poder passar por obstaculos
int inimigoPodeMover(TOUPEIRA *toupeira, TOUPEIRA *toupeiras, int *qnt_toupeiras){
    int pode_mover = 1, rota_colisao = 0;
    int count = 0;

    // checa se a toupeira esta em rota de colisao com outra toupeira
    for (count = 0; count < *qnt_toupeiras && !rota_colisao; count++) {
        if (toupeiras[count].id != toupeira->id) {
            rota_colisao = CheckCollisionRecs((Rectangle) { toupeira->pos.x + toupeira->des.x, toupeira->pos.y + toupeira->des.y, ARESTA, ARESTA}, (Rectangle) { toupeiras[count].pos.x, toupeiras[count].pos.y, ARESTA, ARESTA }); 
        }
    }

    // checa se a toupeira esta nos limites do mapa e nao esta em rota de colisao
    pode_mover = dentroDosLimites(toupeira->pos, toupeira->des, LARGURA_MAPA, ALTURA_MAPA) && !rota_colisao; 

    // inverte movimento caso nao puder se deslocar na direcao desejada
    if (!pode_mover) {
        if (toupeira->des.x) toupeira->des.x *= -1;
        if (toupeira->des.y) toupeira->des.y *= -1;
    }

    return pode_mover;
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
                toupeiras[*qnt_toupeiras].id = *qnt_toupeiras;
                toupeiras[*qnt_toupeiras].des.x = 0;
                toupeiras[*qnt_toupeiras].des.y = 0;
                
                *qnt_toupeiras += 1;
            }
        }

    }
}

void iniciaJogador(PLAYER *player, char mapa[MAX_LINHAS][MAX_COLUNAS]) {
    int l, c;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            if (mapa[l][c] == 'J') {
                player->posInicial.x = c * ARESTA;
                player->posInicial.y = l * ARESTA;
                player->pos.x = player->posInicial.x;
                player->pos.y = player->posInicial.y;
                player->vidas = QTD_VIDAS;
                player->des.x = 0;
                player->des.y = 0;
            }
        }
    }
}

void desenhaMapa(int max_linhas, int max_colunas, PLAYER *player, TOUPEIRA *toupeiras, char mapa[MAX_LINHAS][MAX_COLUNAS], int *qnt_toupeiras) {
    Color cor_bloco;
    Rectangle bloco;
    int l, c, c_toup;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            cor_bloco = RAYWHITE;

            switch (mapa[l][c]) {
                case PAREDE_INDESTRUTIVEL:
                    cor_bloco = BLACK;
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
    for (c_toup = 0; c_toup < *qnt_toupeiras; c_toup++) {
        DrawRectangle(toupeiras[c_toup].pos.x, toupeiras[c_toup].pos.y, ARESTA, ARESTA, RED);
    }
}

void movimentaJogador(PLAYER *player) {
    player->des = (POS) { x: 0, y: 0 };

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        player->des.x = 1;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        player->des.x = -1;
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        player->des.y = -1;
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        player->des.y = 1;
    }

    if (podeMover(*player)) {
       move(&player->pos, player->des, PASSO);
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

int main() {
    srand(time(NULL));
    PLAYER player;
    TOUPEIRA toupeiras[MAX_TOUPEIRAS];
    char mapa[MAX_LINHAS][MAX_COLUNAS];
    int i, cont, encerra = 0;
    int qnt_toupeiras = 0;

    carregaMapa(mapa, 1);
    iniciaToupeiras(toupeiras, mapa, &qnt_toupeiras);
    iniciaJogador(&player, mapa);

    InitWindow(LARGURA_MAPA, ALTURA_MAPA, "PACMINE - UFRGS");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        //*********************************************************************************
        //movimento do jogador
        movimentaJogador(&player);

        // movimento das toupeiras
        for (i = 0; i < qnt_toupeiras; i++) {
            // define uma direcao a cada segundo
            if (cont == 0) {
                // reseta o deslocamento
                toupeiras[i].des = (POS) {x: 0, y: 0};

                toupeiras[i].des.x = GetRandomValue(-1, 1);
                if (toupeiras[i].des.x == 0) {
                    do {
                        toupeiras[i].des.y = GetRandomValue(-1, 1);
                    } while (toupeiras[i].des.y == 0);
                }
            }
        }

        cont++;
        if (cont == 60) {
            cont = 0;
        }

        // move as toupeiras
        for (i = 0; i < qnt_toupeiras; i++) {
            if (inimigoPodeMover(&toupeiras[i], toupeiras, &qnt_toupeiras)) {
                move(&toupeiras[i].pos, toupeiras[i].des, PASSO_TOUPEIRAS);
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
        desenhaMapa(MAX_LINHAS, MAX_COLUNAS, &player, toupeiras, mapa, &qnt_toupeiras);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
