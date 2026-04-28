#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>

void delete_district(char *district_name){
    printf("Exit code: %i\n", chdir(district_name));
    chdir("..");
    int pid=fork();
    if(pid==0){rmdir(district_name);exit(0);} // child
    if(pid>0){ 
        wait(NULL);// parent
        printf("File deleted\n");
    }else{
        printf("Error forking!\n");
        exit(EXIT_FAILURE);
    }
    return;
}

int main(int argc, char *argv[]){
    if(argc!=2){
        return 1;
    }
    delete_district(argv[1]);
    return 0;
}

/*
char linkname[150];
        snprintf(linkname, sizeof(linkname), "active_reports-%s", district_name);
        unlink(district_name);s */