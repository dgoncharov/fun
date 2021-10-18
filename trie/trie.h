#ifndef _TRIE_H_
#define _TRIE_H_

void *trie_init ();
int trie_free (void *trie);
int trie_push (void *trie, const char *key);
const char *trie_find (const void *trie, const char *key);
int trie_has (const void *trie, const char *key, int prefer_fuzzy_match);
int trie_size (const void *trie);
int trie_print (const void *trie);

#endif
