/* controller_fire.c
 *
 * Demonstração de sistema de missões por jogador usando C:
 * - criação de vetor de missões (strings)
 * - atribuirMissao copia a missão sorteada para destino (strcpy)
 * - armazenar missão dinamicamente (malloc)
 * - verificarMissao: lógica simples (contagem de territórios do jogador)
 * - funções modularizadas: atacar, exibirMapa, exibirMissao, liberarMemoria
 *
 * Compile: gcc jogo_missoes.c -o jogo_missoes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Estrutura do território */
typedef struct {
    char nome[30];
    char cor[10];   /* cor/dono: ex "Red", "Blue" */
    int tropas;
} Territorio;

/* Prototipos */
void atribuirMissao(char *destino, char *missoes[], int totalMissoes);
int verificarMissao(char *missao, Territorio *mapa, int tamanho, const char *corJogador);
void exibirMissao(const char *missao);
void atacar(Territorio *atacante, Territorio *defensor);
void exibirMapa(Territorio *mapa, int tamanho);
void liberarMemoria(Territorio *mapa, int tamanho, char **missoesJogadores, int numJogadores);

/* Implementação */

/* Sorteia uma missão e copia para 'destino' (deve estar previamente alocado) */
void atribuirMissao(char *destino, char *missoes[], int totalMissoes) {
    if (totalMissoes <= 0) return;
    int idx = rand() % totalMissoes;
    strcpy(destino, missoes[idx]);
}

/* Verifica se a missão está cumprida.
   Para permitir verificações simples e genéricas, vamos usar missões do tipo:
   - "Possuir >= N territorios"  (exemplo: "Possuir 3 territorios")
   - "Eliminar tropas de cor X"  (exemplo: "Eliminar todas tropas vermelhas")
   Esta função é simples e procura palavras-chave na string da missão. */
int verificarMissao(char *missao, Territorio *mapa, int tamanho, const char *corJogador) {
    if (!missao || !mapa || !corJogador) return 0;

    /* Checar se missão pede "Possuir N territorios" */
    if (strstr(missao, "Possuir") != NULL && strstr(missao, "territorios") != NULL) {
        /* extrair número N na string (procura primeiro dígito) */
        int N = 0;
        const char *p = missao;
        while (*p) {
            if (*p >= '0' && *p <= '9') {
                N = atoi(p);
                break;
            }
            p++;
        }
        if (N > 0) {
            int cnt = 0;
            for (int i = 0; i < tamanho; i++) {
                if (strcmp(mapa[i].cor, corJogador) == 0) cnt++;
            }
            return (cnt >= N) ? 1 : 0;
        }
    }

    /* Checar se missão pede "Conquistar territorio X seguidos" - simplificamos para contar total */
    if (strstr(missao, "Conquistar") != NULL) {
        /* procurar número */
        int N = 0;
        const char *p = missao;
        while (*p) {
            if (*p >= '0' && *p <= '9') {
                N = atoi(p);
                break;
            }
            p++;
        }
        if (N > 0) {
            int cnt = 0;
            for (int i = 0; i < tamanho; i++) if (strcmp(mapa[i].cor, corJogador) == 0) cnt++;
            return (cnt >= N) ? 1 : 0;
        }
    }

    /* Checar missão "Eliminar todas as tropas da cor X" */
    if (strstr(missao, "Eliminar todas as tropas da cor") != NULL) {
        /* determinar cor alvo na string (ultima palavra) */
        /* simplificação: assumimos que cor tem uma palavra curta no final */
        char corAlvo[20] = {0};
        const char *last = strrchr(missao, ' ');
        if (last) {
            strncpy(corAlvo, last + 1, sizeof(corAlvo) - 1);
            /* remover eventual pontuação */
            char *nl = strchr(corAlvo, '.');
            if (nl) *nl = '\0';
            /* verificar se todas as tropas dessa cor estão a zero */
            int totalTropas = 0;
            for (int i = 0; i < tamanho; i++) {
                if (strcmp(mapa[i].cor, corAlvo) == 0) totalTropas += mapa[i].tropas;
            }
            return (totalTropas == 0) ? 1 : 0;
        }
    }

    /* Missão padrão: false */
    return 0;
}

/* Exibe missão (passada por valor) - mostrada apenas uma vez no início do jogo */
void exibirMissao(const char *missao) {
    if (!missao) return;
    printf("Sua missão: %s\n", missao);
}

