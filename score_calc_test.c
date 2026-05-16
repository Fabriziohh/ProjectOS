#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

typedef struct report {
    int report_ID;
    char inspector_name[30];
    float latitude;
    float longitude;
    char issue[30];
    int severity;
    time_t timestamp;
    char description[100];
} report;

void print_report(report *r){
    printf("ID: %d\n", r->report_ID);
    printf("Inspector: %s\n", r->inspector_name);
    printf("Latitude: %f\n", r->latitude);
    printf("Longitude: %f\n", r->longitude);
    printf("Issue: %s\n", r->issue);
    printf("Severity: %d\n", r->severity);
    printf("Timestamp: %ld\n", r->timestamp);
    printf("Description: %s\n", r->description);
    printf("\n");
}

void process_directory(char *filename){
    chdir(filename);
    FILE *file = fopen("reports.dat", "rb");
    report r;
    while(fread(&r, sizeof(r), 1, file) == 1){
        print_report(&r);
    }
    fclose(file);
    chdir("..");
}

void process_directory_list(int arrSize, char *argv[]){
    int fd[2];
    for(int i = 1; i < arrSize; i++){
        pipe(fd);
        pid_t child = fork();
        if(child == 0){
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            char district_name[30];
            strncpy(district_name, argv[i], sizeof(district_name) - 1);
            district_name[sizeof(district_name) - 1] = '\0';
            process_directory(district_name);
            exit(0);
        }
        close(fd[1]);
        char buf[1024];
        int n;
        while((n = read(fd[0], buf, sizeof(buf))) > 0){
            write(STDOUT_FILENO, buf, n);
        }
        close(fd[0]);
        wait(NULL);
    }
}

void start_monitor(){
    pid_t hub_mon = fork();
    if(hub_mon != 0) return;
    int fd[2];
    pipe(fd);
    pid_t mon = fork();
    if(mon == 0){
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execl("./monitor", "monitor", NULL);
        exit(1);
    }
    close(fd[1]);
    char buf[256];
    int n;
    while((n = read(fd[0], buf, sizeof(buf)-1)) > 0){
      buf[n] = '\0';
      if(strcmp(buf, "ERROR") == 0 || strcmp(buf, "EXIT") == 0)
          printf("[hub_mon] Monitor ended: %s", buf);
      else
          printf("[hub_mon] %s", buf);
      fflush(stdout);
    }
    close(fd[0]);
    wait(NULL);
    exit(0);
}

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Usage: %s <command> [args...]\n", argv[0]);
        return 1;
    }
    if(strcmp(argv[1], "--calculate_scores") == 0){
        process_directory_list(argc, argv + 1);
    } else if(strcmp(argv[1], "--start_monitor") == 0){
        start_monitor();
    } else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }
    return 0;
}