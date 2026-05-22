#include "kernel/types.h"
#include "user/user.h"

// consumir cpu
void
delay()
{
    volatile int i;
    for(i = 0; i < 5000000; i++); // Dependendo da velocidade do seu PC, pode aumentar ou diminuir
}

void
worker(char *name, int priority)
{
    // define a prioridade do processo
    setpriority(getpid(), priority);

    int progress = 0;
    
    // simula um trabalho pesado, consumindo CPU e imprimindo o progresso
    for(int i = 0; i < 100; i++) {
        delay();
        progress++;
        
        // imrpime apenas de 10 em 10 ciclos
        if(progress % 10 == 0) {
            printf("[%s] (Classe %d) concluido: %d%%\n", name, priority, progress);
        }
    }
    
    printf(">>> %s TERMINOU! <<<\n", name);
    exit(0); // mata o filho
}

int
main()
{
    int pid;
    
    printf("Iniciando Experimento da Loteria (T1)...\n");

    // cria 4 processos filhos cada um com uma classe diferente
    for(int i = 0; i < 4; i++) {
        pid = fork();
        
        if(pid == 0) {
            // filho
            char name[3] = "P0";
            name[1] += i; // gera os nomes P0, P1, P2, P3
            
            worker(name, i); // chama o worker
        }
    }

    // pai espera os filhos terminarem
    for(int i = 0; i < 4; i++) {
        wait(0);
    }

    printf("Todos os processos finalizaram. Teste concluido.\n");
    exit(0);
}