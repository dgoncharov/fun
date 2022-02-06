#include "trie.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

// Unix filenames cannot contain '\0' (slot 0) and '/' (slot 47).
// Keys which trie accepts can contain a slash though. e.g. "obj/hello.o".
// This leaves slot 0 for trie's internal purposes.
// Escaped percent is stored in slot 0.
enum {asciisz = 128, escaped_percent = 0};
struct node {
    struct node *next[asciisz];
    char *key; // The key which ends here. This is the key that was passed to
               // node_push intact with all escaping backslashes preserved.
    unsigned end:1; // When set this node is the end of a word.
    unsigned has_percent:1; // When set the node contains a naked %.
};

struct node_allocator {
    struct node *pos;
    struct node *end;
};

static
int alloc_init (struct node_allocator *alloc, struct node *begin, size_t nnodes)
{
    alloc->pos = begin;
    alloc->end = begin + nnodes;
    //printf("begin = %p, end = %p\n", alloc->pos, alloc->end);
    return 0;
}

static
struct node* alloc_node (struct node_allocator *alloc)
{
    assert (alloc->pos < alloc->end);
    return alloc->pos++;
}

static
int node_free (struct node *node)
{
    for (size_t k = 0; k < asciisz; ++k)
        if (node->next[k])
            node_free (node->next[k]);
    free (node->key);
    return 0;
}

static
int node_push (struct node *root, const char *key, struct node_allocator *alloc)
{
    struct node *node = root;
    int index;
    size_t klen;
    int stored_naked_percent = 0;

    for (const char *k = key; *k; ++k) {
        if (*k == '\\') {
            // This whole if-block is dedicated to reading this backslash.
            char c;
            int index = '\\';
            size_t n = strspn (k, "\\");
            //printf("*k=%c, n=%lu, n/2=%lu, n%%2=%lu\n", *k, n, n/2, n%2);
            k += n - 1; // Skip the backslashes.
            c = *(k+1); // Next char.
            //printf("c=%c\n", c);
            // gmake allows multiple % in a rule, as long as first is naked and
            // the others are escaped.
            if (c == '%') {
                // This whole if-block is dedicated to reading this percent.
                if (stored_naked_percent && n % 2 == 0) {
                    // Malformed key with multiple naked %.
                    // The caller should print an error message and terminate.
                    return -1;
                }
                // The backslashes are immediately followed by a '%'.
                // Each odd backslash escapes immediately following even
                // backslash.
                //
                // If number of backslashes is odd, then the last backslash
                // escapes the %. Push half of the backslashes and an escaped
                // %.
                //
                // If the number of backslashes is even, then the % is not
                // escaped.  Push half of the backslashes and the % (not
                // escaped).

                // Push half of the backslashes.
                for (size_t j = n/2; j > 0; --j) {
                    if (node->next[index] == 0)
                        node->next[index] = alloc_node (alloc);
                    node = node->next[index];
                }

                // Push escaped or naked %.
                if (n % 2) {
                    //printf("escaped %%\n");
                    index = escaped_percent;
                    root->has_percent = 1;
                } else {
                    //printf("naked %%\n");
                    index = '%';
                    stored_naked_percent = 1;
                }

                if (node->next[index] == 0)
                    node->next[index] = alloc_node (alloc);
                node = node->next[index];
                ++k; // Advance k, because we pushed the %.
                //printf("next k = %c\n", *k);
                continue;
            }
            // The backslashes are not immediately followed by a '%'.
            // None of these backslashes escapes another backslash or %. Push
            // them all.
            for (; n > 0; --n) {
                if (node->next[index] == 0)
                    node->next[index] = alloc_node (alloc);
                node = node->next[index];
            }
            continue;
        }
        // Not a backslash.
        if (*k == '%') {
            if (stored_naked_percent == 1)
                // Malformed key with multiple naked %.
                // The caller should print an error message and terminate.
                return -1;
            stored_naked_percent = 1;
            root->has_percent = 1;
        }
        index = *k;
        if (node->next[index] == 0)
            node->next[index] = alloc_node (alloc);
        node = node->next[index];
    }
    node->end = 1;
    assert (node->key == 0 || strcmp (node->key, key) == 0);

    // Pushing the same key multiple times is allowed, even though
    // pointless.
    // If node->key is set, that means this key was already pushed earlier.
    if (node->key)
        // Duplicate key.
        return 1;

    // This key is not present in trie yet.
    klen = strlen (key) + 1; // + 1 for null terminator.
    node->key = malloc (klen);
    memcpy (node->key, key, klen);
    return 0;
}

