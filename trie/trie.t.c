#include "trie.h"
#include "ctest.h"

static
int run_test (int test)
{
    int rc;
    void *trie = trie_init ();
    switch (test) {
    case 0:
        trie_push (trie, "%e.c");
        trie_push (trie, "hell.c");
        trie_push (trie, "%.o");
        rc = trie_find (trie, "hello.o");
        ASSERT(rc == 1, "rc = %d\n", rc);
        break;
    case 1:
        rc = trie_find (trie, "hello");
        ASSERT(rc == 0, "rc = %d\n", rc);
        trie_push (trie, "hello");
        rc = trie_find (trie, "hello");
        ASSERT(rc == 1, "rc = %d\n", rc);
        break;
    case 2:
        rc = trie_find (trie, "");
        ASSERT(rc == 0, "rc = %d\n", rc);
        trie_push (trie, "");
        rc = trie_find (trie, "");
        ASSERT(rc == 1, "rc = %d\n", rc);
        break;
    case 3:
        trie_push (trie, "hell");
        trie_push (trie, "helloo");
        trie_push (trie, "he");
        trie_push (trie, "oooohello");
        trie_push (trie, "house");
        trie_push (trie, "this is a very very very very very very very very very very very very very long word");
        trie_push (trie, "bell");
        trie_push (trie, "bye");
        trie_push (trie, "h");
        trie_push (trie, "a");

        rc = trie_find (trie, "hello");
        ASSERT(rc == 0, "rc = %d\n", rc);
        rc = trie_find (trie, "hell");
        ASSERT(rc == 1, "rc = %d\n", rc);
        trie_push (trie, "hello");
        rc = trie_find (trie, "hello");
        ASSERT(rc == 1, "rc = %d\n", rc);
        break;
    case 4:
        // Test trie_print with an empty trie.
        break;
    default:
        status = -1;
        break;
    }
    trie_print (trie);
    trie_free (trie);
    return status;
}

int main (int argc, char *argv[])
{
    if (argc >= 2) {
        // Run the specified test.
        int test = atoi (argv[1]);
        run_test (test);
        if (status > 0)
            fprintf (stderr, "%d tests failed\n", status);
        return status;
    }
    // Run all tests.
    for (int k = 0; run_test (k) != -1; ++k)
            ;
    if (status > 0)
        fprintf (stderr, "%d tests failed\n", status);
    return status;
}
