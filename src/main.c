#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define PAGE_SIZE 256 // o tamanho dos frames é igual ao tamanho das paginas
#define NUMERO_PAGINAS 256
#define NUMERO_FRAMES 128
#define TAMANHO_MEMORIA (NUMERO_FRAMES * PAGE_SIZE)
#define TAMANHO_TLB 16

signed char memoria[TAMANHO_MEMORIA];
int tabelaPaginas[256];
int framesUsados = 0;
int entradasTLBUsadas = 0;
int tempo = 0;
int tempoTLB[TAMANHO_TLB];
int tempoPaginas[NUMERO_PAGINAS];
int algoritmoPaginas;
int algoritmoTLB;
int ordemChegada[NUMERO_PAGINAS];
int ordemChegadaTLB[TAMANHO_TLB];
int contadorChegadaTLB = 0;

typedef struct {
    int indice;
    int numeroPagina;
    int *paginaTLB;
    int *resultadoTLB;
} DadosThread;

void *buscarTLB(void *argumento) {
    DadosThread *dados = (DadosThread *) argumento;

    if(dados->paginaTLB[dados->indice] == dados->numeroPagina) {
        dados->resultadoTLB[dados->indice] = 1;
    } else {
        dados->resultadoTLB[dados->indice] = 0;
    }

    return NULL;
}

int consultarTLB(int numeroPagina, int paginaTLB[]) {
    pthread_t threads[TAMANHO_TLB];
    DadosThread dados[TAMANHO_TLB];
    int resultadoTLB[TAMANHO_TLB];
    int threadsCriadas = 0;
    int indiceEncontrado = -1;

    for(int i = 0; i < TAMANHO_TLB; i++) {
        dados[i].indice = i;
        dados[i].numeroPagina = numeroPagina;
        dados[i].paginaTLB = paginaTLB;
        dados[i].resultadoTLB = resultadoTLB;

        if(pthread_create(&threads[i], NULL, buscarTLB, &dados[i]) != 0) {
            for(int j = 0; j < threadsCriadas; j++) {
                pthread_join(threads[j], NULL);
            }

            return -2;
        }

        threadsCriadas++;
    }

    for(int i = 0; i < TAMANHO_TLB; i++) {
        if(pthread_join(threads[i], NULL) != 0) {
            return -2;
        }
    }

    for(int i = 0; i < TAMANHO_TLB; i++) {
        if(resultadoTLB[i] == 1) {
            indiceEncontrado = i;
            break;
        }
    }

    return indiceEncontrado;
}

void inserirTLB(int numeroPagina, int frame, int paginaTLB[], int frameTLB[]) {
    int indiceLivre = -1;

    for(int i = 0; i < TAMANHO_TLB; i++) {
        if(paginaTLB[i] == -1) {
            indiceLivre = i;
            break;
        }
    }

    if(indiceLivre == -1) {
        if(algoritmoTLB == 1) {
            int menorTempo = contadorChegadaTLB + 1;
            int indiceAntigo = -1;

            for(int i = 0; i < TAMANHO_TLB; i++) {
                if(paginaTLB[i] != -1 && ordemChegadaTLB[i] < menorTempo) {
                    menorTempo = ordemChegadaTLB[i];
                    indiceAntigo = i;
                }
            }

            indiceLivre = indiceAntigo;
        } else {
            int menorTempo = -1;
            int indiceAntigo = -1;

            for(int i = 0; i < TAMANHO_TLB; i++) {
                if(paginaTLB[i] != -1) {
                    if(indiceAntigo == -1 || tempoTLB[i] < menorTempo) {
                        menorTempo = tempoTLB[i];
                        indiceAntigo = i;
                    }
                }
            }

            indiceLivre = indiceAntigo;
        }
    }

    paginaTLB[indiceLivre] = numeroPagina;
    frameTLB[indiceLivre] = frame;
    tempoTLB[indiceLivre] = tempo;
    ordemChegadaTLB[indiceLivre] = ++contadorChegadaTLB;
}

