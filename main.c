#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#define TAMANHO_MAPA 600
#define MAX_PATH_SIZE 60
#define MAX_MAPS 100
#define MAX_COLUNAS 30
#define MAX_LINHAS 20
#define FONT_SIZE 20
#define FONT_SIZE_MENU 60

#define ALTURA_MENU_SUPERIOR 50
#define ALTURA_MENU_INFERIOR 60
#define N_BLOCOS_ESPECIAIS 3
#define N_TIPOS_BLOCOS 8
#define MAX_TOUPEIRAS 200
#define QTD_VIDAS 3

// define variáveis visuais do jogo
#define ARESTA 30
#define PASSO 3
#define PASSO_TOUPEIRAS 1

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
#define NOVO_JOGO "Novo jogo"
#define OP_NOVO_JOGO 'N'
#define CARREGAR_JOGO "Carregar jogo"
#define OP_CARREGAR_JOGO 'C'
#define SALVAR_JOGO "Salvar jogo"
#define OP_SALVAR_JOGO 'S'
#define SAIR "Sair"
#define OP_SAIR 'E'
#define VOLTAR "Voltar"
#define OP_VOLTAR 'V'

// define estados do jogo
#define JOGANDO 'J'
#define MENU 'M'
#define GANHOU 'G'
#define PERDEU 'P'

typedef struct{
    Vector2 pos, des;
    int estado, tamanho;
    char blocos_intransponiveis[N_TIPOS_BLOCOS];
} TIRO;

typedef struct {
    Vector2 pos, posInicial, des;
    int vidas, pontos, esmeraldas_coletadas, power_up;
    double time_power_up;
    char blocos_intransponiveis[N_TIPOS_BLOCOS];
    TIRO tiro;
} PLAYER;

typedef struct {
    Vector2 pos, posInicial, des;
    int estado; // viva ou morta
    int id;
    char blocos_intransponiveis[N_TIPOS_BLOCOS];
} TOUPEIRA;


typedef struct {
    char mapas[MAX_MAPS][MAX_PATH_SIZE];
    char mapa[MAX_LINHAS][MAX_COLUNAS];
    int altura_mapa, largura_mapa, aresta, nivel, qnt_mapas;
    int qnt_toupeiras, qnt_esmeraldas;
    char estado, blocos_especiais[N_BLOCOS_ESPECIAIS];
    double toup_muda_movimento;
} JOGO;

int ValorNoArray(char valor, char array[], int t_array) {
    if (t_array < 0) return 0;
    return array[t_array] == valor || ValorNoArray(valor, array, t_array - 1);
}

// desenha um fundo com transparencia para ofuscar o jogo
void ofuscaJogo(JOGO *jogo) {
    DrawRectangle(0, 0, jogo->largura_mapa, jogo->altura_mapa + ALTURA_MENU_INFERIOR + ALTURA_MENU_SUPERIOR, (Color) { 0, 0, 0, 180});
}

// Função responsável pela movimentação do jogador e dos inimigos.
void move(Vector2 *pos, Vector2 des, int passo) {
    pos->x += des.x * passo;
    pos->y += des.y * passo;
}

// verifica se a entidade (player/toupeira) não está colidindo com nenhum obstáculo que a interrompa.
int podeMover(Vector2 pos, Vector2 des, JOGO jogo, char blocos_intransponiveis[], int tamanho) {
    int linha, coluna;
    int linha_entidade, coluna_entidade, colisao = 0;
    int linha_checada, coluna_checada;

    linha_entidade = ((int) pos.y - ALTURA_MENU_SUPERIOR) / jogo.aresta;
    coluna_entidade = (int) pos.x / jogo.aresta;

    for (linha = -1; linha < 2 && !colisao; linha++) {
        for (coluna = -1; coluna < 2 && !colisao; coluna++) {

            linha_checada = linha_entidade + linha;
            coluna_checada = coluna_entidade + coluna;

            if (ValorNoArray(jogo.mapa[linha_checada][coluna_checada], blocos_intransponiveis, N_TIPOS_BLOCOS)) {
                if (CheckCollisionRecs(
                    (Rectangle) { pos.x + des.x, pos.y + des.y, tamanho, tamanho },
                    (Rectangle) { coluna_checada * jogo.aresta, (linha_checada * jogo.aresta) + ALTURA_MENU_SUPERIOR, jogo.aresta, jogo.aresta })) {
                    colisao = 1;
                }
            }
        }
    }

    return !colisao;    
}

