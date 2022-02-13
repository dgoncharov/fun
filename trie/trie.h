#ifndef _TRIE_H_
#define _TRIE_H_

struct result {
    const char *key; /* This is the key that was passed to node_push intact
                      * with all escaping backslashes preserved. */
    void *userdata; /* The userdata that was passed to trie_push. */
    int order; /* Used for sorting. */
};

void *trie_init (int maxkeys, int maxchars, int loglevel);
int trie_free (void *trie);
int trie_push (void *trie, const char *key, void *userdata);
const struct result *trie_find (void *trie, const char *key);
const struct result **trie_find_all (void *trie, const char *key);
int trie_has (const void *trie, const char *key, int prefer_fuzzy_match);
int trie_size (const void *trie);
int trie_print (const void *trie);

#endif
