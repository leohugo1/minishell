#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TAM_ENTRADA_MAX 100
#define TAM_COMANDO_MAX 60
struct Comandos
{
    char *nome;
    int argc;
    char *argv[TAM_COMANDO_MAX];
};

struct Comandos Comandos;

char *VerificaCaminho(char **argv, char **dir)
{
    char *result;
    char pName[TAM_COMANDO_MAX];
    if (*argv[0] == '/')
    {
        return argv[0];
    }
    if (*argv[0] == '.')
    {
        if (*++argv[0] == '.')
        {
            if (getcwd(pName, sizeof(pName)) == NULL)
            {
                printf("Erro ao pegar o diretorio atual\n");
            }
            else
            {
                *--argv[0];
                printf(&result, "%s%s%S\n", pName, "/", *argv[0]);
            }
            return result;
        }
        if (*++argv[0] == '/')
        {
            if (getcwd(pName, sizeof(pName)) == NULL)
                perror("getcwd(): error\n");
            else
            {
                asprintf(&result, "%s%s", pName, argv[0]);
            }
            return result;
        }
    }
}

void AtualPath(char *diretorios[])
{
    char *pathvar = (char *)getenv("PATH");
    char *path;

    for (int i = 0; i < TAM_COMANDO_MAX; i++)
    {
        diretorios[i] = NULL;
    }
    path = (char *)malloc(strlen(pathvar) + 1);
    strcpy(path, pathvar);

    int i = 0;
    char *caminho;
    caminho = strtok(path, ":");

    while (caminho != NULL)
    {
        caminho = strtok(NULL, ":");
        diretorios[i] = caminho;
        i++;
    }
}

void Diretorio()
{
    char *dir = Comandos.argv[1];
    if (dir == NULL)
    {
        chdir(getenv("HOME"));
    }
    else
    {
        if (chdir(dir) == -1)
        {
            printf("Diretorio nao encontrado\n");
        }
    }
}

int VerificarComando()
{
    if (strcmp(Comandos.argv[0], "cd") == 0)
    {
        Diretorio();
        return 1;
    }
}

int SeparaArgumentos(char *comandLine, struct Comandos *comandos)
{
    int aux = 0;
    char *token = strtok(comandLine, " ");
    while (token != NULL)
    {
        comandos->argv[aux] = token;
        aux++;
        token = strtok(NULL, " ");
    }
    comandos->argv[aux] = NULL;
    comandos->argc = aux;
    return 0;
}

int RegistrarEntrada(char *comandLine, char *comandInput)
{
    int aux = 0;
    while ((*comandInput != '\n') && (aux < TAM_ENTRADA_MAX))
    {
        comandLine[aux] = *comandInput;
        aux++;
        *comandInput = getchar();
    }
    comandLine[aux] = '\0';

    return 0;
}

int main(int argc, char *argv[])
{
    char *diretorios[TAM_COMANDO_MAX];
    char Entrada = '\0';
    char ListaComandos[TAM_ENTRADA_MAX];
    AtualPath(diretorios);
    printf("%s", diretorios[0]);
    printf("\nWelcome to mini-shell\n");
    while (1)
    {
        Entrada = getchar();
        if (Entrada == '\n')
        {
            printf("Please enter a command: ");
            continue;
        }
        else
        {
            RegistrarEntrada(ListaComandos, &Entrada);
            if (strcmp(ListaComandos, "exit") == 0)
            {
                printf("Bye\n");
                break;
            }
            SeparaArgumentos(ListaComandos, &Comandos);
            if (VerificarComando() == 0)
            {
                continue;
            }
        }
    }
    return 0;
}