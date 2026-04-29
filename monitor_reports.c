#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void user_handle_print(int sig){
    FILE *f = fopen("monitor.log", "a");
    if(f){
        fprintf(f, "Monitor notified: a new report has been added.\n");
        fclose(f);
    }
}

void exit_handle_print(int sig){
    exit(0);
}

int main(){
    FILE *f = fopen(".monitor_pid", "w");
    fprintf(f, "%d", getpid());
    fclose(f);

    struct sigaction sa, ex;
    memset(&sa, 0, sizeof(sa));
    memset(&ex, 0, sizeof(ex));
    sa.sa_handler = user_handle_print;
    ex.sa_handler = exit_handle_print;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGINT,  &ex, NULL);

    while(1){ pause(); }
    return 0;
}