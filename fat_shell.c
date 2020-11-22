#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fat_shell.h"
#include "fat.h"

int main(){
	char comando[256];

	do {
		printf(PS1);
		/*scanf("%s", comando);*/
		
		if(fgets(comando, 256, stdin) == NULL){
			perror("deu ruim: ");
			return(-1);
		}
		comando[strcspn(comando, "\n")] = '\0';

		int argn = 0;
		for(int i = 0; comando[i] != '\0'; i++)
			if(comando[i] == ' ')
				argn++;
		
		char *primeiro_comando = strtok(comando, " ");
			
		char *args[2];
		int i;
		for(i = 0; i < 10 && strcmp(primeiro_comando, comandos_disponiveis[i]) != 0; i++);
		if(i > 1){
			for(int i = 0; i < argn; i++){
				args[i] = strtok(NULL, " ");
				/*printf("%s\n",args[i]);*/
			}
			
		}
			
		if(i < 10){
			switch(i){
				case 0: //init
					init();
					break;
				case 1: //load
					load();
					break;
				case 2: //ls
					break;
				case 3: //mkdir
					/*mkdir();*/
					break;
				case 4: //create
					break;
				case 5: //unlink
					break;
				case 6: //write
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
	}
	while(strcmp(comando, "quit") != 0);
	return 0;
}
