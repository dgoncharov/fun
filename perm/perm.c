/*
 * Being called with a single command line argument this program computes
 * and prints to stdout all possible permutations of the argument.
 *
 * Copyright (c) 2011 Dmitry Goncharov.
 * Distributed under the terms of the bsd license.
 *
 * usage: perm <string>
 */

#include <stdio.h>

void swap(char* s1, char* s2)
{
    *s1 ^= *s2;
    *s2 ^= *s1;
    *s1 ^= *s2;
}

int main(int argc, char* argv[])
{
    if (*argv[0])
    {
        printf("%s\n", argv[1]);
        *argv[0] = '\0';
    }
    if (*(argv[1] + argc - 1))
        main(argc + 1, argv);
    char* s = argv[1] + argc - 1;
    for (; *s; ++s)
    {
        swap(argv[1] + argc - 2, s);
        printf("%s\n", argv[1]);
        main(argc + 1, argv);
        swap(argv[1] + argc - 2, s);
    }
    return 0;
}

