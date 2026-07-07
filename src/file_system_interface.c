#include "disk_interface.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "stdint.h"
#include "filesystem_formats.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"
#include "filesystem_interface.h"
superblock_t MasterSuperBlock = {0};
bool FS_mounted = false;
FILE *MountedDisk = NULL;
int format_disk(FILE *disk, uint32_t diskSize)
{
    // (void)disk;   // Remove once you start writing to the disk

    uint32_t totalBlocks = diskSize / BLOCK_SIZE;
    uint32_t dataBlocks = totalBlocks;
    uint32_t oldDataBlocks;

    uint32_t inodeCount;
    uint32_t inodeBytes;
    uint32_t inodeBlocks;

    uint32_t bitmapBytes;
    uint32_t bitmapBlocks;

    uint32_t metadataBlocks;

    do
    {
        oldDataBlocks = dataBlocks;

        // 1 inode per 16 KiB about 4 blocks
        inodeCount = (dataBlocks + 3) / 4;

        inodeBytes = inodeCount * sizeof(Inode);
        inodeBlocks = (inodeBytes + BLOCK_SIZE - 1) / BLOCK_SIZE;

        /* One bit per data block */
        bitmapBytes = (dataBlocks + 7) / 8;
        bitmapBlocks = (bitmapBytes + BLOCK_SIZE - 1) / BLOCK_SIZE;

        /* Superblock + inode table + bitmap */
        metadataBlocks =
            1 +             // Superblock
            inodeBlocks +
            bitmapBlocks;

        dataBlocks = totalBlocks - metadataBlocks;

    } while (dataBlocks != oldDataBlocks);

    printf("Disk Size      : %u bytes\n", diskSize);
    printf("Total Blocks   : %u\n", totalBlocks);
    printf("Data Blocks    : %u\n", dataBlocks);
    printf("Inodes         : %u\n", inodeCount);
    printf("Inode Blocks   : %u\n", inodeBlocks);
    printf("Bitmap Blocks  : %u\n", bitmapBlocks);
    printf("Metadata Blocks: %u\n", metadataBlocks);
    int32_t inodeStart      = 1;
    uint32_t bitmapStart     = inodeStart + inodeBlocks;
    uint32_t dataStart       = bitmapStart + bitmapBlocks;
    superblock_t MainSuperblock = {0};
    MainSuperblock.block_size = BLOCK_SIZE;
    MainSuperblock.total_blocks = totalBlocks;
    MainSuperblock.inode_start = inodeStart;
    MainSuperblock.bitmap_start = bitmapStart;
    MainSuperblock.data_start = dataStart;
    MainSuperblock.magic = MAGIC;
    MainSuperblock.version = 1;
    MainSuperblock.total_inodes = inodeCount;
    MainSuperblock.total_data_blocks = dataBlocks;
    MainSuperblock.current_id = 0;
    MainSuperblock.total_bitmap_blocks = bitmapBlocks;
    char buf[BLOCK_SIZE];
    // Write Superblock
    memset(buf,0,BLOCK_SIZE);
    memcpy(buf, &MainSuperblock, sizeof(MainSuperblock));
    write_sector(disk, 0, buf);

    // Write Inodes
    uint32_t inodesPerBlock = BLOCK_SIZE / sizeof(Inode);

    for (uint32_t block = 0; block < inodeBlocks; block++)
    {
        memset(buf, 0, BLOCK_SIZE);

        Inode *inodeBuf = (Inode *)buf;

        for (uint32_t i = 0; i < inodesPerBlock; i++)
        {
            uint32_t inodeIndex = block * inodesPerBlock + i;

            if (inodeIndex >= inodeCount)
                break;

            inodeBuf[i].current_generation_number = 0;
            inodeBuf[i].type = FREE_INODE;
            inodeBuf[i].file_id = 0;
            inodeBuf[i].size = 0;
        }

        write_sector(disk, inodeStart + block, buf);
    }
    // Write Free Bitmap
    for (uint32_t b=0; b < bitmapBlocks; b++) {
        memset(buf,0,BLOCK_SIZE);
        uint32_t curr_bit_sector = bitmapStart+b;
        write_sector(disk,  curr_bit_sector, buf);
    
    }
    MountedDisk = disk;
    MasterSuperBlock = MainSuperblock;
    Inode RootInode;
    uint32_t inode_index = allocate_inode(DIR_INODE);
    read_inode(inode_index, &RootInode);
    
    uint32_t block_id = allocate_blocks();
    uint8_t blockdata[BLOCK_SIZE];
    Directory RootDir;
    RootDir.number_of_entries = 0;
    memcpy(blockdata,&RootDir, sizeof(RootDir));
    write_block(block_id, blockdata);
    printf("block_id = %u\n",block_id);
    RootInode.blocks[0] = block_id;
    RootInode.file_id = MasterSuperBlock.current_id+1;
    MasterSuperBlock.current_id++;
    RootInode.current_generation_number = 1;
    RootInode.size = sizeof(RootDir);
    RootInode.num_blocks = 1;
    RootInode.start = 's';
    RootInode.end = 'e';
    write_inode(inode_index, &RootInode);
    
    

    /*
        Layout:

        Block 0
        +------------------+
        | Superblock       |
        +------------------+

        Next inodeBlocks blocks
        +------------------+
        | Inode Table      |
        +------------------+

        Next bitmapBlocks blocks
        +------------------+
        | Block Bitmap     |
        +------------------+

        Remaining blocks
        +------------------+
        | Data Blocks      |
        +------------------+
    */

    return 0;
}


