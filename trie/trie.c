#include "trie.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

static int g_loglevel;

static
int print (const char *format, ...)
{
    int retcode;
    va_list ap;
    if (g_loglevel == 0)
        return 0;

    va_start (ap, format);
    retcode = vprintf (format, ap);
    va_end (ap);
    return retcode;
}

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
    print ("begin = %p, end = %p\n", alloc->pos, alloc->end);
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
            print ("*k=%c, n=%lu, n/2=%lu, n%%2=%lu\n", *k, n, n/2, n%2);
            k += n - 1; // Skip the backslashes.
            c = *(k+1); // Next char.
            print ("c=%c\n", c);
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
                    print ("escaped %%\n");
                    index = escaped_percent;
                    root->has_percent = 1;
                } else {
                    print ("naked %%\n");
                    index = '%';
                    stored_naked_percent = 1;
                }

                if (node->next[index] == 0)
                    node->next[index] = alloc_node (alloc);
                node = node->next[index];
                ++k; // Advance k, because we pushed the %.
                print ("next k = %c\n", *k);
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

static
const char **node_find_fuzzy (const char **result, const struct node *node, const char *key, int inside_wildcard, int wildcard_spent, int depth)
{
    const struct node *next;
    int index;
    const char **r;


    print ("%*skey = %s, inside wildcard = %d, wildcard spent = %d\n", depth, "", key, inside_wildcard, wildcard_spent);
    // Ensure recursion is limited to 3.
    // This function cannot afford deep recursion, because deep recursion would
    // prevent long keys.
    assert (depth < 3);
    assert (wildcard_spent || depth == 0);
    for (; *key; ++key) {
        assert (node);
        // First see if inside_wildcard.
        if (inside_wildcard) {
            print ("%*strying %c exactly inside wirdcard\n", depth, "", *key);
            assert (wildcard_spent);
            // Then see if the node has this character.
            index = *key == '%' ? escaped_percent : *key;
            next = node->next[index];
            if (next) {
                print ("%*s%c matches exactly\n", depth, "", *key);
                // No more inside wildcard.
                r = node_find_fuzzy (result, next, key+1, 0, 1, depth+1);
                if (r > result)
                    result = r;
            }
            print ("%*s%c does not match, continue inside wildcard\n", depth, "", *key);
            // Continue inside wildcard.
            // Keep node intact.
            continue;
        }
        assert (inside_wildcard == 0);
        // Then see if wildcard was used already.
        if (wildcard_spent) {
            // Not inside wildcard and wildcard was used already.
            // Every character has to match exactly.
            print ("%*strying %c exactly outside wirdcard\n", depth, "", *key);
            index = *key == '%' ? escaped_percent : *key;
            next = node->next[index];
            if (next == 0) {
                // No match.
                // Need to backtrack and resume from the prior fork and take
                // the other path.
                print ("%*s%c does not match, wildcard used already, no match\n", depth, "", *key);
                return result;
            }
            print ("%*s%c matches outside wildcard, continue matching exactly\n", depth, "", *key);
            node = next;
            continue;
        }
        print ("%*ssee if the node has a %%\n", depth, "");
        // Not inside wildcard and wildcard is still available.
        // Then see if the node has a wildcard.
        next = node->next['%'];
        if (next) {
            print ("%*sfound %%, recursing\n", depth, "");
            r = node_find_fuzzy (result, next, key+1, 1, 1, depth+1);
            if (r > result)
                result = r;
        }

        print ("%*sno %%, matching %c exactly\n", depth, "", *key);
        // Then see if the node has this character.
        index = *key == '%' ? escaped_percent : *key;
        next = node->next[index];
        if (next == 0)
            return result; // No match.
        print ("%*sno %%, %c matches exactly\n", depth, "", *key);
        node = next;
    }
    print ("%*skey exhausted\n", depth, "");

    if (*key == '\0' && node->end) {
        print ("%*sfound %s\n", depth, "", node->key);
        *result++ = node->key;
    }
    return result;
}

static
const struct node *node_find_exact (const struct node *node, const char *key)
{
    const struct node *next;
    int index;


    print ("key = %s\n", key);
    for (; *key; ++key) {
        assert (node);

        print ("no %%, matching %c exactly\n", *key);
        // Then see if the node has this character.
        index = *key == '%' ? escaped_percent : *key;
        next = node->next[index];
        if (next == 0)
            return 0; // No match.
        print ("no %%, %c matches exactly\n", *key);
        node = next;
    }
    print ("key exhausted\n");

    if (*key || node->end)
        return node;
    return 0;
}

