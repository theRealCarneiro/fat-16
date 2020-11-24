#ifndef __FAT_H
#define __FAT_H

#include <stdint.h>

#define CLUSTER_SIZE 1024

/* entrada de diretorio, 32 bytes cada */
typedef struct {
	uint8_t filename[18];
	uint8_t attributes;
	uint8_t reserved[7];
	uint16_t first_block;
	uint32_t size;
} dir_entry_t;

typedef union data_cluster data_cluster;
union data_cluster{
	dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
	uint8_t data[CLUSTER_SIZE];
};

int init(); //foi
int load(); //foi
int ls(char *dir);
int mkdir(char *dir); 
int create();
int unlink();
int write();
int append();
int read();

int break_dir(char *dir, char ***dir_list);

data_cluster get_data_cluster(unsigned index);
void set_data_cluster(unsigned index, data_cluster cluster);
int dir_nav(char **dir_list, int dir_num, int *index, int want);

#define ROOT_DIR 0
#define DIR_NOT_FOUND 1
#define DIR_EXIST 2
#define DIR_FULL 3
#define DIR_READY 4

//
#define NEW_DIR 1
#define NEW_FILE 1
#define SEARCH_DIR 0


#endif /* __FAT_H */