int mount(FILE *disk)
{
    char buf[BLOCK_SIZE];
    memset(buf,0,BLOCK_SIZE);
    read_sector(disk, 0, buf);
    superblock_t MainSuperblock = {0};
    memcpy(&MainSuperblock, buf, sizeof(MainSuperblock));
    if(MainSuperblock.magic != MAGIC)
    {
        printf("Error with Magic\n");
        return -1;
    }
    else
    {
        printf("Supernode correct\n");
        MasterSuperBlock = MainSuperblock;
        MountedDisk = disk;
        FS_mounted = true;
    }
    return 0;
}

Inode *get_inode(uint32_t sector, uint32_t inode_num)
{
    uint8_t buf[BLOCK_SIZE];
    // printf("Reading Sector\n");
    read_sector(MountedDisk, sector, buf);
    // printf("Read Sector\n");
    uint32_t offset = 0 + (inode_num * sizeof(Inode));
    Inode *fetched_inode = malloc(sizeof(Inode));
    // printf("Copying Read Inode\n");
    memcpy(fetched_inode, buf+offset,sizeof(Inode));
    // printf("Copied Read Inode\n");

    return fetched_inode;
}

int read_inode(uint32_t inode_num, Inode *inode)
{
    uint32_t inodes_per_block = MasterSuperBlock.block_size / sizeof(Inode);
    uint32_t sector;
    uint32_t index;
    // printf("inodes_per_block = %u\n",inodes_per_block);
    if(inode_num == 0)
    {
        sector = MasterSuperBlock.inode_start;
        index = 0;
    }
    else
    {
        sector = MasterSuperBlock.inode_start +
                      (inode_num / inodes_per_block);

        index = inode_num % inodes_per_block;
    }
    
    // printf("Getting Inode from Sector %u\n",sector);
    *inode = *get_inode(sector, index);
    // printf("Got Inode from sector %u\n",sector);
    return 0;
}
int write_inode(uint32_t inode_num, const Inode *inode)
{
    if (inode_num >= MasterSuperBlock.total_inodes)
        return -1;

    uint32_t inodes_per_block = MasterSuperBlock.block_size / sizeof(Inode);

    uint32_t sector = MasterSuperBlock.inode_start +
                      (inode_num / inodes_per_block);

    uint32_t index = inode_num % inodes_per_block;

    char buf[BLOCK_SIZE];

    read_sector(MountedDisk, sector, buf);

    Inode *inode_table = (Inode *)buf;
    inode_table[index] = *inode;

    write_sector(MountedDisk, sector, buf);

    return 0;
}

uint32_t allocate_inode(uint8_t type)
{
    for (uint32_t i=0; i<MasterSuperBlock.total_inodes; i++) {
        Inode Current_Inode;
        // printf("Getting inode to check\n");
        read_inode(i, &Current_Inode);
        // printf("Got inode to check\n");
        if(Current_Inode.type == FREE_INODE)
        {
            Current_Inode.type = type;
            write_inode(i, &Current_Inode);
            return i;
        }
    }
    return  -1;

}

