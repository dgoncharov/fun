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

//FIXME: trie_find_pattern fails to find the shortest pattern.
static
int trie_find_pattern (const void *trie, const char *key)
{
    const struct trie *tr = trie;
    int pattern = 0;
    for (; *key; ++key) {
        const int index = *key;
        if (pattern == 0 && tr->next['%']) {
            tr = tr->next['%'];
            pattern = 1;
        }
        if (tr->next[index] == 0) {
            if (pattern)
                continue;
            return 0;
        }
        tr = tr->next[index];
    }
    return tr->end;
}

int trie_find (const void *trie, const char *key)
{
    const struct trie *root = trie;
    const struct trie *tr = trie;
    for (const char *k = key; *k; ++k) {
        const int index = *k;
        if (tr->next[index] == 0) {
            if (root->pattern)
                return trie_find_pattern (trie, key);
            return 0;
        }
        tr = tr->next[index];
    }
    if (tr->end)
        return 1;
    if (root->pattern)
        return trie_find_pattern (trie, key);
    return 0;
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
