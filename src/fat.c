#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/fat.h"
#define STR_EQUAL(a,b) strcmp(a,b) == 0

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
static uint16_t fat[4096];
static dir_entry_t root_dir[32];

/* diretorios (incluindo ROOT), 32 entradas de diretorio
com 32 bytes cada = 1024 bytes ou bloco de dados de 1024 bytes*/
 
typedef union data_cluster data_cluster;

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
	/*uint8_t boot[1024];*/
	/*fread(boot, 1, 1024, fat_part); */
	fseek(fat_part, 1024, SEEK_SET); // Chega até a FAT.

	fread(fat, sizeof(uint16_t), 4096, fat_part);

	fread(root_dir, sizeof(dir_entry_t), 32, fat_part);

	printf("%x\n",fat[9]);

	fclose(fat_part);
	return 0;
}

data_cluster get_data_cluster(unsigned index){
	FILE *fat_part = fopen("fat.part", "rb+");
	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}

	data_cluster cluster;
	memset(&cluster, 0, CLUSTER_SIZE);

	/*fseek(fat_part, 10 * sizeof(data_cluster), SEEK_SET); // Chega aos clusteres de dados.*/
	fseek(fat_part, index * sizeof(data_cluster), SEEK_CUR); // Vai ao índice desejado.
	fread(&cluster, sizeof(data_cluster), 1, fat_part); // Lê o cluster e coloca no union.
	fclose(fat_part);
	return cluster;
}

void set_data_cluster(unsigned index, data_cluster cluster){
	FILE *fat_part = fopen("fat.part", "rb+");
	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}
	/*fseek(fat_part, 10 * sizeof(data_cluster), SEEK_SET); // Chega aos clusteres de dados*/
	fseek(fat_part, index * sizeof(data_cluster), SEEK_SET); // Vai ao índice desejado
	fwrite(&cluster, sizeof(data_cluster), 1, fat_part); // Lê o union e escreve no cluster
	fclose(fat_part);
}

int dir_nav(char **dir_list, int dir_num, int *index, int want){
	if(dir_list == NULL){ //retorna o index do /
		*index = 9;
		return DIR_EXIST;
	}
	
	int i;
	for(i = 0; i < 32; i++){
		if(STR_EQUAL(root_dir[i].filename, dir_list[0])) {
			break;
		}
	}
	if(i == 32)
		return DIR_NOT_FOUND;
	
	*index = root_dir[i].first_block;
	data_cluster data = get_data_cluster(*index);
	int j;
	for(i = 1; i < dir_num - want; i++){
		for(j = 0; j < 32; j++){
			if(data.dir[j].first_block != 0 &&
			   data.dir[j].attributes == 1 &&
			   STR_EQUAL(data.dir[j].filename, dir_list[i])) {
				if(i == dir_num - 1 - want) {
					*index = data.dir[j].first_block;
				}
				data = get_data_cluster(data.dir[j].first_block);
				break;
			}
		}
		if(j == 32)
			return DIR_NOT_FOUND;
	}
	if(want == NEW_DIR || want == NEW_FILE)
		return DIR_READY;
	else
		return DIR_EXIST;
}

int ls(char *dir){

	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}
	char **dir_list = NULL;
	/*int i;*/
	int retorno = break_dir(dir, &dir_list);
	if(retorno == -1){ //Se ouver algum erro
		return -1;
	}
	int index;
	data_cluster data;
	switch(dir_nav(dir_list, retorno, &index, SEARCH_DIR)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretorio nao encontrado\n");
			break;
		case DIR_EXIST:
			data = get_data_cluster(index);
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0) {
					printf("%s ", data.dir[i].filename);
				}
			}
			printf("\n");
			break;
	}
	return 0;
}

