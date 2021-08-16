#include "trie.h"
#include "ctest.h"

int main (int argc, char *argv[])
{
    int rc;
    struct trie trie;
    trie_init (&trie);
    trie_push (&trie, "hello");
    rc = trie_find (&trie, "hello");
    ASSERT(rc == 0, "rc = %d\n", rc);

    return 0;
}
