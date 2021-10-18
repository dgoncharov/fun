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
struct trie {
    struct trie *next[asciisz];
    int size; // Number of keys in this trie.
    char *key; // The key which ends here. This is the key that was passed to
               // trie_push intact with all escaping backslashes preserved.
    unsigned end:1; // When set this node is the end of a word.
    unsigned has_percent:1; // When set the trie contains a naked %.
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
    int stored_naked_percent = 0;

    for (const char *k = key; *k; ++k) {
        if (*k == '\\') {
            // This whole if-block is dedicated to reading this backslash.
            char c;
            int index = '\\';
            size_t n = strspn (k, "\\");
            printf("*k=%c, n=%lu, n/2=%lu, n%%2=%lu\n", *k, n, n/2, n%2);
            k += n - 1; // Skip the backslashes.
            c = *(k+1); // Next char.
            printf("c=%c\n", c);
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
                    if (tr->next[index] == 0)
                        tr->next[index] = calloc (1, sizeof (struct trie));
                    tr = tr->next[index];
                }

                // Push escaped or naked %.
                if (n % 2) {
                    printf("escaped %%\n");
                    index = escaped_percent;
                    root->has_percent = 1;
                } else {
                    printf("naked %%\n");
                    index = '%';
                    stored_naked_percent = 1;
                }

                if (tr->next[index] == 0)
                    tr->next[index] = calloc (1, sizeof (struct trie));
                tr = tr->next[index];
                ++k; // Advance k, because we pushed the %.
                printf("next k = %c\n", *k);
                continue;
            }
            // The backslashes are not immediately followed by a '%'.
            // None of these backslashes escapes another backslash or %. Push
            // them all.
            for (; n > 0; --n) {
                if (tr->next[index] == 0)
                    tr->next[index] = calloc (1, sizeof (struct trie));
                tr = tr->next[index];
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
        if (tr->next[index] == 0)
            tr->next[index] = calloc (1, sizeof (struct trie));
        tr = tr->next[index];
    }
    tr->end = 1;
    assert (tr->key == 0 || strcmp (tr->key, key) == 0);
    if (tr->key == 0) {
        // Pushing the same key multiple times is allowed, even though
        // pointless.
        // If tr->key is set, that means this key was already pushed earlier.
        klen = strlen (key) + 1; // + 1 for null terminator.
        tr->key = malloc (klen);
        memcpy (tr->key, key, klen);
        ++root->size;
    }
    printf("pushed %s, size = %d\n", key, root->size);
    return 0;
}