bool get_bit(uint8_t *bitmap, uint32_t bit)
{
    return (bitmap[bit / 8] & (1 << (bit % 8))) != 0;
}


void set_bit(uint8_t *bitmap, uint32_t bit)
{
    bitmap[bit / 8] |= (1 << (bit % 8));
}

void clear_bit(uint8_t *bitmap, uint32_t bit)
{
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}


void write_bit(uint8_t *bitmap, uint32_t bit, bool value)
{
    if (value)
        set_bit(bitmap, bit);
    else
        clear_bit(bitmap, bit);
}

uint32_t allocate_blocks(void)
{
    uint32_t read_offset = 0;
    uint8_t *bitmap = malloc(MasterSuperBlock.total_bitmap_blocks*BLOCK_SIZE);
    for (uint32_t s = 0; s<MasterSuperBlock.total_bitmap_blocks; s++) {
        read_sector(MountedDisk, MasterSuperBlock.bitmap_start+s, bitmap+read_offset);
        read_offset+=BLOCK_SIZE;
        
    }
    for(uint32_t b = 1;b < MasterSuperBlock.total_data_blocks;b++)
    {
        // printf("Checking bit %u\n",b);
        bool is_block_used = get_bit(bitmap, b);
        if(is_block_used == false)
        {
            set_bit(bitmap, b);
            uint32_t read_offset = 0;
            for (uint32_t s = 0; s<MasterSuperBlock.total_bitmap_blocks; s++) {
                write_sector(MountedDisk, MasterSuperBlock.bitmap_start+s, bitmap+read_offset);
                read_offset+=BLOCK_SIZE;
                
            }
            return b;
        }
    }
    // printf("Issue\n");
    return 0;

}

int write_block(uint32_t block_num,uint8_t data[BLOCK_SIZE])
{
    uint32_t block_sector = MasterSuperBlock.data_start+block_num;
    write_sector(MountedDisk, block_sector, data);
}

int read_block(uint32_t block_num,uint8_t *data)
{
    uint32_t block_sector = MasterSuperBlock.data_start+block_num;
    read_sector(MountedDisk, block_sector, data);
}

int write_file_data(char filename[10],uint8_t *data,uint32_t size)
{
    Inode RootInode;
    read_inode(0, &RootInode);

    uint8_t *buffer = malloc(RootInode.num_blocks*BLOCK_SIZE);
    
    for (uint32_t i = 0; i < RootInode.num_blocks; i++) {
        read_block(RootInode.blocks[i], buffer + i * BLOCK_SIZE);
    }
    Directory *RootDir = (Directory *)buffer;
    for (uint32_t i = 0; i < RootDir->number_of_entries; i++) {
        DirEntry *entry = &RootDir->entries[i];
        printf("Checking entry %s\n",entry->name);
        if(strcmp(entry->name, filename) == 0)
        {
            uint32_t file_inode_index = entry->inode_index;
            Inode file_inode;;
            read_inode(file_inode_index, &file_inode);
            uint32_t num_blocks_needed = (size/BLOCK_SIZE)+1;
            uint32_t offset = 0;
            uint32_t copy_size = 0;
            uint32_t store_size = size;
            file_inode.current_generation_number++;
            DataBlock sizing;
            if(size > sizeof(sizing.data))
            {
                copy_size = sizeof(sizing.data);

            }
            else{
                copy_size = size;
            }
            for (uint32_t bc = 0 ; bc < num_blocks_needed; bc++) {
                uint32_t block_id = allocate_blocks();
                file_inode.blocks[file_inode.num_blocks] = block_id;
                
                DataBlockHeader header;
                header.file_id = file_inode.file_id;
                header.generation_number = file_inode.current_generation_number;
                header.is_old = false;
                header.block_number = file_inode.num_blocks;
                header.size = copy_size;
                DataBlock data_block;
                data_block.header = header;
                memcpy(data_block.data, data+offset, copy_size);
                write_block(block_id, (uint8_t *)&data_block);
                offset+=copy_size;
                copy_size = size-sizeof(data_block.data);



                file_inode.num_blocks++;
                // uint8_t chunk[BLOCK_SIZE] = {0};
                // memcpy(chunk, data+offset,copy_size );
                // write_block(block_id, chunk);
                // copy_size = store_size-BLOCK_SIZE;
            }
            file_inode.size = size;
            write_inode(file_inode_index, &file_inode);
            return 0;



        }
    }
    return -1;
}


