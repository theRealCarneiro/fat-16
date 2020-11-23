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

int init(); //foi
int load(); //foi
int ls();
int mkdir(); 
int create();
int unlink();
int write();
int append();
int read();

int break_dir(char *dir, char ***dir_list);

#endif /* __FAT_H */
