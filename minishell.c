
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
unsigned long getTimestamp()
{
  /* Holds the current time from the gettimeofday call. */
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == -1)
  {
    /* If we are unable to get the time of day we return 0, as this
       should not be a fatal error. */
    fprintf(stderr, "Unable to get current time stamp.\n");
    return 0;
  }
  return 1000000 * tv.tv_sec + tv.tv_usec;
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
    fprintf(stderr, "Unable to go to HOME directory. Possibly not set.\n");
  }
}

// void runCommand(char *command, char *args[])
// {

//   unsigned long startTime;

//   pid_t pid = fork();
//   if (pid == -1)
//   {
//     fprintf(stderr, "Unable to fork.\n");
//     quit(1);
//   }

//   if (pid == 0)
//   {
//     resetSignal(SIGINT);
//     resetSignal(SIGTERM);
//     execvp(command, args);
//     fprintf(stderr, "Unable to start command: %s\n", command);
//     quit(1);
//   }
//   else if (pid > 0)
//   {
//     startTime = getTimestamp();
//     printf("Started process with pid %d\n", pid);
//     while (waitpid(pid, NULL, 0) == -1)
//       ;
//     printf("Foreground process %d ended\n", pid);
//     printf("Wallclock time: %.3f\n", (getTimestamp() - startTime) / 1000.0);
//   }
// }

void cd(char *directory)
{
  if (directory == NULL)
  {
    goHome();
    return;
  }

  if (chdir(directory) == -1)
  {
    fprintf(stderr, "diretorio não encontrado %s\n", directory);
    goHome();
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

void ListarConteudoDetalhado(char *directory)
{
  struct dirent *de;
  struct stat fileStat;

  DIR *dr = opendir(directory);
  if (dr == NULL)
  {
    perror("Não foi possível abrir o diretório");
    return;
  }
  while ((de = readdir(dr)) != NULL)
  {

    char filePath[1024];
    snprintf(filePath, sizeof(filePath), "%s/%s", directory, de->d_name);
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
      cd(args[1]);
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
      }
      else
      {
        printf("Não foi possivel acessar %s: arquivo ou diretorio não encontrado\n", args[0]);
      }
    }
    else
    {
      printf("Comando %s não encontrado\n", command);
    }
  }

  exit(1);
}