int create_directory(char dirname[10])
{
    Inode RootInode;
    read_inode(0, &RootInode);

    uint8_t *buffer = malloc(RootInode.num_blocks*BLOCK_SIZE);
    for (uint32_t i = 0; i < RootInode.num_blocks; i++) {
        read_block(RootInode.blocks[i], buffer + i * BLOCK_SIZE);
    }
    Directory *RootDir = (Directory *)buffer;
    DirEntry *entry = &RootDir->entries[RootDir->number_of_entries];

    strcpy(entry->name, dirname);
    uint32_t dir_inode_index = allocate_inode(DIR_INODE);
    entry->inode_index = dir_inode_index;

    RootDir->number_of_entries++;
    RootInode.size =
    sizeof(uint32_t) +
    RootDir->number_of_entries * sizeof(DirEntry);
    for (uint32_t i = 0; i < RootInode.num_blocks; i++)
    {
        write_block(RootInode.blocks[i],
                    buffer + i * BLOCK_SIZE);
    }
    write_inode(0, &RootInode);
    Directory NewDir;
    NewDir.number_of_entries = 0;
    uint32_t block_id = allocate_blocks();
    uint8_t blockdata[BLOCK_SIZE];
    memcpy(blockdata,&NewDir, sizeof(NewDir));
    write_block(block_id, blockdata);
    Inode dir_inode;;
    read_inode(dir_inode_index, &dir_inode);
    dir_inode.blocks[0] = block_id;
    dir_inode.file_id = MasterSuperBlock.current_id+1;
    MasterSuperBlock.current_id++;
    dir_inode.current_generation_number = 0;
    dir_inode.size = sizeof(NewDir);
    dir_inode.num_blocks = 1;
    dir_inode.start = 's';
    dir_inode.end = 'e';
    write_inode(dir_inode_index, &dir_inode);
    return 0;

}


