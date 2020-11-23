# FAT 16

### Comandos disponíveis

| Comando                 | Descrição                                         |
| ----------------------- | ------------------------------------------------- |
| init                    | inicializa o sistema de arquivos em branco        |
| load                    | carregga um sistema de arquivos do disco          |
| mkdir  [path]           | lista diretório                                   |
| create [path]           | cria diretório                                    |
| unlink [path]           | exclui arquivo ou diretório                       |
| write  "string" [path]  | sobrescreve dados em um arquivo                   |
| append "string" [path]  | anexa dados em um arquivo                         |
| read   [path]           | le conteúdo de um arquivo                         |

### Para compilar

```sh
$ make
```

### Para executar 

```sh
$ ./fat_shell
```