//verifica se o inimigo pode mover e se ele não está colidindo com outra toupeira
int inimigoPodeMover(TOUPEIRA *toupeira, TOUPEIRA *toupeiras, JOGO jogo){
    int rota_colisao = 0, count = 0;

    // checa se a toupeira esta em rota de colisao com outra toupeira
    for (count = 0; count < jogo.qnt_toupeiras && !rota_colisao; count++) {
        if (toupeiras[count].id != toupeira->id) {
            rota_colisao = CheckCollisionRecs((Rectangle) { toupeira->pos.x + toupeira->des.x, toupeira->pos.y + toupeira->des.y, ARESTA, ARESTA}, (Rectangle) { toupeiras[count].pos.x, toupeiras[count].pos.y, ARESTA, ARESTA }); 
        }
    }

    // checa se a toupeira esta pode mover e nao esta em rota de colisao com outra toupeira
    return podeMover(toupeira->pos, toupeira->des, jogo, toupeira->blocos_intransponiveis, jogo.aresta) && !rota_colisao; 
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
                toupeiras[pos_vetor].estado = 1;
                toupeiras[pos_vetor].posInicial.x = c * jogo->aresta;
                toupeiras[pos_vetor].posInicial.y = l * jogo->aresta + ALTURA_MENU_SUPERIOR;
                toupeiras[pos_vetor].pos.x = c * jogo->aresta;
                toupeiras[pos_vetor].pos.y = l * jogo->aresta + ALTURA_MENU_SUPERIOR;
                toupeiras[pos_vetor].id = pos_vetor;
                                sprintf(toupeiras[pos_vetor].blocos_intransponiveis, "%c", PAREDE_INDESTRUTIVEL);

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
                player->power_up = 0;
                player->tiro.des.x = 1;
                player->tiro.des.y = 0;
                player->tiro.pos.x = -50;
                player->tiro.pos.y = -50;
                player->tiro.estado = 0;
                player->tiro.tamanho = 15;
                sprintf(player->blocos_intransponiveis, "%c%c", PAREDE_INDESTRUTIVEL, AREA_SOTERRADA);
                sprintf(player->tiro.blocos_intransponiveis, "%c%c%c", PAREDE_INDESTRUTIVEL, ESMERALDA, OURO);
            }
        }
    }
}

// checa se um bloco esta dentro do coampo de visao do jogador (9x9)
// Corrigir bug de visao de toupeiras no canto inferior direito;
int campoDeVisao(PLAYER player, int x, int y) {
    // return 1;
    int linha, coluna, visivel = 0;
    int linha_checada, coluna_checada, linha_player, coluna_player;

    linha_player = ((int) player.pos.y - ALTURA_MENU_SUPERIOR) / ARESTA;
    coluna_player = (int) player.pos.x / ARESTA;

    for (linha = -2; linha < 4 && !visivel; linha++) {
        for (coluna = -2; coluna < 4 && !visivel; coluna++) {
            linha_checada = linha_player + linha;
            coluna_checada = coluna_player + coluna;

            if (CheckCollisionRecs(
                (Rectangle) { x, y, ARESTA, ARESTA},
                (Rectangle) { coluna_checada * ARESTA, linha_checada * ARESTA + ALTURA_MENU_SUPERIOR, ARESTA, ARESTA}
            )) {
                visivel = 1;
            }

        }
    }

    return visivel;
}

