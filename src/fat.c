#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/fat.h"

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
static uint16_t fat[4096];
static dir_entry_t root_dir[32];

/* diretorios (incluindo ROOT), 32 entradas de diretorio
com 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
union {
	dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
	uint8_t data[CLUSTER_SIZE];
} data_cluster;

int init() {
	FILE *fat_part = fopen("fat.part", "wb");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		return 1;
	}

	fat[0] = 0xfffd;
	for(int i = 1; i < 10; i++)
		fat[i] = 0xfffe;

	fat[9] = 0xffff;

	for(int i = 10; i < 4096; i++)
		fat[i] = 0x0000;

	uint16_t boot = 0xbbbb;
	for(int i = 0; i < 512; i++)
		fwrite(&boot, sizeof(boot), 1, fat_part);

	fwrite(fat, sizeof(uint16_t), 4096, fat_part);

	memset(root_dir, 0, 32 * sizeof(dir_entry_t));

	fwrite(root_dir, sizeof(dir_entry_t), 32, fat_part);

	uint8_t t[CLUSTER_SIZE];
	memset(t, 0, CLUSTER_SIZE);

	for(int i = 0; i < 4086; i++)
		fwrite(t, 1, CLUSTER_SIZE, fat_part);

	fclose(fat_part);
	printf("FAT inicializada com sucesso\n");
	return 0;
}

int load(){
	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		return 1;
	}

	/*Le a FAT*/
	uint8_t boot[1024];
	fread(boot, 1, 1024, fat_part); 

	fread(fat, sizeof(uint16_t), 4096, fat_part);

	fread(root_dir, sizeof(dir_entry_t), 32, fat_part);

	printf("%x\n",fat[9]);

	fclose(fat_part);
	return 0;
}

int mkdir(char **dir){

	int i;
	
	for(i = 0; i < 32; i++){
		if(root_dir[i].first_block == 0) {
			break;
		}
	}

	if(i == 32){ //nao tem nada vazio aqui
	}

	dir_entry_t new_dir;
	/*strcpy(new_dir.filename, dir);*/
	/*new_dir.filename = strndup(dir, 18);*/
	new_dir.attributes = 1;

	int block;
	for(block = 10; block < 4096 && block == 0x0000; block++);
	new_dir.first_block = block;
	root_dir[i] = new_dir;

	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		return 1;
	}
	return 0;
}

int break_dir(char *dir, char ***dir_list){
	if(dir[0] != '/')
		return -1;
	/*if(*dir_list != NULL)*/
		/*free(*dir_list);*/
	unsigned num_dir = 0;
	for(int i = 0; dir[i] != '\0' && dir[i] != '\n'; i++){
		if(dir[i] == '/'){
			num_dir++;
		}
	}
	
	//se for direto no root
	if(num_dir == 1){
		*dir_list = (char **)malloc(sizeof(char*));
		(*dir_list)[0] = (char*) malloc(18 * sizeof(char));
		(*dir_list)[0] = strtok(dir, "/");
		return 0;
	}

	*dir_list = (char **)malloc(num_dir * sizeof(char*));
	for(int i = 0; i < num_dir; i++){
		(*dir_list)[i] = (char*) malloc(18 * sizeof(char));
	}

	(*dir_list)[0] = strtok(dir, "/");
	/*printf("%s\n",(*dir_list)[0]);*/
	for(int i = 1; i < num_dir; i++){
		(*dir_list)[i] = strtok(NULL, "/");
		/*printf("%s\n",(*dir_list)[i]);*/
	}
	return 0;
}
