#include "libsa.h"
#include "ctest.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>



static
int run_test (long test, int argc, char *argv[])
{
    int retcode;
    const char input[] = "hello, world";
    size_t sa[sizeof(input)];

    printf ("test %ld\n", test);
    retcode = 0;

    switch (test) {
    case 0:
        libsa_build(&sa, input);
        break;
    default:
        retcode = -1;
        break;
    }
    return retcode;
}

int main (int argc, char *argv[])
{
    if (argc >= 2) {
        // Run the specified test.
        char *r;
        long test;

        errno = 0;
        test = strtol(argv[1], &r, 0);
        if (errno || r == argv[1]) {
            fprintf(stderr, "usage: %s [test] [test arg]...\n", argv[0]);
            return 1;
        }
        run_test (test, argc, argv);
        if (status > 0)
            fprintf (stderr, "%d tests failed\n", status);
        return status;
    }
    // Run all tests.
    for (int k = 0; run_test (k, argc, argv) != -1; ++k)
            ;
    if (status > 0)
        fprintf (stderr, "%d tests failed\n", status);
    return status;
}
