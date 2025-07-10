//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIN_COUNT 4
#define BIN_WIDTH 1000
#define PAGE_SIZE 4096
//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

typedef struct my_metadata_t
{
    size_t size;
    struct my_metadata_t *next;
} my_metadata_t;

typedef struct my_heap_t
{
    my_metadata_t *free_head;
    my_metadata_t dummy;
} my_heap_t;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
my_heap_t my_heap;

// binの定義
my_heap_t *bins[BIN_COUNT];

//
// Helper functions (feel free to add/remove/edit!)
//

// bin indexを計算
// sizeに応じてどのbinを使うか
int get_bin_index(size_t size)
{
    int index = size / BIN_WIDTH;
    // 最大サイズを超えた場合は最後のbinに入れる
    if (index >= BIN_COUNT)
        index = BIN_COUNT - 1;
    return index;
}

// free listに追加
void my_add_to_free_list(my_metadata_t *metadata)
{
    int index = get_bin_index(metadata->size);
    metadata->next = bins[index]->free_head;
    bins[index]->free_head = metadata;
}

// free listから削除
void my_remove_from_free_list(my_metadata_t *target, my_metadata_t *prev, int index)
{
    if (prev)
    {
        prev->next = target->next;
    }
    else
    {
        bins[index]->free_head = target->next;
    }
}

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
void my_initialize()
{
    my_heap.free_head = &my_heap.dummy;
    my_heap.dummy.size = 0;
    my_heap.dummy.next = NULL;
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size)
{
    my_metadata_t *best_fit = NULL;
    my_metadata_t *best_fit_prev = NULL;
    int best_fit_index = -1;

    printf("要求サイズ:  %zu", size);

    // 必要なサイズから探索する最初のbinのindexを返す
    int start_index = get_bin_index(size);

    // bin[start_index]以降のbinをすべて探索してbest_fitを探す
    //  結果：975,70,640,40,774,51,8009,72,4291,75,
    // for (int i = start_index; i < BIN_COUNT; ++i)
    // {
    //     my_metadata_t *metadata = bins[i].free_head;
    //     my_metadata_t *prev = NULL;
    //     while (metadata)
    //     {
    //         if (metadata->size >= size &&
    //             (!best_fit || metadata->size < best_fit->size))
    //         {
    //             best_fit = metadata;
    //             best_fit_prev = prev;
    //             best_fit_index = i;
    //         }
    //         prev = metadata;
    //         metadata = metadata->next;
    //     }
    //     if (best_fit)
    //         break; // 最小 fitting が見つかったら終了
    // }

    // bins[start_index]だけ探索するコード
    // 結果：971,70,639,40,780,51,7912,72,4253,75

    my_metadata_t *metadata = bins[start_index]->free_head;
    my_metadata_t *prev = NULL;
    while (metadata)
    {
        if (metadata->size >= size && (!best_fit || metadata->size < best_fit->size))
        {
            best_fit_prev = prev;
            best_fit = metadata;
        }
        prev = metadata;
        metadata = metadata->next;
    }

    if (!best_fit)
    {
        size_t buffer_size = PAGE_SIZE;
        my_metadata_t *new_metadata = (my_metadata_t *)mmap_from_system(buffer_size);
        new_metadata->size = buffer_size - sizeof(my_metadata_t);
        new_metadata->next = NULL;
        my_add_to_free_list(new_metadata);
        return my_malloc(size); // 再試行
    }

    void *ptr = best_fit + 1;
    size_t remaining_size = best_fit->size - size;

    my_remove_from_free_list(best_fit, best_fit_prev, best_fit_index);

    if (remaining_size > sizeof(my_metadata_t))
    {
        best_fit->size = size;

        my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
        new_metadata->size = remaining_size - sizeof(my_metadata_t);
        new_metadata->next = NULL;
        my_add_to_free_list(new_metadata);
    }

    return ptr;
}

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr)
{
    // Look up the metadata. The metadata is placed just prior to the object.
    //
    // ... | metadata | object | ...
    //     ^          ^
    //     metadata   ptr
    my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
    // Add the free slot to the free list.
    my_add_to_free_list(metadata);
}

// This is called at the end of each challenge.
void my_finalize()
{
    // Nothing is here for now.
    // feel free to add something if you want!
}

void test()
{
    // Implement here!
    assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
}
