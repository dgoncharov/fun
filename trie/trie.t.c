#include "trie.h"
#include "ctest.h"
#include <string.h>

static
int run_test (int test)
{
    int rc;
    const char *key;
    void *trie = trie_init ();


    switch (test) {
    case 0:
        // Test that trie_find finds the pattern with the shortest stem.
        trie_push (trie, "h%.c");
        trie_push (trie, "%e.c");
        trie_push (trie, "hell.c");
        trie_push (trie, "h%.o");
        trie_push (trie, "he%.o");
        trie_push (trie, "h%llo.o");
        trie_push (trie, "hell%.c");
        trie_push (trie, "hel%.o");
        trie_push (trie, "%.o");
        rc = trie_has (trie, "hello.o");
        ASSERT(rc);
        key = trie_find (trie, "hello.o");
        ASSERT(strcmp (key, "h%llo.o") == 0, "key = %s\n", key);
        break;
    case 1:
        // Test explicit match.
        trie_push (trie, "hello");
        rc = trie_has (trie, "hello");
        ASSERT(rc);
        key = trie_find (trie, "hello");
        ASSERT(key, "key = %s\n", key);
        break;
    case 2:
        // Test that an empty string is a valid key.
        rc = trie_has (trie, "");
        ASSERT(rc == 0);
        key = trie_find (trie, "");
        ASSERT(key == 0, "key = %s\n", key);
        trie_push (trie, "");
        rc = trie_has (trie, "");
        ASSERT(rc);
        key = trie_find (trie, "");
        ASSERT(key, "key = %s\n", key);
        break;
    case 3:
        // Test various near matches.
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

        rc = trie_has (trie, "hello");
        ASSERT(rc == 0);
        key = trie_find (trie, "hello");
        ASSERT(key == 0, "key = %s\n", key);

        rc = trie_has (trie, "hell");
        ASSERT(rc);
        key = trie_find (trie, "hell");
        ASSERT(key, "key = %s\n", key);

        trie_push (trie, "hello");

        rc = trie_has (trie, "hello");
        ASSERT(rc);
        key = trie_find (trie, "hello");
        ASSERT(key, "key = %s\n", key);
        break;
    case 4:
        // Test trie_print with an empty trie.
        break;
    case 5:
        // Test multiple patterns, none of which matches.
        trie_push (trie, "%.x");
        trie_push (trie, "%.q");
        trie_push (trie, "%.z");
        trie_push (trie, "%.u");

        rc = trie_has (trie, "hello.g");
        ASSERT(rc == 0);
        key = trie_find (trie, "hello.g");
        ASSERT(key == 0, "key = %s\n", key);
        break;
    case 6:
        // Explicit match beats fuzzy match.
        trie_push (trie, "h%llo.o");
        trie_push (trie, "hello.o");
        rc = trie_has (trie, "hello.o");
        ASSERT(rc);
        key = trie_find (trie, "hello.o");
        ASSERT(strcmp (key, "hello.o") == 0, "key = %s\n", key);
        break;
    case 7:
        // Test that pattern % matches any key that has atleast 1 char.
        rc = trie_has (trie, "");
        ASSERT(rc == 0);
        key = trie_find (trie, "");
        ASSERT(key == 0, "key = %s\n", key);

        trie_push (trie, "bye.o");
        trie_push (trie, "by%.o");
        trie_push (trie, "%");
        trie_push (trie, "hell.o");

        rc = trie_has (trie, "");
        ASSERT(rc == 0);
        key = trie_find (trie, "");
        ASSERT(key == 0, "key = %s\n", key);

        rc = trie_has (trie, "hello.o");
        ASSERT(rc);
        key = trie_find (trie, "hello.o");
        ASSERT(strcmp (key, "%") == 0, "key = %s\n", key);
        break;
    case 8:
        // Test that pushing a key more than once has no effect.
        trie_push (trie, "hello.o");
        trie_push (trie, "hello.o");
        trie_push (trie, "hell%.o");
        trie_push (trie, "hell%.o");
        trie_push (trie, "hello.o");
        trie_push (trie, "hell%.o");
        rc = trie_has (trie, "hello.o");
        ASSERT(rc);
        key = trie_find (trie, "hello.o");
        ASSERT(strcmp (key, "hello.o") == 0, "key = %s\n", key);
        break;
    case 9:
        // Test that subsequent lookups find the same key.
        trie_push (trie, "yhello.o");
        trie_push (trie, "hello.oy");
        trie_push (trie, "ello.o");
        trie_push (trie, "%.o");
        trie_push (trie, "hell%.o");

        rc = trie_has (trie, "hello.o");
        ASSERT(rc);
        key = trie_find (trie, "hello.o");
        ASSERT(strcmp (key, "hell%.o") == 0, "key = %s\n", key);

        rc = trie_has (trie, "hello.o");
        ASSERT(rc);
        key = trie_find (trie, "hello.o");
        ASSERT(strcmp (key, "hell%.o") == 0, "key = %s\n", key);
        break;
    case 14:
        // Test a near match.
        trie_push (trie, "hhello.o");
        trie_push (trie, "hello.oo");
        trie_push (trie, "ello.o");
        trie_push (trie, "hell%.");

        rc = trie_has (trie, "hello.o");
        ASSERT(rc == 0);
        key = trie_find (trie, "hello.o");
        ASSERT(key == 0, "key = %s\n", key);
        break;
    case 15:
        // Test lookup in an empty trie.
        rc = trie_has (trie, "hello.o");
        ASSERT(rc == 0);
        key = trie_find (trie, "hello.o");
        ASSERT(key == 0, "key = %s\n", key);
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
    if (status == -1)
        status = 0;
    return status;
}
