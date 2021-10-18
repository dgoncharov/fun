#include "vector.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct vector {
    char *pos; // Always points to where to push the next key.
    char *end; // The end of the allocated memory.
    int size; // The number of keys.
};

void* vector_init (int capacity)
{
    struct vector *v;
    const size_t a = capacity + sizeof (*v) + 1;
    printf ("reserving %zu bytes\n", a);
    v = (struct vector *) calloc (1, a);
    assert (v);
    v->pos = (char *) v + sizeof (*v);
    v->end = v->pos + capacity + 1;
    printf ("begin = %p, end = %p\n", v->pos, v->end);
    return v;
}

void vector_free (void *vector)
{
    free (vector);
}

int vector_push (void *vector, const char *key)
{
    struct vector *v = (struct vector *) vector;
    // Do not reallocate. Expect the user to pass the full size to init.
    const size_t klen = strlen (key) + 1; // +1 for null terminator.
    assert (v->pos + klen < v->end);
    memcpy (v->pos, key, klen);
    v->pos += klen;
    ++v->size;
//    printf ("pushed %s\n", key);
    return 0;
}

static
int match (const char *pattern, const char *key)
{
    return strcmp (pattern, key) == 0;
}

int vector_has (const void *vector, const char *key)
{
    const struct vector *v = (const struct vector *) vector;
    const char *p = (const char *) v + sizeof (*v);
    size_t plen = strlen (p) + 1; // +1 for '\0'.
    const char *end = v->pos + strlen (v->pos) + 1;

//    printf ("looking for %s\n", key);
    while (p + plen < end) {
        if (match (p, key))
            return 1;
        p += plen;
        plen = strlen (p) + 1;
    }
    return 0;
}

int vector_size (const void *vector)
{
    const struct vector *v = (const struct vector *) vector;
    return v->size;
}