int mkdir(char *dir){

	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}

	char **dir_list = NULL;
	int i;
	int retorno = break_dir(dir, &dir_list);
	if(retorno == -1){ //Se ouver algum erro
		return -1;
	}
	int index;
	data_cluster data;
	switch(dir_nav(dir_list, retorno, &index, NEW_DIR)){
	}

	if(retorno == ROOT_DIR){ //Se for um diretorio no /

		for(i = 0; i < 32; i++){
			if(root_dir[i].first_block == 0) {
				break;
			}
		}
			
		if(i == 32){ //nao tem nada vazio aqui
			return -1;
		}

		dir_entry_t new_dir[32];
		memset(new_dir, 0, 32 * sizeof(dir_entry_t));
		
		if(strlen(dir_list[0]) <= 18)
			strncpy(root_dir[i].filename, dir_list[0], strlen(dir_list[0]));
		else 
			strncpy(root_dir[i].filename, dir_list[0], 18);
		root_dir[i].attributes = 1;

		int block;
		for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
		root_dir[i].first_block = block;
		fat[block] = 0xffff;

		fseek(fat_part, CLUSTER_SIZE, SEEK_SET); // Chega até a FAT.
		fwrite(fat, sizeof(uint16_t), 4096, fat_part);

		fwrite(root_dir, sizeof(dir_entry_t), 32, fat_part);

		fseek(fat_part, CLUSTER_SIZE * (10 - block), SEEK_CUR); 
		fwrite(new_dir, sizeof(dir_entry_t), 32, fat_part);
	}
	else if(retorno >= 1){
		for(i = 0; i < 32; i++){
			if(STR_EQUAL(root_dir[i].filename, dir_list[0])) {
				break;
			}
		}
		data_cluster data = get_data_cluster(root_dir[i].first_block);
		int k;
		int index_to_parent_dir = root_dir[i].first_block;
		for(int j = 1; j < retorno - 1; j++){
			for(k = 0; k < 32; k++){
				if(data.dir[k].first_block != 0 &&
				   data.dir[k].attributes == 1 &&
				   STR_EQUAL(data.dir[k].filename, dir_list[j])) {
					if(j == retorno - 2)
						index_to_parent_dir = data.dir[k].first_block;
					data = get_data_cluster(data.dir[k].first_block);
					break;
				}
			}
		}

		for(i = 0; i < 32; i++){
			if(data.dir[i].first_block == 0) {
				break;
			}
		}
			
		if(i == 32){ //nao tem nada vazio aqui
			return -1;
		}


		data_cluster new_dir;
		memset(new_dir.dir, 0, 32 * sizeof(dir_entry_t));
		
		if(strlen(dir_list[0]) < 18)
			strncpy(data.dir[i].filename, dir_list[retorno - 1], strlen(dir_list[retorno - 1]));
		else 
			strncpy(data.dir[i].filename, dir_list[retorno - 1], 17);
		data.dir[i].attributes = 1;
		/*root_dir[i].reserved = 0;*/

		int block;
		for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
		data.dir[i].first_block = block;
		fat[block] = 0xffff;

		fseek(fat_part, CLUSTER_SIZE, SEEK_SET); // Chega até a FAT.
		fwrite(fat, sizeof(uint16_t), 4096, fat_part);

		set_data_cluster(index_to_parent_dir, data);
		set_data_cluster(block, new_dir);
	}

	/*dir_entry_t new_dir;*/
	/*strcpy(new_dir.filename, dir);*/
	/*new_dir.filename = strndup(dir, 18);*/
	/*new_dir.attributes = 1;*/

	/*int block;*/
	/*for(block = 10; block < 4096 && block == 0x0000; block++);*/
	/*new_dir.first_block = block;*/
	/*root_dir[i] = new_dir;*/

	/*fseek();*/

	fclose(fat_part);
	return 0;
}

int break_dir(char *dir, char ***dir_list){
	if(dir == NULL || (dir[0] == '/' && strlen(dir) == 1))
		return ROOT_DIR;
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
	return num_dir;
}