/* Função de ataque simplificada:
   - rola 1d6 para atacante e defensor
   - quem tirar maior vence
   - se atacante vencer: defensor muda de cor para atacante->cor e recebe metade das tropas do atacante (arredondado para baixo)
   - se defensor vencer: atacante perde 1 tropa
   - validações: atacante.tropas must be > 1, cores diferentes */
void atacar(Territorio *atacante, Territorio *defensor) {
    if (!atacante || !defensor) return;
    if (strcmp(atacante->cor, defensor->cor) == 0) {
        /* não pode atacar território do mesmo jogador */
        return;
    }
    if (atacante->tropas <= 1) {
        /* não há tropas suficientes para atacar */
        return;
    }

    int dadoA = (rand() % 6) + 1;
    int dadoD = (rand() % 6) + 1;

    printf("Ataque: %s (%s, %d tropas) -> %s (%s, %d tropas) | rolagem A=%d D=%d\n",
           atacante->nome, atacante->cor, atacante->tropas,
           defensor->nome, defensor->cor, defensor->tropas,
           dadoA, dadoD);

    if (dadoA > dadoD) {
        /* atacante vence */
        int transferencia = atacante->tropas / 2;
        if (transferencia < 1) transferencia = 1;
        printf("Vencedor: atacante. %d tropas transferidas. Territorio %s agora pertence a %s.\n",
               transferencia, defensor->nome, atacante->cor);
        defensor->tropas = transferencia;
        strncpy(defensor->cor, atacante->cor, sizeof(defensor->cor)-1);
        defensor->cor[sizeof(defensor->cor)-1] = '\0';
        /* atacante perde as tropas transferidas */
        atacante->tropas -= transferencia;
        if (atacante->tropas < 0) atacante->tropas = 0;
    } else {
        /* defensor vence ou empate: atacante perde 1 tropa */
        atacante->tropas -= 1;
        if (atacante->tropas < 0) atacante->tropas = 0;
        printf("Defensor resiste. Atacante perde 1 tropa. Tropas restantes: %d\n", atacante->tropas);
    }
}

/* Exibe mapa */
void exibirMapa(Territorio *mapa, int tamanho) {
    printf("----- MAPA -----\n");
    for (int i = 0; i < tamanho; i++) {
        printf("%2d: %s | Cor: %-6s | Tropas: %d\n", i, mapa[i].nome, mapa[i].cor, mapa[i].tropas);
    }
    printf("----------------\n");
}

/* Libera memoria (para este exemplo, territórios são alocados estaticamente, mas missões dinamicamente) */
void liberarMemoria(Territorio *mapa, int tamanho, char **missoesJogadores, int numJogadores) {
    /* mapa pode ter sido alocado dinamicamente; se sim, descomente free(mapa) no chamador.
       Aqui liberamos o vetor de strings das missões */
    if (missoesJogadores) {
        for (int i = 0; i < numJogadores; i++) {
            if (missoesJogadores[i]) free(missoesJogadores[i]);
        }
        free(missoesJogadores);
    }
}

