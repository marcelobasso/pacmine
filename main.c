#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAMANHO_MAPA 600
#define MAX_PATH_SIZE 20
#define MAX_COLUNAS 30
#define MAX_LINHAS 20
#define FONT_SIZE 20

#define ALTURA_MENU_SUPERIOR 50
#define ALTURA_MENU_INFERIOR 60
#define MAX_TOUPEIRAS 200
#define N_TIPOS_BLOCOS 8

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
    int vidas, pontos, esmeraldas_coletadas;
    char blocos_atravessaveis[N_TIPOS_BLOCOS];
} PLAYER;

typedef struct Toupeira {
    Vector2 pos, posInicial, des;
    int estado; // viva ou morta
    int id;
    char blocos_atravessaveis[N_TIPOS_BLOCOS];
} TOUPEIRA;

typedef struct {
    char mapa_atual[PATH_MAX], mapas[10][PATH_MAX];
    char mapa[MAX_LINHAS][MAX_COLUNAS];
    int altura_mapa, largura_mapa, aresta, nivel;
    int qnt_toupeiras, qnt_esmeraldas;
    char opcao;
    double toup_muda_movimento;
} JOGO;

int ValorNoArray(char valor, char array[], int t_array) {
    if (t_array < 0) return 0;
    return array[t_array] == valor || ValorNoArray(valor, array, t_array - 1);
}

// Função responsável pela movimentação do jogador e dos inimigos.
void move(Vector2 *pos, Vector2 des, int passo) {
    pos->x += des.x * passo;
    pos->y += des.y * passo;
}

// verifica se a entidade (player/toupeira) está dentro da tela e se ele não está colidindo com nenhum obstáculo
int podeMover(Vector2 pos, Vector2 des, JOGO jogo, char blocos_atravessaveis[]) {
    int linha, coluna;
    int linha_entidade, coluna_entidade, colisao = 0;
    int linha_checada, coluna_checada;

    linha_entidade = ((int) pos.y - ALTURA_MENU_SUPERIOR) / jogo.aresta;
    coluna_entidade = (int) pos.x / jogo.aresta;

    for (linha = -1; linha < 2 && !colisao; linha++) {
        for (coluna = -1; coluna < 2 && !colisao; coluna++) {

            linha_checada = linha_entidade + linha;
            coluna_checada = coluna_entidade + coluna;

            if (!ValorNoArray(jogo.mapa[linha_checada][coluna_checada], blocos_atravessaveis, N_TIPOS_BLOCOS)) {
                if (CheckCollisionRecs(
                    (Rectangle) { pos.x + des.x, pos.y + des.y, jogo.aresta, jogo.aresta },
                    (Rectangle) { coluna_checada * jogo.aresta, (linha_checada * jogo.aresta) + ALTURA_MENU_SUPERIOR, jogo.aresta, jogo.aresta })) {
                    colisao = 1;
                }
            }
        }
    }

    return !colisao;    
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
    pode_mover = podeMover(toupeira->pos, toupeira->des, jogo, toupeira->blocos_atravessaveis) && !rota_colisao; 

    // inverte movimento caso nao puder se deslocar na direcao desejada
    if (!pode_mover) {
        if (toupeira->des.x) toupeira->des.x *= -1;
        if (toupeira->des.y) toupeira->des.y *= -1;
    }

    return pode_mover;
}

// coloca o player e os inimigos vivos na posicao inicial
void resetPosicoes(PLAYER *player, TOUPEIRA *toupeiras, int qnt_toupeiras) {
    int i = 0;

    for (i = 0; i < qnt_toupeiras; i++) {
        toupeiras[i].pos.x = toupeiras[i].posInicial.x;
        toupeiras[i].pos.y = toupeiras[i].posInicial.y;
    }
    player->pos.x = player->posInicial.x;
    player->pos.y = player->posInicial.y;
}