// funcao responsável por desenhar todos os elementos do jogo (fora os menus e textos)
void desenhaMapa(int max_linhas, int max_colunas, PLAYER *player, TOUPEIRA *toupeiras, JOGO jogo) {
    Color cor_bloco;
    Rectangle bloco;
    int l, c, c_toup, x, y, width, height;
    int visivel = 0;

    if (player->power_up && (GetTime() - player->time_power_up) > 1 && jogo.estado == JOGANDO) {
        player->power_up = 0;
    }

    for (l = 0; l < MAX_LINHAS; l++) {
        for (c = 0; c < MAX_COLUNAS; c++) {
            cor_bloco = GRAY;
            x = c * ARESTA;
            y = (l * ARESTA) + ALTURA_MENU_SUPERIOR;
            visivel = player->power_up || campoDeVisao(*player, x, y);
            width = jogo.aresta;
            height = jogo.aresta;

            switch (jogo.mapa[l][c]) {
                case PAREDE_INDESTRUTIVEL:
                    cor_bloco = BLACK;
                    break;

                case POS_INICIAL:
                case POS_INICIAL_TOUPEIRA:
                    cor_bloco = visivel ? RAYWHITE : GRAY;
                    break;

                case OURO:
                    cor_bloco = visivel ? GOLD : GRAY;
                    break;

                case ESMERALDA:
                    cor_bloco = visivel ? (Color) { 10, 228, 100, 122 } : GRAY;
                    break;

                case AREA_SOTERRADA:
                    cor_bloco = visivel ? (Color) { 120, 100, 80, 255 } : GRAY;
                    break;

                case POWER_UP:
                    if (visivel || campoDeVisao(*player, x, y)) {
                        x += 7.5;
                        y += 7.5;
                        width = 15;
                        height = 15;
                        cor_bloco = (Color) { 255, 60, 180, 255 };
                    }
                    break;

                case LIVRE:
                    cor_bloco = visivel || campoDeVisao(*player, x, y) ? RAYWHITE : GRAY;
            }

            bloco = (Rectangle) { x, y, width, height };
            DrawRectangleRec(bloco,  cor_bloco);
        }
    }

    // desenha jogador
    DrawRectangle(player->pos.x, player->pos.y, ARESTA, ARESTA, BLUE);

    // desenha toupeiras
    for (c_toup = 0; c_toup < jogo.qnt_toupeiras; c_toup++) {
        if(toupeiras[c_toup].estado == 1){
            cor_bloco = visivel || campoDeVisao(*player, toupeiras[c_toup].pos.x, toupeiras[c_toup].pos.y) ? RED : GRAY;
            DrawRectangle(toupeiras[c_toup].pos.x, toupeiras[c_toup].pos.y, ARESTA, ARESTA, cor_bloco);
        }
    }
    if(player->tiro.estado == 1){
            DrawRectangle(player->tiro.pos.x, player->tiro.pos.y, player->tiro.tamanho, player->tiro.tamanho, YELLOW);
    }
}

void colisaoBlocoEspcial(char bloco, PLAYER *player, JOGO *jogo, int linha, int coluna) {
    switch (bloco) {
        case OURO:
            player->pontos += 50;
            break;

        case ESMERALDA:
            player->pontos += 100;
            player->esmeraldas_coletadas++;
            break;
        
        case POWER_UP:
            player->power_up = 1;
            player->time_power_up = GetTime();
    }

    // coleta o bloco (remove ele do mapa)
    jogo->mapa[linha][coluna] = LIVRE;
}

