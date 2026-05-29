#include "kernel/types.h"
#include "user/user.h"

// 1. Criamos uma estrutura para enviar pacote de dados pelo pipe
struct proc_info {
    int pid;
    int classe;
};

// Consumir CPU
void delay() {
    volatile int i;
    for(i = 0; i < 5000000; i++);
}

int main() {
    int pids[8];
    int fd[2];
    
    if(pipe(fd) < 0) {
        printf("Erro ao criar o pipe\n");
        exit(1);
    }

    int classes[8] = {0, 0, 0, 1, 1, 2, 2, 3}; 

    printf("\n===== INICIANDO TESTE =====\n\n");

    for(int i = 0; i < 8; i++) {
        pids[i] = fork();
        
        if(pids[i] == 0) {
            // Código do Filho
            setpriority(getpid(), classes[i]);
            
            // 2. Prepara a estrutura com os dados deste processo
            struct proc_info info;
            info.pid = getpid();
            info.classe = classes[i];

            while(1) {
                delay(); 
                
                // Envia a estrutura completa (PID e Classe) pelo pipe
                write(fd[1], &info, sizeof(struct proc_info));
            }
        }
    }

    // Código do Pai
    int count[4] = {0, 0, 0, 0};
    struct proc_info lido;

    // 3. O pai lê a estrutura do pipe
    for(int i = 0; i < 150; i++) {
        read(fd[0], &lido, sizeof(struct proc_info));
        
        if(lido.classe >= 0 && lido.classe <= 3) {
            count[lido.classe]++;
            // Agora mostramos o PID e a Classe juntos
            printf("Sorteio %d: PID=%d | Classe=%d\n", i + 1, lido.pid, lido.classe);
        }
    }

    // Encerra os filhos
    for(int i = 0; i < 8; i++) {
        kill(pids[i]);
        wait(0); 
    }

    // Exibe o placar finalt
    printf("\n===== RESULTADO FINAL==================\n");
    printf("Classe 0: %d vezes\n", count[0]);
    printf("Classe 1: %d vezes\n", count[1]);
    printf("Classe 2: %d vezes\n", count[2]);
    printf("Classe 3: %d vezes\n", count[3]);
    printf("=========================================\n\n");

    exit(0);
}