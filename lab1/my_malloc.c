//
//  my_malloc.c
//  Lab1: Malloc
//

#include "my_malloc.h"
#include <unistd.h>

MyErrorNo my_errno=MYNOERROR;
static int MAGICNUMBER = 123456;
FreeListNode first_node;


FreeListNode free_list_begin(){
    return first_node;
}

void insert_node(FreeListNode node){
    //if list is empty
    if(free_list_begin() == NULL){
        first_node = node;
        first_node->flink = NULL;
    }
    //if the node being inserted is less than the first node
    else if((uint32_t*)node <= (uint32_t*)first_node){
        node->flink = first_node;
        first_node = node;
    }
    //traverse the list and insert the node where needed
    else{
        FreeListNode current = first_node;
        while(current->flink != NULL && (uint32_t*)(current->flink) <= (uint32_t*)node){
            current = current->flink;
        }
        node->flink = current->flink;
        current->flink = node;
    }
}

void delete_node(FreeListNode node){
    if(node == first_node){
        first_node->flink = NULL;
        first_node = NULL;
        return;
    }

    FreeListNode current;
    FreeListNode prev;

    for(current = first_node; current != NULL; prev = current, current = current->flink){
        if(current = node){
            if(prev != NULL){
                prev->flink = current->flink;
            }
            else{
                first_node = current->flink;
            } 
            current->flink = NULL;
            break;
        }
    }
}

void *my_malloc(uint32_t size){
    if(size == 0){
        return NULL;
    }

    uint32_t * ptr;

    size += 8;  //bytes for bookkeeping;
    if(size % 8 != 0){ //make chunk size divisble by 8
        size += (8 - size % 8);
    }

    uint32_t minimum_chunk_size = 16; //set minimum chunk size
    if(size > 16){
        minimum_chunk_size = size;
    }

    //check free list empty
    if(free_list_begin() == NULL){
        uint32_t * free_heap_ptr = (uint32_t*)sbrk(0);
        FreeListNode node = (FreeListNode)free_heap_ptr;
        if(size > 8192){
            sbrk(size);
            node->size = size;
        }
        else{
            sbrk(8192);
            node->size = 8192;
        }
        //set the size of the node
        insert_node(node);
        ptr = (uint32_t*)free_heap_ptr;
    }

    //traverse the free list:
    uint32_t usable_chunk = 1;

    FreeListNode free_chunk_ptr = free_list_begin();

    //check the first node
    if(free_chunk_ptr->size >= size){
        usable_chunk = 0;
        ptr = (uint32_t*)free_chunk_ptr;
    }

    //traverse free list to find usable chunk
    while(free_chunk_ptr->flink != NULL && usable_chunk == 1){
        if(free_chunk_ptr->size >= size){
            usable_chunk = 0;
            ptr = (uint32_t*)free_chunk_ptr;
            break;
        }
        free_chunk_ptr = free_chunk_ptr->flink;
    }

    //if no usable chunks found, create new chunk at end of list
    if(usable_chunk == 1){
        uint32_t * free_heap_ptr = (uint32_t*)sbrk(0);
        FreeListNode node = (FreeListNode)free_heap_ptr;
        if(size > 8192){
            sbrk(size);
            node->size = size;
        }
        else{
            sbrk(8192);
            node->size = 8192;
        }
        //set the size of the node
        insert_node(node);
        ptr = (uint32_t*)free_heap_ptr;
    }

    //oversized chunk: if chunk can be halved, put remainder into free list
    if(((FreeListNode)ptr)->size > minimum_chunk_size){
        //split chunk
        uint32_t chunk_size = ((FreeListNode)ptr)->size;
        chunk_size -= size;
        uint32_t * new_ptr = ptr;
        FreeListNode new_node = (FreeListNode)(new_ptr + chunk_size);
        new_node->size = chunk_size;
        new_node->flink = NULL;
        delete_node((FreeListNode)ptr);
        insert_node(new_node);
    }

    //if chunk_begin is still null:
    if(ptr == NULL){
        my_errno = MYENOMEM;
        return NULL;
    }

    //initialize bookkeeping variables in the header
    *ptr = size;
    *(ptr + 1) = MAGICNUMBER; //MAGICNUMBER indicates that the chunk is malloc'd

    return ptr + 2;

}
      
void my_free(void *ptr){
    //check if chunk is malloc'd
    if(ptr == NULL || *((uint32_t*)ptr - 1) != MAGICNUMBER){
        my_errno = MYBADFREEPTR;
    }
    else{
        FreeListNode node = (FreeListNode)ptr;
        node->size = *((uint32_t*)ptr - 2);
        node->flink = NULL;
        //if free list empty
        insert_node(node);
    }
}

void coalesce_free_list(){
    FreeListNode head = free_list_begin();
    //traverse linked list
    while(head->flink != NULL){
        //test if nodes are adjacent
        if((uint32_t*)head + head->size/4 + 1 == (uint32_t*)(head->flink)){
            //merge the two nodes together
            FreeListNode next = head->flink;
            uint32_t new_size = head->size + next->size;
            head->size = new_size;
            head->flink = next->flink;
            next = NULL;
        }
        head = head->flink;
    }
}