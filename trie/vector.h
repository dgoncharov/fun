#ifndef _VECTOR_H_
#define _VECTOR_H_

void *vector_init (int size);
void vector_free (void *vector);
int vector_push (void *vector, const char *key);
int vector_has (const void *vector, const char *key);
int vector_size (const void *vector);

#endif
