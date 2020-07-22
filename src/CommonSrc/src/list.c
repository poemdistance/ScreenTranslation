#include <string.h>
#include <stdlib.h>

#include "../CommonHeader/list.h"
#include "printWithColor.h"

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

void insertToRight( ptrToNode *node, ElementType data ) {

    if ( node == NULL ) return;

    ptrToNode newNode = calloc( 1, sizeof(Node) );
    if ( newNode == NULL )
    {
        pred("calloc newNode failed");
        exit(1);
    }
    
    if ( data == NULL )
    {
        pred("待插入str为空");
        return;
    }

    strcpy( newNode->str, data );
    newNode->down = NULL;
}

void insertToDown( ptrToNode *node ) {

}
