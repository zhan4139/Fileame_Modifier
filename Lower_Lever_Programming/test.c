/* 
  test.c
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mini_filesystem.h"
#include <sys/dir.h>

/* Test Helper Interface */
char jpgFilename[MAXFILES][BLOCKSIZE];
char fn[MAXFILES][BLOCKSIZE];
char fname[MAXFILES][BLOCKSIZE];
int jpgNum = 0;
int filecount = 0;
int imageNum = 0;
char *get_filename(char *filename){
  char *dot = strrchr(filename, '/');
  if(!dot || dot == filename) return filename;
  return dot + 1;
}

// Taken from stackoverflow.com/questions/5309471/getting-file-extension-in-c
char *get_filename_ext(char *filename){
  char *dot = strrchr(filename, '.');
  if(!dot || dot == filename) return "";
  return dot + 1;
}

// Borrowed from http://stackoverflow.com/questions/2736753/how-to-remove-extension-from-file-name
char *removeExt(char* mystr) {
    char *retstr;
    char *lastdot;
    if (mystr == NULL)
         return NULL;
    if ((retstr = malloc (strlen (mystr) + 1)) == NULL)
        return NULL;
    strcpy (retstr, mystr);
    lastdot = strrchr (retstr, '.');
    if (lastdot != NULL)
        *lastdot = '\0';
    return retstr;
}
//This is the function that report the summary of mini_filesystem
void make_filesystem_summary(char* filename){
  FILE* summaryFile;
  int i;
  int index = 0;
  int inode_No;
  int j;
  for(j = 0; j < filecount;j++){
    struct stat s;
    stat(fn[j], &s);
    int file_size;
    char* filename2;
    char* extension;
    char* fname2;
    int f = 0;
    file_size = s.st_size;
    filename2 = get_filename(fn[j]);
    extension = get_filename_ext(filename2);
    fname2 = removeExt(filename2);
    if(strlen(filename2)>0){
      f = 1;
      inode_No  = Open_File(fname[j]);
    }
    if(f){
      summaryFile = fopen(filename, "a");
      fprintf(summaryFile, "%20s\t %6s\t %6d\t %20d\n", fname2, extension, inode_No, file_size);
      fclose(summaryFile);
    }
  }
    //fclose(summaryFile);
}
void generate_html_file(char* filename, char* output_directory){
  FILE* Html;
  char f[200];
  sprintf(f, filename);
  char* filename_noPath = get_filename(f);
  char basename1[64];
  char thumbnail1[64];
  char out[200];
  char out2[200];
  char out3[200];
  sprintf(out, output_directory);
  sprintf(out2, ".");
  strcat(out2, "/");
  strcat(out2, filename_noPath);
  sprintf(out3, ".");
  strcat(out3, "/");
  strncpy(basename1, filename_noPath, strlen(filename_noPath)-4);
  basename1[strlen(filename_noPath)-4] = 0;
  sprintf(thumbnail1, "%s_%s.jpg",basename1, "thumb");
  strcat(out, "/");
  char html[200];
  sprintf(html, out);
  strcat(html, "Image.html");
  strcat(out, thumbnail1);
  strcat(out3, thumbnail1);
  Html = fopen(html, "a");
  
  if(imageNum == 0){
    fprintf(Html, "    <a href=\"%s\">\n    <img src=\"%s\"/></a>\n", out2, out3);
    fprintf(Html, "  </body>\n</html>");
    imageNum++;
  }
  else{
    if(imageNum == jpgNum - 1){
      fprintf(Html, "<html>\n  <head>\n    <title>File System Contents</title>\n  </head>\n  <body>\n");
    }
    fprintf(Html, "    <a href=\"%s\">\n    <img src=\"%s\"/></a>\n", out2, out3);
    imageNum++;
  }
  pid_t child = fork();
  if(child == 0){
    execlp("convert","convert", "-thumbnail" ,"200x200", f, out, (char*)NULL);
    //Check system call
    perror("Fail to convert!\n");
  }
  wait();
}

