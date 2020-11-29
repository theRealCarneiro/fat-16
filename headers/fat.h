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

/* inicialza o sistema de arquivos */
int init() ;

/* carrega a tabela fat e o diretorio raiz para a memoria principal */
int load();

/* le um cluster de memoria no disco */
data_cluster read_data_cluster(unsigned index);

/* grava um cluster no disco */
void write_data_cluster(unsigned index, data_cluster cluster);

/* navega no sistema de arquivos */
int dir_nav(char **dir_list, int dir_num, int *index, int want);

/*imprime na tela os diretorios e arquivos de um determinado diretorio*/
int ls(char *dir);

/*cria um novo diretorio*/
int mkdir(char *dir);

/*cria um arquivo*/
int create(char *dir);

/*deleta um arquivo ou diretorio*/
int unlink(char *dir);

/*escreve uma string em um arquivo*/
int write(char *string, char *dir);

/*concatena uma string em um arquivo*/
int append(char *string, char *dir);

/*le um arquivo e imprime na tela*/
int read(char *dir);

/*quebra um caminho de diretorio em varias strings*/
int break_dir(char *dir, char ***dir_list);

/*grava a tabela fat no disco*/
int save_fat();

/*quebra uma string em varios clusters*/
int break_str_into_clusters(char *string, data_cluster **buffer);

//define o diretorio atual
int cd(char *dir);

// retorno
#define ROOT_DIR		0
#define DIR_NOT_FOUND	1
#define DIR_EXIST		2
#define DIR_FULL		3
#define DIR_READY		4
#define NOT_A_DIR		5
#define NOT_A_FILE		6

// request
#define WANT_CLUSTER	0
#define WANT_PARENT		1

#endif /* __FAT_H */