//TODO: do not recurse for every char. Recurse at most once per attemped full
// match.
//TODO: look at wildcard_spent and avoid checking %, if found a perfect match.
static
const struct node *node_find_imp (const struct node *node, const char *key, int inside_wildcard, int wildcard_spent, int depth)
{
    const struct node *next;
    int index;


    if (*key == '\0') {
        if (node->key) {
            //printf("%*sfound %s\n", depth, "", node->key);
            return node;
        }
        //printf("%*sexhaused key, fail\n", depth, "");
        return 0;
    }

    index = *key == '%' ? escaped_percent : *key;
    next = node->next[index];
    //printf("%*skey=%s, inside_wildcard=%d, wildcard_spent=%d, next[%c]=%p\n", depth, "", key, inside_wildcard, wildcard_spent, *key, next);
    if (next && (next = node_find_imp (next, key+1, 0, wildcard_spent, depth+1))) {
        const struct node *alt;
        size_t klen, alen;
        //printf("%*sfound by %c\n", depth, "", *key);
        if (wildcard_spent)
            return next;
        //printf ("%*strying alternative %%, key = %s\n", depth, "", key);
        alt = node->next['%'];
        if (alt == 0)
            return next;
        alt = node_find_imp (alt, key+1, 1, 1, depth+1);
        if (alt == 0)
            return next;
        //printf ("%*sfound alternative %%, key = %s, alt->key=%s, next->key=%s\n", depth, "", key, alt->key, next->key);
        /* alt->key contains a %. next->key may or may not contain a %.
         * If alt-key is longer than next->key, next->key also contains a %.
         * Otherwise, next-key would not match.
         * Therefore, prefer alt->key only if alt->key is longer.
         * If both contain % and are equal length, prefer the one
         * pushed earlier, because that one was specified earlier in the
         * makefile.  */
        alen = strlen (alt->key);
        klen = strlen (next->key);
        assert (strchr (alt->key, '%'));
        assert (alen <= klen || strchr (next->key, '%'));
        return alen > klen ? alt : next;
    }
    if (inside_wildcard) {
        //printf("%*smatching %c to %%\n", depth, "", *key);
        return node_find_imp (node, key+1, 1, wildcard_spent, depth+1);
    }
    if (wildcard_spent) {
        //printf("%*swildcard was already used, fail\n", depth, "");
        return 0;
    }
    //printf("%*strying %%\n", depth, "");
    next = node->next['%'];
    //printf("%*snext[%%]=%p\n", depth, "", next);
    if (next == 0) {
        //printf("%*swildcard not found, fail\n", depth, "");
        return 0;
    }
    //printf("%*sfound %%\n", depth, "");
    return node_find_imp (next, key+1, 1, 1, depth+1);
}

static
const char *node_find (const struct node *node, const char *key)
{
    node = node_find_imp (node, key, 0, 0, 0);
    if (node == 0)
        return 0;
    return node->key;
}

