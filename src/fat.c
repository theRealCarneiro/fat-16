#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/fat.h"
#define STR_EQUAL(a,b) strcmp(a,b) == 0

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
static uint16_t fat[4096];
static dir_entry_t root_dir[32];

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
	fat[4095] = 0x66;

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
	fseek(fat_part, 1024, SEEK_SET); // Chega até a FAT.

	fread(fat, sizeof(uint16_t), 4096, fat_part);

	fread(root_dir, sizeof(dir_entry_t), 32, fat_part);

	fclose(fat_part);
	printf("FAT carregada com sucesso\n");
	return 0;
}

data_cluster read_data_cluster(unsigned index){
	FILE *fat_part = fopen("fat.part", "rb+");
	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}

	data_cluster cluster;
	memset(&cluster, 0, CLUSTER_SIZE);

	fseek(fat_part, index * CLUSTER_SIZE, SEEK_CUR); // Vai ao índice desejado.
	fread(&cluster, CLUSTER_SIZE, 1, fat_part); // Lê o cluster e coloca no union.
	fclose(fat_part);
	return cluster;
}

void write_data_cluster(unsigned index, data_cluster cluster){
	FILE *fat_part = fopen("fat.part", "rb+");
	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}
	fseek(fat_part, index * sizeof(data_cluster), SEEK_SET); // Vai ao índice desejado
	fwrite(&cluster, CLUSTER_SIZE, 1, fat_part); // Lê o union e escreve no cluster
	fclose(fat_part);
}

int dir_nav(char **dir_list, int dir_num, int *index, int want){
	if(dir_list == NULL){ //retorna o index do /
		*index = 9;
		return DIR_EXIST;
	}
	
	int i;
	*index = 9;
	data_cluster data = read_data_cluster(*index);
	int j;
	for(i = 0; i < dir_num - want; i++){
		for(j = 0; j < 32; j++){
			if(data.dir[j].first_block != 0 && STR_EQUAL(data.dir[j].filename, dir_list[i])) {
				if(data.dir[j].attributes == 1){
					if(i == dir_num - 1 - want) 
						*index = data.dir[j].first_block;
					data = read_data_cluster(data.dir[j].first_block);
					break;
				} 
				else{
					if(i == dir_num - 1 - want) 
						*index = data.dir[j].first_block;
					data = read_data_cluster(data.dir[j].first_block);
					return NOT_A_DIR;
				}
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
	#include "../headers/fat_shell.h"

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
			fprintf(stderr, "Diretório não encontrado\n");
			break;
		case DIR_EXIST:
			data = read_data_cluster(index);
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0) {
					if(data.dir[i].attributes == 1) 
						printf(AZUL("%s "), data.dir[i].filename);
					else 
						printf(CLEAR("%s "), data.dir[i].filename);
				}
			}
			printf("\n");
			break;
		case NOT_A_DIR:
			fprintf(stderr, "Não é um diretório\n");
			break;
	}
	free(dir_list);
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
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretorio nao encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Diretorio ja existe\n");
			break;
		case DIR_READY:
			/*printf("a");*/
			data = read_data_cluster(index);

			data_cluster new_dir;
			memset(new_dir.dir, 0, 32 * sizeof(dir_entry_t));
			int pos = retorno - 1;

			if(pos < 0)
				pos = 0;
			int i, achou_espaco = 0;
			for(int j = 0; j < 32; j++){
				if(strcmp(data.dir[j].filename, dir_list[pos]) == 0){
					fprintf(stderr, "Diretorio/Arquivo ja existe\n");
					return 1;
				}

				if(achou_espaco == 0 && data.dir[j].first_block == 0) {
					achou_espaco = 1;
					i = j;
				}
			}
			
			if(strlen(dir_list[0]) < 18)
				strncpy(data.dir[i].filename, dir_list[pos], strlen(dir_list[pos]));
			else 
				strncpy(data.dir[i].filename, dir_list[pos], 17);
			data.dir[i].attributes = 1;

			int block;
			for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
			data.dir[i].first_block = block;
			fat[block] = 0xffff;

			save_fat();

			write_data_cluster(index, data);
			write_data_cluster(block, new_dir);

			break;
	}

	free(dir_list);
	fclose(fat_part);
	return 0;
}

