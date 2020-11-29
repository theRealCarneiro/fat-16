#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/fat.h"
#include "../headers/fat_shell.h"

#define CMD_SIZE 4096

int fs_loaded = 0;
char cur_dir[256] = "/";

int main(){
	char comando[CMD_SIZE];
	char **args = (char**)malloc(sizeof(char*) * 2);
	args[0] = (char*)malloc(sizeof(char) * CMD_SIZE);
	args[1] = (char*)malloc(sizeof(char) * CMD_SIZE);
	memset(args[0], 0, CMD_SIZE);
	memset(args[1], 0, CMD_SIZE);

	do {
		do{
			printf(AZUL("%s"), cur_dir);
			printf(PS1);
			if(fgets(comando, CMD_SIZE, stdin) == NULL){
				fprintf(stderr, "Erro ao ler entrada\n");
				return(-1);
			} 
		} while(strcspn(comando, "\n") == 0); 
		comando[strcspn(comando, "\n")] = '\0';//substinui \n por \0

		/*recebe o primeiro comando*/
		short tamanho_comando = strlen(comando); 
		char *primeiro_comando = strtok(comando, " ");
			
		/*checa se o comando existe*/
		int i;
		for(i = 0; i < 12 && strcmp(primeiro_comando, comandos_disponiveis[i]) != 0; i++);
		char *aux;

		//caso o sistema de arquivo não tenha sido inicializado 
		if((i > 1 && i != 9 && i != 11 || i > 11) && fs_loaded == 0){
			fprintf(stderr, "Erro, sistema de arquivos não foi carregado, use o comando init ou load para iniciar\n");
			i = -1;
		}

		/*quebra a string para o comando especificado*/
		if(i > 1 && i < 11){ //Se não for init nem load
			if(comando != NULL){
				if(i == 6 || i == 7){ //write e append
					aux = strtok(NULL, "\""); //quebra a string
					if(aux != NULL)
						strcpy(args[0],aux);

					if(comando != NULL){ //caso o comando não esteja incompleto
						aux = strtok(NULL, "");//quebra a string
						if(aux != NULL)
							if(aux[0] != '/'){
								/*concatena com o diretorio atual caso esteja procurando nele*/
								if(strcmp(cur_dir, "/") != 0) 
									snprintf(args[1], strlen(cur_dir) + strlen(aux) + 2,"%s/%s", cur_dir, &aux[1]);
								else
									snprintf(args[1], strlen(aux) + 2,"/%s", &aux[1]);
							}
							else
								strcpy(args[1], aux); //caso seja passado o caminho completo 
					} else fprintf(stderr, "Comando inválido\n");
				} 
				else{ //qqr outro comando nao sendo write e append
					aux = strtok(NULL, " "); //quebra a string, separa com espaços
					if(aux != NULL){
						if(aux[0] != '/'){
							/*concatena com o diretorio atual caso esteja procurando nele*/
							if(strcmp(cur_dir, "/") != 0) 
								snprintf(args[0], strlen(cur_dir) + strlen(aux) + 2,"%s/%s", cur_dir, aux);
							else
								snprintf(args[0], strlen(aux) + 2,"/%s", aux);
						}
						else
							strcpy(args[0], aux); //caso seja passado o caminho completo 
					} 
					else if(i != 2)
						fprintf(stderr, "Comando inválido\n");
					else 
						strcpy(args[0], cur_dir);

					aux = strtok(NULL, "");//quebra a string
					if(aux != NULL){ //caso tenha um espaço no comando
						fprintf(stderr, "Não pode haver espaco no nome de um diretorio\n");
						i = -1;
					}
				}
			} 
		}
			
		switch(i){
			case 0: //init
				fs_loaded = 1;
				init();
				break;
			case 1: //load
				fs_loaded = 1;
				load();
				break;
			case 2: //ls
				if(args[0][0] == '\0'){
					ls(NULL);
				}else
					ls(args[0]);
				break;
			case 3: ;//mkdir
				mkdir(args[0]);
				break;
			case 4: //create
				create(args[0]);
				break;
			case 5: //unlink
				unlink(args[0]);
				break;
			case 6: ;//write
				write(args[0], args[1]);
				break;
			case 7: //append
				append(args[0], args[1]);
				break;
			case 8: //read
				read(args[0]);
				break;
			case 9: //clear
				system("clear");
				break;
			case 10: //cd
				cd(args[0]);
				break;
		}
	}
	while(strcmp(comando, "quit"));
	return 0;
}

int cd(char *dir){

	FILE *fat_part = fopen("fat.part", "rb+");
	char aux[256];
	strcpy(aux, dir);
	if(fat_part == NULL) {
		fprintf(stderr, "Erro ao abrir disco FAT fat.part\n");
		exit(1);
	}
	char **dir_list = NULL;
	int retorno = break_dir(aux, &dir_list);
	if(retorno == -1){ //Se ouver algum erro
		return -1;
	}
	int index;
	switch(dir_nav(dir_list, retorno, &index, WANT_CLUSTER)){
		case DIR_EXIST:
			memset(cur_dir, 0, 256);
			strncpy(cur_dir, dir, strlen(dir) + 1);
			break;
		case DIR_NOT_FOUND:
			fprintf(stderr, "Diretório não encontrado\n");
			break;
		case NOT_A_DIR:
			fprintf(stderr, "Não é um diretório\n");
			break;
	}
	free(dir_list);
	return 0;
}