//TODO: look at wildcard_used and avoid checking %, if found a perfect match.
static
const struct trie *trie_find_imp (const struct trie *trie, const char *key, int matching_wildcard, int wildcard_used, int depth)
{
    const struct trie *tr = trie;
    const struct trie *next;
    int index;


    if (*key == '\0') {
        if (tr->key) {
            printf("%*sfound %s\n", depth, "", tr->key);
            return tr;
        }
        printf("%*sexhaused key, fail\n", depth, "");
        return 0;
    }

    index = *key == '%' ? escaped_percent : *key;
    next = tr->next[index];
    printf("%*skey=%s, matching_wildcard=%d, wildcard_used=%d, next[%c]=%p\n", depth, "", key, matching_wildcard, wildcard_used, *key, next);
    if (next && (next = trie_find_imp (next, key+1, 0, wildcard_used, depth+1))) {
        const struct trie *alt;
        size_t klen, alen;
        printf("%*sfound by %c\n", depth, "", *key);
        if (wildcard_used)
            return next;
        printf ("%*strying alternative %%, key = %s\n", depth, "", key);
        alt = tr->next['%'];
        if (alt == 0)
            return next;
        alt = trie_find_imp (alt, key+1, 1, 1, depth+1);
        if (alt == 0)
            return next;
        printf ("%*sfound alternative %%, key = %s, alt->key=%s, next->key=%s\n", depth, "", key, alt->key, next->key);
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
    if (matching_wildcard) {
        printf("%*smatching %c to %%\n", depth, "", *key);
        return trie_find_imp (trie, key+1, 1, wildcard_used, depth+1);
    }
    if (wildcard_used) {
        printf("%*swildcard was already used, fail\n", depth, "");
        return 0;
    }
    printf("%*strying %%\n", depth, "");
    next = tr->next['%'];
    printf("%*snext[%%]=%p\n", depth, "", next);
    if (next == 0) {
        printf("%*swildcard not found, fail\n", depth, "");
        return 0;
    }
    printf("%*sfound %%\n", depth, "");
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
int trie_has_prefer_exact_match (const struct trie *trie, const char *key,
                  int matching_wildcard, int wildcard_used, int depth)
{
    const struct trie *tr = trie;
    const struct trie *next;
    int index;

    if (*key == '\0') {
        printf("%*skey exhausted, tr->end = %d\n", depth, "", tr->end);
        return tr->end;
    }

    index = *key == '%' ? escaped_percent : *key;
    next = tr->next[index];
    printf("%*skey=%s, matching_wildcard=%d, wildcard_used=%d, next[%c]=%p\n", depth, "", key, matching_wildcard, wildcard_used, *key, next);
    if (next && trie_has_prefer_exact_match (next, key+1, 0, wildcard_used, depth+1)) {
        printf("%*s%c is found\n", depth, "", *key);
        return 1;
    }
    if (matching_wildcard) {
        printf("%*smatched %c to %%\n", depth, "", *key);
        return trie_has_prefer_exact_match (trie, key+1, 1, wildcard_used, depth+1);
    }
    if (wildcard_used) {
        printf("%*s%% was already used, backtrack key\n", depth, "");
        return 0;
    }
    next = tr->next['%'];
    printf("%*snext[%%]=%p\n", depth, "", next);
    if (next == 0) {
        printf("%*sbacktrack key\n", depth, "");
        return 0;
    }
    printf("%*smatching %c to %%\n", depth, "", *key);
    return trie_has_prefer_exact_match (next, key+1, 1, 1, depth+1);
}

// trie_has_prefer_exact_match2 prefers fuzzy to exact match.
static
int trie_has_prefer_fuzzy_match (const struct trie *trie, const char *key,
                  int matching_wildcard, int wildcard_used, int depth)
{
    const struct trie *tr = trie;
    const struct trie *next;
    int index;

    if (*key == '\0') {
        printf("%*skey exhausted, tr->end = %d\n", depth, "", tr->end);
        return tr->end;
    }

    if (matching_wildcard) {
        printf("%*smatched %c to %%\n", depth, "", *key);
        if (trie_has_prefer_fuzzy_match (trie, key+1, 1, wildcard_used, depth+1))
            return 1;
    } else {
        next = tr->next['%'];
        printf("%*snext[%%]=%p\n", depth, "", next);
        if (next && trie_has_prefer_fuzzy_match (next, key+1, 1, 1, depth+1))
            return 1;
    }
    index = *key == '%' ? escaped_percent : *key;
    next = tr->next[index];
    printf("%*skey=%s, next[%c]=%p\n", depth, "", key, *key, next);
    if (next && trie_has_prefer_fuzzy_match (next, key+1, 0, 0, depth+1)) {
        printf("%*s%c is found\n", depth, "", *key);
        return 1;
    }
    return 0;
}

// Return 1 if KEY is present in TRIE.
// Return 0 otherwise.
// trie_has_prefer_fuzzy_match tries matching each
// character of the key to wildcard % first. If this fuzzy match fails, then
// trie_has_prefer_fuzzy_match tries matching the character exactly.
// trie_has_prefer_exact_match does the opposite. trie_has_prefer_exact_match
// tries matching each character of the key exactly first. If this exact match
// fails, then trie_has_prefer_exact_match tries matching the character fuzzy.
// Both functions return the same result. The difference is in how fast the
// result is found. trie_has_prefer_exact_match performs better on some
// contents of trie, trie_has_prefer_fuzzy_match performs better on other
// contents of trie.
int trie_has (const void *trie, const char *key, int prefer_fuzzy_match)
{
    int rc;
    printf ("looking for %s\n", key);
    if (prefer_fuzzy_match)
        rc = trie_has_prefer_fuzzy_match (trie, key, 0, 0, 0);
    else
        rc = trie_has_prefer_exact_match (trie, key, 0, 0, 0);
    printf ("%sfound %s\n", rc ? "" : "not ", key);
    return rc;
}

int trie_size (const void *trie)
{
    const struct trie *tr = trie;
    return tr->size;
}

static
char *print (const struct trie *trie, char *buf, ssize_t buflen, off_t off)
{
    const struct trie *tr = trie;
    if (tr->end) {
        buf[off] = '\0';
        printf ("%s\n", buf);
    }
    for (size_t k = 0; k < asciisz; ++k)
        if (tr->next[k]) {
            // Add extra 8 chars, because off+1 is passed to print at the end
            // of this if check and if tr->end is set, then buf[off] is
            // assigned '\0'.
            if (buflen <= off + 8) {
                while (buflen <= off + 8)
                    buflen *= 2;
                assert(buflen > off + 8);
                buf = realloc (buf, buflen);
            }
            assert(buflen > off + 8);
            buf[off] = (char) k;
            buf = print (tr->next[k], buf, buflen, off+1);
        }
    return buf;
}

int trie_print (const void *trie)
{
//TODO: use a bigger initial value. e.g. 128.
    const size_t n = 2;
    char *buf = malloc (n);
    buf = print (trie, buf, n, 0);
    free (buf);
    return 0;
}
