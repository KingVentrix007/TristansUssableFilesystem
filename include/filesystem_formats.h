#ifndef __FILESYSTEM_FORMATS__H
#define __FILESYSTEM_FORMATS__H
#include "stdint.h"
#include <stdint.h>
#include "stdbool.h"
#define  MAGIC 0x54465531
#define FREE_INODE 0x1
#define FILE_INODE 0x2
#define DIR_INODE 0x3
enum {
    BLOCK_SIZE = 4096,
    CHECKSUM_SIZE = 8
};

typedef struct 
{
    uint32_t magic;
    int version;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t inode_start;
    uint32_t total_inodes;
    uint32_t bitmap_start;
    uint32_t total_bitmap_blocks;
    // uint32_t total_bitmap_bits;
    uint32_t data_start;
    uint32_t total_data_blocks;
    uint32_t current_id;
    
}superblock_t;

typedef struct
{
    char start;
    uint32_t size;
    uint32_t blocks[12];
    uint32_t num_blocks;
    uint8_t type;
    uint8_t current_generation_number;
    uint32_t file_id;
    char end;
} Inode;

typedef struct
{
    char name[128];
    uint32_t inode_index;
}DirEntry ;



typedef struct
{
    uint32_t generation_number;
    uint32_t file_id;
    uint32_t size;
    bool is_old;
    uint32_t block_number;
} DataBlockHeader;

#define DataSize BLOCK_SIZE- sizeof(DataBlockHeader)- 32
typedef struct
{
    DataBlockHeader header;
    char data[DataSize];
    uint8_t checksum[32];
} DataBlock;
typedef struct
{
    uint32_t number_of_entries;
    DirEntry entries[];
} Directory;

#endif