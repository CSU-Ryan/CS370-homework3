#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>

/**
 * Reads the shared memory ID from the given pipeFD.
 * @param pipeFD File descriptor for read-end of pipe
 * @return Shared memory ID from the pipe. If errors occur, returns -1.
 */
int get_shm_id(const int pipeFD) {
    int pipe_read;
    if (read(pipeFD, &pipe_read, sizeof (int)) <= 0) { return -1; }

    return pipe_read;
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

    const int divisor = atoi(argv[1]);
    const int dividend = atoi(argv[2]);
    const int pipe_FD = atoi(argv[3]);

    int divides = (dividend % divisor == 0);

    const int shm_id = get_shm_id(pipe_FD);
    if (shm_id == -1) { return 1; }

    return write_shm(shm_id, divides);
}
