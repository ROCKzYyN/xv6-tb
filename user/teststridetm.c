#include "kernel/types.h"
#include "user/user.h"

#define NCHILD 3

// Estrutura para não misturar os resultados no pipe
struct proc_info {
    int idx;
    int work;
};

int main() {
    int tickets[NCHILD] = {100, 50, 250};
    int pids[NCHILD];
    int fd[2];

    if(pipe(fd) < 0) {
        printf("Erro ao criar o pipe\n");
        exit(1);
    }

    printf("\n===== TESTE STRIDE SCHEDULING (CPU-BOUND) =====\n");
    printf("Aguarde alguns segundos...\n\n");

    int total_tickets = 0;
    for(int i = 0; i < NCHILD; i++) {
        total_tickets += tickets[i];
    }

    for(int i = 0; i < NCHILD; i++) {
        pids[i] = fork();

        if(pids[i] == 0) {
            settickets(getpid(), tickets[i]);

            int start_time = uptime();
            int current_time = start_time;
            int work_counter = 0; // Usando int normal agora

            // Roda em CPU pura por 300 ticks
            while(current_time - start_time < 400) {
                // O volatile força a CPU a fazer o loop em vez de otimizá-lo,
                // gastando tempo sem aumentar o counter astronomicamente.
                for(volatile int j = 0; j < 500; j++); 
                
                work_counter++;
                current_time = uptime();
            }

            // Empacota o resultado com a "assinatura" do filho
            struct proc_info info;
            info.idx = i;
            info.work = work_counter;
            
            write(fd[1], &info, sizeof(struct proc_info));
            exit(0);
        }
    }

    for(int i = 0; i < NCHILD; i++) {
        wait(0);
    }

    // Coleta os resultados de forma segura
    int results[NCHILD] = {0};
    int total_work = 0;

    for(int i = 0; i < NCHILD; i++) {
        struct proc_info lido;
        read(fd[0], &lido, sizeof(struct proc_info));
        
        // Salva o trabalho na posição correta do array, não importando a ordem
        results[lido.idx] = lido.work;
        total_work += lido.work;
    }

    printf("===== RESULTADO FINAL =====\n");
    for(int i = 0; i < NCHILD; i++) {
        int obtido = (results[i] * 100) / total_work;
        int esperado = (tickets[i] * 100) / total_tickets;
        printf("Filho %d: %d bilhetes | trabalho: %d | obtido: %d%% | esperado: %d%%\n",
               i, tickets[i], results[i], obtido, esperado);
    }
    printf("===========================\n\n");

    exit(0);
}