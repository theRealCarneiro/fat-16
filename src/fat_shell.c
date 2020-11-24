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
			
		char *args[2];
		int i;
		/*char **dir_list = NULL;*/
		for(i = 0; i < 10 && strcmp(primeiro_comando, comandos_disponiveis[i]) != 0; i++);

		if(i > 1){ //Se nao for init nem load

			if(tamanho_comando != strlen(primeiro_comando)){ //Se o comando nao estiver incompleto

				if(comando != NULL){

					if(i == 6 || i == 7){ //write e append
						args[0] = strtok(NULL, "\" "); //recebe a string

						if(comando != NULL){ //caso o comando nao esteja incompleto
							args[1] = strtok(NULL, "");
							/*break_dir(args[0], &dir_list); //retorna a lista de diretorios*/
						} else fprintf(stderr, "Comando invalido\n");
					} 
					else{
						args[0] = strtok(NULL, "");
						/*break_dir(args[0], &dir_list);*/
					}
				} 
			} else if(i != 10 && i != 2) //comando incompleto
				fprintf(stderr, "Comando invalido\n");
		}

			
		
			
		if(i < 11){
			switch(i){
				case 0: //init
					init();
					break;
				case 1: //load
					load();
					break;
				case 2: //ls
					if(comando == NULL)
						ls(NULL);
					else
						ls(args[0]);
					break;
				case 3: ;//mkdir
					mkdir(args[0]);
					break;
				case 4: //create
					break;
				case 5: //unlink
					break;
				case 6: //write
					printf("%s\n",args[0]);
					break;
				case 7: //append
					break;
				case 8: //read
					break;
				case 9: //clear
					system("clear");
					break;
				case 10: //quit
					break;
			}
		}
		else
			printf("Comando não encontrado\n");
		/*free(dir_list);*/
	}
	while(strcmp(comando, "quit") != 0);
	return 0;
}

