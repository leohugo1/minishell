
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <sys/stat.h>
#define MAX_COMMAND_LENGTH (100)
#define MAX_NUM_ARGS (5)

void ignoreSignal(int sig)
{
  if (signal(sig, SIG_IGN) == SIG_ERR)
  {
    printf("Failed to install signal handler for signal %d\n", sig);
    exit(1);
  }
}

void resetSignal(int sig)
{
  if (signal(sig, SIG_DFL) == SIG_ERR)
  {
    printf("Failed to reset signal handler for signal %d\n", sig);
    exit(1);
  }
}
void quit(int status)
{

  kill(0, SIGTERM);
  exit(status);
}
void readCommand(char *buffer, int max_size)
{
  size_t ln;

  char *return_value = fgets(buffer, max_size, stdin);

  if (return_value == NULL)
  {
    quit(1);
  }
  ln = strlen(buffer) - 1;
  if (buffer[ln] == '\n')
  {
    buffer[ln] = '\0';
  }
}

void parseCommand(char *buffer, char **command, char **args)
{
  *command = strtok(buffer, " ");

  int aux = 0;
  char *token = strtok(NULL, " ");
  while (token != NULL)
  {
    args[aux] = token;
    aux++;
    token = strtok(NULL, " ");
  }
  args[aux] = NULL;
}

void goHome()
{
  char *home = getenv("HOME");
  if (chdir(home) == -1)
  {
    fprintf(stderr, "Não foi possivel ir para o diretorio home.\n");
  }
}
void cat(char *caminho)
{
  FILE *fp;
  char c;
  fp = fopen(caminho, "r");
  if (fp == NULL)
  {
    perror("Erro ao abrir o arquivo");
    return;
  }
  c = fgetc(fp);
  while (c != EOF)
  {
    printf("%c", c);
    c = fgetc(fp);
  }
  fclose(fp);
}

void cdArg(char *caminho)
{
  if (chdir(caminho) == -1)
  {
    perror("Erro ao mudar de diretório");
  }
  char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char *last_slash = strrchr(cwd, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
            chdir(strcat(cwd,caminho));
        }
    } else {
        perror("Erro ao obter o diretório atual");
    }
}

void cd()
{
  char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char *last_slash = strrchr(cwd, '/');
        if (last_slash != NULL) {
            *last_slash = '\0';
            chdir(cwd);
        }
    } else {
        perror("Erro ao obter o diretório atual");
    }


}
void pwd()
{
  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  printf("%s\n", cwd);
}

void ListarConteudo(char *directory)
{
  struct dirent *de;
  DIR *dr = opendir(directory);
  if (dr == NULL)
  {
    perror("Não foi possível abrir o diretório");
    return;
  }
  while ((de = readdir(dr)) != NULL)
    printf("%s ", de->d_name);
  printf("\n");
  closedir(dr);
}

void ListarConteudoDetalhado(char *diretorio)
{
  struct dirent *de;
  struct stat fileStat;

  DIR *dr = opendir(diretorio);
  if (dr == NULL)
  {
    perror("Não foi possível abrir o diretório");
    return;
  }
  while ((de = readdir(dr)) != NULL)
  {

    char filePath[1024];
    snprintf(filePath, sizeof(filePath), "%s/%s", diretorio, de->d_name);
    if (stat(de->d_name, &fileStat) == -1)
    {
      perror("Erro ao carregar os status");
      continue;
    }
    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group *gr = getgrgid(fileStat.st_gid);

    printf("%s %2ld %-8s %-8s %8lld %s %s\n",
           (S_ISDIR(fileStat.st_mode)) ? "d" : "-",
           fileStat.st_nlink,
           pw ? pw->pw_name : "",
           gr ? gr->gr_name : "",
           (long long)fileStat.st_size,
           ctime(&fileStat.st_mtime),
           de->d_name);
  }
  printf("\n");
  closedir(dr);
}
int removerArquivo(char *caminho)
{
  struct stat fileStat;
  if (stat(caminho, &fileStat) != 0)
  {
    perror("Erro ao carregar status");
  }
  if (S_ISREG(fileStat.st_mode))
  {
    if (unlink(caminho) != 0)
    {
      perror("Erro ao remover arquivo");
    }
  }
  else if (S_ISDIR(fileStat.st_mode))
  {
    if (rmdir(caminho) != 0)
    {
      perror("Erro ao deletar diretório");
    }
  }
}

