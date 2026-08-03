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
    int ret = format_disk(disk,2.5e+8);
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
    strcpy(path,"/dir1/fileCat.txt");
    printf("Creating nested file %s\n",path);
    create_nested_file(path);
    // char *data_to_write = "This is a secure text message";
    // int write_ret = write_nested_file_data(path, (uint8_t *)data_to_write, strlen(data_to_write));
    // printf("Write return value %d\n",write_ret);
    
    free(path);


   
    strcpy(path,"/dir1/TestingFile.txt");
    printf("Creating nested file %s\n",path);
    create_nested_file(path);

    printf("\nWriting data to %s\n",path);
    char *data_to_write = "Can you see, that is is imperative that i live to the year 2800. So that i may come back here, kill you, and run away with all my money. I also really like how fish look, taste and smell, but now how they sound, or speak";
    char test_write[(DataSize)*2];
    for (size_t c = 0; c < (DataSize)*2; c++) {
        test_write[c] = 'C';
    
    }

    // sizeof(data_to_write);
    int write_ret = write_nested_file_data(path, (uint8_t *)test_write, (DataSize)*2);
    printf("Write return value %d\n",write_ret);
    printf("\n\n");


    printf("\n\nReading data\n\n");
    uint8_t *read_data = NULL;
    size_t data_size = read_nested_file_data(path, &read_data);
    printf("data_size == %zu\n",data_size);
    char *text = (char *)read_data;
    printf("Read data = [%s]\n",text+1);
    for (size_t i=0; i<100; i++) {
        printf("[%c]",text[i]);
    }
    printf("\n");
    if(memcmp(test_write,(char *)read_data, (DataSize)*2) == 0)
    {
        printf("Multi block write works\n");
    }
    return 0;
}