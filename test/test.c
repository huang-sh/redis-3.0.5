#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sds.h"

void test(uint32_t num)
{
    for (int i = 0; i < num; ++i)
    {
        goto jump;
    }
jump:
    return;
}

int main(void)
{
    char *p = "123---456---798--010--";
    int count = 0;
    sds *l = sdssplitlen(p, strlen(p), "---", 3, &count);
    for (int i = 0; i < count; i++)
    {
        printf("%s", l[i]);
        sdsfree(l[i]);
    }
    free(l);
    return 0;
}



















