void Mkdir(char *caminho)
{
  char *caminhoclone = strdup(caminho);
  char *token = strtok(caminhoclone, "/");
  char  current_path[100]="";
  while (token != NULL)
  {
    strcat(current_path,token);
    strcat(current_path,"/");
    if (mkdir(caminho, 0777) != 0 && errno != EEXIST)
    {
      perror("Erro ao criar diretório");
      free(caminhoclone);
      return;
    }
    token = strtok(NULL, "/");
    
  }
  free(caminhoclone);
}
int CopararArquivos( void* a,  void* b) {
    return strcmp(*( char**)a, *( char**)b);
}

void ordenarArquivos(char* path_pasta) {
    DIR* dir = opendir(path_pasta);

    if (dir == NULL) {
        perror("Erro ao abrir o diretório");
        return;
    }


    char* arquivos[1024];
    int file_count = 0;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            arquivos[file_count] = strdup(entry->d_name);
            file_count++;
        }
    }

    closedir(dir);

    qsort(arquivos, file_count, sizeof(char*), CopararArquivos);

    for (int i = 0; i < file_count; i++) {
        printf("%s\n", arquivos[i]);
        free(arquivos[i]);
    }
}

void CP(char* source_path,char* destination_path){
  DIR* source_dir = opendir(source_path);
   if (source_dir == NULL) {
        FILE* source_file = fopen(source_path, "rb");
        FILE* destination_file = fopen(destination_path, "wb");

    if (source_file == NULL || destination_file == NULL) {
        printf("Failed to open the file.\n");
        return;
    }

    char buffer[1024];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
        fwrite(buffer, 1, bytes_read, destination_file);
    }

    fclose(source_file);
    fclose(destination_file);

    } else {
        DIR* source_dir = opendir(source_path);

    if (source_dir == NULL) {
        printf("Failed to open the source directory.\n");
        return;
    }

    if (mkdir(destination_path, 0755) == -1) {
        printf("Failed to create the destination directory.\n");
        closedir(source_dir);
        return;
    }

    struct dirent* dir_entry;

    while ((dir_entry = readdir(source_dir)) != NULL) {
        if (strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0) {
            char source_file_path[1024];
            char destination_file_path[1024];

            sprintf(source_file_path, "%s/%s", source_path, dir_entry->d_name);
            sprintf(destination_file_path, "%s/%s", destination_path, dir_entry->d_name);

            copy_file(source_file_path, destination_file_path);
        }
    }

    closedir(source_dir);
    }

}

int main()
{
  char buffer[MAX_COMMAND_LENGTH];

  char *command;

  char *args[MAX_NUM_ARGS + 2];
  pid_t child_pid;
  args[MAX_NUM_ARGS + 1] = NULL;

  ignoreSignal(SIGINT);
  ignoreSignal(SIGTERM);

  while (1)
  {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s> ", cwd);
    readCommand(buffer, MAX_COMMAND_LENGTH);
    parseCommand(buffer, &command, args);
    if (command == NULL || strcmp(command, "") == 0)
    {
      continue;
    }
    if (strcmp(command, "exit") == 0)
    {
      quit(0);
    }
    else if (strcmp(command, "cd") == 0)
    {
      if(args[0]==NULL){
        goHome();
        printf("\n");
      }else if(strcmp(args[0],"..")==0){
        cd();
      }else{
        cdArg(args[0]);
      }
    }else if(strcmp(command,"cat")==0){
      if(args[0]==NULL){
        perror("cat: falta operando");
        printf("\n");
      }else{
        cat(args[0]);
      }
    }
    else if(strcmp(command,"mkdir")==0){
      if(args[0]==NULL){
        perror("mkdir: falta operando");
        printf("\n");
    }else{
      Mkdir(args[0]);
    }
    }
    else if (strcmp(command, "pwd") == 0)
    {
      pwd();
    }
    else if (strcmp(command, "rm") == 0)
    {
      if (args[0] == NULL)
      {
        perror("rm: falta operando");
        printf("\n");
      }
      else
      {
        removerArquivo(args[0]);
      }
    }
    else if (strcmp(command, "ls") == 0)
    {
      if (args[0] == NULL)
      {
        ListarConteudo(cwd);
      }
      else if (strcmp(args[0], "-l") == 0)
      {
        ListarConteudoDetalhado(cwd);
      }
      else if (strcmp(args[0], "-a") == 0)
      {
        ListarConteudo(cwd);
      }else if(strcmp(args[0],"|") == 0 && strcmp(args[1],"sort") == 0){
        ordenarArquivos(cwd);
      }
      else
      {
        printf("Não foi possivel acessar %s: arquivo ou diretorio não encontrado\n", args[0]);
      }
    }else if(strcmp(command,"cp") == 0){
        if(args[0]==NULL || args[1]==NULL){
          perror("cp: falta operando");
          printf("\n");
        }else{
          CP(args[0],args[1]);
        }
      }
    else
    {
      printf("Comando %s não encontrado\n", command);
    }
  }

  exit(1);
}