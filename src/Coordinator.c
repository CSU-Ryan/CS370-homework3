#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define CHECKER_COUNT 4


/**
 * Creates shared memory segments for all child processes
 * @param ids list to be filled with shared memory ids
 * @return 0 if success, 1 if failure
 */
int init_memory(int ids[]) {
    for (int i = 0; i < CHECKER_COUNT; i++) {
        const int shm_id = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT);

        if (shm_id < 0) {
            printf("Coordinator: shmget failed.\n");
            return 1;
        }

        ids[i] = shm_id;
    }
    return 0;
}

/**
 * Closes shared memory segments.
 * @param ids shared memory ids
 */
void clean_memory(const int ids[]) {
    for (int i = 0; i < CHECKER_COUNT; i++) {
        shmctl(ids[i], IPC_RMID, NULL);
    }
}

/**
 * Creates pipes
 * @param pipes array to have pipe FDs written to
 * @return 0 on success, 1 on error
 */
int init_pipes(int pipes[][2]) {
    for (int i = 0; i < CHECKER_COUNT; i++) {
        if (pipe(pipes[i])) {
            printf("Coordinator: pipe failed.\n");
            return 1;
        }
    }
    return 0;
}

/**
 * Closes read-end of pipes.
 * @param pipes List of pipe FDs
 * @warning Only use when pipes have not been given to other threads.
 */
void clean_pipe_readers(int pipes[][2]) {
    for (int i = 0; i < CHECKER_COUNT; i++) {
        close(pipes[i][0]);
    }
}

/**
 * Closes write-end of pipes.
 * @param pipes List of pipe FDs
 */
void clean_pipe_writers(int pipes[][2]) {
    for (int i = 0; i < CHECKER_COUNT; i++) {
        close(pipes[i][1]);
    }
}

/**
 * Closes all pipes and allocated memory
 * @param memory_ids Shared memory id list
 * @param pipes Pipe FD list
 * @warning Only use when pipes have not been given to other threads.
 */
void quick_exit(int memory_ids[], int pipes[][2]) {
    clean_memory(memory_ids);
    clean_pipe_readers(pipes);
    clean_pipe_writers(pipes);
}


/**
 * Makes current process run Checker code
 */
void execute_checker(const int divisor, const int dividend, const int pipe_fd) {
    char divisor_string[16];
    sprintf(divisor_string, "%d", divisor);
    char dividend_string[16];
    sprintf(dividend_string, "%d", dividend);
    char pipe_string[16];
    sprintf(pipe_string, "%d", pipe_fd);

    execlp("bin/checker.o",
        "bin/checker.o", divisor_string, dividend_string, pipe_string,
        (char *) NULL);
}

void write_pipe(const int write_fd, int data, int size) {

}

/**
 * Expects 5 integer arguments: divisor, dividend1, ..., dividend4
 * Runs a Checker process for each dividend, checking whether the divisor divides the dividend.
 */
int main(const int argc, char **argv) {
    if (argc != CHECKER_COUNT + 2) {
        printf("Coordinator: invalid number of arguments.\n");
        return 1;
    }

    const int divisor = atoi(argv[1]);
    int dividends[CHECKER_COUNT];
    for (int i = 0; i < CHECKER_COUNT; i++) {
        dividends[i] = atoi(argv[i + 2]);
    }

    int child_pids[CHECKER_COUNT];
    int shm_ids[CHECKER_COUNT];
    int pipes[CHECKER_COUNT][2];

    // Initialize shared memory
    if (init_memory(shm_ids)) {
        quick_exit(shm_ids, pipes);
        return 1;
    }

    // Initialize pipes
    if (init_pipes(pipes)) {
        quick_exit(shm_ids, pipes);
        return 1;
    }

    // Checker creation loop
    for (int i = 0; i < CHECKER_COUNT; i++) {
        const int dividend = dividends[i];
        const int pid = fork();

        if (pid < 0) {
            // Failed to fork
            printf("Coordinator: could not fork child process.\n");
            continue;
        }

        if (pid > 0) {
            // Code for the Parent
            child_pids[i] = pid;
            printf("Coordinator: forked process with ID %d.\n", pid);

            write(pipes[i][1], &shm_ids[i], sizeof shm_ids[i]);
            printf("Coordinator: wrote shm ID %d to pipe (%llu bytes)",
                shm_ids[i], sizeof shm_ids[i]);
        }
        else {
            // Code for the Child
            execute_checker(divisor, dividend, pipes[i][0]);

            // Code should be unreachable
            printf("Coordinator Child: failed to execute checker.\n");
        }
    }

    for (int i = 0; i < CHECKER_COUNT; i++) {
        printf("Coordinator: waiting on child process ID %d...\n", child_pids[i]);

        int _;
        waitpid(child_pids[i], &_, 0);

        const int *shared_address = shmat(shm_ids[i], NULL, 0);
        const int result = *shared_address;

        printf("Coordinator: result %d read from shared memory: %d is divisible by %d.\n",
            result, dividends[i], divisor);
    }


    printf("Coordinator: exiting.\n");
}