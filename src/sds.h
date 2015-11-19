/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdarg.h>

typedef char *sds;

struct sdshdr {
    unsigned int len;   // 使用的长度
    unsigned int free;  // 开辟的空间
    // 利用c99(C99 specification 6.7.2.1.16)中引入的
    // flexible array member,通过buf来引用sdshdr后面的地址，
    char buf[];         // 存储
};

/**
 * 获取sds所使用的空间大小
 */
static inline size_t sdslen(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->len;
}

/**
 * 获取sds剩余的空间大小
 */
static inline size_t sdsavail(const sds s) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    return sh->free;
}
// 申请一个sds空间,根据init以及initlen
sds sdsnewlen(const void *init, size_t initlen);
// 申请一个sds空间,根据init
sds sdsnew(const char *init);
// 申请一个空的sds
sds sdsempty(void);
// 获取sds的使用空间
size_t sdslen(const sds s);
// 拷贝一份新额sds
sds sdsdup(const sds s);
// 回收空间
void sdsfree(sds s);
size_t sdsavail(const sds s);
sds sdsgrowzero(sds s, size_t len);
sds sdscatlen(sds s, const void *t, size_t len);
sds sdscat(sds s, const char *t);
sds sdscatsds(sds s, const sds t);
sds sdscpylen(sds s, const char *t, size_t len);
sds sdscpy(sds s, const char *t);

sds sdscatvprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
// fmt是类是于print的,可变变量从3开始
sds sdscatprintf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sdscatprintf(sds s, const char *fmt, ...);
#endif

sds sdscatfmt(sds s, char const *fmt, ...);
sds sdstrim(sds s, const char *cset);
void sdsrange(sds s, int start, int end);
// 更新sds的使用长度
void sdsupdatelen(sds s);
// 清空sds字符串,但不回收空间
void sdsclear(sds s);
int sdscmp(const sds s1, const sds s2);
sds *sdssplitlen(const char *s, int len, const char *sep, int seplen, int *count);
void sdsfreesplitres(sds *tokens, int count);
void sdstolower(sds s);
void sdstoupper(sds s);
sds sdsfromlonglong(long long value);
sds sdscatrepr(sds s, const char *p, size_t len);
sds *sdssplitargs(const char *line, int *argc);
sds sdsmapchars(sds s, const char *from, const char *to, size_t setlen);
sds sdsjoin(char **argv, int argc, char *sep);

/* Low level functions exposed to the user API */
// 扩大sds的存储空间,只有保证free小于 addlen才会扩大
sds sdsMakeRoomFor(sds s, size_t addlen);
// 这个是在原来sds的基础上修改free以及len的属性
// 缩小空间时保证free >= incr 扩大空间时保证len >= incr
void sdsIncrLen(sds s, int incr);
// 缩小sds中buf的空间,仅仅缩小到len,free为0
sds sdsRemoveFreeSpace(sds s);
// 返回sds所在的struct sdshdr的空间大小,包括buf`
size_t sdsAllocSize(sds s);

#endif
