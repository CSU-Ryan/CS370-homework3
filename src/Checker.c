#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>

/**
 * Reads the shared memory ID from the given pipeFD.
 * @param pipe_fd File descriptor for read-end of pipe
 * @return Shared memory ID from the pipe. If errors occur, returns -1.
 */
int get_shm_id(const int pipe_fd) {
    int pipe_output;
    const int bytes_read = read(pipe_fd, &pipe_output, sizeof (int));

    printf("Checker process [%d]: read %d bytes containing shm ID %d.\n",
        getpid(), bytes_read, pipe_output);

    if (bytes_read <= 0) { return -1; }

    return pipe_output;
}

/**
 * Writes the given data to the specified shared memory
 * @param shm_id ID for shared memory
 * @param data data to be written
 * @return 0 on successful write, 1 on fail.
 */
int write_shm(const int shm_id, const int data) {
    int *shm = shmat(shm_id, NULL, 0);
    if (shm == (int *)-1) { return 1; }
    *shm = data;
    shmdt(shm);

    return 0;
}

/**
 * Expects 3 integer arguments: divisor, dividend, and the read-end FD of a pipe.
 * Writes whether divisor divides dividend to shared memory, with ID received from the pipe.
 * @returns Returns 1 if divisor divides dividend, and 0 otherwise
 */
int main(const int argc, char **argv) {
    if (argc != 4) {
        printf("Checker process [%d]: Invalid number of arguments.\n", getpid());
        return 1;
    }

    printf("Checker process [%d]: starting.\n", getpid());

    const int divisor = atoi(argv[1]);
    const int dividend = atoi(argv[2]);
    const int pipe_FD = atoi(argv[3]);

    const int shm_id = get_shm_id(pipe_FD);
    if (shm_id == -1) { return 1; }

    const int divides = (dividend % divisor == 0);

    { // Prints divisibility
        printf("Checker process [%d]: %d *IS",
            getpid(), dividend);

        if (!divides) { printf(" NOT"); }

        printf("* divisible by %d.\n",
            divisor);
    }

    if (write_shm(shm_id, divides)) { return 1; }

    printf("Checker process [%d]: wrote result (%d) to shared memory.\n",
        getpid(), divides);

    return 0;
}
