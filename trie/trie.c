#include "trie.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum {asciisz = 128};
struct trie {
    struct trie *next[asciisz];
    char *key; // The key which ends here.
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
    free (tr->key);
    free (trie);
    return 0;
}

int trie_push (void *trie, const char *key)
{
    struct trie *root = trie;
    struct trie *tr = trie;
    int index;
    size_t klen;
    for (const char *k = key; *k; ++k) {
        index = *k;
        if (tr->next[index] == 0)
            tr->next[index] = calloc (1, sizeof (struct trie));
        tr = tr->next[index];
        if (*k == '%')
            root->pattern = 1;
    }
    tr->end = 1;
    klen = strlen (key) + 1; // + 1 for null terminator.
    tr->key = malloc (klen);
    memcpy (tr->key, key, klen);
    return 0;
}

static
const struct trie *trie_find_imp (const void *trie, const char *key, int matching_pattern, int pattern_used, int depth)
{
    const struct trie *tr = trie;
    const struct trie *next;


    if (*key == '\0') {
        if (tr->key) {
            for (int k = depth; k; --k)
                printf(" ");
            printf("found pattern %s\n", tr->key);
            return tr;
        }
        for (int k = depth; k; --k)
            printf(" ");
        printf("exhaused key, fail\n");
        return 0;
    }

    next = tr->next[(int) *key];
    for (int k = depth; k; --k)
        printf(" ");
    printf("key=%s, matching_pattern=%d, pattern_used=%d, next[%c]=%p\n", key, matching_pattern, pattern_used, *key, next);
    if (next && (next = trie_find_imp (next, key+1, 0, pattern_used, depth+1))) {
        const struct trie *alt;
        for (int k = depth; k; --k)
            printf(" ");
        printf("found by %c\n", *key);
        if (pattern_used)
            return next;
        for (int k = depth; k; --k)
            printf(" ");
        printf ("trying alternative %%, key = %s\n", key);
        alt = tr->next['%'];
        if (alt == 0)
            return next;
        alt = trie_find_imp (alt, key+1, 1, 1, depth+1);
        if (alt == 0)
            return next;
        for (int k = depth; k; --k)
            printf(" ");
        printf ("found alternative %%, key = %s, alt->key=%s, next->key=%s\n", key, alt->key, next->key);
        return strlen (alt->key) > strlen (next->key) ? alt : next;
    }
    if (matching_pattern) {
        for (int k = depth; k; --k)
            printf(" ");
        printf("matching %c to %%\n", *key);
        return trie_find_imp (trie, key+1, 1, pattern_used, depth+1);
    }
    if (pattern_used) {
        for (int k = depth; k; --k)
            printf(" ");
        printf("pattern was already used, fail\n");
        return 0;
    }
    for (int k = depth; k; --k)
        printf(" ");
    printf("trying %%\n");
    next = tr->next['%'];
    for (int k = depth; k; --k)
        printf(" ");
    printf("next[%%]=%p\n", next);
    if (next == 0) {
        for (int k = depth; k; --k)
            printf(" ");
        printf("pattern not found, fail\n");
        return 0;
    }
    for (int k = depth; k; --k)
        printf(" ");
    printf("found %%\n");
    return trie_find_imp (next, key+1, 1, 1, depth+1);
}

const char *trie_find (const void *trie, const char *key)
{
    const struct trie *tr = trie_find_imp (trie, key, 0, 0, 0);
    if (tr == 0)
        return 0;
    return tr->key;
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
