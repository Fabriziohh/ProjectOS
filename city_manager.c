// I will first implement the functions without caring about the user and their roles and then i will change

/* 
First Step
We have a directory specific for a district
For now we will create and care about a single file -> reports.dat
reports.dat is a binary file that on each line will contain a struct that has the following information

Report ID (integer)
Inspector name (fixed-length string, provided as a --user argument)
GPS coordinates (latitude and longitude as floating-point numbers)
Issue category (fixed-length string, e.g. "road", "lighting", "flooding")
Severity level (integer: 1 = minor, 2 = moderate, 3 = critical)
Timestamp (time_t)
Description text (fixed-length string)

First we create the function that sets the inspector name on null and then adds the other records
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

int find_no_records(char *directory) {
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/reports.dat", directory);
    FILE *f = fopen(filename, "rb");
    int i = 0;
    report aux;
    while (fread(&aux, sizeof(report), 1, f) == 1) {
        i++;
    }
    fclose(f);
    return i;
}

bool view_report(char *directory, int ID) {
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/reports.dat", directory);
    FILE *f = fopen(filename, "rb");
    if (f == NULL) { return false; }
    report aux;
    while (fread(&aux, sizeof(report), 1, f) == 1) {
        if (aux.report_ID == ID) {
            printf("ID: %d\n", aux.report_ID);
            printf("Inspector: %s\n", aux.inspector_name);
            printf("Category: %s\n", aux.issue);
            printf("Severity: %d\n", aux.severity);
            printf("Latitude: %f\n", aux.latitude);
            printf("Longitude: %f\n", aux.longitude);
            printf("Timestamp: %ld\n", aux.timestamp);
            printf("Description: %s\n", aux.description);
            printf("---\n");
            fclose(f);
            return true;
        }
    }
    fclose(f);
    return false;
}  

bool add_report(char *directory, report *data) {
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/reports.dat", directory);

    FILE *f = fopen(filename, "ab");
    if (f == NULL) { return false; }
    if (fwrite(data, sizeof(report), 1, f) != 1) {
        fclose(f);
        return false;
    }
    fclose(f);
    return true;
}

void list_report(char *directory) {
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/reports.dat", directory);
    // We compute the directory/reports.dat in order to open the file at the specific directory/district !!

    FILE *f = fopen(filename, "rb");
    if (f == NULL) { return; }

    report aux;
    while (fread(&aux, sizeof(report), 1, f) == 1) {
        printf("ID: %d\n", aux.report_ID);
        printf("Inspector: %s\n", aux.inspector_name);
        printf("Category: %s\n", aux.issue);
        printf("Severity: %d\n", aux.severity);
        printf("Latitude: %f\n", aux.latitude);
        printf("Longitude: %f\n", aux.longitude);
        printf("Timestamp: %ld\n", aux.timestamp);
        printf("Description: %s\n", aux.description);
        printf("---\n");
    }
    fclose(f);
}

report* create_record(int ID, char IN[], float lat, float lon, char I[], int sev, time_t ts, char desc[]) {
    report *new_report = malloc(sizeof(report));
    if (new_report == NULL) return NULL;

    new_report->report_ID = ID;
    strncpy(new_report->inspector_name, IN, 29);
    new_report->inspector_name[29] = '\0';
    new_report->latitude = lat;
    new_report->longitude = lon;
    strncpy(new_report->issue, I, 29);
    new_report->issue[29] = '\0';
    new_report->severity = sev;
    new_report->timestamp = ts;
    strncpy(new_report->description, desc, 99);
    new_report->description[99] = '\0';

    return new_report;
}

bool delete_record(int ID, char *directory) {
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/reports.dat", directory);

    FILE *f = fopen(filename, "r+b");
    if (f == NULL) { return false; }

    int total = find_no_records(directory);
    rewind(f);

    report aux;
    for (int i = ID; i < total; i++) {
        fseek(f, i * sizeof(report), SEEK_SET);      // go to aux position
        fread(&aux, sizeof(report), 1, f);            // read aux
        fseek(f, (i - 1) * sizeof(report), SEEK_SET); // go to prev position
        fwrite(&aux, sizeof(report), 1, f);           // overwrite prev with aux
    }

    ftruncate(fileno(f), (total - 1) * sizeof(report));
    fclose(f);
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: city_manager --add <district> <inspector> <lat> <lon> <category> <severity> <description>\n");
        printf("       city_manager --list <district>\n");
        return 1;
    }
    if (strcmp(argv[1], "--add") == 0) {
        if (argc < 9) {
            printf("Not enough arguments for --add\n");
            return 1;
        }
        char *district    = argv[2];
        char *inspector   = argv[3];
        float lat         = atof(argv[4]);
        float lon         = atof(argv[5]);
        char *category    = argv[6];
        int severity      = atoi(argv[7]);
        char *description = argv[8];
        int ID = find_no_records(district);
        report *r = create_record(ID, inspector, lat, lon, category, severity, time(NULL), description);
        if (r == NULL) { return 1; }
        add_report(district, r);
        free(r);
    } else if (strcmp(argv[1], "--list") == 0) {
        list_report(argv[2]);
    } else if (strcmp(argv[1], "--view") == 0) {
        if (argc < 4) {
          printf("Not enough arguments for --view\n");
          return 1;
        }
        view_report(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "--delete") == 0) {
        if (argc < 4) {
          printf("Not enough arguments for --delete\n");
          return 1;
        }
        delete_record(atoi(argv[3]), argv[2]);
    } else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }
    return 0;
}