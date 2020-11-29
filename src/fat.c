#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/fat.h"
#define STR_EQUAL(a,b) strcmp(a,b) == 0

/* 8 clusters da tabela FAT, 4096 entradas de 16 bits = 8192 bytes*/
static uint16_t fat[4096];
static dir_entry_t root_dir[32];

/* inicialza o sistema de arquivos */
int init() {
	FILE *fat_part = fopen("fat.part", "wb");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		return 1;
	}

	/* inicializa a tabela fat */
	fat[0] = 0xfffd;
	for(int i = 1; i < 10; i++)
		fat[i] = 0xfffe;
	fat[9] = 0xffff;
	for(int i = 10; i < 4096; i++)
		fat[i] = 0x0000;
	
	/* grava os clusters de boot */
	uint16_t boot = 0xbbbb;
	for(int i = 0; i < 512; i++)
		fwrite(&boot, sizeof(boot), 1, fat_part);
	/* grava a tabela fat no disco */
	fwrite(fat, sizeof(uint16_t), 4096, fat_part);
	
	/* inicializa o diretório raiz */
	memset(root_dir, 0, 32 * sizeof(dir_entry_t));

	/* grava o diretorio raiz */
	fwrite(root_dir, sizeof(dir_entry_t), 32, fat_part);

	/* inicializa o restando do disco com o valor 0 */
	uint8_t t[CLUSTER_SIZE];
	memset(t, 0, CLUSTER_SIZE);
	for(int i = 0; i < 4086; i++)
		fwrite(t, 1, CLUSTER_SIZE, fat_part);

	fclose(fat_part);
	printf("FAT inicializada com sucesso\n");
	return 0;
}

/* carrega a tabela fat e o diretorio raiz para a memoria principal */
int load(){
	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		return 1;
	}
	/* Chega até a FAT */
	fseek(fat_part, 1024, SEEK_SET); 

	/*Le a FAT*/
	fread(fat, sizeof(uint16_t), 4096, fat_part);

	/*Le o diretorio raiz*/
	fread(root_dir, sizeof(dir_entry_t), 32, fat_part);

	fclose(fat_part);
	printf("FAT carregada com sucesso\n");
	return 0;
}

/* le um cluster de memoria no disco */
data_cluster read_data_cluster(unsigned index){
	FILE *fat_part = fopen("fat.part", "rb+");
	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}

	/* inicializa um cluster vazio para não ter lixo de memoria na leitura */
	data_cluster cluster;
	memset(&cluster, 0, CLUSTER_SIZE);

	fseek(fat_part, index * CLUSTER_SIZE, SEEK_CUR); // Vai ao índice desejado.
	fread(&cluster, CLUSTER_SIZE, 1, fat_part); // Lê o cluster e coloca no union.
	fclose(fat_part);
	return cluster;
}

/* grava um cluster no disco */
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

/* navega no sistema de arquivos */
int dir_nav(char **dir_list, int dir_num, int *index, int want){
	if(dir_list == NULL){ //retorna o index do /
		*index = 9;
		return DIR_EXIST;
	}
	
	int i;
	*index = 9;
	data_cluster data = read_data_cluster(*index);
	int j;

	/*want = 0 retorna o arquivo ou diretorio, want = 1 retorna o diretorio pai*/
	for(i = 0; i < dir_num - want; i++){

		/*navega nas 32 entradas de diretorio*/
		for(j = 0; j < 32; j++){

			/* testa se existe um diretorio com o nome no caminho */
			if(data.dir[j].first_block != 0 && STR_EQUAL(data.dir[j].filename, dir_list[i])) {
				if(data.dir[j].attributes == 1){
					if(i == dir_num - 1 - want) //caso esteja no diretorio especificado
						*index = data.dir[j].first_block;
					data = read_data_cluster(data.dir[j].first_block);
					break;
				} 
				else{
					if(i == dir_num - 1 - want) //caso esteja no diretorio especificado
						*index = data.dir[j].first_block;
					data = read_data_cluster(data.dir[j].first_block);
					return NOT_A_DIR;
				}
			}
		}
		if(j == 32)
			return DIR_NOT_FOUND;
	}
	if(want == WANT_PARENT || want == WANT_PARENT)
		return DIR_READY;
	else
		return DIR_EXIST;
}

