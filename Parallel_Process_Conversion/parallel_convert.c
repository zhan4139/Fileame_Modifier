/* Intextion
CSci 4061 Spring 2017 Assignment 2
Name1=Jingtao Yu
Name2=Yi Ren
StudentID1=5105881
StudentID2=5107013
Commentary: This is a converting program using parallel processes, where the
            maximum number of processes will be limited by user's input.

            THe program will first search input directory to find images of
          ".png" ".bmp" and ".gif" type, then convert them to output directory
          with jpg extension (thumbnail image) together with html files.
          For the rest of file type from input directory, name will be recorded
          in "junk.txt" in output directory. Notice that the html files will
          be displayed one by one and all the converting processes is recorded
          in "log.txt" file.

Usage Requirements:
username@Directory/path:$ make
username@Directory/path:$ parallel_convert [convert_count] [output_dir] [input_dir]
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/mman.h>
#include <stdarg.h>

static int *glob_var; // shared_memory for number of png file
static int *glob_var1;// for number of bmp file
static int *glob_var2;// for number of gif file
static int *glob_var3;// for number of junk file
static int *glob_var4;// for current running processes
static FILE *logPtr; // for log.txt
static FILE *junkPtr; // for junk.txt

//Function Declaration
void writeTo(char content[]);
void writeToFile(char* text, ...);
void writeHTML(const char *out_dir);

int main(int argc, char *argv[]){
  int convert_count;
  char* output=argv[2];
  char* input=argv[3];
  char in[100]; // for input directory name
  char out[100]; //for output directory name
  char logging[200];//for creating logging txt file
  char junklogging[200];//for junk txt file
  //Copy input and output directory for further use
  strcpy(in, input);
  strcpy(out, output);
  //---------------------------Error Handling-----------------------------------
  //Check proper number of arguments
  if(argc != 4){
    printf("Wrong arguments number! Usage: parallel_convert convert_count output_dir input_dir\n");
    exit(-1);
  }
  //check convert_count format
  if (sscanf (argv[1], "%i", &convert_count)!=1){
    printf("error - not an integer");
  }
  //check out_dir
  if (mkdir(output, 0777) == -1){
    perror("Error");
  }
  //check convert_count
  if(convert_count <= 0){
    printf("Desired convert process must be greater than 0!\n");
    exit(-1);
  }
  //-------------------------------------------------------------------
  //creat shared_memory for files to be converted
  glob_var = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  glob_var1 = mmap(NULL, sizeof *glob_var1, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  glob_var2 = mmap(NULL, sizeof *glob_var2, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  glob_var3 = mmap(NULL, sizeof *glob_var3, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  glob_var4 = mmap(NULL, sizeof *glob_var4, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  *glob_var = 0;
  *glob_var1 = 0;
  *glob_var2 = 0;
  *glob_var3 = 0;
  *glob_var4 = 0;

//printf("---------------------------------------------------------------------\n");
//Open input directory
  int len;
	struct dirent *pDirent;
	DIR *pDir;

	pDir = opendir (input);
  //check if input_dir can be opened
	if (pDir == NULL) {
	  printf ("Cannot open directory '%s'\n", input);
	  return 1;
	}
//printf("---------------------------------------------------------------------\n");
  // Create a string array of filename
  char **fn = (char**)malloc(102*sizeof(char*));
  int i = 0;
  //allocate space for each string
  // here allocate 64 bytes, which is more than enough for the strings
  for(i = 0; i < 102; i++){
    fn[i] = (char*)malloc(64*sizeof(char));
  }
  //save the filenae in a string array
	int filecount=0;
	while ((pDirent = readdir(pDir)) != NULL) {
		sprintf(fn[filecount], pDirent->d_name);
		filecount++;
	}
  //printf("---------------------------------------------------------------------\n");
  //Get 3 different image and junk file, put'em in 4 string arrays
  char* png_array[100];
  char* bmp_array[100];
  char* gif_array[100];
  char* junk_array[100];

  for(i = 0; i < filecount; i++){
    if(strlen(fn[i])>=4){
      int slen = strlen(fn[i]);
      const char *last_four = &fn[i][slen-4];
	    if(strcmp(last_four, ".png") == 0){
        png_array[*glob_var] = fn[i];
        *glob_var = *glob_var + 1;
      }
      else if(strcmp(last_four, ".bmp") == 0){
        bmp_array[*glob_var1] = fn[i];
        *glob_var1 = *glob_var1 + 1;
      }
      else if(strcmp(last_four, ".gif") == 0){
        gif_array[*glob_var2] = fn[i];
        *glob_var2 = *glob_var2 + 1;
      }
      else{
        junk_array[*glob_var3] = fn[i];
        *glob_var3 = *glob_var3 + 1;
      }
    }
  }

//printf("---------------------------------------------------------------------\n");
//Deal with filename, preparing for converting
strcat(in, "/");
strcat(out, "/");
strcpy(logging, out);
strcat(logging, "log.txt");
strcpy(junklogging, out);
strcat(junklogging, "junk.txt");
//printf("------------------------Convering Processes---------------------------\n");
//open log.txt for processes recording
logPtr = fopen(logging, "a");//Append string to log.txt
if (logPtr == NULL)
{
    printf("Error opening file!\n");
    exit(1);
}
writeToFile("Start convering files......\n");
pid_t pid;
//if there is file left, create processes
while(*glob_var + *glob_var1 + *glob_var2 + *glob_var3 > 0){
  if(*glob_var4 < convert_count){
    pid = fork();
    //Fail to create processes;
    if(pid == -1){
      printf("Fail to creat child process!");
      exit(-1);
    }
    //If in child process
    else if (pid == 0){
      //Deal with gif file
      if(getpid()%2==0 && getpid()%3==0){
        if(*glob_var2 > 0){
          *glob_var2 = *glob_var2 - 1;
          char basename2[64];
          char thumbnail2[64];
          strncpy(basename2, gif_array[*glob_var2], strlen(gif_array[*glob_var2])-4);
          basename2[strlen(gif_array[*glob_var2])-4] = 0;
          sprintf(thumbnail2,"%s_%d.jpg", basename2, getpid());
          strcat(out, thumbnail2);
          strcat(in, gif_array[*glob_var2]);
          //Writing to log.txt and print message
          writeToFile("%s converted to %s with child id: %d and parent id :%d\n", in, out, getpid(), getppid());
          execlp("convert","convert", "-thumbnail" ,"200x200", in, out, (char*)NULL);
        }
        else{
          printf("No gif file left!\n");
          exit(0);
        }
      }

      //deal with png files
      else if(getpid()%2==0) {
        if(*glob_var > 0){
          *glob_var = *glob_var - 1;
          char basename[64];
          char thumbnail[64];

          strncpy(basename, png_array[*glob_var], strlen(png_array[*glob_var])-4);
          basename[strlen(png_array[*glob_var])-4] = 0;
          sprintf(thumbnail, "%s_%d.jpg",basename, getpid());
          strcat(out, thumbnail);
          strcat(in, png_array[*glob_var]);
          //Writing to log.txt and print message
          writeToFile("%s converted to %s with child id: %d and parent id :%d\n", in, out, getpid(), getppid());
          execlp("convert","convert", "-thumbnail" ,"200x200", in, out, (char*)NULL);
        }
        else{
          printf("No gif file left!\n");
          exit(0);
        }
      }

      //deal with bmp files
      else if(getpid()%3==0) {
        if(*glob_var1 > 0){
          *glob_var1 = *glob_var1 - 1;
          char basename1[64];
          char thumbnail1[64];
          strncpy(basename1, bmp_array[*glob_var1], strlen(bmp_array[*glob_var1])-4);
          basename1[strlen(bmp_array[*glob_var1])-4] = 0;
          sprintf(thumbnail1, "%s_%d.jpg",basename1, getpid());
          strcat(out, thumbnail1);
          strcat(in, bmp_array[*glob_var1]);
          //Writing to log.txt and print message
          writeToFile("%s converted to %s with child id: %d and parent id :%d\n", in, out, getpid(), getppid());
          execlp("convert","convert", "-thumbnail" ,"200x200", in, out, (char*)NULL);

        }
        else{
          printf("No gif file left!\n");
          exit(0);
        }
      }

      //deal with junk files
      else {
        if(*glob_var3 > 0){
          *glob_var3 = *glob_var3 - 1;
          //Writing to junk.txt
          junkPtr = fopen(junklogging, "a");//Append string to junk.txt
          if (logPtr == NULL)
          {
              printf("Error opening file!\n");
              exit(1);
          }
          char junkfile[100];
          sprintf(junkfile,junk_array[*glob_var3]);
          strcat(junkfile,"\n");
          fputs(junkfile, junkPtr);
          fclose(junkPtr);
        }
        else{
          printf("No gif file left!\n");
        }
        exit(0);
      }
    }
    else {//parent process;
      *glob_var4 = *glob_var4 + 1;
      if(*glob_var4 == convert_count){
        int stat;
        if(wait(&stat))
          *glob_var4 = *glob_var4 - 1;
      }
    }
    wait(NULL);
  }
}
  printf("Waited for all child process and exit with parent id: %d\n", getpid());
  fprintf(stdout, "Starting Generating HTML Files......\n");
  writeHTML(output);
  fprintf(stdout, "All Done!!\n");
  //Release shared_memory Space
  munmap(glob_var4, sizeof *glob_var4);
	munmap(glob_var, sizeof *glob_var);
	munmap(glob_var1, sizeof *glob_var1);
	munmap(glob_var2, sizeof *glob_var2);
  munmap(glob_var3, sizeof *glob_var3);
  fclose(logPtr);
  return 0;
}

void writeHTML(const char *out_dir){
  char *file_array[100];
  int File_Counter = 0;
  DIR *outdir = opendir(out_dir);
  struct dirent *dir;
  char imageNOExt[100];
  while((dir = readdir(outdir)) != NULL){
    //const char *last_four = &fn[i][slen-4];
    const char *extension = &(dir->d_name)[strlen(dir->d_name)-3];
    if(strcmp(extension, "jpg") == 0){
      int len = strlen(dir->d_name);
      dir->d_name[len-4] = '\0';
      file_array[File_Counter] = dir->d_name;
      File_Counter++;
    }
  }

  char array[1024];
  getcwd(array, sizeof(array));
  int i;
  for(i = 0; i < File_Counter; i++){
    // Make file
    char outBuf[100];
    char numCountString[10];

    strcpy(outBuf, out_dir);
    strcat(outBuf,"/");
    sprintf(numCountString, "%d", i);
    strcat(outBuf, numCountString);
    strcat(outBuf, ".html");

    int nextImage;
    if(i == (File_Counter - 1)){
      nextImage = 0;
    }else{
      nextImage = i + 1;
    }

    FILE *file = fopen(outBuf, "w");
    fprintf(file, "<html><head><meta http-equiv=\"refresh\" content='1;URL=file:///%s/%s/%d.html'></head><body><img src='%s.jpg'></body></html>", array, out_dir, nextImage, file_array[i]);
    fclose(file);

  }
}

void writeToFile(char* text, ...)
{
	va_list list;
	va_start(list, text);
	char content[200];
	vsprintf(content, text, list);
	va_end(list);
	writeTo(content);
}

void writeTo(char content[])
{
	fprintf(logPtr, content);
	printf(content);
	fflush(logPtr);
}
