#include "kernel/types.h"
#include "user/user.h"

// Função para consumir CPU sem bloquear o processo
void
delay()
{
    volatile int i;
    for(i = 0; i < 5000000; i++); // Dependendo da velocidade do seu PC, pode aumentar ou diminuir
}

void
worker(char *name, int priority)
{
    // Define a prioridade deste processo
    setpriority(getpid(), priority);

    int progress = 0;
    
    // Roda 100 "ciclos pesados" de CPU
    for(int i = 0; i < 100; i++) {
        delay();
        progress++;
        
        // Imprime apenas a cada 10 ciclos para não sobrecarregar o console do xv6
        if(progress % 10 == 0) {
            printf("[%s] (Classe %d) concluido: %d%%\n", name, priority, progress);
        }
    }
    
    printf(">>> %s TERMINOU! <<<\n", name);
    exit(0); // Filho morre ao terminar
}

int
main()
{
    int pid;
    
    printf("Iniciando Experimento da Loteria (T1)...\n");

    // Cria os 4 processos filhos em um loop para o código ficar mais limpo
    for(int i = 0; i < 4; i++) {
        pid = fork();
        
        if(pid == 0) {
            // Filho
            char name[3] = "P0";
            name[1] += i; // Transforma "P0" em "P1", "P2", "P3" baseado no i
            
            worker(name, i); // Chama o worker passando o nome e a classe de prioridade
        }
    }

    // O processo Pai não faz processamento pesado, ele apenas espera os 4 filhos terminarem
    for(int i = 0; i < 4; i++) {
        wait(0);
    }

    printf("Todos os processos finalizaram. Teste concluido.\n");
    exit(0);
}