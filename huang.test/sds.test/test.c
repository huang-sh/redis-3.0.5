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
    char *p = "123---456---798---010---lkis---sdsdsd----";
    int count = 0;
    sds *l = sdssplitlen(p, strlen(p), "---", 3, &count);
    for (int i = 0; i < count; i++)
    {
        printf("%s\n", l[i]);
    }
    printf("---------------------\n");
    sdsfreesplitres(l, count);
    count = 0;
    sds *arr = sdssplitargs("timeout 10086\r\nport 123321\r\n", &count);
    for (int i = 0; i < count; ++i)
    {
        printf("%s\n", arr[i]);
    }
    sdsfreesplitres(arr, count);
    char *argv[]={"123", "456", "789"}, *sep="$$";
    int argc = 3;
    sds str = sdsjoin(argv, argc, sep);
    printf("\n%s\n", str);
    sdsfree(str);
    printf("测试sdsupdatelen:\n");
    sds test = sdsnew("foobar");
    test[2] = '\0';
    printf("%d\n", sdslen(test));
    sdsupdatelen(test);
    printf("%d\n", sdslen(test));
    return 0;
}
