#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/**
 * Expects 2 integer arguments: divisor and dividend.
 * Returns 1 if divisor divides dividend, and 0 otherwise
 */
int main(const int argc, char **argv) {
    if (argc != 3) {
        printf("Checker process [%d]: Invalid number of arguments (%d =/= 2).\n", getpid(), argc);
        return 1;
    }

    const int divisor = atoi(argv[1]);
    const int dividend = atoi(argv[2]);

    printf("Checker process [%d]: Starting.\n", getpid());

    int divides = (dividend % divisor == 0);

    if (divides) {
        printf("Checker process [%d]: %d *IS* divisible by %d.\n",
        getpid(),
        dividend,
        divisor
        );
    } else {
        printf("Checker process [%d]: %d *IS NOT* divisible by %d.\n",
        getpid(),
        dividend,
        divisor
        );
    }

    printf("Checker process [%d]: Returning %d.\n", getpid(), divides);

    return divides;
}