static
int nodecmp (const void *x, const void *y)
{
    const char *xkey = *(char**) x, *ykey = *(char**) y;
    size_t xlen, ylen;


    print ("xkey = %s, ykey = %s\n", xkey, ykey);

    xlen = strlen (xkey);
    ylen = strlen (ykey);

    // x and y are reversed, because the longest key is the most specific
    // match.
    if (xlen < ylen)
        return 1;
    if (ylen < xlen)
        return -1;

//TODO: when patterns are of equal length return the one that was pushed first.
    return 1;
}

static
const char **node_find (const char **result, const struct node *node, const char *key, int all)
{
    const struct node *next;
    const char **r;

    r = result;
    *r = '\0';
    // Exact match always beats fuzzy match.
    next = node_find_exact (node, key);
    if (next && all == 0) {
        *r++ = next->key;
        return r;
    }
    
    return node_find_fuzzy (r, node, key, 0, 0, 0);
}

static
int node_has_prefer_exact_match (const struct node *node, const char *key,
                  int inside_wildcard, int wildcard_spent, int depth)
{
    const struct node *next;
    int index;

    if (*key == '\0') {
        print ("%*skey exhausted, node->end = %d\n", depth, "", node->end);
        return node->end;
    }

    index = *key == '%' ? escaped_percent : *key;
    next = node->next[index];
    print ("%*skey=%s, inside_wildcard=%d, wildcard_spent=%d, next[%c]=%p\n", depth, "", key, inside_wildcard, wildcard_spent, *key, next);
    if (next && node_has_prefer_exact_match (next, key+1, 0, wildcard_spent, depth+1)) {
        print ("%*s%c is found\n", depth, "", *key);
        return 1;
    }
    if (inside_wildcard) {
        print ("%*smatched %c to %%\n", depth, "", *key);
        return node_has_prefer_exact_match (node, key+1, 1, wildcard_spent, depth+1);
    }
    if (wildcard_spent) {
        print ("%*s%% was already used, backtrack key\n", depth, "");
        return 0;
    }
    next = node->next['%'];
    print ("%*snext[%%]=%p\n", depth, "", next);
    if (next == 0) {
        print ("%*sbacktrack key\n", depth, "");
        return 0;
    }
    print ("%*smatching %c to %%\n", depth, "", *key);
    return node_has_prefer_exact_match (next, key+1, 1, 1, depth+1);
}

static
int node_has_prefer_fuzzy_match (const struct node *node, const char *key, int inside_wildcard, int wildcard_spent, int depth)
{
    const struct node *next;
    int index;


    print ("%*skey = %s, inside wildcard = %d, wildcard spent = %d\n", depth, "", key, inside_wildcard, wildcard_spent);
    // Ensure recursion is limited to 3.
    // This function cannot afford deep recursion, because deep recursion would
    // prevent long keys.
    assert (depth < 3);
    assert (wildcard_spent || depth == 0);
    for (; *key; ++key) {
        assert (node);
        // First see if inside_wildcard.
        if (inside_wildcard) {
            print ("%*strying %c exactly inside wirdcard\n", depth, "", *key);
            assert (wildcard_spent);
            // Then see if the node has this character.
            index = *key == '%' ? escaped_percent : *key;
            next = node->next[index];
            if (next) {
                print ("%*s%c matches\n", depth, "", *key);
                // No more inside wildcard.
                if (node_has_prefer_fuzzy_match (next, key+1, 0, 1, depth+1))
                    return 1;
            }
            print ("%*s%c does not match, continue inside wildcard\n", depth, "", *key);
            // Continue inside wildcard.
            // Keep node intact.
            continue;
        }
        assert (inside_wildcard == 0);
        // Then see if wildcard was used already.
        if (wildcard_spent) {
            // Not inside wildcard and wildcard was used already.
            // Every character has to match exactly.
            print ("%*strying %c exactly outside wirdcard\n", depth, "", *key);
            index = *key == '%' ? escaped_percent : *key;
            next = node->next[index];
            if (next == 0) {
                // No match.
                // Need to backtrack and resume from the prior fork and take
                // the other path.
                print ("%*s%c does not match, wildcard used already, no match\n", depth, "", *key);
                return 0;
            }
            print ("%*s%c matches outside wildcard, continue matching exactly\n", depth, "", *key);
            node = next;
            continue;
        }
        print ("%*ssee if the node has a %%\n", depth, "");
        // Not inside wildcard and wildcard is still available.
        // Then see if the node has a wildcard.
        next = node->next['%'];
        if (next) {
            print ("%*sfound %%, recursing\n", depth, "");
            if (node_has_prefer_fuzzy_match (next, key+1, 1, 1, depth+1))
                return 1;
        }
        print ("%*sno %%, matching %c exactly\n", depth, "", *key);
        // Then see if the node has this character.
        index = *key == '%' ? escaped_percent : *key;
        next = node->next[index];
        if (next == 0)
            return 0; // No match.
        print ("%*sno %%, %c matches exactly\n", depth, "", *key);
        node = next;
    }
    print ("%*skey exhausted\n", depth, "");

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
    print ("looking for %s\n", key);
    if (prefer_fuzzy_match)
        rc = node_has_prefer_fuzzy_match (node, key, 0, 0, 0);
    else
        rc = node_has_prefer_exact_match (node, key, 0, 0, 0);
    print ("%sfound %s\n", rc ? "" : "not ", key);
    return rc;
}



