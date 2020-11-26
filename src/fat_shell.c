#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../headers/fat_shell.h"
#include "../headers/fat.h"

int main(){
	char comando[256];

	do {
		do{
			printf(PS1);
			if(fgets(comando, 256, stdin) == NULL){
				fprintf(stderr, "Erro ao ler entrada\n");
				return(-1);
			} 
		} while(strcspn(comando, "\n") == 0);
		comando[strcspn(comando, "\n")] = '\0';

		short tamanho_comando = strlen(comando);
		char *primeiro_comando = strtok(comando, " ");

		char *args[2] = {"\0","\0"};
			
		int i;
		for(i = 0; i < 11 && strcmp(primeiro_comando, comandos_disponiveis[i]) != 0; i++);
		char *aux;

		if(i > 1){ //Se não for init nem load
			if(i < 11 && tamanho_comando != strlen(comandos_disponiveis[i])){ //Se o comando não estiver incompleto
				if(comando != NULL){
					if(i == 6 || i == 7){ //write e append
						args[0] = strtok(NULL, "\" "); //recebe a string

						if(comando != NULL){ //caso o comando não esteja incompleto
							args[1] = strtok(NULL, "");
							args[1][strcspn(args[1], " ")] = '\0';
						} else fprintf(stderr, "Comando inválido\n");
					} 
					else{ //
						aux = strtok(NULL, " ");
						if(aux != NULL)
							args[0] = aux;
						aux = strtok(NULL, "");
						if(aux != NULL){
							fprintf(stderr, "Não pode haver espaco no nome de um diretorio\n");
							i = -1;
						}
					}
				} 
			} else if(i != 10 && i != 2) //comando incompleto
				fprintf(stderr, "Comando inválido\n");
		}
			
		switch(i){
			case 0: //init
				init();
				break;
			case 1: //load
				load();
				break;
			case 2: //ls
				if(args[0][0] == '\0'){
					ls(NULL);
					/*printf("aa");*/
				}else
					ls(args[0]);
				break;
			case 3: ;//mkdir
				/*printf("%s",args[0]);*/
				mkdir(args[0]);
				break;
			case 4: //create
				create(args[0]);
				break;
			case 5: //unlink
				unlink(args[0]);
				break;
			case 6: //write
				args[0] = strtok(NULL, "\" "); //recebe a string
				if(comando != NULL){ //caso o comando não esteja incompleto
					args[1] = strtok(NULL, "");
				} else fprintf(stderr, "Comando inválido\n");
				/*printf("%s\n",args[0]);*/
				break;
			case 7: //append
				args[0] = strtok(NULL, "\" "); //recebe a string
				if(comando != NULL){ //caso o comando não esteja incompleto
					args[1] = strtok(NULL, "");
				} else fprintf(stderr, "Comando inválido\n");
				/*printf("append");*/
				break;
			case 8: //read
				printf("read");
				break;
			case 9: //clear
				system("clear");
				break;
			case 10: //quit
				/*exit(0);*/
				break;
			/*default:*/
				/*printf("Comando não encontrado\n");*/
				/*break;*/
		}
	}
	while(strcmp(comando, "quit"));
	return 0;
}