int create_nested_directory(char *path)
{
    Inode RootInode; // This is the inital root inode that we will start from
    uint32_t Current_Inode_index; // This is the current inode that we will be working with as we traverse the path
    read_inode(0, &RootInode);
    //Get root Dir list
    Directory *CurrentDir = malloc(RootInode.num_blocks*BLOCK_SIZE);
    for (uint32_t i = 0; i < RootInode.num_blocks; i++) {
        read_block(RootInode.blocks[i], (uint8_t *)CurrentDir + i * BLOCK_SIZE);
    }
    char *current_dir_name_from_path = malloc(128);
    uint32_t path_index = 0;
    if(path[0] == '/')
    {
        path_index = 1; // Skip the leading '/'
    }
    while (1)
    {   
        //Get dir name from path
        uint32_t dir_name_index = 0;

        while (path[path_index] != '/' && path[path_index] != '\0')
        {
            current_dir_name_from_path[dir_name_index] = path[path_index];
            dir_name_index++;
            path_index++;
        }
        bool found_part = false;
        current_dir_name_from_path[dir_name_index] = '\0';
        for (uint32_t direntry_index =0; direntry_index < CurrentDir->number_of_entries; direntry_index++) {
            DirEntry *entry = &CurrentDir->entries[direntry_index];
            printf("Checking entry (%u/%u)%s against %s\n", direntry_index, CurrentDir->number_of_entries, entry->name, current_dir_name_from_path);
            if(strcmp(entry->name, current_dir_name_from_path) == 0)
            {
                //Load inode for this dir entry
                Inode dir_inode;;
                read_inode(entry->inode_index, &dir_inode);
                if(dir_inode.type == DIR_INODE)
                {
                    //Load dir for this inode
                    Directory *NewDir = malloc(dir_inode.num_blocks*BLOCK_SIZE);
                    for (uint32_t i = 0; i < dir_inode.num_blocks; i++) {
                        read_block(dir_inode.blocks[i], (uint8_t *)NewDir + i * BLOCK_SIZE);
                    }
                    CurrentDir = NewDir;
                    path_index++; // Move to the next part of the path
                    found_part = true;
                    Current_Inode_index = entry->inode_index;
                    break; // Break out of the for loop to process the next part of the path
                }
            }
            
        }
        if(found_part == false)
            {
                if(path_index >= strlen(path)-1)
                {
                    //We have reached the end of the path and did not find the dir, so we need to create it
                    DirEntry *new_entry = &CurrentDir->entries[CurrentDir->number_of_entries];
                    strcpy(new_entry->name, current_dir_name_from_path);
                    printf("Creating new directory %s\n", current_dir_name_from_path);
                    uint32_t new_dir_inode_index = allocate_inode(DIR_INODE);
                    new_entry->inode_index = new_dir_inode_index;
                    CurrentDir->number_of_entries++;
                    Inode new_dir_inode;;
                    read_inode(new_dir_inode_index, &new_dir_inode);
                    new_dir_inode.file_id = MasterSuperBlock.current_id+1;
                    MasterSuperBlock.current_id++;
                    new_dir_inode.current_generation_number = 0;
                    write_inode(new_dir_inode_index, &new_dir_inode);
                    //TODO. We also need to write the current dir back to disk here. using Current_Inode_index
                    Inode Current_Inode;;
                    read_inode(Current_Inode_index, &Current_Inode);
                    //Write CurrentDir back to disk
                    for (uint32_t i = 0; i < Current_Inode.num_blocks; i++)
                    {
                        write_block(Current_Inode.blocks[i],
                                    (uint8_t *)CurrentDir + i * BLOCK_SIZE);
                    }
                    
                    return 0;

                    
                }
                else
                {
                    printf("path_index == %u\n",path_index);
                    printf("strlen(path) == %zu\n",strlen(path));
                    printf("Error: %s not found\n", current_dir_name_from_path);
                    return -1;
                }
            }
    }
    
    //TODO Find the error in this code, i think its the first for loop. coz i dont think its iterating through the path correctly. i think it should be a while loop instead of a for loop. and also the path_index should be incremented inside the while loop. and also the current_dir_name_from_path should be reset to empty string after each iteration of the while loop.
    //TODO. THAT WAS FLIPPING AI. Okay so i also think i really need to look at the inode/root inode and stop using the AI coz its messing with my thinking
    //TODO. It needs to. 1. Load RootInode, 2. Load Rootdir. 3. Loop through the path. 4. Check each path part against each entry in the current dir. 5. If it finds a path that matchs, it needs to load the Inode for that entry, then load the dir for that entry, loop through each entires in the new dir. once it reaches the end of the path. [THIS IS WHERE IM UNSuRE]. Next line
    // TODO cont.. It either needs to create a new dir entry in the current dir, and then point it to a new inode, and update the current dirs inode data. 
    //TODO cont... Or it needs to create a new inode, But idk im tired and i hope this makes sense in the morning

}


int flush_master_superblock()
{
    char buf[BLOCK_SIZE];
    memset(buf,0,BLOCK_SIZE);
    memcpy(buf, &MasterSuperBlock, sizeof(MasterSuperBlock));
    write_sector(MountedDisk, 0, buf);
}

int read_file_data(char filename[10],uint8_t *data,uint32_t *size)
{
    Inode RootInode;
    read_inode(0, &RootInode);

    uint8_t *buffer = malloc(RootInode.num_blocks*BLOCK_SIZE);
    
    for (uint32_t i = 0; i < RootInode.num_blocks; i++) {
        read_block(RootInode.blocks[i], buffer + i * BLOCK_SIZE);
    }
    Directory *RootDir = (Directory *)buffer;
    for (uint32_t i = 0; i < RootDir->number_of_entries; i++) {
        DirEntry *entry = &RootDir->entries[i];
        if(strcmp(entry->name, filename) == 0)
        {
            uint32_t file_inode_index = entry->inode_index;
            Inode file_inode;;
            read_inode(file_inode_index, &file_inode);
            uint32_t offset = 0;
            for (uint32_t bc = 0 ; bc < file_inode.num_blocks; bc++) {
                DataBlock data_block;
                read_block(file_inode.blocks[bc], (uint8_t *)&data_block);
                memcpy(data+offset, data_block.data, data_block.header.size);
                offset+=sizeof(data_block.data);
            }
            *size = file_inode.size;
            printf("Size == %u\n",*size);
            return 0;
        }
    }
    return -1;
}