int create(char *dir){

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
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretorio nao encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Diretorio/Arquivo ja existe\n");
			break;
		case DIR_READY:
			data = read_data_cluster(index);

			data_cluster new_file;
			memset(new_file.data, 0, CLUSTER_SIZE);
			int pos = retorno - 1;
			if(pos < 0)
				pos = 0;
			/*printf("%d ", index);*/

			int i, achou_espaco = 0;
			for(int j = 0; j < 32; j++){
				if(strcmp(data.dir[j].filename, dir_list[pos]) == 0){
					fprintf(stderr, "Diretorio/Arquivo ja existe\n");
					return 1;
				}

				if(data.dir[j].first_block == 0 && achou_espaco == 0) {
					achou_espaco = 1;
					i = j;
				}
			}
			
			
			if(strlen(dir_list[0]) < 18)
				strncpy(data.dir[i].filename, dir_list[pos], strlen(dir_list[pos]));
			else 
				strncpy(data.dir[i].filename, dir_list[pos], 17);
			data.dir[i].attributes = 0;

			int block;
			for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
			data.dir[i].first_block = block;
			fat[block] = 0xffff;

			save_fat();

			write_data_cluster(index, data);
			write_data_cluster(block, new_file);

			break;
	}

	free(dir_list);
	fclose(fat_part);
	return 0;
}

int unlink(char *dir){
	/*#include "../headers/fat_shell.h"*/

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
	int pos = retorno - 1;
	if(pos < 0)
		pos = 0;
	dir_entry_t aux;
	switch(dir_nav(dir_list, retorno, &index, UNLINK_DIR)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretório/Arquivo não encontrado\n");
			break;
		case DIR_READY:
			data = read_data_cluster(index);
			memset(&aux, 0, 32);
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0) { //se o diretorio/arquivo existe 
					if(STR_EQUAL(data.dir[i].filename, dir_list[pos])) {
						if(data.dir[i].attributes == 1){ //se for um diretorio
							data_cluster temp = read_data_cluster(data.dir[i].first_block);
							int j;
							for(j = 0; j < 32 && temp.dir[i].first_block != 0; j++);
							if(j == 32){
								fprintf(stderr, "Diretório não está vazio\n");
								return 1;
							}
							else{
								fat[data.dir[i].first_block] = 0;
								data.dir[i] = aux;
								write_data_cluster(index, data);
								save_fat();
							}
						} 
						else{ //se for um arquivo
							int posicao = data.dir[i].first_block;
							while(fat[posicao] != 0xffff){
								int t = posicao;
								posicao = fat[posicao];
								fat[t] = 0;
							}
							data.dir[i] = aux;
							write_data_cluster(index, data);
							save_fat();
						}
					}
				}
			}
	}
	free(dir_list);
	return 0;
}

int write(char *string, char *dir){

	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}
	char **dir_list = NULL;
	int retorno = break_dir(dir, &dir_list);
	if(retorno == -1){ //Se ouver algum erro
		return -1;
	}
	int pos = retorno - 1;
	if(pos < 0)
		pos = 0;
	int index;
	int num_buffer;
	data_cluster *buffer;
	switch(dir_nav(dir_list, retorno, &index, SEARCH_DIR)){
		case NOT_A_DIR: 
			num_buffer = break_str_into_clusters(string, &buffer);
			int index_final = index;
			while(fat[index_final] != 0xffff){ //caso tenha blocos extras que não são mais necessarios
				int t = index_final;
				index_final = fat[index_final]; 
				fat[t] = 0;
			}
			fat[index_final] = 0;
			fat[index] = 0xffff;

			write_data_cluster(index, buffer[0]);
			if(num_buffer > 1)
				for(int i = 1; i < num_buffer; i++){
					int block;
					for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
					fat[index] = block;
					index = block;
					fat[index] = 0xffff;
					write_data_cluster(index, buffer[i]);
				}
			
			save_fat();
			printf("Arquivo sobrescrito com sucesso\n");
			break;

		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretório não encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Não é um arquivo\n");
			break;
	}
	free(dir_list);
	return 0;
}

int append(char *string, char *dir){

	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}
	char **dir_list = NULL;
	int retorno = break_dir(dir, &dir_list);
	if(retorno == -1){ //Se ouver algum erro
		return -1;
	}
	int pos = retorno - 1;
	if(pos < 0)
		pos = 0;
	int index;
	data_cluster data;
	data_cluster *buffer;
	int file_size;
	int num_buffer;
	switch(dir_nav(dir_list, retorno, &index, SEARCH_DIR)){
		case NOT_A_DIR:
			while(fat[index] != 0xffff){
				index = fat[index]; 
			}


			data = read_data_cluster(index);
			file_size = strlen(data.data);

			if(file_size + strlen(string) < 1024){
				strcat(data.data, string);							
				write_data_cluster(index, data);
			}
			else{ //caso não tenha espaço o suficiente no cluster

				int tmp = 1024 - file_size;
				strncat(data.data, string, tmp); //cluster cheio agr precisa de um novo
				write_data_cluster(index, data); //grava o primeiro cluster do arquivo
				num_buffer = break_str_into_clusters(&string[tmp], &buffer);

				int block;
				for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
				fat[index] = block;
				fat[block] = 0xffff;

				write_data_cluster(block, buffer[0]);
				if(num_buffer > 1)
					for(int i = 1; i < num_buffer; i++){
						/*printf("%s\n", buffer[i]);*/
						int block;
						for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
						fat[index] = block;
						index = block;
						fat[index] = 0xffff;
						write_data_cluster(index, buffer[i]);
					}
				save_fat();
				
			}
			printf("Arquivo concatenado com sucesso\n");
			break;
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretório não encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Não é um arquivo\n");
			break;
		case DIR_READY:
			break;
	}
	free(dir_list);
	return 0;
}