static
int node_has_prefer_exact_match (const struct node *node, const char *key,
                  int inside_wildcard, int wildcard_spent, int depth)
{
    const struct node *next;
    int index;

    if (*key == '\0') {
        //printf("%*skey exhausted, node->end = %d\n", depth, "", node->end);
        return node->end;
    }

    index = *key == '%' ? escaped_percent : *key;
    next = node->next[index];
    //printf("%*skey=%s, inside_wildcard=%d, wildcard_spent=%d, next[%c]=%p\n", depth, "", key, inside_wildcard, wildcard_spent, *key, next);
    if (next && node_has_prefer_exact_match (next, key+1, 0, wildcard_spent, depth+1)) {
        //printf("%*s%c is found\n", depth, "", *key);
        return 1;
    }
    if (inside_wildcard) {
        //printf("%*smatched %c to %%\n", depth, "", *key);
        return node_has_prefer_exact_match (node, key+1, 1, wildcard_spent, depth+1);
    }
    if (wildcard_spent) {
        //printf("%*s%% was already used, backtrack key\n", depth, "");
        return 0;
    }
    next = node->next['%'];
    //printf("%*snext[%%]=%p\n", depth, "", next);
    if (next == 0) {
        //printf("%*sbacktrack key\n", depth, "");
        return 0;
    }
    //printf("%*smatching %c to %%\n", depth, "", *key);
    return node_has_prefer_exact_match (next, key+1, 1, 1, depth+1);
}

static
int node_has_prefer_fuzzy_match (const struct node *node, const char *key, int inside_wildcard, int wildcard_spent, int depth)
{
    const struct node *next;
    int index;


    //printf("%*skey = %s, inside wildcard = %d, wildcard spent = %d\n", depth, "", key, inside_wildcard, wildcard_spent);
    // Ensure recursion is limited to 3.
    // This function cannot afford deep recursion, because deep recursion would
    // prevent long keys.
    assert (depth < 3);
    for (; *key; ++key) {
        assert (node);
        // First see if inside_wildcard.
        if (inside_wildcard) {
            //printf("%*strying %c explicitly inside wirdcard\n", depth, "", *k);
            assert (wildcard_spent);
            // Then see if the node has this character.
            index = *key == '%' ? escaped_percent : *key;
            next = node->next[index];
            if (next) {
                //printf("%*s%c matches\n", depth, "", *key);
                // No more inside wildcard.
                if (node_has_prefer_fuzzy_match (next, key+1, 0, 1, depth+1))
                    return 1;
            }
            //printf("%*s%c does not match, continue inside wildcard\n", depth, "", *key);
            // Continue inside wildcard.
            // Keep node intact.
            continue;
        }
        assert (inside_wildcard == 0);
        // Then see if wildcard was used already.
        if (wildcard_spent) {
            // Not inside wildcard and wildcard was used already.
            // Every character has to match explicitly.
            //printf("%*strying %c explicitly outside wirdcard\n", depth, "", *key);
            index = *key == '%' ? escaped_percent : *key;
            next = node->next[index];
            if (next == 0) {
                // No match.
                // Need to backtrack and resume from the prior fork and take
                // the other path.
                //printf("%*s%c does not match, wildcard used already, no match\n", depth, "", *key);
                return 0;
            }
            //printf("%*s%c matches outside wildcard, continue matching explicitly\n", depth, "", *key);
            node = next;
            continue;
        }
        //printf("%*ssee if the node has a %%\n", depth, "");
        // Not inside wildcard and wildcard is still available.
        // Then see if the node has a wildcard.
        next = node->next['%'];
        if (next) {
            //printf("%*sfound %%, recursing\n", depth, "");
            if (node_has_prefer_fuzzy_match (next, key+1, 1, 1, depth+1))
                return 1;
        }
        //printf("%*sno %%, matching %c explicitly\n", depth, "", *key);
        // Then see if the node has this character.
        index = *key == '%' ? escaped_percent : *key;
        next = node->next[index];
        if (next == 0)
            return 0; // No match.
        //printf("%*sno %%, %c matches explicitly\n", depth, "", *key);
        node = next;
    }
    //printf("%*skey exhausted\n", depth, "");

    if (*key == '\0')
        return node->end;
    return 0;
}

