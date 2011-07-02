#include <iostream>
#include <utility>
#include <stddef.h>
#include <string.h>

void perm(size_t index, char* s)
{
    const size_t slen = strlen(s);
    if (index + 1 < slen)
        perm(index + 1, s);
    for (size_t k = index + 1; k < slen; ++k)
    {
        std::swap(s[index], s[k]);
        std::cout << s << std::endl;
        perm(index + 1, s);
        std::swap(s[index], s[k]);
    }
}

int main(int argc, char* argv[])
{
    std::cout << argv[1] << std::endl;
    perm(0, argv[1]);
}