void write_into_filesystem(char* input_directory, char *log_filename){
  int len;
  struct dirent *pDirent;
  struct dirent *pDirent2;
  DIR *pDir;
  DIR *pDir2;
  pDir = opendir (input_directory);
  //check if input_dir can be opened
  if (pDir == NULL) {
    printf ("Cannot open directory '%s'\n", input_directory);
    exit(0);
  }

  //if exist, remove log file, then Initialize filesystem.
  char* lFile = log_filename;
  remove(log_filename);
  Initialize_Filesystem(lFile);

  int i = 0;
  char directory_name[128][200];
  //save the filename and directory_name in string arrays
  int dirCount = 0;
  while ((pDirent = readdir(pDir)) != NULL) {
    if(strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0 && pDirent->d_type == DT_DIR){
      strcat(directory_name[dirCount], input_directory);
      strcat(directory_name[dirCount], "/");
      strcat(directory_name[dirCount], pDirent->d_name);
      dirCount++;
    }
    else if(strcmp(pDirent->d_name, ".") != 0  && strcmp(pDirent->d_name, "..") != 0){
      if(strcmp(get_filename_ext(pDirent->d_name), "jpg") == 0){
        sprintf(jpgFilename[jpgNum++], pDirent->d_name);
      }
      strcat(fn[filecount], input_directory);
      strcat(fn[filecount], "/");
      strcat(fn[filecount], pDirent->d_name);
      strcat(fname[filecount],pDirent->d_name);
      filecount++;
    }
  }

  //go into subarrary
  int dirNumber = 0;
  strcat(directory_name[dirCount], "/");
  while(dirNumber < dirCount){
    pDir2 = opendir(directory_name[dirNumber]);
    while ((pDirent2 = readdir(pDir2)) != NULL) {
      if(strcmp(pDirent2->d_name, ".") != 0 && strcmp(pDirent2->d_name, "..") != 0 && pDirent2->d_type == DT_DIR){
        strcat(directory_name[dirCount], directory_name[dirNumber]);
        strcat(directory_name[dirCount], "/");
        strcat(directory_name[dirCount], pDirent2->d_name);
        dirCount++;
      }
      else if(strcmp(pDirent2->d_name, ".") != 0 && strcmp(pDirent2->d_name, "..") != 0){
        if(strcmp(get_filename_ext(pDirent2->d_name), "jpg") == 0 ){
          sprintf(jpgFilename[jpgNum++], pDirent2->d_name);
        }
        strcat(fn[filecount], directory_name[dirNumber]);
        strcat(fn[filecount], "/");
        strcat(fn[filecount], pDirent2->d_name);
        strcat(fname[filecount],pDirent2->d_name);
        filecount++;
      }
    }
    dirNumber++;
  }

//Add file into mini filesystem
  int k;
  for(k = 0; k < filecount; k++){
    FILE* tempFile = fopen(fn[k], "r");
    if(tempFile == NULL){
			perror("fopen");
		}
    else{
      struct stat s;
    	stat(fn[k], &s);
    	int file_size = s.st_size;
      int iNum = Create_File(fname[k]);
      if(iNum == -1){//same image name continue to next one
        sprintf(fn[k], "");
        jpgNum--;
        //sprintf(fname[k], "");//-------------------------------------------------------
        continue;
      }
      int inode = Open_File(fname[k]);

      char* fileCts = (char*) malloc(sizeof(char)*file_size);
  		int ch;
  		int j = 0;
      while((ch = fgetc(tempFile)) != EOF){
      	fileCts[j] = ch;
      	j++;
      }

      int offset = 0;
      int file_size2 = file_size;

      while(file_size != 0){
        if(file_size > BLOCKSIZE){
          char* tempArray = (char*)malloc(sizeof(char)*BLOCKSIZE);
          int x = 0;
          for(x = 0;x<BLOCKSIZE;x++)
            tempArray[x] = fileCts[x];
          Write_File(iNum, offset, BLOCKSIZE, tempArray);
          fileCts += BLOCKSIZE;
          offset += BLOCKSIZE;
          file_size2 -= BLOCKSIZE;
          file_size -= BLOCKSIZE;
        }
        else{//less than 512bytes
          Write_File(iNum, offset, file_size2, fileCts);
          file_size = 0;
        }
      }
      Close_File(iNum);
    }
    //make_filesystem_summary(fn[k]);
  }
  //int close = fcloseall();
}


void read_images_from_filesystem_and_write_to_output_directory(char* output_directory){
  int n;
  for(n = 0; n < filecount; n++){
    if(strcmp(get_filename_ext(fn[n]), "jpg") == 0){
      int inodeNum = Open_File(fname[n]);
      char* outName = (char*) malloc(200);
      strcpy(outName, output_directory);
      strcat(outName, "/");
      strcat(outName, fname[n]);
      if(inodeNum != -1){
        // offset and count set to 0
        FILE *fp2;
        char* outputString;
        outputString = malloc(BLOCKSIZE*MAXBLOCKS);//max size of system
        int sizeOfFile = Read_File(inodeNum, 0, 0, outputString);
        int counter = 0;
        fp2 = fopen(outName, "w");
        if(fp2 == NULL){
          puts("Not able to open this file\n");
          counter++;
        }
        else{
          //copys from Read_File into the output file path
          while(counter<sizeOfFile){
            fputc(outputString[counter], fp2);
            counter++;
          }
        }
      }
      Close_File(inodeNum);
      printf("%s\n", fn[n]);
      generate_html_file(fn[n], output_directory);
    }
  }
  close_logging();
  int close = fcloseall();
}

/* Main function */
int main(int argc, char* argv[]){
  //Error Handling-----------------------------------
  //Check proper number of arguments
  if(argc != 4){
    printf("Wrong arguments number! Usage: test convert_count output_dir input_dir\n");
    exit(1);
  }
  //check out_dir
  if (mkdir(argv[2], 0777) == -1){
    perror("Error");
  }

  printf("-----------------------------------------------------------\n");
  printf("Writing into filesystem... \n");
  write_into_filesystem(argv[1],argv[3]);
  printf("Finished writing into filesystem!\n\n");

  printf("Reading from filesystem and write into output directory...\n");
  printf("/*The images read from filesystem is:*/\n");
  make_filesystem_summary("summary.txt");
  read_images_from_filesystem_and_write_to_output_directory(argv[2]);
  printf("/*Images read and write to output directory finished.*/\n");
  printf("All Completed.\n");
  printf("-----------------------------------------------------------\n");
  //printf("Super_block is %d\n", Superblock.next_free_block);
  printf("The final count is %d\n", Count);
  return 0;
}
