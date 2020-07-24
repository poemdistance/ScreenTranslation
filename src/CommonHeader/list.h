#ifndef __LIST_H__
#define __LIST_H__

struct Node;
typedef struct Node* ptrToNode;
typedef ptrToNode List;
typedef ptrToNode Position;
typedef char* ElementType;

struct MemoryNode;

void deleteList( List L );
int  isEmpty( List L );
void insertToRight( Position position, ElementType data );
void insertToDown( Position position, ElementType data );
void delete( ptrToNode node );
ptrToNode getNode();

ptrToNode getNewNode();

/*  NODE_______   ---> NODE_______   ---> NODE_______  --->
 * [           ]  !   [           ]  !  [           ]  !   
 * [ char[512] ]  !   [ char[512] ]  !  [ char[512] ]  !   
 * [           ]  !   [           ]  !  [           ]  !   
 * [   right -----!   [   right -----!  [   right -----!   (横向存储想要的条目)
 * [   down    ]      [   down    ]     [   down    ]      (纵向存储对应条目包含的N个选项)
 * [_____↓_____]      [_____._____]     [_____._____]      
 *   ____↓                  .                 . 
 *  ↓                       .                 .
 *  NODE_______  
 * [           ] 
 * [ char[512] ] 
 * [           ] 
 * [   right   ]
 * [   down    ] 
 * [_____|_____] 
 *       |
 *
 * */

typedef struct Node {

    char str[512];
    ptrToNode right;
    ptrToNode down;

}Node;

typedef struct MemoryNode {

    ptrToNode *nodeAddr;
    struct MemoryNode *next;

} MemoryNode;

#endif