// Return 1 if KEY is present in node.
// Return 0 otherwise.
// node_has_prefer_fuzzy_match nodes matching each
// character of the key to wildcard % first. If this fuzzy match fails, then
// node_has_prefer_fuzzy_match nodes matching the character exactly.
// node_has_prefer_exact_match does the opposite. node_has_prefer_exact_match
// nodes matching each character of the key exactly first. If this exact match
// fails, then node_has_prefer_exact_match nodes matching the character fuzzy.
// Both functions return the same result. The difference is in how fast the
// result is found. node_has_prefer_exact_match performs better on some
// contents of node, node_has_prefer_fuzzy_match performs better on other
// contents of node.
static
int node_has (const struct node *node, const char *key, int prefer_fuzzy_match)
{
    int rc;
    //printf ("looking for %s\n", key);
    if (prefer_fuzzy_match)
        rc = node_has_prefer_fuzzy_match (node, key, 0, 0, 0);
    else
        rc = node_has_prefer_exact_match (node, key, 0, 0, 0);
    //printf ("%sfound %s\n", rc ? "" : "not ", key);
    return rc;
}



static
char *print (const struct node *node, char *buf, ssize_t buflen, off_t off)
{
    if (node->end) {
        buf[off] = '\0';
        printf ("%s\n", buf);
    }
    for (size_t k = 0; k < asciisz; ++k)
        if (node->next[k]) {
            // Add extra 8 chars, because off+1 is passed to print at the end
            // of this if check and if node->end is set, then buf[off] is
            // assigned '\0'.
            if (buflen <= off + 8) {
                while (buflen <= off + 8)
                    buflen *= 2;
                assert(buflen > off + 8);
                buf = realloc (buf, buflen);
            }
            assert(buflen > off + 8);
            buf[off] = (char) k;
            buf = print (node->next[k], buf, buflen, off+1);
        }
    return buf;
}

static
int node_print (const struct node *node)
{
//TODO: use a bigger initial value. e.g. 128.
    const size_t n = 2;
    char *buf = malloc (n);
    buf = print (node, buf, n, 0);
    free (buf);
    return 0;
}

struct trie {
    struct node *root;
    int size; // Number of keys in this trie.
    struct node_allocator alloc;
};

// LIMIT is the max number of characters (equals the number of nodes) that
// can be pushed to this trie.
void *trie_init (int limit)
{
    struct trie *trie;
    const size_t nbytes = sizeof (struct node) * limit + sizeof (struct trie);
    const int nnodes = limit - 1; // Minus the root node.

    assert (limit > 0);
    printf("allocating %zu bytes for %d nodes\n", nbytes, nnodes);
    trie = calloc (1, nbytes);
    printf("trie = %p\n", trie);
    assert (trie);
    trie->root = (struct node *) (trie + 1);
    printf("trie->root = %p, sizeof (struct node) = %zu, sizeof (*trie->root) = %zu, first node = %p\n", trie->root, sizeof (struct node), sizeof (*trie->root), trie->root + 1);
    // The first node is the root node.
    // The first available node is right next to root.
    alloc_init (&trie->alloc, trie->root + 1, nnodes);
    return trie;
}

int trie_free (void *trie)
{
    const struct trie *tr = trie;
    node_free (tr->root);
    free (trie);
    return 0;
}

int trie_push (void *trie, const char *key)
{
    int rc;
    struct trie *tr = trie;
    rc = node_push (tr->root, key, &tr->alloc);
    if (rc == 0) {
        ++tr->size;
        //printf("pushed %s, size = %d\n", key, tr->size);
    }
    return rc;
}

const char *trie_find (const void *trie, const char *key)
{
    const struct trie *tr = trie;
    return node_find (tr->root, key);
}

int trie_has (const void *trie, const char *key, int prefer_fuzzy_match)
{
    const struct trie *tr = trie;
    return node_has (tr->root, key, prefer_fuzzy_match);
}

int trie_size (const void *trie)
{
    const struct trie *tr = trie;
    return tr->size;
}

int trie_print (const void *trie)
{
    const struct trie *tr = trie;
    return node_print (tr->root);
}