/*imprime na tela os diretorios e arquivos de um determinado diretorio*/
int ls(char *dir){
	/*inclui as definições de cores*/
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
	switch(dir_nav(dir_list, retorno, &index, WANT_CLUSTER)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretório não encontrado\n");
			break;
		case DIR_EXIST:
			data = read_data_cluster(index);
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0) {
					if(data.dir[i].attributes == 1) 
						printf(AZUL("%s "), data.dir[i].filename); //imprime diretorios com a cor azul
					else 
						printf(CLEAR("%s "), data.dir[i].filename); //imprime arquivos com a cor branca
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

/*cria um novo diretorio*/
int mkdir(char *dir){

	FILE *fat_part = fopen("fat.part", "rb+");

	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}

	char **dir_list = NULL;
	int i;
	/* chama a função de quebrar o caminho em varias strings */
	int retorno = break_dir(dir, &dir_list); 
	if(retorno == -1){ //Se ouver algum erro
		return -1;
	}
	int index;
	data_cluster data;
	switch(dir_nav(dir_list, retorno, &index, WANT_PARENT)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretorio nao encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Diretorio ja existe\n");
			break;
		case DIR_READY:
			data = read_data_cluster(index);

			data_cluster new_dir;
			memset(new_dir.dir, 0, 32 * sizeof(dir_entry_t));
			int pos = retorno - 1; 

			if(pos < 0)
				pos = 0;
			int i, achou_espaco = 0;
			for(int j = 0; j < 32; j++){
				/*testa se ja existe um diretorio com o nome desejado*/
				if(strcmp(data.dir[j].filename, dir_list[pos]) == 0){ 
					fprintf(stderr, "Diretorio/Arquivo ja existe\n");
					return 1;
				}

				/*procura uma entrada de diretorio vazia*/
				if(achou_espaco == 0 && data.dir[j].first_block == 0) {
					achou_espaco = 1;
					i = j;
				}
			}
			
			/*coloca o nome no diretorio*/
			if(strlen(dir_list[0]) < 18)
				strncpy(data.dir[i].filename, dir_list[pos], strlen(dir_list[pos]));
			else 
				strncpy(data.dir[i].filename, dir_list[pos], 17);
			data.dir[i].attributes = 1; //1 == diretorio, 0 == arquivo

			/*procura um cluster vazio no disco*/
			int block;
			for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
			data.dir[i].first_block = block;
			fat[block] = 0xffff; //fim de arquivo

			save_fat();
			

			write_data_cluster(index, data); //atualiza o diretorio pai no disco
			write_data_cluster(block, new_dir); //grava o diretorio no disco

			break;
	}

	free(dir_list);
	fclose(fat_part);
	return 0;
}

/*cria um arquivo*/
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
	switch(dir_nav(dir_list, retorno, &index, WANT_PARENT)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretorio nao encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Diretorio/Arquivo ja existe\n");
			break;
		case DIR_READY:
			data = read_data_cluster(index);

			/*iniciliza um cluster vazio para nao haver lixo de memoria*/
			data_cluster new_file;
			memset(new_file.data, 0, CLUSTER_SIZE);
			int pos = retorno - 1;
			if(pos < 0)
				pos = 0;

			int i, achou_espaco = 0;
			for(int j = 0; j < 32; j++){
				/*verifica se ja existe uma entrada de diretorio com o mesmo nome*/
				if(strcmp(data.dir[j].filename, dir_list[pos]) == 0){
					fprintf(stderr, "Diretorio/Arquivo ja existe\n");
					return 1;
				}

				/*procura uma entrada de diretorio vazia*/
				if(data.dir[j].first_block == 0 && achou_espaco == 0) {
					achou_espaco = 1;
					i = j;
				}
			}
			
			/*da o nome ao arquivo*/
			if(strlen(dir_list[0]) < 18)
				strncpy(data.dir[i].filename, dir_list[pos], strlen(dir_list[pos]));
			else 
				strncpy(data.dir[i].filename, dir_list[pos], 17);
			data.dir[i].attributes = 0;

			/*procura um cluster vazio no disco*/
			int block;
			for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
			data.dir[i].first_block = block;
			fat[block] = 0xffff;

			save_fat();

			write_data_cluster(index, data); //atualiza o diretorio pai no disco
			write_data_cluster(block, new_file); //grava o arquivo no disco

			break;
	}

	free(dir_list);
	fclose(fat_part);
	return 0;
}

/*deleta um arquivo ou diretorio*/
int unlink(char *dir){

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
	switch(dir_nav(dir_list, retorno, &index, WANT_PARENT)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretório/Arquivo não encontrado\n");
			break;
		case DIR_READY:
			/* lê  o arquivo/diretorio que deseja remover */
			data = read_data_cluster(index);
			memset(&aux, 0, 32);
			for(int i = 0; i < 32; i++){
				if(data.dir[i].first_block != 0) { //se o diretorio/arquivo existe 
					if(STR_EQUAL(data.dir[i].filename, dir_list[pos])) { //procura o arquivo/diretorio no diretorio pai
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
								printf("Diretório removido com sucesso\n");
							}
						} 
						else{ //se for um arquivo
							int posicao = data.dir[i].first_block;
							while(fat[posicao] != 0xffff){ //limpa recursivamente a fat
								int t = posicao;
								posicao = fat[posicao];
								fat[t] = 0;
							}
							data.dir[i] = aux; // recebe uma entrada de diretorio vazia
							write_data_cluster(index, data); //grava o diretorio pai no disco
							save_fat();
							printf("Arquivo removido com sucesso\n");
						}
					}
				}
			}
	}
	free(dir_list);
	return 0;
}

