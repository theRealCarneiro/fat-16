//#ifndef __FAT_SHELL_H
//#define __FAT_SHELL_H

//		testa se strings sao iguais
#define	STR_EQUAL(a,b)	strcmp(a,b) == 0

//		transforma o texto em uma string
#define	STR(TEXTO)	#TEXTO 

//		funcao		escape para a cor	texto	//reseta a cor
#define	VERMELHO(X)	STR(\033[0;31m)	X		STR(\033[0m)
#define	VERDE(X)		STR(\033[0;32m)	X		STR(\033[0m)
#define	AMARELO(X)	STR(\033[0;33m)	X		STR(\033[0m)
#define	AZUL(X)		STR(\033[0;34m)	X		STR(\033[0m)
#define	MAGENTA(X)	STR(\033[0;35m)	X		STR(\033[0m)
#define	CIANO(X)		STR(\033[0;36m)	X		STR(\033[0m)
#define	BRANCO(X)		STR(\033[0;37m)	X		STR(\033[0m)
#define	CLEAR(X)		STR(\033[0m)		X		

#define USER_SYMBLE " $ "
#define MESSAGE "fatshell"

const char *PS1 = VERDE(USER_SYMBLE);

const char *comandos_disponiveis[] = {
	"init",
	"load",
	"ls",
	"mkdir",
	"create",
	"unlink",
	"write",
	"append",
	"read",
	"clear",
	"cd",
	"quit",
};

//#endif /* __FAT_SHELL_H */
