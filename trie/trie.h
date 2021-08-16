#ifndef _TRIE_H_
#define _TRIE_H_

struct trie {
    struct trie *next[128];
    unsigned leaf:1; // Leaf node.
};

int trie_init (struct trie *trie);
int trie_push (struct trie *trie, const char *key);
int trie_find (const struct trie *trie, const char *key);

#endif
