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
int algoritmoPaginas;
int algoritmoTLB;
int ordemChegada[NUMERO_PAGINAS];
int ordemChegadaTLB[TAMANHO_TLB];
int contadorChegadaTLB = 0;

int main(int argc, char* argv[]) {
    
    if(argc != 4){
        fprintf(stderr,"ERRO: quantidade de argumentos invalida");
        exit(1);
    }
    
    if(strcmp(argv[2],"fifo") == 0){
        algoritmoPaginas = 1;
    }else if(strcmp(argv[2],"lru") == 0){
        algoritmoPaginas = 2;
    }else{
        fprintf(stderr, "ERRO: algoritmo invalido");
        exit(1);
    }
    
    if(strcmp(argv[3],"fifo") == 0){
        algoritmoTLB = 1;
    }else if(strcmp(argv[3],"lru") == 0){
        algoritmoTLB = 2;
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
        
        int indiceTLB = -1;
        int frame;
        int enderecoFisico;
        int temTLB = 0;
        
        for(int i = 0; i < TAMANHO_TLB; i++){
            if(paginaTLB[i] == numeroPagina){ // TLB HIT
                indiceTLB = i;
                frame = frameTLB[i];
                temTLB = 1;
                tempo++;
                tempoTLB[i] = tempo;
                tempoPaginas[numeroPagina] = tempo;
                break;
            } //miss não entra aqui pois e não da para concluir que tem o miss antes de consultar toda a TLB, logo vemos somente apos o for
            
        }
        
        if(temTLB == 1){//TLBhit
            enderecoFisico = (frame * PAGE_SIZE) + offset; //calculo do livro
            int valor = memoria[enderecoFisico];

        }else{//TLBmiss
            
            tempo++;
            tempoPaginas[numeroPagina] = tempo;

            if(tabelaPaginas[numeroPagina] == -1){// pagefault
                if(framesUsados < NUMERO_FRAMES){
                    frame = framesUsados;
                    framesUsados++;
                    fseek(backingStore, numeroPagina * PAGE_SIZE, SEEK_SET);
                    fread(memoria + (frame * PAGE_SIZE), sizeof(signed char), PAGE_SIZE, backingStore);
                    enderecoFisico = (frame * PAGE_SIZE) + offset;
                    ordemChegada[numeroPagina] = tempo;
                    int valor = memoria[enderecoFisico];
                    
                    if(entradasTLBUsadas < TAMANHO_TLB){ //inserindo uma traducao na TLB
                        paginaTLB[entradasTLBUsadas] = numeroPagina;
                        frameTLB[entradasTLBUsadas] = frame;
                        tempoTLB[entradasTLBUsadas] = tempo;
                        ordemChegadaTLB[entradasTLBUsadas] = contadorChegadaTLB++;
                        entradasTLBUsadas++;

                    }else{
                        if(algoritmoTLB == 1){
                            int menorTempo = contadorChegadaTLB + 1;
                            int indiceAntigo = -1;
    
                            for(int i = 0; i < TAMANHO_TLB; i++){
                                if(paginaTLB[i] != -1 && ordemChegadaTLB[i] < menorTempo){
                                    menorTempo = ordemChegadaTLB[i];
                                    indiceAntigo = i;
                                }
                            }
    
                            paginaTLB[indiceAntigo] = numeroPagina;
                            frameTLB[indiceAntigo] = frame;
                            tempoTLB[indiceAntigo] = tempo;
                            ordemChegadaTLB[indiceAntigo] = contadorChegadaTLB++;

                        }else{
                            int menorTempo = -1;
                            int indiceAntigo = -1;

                            for(int i = 0; i < TAMANHO_TLB; i++){
                                if(paginaTLB[i] != -1){
                                    if(indiceAntigo == -1 || tempoTLB[i] < menorTempo){
                                        menorTempo = tempoTLB[i];
                                        indiceAntigo = i;
                                    }
                                }
                            }

                            paginaTLB[indiceAntigo] = numeroPagina;
                            frameTLB[indiceAntigo] = frame;
                            tempoTLB[indiceAntigo] = tempo;
                            ordemChegadaTLB[indiceAntigo] = contadorChegadaTLB++;
                        
                        }
                        
                    }
                    tabelaPaginas[numeroPagina] = frame;
                }else{

                    if(algoritmoPaginas == 1){

                        int menorTempo = tempo + 1;
                        int paginaAntiga = -1;
    
                        for(int i = 0; i < NUMERO_PAGINAS; i++){
                            if(tabelaPaginas[i] != -1 && ordemChegada[i] < menorTempo){
                                menorTempo = ordemChegada[i];
                                paginaAntiga = i;
                            }
                        }
    
                        frame = tabelaPaginas[paginaAntiga];
                        tabelaPaginas[paginaAntiga] = -1; // tirando a pagina antiga da tabela de PAGINAS
                        for(int i = 0; i < TAMANHO_TLB; i++){
                            if(paginaTLB[i] == paginaAntiga){
                                paginaTLB[i] = -1;
                                frameTLB[i] = -1;
                            }
                        }
    
                        fseek(backingStore, numeroPagina * PAGE_SIZE, SEEK_SET);
                        fread(memoria + (frame * PAGE_SIZE), sizeof(signed char), PAGE_SIZE, backingStore);
    
                        tabelaPaginas[numeroPagina] = frame;
                        ordemChegada[numeroPagina] = tempo;
    
                        enderecoFisico = (frame * PAGE_SIZE) + offset;
                        int valor = memoria[enderecoFisico];
                        
                    }else{
                        int menorTempo = tempo + 1;
                        int paginaAntiga = -1;

                        for(int i = 0; i < NUMERO_PAGINAS; i++){
                            if(tabelaPaginas[i] != -1){
                                if(paginaAntiga == -1 || tempoPaginas[i] < menorTempo){ // como aqui é LRU mudou apenas as comparacoes do IF
                                    menorTempo = tempoPaginas[i];
                                    paginaAntiga = i;
                                }
                            }
                        }

                        frame = tabelaPaginas[paginaAntiga];
                        tabelaPaginas[paginaAntiga] = -1;

                        for(int i = 0; i < TAMANHO_TLB; i++){
                            if(paginaTLB[i] == paginaAntiga){
                                paginaTLB[i] = -1;
                                frameTLB[i] = -1;
                            }
                        }

                        fseek(backingStore, numeroPagina * PAGE_SIZE, SEEK_SET);
                        fread(memoria + (frame * PAGE_SIZE), sizeof(signed char), PAGE_SIZE, backingStore);

                        tabelaPaginas[numeroPagina] = frame;
                        ordemChegada[numeroPagina] = tempo;

                        enderecoFisico = (frame * PAGE_SIZE) + offset;
                        int valor = memoria[enderecoFisico];
                    }
                }
            }else{
                frame = tabelaPaginas[numeroPagina];
                enderecoFisico = (frame * PAGE_SIZE) + offset;
                int valor = memoria[enderecoFisico];

                if(entradasTLBUsadas < TAMANHO_TLB){
                        paginaTLB[entradasTLBUsadas] = numeroPagina;
                        frameTLB[entradasTLBUsadas] = frame;
                        tempoTLB[entradasTLBUsadas] = tempo;
                        entradasTLBUsadas++;

                }else{

                    if(algoritmoTLB == 1){
                        int menorTempo = contadorChegadaTLB + 1;
                        int indiceAntigo = -1;
    
                        for(int i = 0; i < TAMANHO_TLB; i++){
                            if(paginaTLB[i] != -1 && ordemChegadaTLB[i] < menorTempo){
                                menorTempo = ordemChegadaTLB[i];
                                indiceAntigo = i;
                            }
                        }
    
                        paginaTLB[indiceAntigo] = numeroPagina;
                        frameTLB[indiceAntigo] = frame;
                        tempoTLB[indiceAntigo] = tempo;
                        ordemChegadaTLB[indiceAntigo] = contadorChegadaTLB++;     

                    }else{
                        int menorTempo = -1;
                        int indiceAntigo = -1;

                        for(int i = 0; i < TAMANHO_TLB; i++){
                            if(paginaTLB[i] != -1){
                                if(indiceAntigo == -1 || tempoTLB[i] < menorTempo){
                                    menorTempo = tempoTLB[i];
                                    indiceAntigo = i;
                                }
                            }
                        }

                        paginaTLB[indiceAntigo] = numeroPagina;
                        frameTLB[indiceAntigo] = frame;
                        tempoTLB[indiceAntigo] = tempo;
                        ordemChegadaTLB[indiceAntigo] = contadorChegadaTLB++;

                    }
                }
            }
        }
    }

    fclose(backingStore);
    fclose(arquivo);    
    return 0;
}