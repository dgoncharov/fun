#include "trie.h"
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
void randomize_trie (void *trie, int maxklen, int nkeys)
{
    int k, j;
    char key[maxklen];
    const char userdata[] = "hello, world";

    srand (time (0));
    for (; nkeys > 0; nkeys -= 5) {
        int klen = (char) (rand() % maxklen);
        if (klen < 8)
            klen = 8; // Atleast 8 chars long.
        for (k = 0; k < klen - 1; ++k)
            key[k] = random_printable_char (); // Only printable chars.
        key[klen-1] = '\0';
        trie_push (trie, key, userdata);
        // Push up to 4 more keys with the same prefix.
        for (j = nkeys > 4 ? 5 : nkeys % 5; j > 1; --j) {
            assert (klen > j+1);
            key[klen-1-j] = random_printable_char ();
            trie_push (trie, key, userdata);
        }
    }
}

static
int run_test (long test, int argc, char *argv[])
{
    int rc, size, retcode;
    const struct result *found;
    void *trie;
    const char userdata[] = "hello, world";

    printf ("test %ld\n", test);
    retcode = 0;

    const int maxklen = 64; // Max length of a key.
    const int nkeys = argc > 2 ? atoi (argv[2]) : 16;
    trie = trie_init (nkeys, maxklen * nkeys, 1);
    size = trie_size (trie);
    ASSERT (size == 0, "size = %d\n", size);

    switch (test) {
    case 0:
        // Test lookup in an empty trie.
        // Also test trie_print with an empty trie.
        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc == 0);

        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc == 0);

        found = trie_find (trie, "hello.o");
        ASSERT (found == 0, "found = %p\n", found);
        break;
    case 1:
        // Test explicit match.
        trie_push (trie, "hello", userdata);
        size = trie_size (trie);
        ASSERT (size == 1, "size = %d\n", size);

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello", 1);
        ASSERT (rc);

        found = trie_find (trie, "hello");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hello") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 2:
        // Test that an empty string is a valid key.
        rc = trie_has (trie, "", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "");
        ASSERT (found == 0, "found = %p\n", found);
        trie_push (trie, "", userdata);
        rc = trie_has (trie, "", 0);
        ASSERT (rc);
        rc = trie_has (trie, "", 1);
        ASSERT (rc);
        found = trie_find (trie, "");
        ASSERT (found, "found = %p\n", found);
        ASSERT (found->key && *found->key == '\0', "found->key = %s\n", found->key);
        break;
    case 3:
        // Test various near matches.
        trie_push (trie, "hell", userdata);
        trie_push (trie, "helloo", userdata);
        trie_push (trie, "hhello", userdata);
        trie_push (trie, "hello%", userdata);
        trie_push (trie, "he", userdata);
        trie_push (trie, "oooohello", userdata);
        trie_push (trie, "house", userdata);
        trie_push (trie, "this is a very very very very very very long word", userdata);
        trie_push (trie, "bell", userdata);
        trie_push (trie, "bye", userdata);
        trie_push (trie, "h", userdata);
        trie_push (trie, "a", userdata);
        size = trie_size (trie);
        ASSERT (size == 12, "size = %d\n", size);

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "hello");
        ASSERT (found == 0, "found = %p\n", found);

        rc = trie_has (trie, "hell", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hell", 1);
        ASSERT (rc);
        found = trie_find (trie, "hell");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hell") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);

        trie_push (trie, "hello", userdata);

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hello") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 4:
        // A pathological case of near matches.
        // Prefering exact match finds the answer slowly.
        // Prefering fuzzy match finds the answer quickly.
        trie_push (trie, "hella", userdata);
        trie_push (trie, "hel%a", userdata);
        trie_push (trie, "he%a", userdata);
        trie_push (trie, "h%a", userdata);
        trie_push (trie, "%a", userdata);
        trie_push (trie, "%lo", userdata);

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "%lo") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 5:
        // Test multiple patterns, none of which matches.
        trie_push (trie, "%.x", userdata);
        trie_push (trie, "%.q", userdata);
        trie_push (trie, "%.z", userdata);
        trie_push (trie, "%.u", userdata);

        rc = trie_has (trie, "hello.g", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello.g", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "hello.g");
        ASSERT (found == 0, "found = %p\n", found);
        break;
    case 6:
        // Test that explicit match beats pattern match.
        trie_push (trie, "h%llo.o", userdata);
        trie_push (trie, "hello.o", userdata);
        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hello.o") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 7:
        // Test that pattern % matches any key that has atleast 1 char.
        rc = trie_has (trie, "", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "");
        ASSERT (found == 0, "found = %p\n", found);

        trie_push (trie, "bye.o", userdata);
        trie_push (trie, "by%.o", userdata);
        trie_push (trie, "%", userdata);
        trie_push (trie, "hell.o", userdata);

        rc = trie_has (trie, "", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "");
        ASSERT (found == 0, "found = %p\n", found);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "%") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 8:
        // Another test that pattern % matches any key that has atleast 1 char.
        rc = trie_has (trie, "", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "");
        ASSERT (found == 0, "found = %p\n", found);

        trie_push (trie, "bye.o", userdata);
        trie_push (trie, "by%.o", userdata);
        trie_push (trie, "hello.o%", userdata);
        trie_push (trie, "hell.o", userdata);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "hello.o");
        ASSERT (found == 0, "found = %p\n", found);

        rc = trie_has (trie, "hello.oo", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.oo", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello.oo");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hello.o%") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 9:
        // Test that trie_find finds the pattern with the shortest stem.
        trie_push (trie, "h%.c", userdata);
        trie_push (trie, "%e.c", userdata);
        trie_push (trie, "hell.c", userdata);
        trie_push (trie, "h%.o", userdata);
        trie_push (trie, "he%.o", userdata);
        trie_push (trie, "h%llo.o", userdata);
        trie_push (trie, "hell%.c", userdata);
        trie_push (trie, "hel%.o", userdata);
        trie_push (trie, "%.o", userdata);
        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "h%llo.o") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 10:
        break;
    case 11:
        // Test that pushing a key more than once has no effect.
        trie_push (trie, "hello.o", userdata);
        trie_push (trie, "hello.o", userdata);
        trie_push (trie, "hell%.o", userdata);
        trie_push (trie, "hell%.o", userdata);
        trie_push (trie, "hello.o", userdata);
        trie_push (trie, "hell%.o", userdata);
        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hello.o") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 12:
        // Test that subsequent lookups find the same key.
        trie_push (trie, "yhello.o", userdata);
        trie_push (trie, "hello.oy", userdata);
        trie_push (trie, "ello.o", userdata);
        trie_push (trie, "%.o", userdata);
        trie_push (trie, "hell%.o", userdata);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hell%.o") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hell%.o") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 13: {
        // Test a key longer than 64k.
        char *key;
        const struct result *found;
        const size_t keysz = 64 * 1024 + 5;

        trie_free (trie);
        trie = trie_init (2, 2*keysz, 0);

        key = malloc(keysz);
        ASSERT (key);
        memset (key, 'y', keysz);
        key[keysz-1] = '\0';


//        rc = trie_has (trie, key, 0);
//        ASSERT (rc == 0);
        rc = trie_has (trie, key, 1);
        ASSERT (rc == 0);
        found = trie_find (trie, key);
        ASSERT (found == 0, "found = %p\n", found);

        trie_push (trie, "y%y", userdata);

//        rc = trie_has (trie, key, 0);
//        ASSERT (rc);
        rc = trie_has (trie, key, 1);
        ASSERT (rc);
        found = trie_find (trie, key);
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "y%y") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);

        trie_push (trie, found->key, userdata);

