/* adlist.c - A generic doubly linked list implementation
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


#include <stdlib.h>
#include "adlist.h"
#include "zmalloc.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
list *listCreate(void)
{
    /* 创建一个新的list. 这个没什么好解释的 */
    struct list *list;

    if ((list = zmalloc(sizeof(*list))) == NULL)
        return NULL;
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Free the whole list.
 *
 * This function can't fail. */
void listRelease(list *list)
{
    /* 回收list双端链表 */
    unsigned long len;
    listNode *current, *next;

    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        // 回收listNode的Value,value是void*类型,
        // 只能通过用户自定义,函数回收
        if (list->free) list->free(current->value);
        zfree(current);
        current = next;
    }
    zfree(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list *listAddNodeHead(list *list, void *value)
{
    /* 创建一个新的node节点,这个节点是头部
     * 这个节点的value将被接管,上层应用不能将value释放*/
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;    // 并没有深拷贝数据
    if (list->len == 0) {
        // 如果当前list存储的大小为0,
        // list的头尾指针都是为node
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;  // 头节点, 上一个节点不存在
        node->next = list->head; // 下一个节点
        list->head->prev = node; // list->head的上一个节点为node
        list->head = node;
    }
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
list *listAddNodeTail(list *list, void *value)
{
    /* 增加一个尾节点,*/
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;    // 并没有深拷贝数据
    if (list->len == 0) {
        // 如果当前list存储的大小为0,
        // list的头尾指针都是为node
        // 不存在上下节点
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;    // 插在尾部,所以上一个节点为原来的末尾节点
        node->next = NULL;  // 没有下一个节点
        list->tail->next = node;    // list->tail的下一个节点为node
        list->tail = node;
    }
    list->len++;
    return list;
}
/**
 * 这是插入一个node节点
 * @param list 链表所在的结构体
 * @param old_node 插入的位置在old_node的前后位置
 * @param after 1 插入在old_node的下一个节点, 0 插入在old_node的上一个节点
 */
list *listInsertNode(list *list, listNode *old_node, void *value, int after) {
    listNode *node;

    if ((node = zmalloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    // after为插入顺序
    if (after) {
        // 将node插入在old_node的下一个节点
        node->prev = old_node;  // node的上一个节点为old_node
        node->next = old_node->next;    // node的下一个节点为old_node->next
        if (list->tail == old_node) {
            // 判断old_node是否为尾节点
            list->tail = node;  // 修改尾节点为node
        }
    } else {
        // 将node插入在old_node的前方
        node->next = old_node;  // node的下一个为old_node
        node->prev = old_node->prev;    // node 的上一个节点是old_node->prev
        if (list->head == old_node) {
            // 判断list-head是否为old_node
            list->head = node;  // 修改头节点node
        }
    }
    // 上面保证了node所指向的的前后关系(node->prev,node->next),以及list的头尾
    // 当没有保证node的前后节点指向node.
    // 下面是保证node的前节点指向node,node的下一个节点指向node
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
void listDelNode(list *list, listNode *node)
{
    if (node->prev) // 是否存在上一个节点
        node->prev->next = node->next;
    else    // node是head
        list->head = node->next;
    if (node->next) // 是否存在下一个节点
        node->next->prev = node->prev;
    else    // node是尾
        list->tail = node->prev;
    // 回收数据空间
    if (list->free) list->free(node->value);
    zfree(node);
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
listIter *listGetIterator(list *list, int direction)
{
    /* 返回一个迭代子 
     * 查看adlist.h中typedef struct listIter  */
    listIter *iter;

    if ((iter = zmalloc(sizeof(*iter))) == NULL) return NULL;
    // 迭代器的方向配置查看adlist.h文件
    if (direction == AL_START_HEAD)
        iter->next = list->head;    // 从未部往头部搜索
    else
        iter->next = list->tail;    // 从头部往尾部搜索
    iter->direction = direction;    // 保存方向标识
    return iter;
}

/* Release the iterator memory */
void listReleaseIterator(listIter *iter) {
    // 回收迭代器的内存
    zfree(iter);
}

/* Create an iterator in the list private iterator structure */
void listRewind(list *list, listIter *li) {
    // 创建一个私有迭代器
    li->next = list->head;
    li->direction = AL_START_HEAD;// 从头开始搜素
}

void listRewindTail(list *list, listIter *li) {
    // 创建一个私有迭代器
    li->next = list->tail;
    li->direction = AL_START_TAIL;// 从尾部开始搜索
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
listNode *listNext(listIter *iter)
{
    // 通过迭代器获取当前迭代器所存储的元素
    // 如果当前的元素不为空,使迭代器向下移动一个单位
    listNode *current = iter->next;

    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
list *listDup(list *orig)
{
    list *copy;
    listIter *iter;
    listNode *node;

    if ((copy = listCreate()) == NULL)
        return NULL;
    copy->dup = orig->dup; // 拷贝value的函数
    copy->free = orig->free;
    copy->match = orig->match;
    // 获取迭代器,从头开始搜索
    iter = listGetIterator(orig, AL_START_HEAD);
    while((node = listNext(iter)) != NULL) {
        void *value;

        if (copy->dup) { // 有设置dup函数
            // 拷贝失败
            value = copy->dup(node->value);
            if (value == NULL) {
                listRelease(copy); // 回收list
                listReleaseIterator(iter); // 回收迭代器
                return NULL;
            }
        } else
            value = node->value;
        // 在尾部增加节点
        if (listAddNodeTail(copy, value) == NULL) {
            listRelease(copy);
            listReleaseIterator(iter);
            return NULL;
        }
    }
    listReleaseIterator(iter);
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
listNode *listSearchKey(list *list, void *key)
{
    /* 这是查找node,和value作比较. 
     * 如果没有设置比较函数(match),
     * 则key将会和所有的node中的value直接比较('==')*/
    /* 返回的node没有深拷贝*/
    listIter *iter;
    listNode *node;

    iter = listGetIterator(list, AL_START_HEAD);
    while((node = listNext(iter)) != NULL) {
        if (list->match) {
            if (list->match(node->value, key)) {
                listReleaseIterator(iter);
                return node;
            }
        } else {
            if (key == node->value) {
                listReleaseIterator(iter);
                return node;
            }
        }
    }
    listReleaseIterator(iter);
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
listNode *listIndex(list *list, long index) {
    /* 根据index查找node
     * 如果index>=0则从头开始搜索
     * 如果index<=-1则从尾开始搜索
     * index为步长,超出返回NULL*/
    listNode *n;

    if (index < 0) {
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
void listRotate(list *list) {
    /* 将尾指针移动到头部 */
    listNode *tail = list->tail;

    // 获取listNode的长度
    if (listLength(list) <= 1) return;

    /* Detach current tail */
    // 分离当前tail,
    list->tail = tail->prev;//修改list中的尾指针,使其指向倒数第二个
    list->tail->next = NULL;
    /* Move it as head */
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}