// Inicia o vetor de toupeiras
void iniciaToupeiras(TOUPEIRA *toupeiras, char mapa[MAX_LINHAS][MAX_COLUNAS], JOGO *jogo) {
    int l, c, pos_vetor;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            pos_vetor = jogo->qnt_toupeiras;

            if (mapa[l][c] == 'T') {
                toupeiras[pos_vetor].posInicial.x = c * jogo->aresta;
                toupeiras[pos_vetor].posInicial.y = l * jogo->aresta;
                toupeiras[pos_vetor].pos.x = c * jogo->aresta;
                toupeiras[pos_vetor].pos.y = l * jogo->aresta + ALTURA_MENU_SUPERIOR;
                toupeiras[pos_vetor].id = pos_vetor;
                sprintf(toupeiras[pos_vetor].blocos_atravessaveis, "%c%c%c%c%c%c%c", LIVRE, AREA_SOTERRADA, POS_INICIAL_TOUPEIRA, POS_INICIAL, OURO, ESMERALDA, POWER_UP);

                // define uma posição inicial para as toupeiras seguirem
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

// Inicia as informações do jogador
void iniciaJogador(PLAYER *player, char mapa[MAX_LINHAS][MAX_COLUNAS]) {
    int l, c;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            if (mapa[l][c] == 'J') {
                player->posInicial.x = c * ARESTA;
                player->posInicial.y = l * ARESTA + ALTURA_MENU_SUPERIOR;
                player->pos.x = player->posInicial.x;
                player->pos.y = player->posInicial.y;
                player->vidas = QTD_VIDAS;
                player->des.x = 0;
                player->des.y = 0;
                player->pontos = 0;
                player->esmeraldas_coletadas = 0;
                sprintf(player->blocos_atravessaveis, "%c%c", LIVRE, POS_INICIAL);
            }
        }
    }
}

// funcao responsável por desenhar todos os elementos do jogo (fora os menus e textos)
void desenhaMapa(int max_linhas, int max_colunas, PLAYER *player, TOUPEIRA *toupeiras, JOGO jogo) {
    Color cor_bloco;
    Rectangle bloco;
    int l, c, c_toup, x, y, width, height;

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            cor_bloco = RAYWHITE;
            x = c * ARESTA;
            y = (l * ARESTA) + ALTURA_MENU_SUPERIOR;
            width = jogo.aresta;
            height = jogo.aresta;

            switch (jogo.mapa[l][c]) {
                case PAREDE_INDESTRUTIVEL:
                    cor_bloco = BLACK;
                    break;

                case OURO:
                    cor_bloco = GOLD;
                    break;

                case ESMERALDA:
                    cor_bloco = (Color) { 10, 228, 100, 122 };
                    break;

                case AREA_SOTERRADA:
                    cor_bloco = (Color) { 120, 100, 80, 255 };
                    break;

                case POWER_UP:
                    x += 7.5;
                    y += 7.5;
                    width = 15;
                    height = 15;
                    cor_bloco = (Color) { 255, 60, 180, 255 };
                    break;
            }

            bloco = (Rectangle) { x, y, width, height };
            DrawRectangleRec(bloco, cor_bloco);
        }
    }

    // desenha jogador
    DrawRectangle(player->pos.x, player->pos.y, ARESTA, ARESTA, BLUE);

    // desenha toupeiras
    for (c_toup = 0; c_toup < jogo.qnt_toupeiras; c_toup++) {
        DrawRectangle(toupeiras[c_toup].pos.x, toupeiras[c_toup].pos.y, ARESTA, ARESTA, RED);
    }
}

