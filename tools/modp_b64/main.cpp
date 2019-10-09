
#include "modp_b64.h"

#include <stdio.h>
#include <string.h>

int main()
{
    const char *  a = "test string.";
    char    b[4096];
    int l = strlen(a) + 1;
    int ll = modp_b64_encode_len(l);
    printf("l = %d, ll = %d\n", l, ll);
    modp_b64_encode(b, a, l);
    printf("%s\n", b);
    return 0;
}