// Funcao responsável por realizar movimentação do jogador
void movimentaJogador(PLAYER *player, int passo, TOUPEIRA *toupeiras, JOGO *jogo) {
    int i;
    int linha, coluna;
    int linha_player, coluna_player;
    int linha_checada, coluna_checada;

    linha_player = ((int) player->pos.y - ALTURA_MENU_SUPERIOR) / jogo->aresta;
    coluna_player = (int) player->pos.x / jogo->aresta;

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
        if (podeMover(player->pos, player->des, *jogo, player->blocos_intransponiveis, jogo->aresta)) {
            move(&player->pos, player->des, passo);

            // checa se o jogador passou por um bloco especial (ouro, esmeralda ou power_up)
            for (linha = -1; linha < 2; linha++) {
                for (coluna = -1; coluna < 2; coluna++) {
                    linha_checada = linha_player + linha;
                    coluna_checada = coluna_player + coluna;

                    if (ValorNoArray(jogo->mapa[linha_checada][coluna_checada], jogo->blocos_especiais, N_BLOCOS_ESPECIAIS)) {
                        if (CheckCollisionRecs(
                            (Rectangle) { player->pos.x + player->des.x, player->pos.y + player->des.y, jogo->aresta, jogo->aresta },
                            (Rectangle) { coluna_checada * jogo->aresta, (linha_checada * jogo->aresta) + ALTURA_MENU_SUPERIOR, jogo->aresta, jogo->aresta })) {
                                colisaoBlocoEspcial(jogo->mapa[linha_checada][coluna_checada], player, jogo, linha_checada, coluna_checada);
                        }
                    }
                }
            }
        }
    }

    // tira a vida do jogador caso ele encostar em uma toupeira
    for (i = 0; i < jogo->qnt_toupeiras; i++) {
        if(toupeiras[i].estado == 1){
            if (CheckCollisionRecs((Rectangle) { player->pos.x, player->pos.y, ARESTA, ARESTA }, (Rectangle) {toupeiras[i].pos.x, toupeiras[i].pos.y, ARESTA, ARESTA})) {
                
                if(toupeiras[i].estado == 1){
                    player->vidas--;
                }
                
                if (player->vidas == 0) {
                    jogo->estado = PERDEU;
                } else {
                    resetPosicoes(player, toupeiras, jogo->qnt_toupeiras);
                }
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
    if (diff_time >= 1) {
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
            if(toupeiras[i].estado == 1){
                move(&toupeiras[i].pos, toupeiras[i].des, PASSO_TOUPEIRAS);
            }
        } else {
            if (toupeiras[i].des.x) toupeiras[i].des.x *= -1;
            if (toupeiras[i].des.y) toupeiras[i].des.y *= -1;
        }
    }
}

// Função responsável por carregar o mapa do nivel atual
int carregaMapa(JOGO *jogo) {
    int linha = 0, coluna = 0, result = 1;
    char caminho_mapa[MAX_PATH_SIZE], c;
    FILE *arquivo_mapa;

    // busca o caminho do mapa em jogo->mapas[nivel - 1]
    sprintf(caminho_mapa, "./maps/%s", jogo->mapas[jogo->nivel - 1]);
    arquivo_mapa = fopen(caminho_mapa, "r");

    if (arquivo_mapa != NULL) {
        while (!feof(arquivo_mapa)) {
            c = getc(arquivo_mapa);

            if (c == '\n') {
                coluna = 0;
                linha++;
            } else if (c != -1) {
                jogo->mapa[linha][coluna] = c;
                coluna++;

                if (c == ESMERALDA) jogo->qnt_esmeraldas += 1;
            }

        }
    } else result = 0;

    fclose(arquivo_mapa);

    return result;
}

// Função responsável por varrer a pasta './maps' e colocar todos os mapas encontrados dentro da struct JOGO.
int carregaMapas(JOGO *jogo) {
    struct dirent *dir;
    int result = 1, map_count = 0;
    char path[MAX_PATH_SIZE];
    DIR *d;

    d = opendir("./maps/");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            strcpy(path, dir->d_name);
            // se o caminho for ".." ou "." apenas o ignora (caminhos utilizados para navegação de diretório)
            if (strcmp(path, "..") && strcmp(path, ".")) {
                strcpy(jogo->mapas[map_count], path);
                map_count++;
            }
        }

        closedir(d);
        jogo->qnt_mapas = map_count;
    } else result = 0;

    // se não há nenhum mapa na pasta, retorna erro.
    if (jogo->qnt_mapas == 0) {
        result = 0;
    }

    return result;
}

