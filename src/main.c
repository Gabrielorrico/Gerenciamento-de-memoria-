#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 256 // o tamanho dos frames é igual ao tamanho das paginas
#define NUMERO_PAGINAS 256
#define NUMERO_FRAMES 128  
#define TAMANHO_MEMORIA (NUMERO_FRAMES * PAGE_SIZE)
#define TAMANHO_TLB 16

signed char memoria[TAMANHO_MEMORIA];
int tabelaPaginas[256]; //o projeto pede para utilizar 16 enderecos e paginas com 256 bytes
int framesUsados = 0;
int entradasTLBUsadas = 0;
int tempo = 0;
int tempoTLB[TAMANHO_TLB];
int tempoPaginas[NUMERO_PAGINAS];
int indiceTLB = -1;

int main(int argc, char* argv[]) {

    if(argc != 4){
        fprintf(stderr,"ERRO: quantidade de argumentos invalida");
        exit(1);
    }

    if(strcmp(argv[2],"fifo") == 0){
        //codigo do fifo
    }else if(strcmp(argv[2],"lru") == 0){
        // codigo do LRU
    }else{
        fprintf(stderr, "ERRO: algoritmo invalido");
        exit(1);
    }

    if(strcmp(argv[3],"fifo") == 0){
        //codigo do fifo
    }else if(strcmp(argv[3],"lru") == 0){
        // codigo do LRU
    }else{
        fprintf(stderr, "ERRO: algoritmo invalido");
        exit(1);
    }

    FILE *backingStore = fopen("BACKING_STORE.bin","rb");
    FILE *arquivo = fopen(argv[1], "r");

    if(backingStore == NULL){
        fprintf(stderr, "ERRO: nao foi possivel abrir o backingStore.bin");
        exit(1);
    }

    if(arquivo == NULL){
        fprintf(stderr, "ERRO: nao foi possivel abrir o arquivo %s", argv[1]);
        exit(1);
    }

    int paginaTLB[TAMANHO_TLB];
    int frameTLB[TAMANHO_TLB];

    for(int i = 0; i < TAMANHO_TLB; i++){
        paginaTLB[i] = -1;
        frameTLB[i] = -1;
    }

    for(int i = 0; i < NUMERO_PAGINAS; i++){
        tabelaPaginas[i] = -1;
    }

    int endereco;

    while(fscanf(arquivo, "%d", &endereco) != EOF){
        int numeroPagina = endereco / PAGE_SIZE; //calculos do livro 
        int offset = endereco % PAGE_SIZE;

        int frame;
        int enderecoFisico;
        int temTLB = 0;

        for(int i = 0; i < TAMANHO_TLB; i++){
            if(paginaTLB[i] == numeroPagina){ // TLB HIT
                frame = frameTLB[i];
                temTLB = 1;
                tempo++;
                tempoTLB[i] = tempo;
                break;
            } //miss não entra aqui pois e não da para concluir que tem o miss antes de consultar toda a TLB, logo vemos somente apos o for

        }

        if(temTLB == 1){//hit
            enderecoFisico = (frame * PAGE_SIZE) + offset; //calculo do livro

        }else{//miss


            if(tabelaPaginas[numeroPagina] == -1){// pagefault
                if(framesUsados < NUMERO_FRAMES){
                    frame = framesUsados;
                    framesUsados++;
                    fseek(backingStore, numeroPagina * PAGE_SIZE, SEEK_SET);
                    fread(memoria + (frame * PAGE_SIZE), sizeof(signed char), PAGE_SIZE, backingStore);
                    if(entradasTLBUsadas < TAMANHO_TLB){
                        paginaTLB[entradasTLBUsadas] = numeroPagina;
                        frameTLB[entradasTLBUsadas] = frame;
                        entradasTLBUsadas++;

                    }else{
                        
                    }
                    tabelaPaginas[numeroPagina] = frame;
                }else{

                }
            }else{
                frame = tabelaPaginas[numeroPagina];
                enderecoFisico = (frame * PAGE_SIZE) + offset;
                int valor = memoria[enderecoFisico];
                if(entradasTLBUsadas < TAMANHO_TLB){
                        paginaTLB[entradasTLBUsadas] = numeroPagina;
                        frameTLB[entradasTLBUsadas] = frame;
                        entradasTLBUsadas++;

                }else{
                        
                }
            }
        }
    }

    fclose(arquivo);    
    return 0;
}