/* This program finds the longest palindrome in a string in linear time.  */

#include <stdio.h>
#include <string.h>

static int longest_palindrome(const char* input)
{
    const size_t n = strlen(input);
    int result = 0;
    const char* begin = input;
    const char* end = input + n;
    const char* left = input + n/2 -1;
    const char* right = input + n/2 + (n%2);
    for (; left >= begin && right < end; --left, ++right, result += 2)
        if (*left != *right)
            break;
    return result + n%2;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "%s <word>\n", argv[0]);
        return 1;
    }
    int n, max = 0;
    size_t inlen = strlen(argv[1]);
    char input[inlen + 1];
    memcpy(input, argv[1], inlen + 1);
    const char* begin = input;
    char* end = input + inlen;

    printf("input = %s\n", input);

    for (; begin < end; ++begin) {
        n = longest_palindrome (begin);
        if (n > max)
            max = n;
        if (max >= end - begin)
            break;
    }
    for (begin = input; end > begin; *--end = '\0') {
        n = longest_palindrome (begin);
        if (n > max)
            max = n;
        if (max >= end - begin)
            break;
    }

    printf("longest palindrome = %d\n", max);
    return 0;
}


