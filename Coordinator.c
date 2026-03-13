#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define CHECKER_COUNT 4
#define CHECKER_EXE "./checker"


/**
 * Creates shared memory segments for all child processes
 * @param ids list to be filled with shared memory ids
 * @return 0 if success, 1 if failure
 */
int init_memory(int ids[]) {
    for (int i = 0; i < CHECKER_COUNT; i++) {
        const int shm_id = shmget(IPC_PRIVATE, sizeof (int),
            IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

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
void quick_close(int memory_ids[], int pipes[][2]) {
    printf("Coordinator: exiting.\n");

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

    execlp(CHECKER_EXE,
        CHECKER_EXE, divisor_string, dividend_string, pipe_string,
        (char *) NULL);
}

/**
 * Reads single integer from shared memory
 * @param shm_id ID of shared memory segment
 * @param addr where to write data from shm
 * @return 0 on success, 1 on failure
 */
int read_shm(const int shm_id, int *addr) {
    int *shm = shmat(shm_id, NULL, 0);

    if (shm == (int *)-1) {
        printf("Coordinator: failed to attach shared memory.\n");
        return 1;
    }

    *addr = *shm;
    shmdt(shm);

    return 0;
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
        quick_close(shm_ids, pipes);
        return 1;
    }

    // Initialize pipes
    if (init_pipes(pipes)) {
        quick_close(shm_ids, pipes);
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
            // Notes child pid and writes shm id to pipe
            child_pids[i] = pid;
            printf("Coordinator: forked process with ID %d.\n", pid);

            write(pipes[i][1], &shm_ids[i], sizeof (int));
            printf("Coordinator: wrote shm ID %d to pipe (%llu bytes)\n",
                shm_ids[i], sizeof (int));
        }
        else {
            // Code for the Child
            execute_checker(divisor, dividend, pipes[i][0]);

            // Code should be unreachable
            printf("Coordinator Child: failed to execute checker.\n");
            return 1;
        }
    }

    // Wait on checkers
    for (int i = 0; i < CHECKER_COUNT; i++) {
        printf("Coordinator: waiting on child process ID %d...\n", child_pids[i]);

        int return_value;
        // int child_pid = wait(&return_value);
        waitpid(child_pids[i], &return_value, 0);

        int response;
        read_shm(shm_ids[i], &response);

        // Prints divisibility
        printf("Coordinator: result %d read from shared memory: %d is",
            response, dividends[i]);

        if (!response) { printf(" not"); }

        printf(" divisible by %d.\n",
            divisor);
        // End print
    }

    quick_close(shm_ids, pipes);
}
