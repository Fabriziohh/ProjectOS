#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>

void user_handle_print(int sig){
    printf("User signal sent!\n");
}

void exit_handle_print(int sig){
    printf("Exit monitoring!\n");
}

void process_monitor_file(){
    int pid=fork();
    if(pid==0){exit(0);}
    if(pid>0){
        wait(NULL);
        printf("%d",pid);
        FILE *f = fopen(".monitor_pid","w");
        fprintf(f,"%d",pid);
        fclose(f);
        struct sigaction sa;
        sa.sa_handler = &user_handle_print;
        struct sigaction ex;
        ex.sa_handler = &exit_handle_print;
        while(1){
            sigaction(SIGUSR1,&sa,NULL);
            sigaction(SIGINT,&ex,NULL);
        }
    }else{
        printf("Error");
        exit(EXIT_FAILURE);
    }
}

int main(){
    process_monitor_file();
    return 0;
}