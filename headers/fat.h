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

/* diretorios (incluindo ROOT), 32 entradas de diretorio
com 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
typedef union data_cluster data_cluster;
union data_cluster{
	dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
	uint8_t data[CLUSTER_SIZE];
};

int init(); 
int load(); 
int ls(char *dir); 
int mkdir(char *dir);  
int create(char *dir); 
int unlink(char *dir); 
int write(char *string, char *dir);
int append(char *string, char *dir);
int read(char *dir);

data_cluster read_data_cluster(unsigned index);
void write_data_cluster(unsigned index, data_cluster cluster);
int dir_nav(char **dir_list, int dir_num, int *index, int want);
int break_dir(char *dir, char ***dir_list);
int save_fat();
int break_str_into_clusters(char *string, data_cluster **buffer);

// retorno
#define ROOT_DIR		0
#define DIR_NOT_FOUND	1
#define DIR_EXIST		2
#define DIR_FULL		3
#define DIR_READY		4
#define NOT_A_DIR		5
#define NOT_A_FILE		6

// request
#define NEW_DIR		1
#define NEW_FILE		1
#define UNLINK_DIR		1
#define WRITE_TO_FILE	1
#define SEARCH_DIR		0


#endif /* __FAT_H */
