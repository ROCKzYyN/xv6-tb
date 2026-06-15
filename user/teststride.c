#include "kernel/types.h"
#include "user/user.h"

// Teste do escalonador por stride (passos largos).
//
// Cria N processos com quantidades diferentes de bilhetes (tickets) e mede
// quantas vezes cada um conseguiu rodar. Como o stride escolhe sempre o
// processo de menor passada e a passada cresce com stride = STRIDE1 / tickets,
// quem tem mais bilhetes roda proporcionalmente mais vezes. Espera-se que a
// fração de CPU de cada processo se aproxime de tickets_i / soma(tickets).
//
// IMPORTANTE: para observar a proporção é preciso haver DISPUTA por CPU, ou
// seja, mais processos prontos do que CPUs. Como há 3 filhos, rode o xv6 com
// uma única CPU para ver o resultado determinístico da especificação:
//     make qemu CPUS=1
// Com várias CPUs e poucos filhos, cada filho roda em um núcleo diferente em
// paralelo e todos recebem fatias parecidas, escondendo o efeito dos bilhetes.

#define NCHILD  3       // número de processos filhos
#define SAMPLES 400     // número de amostras lidas pelo pai

// Pacote enviado pelo pipe identificando quem rodou.
struct proc_info {
    int pid;
    int idx;        // índice do filho (0..NCHILD-1)
    int tickets;
};

// Consome CPU para que o filho gaste uma fatia de tempo antes de reportar.
void delay() {
    volatile int i;
    for(i = 0; i < 5000000; i++);
}

int main() {
    int tickets[NCHILD] = {100, 50, 250}; // exemplo da especificação (A, B, C)
    int pids[NCHILD];
    int fd[2];

    if(pipe(fd) < 0) {
        printf("Erro ao criar o pipe\n");
        exit(1);
    }

    printf("\n===== TESTE STRIDE SCHEDULING =====\n\n");

    int total_tickets = 0;
    for(int i = 0; i < NCHILD; i++)
        total_tickets += tickets[i];

    for(int i = 0; i < NCHILD; i++) {
        pids[i] = fork();

        if(pids[i] == 0) {
            // Código do filho: define seus bilhetes e entra no laço de trabalho.
            settickets(getpid(), tickets[i]);

            struct proc_info info;
            info.pid = getpid();
            info.idx = i;
            info.tickets = tickets[i];

            while(1) {
                delay();
                write(fd[1], &info, sizeof(struct proc_info));
            }
        }
    }

    // Código do pai: lê amostras e conta quantas vezes cada filho reportou.
    int count[NCHILD] = {0};
    struct proc_info lido;

    for(int i = 0; i < SAMPLES; i++) {
        read(fd[0], &lido, sizeof(struct proc_info));
        if(lido.idx >= 0 && lido.idx < NCHILD)
            count[lido.idx]++;
    }

    // Encerra os filhos.
    for(int i = 0; i < NCHILD; i++) {
        kill(pids[i]);
        wait(0);
    }

    // Placar final: comparação entre a fração obtida e a esperada.
    printf("\n===== RESULTADO FINAL =====\n");
    printf("Total de bilhetes: %d | amostras: %d\n\n", total_tickets, SAMPLES);
    for(int i = 0; i < NCHILD; i++) {
        int obtido = (count[i] * 100) / SAMPLES;
        int esperado = (tickets[i] * 100) / total_tickets;
        printf("Filho %d (PID %d): %d bilhetes | rodou %d vezes | obtido %d%% | esperado %d%%\n",
               i, pids[i], tickets[i], count[i], obtido, esperado);
    }
    printf("===========================\n\n");

    exit(0);
}