static
char *node_print_imp (const struct node *node, char *buf, ssize_t buflen, off_t off)
{
    if (node->end) {
        buf[off] = '\0';
        printf ("%s\n", buf);
    }
    for (size_t k = 0; k < asciisz; ++k)
        if (node->next[k]) {
            // Add extra 8 chars, because off+1 is passed to node_print_imp at
            // the end of this if check and if node->end is set, then buf[off]
            // is assigned '\0'.
            if (buflen <= off + 8) {
                while (buflen <= off + 8)
                    buflen *= 2;
                assert(buflen > off + 8);
                buf = realloc (buf, buflen);
            }
            assert(buflen > off + 8);
            buf[off] = (char) k;
            buf = node_print_imp (node->next[k], buf, buflen, off+1);
        }
    return buf;
}

static
int node_print (const struct node *node)
{
//TODO: use a bigger initial value. e.g. 128.
    const size_t n = 2;
    char *buf = malloc (n);
    buf = node_print_imp (node, buf, n, 0);
    free (buf);
    return 0;
}

struct trie {
    struct node *root;
    const char **result; // trie_find stores the found matching keys here.
    int size; // The number of keys in this trie.
    struct node_allocator alloc;
};

// LIMIT is the max number of characters (equals the number of nodes) that
// can be pushed to this trie.
void *trie_init (int limit, int loglevel)
{
    struct trie *trie;
    const size_t nbytes = sizeof (struct node) * limit + sizeof (struct trie);
    const int nnodes = limit - 1; // Minus the root node.

    g_loglevel = loglevel;

    assert (limit > 0);
    print ("allocating %zu bytes for %d nodes\n", nbytes, nnodes);
    trie = calloc (1, nbytes);
    print ("trie = %p\n", trie);
    assert (trie);
    trie->root = (struct node *) (trie + 1);
    print ("trie->root = %p, sizeof (struct node) = %zu, sizeof (*trie->root) = %zu, first node = %p\n", trie->root, sizeof (struct node), sizeof (*trie->root), trie->root + 1);
    // The first node is the root node.
    // The first available node is right next to root.
    alloc_init (&trie->alloc, trie->root + 1, nnodes);
//TODO: take the number of future keys and init trie->result.
    trie->result = malloc (1024 * sizeof (char*));
    return trie;
}

int trie_free (void *trie)
{
    const struct trie *tr = trie;
    node_free (tr->root);
    free (tr->result);
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
        print ("pushed %s, size = %d\n", key, tr->size);
    }
    return rc;
}

static
int integrity (const char **result, const char *key)
{
    const size_t klen = strlen (key);
    int nexact = 0; /* The number of results that match the key exactly. */

    for (; *result; ++result) {
        const size_t rlen = strlen (*result);
        assert (klen >= rlen || strchr (*result, '\\'));
        assert (nexact == 0 || strchr (*result, '%'));
        nexact += strcmp (key, *result) == 0;
    }
    assert (nexact == 0 || nexact == 1);
    return 1;
}

const char **trie_find_all (void *trie, const char *key)
{
    struct trie *tr = trie;
    const char **r;
    const char *del = "";

    r = node_find (tr->result, tr->root, key, 1);
    qsort (tr->result, r - tr->result, sizeof (char *), nodecmp);
    *r = 0; // Null terminator.
    print ("sorted results ");
    for (r = tr->result; *r; ++r, del = ", ")
        print ("%s%s", del, *r);
    print ("\n");
    assert (integrity (tr->result, key));
    return tr->result;
}

const char *trie_find (void *trie, const char *key)
{
    return *trie_find_all (trie, key);
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
