#ifndef _TRIE_H_
#define _TRIE_H_

void *trie_init ();
int trie_free (void *trie);
int trie_push (void *trie, const char *key);
int trie_find (const void *trie, const char *key);
int trie_print (const void *trie);

#endif
