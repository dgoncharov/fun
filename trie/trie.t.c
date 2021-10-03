#include "trie.h"
#include "ctest.h"
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

static
int run_test (long test)
{
    int rc;
    const char *key;
    void *trie;

    printf ("test %ld\n", test);

    trie = trie_init ();

    switch (test) {
    case 0:
        // Test lookup in an empty trie.
        // Also test trie_print with an empty trie.
        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc == 0);

        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc == 0);

        key = trie_find (trie, "hello.o");
        ASSERT (key == 0, "key = %s\n", key);
        break;
    case 1:
        // Test explicit match.
        trie_push (trie, "hello");
        rc = trie_has (trie, "hello", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello", 1);
        ASSERT (rc);

        key = trie_find (trie, "hello");
        ASSERT (strcmp (key, "hello") == 0, "key = %s\n", key);
        break;
    case 2:
        // Test that an empty string is a valid key.
        rc = trie_has (trie, "", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "");
        ASSERT (key == 0, "key = %s\n", key);
        trie_push (trie, "");
        rc = trie_has (trie, "", 0);
        ASSERT (rc);
        rc = trie_has (trie, "", 1);
        ASSERT (rc);
        key = trie_find (trie, "");
        ASSERT (key && *key == '\0', "key = %s\n", key);
        break;
    case 3:
        // Test various near matches.
        trie_push (trie, "hell");
        trie_push (trie, "helloo");
        trie_push (trie, "hhello");
        trie_push (trie, "hello%");
        trie_push (trie, "he");
        trie_push (trie, "oooohello");
        trie_push (trie, "house");
        trie_push (trie, "this is a very very very very very very long word");
        trie_push (trie, "bell");
        trie_push (trie, "bye");
        trie_push (trie, "h");
        trie_push (trie, "a");

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "hello");
        ASSERT (key == 0, "key = %s\n", key);

        rc = trie_has (trie, "hell", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hell", 1);
        ASSERT (rc);
        key = trie_find (trie, "hell");
        ASSERT (strcmp (key, "hell") == 0, "key = %s\n", key);

        trie_push (trie, "hello");

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello", 1);
        ASSERT (rc);
        key = trie_find (trie, "hello");
        ASSERT (strcmp (key, "hello") == 0, "key = %s\n", key);
        break;
    case 4:
        // A pathological case of near matches.
        // Prefering exact match finds the answer slowly.
        // Prefering fuzzy match finds the answer quickly.
        trie_push (trie, "hella");
        trie_push (trie, "hel%a");
        trie_push (trie, "he%a");
        trie_push (trie, "h%a");
        trie_push (trie, "%a");
        trie_push (trie, "%lo");

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello", 1);
        ASSERT (rc);
        key = trie_find (trie, "hello");
        ASSERT (strcmp (key, "%lo") == 0, "key = %s\n", key);
        break;
    case 5:
        // Test multiple patterns, none of which matches.
        trie_push (trie, "%.x");
        trie_push (trie, "%.q");
        trie_push (trie, "%.z");
        trie_push (trie, "%.u");

        rc = trie_has (trie, "hello.g", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello.g", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "hello.g");
        ASSERT (key == 0, "key = %s\n", key);
        break;
    case 6:
        // Test that explicit match beats pattern match.
        trie_push (trie, "h%llo.o");
        trie_push (trie, "hello.o");
        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "hello.o");
        ASSERT (strcmp (key, "hello.o") == 0, "key = %s\n", key);
        break;
    case 7:
        // Test that pattern % matches any key that has atleast 1 char.
        rc = trie_has (trie, "", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "");
        ASSERT (key == 0, "key = %s\n", key);

        trie_push (trie, "bye.o");
        trie_push (trie, "by%.o");
        trie_push (trie, "%");
        trie_push (trie, "hell.o");

        rc = trie_has (trie, "", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "");
        ASSERT (key == 0, "key = %s\n", key);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "hello.o");
        ASSERT (strcmp (key, "%") == 0, "key = %s\n", key);
        break;
    case 8:
        // Another test that pattern % matches any key that has atleast 1 char.
        rc = trie_has (trie, "", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "");
        ASSERT (key == 0, "key = %s\n", key);

        trie_push (trie, "bye.o");
        trie_push (trie, "by%.o");
        trie_push (trie, "hello.o%");
        trie_push (trie, "hell.o");

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "hello.o");
        ASSERT (key == 0, "key = %s\n", key);

        rc = trie_has (trie, "hello.oo", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.oo", 1);
        ASSERT (rc);
        key = trie_find (trie, "hello.oo");
        ASSERT (strcmp (key, "hello.o%") == 0, "key = %s\n", key);
        break;
    case 9:
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
        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "hello.o");
        ASSERT (strcmp (key, "h%llo.o") == 0, "key = %s\n", key);
        break;
    case 10:
        break;
    case 11:
        // Test that pushing a key more than once has no effect.
        trie_push (trie, "hello.o");
        trie_push (trie, "hello.o");
        trie_push (trie, "hell%.o");
        trie_push (trie, "hell%.o");
        trie_push (trie, "hello.o");
        trie_push (trie, "hell%.o");
        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
         key = trie_find (trie, "hello.o");
        ASSERT (strcmp (key, "hello.o") == 0, "key = %s\n", key);
        break;
    case 12:
        // Test that subsequent lookups find the same key.
        trie_push (trie, "yhello.o");
        trie_push (trie, "hello.oy");
        trie_push (trie, "ello.o");
        trie_push (trie, "%.o");
        trie_push (trie, "hell%.o");

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
         key = trie_find (trie, "hello.o");
        ASSERT (strcmp (key, "hell%.o") == 0, "key = %s\n", key);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "hello.o");
        ASSERT (strcmp (key, "hell%.o") == 0, "key = %s\n", key);
        break;
    case 13: {
        // Test a key longer than 64k.
//TODO: replace recusion with iteration.
        break; // Recursion overflows stack.
        char *key;
        const char *key2;
        const size_t keysz = 64 * 1024 + 5;

        key = malloc(keysz);
        ASSERT (key);
        memset (key, 'y', keysz);
        key[keysz-1] = '\0';

        rc = trie_has (trie, key, 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, key, 1);
        ASSERT (rc == 0);
        key2 = trie_find (trie, key);
        ASSERT (key2 == 0, "key2 = %s\n", key2);

        trie_push (trie, "y%y");

        rc = trie_has (trie, key, 0);
        ASSERT (rc);
        rc = trie_has (trie, key, 1);
        ASSERT (rc);
        key2 = trie_find (trie, key);
        ASSERT (strcmp (key2, "y%y") == 0, "key2 = %s\n", key2);

        trie_push (trie, key);

        rc = trie_has (trie, key, 0);
        ASSERT (rc);
        rc = trie_has (trie, key, 1);
        ASSERT (rc);
        key2 = trie_find (trie, key);
        ASSERT (strcmp (key2, key) == 0, "key = %s, key2 = %s\n", key, key2);

        free (key);
        break;
    }
    case 14:
        // The target contains a naked %.
        trie_push (trie, "hello.o");

        rc = trie_has (trie, "he%lo.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "he%lo.o", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "he%lo.o");
        ASSERT (key == 0, "key = %s\n", key);
        break;
    case 15:
        // A pattern contains an escaped %.
        // The compiler removes the first backslash.
        // Trie uses to seconds backslash to escape the following %.
        // "he%lo.o" is pushed with percent matching '%' only.
        trie_push (trie, "he\\%lo.o");

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "hello.o");
        ASSERT (key == 0, "key = %s\n", key);
        break;
    case 16:
        // A pattern contains an escaped %.
        // The compiler removes the first backslash.
        // Trie uses the seconds backslash to escape the following %.
        // "he%lo.o" is pushed with percent matching '%' only.
        // The target contains a %.
        trie_push (trie, "he\\%lo.o");

        rc = trie_has (trie, "he%lo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he%lo.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "he%lo.o");
        ASSERT (strcmp (key, "he\\%lo.o") == 0, "key = %s\n", key);
        break;
    case 17:
        // A pattern contains an escaped %.
        // The compiler removes the first backslash.
        // Trie uses to seconds backslash to escape the following %.
        // "he%lo.o" is pushed with percent matching '%' only.
        // The target contains a backslash followed by a %.
        trie_push (trie, "he\\%lo.o");

        rc = trie_has (trie, "he\\%lo.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "he\\%lo.o", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "he\\%lo.o");
        ASSERT (key == 0, "key = %s\n", key);
        break;
    case 18:
        // A pattern contains an escaped %.
        // The compiler removes the first backslash.
        // Trie uses to seconds backslash to escape the following %.
        // "he%lo.o" is pushed with percent matching '%' only.
        // The target to lookup is "he\%lo.o", which does not match.
        trie_push (trie, "he\\%lo.o");

        rc = trie_has (trie, "he\\%lo.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "he\\%lo.o", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "he\\%lo.o");
        ASSERT (key == 0, "key = %s\n", key);
        break;
    case 19:
        // A pattern contains an escaped \, followed by %.
        // The compiler removes 2 backslashes and "he\\%lo.o" is pushed.
        // In this pattern trie uses the 1st backslash to escape the second
        // backslash and percent is not escaped.  "he\%lo.o" is pushed.
        // % matches any char(s).
        trie_push (trie, "he\\\\%lo.o");

        rc = trie_has (trie, "he\\alo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\alo.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "he\\alo.o");
        ASSERT (strcmp (key, "he\\\\%lo.o") == 0, "key = %s\n", key);
        break;
    case 20:
        // The pattern contains \\, followed by %.
        // The compiler removes 4 backslashes and passes "he\\\\%lo.o" to
        // trie_push.
        // In this pattern trie uses the 1st backslash to escape the second
        // backslash and the 3rd backslash to escape the 4th backslash and
        // percent is not escaped.  "he\\%lo.o" is pushed.
        // % matches any char(s).
        trie_push (trie, "he\\\\\\\\%lo.o");

        rc = trie_has (trie, "he\\\\alo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\\\alo.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "he\\\\alo.o");
        ASSERT (strcmp (key, "he\\\\\\\\%lo.o") == 0, "key = %s\n", key);
        break;
    case 21:
        // A pattern contains an escaped \, followed by an escaped %.
        // The compiler removes 3 backslashes and passes "he\\\%lo.o" to
        // trie_push.
        // In this pattern trie uses the 1st backslash to escape the second
        // backslash and the 3rd backslash to escape %.  "he\%lo.o" is pushed.
        // % matches '%' only.
        trie_push (trie, "he\\\\\\%lo.o");

        rc = trie_has (trie, "he\\aaalo.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "he\\aaalo.o", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "he\\aaalo.o");
        ASSERT (key == 0, "key = %s\n", key);

        rc = trie_has (trie, "he\\%lo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\%lo.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "he\\%lo.o");
        ASSERT (strcmp (key, "he\\\\\\%lo.o") == 0, "key = %s\n", key);
        break;
    case 22:
        // A sole backslash. Not followed by a %.
        trie_push (trie, "he\\lo.o");

        rc = trie_has (trie, "he\\lo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\lo.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "he\\lo.o");
        ASSERT (strcmp (key, "he\\lo.o") == 0, "key = %s\n", key);
        break;
    case 23:
        // Two backslashes. Not followed by a %.
        trie_push (trie, "he\\\\lo.o");

        rc = trie_has (trie, "he\\\\lo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\\\lo.o", 1);
        ASSERT (rc);
         key = trie_find (trie, "he\\\\lo.o");
        ASSERT (strcmp (key, "he\\\\lo.o") == 0, "key = %s\n", key);
        break;
    case 24:
        // A trailing backslash. Not followed by a %.
        trie_push (trie, "hello.\\");

        rc = trie_has (trie, "hello.\\", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.\\", 1);
        ASSERT (rc);
         key = trie_find (trie, "hello.\\");
        ASSERT (strcmp (key, "hello.\\") == 0, "key = %s\n", key);
        break;
    case 25:
        // Two trailing backslashes. Not followed by a %.
        trie_push (trie, "hello.\\\\");

        rc = trie_has (trie, "hello.\\\\", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.\\\\", 1);
        ASSERT (rc);
        key = trie_find (trie, "hello.\\\\");
        ASSERT (strcmp (key, "hello.\\\\") == 0, "key = %s\n", key);
        break;
    case 26:
        // Multiple naked %.
        rc = trie_push (trie, "he%llo%");
        ASSERT (rc == -1);
        break;
    case 27:
        // One naked and multiple escaped %.
        rc = trie_push (trie, "he%llo\\%.\\%");
        ASSERT (rc == 0);
        break;
    case 28:
        // A key with a slash and a target with a slash.
        rc = trie_push (trie, "obj/hello.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_push (trie, "obj/%.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "obj/hello.o");
        ASSERT (strcmp (key, "obj/hello.o") == 0, "key = %s\n", key);
        break;
    case 29:
        // A key with a slash, a target w/o a slash.
        rc = trie_push (trie, "obj/hello.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "hello.o");
        ASSERT (key == 0, "key = %s\n", key);
        break;
    case 30:
        // A key with a slash and a percent, a target with a slash.
        rc = trie_push (trie, "obj/%.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "obj/hello.o");
        ASSERT (strcmp (key, "obj/%.o") == 0, "key = %s\n", key);
        break;
    case 31:
        // Three keys which match and a  key which does not match, a target
        // with a slash.
        rc = trie_push (trie, "%.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_push (trie, "obj/hello.oo");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_push (trie, "obj/hel%o.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_push (trie, "obj/%.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "obj/hello.o");
        ASSERT (strcmp (key, "obj/hel%o.o") == 0, "key = %s\n", key);
        break;
    case 32:
        // A key w/o a slash and a target with a slash.
        rc = trie_push (trie, "hello.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc == 0);
        key = trie_find (trie, "obj/hello.o");
        ASSERT (key == 0, "key = %s\n", key);
        break;
    case 33:
        // A key with a percent and a target with a slash.
        rc = trie_push (trie, "hello.o");
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "%.o");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc);
        key = trie_find (trie, "obj/hello.o");
        ASSERT (strcmp (key, "%.o") == 0, "key = %s\n", key);
        break;
    case 34:
        // Another pathological case of near matches.
        // Prefering exact match finds the answer quickly.
        // Prefering fuzzy match finds the answer slowly.
        rc = trie_push (trie, "%ella");
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "h%lla");
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "he%la");
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "hel%a");
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "hell%");
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc);

        rc = trie_has (trie, "hello", 1);
        ASSERT (rc);
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
        char *r;
        long test;

        errno = 0;
        test = strtol(argv[1], &r, 0);
        if (errno || r == argv[1]) {
            fprintf(stderr, "usage: %s [test]\n", argv[0]);
            return 1;
        }
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
