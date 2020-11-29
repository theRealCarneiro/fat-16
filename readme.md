# FAT 16

### Comandos disponíveis

| Comando                 | Descrição                                         |
| ----------------------- | ------------------------------------------------- |
| init                    | inicializa o sistema de arquivos em branco        |
| load                    | carrega um sistema de arquivos do disco           |
| mkdir  [path]           | lista diretório                                   |
| create [path]           | cria diretório                                    |
| unlink [path]           | exclui arquivo ou diretório                       |
| write  "string" [path]  | sobrescreve dados em um arquivo                   |
| append "string" [path]  | anexa dados em um arquivo                         |
| read   [path]           | le conteúdo de um arquivo                         |
| cd     [path]           | vai ao diretório especificado                     |
| help                    | exibe informações sobre os comandos               |
| quit                    | sai do programa                                   |

### Para compilar

```sh
$ make
```

### Para executar 

```sh
$ ./fat_shell
```

### Para ler o manual 
```sh
$ man ./fat_shell.1
```
Link para o repositório no github: https://github.com/theRealCarneiro/fat-16
