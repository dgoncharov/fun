#include <string.h>
#include <stdio.h>

void swap(char* s1, char* s2)
{
    *s1 ^= *s2;
    *s2 ^= *s1;
    *s1 ^= *s2;
}

int main(int argc, char* argv[])
{
    if (*argv[0]) {
        printf("%s\n", argv[1]);
        argv[0][0] = '\0';
    }
    const size_t index = argc - 2;
    char* s = argv[1];
    const size_t slen = strlen(s);
    if (index + 1 < slen)
        main(index + 3, argv);
    for (size_t k = index + 1; k < slen; ++k)
    {
        swap(s + index, s + k);
        printf("%s\n", s);
        main(index + 3, argv);
        swap(s + index, s + k);
    }
}