int main(int argc, char *argv[]) {

    if(argc != 4) {
        fprintf(stderr, "ERRO: quantidade de argumentos invalida\n");
        return 1;
    }

    if(strcmp(argv[2], "fifo") == 0) {
        algoritmoPaginas = 1;
    } else if(strcmp(argv[2], "lru") == 0) {
        algoritmoPaginas = 2;
    } else {
        fprintf(stderr, "ERRO: algoritmo de paginas invalido\n");
        return 1;
    }

    if(strcmp(argv[3], "fifo") == 0) {
        algoritmoTLB = 1;
    } else if(strcmp(argv[3], "lru") == 0) {
        algoritmoTLB = 2;
    } else {
        fprintf(stderr, "ERRO: algoritmo da TLB invalido\n");
        return 1;
    }

    FILE *backingStore = fopen("BACKING_STORE.bin", "rb");
    FILE *arquivo = fopen(argv[1], "r");

    if(backingStore == NULL) {
        fprintf(stderr, "ERRO: nao foi possivel abrir o BACKING_STORE.bin\n");
        if(arquivo != NULL) {
            fclose(arquivo);
        }
        return 1;
    }

    if(arquivo == NULL) {
        fprintf(stderr, "ERRO: nao foi possivel abrir o arquivo %s\n", argv[1]);
        fclose(backingStore);
        return 1;
    }

    FILE *saida = fopen("correct.txt", "w");

    if(saida == NULL) {
        fprintf(stderr, "ERRO: nao foi possivel criar o arquivo correct.txt\n");
        fclose(backingStore);
        fclose(arquivo);
        return 1;
    }

    int paginaTLB[TAMANHO_TLB];
    int frameTLB[TAMANHO_TLB];

    for(int i = 0; i < TAMANHO_TLB; i++) {
        paginaTLB[i] = -1;
        frameTLB[i] = -1;
        tempoTLB[i] = 0;
        ordemChegadaTLB[i] = 0;
    }

    for(int i = 0; i < NUMERO_PAGINAS; i++) {
        tabelaPaginas[i] = -1;
        tempoPaginas[i] = 0;
        ordemChegada[i] = 0;
    }

    int endereco;
    int numeroEnderecos = 0;
    int tlbHits = 0;
    int tlbMisses = 0;
    int pageFaults = 0;
    int erroExecucao = 0;

    while(1) {
        int leitura = fscanf(arquivo, "%d", &endereco);

        if(leitura == EOF) {
            break;
        }

        if(leitura != 1) {
            fprintf(stderr, "ERRO: arquivo de enderecos possui um valor invalido\n");
            erroExecucao = 1;
            break;
        }

        if(endereco < 0 || endereco >= (NUMERO_PAGINAS * PAGE_SIZE)) {
            fprintf(stderr, "ERRO: endereco logico fora do intervalo: %d\n", endereco);
            erroExecucao = 1;
            break;
        }

        numeroEnderecos++;

        int numeroPagina = endereco / PAGE_SIZE;
        int offset = endereco % PAGE_SIZE;

        int frame;
        int enderecoFisico;
        int temTLB = 0;

        int indiceTLB = consultarTLB(numeroPagina, paginaTLB);

        if(indiceTLB == -2) {
            fprintf(stderr, "ERRO: nao foi possivel criar ou aguardar as threads da TLB\n");
            erroExecucao = 1;
            break;
        }

        if(indiceTLB != -1) {
            frame = frameTLB[indiceTLB];
            temTLB = 1;
            tlbHits++;
            tempo++;
            tempoTLB[indiceTLB] = tempo;
            tempoPaginas[numeroPagina] = tempo;
        } else {
            tlbMisses++;
            tempo++;
            tempoPaginas[numeroPagina] = tempo;
        }

        if(temTLB == 1) {
            enderecoFisico = (frame * PAGE_SIZE) + offset;
        } else {
            if(tabelaPaginas[numeroPagina] == -1) {
                pageFaults++;

                if(framesUsados < NUMERO_FRAMES) {
                    frame = framesUsados;
                    framesUsados++;

                    if(fseek(backingStore, numeroPagina * PAGE_SIZE, SEEK_SET) != 0 ||
                       fread(memoria + (frame * PAGE_SIZE), sizeof(signed char), PAGE_SIZE, backingStore) != PAGE_SIZE) {
                        fprintf(stderr, "ERRO: nao foi possivel ler a pagina %d do BACKING_STORE.bin\n", numeroPagina);
                        erroExecucao = 1;
                        break;
                    }

                    tabelaPaginas[numeroPagina] = frame;
                    ordemChegada[numeroPagina] = tempo;
                } else {
                    if(algoritmoPaginas == 1) {
                        int menorTempo = -1;
                        int paginaAntiga = -1;

                        for(int i = 0; i < NUMERO_PAGINAS; i++) {
                            if(tabelaPaginas[i] != -1) {
                                if(paginaAntiga == -1 || ordemChegada[i] < menorTempo) {
                                    menorTempo = ordemChegada[i];
                                    paginaAntiga = i;
                                }
                            }
                        }

                        frame = tabelaPaginas[paginaAntiga];
                        tabelaPaginas[paginaAntiga] = -1;

                        for(int i = 0; i < TAMANHO_TLB; i++) {
                            if(paginaTLB[i] == paginaAntiga) {
                                paginaTLB[i] = -1;
                                frameTLB[i] = -1;
                            }
                        }
                    } else {
                        int menorTempo = -1;
                        int paginaAntiga = -1;

                        for(int i = 0; i < NUMERO_PAGINAS; i++) {
                            if(tabelaPaginas[i] != -1) {
                                if(paginaAntiga == -1 || tempoPaginas[i] < menorTempo) {
                                    menorTempo = tempoPaginas[i];
                                    paginaAntiga = i;
                                }
                            }
                        }

                        frame = tabelaPaginas[paginaAntiga];
                        tabelaPaginas[paginaAntiga] = -1;

                        for(int i = 0; i < TAMANHO_TLB; i++) {
                            if(paginaTLB[i] == paginaAntiga) {
                                paginaTLB[i] = -1;
                                frameTLB[i] = -1;
                            }
                        }
                    }

                    if(fseek(backingStore, numeroPagina * PAGE_SIZE, SEEK_SET) != 0 ||
                       fread(memoria + (frame * PAGE_SIZE), sizeof(signed char), PAGE_SIZE, backingStore) != PAGE_SIZE) {
                        fprintf(stderr, "ERRO: nao foi possivel ler a pagina %d do BACKING_STORE.bin\n", numeroPagina);
                        erroExecucao = 1;
                        break;
                    }

                    tabelaPaginas[numeroPagina] = frame;
                    ordemChegada[numeroPagina] = tempo;
                }
            } else {
                frame = tabelaPaginas[numeroPagina];
            }

            enderecoFisico = (frame * PAGE_SIZE) + offset;
            inserirTLB(numeroPagina, frame, paginaTLB, frameTLB);
        }

        int valor = memoria[enderecoFisico];

        if(fprintf(saida, "Virtual address: %d Physical address: %d Value: %d\n",
                   endereco, enderecoFisico, valor) < 0) {
            fprintf(stderr, "ERRO: nao foi possivel escrever no arquivo correct.txt\n");
            erroExecucao = 1;
            break;
        }
    }

    if(ferror(arquivo)) {
        fprintf(stderr, "ERRO: falha durante a leitura do arquivo de enderecos\n");
        erroExecucao = 1;
    }

    if(fclose(saida) != 0) {
        fprintf(stderr, "ERRO: falha ao fechar correct.txt\n");
        erroExecucao = 1;
    }

    fclose(backingStore);
    fclose(arquivo);

    if(erroExecucao == 1) {
        remove("correct.txt");
        return 1;
    }

    double taxaPageFaults = 0.0;
    double taxaTLBHits = 0.0;

    if(numeroEnderecos > 0) {
        taxaPageFaults = (double) pageFaults / numeroEnderecos;
        taxaTLBHits = (double) tlbHits / numeroEnderecos;
    }

    printf("\nNumero de enderecos traduzidos = %d\n", numeroEnderecos);
    printf("Page Faults = %d\n", pageFaults);
    printf("Page Fault Rate = %.3f\n", taxaPageFaults);
    printf("TLB Hits = %d\n", tlbHits);
    printf("TLB Misses = %d\n", tlbMisses);
    printf("TLB Hit Rate = %.3f\n", taxaTLBHits);

    return 0;
}