void iniciaJogo(JOGO *jogo, TOUPEIRA *toupeiras, PLAYER *player, int nivel) {
    jogo->altura_mapa = MAX_LINHAS * ARESTA;
    jogo->largura_mapa = MAX_COLUNAS * ARESTA;
    jogo->aresta = ARESTA;
    jogo->qnt_toupeiras = 0;
    jogo->qnt_esmeraldas = 0;
    jogo->toup_muda_movimento = GetTime();
    jogo->nivel = nivel;
    jogo->estado = JOGANDO;
    jogo->qnt_mapas = 0;
    sprintf(jogo->blocos_especiais, "%c%c%c", OURO, ESMERALDA, POWER_UP);

    if (carregaMapas(jogo)) {
        if (carregaMapa(jogo)) {
            iniciaToupeiras(toupeiras, jogo->mapa, jogo);
            iniciaJogador(player, jogo->mapa);
        } else {
            printf("Ocorreu um erro no carregamento do mapa '%s', verifique sua integridade.", jogo->mapas[jogo->nivel - 1]);
            jogo->estado = OP_SAIR;
        }
    } else {
        printf("Ocorreu um erro no carregamento dos mapas!\nVerifique se eles estão na pasta correta: './maps'");
        jogo->estado = OP_SAIR;
    }
}

void desenhaTextos(JOGO jogo, PLAYER player) {
    char title[50] = { "PACMINE - UFRGS" };
    DrawRectangle(0, 0, jogo.largura_mapa, ALTURA_MENU_SUPERIOR, WHITE);
    DrawRectangle(0, (jogo.altura_mapa + ALTURA_MENU_SUPERIOR), jogo.largura_mapa, ALTURA_MENU_INFERIOR, WHITE);
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

// trata casos especiais de selecao no menu
void opcaoMenuSelecionada(char opcao, JOGO *jogo) {
    switch (opcao) {
        case OP_CARREGAR_JOGO:
            break;

        case OP_SALVAR_JOGO:
            break;

        case OP_VOLTAR:
            jogo->estado = JOGANDO;
            break;

        default:
            jogo->estado = opcao;
    }
}

// função responsável por desenhar todos os elementos do menu
void desenhaMenu(JOGO *jogo) {
    int altura_tela = jogo->altura_mapa + ALTURA_MENU_INFERIOR + ALTURA_MENU_SUPERIOR;
    int espaçamento_linhas = 40, pos_y, pos_x, offset_linhas, offset_tela, i;
    char opcoesMenu[5][30] = { NOVO_JOGO, CARREGAR_JOGO, SALVAR_JOGO, VOLTAR, SAIR };
    char opcoesMenuChar[5] = { OP_NOVO_JOGO, OP_CARREGAR_JOGO, OP_SALVAR_JOGO, OP_VOLTAR, OP_SAIR };
    int n_opcoes_menu = sizeof(opcoesMenuChar) / sizeof(char);
    Color cor_texto;
    Rectangle retangulo_opcao;

    ofuscaJogo(jogo);

    for (i = 0; i < n_opcoes_menu; i++) {
        // calcula a posicao dos elementos na tela
        pos_x = (jogo->largura_mapa - MeasureText(opcoesMenu[i], FONT_SIZE_MENU)) / 2;
        offset_linhas = (espaçamento_linhas * (n_opcoes_menu - (i + (n_opcoes_menu - 1))));
        offset_tela = (altura_tela - (FONT_SIZE_MENU * (n_opcoes_menu - i))) / 2; 
        pos_y = offset_tela - offset_linhas;
        // define retangulo de hitbox para selecionar uma opcao com o mouse
        retangulo_opcao = (Rectangle) { pos_x - 10, pos_y, MeasureText(opcoesMenu[i], FONT_SIZE_MENU) + 20, FONT_SIZE_MENU };
        cor_texto = WHITE;

        // checa se o mouse esta em cima de uma opcao
        if (CheckCollisionPointRec((Vector2) { GetMouseX(), GetMouseY() }, retangulo_opcao)) {
            cor_texto = GRAY;

            // captura o clique do mouse em uma opcao
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                opcaoMenuSelecionada(opcoesMenuChar[i], jogo);
            }
        }

        DrawText(opcoesMenu[i], pos_x, pos_y, FONT_SIZE_MENU, cor_texto);
    }
}

