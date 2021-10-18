#include "vector.h"
#include "ctest.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static
void gettime(struct timeval *result)
{
    int rc;
    rc = gettimeofday(result, 0);
    assert (rc == 0);
}

static
suseconds_t timediff (const struct timeval *start, const struct timeval *stop)
{
    suseconds_t result;
    assert (stop->tv_sec >= start->tv_sec);
    assert (stop->tv_sec > start->tv_sec || stop->tv_usec >= start->tv_usec);

    result = (stop->tv_sec - start->tv_sec) * 1000 * 1000;
    result += stop->tv_usec - start->tv_usec;
    return result;
}

static
char random_printable_char ()
{
    return (char) (rand() % 94 + 32);
}

static
void randomize_vector (void *vector, int maxklen, int nkeys)
{
    int k, j;
    char key[maxklen];

    srand (time (0));
    for (; nkeys > 0; nkeys -= 5) {
        int klen = (char) (rand() % maxklen);
        if (klen < 8)
            klen = 8; // Atleast 8 chars long.
        for (k = 0; k < klen - 1; ++k)
            // Only printable chars.
            key[k] = random_printable_char ();
        key[klen-1] = '\0';
        vector_push (vector, key);
        for (j = nkeys > 4 ? 5 : nkeys % 5; j > 1; --j) {
            assert (klen > j+1);
            key[klen-1-j] = random_printable_char ();
            vector_push (vector, key);
        }
    }
}

static
int run_test (long test, int argc, char *argv[])
{
    void *vector = 0;

    printf ("test %ld\n", test);

    switch (test) {
    case -1: {
        // Performance test.
        // vector.t.tsk without arguments does not run this test.
        int m, size;
        const int maxklen = 64; // Max length of a key.
        struct timeval start, stop;
        suseconds_t duration;
        const int nkeys = argc > 2 ? atoi (argv[2]) : 10; // Number of keys.

        vector = vector_init (maxklen*nkeys); // Pass required space.

        gettime (&start);
        randomize_vector (vector, maxklen, nkeys);
        gettime (&stop);
        size = vector_size (vector);

        duration = timediff (&start, &stop);
        printf ("building a vector of size %d took %ldus\n", size, duration);

        gettime (&start);
        for (m = 0; m < nkeys; ++m)
            vector_has (vector, "hello");
        gettime (&stop);

        duration = timediff (&start, &stop);
        printf ("%d lookups in vector of size %d took %ldus\n",
                m, size, duration);
        break;
    }
    default:
        status = -1;
        break;
    }
    vector_free (vector);
    return status;
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
    ASSERT (1, ""); // Suppress the warning about unused vprint.
    // Run all tests.
    for (int k = 0; run_test (k, argc, argv) != -1; ++k)
            ;
    if (status > 0)
        fprintf (stderr, "%d tests failed\n", status);
    if (status == -1)
        status = 0;
    return status;
}