/*escreve uma string em um arquivo*/
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
	switch(dir_nav(dir_list, retorno, &index, WANT_CLUSTER)){
		case NOT_A_DIR: 
			/*quebra a string em clusters de 1024 bytes*/
			num_buffer = break_str_into_clusters(string, &buffer); 
			int index_final = index;
			while(fat[index_final] != 0xffff){ //caso tenha blocos extras que não são mais necessarios
				int t = index_final;
				index_final = fat[index_final]; 
				fat[t] = 0;
			}
			fat[index_final] = 0;
			fat[index] = 0xffff;

			/*escreve o primeiro cluster*/
			write_data_cluster(index, buffer[0]);
			if(num_buffer > 1)
				for(int i = 1; i < num_buffer; i++){ //grava o restante dos clusters
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
	free(buffer);
	free(dir_list);
	return 0;
}

/*concatena uma string em um arquivo*/
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
	switch(dir_nav(dir_list, retorno, &index, WANT_CLUSTER)){
		case NOT_A_DIR:
			/*procura o ultimo cluster do arquivo*/
			while(fat[index] != 0xffff){ 
				index = fat[index]; 
			}
			
			/*le o cluster*/
			data = read_data_cluster(index);
			file_size = strlen(data.data);

			/*verifica se a string cabe no cluster e e grava*/
			if(file_size + strlen(string) < 1024){ 
				strcat(data.data, string);							
				write_data_cluster(index, data);
			}
			else{ //caso não tenha espaço o suficiente no cluster

				int tmp = 1024 - file_size;
				strncat(data.data, string, tmp); //cluster cheio agr precisa de um novo
				write_data_cluster(index, data); //grava o primeiro cluster do arquivo
				num_buffer = break_str_into_clusters(&string[tmp], &buffer); // quebra o restante da string em clusters

				/*procura um cluster vazio para o primeiro cluster*/
				int block;
				for(block = 10; block < 4096 && fat[block] != 0x0000; block++);
				fat[index] = block;
				fat[block] = 0xffff;

				write_data_cluster(block, buffer[0]); //grava o cluster no disco
				if(num_buffer > 1) //se ouverem mais clusters a serem gravados
					for(int i = 1; i < num_buffer; i++){ //grava o restante dos clusters
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
	free(buffer);
	free(dir_list);
	return 0;
}

/*le um arquivo e imprime na tela*/
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
	uint8_t buffer[1025];
	memset(buffer, 0, 1025);
	switch(dir_nav(dir_list, retorno, &index, WANT_CLUSTER)){
		case DIR_NOT_FOUND:
			fprintf(stderr, "Arquivo não encontrado\n");
			break;
		case DIR_EXIST:
			fprintf(stderr, "Não é um arquivo\n");
			break;
		case NOT_A_DIR:
			/*le o primeiro cluster*/
			data = read_data_cluster(index);
			snprintf(buffer, 1024, "%s", data.data);
			printf("%s", buffer);
			/*le o restante dos clusters*/
			while(fat[index] != 0xffff){
				data = read_data_cluster(fat[index]);
				snprintf(buffer, 1024, "%s", data.data);
				printf("%s", buffer);
				index = fat[index]; 
			}
			printf("\n");
			break;
	}
	free(dir_list);
	return 0;
}

/*quebra um caminho de diretorio em varias strings*/
int break_dir(char *dir, char ***dir_list){
	//se for no root
	if(dir == NULL || strcmp(dir, "/") == 0){
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

	/*quebra o caminho*/
	(*dir_list)[0] = strtok(dir, "/");
	for(int i = 1; i < num_dir; i++){
		(*dir_list)[i] = strtok(NULL, "/");
	}
	return num_dir;
}

/*grava a tabela fat no disco*/
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

/*quebra uma string em varios clusters*/
int break_str_into_clusters(char *string, data_cluster **buffer){
	int str_size = strlen(string);
	int num_clusters = str_size / CLUSTER_SIZE; //numero de clusters cheios
	int left_over = str_size % CLUSTER_SIZE; //restante 
	//caso seja necessario apenas um cluster
	if(num_clusters == 0){
		*buffer = (data_cluster*)malloc(CLUSTER_SIZE);
		memset(*buffer, 0, CLUSTER_SIZE);
		memcpy(&(*buffer)[0], string, str_size);
		return 1; //retorna o numero de clusters
	}
	else{ //caso mais clusters sejam necessarios
		int more;
		if(left_over > 0) //caso tenha mais um resto de string para se guardado
			more = 1;
		else more = 0;

		int i;
		*buffer = (data_cluster*)malloc(CLUSTER_SIZE * (num_clusters + more)); //aloca a memoria para o cluster
		for(i = 0; i < num_clusters; i++){ //quebra a string em varios clusters
			memset(&(*buffer)[i], 0, CLUSTER_SIZE);
			memcpy(&(*buffer)[i].data, &string[i * (CLUSTER_SIZE)], CLUSTER_SIZE);
			((*buffer)[i].data)[CLUSTER_SIZE] = '\0';
		}
		if(more){//se tiver um resto que não ocupa exatamente um cluster inteiro
			memset(&(*buffer)[i], 0, CLUSTER_SIZE);
			memcpy(&(*buffer)[i].data, &string[i * (CLUSTER_SIZE)], left_over);
			(*buffer)[i].data[left_over] = '\0';
		}
		return (num_clusters + more);
	}
}