// Funcao responsável por realizar movimentação do jogador
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

    if (player->des.y || player->des.x) {
        if (podeMover(player->pos, player->des, jogo, player->blocos_atravessaveis)) {
            move(&player->pos, player->des, passo);
        }
    }

    // tira a vida do jogador caso ele encostar em uma toupeira
    for (i = 0; i < jogo.qnt_toupeiras; i++) {
        if (CheckCollisionRecs((Rectangle) { player->pos.x, player->pos.y, ARESTA, ARESTA }, (Rectangle) {toupeiras[i].pos.x, toupeiras[i].pos.y, ARESTA, ARESTA})) {
            resetPosicoes(player, toupeiras, jogo.qnt_toupeiras);
            player->vidas--;
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

void carregaMapa(JOGO *jogo) {
    char caminho_mapa[MAX_PATH_SIZE];
    FILE *arquivo_mapa;
    int linha = 0;
    int coluna = 0;
    char c;

    sprintf(caminho_mapa, "./maps/mapa%d.txt", jogo->nivel);
    arquivo_mapa = fopen(caminho_mapa, "r");

    if (NULL == arquivo_mapa) {
        printf("file can't be opened \n");
    } else {
        while (!feof(arquivo_mapa)) {
            c = getc(arquivo_mapa);

            if (c == '\n') {
                coluna = 0;
                linha++;
            } else if (c != -1) {
                jogo->mapa[linha][coluna] = c;
                coluna++;

                if (c == ESMERALDA) {
                   jogo->qnt_esmeraldas += 1;
                }
            }

        }
    }

    fclose(arquivo_mapa);
}

void iniciaJogo(JOGO *jogo) {
    jogo->altura_mapa = MAX_LINHAS * ARESTA;
    jogo->largura_mapa = MAX_COLUNAS * ARESTA;
    jogo->aresta = ARESTA;
    jogo->qnt_toupeiras = 0;
    jogo->qnt_esmeraldas = 0;
    jogo->toup_muda_movimento = GetTime();
    jogo->nivel = 1;
}

void desenhaTextos(JOGO jogo, PLAYER player) {
    char title[50] = { "PACMINE - UFRGS" };
    DrawRectangle(0, 0, jogo.largura_mapa, ALTURA_MENU_SUPERIOR, GRAY);
    DrawRectangle(0, (jogo.altura_mapa + ALTURA_MENU_SUPERIOR), jogo.largura_mapa, ALTURA_MENU_INFERIOR, GRAY);
    DrawText(title, (jogo.largura_mapa - MeasureText(title, FONT_SIZE)) / 2, 5, FONT_SIZE, BLACK);

    // desenha informações do jogo
    char vidas[30], pontuacao[20], nivel[20], esmeraldas_coletadas[30], esmeraldas_totais[30];
    sprintf(nivel, "Nível: %d", jogo.nivel); 
    sprintf(pontuacao, "Pontos: %d", player.pontos);
    sprintf(vidas, "Vidas restantes: %d", player.vidas);
    sprintf(esmeraldas_totais, "Esmeraldas totais: %d", jogo.qnt_esmeraldas);
    sprintf(esmeraldas_coletadas, "Esmeraldas coletadas: %d", player.esmeraldas_coletadas);

    DrawText(nivel, (jogo.largura_mapa - MeasureText(nivel, 16)) / 2, 30, 16, BLACK);
    DrawText(vidas, 20, ALTURA_MENU_SUPERIOR + jogo.altura_mapa + 7.5, FONT_SIZE, BLACK);
    DrawText(pontuacao, 20, ALTURA_MENU_SUPERIOR + jogo.altura_mapa + 10 + FONT_SIZE, FONT_SIZE, BLACK);
    DrawText(esmeraldas_coletadas, jogo.largura_mapa / 2, ALTURA_MENU_SUPERIOR + jogo.altura_mapa + 7.5, FONT_SIZE, BLACK);
    DrawText(esmeraldas_totais, jogo.largura_mapa / 2, ALTURA_MENU_SUPERIOR + jogo.altura_mapa + 10 + FONT_SIZE, FONT_SIZE, BLACK);
    
}

int main() {
    JOGO jogo;
    PLAYER player;
    TOUPEIRA toupeiras[MAX_TOUPEIRAS];

    iniciaJogo(&jogo);
    carregaMapa(&jogo);

    iniciaToupeiras(toupeiras, jogo.mapa, &jogo);
    iniciaJogador(&player, jogo.mapa);

    InitWindow(jogo.largura_mapa, jogo.altura_mapa + (ALTURA_MENU_INFERIOR + ALTURA_MENU_SUPERIOR), "PACMINE - UFRGS");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        desenhaTextos(jogo, player);

        //movimento do jogador
        movimentaJogador(&player, PASSO, toupeiras, jogo);
        movimentaToupeiras(toupeiras, PASSO_TOUPEIRAS, &jogo);

        // inicia mapa
        desenhaMapa(MAX_LINHAS, MAX_COLUNAS, &player, toupeiras, jogo);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
