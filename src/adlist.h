/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */
/* 基本数据结构的定义 双端链表*/
typedef struct listNode {
    struct listNode *prev;  // 上一个节点
    struct listNode *next;  // 下一个节点
    void *value;            // 存储区
} listNode; // 名称

/* 迭代器 */
typedef struct listIter {
    listNode *next; // 当前迭代子保存的节点位置
    int direction; // 迭代器的方向 查看定义
} listIter;

typedef struct list {
    listNode *head;     // 头节点
    listNode *tail;     // 尾节点
    // 函数指针,void*类型的参数,返回void*的指针
    void *(*dup)(void *ptr);//和拷贝函数(listDup())互相配合的用户自定义函数,主要是拷贝value
    void (*free)(void *ptr);// 回收listNode的Value
    int (*match)(void *ptr, void *key); //是比对value是否相等,的函数
    unsigned long len;      // 长度,双端链表的长度
} list;

/* Functions implemented as macros */
/* 宏定义区 */
#define listLength(l) ((l)->len)    // list长度
#define listFirst(l) ((l)->head)    // list存储的,listNode的头指针
#define listLast(l) ((l)->tail)     // list存储的,listNode的尾指针
#define listPrevNode(n) ((n)->prev) // listNode的上一个节点,双端链表
#define listNextNode(n) ((n)->next) // listNode的下一个节点,双端链表
#define listNodeValue(n) ((n)->value)// listNode的值

/* 设置链表中的函数指针的值 */
#define listSetDupMethod(l,m) ((l)->dup = (m))
#define listSetFreeMethod(l,m) ((l)->free = (m))
#define listSetMatchMethod(l,m) ((l)->match = (m))

/* 获取链表中的函数指针的值 */
#define listGetDupMethod(l) ((l)->dup)
#define listGetFree(l) ((l)->free)
#define listGetMatchMethod(l) ((l)->match)

/* Prototypes 函数原型 */
// 创建 list
list *listCreate(void);
// 回收 list
void listRelease(list *list);
// 在head处增加
list *listAddNodeHead(list *list, void *value);
// 在Tail处增加
list *listAddNodeTail(list *list, void *value);
// 插入一个节点
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
// 删除一个节点
void listDelNode(list *list, listNode *node);
// 获取 迭代器
listIter *listGetIterator(list *list, int direction);
// 获取迭代器当前所指向的指针,
// 并且使迭代器移动到下一个位置
// 移动的方向由direction决定
listNode *listNext(listIter *iter);
// 回收 迭代器
void listReleaseIterator(listIter *iter);
// 复制整个list
list *listDup(list *orig);
// 查找Key
listNode *listSearchKey(list *list, void *key);
// 访问第index位置的listNode
listNode *listIndex(list *list, long index);
// 创建一个私有的迭代器,从头部开始搜索
void listRewind(list *list, listIter *li);
// 创建一个私有的迭代器,从尾部开始搜索
void listRewindTail(list *list, listIter *li);
// 将list中的尾指针(tail)移动到头部(head)
void listRotate(list *list);

/* Directions for iterators */
#define AL_START_HEAD 0 // 从尾部往头部搜索
#define AL_START_TAIL 1 // 从头部往尾部搜索

#endif /* __ADLIST_H__ */
