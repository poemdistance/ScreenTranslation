#include <string.h>
#include <stdlib.h>

#include "../CommonHeader/list.h"
#include "printWithColor.h"

static MemoryNode memoryManageHead = { NULL };

/* 返回内存管理链表头部的下一个节点保存的node节点地址
 * 头部节点不保存node节点地址*/
ptrToNode getNode() {

    if ( memoryManageHead.next == NULL )
    {
        ptrToNode newNode = calloc( 1, sizeof(Node) );
        if ( newNode == NULL )
        {
            pred("calloc newNode failed");
            exit(1);
        }

        return newNode;
    }

    MemoryNode *memoryNode = memoryManageHead.next;
    ptrToNode  *nodeAddr   = memoryNode->nodeAddr;

    /* 释放头部的下一个节点, 将下下一个节点链接到头部*/
    memoryManageHead.next = memoryNode->next;
    free( memoryNode );

    return nodeAddr;
}

void delete( ptrToNode node ) {

    MemoryNode *p = &memoryManageHead;

    /* 将p定位到链表末尾*/
    while ( p->next != NULL )
        p = p->next;

    /* 创建新的内存管理节点,存储"删除"的node*/
    MemoryNode *newMemoryNode = calloc ( 1, sizeof(MemoryNode) );
    if ( newMemoryNode == NULL ) 
    {
        pred("calloc newMemoryNode failed");
        exit(1);
    }

    /* 清空node链接的节点*/
    node->right = NULL;
    node->down  = NULL;

    newMemoryNode->nodeAddr = node;
    newMemoryNode->next = NULL;

    /* 将新的节点链接到内存管理链表*/
    p->next = newMemoryNode;
}

int isEmpty( List L) {

    if ( L == NULL )
        return 1;

    return L->right == NULL && \
                     L->down  == NULL;
}

void deleteDownList( List L ) {

    if ( isEmpty( L ) ) return;

    ptrToNode p = L;
    ptrToNode tmp = NULL;

    while ( p != NULL ) {
        tmp = p->right;
        free(p);
        p = tmp;
    }
}

void deleteList( List L ) {

    if ( isEmpty( L ) ) return;

    ptrToNode p = L;
    ptrToNode tmp = NULL;

    while ( p != NULL ) {

        /* 暂存右端节点,防止p被释放后丢失*/
        tmp = p->right;

        /* 删除p下方链表, 并释放p*/
        deleteDownList( p->down );
        free(p);

        /* 使p指向下一个右端节点*/
        p = tmp;
    }
}

/* 插入一个数据为data的新节点到position的后面*/
void insertToRight( Position position, ElementType data ) {

    if ( position == NULL ) return;

    if ( data == NULL )
    {
        pred("待插入str为空");
        return;
    }

    ptrToNode newNode = getNode();

    strcpy( newNode->str, data );

    newNode->down = NULL;

    /* 当前位置的下一个元素变成了新节点的下一个元素*/
    newNode->right = position->right; 

    /* 当前位置的下一个节点变成新增节点*/
    position->right = newNode;
}

void insertToDown( Position position, ElementType data ) {

    if ( position == NULL ) return;

    if ( data == NULL )
    {
        pred("待插入str为空");
        return;
    }

    ptrToNode newNode = getNode();

    strcpy( newNode->str, data );

    newNode->right = NULL;

    /* 当前位置的下一个元素变成了新节点的下一个元素*/
    newNode->down = position->down; 

    /* 当前位置的下一个节点变成新增节点*/
    position->down = newNode;
}