//        rc = trie_has (trie, key, 0);
//        ASSERT (rc);
        rc = trie_has (trie, key, 1);
        ASSERT (rc);
        found = trie_find (trie, key);
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "y%y") == 0, "found->key = %s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);

        free (key);
        break;
    }
    case 14:
        // The target contains a naked %.
        trie_push (trie, "hello.o", userdata);

        rc = trie_has (trie, "he%lo.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "he%lo.o", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "he%lo.o");
        ASSERT (found == 0, "found = %p\n", found);
        break;
    case 15:
        // A pattern contains an escaped %.
        // The compiler removes the first backslash.
        // Trie uses to seconds backslash to escape the following %.
        // "he%lo.o" is pushed with percent matching '%' only.
        trie_push (trie, "he\\%lo.o", userdata);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "hello.o");
        ASSERT (found == 0, "found = %p\n", found);
        break;
    case 16:
        // A pattern contains an escaped %.
        // The compiler removes the first backslash.
        // Trie uses the seconds backslash to escape the following %.
        // "he%lo.o" is pushed with percent matching '%' only.
        // The target contains a %.
        trie_push (trie, "he\\%lo.o", userdata);

        rc = trie_has (trie, "he%lo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he%lo.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "he%lo.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "he\\%lo.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 17:
        // A pattern contains an escaped %.
        // The compiler removes the first backslash.
        // Trie uses to seconds backslash to escape the following %.
        // "he%lo.o" is pushed with percent matching '%' only.
        // The target contains a backslash followed by a %.
        trie_push (trie, "he\\%lo.o", userdata);

        rc = trie_has (trie, "he\\%lo.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "he\\%lo.o", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "he\\%lo.o");
        ASSERT (found == 0, "found = %p\n", found);
        break;
    case 18:
        // A pattern contains an escaped %.
        // The compiler removes the first backslash.
        // Trie uses to seconds backslash to escape the following %.
        // "he%lo.o" is pushed with percent matching '%' only.
        // The target to lookup is "he\%lo.o", which does not match.
        trie_push (trie, "he\\%lo.o", userdata);

        rc = trie_has (trie, "he\\%lo.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "he\\%lo.o", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "he\\%lo.o");
        ASSERT (found == 0, "found = %p\n", found);
        break;
    case 19:
        // A pattern contains an escaped \, followed by %.
        // The compiler removes 2 backslashes and "he\\%lo.o" is pushed.
        // In this pattern trie uses the 1st backslash to escape the second
        // backslash and percent is not escaped.  "he\%lo.o" is pushed.
        // % matches any char(s).
        trie_push (trie, "he\\\\%lo.o", userdata);

        rc = trie_has (trie, "he\\alo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\alo.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "he\\alo.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "he\\\\%lo.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 20:
        // The pattern contains \\, followed by %.
        // The compiler removes 4 backslashes and passes "he\\\\%lo.o" to
        // trie_push.
        // In this pattern trie uses the 1st backslash to escape the second
        // backslash and the 3rd backslash to escape the 4th backslash and
        // percent is not escaped.  "he\\%lo.o" is pushed.
        // % matches any char(s).
        trie_push (trie, "he\\\\\\\\%lo.o", userdata);

        rc = trie_has (trie, "he\\\\alo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\\\alo.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "he\\\\alo.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "he\\\\\\\\%lo.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 21:
        // A pattern contains an escaped \, followed by an escaped %.
        // The compiler removes 3 backslashes and passes "he\\\%lo.o" to
        // trie_push.
        // In this pattern trie uses the 1st backslash to escape the second
        // backslash and the 3rd backslash to escape %.  "he\%lo.o" is pushed.
        // % matches '%' only.
        trie_push (trie, "he\\\\\\%lo.o", userdata);

        rc = trie_has (trie, "he\\aaalo.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "he\\aaalo.o", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "he\\aaalo.o");
        ASSERT (found == 0, "found = %p\n", found);

        rc = trie_has (trie, "he\\%lo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\%lo.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "he\\%lo.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "he\\\\\\%lo.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 22:
        // A sole backslash. Not followed by a %.
        trie_push (trie, "he\\lo.o", userdata);

        rc = trie_has (trie, "he\\lo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\lo.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "he\\lo.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "he\\lo.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 23:
        // Two backslashes. Not followed by a %.
        trie_push (trie, "he\\\\lo.o", userdata);

        rc = trie_has (trie, "he\\\\lo.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "he\\\\lo.o", 1);
        ASSERT (rc);
         found = trie_find (trie, "he\\\\lo.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "he\\\\lo.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 24:
        // A trailing backslash. Not followed by a %.
        trie_push (trie, "hello.\\", userdata);

        rc = trie_has (trie, "hello.\\", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.\\", 1);
        ASSERT (rc);
         found = trie_find (trie, "hello.\\");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hello.\\") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 25:
        // Two trailing backslashes. Not followed by a %.
        trie_push (trie, "hello.\\\\", userdata);

        rc = trie_has (trie, "hello.\\\\", 0);
        ASSERT (rc);
        rc = trie_has (trie, "hello.\\\\", 1);
        ASSERT (rc);
        found = trie_find (trie, "hello.\\\\");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "hello.\\\\") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 26:
        // Multiple naked %.
        rc = trie_push (trie, "he%llo%", userdata);
        ASSERT (rc == -1);
        break;
    case 27:
        // One naked and multiple escaped %.
        rc = trie_push (trie, "he%llo\\%.\\%", userdata);
        ASSERT (rc == 0);
        break;
    case 28:
        // A key with a slash and a target with a slash.
        rc = trie_push (trie, "obj/hello.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_push (trie, "obj/%.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "obj/hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "obj/hello.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 29:
        // A key with a slash, a target w/o a slash.
        rc = trie_push (trie, "obj/hello.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "hello.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "hello.o", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "hello.o");
        ASSERT (found == 0, "found = %p\n", found);
        break;
    case 30:
        // A key with a slash and a percent, a target with a slash.
        rc = trie_push (trie, "obj/%.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "obj/hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "obj/%.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 31:
        // Three keys which match and a  key which does not match, a target
        // with a slash.
        rc = trie_push (trie, "%.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_push (trie, "obj/hello.oo", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_push (trie, "obj/hel%o.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_push (trie, "obj/%.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "obj/hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "obj/hel%o.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 32:
        // A key w/o a slash and a target with a slash.
        rc = trie_push (trie, "hello.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc == 0);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc == 0);
        found = trie_find (trie, "obj/hello.o");
        ASSERT (found == 0, "found = %p\n", found);
        break;
    case 33:
        // A key with a percent and a target with a slash.
        rc = trie_push (trie, "hello.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "%.o", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "obj/hello.o", 0);
        ASSERT (rc);
        rc = trie_has (trie, "obj/hello.o", 1);
        ASSERT (rc);
        found = trie_find (trie, "obj/hello.o");
        ASSERT (found, "found = %p\n", found);
        ASSERT (strcmp (found->key, "%.o") == 0, "found->key =%s\n", found->key);
        ASSERT (found->userdata == userdata, "found->userdata = %p\n", found->userdata);
        break;
    case 34:
        // Another pathological case of near matches.
        // Prefering exact match finds the answer quickly.
        // Prefering fuzzy match finds the answer slowly.
        rc = trie_push (trie, "%ella", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "h%lla", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "he%la", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "hel%a", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);
        rc = trie_push (trie, "hell%", userdata);
        ASSERT (rc == 0, "rc = %d\n", rc);

        rc = trie_has (trie, "hello", 0);
        ASSERT (rc);

        rc = trie_has (trie, "hello", 1);
        ASSERT (rc);
        break;
    case -1: {
        // Performance test.
        // trie.t.tsk without arguments does not run this test.
        int k, m, size;
        struct timeval start, stop;
        suseconds_t duration;

        gettime (&start);
        randomize_trie (trie, maxklen, nkeys);
        gettime (&stop);
        size = trie_size (trie);

        duration = timediff (&start, &stop);
        printf ("building a trie of size %d took %ldus\n", size, duration);

        for (k = 0; k < 2; ++k) {
            gettime (&start);
            for (m = 0; m < nkeys; ++m)
                trie_has (trie, "hello", k);
            gettime (&stop);

            duration = timediff (&start, &stop);
            printf ("%d lookups in trie of %d took %ldus, prefer fuzzy = %d\n",
                    m, size, duration, k);
        }
        break;
    }
    default:
        retcode = -1;
        break;
    }
    if (test >= 0)
        trie_print (trie);
    trie_free (trie);
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