int read(char *dir){
	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}
	char **dir_list = NULL;
	int retorno = break_dir(dir, &dir_list);
	if(retorno == -1){ //Se ouver algum erro
		return -1;
	}
	int pos = retorno - 1;
	if(pos < 0)
		pos = 0;
	int index;
	data_cluster data;
	uint8_t buffer[1024];
	memset(buffer, 0, 1024);
	switch(dir_nav(dir_list, retorno, &index, SEARCH_DIR)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Arquivo não encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Não é um arquivo\n");
			break;
		case NOT_A_DIR:
			data = read_data_cluster(index);
			printf("%s", data.data);
			while(fat[index] != 0xffff){
				data = read_data_cluster(fat[index]);
				printf("%s", data.data);
				index = fat[index]; 
			}
			printf("\n");
			break;
	}
	free(dir_list);
	return 0;
}

int break_dir(char *dir, char ***dir_list){
	//se for no root
	if(dir == NULL || (dir[0] == '/' && strlen(dir) == 1)){
		return ROOT_DIR;
	}
	//se o formato estiver invalido
	if(dir[0] != '/')
		return -1;

	//conta quantos diretorios tem na cadeia
	unsigned num_dir = 0;
	for(int i = 0; dir[i] != '\0' && dir[i] != '\n'; i++){
		if(dir[i] == '/'){
			num_dir++;
		}
	}
	
	//se for um diretorio no root
	if(num_dir == 1){
		*dir_list = (char **)malloc(sizeof(char*));
		(*dir_list)[0] = (char*) malloc(18 * sizeof(char));
		(*dir_list)[0] = strtok(dir, "/");
		return 1;
	}
	
	//aloca a memoria
	*dir_list = (char **)malloc(num_dir * sizeof(char*));
	/*for(int i = 0; i < num_dir; i++){*/
		/*(*dir_list)[i] = (char*) malloc(18 * sizeof(char));*/
	/*}*/

	(*dir_list)[0] = strtok(dir, "/");
	/*printf("%s\n",(*dir_list)[0]);*/
	for(int i = 1; i < num_dir; i++){
		(*dir_list)[i] = strtok(NULL, "/");
		/*printf("%s\n",(*dir_list)[i]);*/
	}
	return num_dir;
}

int save_fat(){
	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}

	fseek(fat_part, CLUSTER_SIZE, SEEK_SET); // Chega até a FAT.
	fwrite(fat, sizeof(uint16_t), 4096, fat_part);
	fclose(fat_part);
	return 0;
}

int break_str_into_clusters(char *string, data_cluster **buffer){
	int str_size = strlen(string);
	int num_clusters = str_size / CLUSTER_SIZE;
	int left_over = str_size % CLUSTER_SIZE;
	if(num_clusters == 0){
		*buffer = (data_cluster*)malloc(CLUSTER_SIZE);
		memset(*buffer, 0, CLUSTER_SIZE);
		memcpy(&(*buffer)[0], string, str_size);
		return 1; //retorna o numero de clusters
	}
	else{
		int more;
		if(left_over > 0)
			more = 1;
		else more = 0;

		int i;
		*buffer = (data_cluster*)malloc(CLUSTER_SIZE * (num_clusters + more));
		for(i = 0; i < num_clusters; i++){
			memset(&(*buffer)[i], 0, CLUSTER_SIZE);
			memcpy(&(*buffer)[i].data, &string[i * (CLUSTER_SIZE - 1)], CLUSTER_SIZE - 1);
			((*buffer)[i].data)[CLUSTER_SIZE] = '\0';
		}
		if(more){//se tiver um resto que não ocupa exatamente um cluster inteiro
			memset(&(*buffer)[i], 0, CLUSTER_SIZE);
			memcpy(&(*buffer)[i].data, &string[i * (CLUSTER_SIZE - 1)], left_over);
			(*buffer)[i].data[left_over] = '\0';
		}
		return (num_clusters + more);
	}
}