int create_nested_file(char *path)
{
    char *token = strtok(path, "/");
    printf("Creating nested file %s\n",token);
    Inode RootInode;
    read_inode(0, &RootInode);

    uint8_t *buffer = malloc(RootInode.num_blocks*BLOCK_SIZE);
    
    for (uint32_t i = 0; i < RootInode.num_blocks; i++) {
        read_block(RootInode.blocks[i], buffer + i * BLOCK_SIZE);
    }
    while (token != NULL)
    {
        printf("%s\n", token);
        
        Directory *RootDir = (Directory *)buffer;
        bool found = false;
        for (uint32_t i = 0; i < RootDir->number_of_entries; i++) {
            DirEntry *entry = &RootDir->entries[i];
            if(strcmp(entry->name, token) == 0)
            {
                uint32_t file_inode_index = entry->inode_index;
                Inode file_inode;;
                read_inode(file_inode_index, &file_inode);
                if(file_inode.type == DIR_INODE)
                {
                    uint8_t *dir_buffer = malloc(file_inode.num_blocks*BLOCK_SIZE);
                    for (uint32_t i = 0; i < file_inode.num_blocks; i++) {
                        read_block(file_inode.blocks[i], dir_buffer + i * BLOCK_SIZE);
                    }
                    found = true;
                    buffer = dir_buffer;
                }
                else
                {
                    printf("Error: %s is not a directory\n", token);
                    return -1;
                }

            }

        }
        if(found == false){
            
            printf("Error: %s not found\n", token);
            return -1;
        }
        token = strtok(NULL, "/");

    }
    Directory *FinalDir = (Directory *)buffer;
    DirEntry *entry = &FinalDir->entries[FinalDir->number_of_entries];
    strcpy(entry->name, token);
    uint32_t file_inode_index = allocate_inode(FILE_INODE);
    entry->inode_index = file_inode_index;
    Inode file_inode;;
    read_inode(file_inode_index, &file_inode);
    file_inode.file_id = MasterSuperBlock.current_id+1;
    MasterSuperBlock.current_id++;
    file_inode.current_generation_number = 0;
    write_inode(file_inode_index, &file_inode);
    FinalDir->number_of_entries++;
    for (uint32_t i = 0; i < RootInode.num_blocks; i++)
    {
        write_block(RootInode.blocks[i],
                    buffer + i * BLOCK_SIZE);
    }
    write_inode(0, &RootInode); 
    return 0;
}


int create_file(char filename[10])
{
    // printf("Getting Inode index\n");
    int file_inode_index = allocate_inode(FILE_INODE);
    // printf("Got Inode index %d\n",file_inode_index);
    Inode file_inode;;
    // printf("Reading Inode data\n");
    read_inode(file_inode_index, &file_inode);
    // printf("Read inode data\n");
    // printf("Copying Inode filename\n");
    // printf("Copied Filename\n");
    file_inode.file_id = MasterSuperBlock.current_id+1;
    MasterSuperBlock.current_id++;
    file_inode.current_generation_number = 0;
    write_inode(file_inode_index, &file_inode);
    Inode RootInode;
    read_inode(0, &RootInode);

    uint8_t *buffer = malloc(RootInode.num_blocks*BLOCK_SIZE);
    uint32_t offset = 0;
    for (uint32_t i = 0; i < RootInode.num_blocks; i++) {
        read_block(RootInode.blocks[i], buffer + i * BLOCK_SIZE);
    }
    Directory *RootDir = (Directory *)buffer;
    DirEntry *entry = &RootDir->entries[RootDir->number_of_entries];

    strcpy(entry->name, filename);
    entry->inode_index = file_inode_index;

    RootDir->number_of_entries++;
    RootInode.size =
    sizeof(uint32_t) +
    RootDir->number_of_entries * sizeof(DirEntry);
    for (uint32_t i = 0; i < RootInode.num_blocks; i++)
    {
        write_block(RootInode.blocks[i],
                    buffer + i * BLOCK_SIZE);
    }
    write_inode(0, &RootInode);
    // uint8_t data[BLOCK_SIZE]
    
    
}
