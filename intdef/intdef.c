/* Various tests which demonstrate deficiencies of integer arithmetic in c.  */

#include <locale.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

static
void f1()
{
    int32_t x = 0x80808080;
    uint32_t y = 0x80808080;
    assert((uint32_t) x == y);
    x >>= 1;
    y >>= 1;
    assert((uint32_t) x == y);
}

static
void f2()
{
    int32_t x = 0x80808080;
    uint32_t y = 0x80808080;
    assert((uint32_t) x == y);
    int shift = 32;
    x >>= shift;
    y >>= shift;
    assert((uint32_t) x == y);
}

static
void f3()
{
    int32_t x = 0x80808080;
    uint32_t y = 0x80808080;
    assert((uint32_t) x == y);
    int shift = 32;
    x <<= shift;
    y <<= shift;
    assert((uint32_t) x == y);
}

static
void f4()
{
    unsigned zero = 0;
    assert(-1 < zero);
}

static
void f5()
{
    char* locale = setlocale(LC_ALL, "en_US.arr1");
    assert(locale);
    signed char c = 0xc0;
    int x = isupper((signed char) c);
    int y = isupper((unsigned char) c);
    assert(x == y);
}

static
void f6()
{
    signed char arr[256];
    memset (arr, 'w', sizeof arr);

    signed char index = 0xf7;
    signed char x = arr[index];
    signed char y = arr[0xf7];
    assert (x == y);
}

static
void f7()
{
    assert(2147483647u > -2147483647 - 1);
}

static
void f8()
{
    int32_t x = 2147483648u;
    int32_t y = 2147483647;
    assert(y < x);
}

static
void f9()
{
    int x = strlen("1") - strlen("11") > 0;
    assert(x == 0);
}

static
void safe_memcpy(char* dst, const char* src, int buflen, int nbytes)
{
    memcpy(dst, src, buflen < nbytes ? buflen : nbytes);
}

static
void f10()
{
    char dst[64], src[64];
    safe_memcpy(dst, src, 64, -1);
}

static
void f11()
{
    char a = 'a';
    size_t x = sizeof(a);
    size_t y = sizeof('a');
    assert(x == y);
}

int main()
{
    f11();
    f10();
    f9();
    f8();
    f7();
    f6();
    f5();
    f4();
    f3();
    f2();
    f1();
    return 0;
}


