#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "disk_interface.h"
#include "filesystem_formats.h"
#include "filesystem_interface.h"
#include <string.h>
#include <stdlib.h>
int main(void)
{
    FILE *disk = fopen("disk.img", "r+b");
    // int ret = format_disk(disk,2.5e+8);
    mount(disk);
    


    flush_master_superblock();
    char *path = malloc(100);
    printf("Create Dir1\n");
    strcpy(path,"/dir1/");
    // create_directory("dir1");
    create_nested_directory(path);
    printf("Create Dir1 complete\n");


    free(path);
    path = malloc(100);
    // int write_ret = write_nested_file_data(path, (uint8_t *)data_to_write, strlen(data_to_write));
    // printf("Write return value %d\n",write_ret);
    


   
    strcpy(path,"/dir1/TestingFile.txt");
    printf("Creating nested file %s\n",path);
    create_nested_file(path);

    printf("\nWriting data to %s\n",path);
    char test_write[(DataSize)*2];
    for (size_t c = 0; c < (DataSize)*2; c++) {
        test_write[c] = 'X';
    
    }

    // sizeof(data_to_write);
    int write_ret = write_nested_file_data(path, (uint8_t *)test_write, (DataSize)*2);
    printf("Write return value %d\n",write_ret);
    if(write_ret == 0)
    {
        printf("Write successful\n");
    }
    else
    {
        printf("Write failed\n");
    }
    printf("\n\n");


    printf("\n\nReading data\n\n");
    uint8_t *read_data = NULL;
    size_t data_size = read_nested_file_data(path, &read_data);
    printf("data_size == %zu\n",data_size);
    char *text = (char *)read_data;
    printf("\n");
    if(memcmp(test_write,(char *)read_data, (DataSize)*2) == 0)
    {
        printf("Multi block write works\n");
    }
    return 0;
}

//Proper function for testing
int test_fs()
{
    printf("Please select an option:\n");
    printf("1. Format disk\n");
    printf("2. Create nested directory\n");
    printf("3. Create nested file\n");
    printf("4. Write data to nested file\n");
    printf("5. Read data from nested file\n");


}