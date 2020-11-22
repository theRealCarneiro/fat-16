# FAT 16

### Comandos disponíveis

| Comando       | Argumentos             | Descrição                                       |
| ------------- |------------------------|-------------------------------------------------|
| init          |                        | inicializa o sistema de arquivos em branco      |
| load          |                        | carregga um sistema de arquivos do disco        |
| mkdir         | /path/to/file          | lista diretório                                 |
| create        | /path/to/file          | cria diretório                                  |
| unlink        | /path/to/file          | exclui arquivo ou diretório                     |
| write         | "string" /path/to/file | sobrescreve dados em um arquivo                 |
| append        | "string" /path/to/file | anexa dados em um arquivo                       |
| read          | /path/to/file          | le conteúdo de um arquivo                       |

### Para compilar

```sh
$ make
```

### Para executar 

```sh
$ ./fat_shell`
```--
