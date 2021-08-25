#include "trie.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum {asciisz = 128};
struct trie {
    struct trie *next[asciisz];
    unsigned end:1; // When set this node is the end of a word.
    unsigned pattern:1; // When set the tree contains a %.
};

void *trie_init (void)
{
    return calloc (1, sizeof (struct trie));
}

int trie_free (void *trie)
{
    struct trie *tr = trie;
    for (size_t k = 0; k < asciisz; ++k)
        if (tr->next[k])
            trie_free (tr->next[k]);
    free (trie);
    return 0;
}

int trie_push (void *trie, const char *key)
{
    struct trie *root = trie;
    struct trie *tr = trie;
    int index;
    for (const char *k = key; *k; ++k) {
        index = *k;
        if (tr->next[index] == 0)
            tr->next[index] = calloc (1, sizeof (struct trie));
        tr = tr->next[index];
        if (*k == '%')
            root->pattern = 1;
    }
    tr->end = 1;
    return 0;
}

static
int trie_find_imp (const void *trie, const char *key, int matching_pattern, int pattern_found)
{
    const struct trie *tr = trie;
    const struct trie *next;


    if (*key == '\0')
       return tr->end;

    next = tr->next[(int) *key];
printf("key=%s, matching_pattern=%d, pattern_found=%d, next[%c]=%p\n", key, matching_pattern, pattern_found, *key, next);
    if (next && trie_find_imp (next, ++key, 0, pattern_found)) {
printf("found by %c\n", *key);
        return 1;
    }
    if (matching_pattern) {
printf("matching %c to %%\n", *key);
        return trie_find_imp (trie, ++key, 1, pattern_found);
}
    if (pattern_found) {
printf("pattern was already used, fail\n");
        return 0;
}
printf("trying %%\n");
    next = tr->next['%'];
printf("    next[%%]=%p\n", next);
    if (next == 0) {
printf("pattern not found, fail\n");
        return 0;
}
printf("found %%\n");
    return trie_find_imp (next, ++key, 1, 1);
}

int trie_find (const void *trie, const char *key)
{
    return trie_find_imp (trie, key, 0, 0);
}

static
char *print (const void *trie, char *buf, ssize_t buflen, off_t off)
{
    const struct trie *tr = trie;
    if (tr->end) {
        buf[off] = '\0';
        printf ("%s\n", buf);
    }
    for (size_t k = 0; k < asciisz; ++k)
        if (tr->next[k]) {
            if (buflen < off) {
                buflen *= 2;
                buf = realloc (buf, buflen);
            }
            buf[off] = (char) k;
            buf = print (tr->next[k], buf, buflen, off+1);
        }
    return buf;
}

int trie_print (const void *trie)
{
    const size_t n = 2;
    char *buf = malloc (n);
    buf = print (trie, buf, n, 0);
    free (buf);
    return 0;
}
