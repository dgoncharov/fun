#include "trie.h"
#include "ctest.h"

int main (int argc, char *argv[])
{
    int rc;
    (void) argc;
    (void) argv;
    void *trie = trie_init ();
    rc = trie_find (trie, "hello");
    ASSERT(rc == 0, "rc = %d\n", rc);
    rc = trie_find (trie, "");
    ASSERT(rc == 0, "rc = %d\n", rc);
    trie_push (trie, "hello");
    rc = trie_find (trie, "hello");
    ASSERT(rc == 1, "rc = %d\n", rc);
    trie_push (trie, "hell");
    trie_push (trie, "he");
    trie_push (trie, "oooohello");
    trie_push (trie, "house");
    trie_push (trie, "this is a very very very very very very very very very very very very very long word");
    trie_push (trie, "bell");
    trie_push (trie, "bye");
    trie_push (trie, "h");
    trie_push (trie, "a");

    rc = trie_find (trie, "hello");
    ASSERT(rc == 1, "rc = %d\n", rc);

    rc = trie_find (trie, "hell");
    ASSERT(rc == 1, "rc = %d\n", rc);

    rc = trie_find (trie, "helloo");
    ASSERT(rc == 0, "rc = %d\n", rc);

    rc = trie_find (trie, "");
    ASSERT(rc == 0, "rc = %d\n", rc);
    trie_push (trie, "");
    rc = trie_find (trie, "");
    ASSERT(rc == 1, "rc = %d\n", rc);

    trie_push (trie, "he%.o");
    trie_push (trie, "%.o");
    rc = trie_find (trie, "hello.o");
    ASSERT(rc == 1, "rc = %d\n", rc);


    trie_print (trie);
    trie_free (trie);

    return 0;
}
