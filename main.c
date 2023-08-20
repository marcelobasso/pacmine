#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAMANHO_MAPA 600
#define MAX_PATH_SIZE 20

#define MAX_COLUNAS 30
#define MAX_LINHAS 20

#define ALTURA_MENU 50
#define MAX_TOUPEIRAS 200

#define ARESTA 30
#define PASSO 5
#define PASSO_TOUPEIRAS 2
#define QTD_VIDAS 3

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

//Adicionei a struct Des para marcar o deslocamento

typedef struct Player {
    Vector2 pos, posInicial, des;
    int vidas;
} PLAYER;

typedef struct Toupeira {
    Vector2 pos, posInicial, des;
    int estado; // viva ou morta
    int id;
} TOUPEIRA;

typedef struct {
    char mapa[PATH_MAX], mapas[10][PATH_MAX];
    int altura_mapa, largura_mapa, aresta;
    int qnt_toupeiras;
    char opcao;
    double toup_muda_movimento;
} JOGO;

// Função que verifica se a posicao atual + o deslocamento de um corpo ainda o manterá dentro dos limites da tela.
int dentroDosLimites(Vector2 posicao, Vector2 deslocamento, int lim_x, int lim_y, int aresta) {
    int dentro = 1;

    if (posicao.x == 0 && deslocamento.x == -1) {
        dentro = 0;
    }
    else if (posicao.y == 0 && deslocamento.y == -1) {
        dentro = 0;
    }
    else if (posicao.x == lim_x - aresta && deslocamento.x == 1){
        dentro = 0;
    }
    else if (posicao.y == lim_y - aresta && deslocamento.y == 1) {
        dentro = 0;
    }

    return dentro;
}


// Função responsável pela movimentação do jogador e dos inimigos.
void move(Vector2 *pos, Vector2 des, int passo) {
    pos->x += des.x * passo;
    pos->y += des.y * passo;
}

//*********************************************************************************
//verifica se o player vai sair da tela
//ainda vai ser melhoras para nao poder passar por obstaculos
int podeMover(PLAYER player, JOGO jogo) {
    return dentroDosLimites(player.pos, player.des, jogo.largura_mapa, jogo.altura_mapa, ARESTA);    
}

//verifica se o inimigo vai sair dos limites da tela
//ainda vai ser melhoras para nao poder passar por obstaculos
int inimigoPodeMover(TOUPEIRA *toupeira, TOUPEIRA *toupeiras, JOGO jogo){
    int pode_mover = 1, rota_colisao = 0;
    int count = 0;

    // checa se a toupeira esta em rota de colisao com outra toupeira
    for (count = 0; count < jogo.qnt_toupeiras && !rota_colisao; count++) {
        if (toupeiras[count].id != toupeira->id) {
            rota_colisao = CheckCollisionRecs((Rectangle) { toupeira->pos.x + toupeira->des.x, toupeira->pos.y + toupeira->des.y, ARESTA, ARESTA}, (Rectangle) { toupeiras[count].pos.x, toupeiras[count].pos.y, ARESTA, ARESTA }); 
        }
    }

    // checa se a toupeira esta nos limites do mapa e nao esta em rota de colisao
    pode_mover = dentroDosLimites(toupeira->pos, toupeira->des, jogo.largura_mapa, jogo.altura_mapa, ARESTA) && !rota_colisao; 

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

void iniciaToupeiras(TOUPEIRA *toupeiras, char mapa[MAX_LINHAS][MAX_COLUNAS], JOGO *jogo) {
    int l, c, pos_vetor;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            pos_vetor = jogo->qnt_toupeiras;

            if (mapa[l][c] == 'T') {
                toupeiras[pos_vetor].posInicial.x = c * ARESTA;
                toupeiras[pos_vetor].posInicial.y = l * ARESTA;
                toupeiras[pos_vetor].pos.x = c * ARESTA;
                toupeiras[pos_vetor].pos.y = l * ARESTA;
                toupeiras[pos_vetor].id = pos_vetor;


                toupeiras[pos_vetor].des.x = GetRandomValue(-1, 1);
                if (toupeiras[pos_vetor].des.x == 0) {
                    do {
                        toupeiras[pos_vetor].des.y = GetRandomValue(-1, 1);
                    } while (toupeiras[pos_vetor].des.y == 0);
                }

                jogo->qnt_toupeiras += 1;
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

void movimentaJogador(PLAYER *player, int passo, TOUPEIRA *toupeiras, JOGO jogo) {
    int i;
    player->des = (Vector2) { 0, 0 };

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

    if (podeMover(*player, jogo)) {
       move(&player->pos, player->des, passo);
    }

    // tira a vida do jogador caso ele encostar em uma toupeira
    for (i = 0; i < jogo.qnt_toupeiras; i++) {
        if (CheckCollisionRecs((Rectangle) { player->pos.x, player->pos.y, ARESTA, ARESTA }, (Rectangle) {toupeiras[i].pos.x, toupeiras[i].pos.y, ARESTA, ARESTA})) {
            resetPosicoes(player, toupeiras, jogo.qnt_toupeiras);
            player->vidas--;

            if (player->vidas == 0) {

            }
        }
    }
}

void movimentaToupeiras(TOUPEIRA *toupeiras, int passo_toupeiras, JOGO *jogo) {
    int i;
    double diff_time;

    diff_time = GetTime() - jogo->toup_muda_movimento;

    // movimento das toupeiras
    // define uma direcao a cada segundo
    if (diff_time >= 1) { // pq q funciona com 1?
        for (i = 0; i < jogo->qnt_toupeiras; i++) {
            // reseta o deslocamento
            toupeiras[i].des = (Vector2) { 0, 0 };
            toupeiras[i].des.x = GetRandomValue(-1, 1);

            if (toupeiras[i].des.x == 0) {
                do {
                    toupeiras[i].des.y = GetRandomValue(-1, 1);
                } while (toupeiras[i].des.y == 0);
            }
        }

        jogo->toup_muda_movimento = GetTime();
    }

    // move as toupeiras
    for (i = 0; i < jogo->qnt_toupeiras; i++) {
        if (inimigoPodeMover(&toupeiras[i], toupeiras, *jogo)) {
            move(&toupeiras[i].pos, toupeiras[i].des, PASSO_TOUPEIRAS);
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

void iniciaJogo(JOGO *jogo) {
    jogo->altura_mapa = MAX_LINHAS * ARESTA;
    jogo->largura_mapa = MAX_COLUNAS * ARESTA;
    jogo->qnt_toupeiras = 0;
    jogo->toup_muda_movimento = GetTime();
}

int main() {
    JOGO jogo;
    PLAYER player;
    TOUPEIRA toupeiras[MAX_TOUPEIRAS];
    char mapa[MAX_LINHAS][MAX_COLUNAS];
    int encerra;
    int qnt_toupeiras = 0;

    iniciaJogo(&jogo);

    carregaMapa(mapa, 1);
    iniciaToupeiras(toupeiras, mapa, &jogo);
    iniciaJogador(&player, mapa);

    qnt_toupeiras = jogo.qnt_toupeiras;

    InitWindow(jogo.largura_mapa, jogo.altura_mapa, "PACMINE - UFRGS");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        //movimento do jogador
        movimentaJogador(&player, PASSO, toupeiras, jogo);
        movimentaToupeiras(toupeiras, PASSO_TOUPEIRAS, &jogo);

        // inicia mapa
        desenhaMapa(MAX_LINHAS, MAX_COLUNAS, &player, toupeiras, mapa, &qnt_toupeiras);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
