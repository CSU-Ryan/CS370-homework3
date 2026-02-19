#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Makes current process run Checker code
 */
void executeChecker(const int divisor, const int dividend) {
    char divisorString[16];
    sprintf(divisorString, "%d", divisor);
    char dividendString[16];
    sprintf(dividendString, "%d", dividend);

    execlp("bin/checker.o",
        "bin/checker.o", divisorString, dividendString,
        (char *)NULL);
}

/**
 * Awaits the return value of the Checker child
 */
void receiveCheck(const int childpid) {
    printf("Coordinator: forked process with ID %d.\n", childpid);
    printf("Coordinator: waiting for process [%d].\n", childpid);

    int status;
    waitpid(childpid, &status, 0);

    printf("Coordinator: child process %d returned %d.\n",
        childpid, WEXITSTATUS(status)
        );
}

/**
 * Expects 5 integer arguments: dividend, divisor1, ..., divisor4
 * Runs a Checker process for each divisor, checking whether the divisor divides the dividend.
 */
int main(int argc, char **argv) {
    if (argc != 6) {
        printf("Coordinator: invalid number of arguments (%d =/= 6).\n", argc);
        return 1;
    }

    const int dividend = atoi(argv[1]);
    const int divisors[4] = {atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5])};

    // Loops over divisors
    for (int i = 0; i < 4; i++) {
        const int divisor = divisors[i];

        const int pid = fork();
        if (pid == 0) {
            // Code for the Child
            executeChecker(divisor, dividend);

            printf("Coordinator: failed to execute checker.\n");
            return 1;
        }
        else if (pid > 0) {
            // Code for the Parent
            receiveCheck(pid);
        }
        else {
            // Failed to fork
            printf("Coordinator: could not fork child process.\n");
            return 1;
        }
    }

    printf("Coordinator: exiting.\n");
}