/* MAIN: demonstração de uso */
int main(void) {
    srand((unsigned)time(NULL));

    /* 1) Criar vetor de missões */
    char *missoesDisponiveis[] = {
        "Possuir 3 territorios",                           /* exemplo simples */
        "Possuir 4 territorios",
        "Conquistar 2 territorios seguidos",
        "Eliminar todas as tropas da cor Red",
        "Possuir 5 territorios"
    };
    int totalMissoes = sizeof(missoesDisponiveis) / sizeof(missoesDisponiveis[0]);

    /* 2) Criar mapa (exemplo com 8 territorios). Alocar dinamicamente */
    int numTerritorios = 8;
    Territorio *mapa = (Territorio *) malloc(sizeof(Territorio) * numTerritorios);
    if (!mapa) {
        fprintf(stderr, "Falha de alocacao do mapa\n");
        return 1;
    }

    /* inicializar nomes, cores e tropas */
    const char *nomes[8] = {"T1","T2","T3","T4","T5","T6","T7","T8"};
    const char *coresIniciais[8] = {"Blue","Red","Blue","Red","Blue","Red","Blue","Red"};
    int tropasInit[8] = {3,4,2,5,3,4,2,3};

    for (int i = 0; i < numTerritorios; i++) {
        strncpy(mapa[i].nome, nomes[i], sizeof(mapa[i].nome)-1);
        mapa[i].nome[sizeof(mapa[i].nome)-1] = '\0';
        strncpy(mapa[i].cor, coresIniciais[i], sizeof(mapa[i].cor)-1);
        mapa[i].cor[sizeof(mapa[i].cor)-1] = '\0';
        mapa[i].tropas = tropasInit[i];
    }

    /* 3) Preparar jogadores (duas cores: Blue e Red) e alocar missão dinamicamente */
    int numJogadores = 2;
    const char *coresJogadores[2] = {"Blue", "Red"};

    /* alocamos um vetor de ponteiros para missões para cada jogador */
    char **missoesJogadores = (char **) malloc(sizeof(char *) * numJogadores);
    if (!missoesJogadores) {
        fprintf(stderr, "Falha alocar missoesJogadores\n");
        free(mapa);
        return 1;
    }
    for (int i = 0; i < numJogadores; i++) {
        missoesJogadores[i] = (char *) malloc(200); /* buffer dinâmico por jogador */
        if (!missoesJogadores[i]) {
            fprintf(stderr, "Falha alocar missao jogador %d\n", i);
            /* liberar já alocado e sair */
            for (int j = 0; j < i; j++) free(missoesJogadores[j]);
            free(missoesJogadores);
            free(mapa);
            return 1;
        }
        /* atribuir missão (copia) */
        atribuirMissao(missoesJogadores[i], missoesDisponiveis, totalMissoes);
    }

    /* 4) Exibir mapa e missões no início (missão exibida apenas uma vez por jogador) */
    exibirMapa(mapa, numTerritorios);
    for (int i = 0; i < numJogadores; i++) {
        printf("Jogador %d (%s) - ", i+1, coresJogadores[i]);
        exibirMissao(missoesJogadores[i]);
    }

    /* 5) Loop de jogo simples: simulamos algumas rodadas onde cada jogador realiza um ataque aleatório válido */
    int rodada = 0;
    int vencedor = -1;
    int maxRodadas = 50; /* só para não rodar indefinidamente em demo */

    while (rodada < maxRodadas && vencedor == -1) {
        printf("\n===== RODADA %d =====\n", rodada+1);

        for (int jogador = 0; jogador < numJogadores && vencedor == -1; jogador++) {
            const char *corAtual = coresJogadores[jogador];

            /* escolher território atacante aleatoriamente que pertença ao jogador e com tropas > 1 */
            int atacanteIdx = -1;
            for (int tries = 0; tries < 20; tries++) {
                int idx = rand() % numTerritorios;
                if (strcmp(mapa[idx].cor, corAtual) == 0 && mapa[idx].tropas > 1) {
                    atacanteIdx = idx;
                    break;
                }
            }
            if (atacanteIdx == -1) {
                printf("Jogador %d (%s) nao tem territorios aptos para atacar.\n", jogador+1, corAtual);
                continue;
            }

            /* escolher defensor aleatorio de cor diferente */
            int defensorIdx = -1;
            for (int tries = 0; tries < 20; tries++) {
                int idx = rand() % numTerritorios;
                if (strcmp(mapa[idx].cor, corAtual) != 0) {
                    defensorIdx = idx;
                    break;
                }
            }
            if (defensorIdx == -1) {
                printf("Jogador %d (%s) nao encontrou alvo.\n", jogador+1, corAtual);
                continue;
            }

            /* realizar ataque */
            atacar(&mapa[atacanteIdx], &mapa[defensorIdx]);

            /* checar missão após ataque (verificação silenciosa) */
            if (verificarMissao(missoesJogadores[jogador], mapa, numTerritorios, corAtual)) {
                vencedor = jogador;
                printf("\n>>> Jogador %d (%s) cumpriu a missão: %s\n", jogador+1, corAtual, missoesJogadores[jogador]);
                break;
            }
        }

        exibirMapa(mapa, numTerritorios);
        rodada++;
    }

    if (vencedor == -1) {
        printf("\nNenhum jogador cumpriu a missão em %d rodadas.\n", maxRodadas);
    } else {
        printf("\nJogador %d (%s) venceu por cumprir a missao!\n", vencedor+1, coresJogadores[vencedor]);
    }

    /* 6) Liberar memória */
    liberarMemoria(mapa, numTerritorios, missoesJogadores, numJogadores);
    free(mapa); /* mapa foi alocado com malloc */

    return 0;
}
