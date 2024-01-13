/*
    COMP3511 Fall 2023
    PA3: Simplified Memory Management (smm)

    Your name:Lam Yeung Kong Sunny
    Your ITSC email:   ykslam        @connect.ust.hk

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks.

*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Data structure of meta_data
struct
    __attribute__((__packed__)) // compiler directive, avoid "gcc" padding bytes to struct
    meta_data
{
    size_t size;            // 8 bytes (in 64-bit OS)
    char free;              // 1 byte ('f' or 'o')
    struct meta_data *next; // 8 bytes (in 64-bit OS)
    struct meta_data *prev; // 8 bytes (in 64-bit OS)
};

// calculate the meta data size and store as a constant (exactly 25 bytes)
const size_t meta_data_size = sizeof(struct meta_data);

// Global variables
void *start_heap = NULL;          // pointing to the start of the heap, initialize in main()
struct meta_data dummy_head_node; // dummy head node of a doubly linked list, initialize in main()
struct meta_data *head = &dummy_head_node;

// The implementation of the following functions are given:
void list_add(struct meta_data *new, struct meta_data *prev, struct meta_data *next);
void list_add_tail(struct meta_data *new, struct meta_data *head);
void init_list(struct meta_data *list);
void mm_print();
void fill_characters(char *p, int n, char ch);

// Students are required to implement these functions below
void *mm_malloc(size_t size)
{
    // TODO: Complete mm_malloc here
    struct meta_data * target_node = head;

    do{ //find sufficiently large free block until reaching dummy_head_node
        if(target_node->size >= size && target_node->free == 'f'){
            if(target_node->size - size < meta_data_size){ //occupy whole block
                target_node->free = 'o';
            }
            else{ //split into two blocks
                struct meta_data temporary_node;

                temporary_node.size = target_node->size - size  - meta_data_size;
                temporary_node.free = 'f';
                temporary_node.next = temporary_node.prev = NULL;

                target_node->size = size;
                target_node->free = 'o';

                struct meta_data* new_node = (struct meta_data*)((char*)target_node + target_node->size + meta_data_size);
                *new_node = temporary_node;

                list_add(new_node, target_node, target_node->next);
            }
            return (void*)target_node + meta_data_size;
        }

        target_node = target_node->next; 
    }while(target_node != &dummy_head_node);

    //when no sufficiently large free block is found
    struct meta_data temporary_node;
    temporary_node.size = size;
    temporary_node.free = 'o';
    temporary_node.prev = temporary_node.next = NULL; 

    struct meta_data* new_node = sbrk(0);
    sbrk(meta_data_size + size);
    *new_node = temporary_node;

    list_add_tail(new_node, head);

    return (void*)(new_node + 1);
}

void mm_free(void *p)
{
    // TODO: Complete mm_free here
    struct meta_data* target = (struct meta_data*)p - 1;
    target->free = 'f';
}

int main()
{
    start_heap = sbrk(0);
    init_list(head);
    // Assume there are at most 26 different malloc/free
    // Here is the mapping rule
    // a=>0, b=>1, ..., z=>25
    void *pointers[26] = {NULL};
    void *target = NULL;

    FILE *fp;
    char command[10];
    char block_name;         // a-z
    unsigned int block_size; // a non-negative integer

    fp = stdin;
    if (fp == NULL)
        exit(1);

    while (fscanf(fp, "%s", command) != EOF)
    {
        if (strcmp(command, "malloc") == 0)
        {
            fscanf(fp, " %c %d", &block_name, &block_size);
            target = mm_malloc(block_size);
            if (target != NULL)
            {
                // This operation ensures that the returned pointer is correct
                // As we only fill characters up to the block size,
                // no meta data should be erased

                // If the program crashes, it means that the returned pointer is wrong
                // you may erase some meta data
                fill_characters(target, block_size, ' ');
            }
            pointers[block_name - 'a'] = target;
            printf("=== %s %c %d ===\n", command, block_name, block_size);
            mm_print();
        }
        else if (strcmp(command, "free") == 0)
        {
            fscanf(fp, " %c", &block_name);
            mm_free(pointers[block_name - 'a']);
            printf("=== %s %c ===\n", command, block_name);
            mm_print();
        }
    }

    fclose(fp);
    return 0;
}

void init_list(struct meta_data *list)
{
    list->next = list;
    list->prev = list;
}

void list_add(struct meta_data *new,
              struct meta_data *prev,
              struct meta_data *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void list_add_tail(struct meta_data *new,
                   struct meta_data *head)
{
    list_add(new, head->prev, head);
}

void mm_print()
{
    struct meta_data *cur = head->next;
    int i = 1;
    while (cur != head)
    {
        printf("Block %02d: [%s] size = %ld bytes\n",
               i,                                    // block number - counting from bottom
               (cur->free == 'f') ? "FREE" : "OCCP", // free or occupied
               cur->size);                           // size, in term of bytes
        i = i + 1;
        cur = cur->next;
    }
}

void fill_characters(char *p, int n, char ch)
{
    for (int i = 0; i < n; i++)
        p[i] = ch;
}