// abre e fecha o menu
void toggleMenu(JOGO *jogo) {
    if (IsKeyPressed(KEY_TAB)) {
        // se o menu estiver aberto, o fecha. Senão, o abre.
        jogo->estado = jogo->estado == MENU ? JOGANDO : MENU;
    }

    if (jogo->estado == MENU) desenhaMenu(jogo);
}

// passa de fase e/ou termina jogo.
void verificaFinal(PLAYER *player, JOGO *jogo, TOUPEIRA *toupeiras) {
    if (player->esmeraldas_coletadas == jogo->qnt_esmeraldas) {
        if (jogo->nivel == jogo->qnt_mapas) {
            jogo->estado = GANHOU;
        } else {
            iniciaJogo(jogo, toupeiras, player, jogo->nivel + 1);
            player->vidas += player->vidas == 3 ? 0 : 1;
        }
    }
}

void desenhaTelaFinal(PLAYER *player, JOGO *jogo) {
    char textoFinal[100], pontuacaoTotal[100];

    ofuscaJogo(jogo);
    sprintf(pontuacaoTotal, "Pontos totais: %d", player->pontos);
    sprintf(textoFinal, jogo->estado == GANHOU ? "Missão bem sucedida." : "A Missão falhou.");
    DrawText(textoFinal, (jogo->largura_mapa - MeasureText(textoFinal, FONT_SIZE_MENU)) / 2, (jogo->altura_mapa - FONT_SIZE_MENU) / 2, FONT_SIZE_MENU, jogo->estado == GANHOU ? DARKGREEN : RED);
    DrawText(pontuacaoTotal, (jogo->largura_mapa - MeasureText(pontuacaoTotal, 30)) / 2, (jogo->altura_mapa - 30) / 2 + FONT_SIZE_MENU + 15, 30, WHITE);
}

void desenhaJogo(int max_linhas, int max_colunas, PLAYER *player, TOUPEIRA *toupeiras, JOGO *jogo) {
    desenhaMapa(MAX_LINHAS, MAX_COLUNAS, player, toupeiras, *jogo);
    desenhaTextos(*jogo, *player);
    toggleMenu(jogo);

    if (jogo->estado == GANHOU || jogo->estado == PERDEU) {
        desenhaTelaFinal(player, jogo);
    }
}

void moveTiro(PLAYER *player){
    player->tiro.pos.x = player->tiro.pos.x + player->tiro.des.x;
    player->tiro.pos.y = player->tiro.pos.y + player->tiro.des.y;
}

void calculaSentido(PLAYER *player){
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        player->tiro.des.x = 1;
    }

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        player->tiro.des.x = -1;
    }

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        player->tiro.des.y = -1;
    }

    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        player->tiro.des.y = 1;
    }
    
    if (!IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_D) && !IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_A)){
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)){
            player->tiro.des.x = 0;
        }
    }
    if (!IsKeyDown(KEY_DOWN) && !IsKeyDown(KEY_S) && !IsKeyDown(KEY_UP) && !IsKeyDown(KEY_W)){
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
            player->tiro.des.y = 0;
        }
    }
}


