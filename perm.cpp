#include <iostream>
#include <utility>
#include <stddef.h>
#include <string.h>

void perm(char* s, size_t slen, size_t index)
{
    if (index + 1 < slen)
        perm(s, slen, index + 1);
    for (size_t k = index + 1; k < slen; ++k)
    {
        std::swap(s[index], s[k]);
        std::cout << s << std::endl;
        perm(s, slen, index + 1);
        std::swap(s[index], s[k]);
    }
}

int main(int argc, char* argv[])
{
    std::cout << argv[1] << std::endl;
    perm(argv[1], strlen(argv[1]), 0);
}

