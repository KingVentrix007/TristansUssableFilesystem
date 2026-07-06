#ifndef __FILESYSTEM_INTERFACE__H
#define __FILESYSTEM_INTERFACE__H

#include "stdio.h"
#include "stdint.h"
#include "filesystem_formats.h"
int read_inode(uint32_t inode_num, Inode *inode);
uint32_t allocate_inode(uint8_t type);
int format_disk(FILE *disk, uint32_t diskSize);
int mount(FILE *disk);
uint32_t allocate_blocks(void);
int write_block(uint32_t block_num,uint8_t data[BLOCK_SIZE]);
int write_inode(uint32_t inode_num, const Inode *inode);
int write_file_data(char filename[10],uint8_t *data,uint32_t size);
#endif