void colideTiro(PLAYER *player, TOUPEIRA *toupeiras, JOGO *jogo){
    
    int linha, coluna;
    int linha_tiro, coluna_tiro;
    int linha_checada, coluna_checada;
    
    if(!podeMover(player->tiro.pos, player->tiro.des, *jogo, player->tiro.blocos_intransponiveis, player->tiro.tamanho)){
        player->tiro.estado = 0;
    }
    
    for (int i = 0; i < jogo->qnt_toupeiras; i++) {
        if (toupeiras[i].estado) {
            if (CheckCollisionRecs((Rectangle) { player->tiro.pos.x, player->tiro.pos.y, player->tiro.tamanho, player->tiro.tamanho }, (Rectangle) {toupeiras[i].pos.x, toupeiras[i].pos.y, ARESTA, ARESTA})) {
                player->pontos += 200;
                player->tiro.estado = 0;
                toupeiras[i].estado = 0;
            }
        }
    }
    
    linha_tiro = ((int) player->tiro.pos.y - ALTURA_MENU_SUPERIOR) / jogo->aresta;
    coluna_tiro = (int) player->tiro.pos.x / jogo->aresta;

    for (linha = -1; linha < 2; linha++) {
                for (coluna = -1; coluna < 2; coluna++) {
                    linha_checada = linha_tiro + linha;
                    coluna_checada = coluna_tiro + coluna;

                    if (jogo->mapa[linha_checada][coluna_checada] == AREA_SOTERRADA) {
                        if (CheckCollisionRecs(
                            (Rectangle) { player->tiro.pos.x + player->tiro.des.x, player->tiro.pos.y + player->tiro.des.y, player->tiro.tamanho, player->tiro.tamanho },
                            (Rectangle) { coluna_checada * jogo->aresta, (linha_checada * jogo->aresta) + ALTURA_MENU_SUPERIOR, jogo->aresta, jogo->aresta })) {
                                
                                jogo->mapa[linha_checada][coluna_checada] = LIVRE;
                                player->tiro.estado = 0;
                        }
                    }
                }
            }
}

void atira(PLAYER *player, TOUPEIRA *toupeiras,  JOGO *jogo) {
    if(IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_G)){
        
        if(player->tiro.estado == 0){
            player->tiro.estado = 1;
            player->tiro.pos.x = player->pos.x + 5;
            player->tiro.pos.y = player->pos.y + 5;
        }
    }
    
    if(player->tiro.estado == 0){
        calculaSentido(player);
    }
    if(player->tiro.estado == 1){
        moveTiro(player);
        moveTiro(player);
        moveTiro(player);
        colideTiro(player, toupeiras, jogo);
    }
    
}

int main(){
    JOGO jogo;
    PLAYER player;
    TOUPEIRA toupeiras[MAX_TOUPEIRAS];

    // inicia jogo, jogador e toupeiras
    iniciaJogo(&jogo, toupeiras, &player, 1);

    InitWindow(jogo.largura_mapa, jogo.altura_mapa + (ALTURA_MENU_INFERIOR + ALTURA_MENU_SUPERIOR), "PACMINE - UFRGS");

    SetTargetFPS(60);

    while (!WindowShouldClose() && jogo.estado != OP_SAIR) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (jogo.estado) {
            case JOGANDO:
                movimentaJogador(&player, PASSO, toupeiras, &jogo);
                movimentaToupeiras(toupeiras, PASSO_TOUPEIRAS, &jogo);
                atira(&player, toupeiras, &jogo);
                break;

            case OP_NOVO_JOGO:
                iniciaJogo(&jogo, toupeiras, &player, 1);
                break;
        }

        verificaFinal(&player, &jogo, toupeiras);
        desenhaJogo(MAX_LINHAS, MAX_COLUNAS, &player, toupeiras, &jogo);
        
        EndDrawing();
    }

    CloseWindow();

    return 0;
}