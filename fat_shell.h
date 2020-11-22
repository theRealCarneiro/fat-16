#define STR_EQUAL(a,b) strcmp(a,b) == 0
#define STR(X) #X

#define VERMELHO STR(\031[0;31m)
#define VERDE STR(\033[0;32m)
#define AMARELO STR(\033[0;33m)
#define AZUL STR(\033[0;34m)
#define MAGENTA STR(\033[0;35m)
#define CIANO STR(\033[0;36m)
#define CLEAR STR(\033[0m)

#define USER_SYMBLE "$ "

const char *PS1 = MAGENTA USER_SYMBLE CLEAR;
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
	"quit",
};
