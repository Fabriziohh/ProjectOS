#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

struct stat st;

void create_district(char *district_name){
    mkdir(district_name, 0750);
    chdir(district_name);
    
    FILE *fptr;
    fptr = fopen("district.cfg", "w");
    if (fptr) fclose(fptr);
    
    fptr = fopen("reports.dat", "wb");
    if (fptr) fclose(fptr);
    
    chdir("..");
    
    char target[150];
    char linkname[150];
    snprintf(target, sizeof(target), "%s/reports.dat", district_name);
    snprintf(linkname, sizeof(linkname), "active_reports-%s", district_name);
    symlink(target, linkname);
}

int decimal_to_octal(int decimal) {
    int arrOctal[3];
    int noEl = 0;
    while (decimal != 0) {
        arrOctal[noEl] = decimal % 8;
        noEl++;
        decimal = decimal / 8;
    }
    int octal = 0;
    for (int i = 0; i < noEl; i++) {
        octal = octal * 10 + arrOctal[noEl - 1 - i]; 
    }
    return octal;
}

int get_permission_code(char *filename){
    stat(filename,&st);
    return decimal_to_octal(st.st_mode&0777);
}

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

    if (lstat(filename, &st) != 0) {
        printf("Warning: dangling symlink or missing file: %s\n", filename);
        return 0;
    }

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

    if (lstat(filename, &st) != 0) {
        printf("Warning: dangling symlink or missing file: %s\n", filename);
        return false;
    }

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

    if (lstat(filename, &st) != 0) {
        printf("Warning: dangling symlink or missing file: %s\n", filename);
        return;
    }

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
        fseek(f, i * sizeof(report), SEEK_SET);
        fread(&aux, sizeof(report), 1, f);
        fseek(f, (i - 1) * sizeof(report), SEEK_SET);
        fwrite(&aux, sizeof(report), 1, f);
    }

    ftruncate(fileno(f), (total - 1) * sizeof(report));
    fclose(f);
    return true;
}

bool update_threshold(int new_threshold, char *directory) {
    char filename[150];
    snprintf(filename, sizeof(filename), "%s/district.cfg", directory);

    if (lstat(filename, &st) != 0) {
        printf("Warning: dangling symlink or missing file: %s\n", filename);
        return false;
    }

    if (get_permission_code(filename) != 640) {
        printf("Error: district.cfg permissions have been changed, expected 640 got %d\n", get_permission_code(filename));
        return false;
    }

    FILE *f = fopen(filename, "w");
    if (f == NULL) { return false; }
    fprintf(f, "%d\n", new_threshold);
    fclose(f);
    return true;
}

int parse_condition(const char *input, char *field, char *op, char *value) {
    const char *first = strchr(input, ':');
    if (first == NULL) return 0;

    strncpy(field, input, first - input);
    field[first - input] = '\0';

    const char *second = strchr(first + 1, ':');
    if (second == NULL) return 0;

    strncpy(op, first + 1, second - first - 1);
    op[second - first - 1] = '\0';

    strncpy(value, second + 1, 29);
    value[29] = '\0';

    return 1;
}

int match_condition(report *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int threshold = atoi(value);
        int v = r->severity;
        if (strcmp(op, "==") == 0) return v == threshold;
        if (strcmp(op, "!=") == 0) return v != threshold;
        if (strcmp(op, "<")  == 0) return v <  threshold;
        if (strcmp(op, "<=") == 0) return v <= threshold;
        if (strcmp(op, ">")  == 0) return v >  threshold;
        if (strcmp(op, ">=") == 0) return v >= threshold;

    } else if (strcmp(field, "category") == 0) {
        int eq = strcmp(r->issue, value) == 0;
        if (strcmp(op, "==") == 0) return eq;
        if (strcmp(op, "!=") == 0) return !eq;

    } else if (strcmp(field, "inspector") == 0) {
        int eq = strcmp(r->inspector_name, value) == 0;
        if (strcmp(op, "==") == 0) return eq;
        if (strcmp(op, "!=") == 0) return !eq;

    } else if (strcmp(field, "timestamp") == 0) {
        time_t threshold = (time_t)atol(value);
        time_t v = r->timestamp;
        if (strcmp(op, "==") == 0) return v == threshold;
        if (strcmp(op, "!=") == 0) return v != threshold;
        if (strcmp(op, "<")  == 0) return v <  threshold;
        if (strcmp(op, "<=") == 0) return v <= threshold;
        if (strcmp(op, ">")  == 0) return v >  threshold;
        if (strcmp(op, ">=") == 0) return v >= threshold;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    char *role = NULL, *user = NULL, *command = NULL;
    char *positional_args[8];
    int  pos_count = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            role = argv[++i];
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            user = argv[++i];
        } else if (command == NULL) {
            command = argv[i];
        } else {
            if (pos_count < 8) {
                positional_args[pos_count++] = argv[i];
            }
        }
    }

    if (role == NULL || command == NULL) {
        printf("Usage: city_manager --role <role> --user <user> <command> [args...]\n");
        return 1;
    }

    if (strcmp(command, "--add") == 0) {
        if (pos_count < 6) {
            printf("Usage: --add <district> <lat> <lon> <category> <severity> <description>\n");
            return 1;
        }
        char *district    = positional_args[0];
        float lat         = atof(positional_args[1]);
        float lon         = atof(positional_args[2]);
        char *category    = positional_args[3];
        int   severity    = atoi(positional_args[4]);
        char *description = positional_args[5];

        int ID = find_no_records(district);
        report *r = create_record(ID, user, lat, lon, category, severity, time(NULL), description);
        if (r == NULL) return 1;
        add_report(district, r);
        free(r);

    } else if (strcmp(command, "--list") == 0) {
        if (pos_count < 1) { printf("Missing district\n"); return 1; }
        list_report(positional_args[0]);

    } else if (strcmp(command, "--view") == 0) {
        if (pos_count < 2) { printf("Missing district or ID\n"); return 1; }
        view_report(positional_args[0], atoi(positional_args[1]));

    } else if (strcmp(command, "--remove_report") == 0) {
        if (strcmp(role, "manager") != 0) {
            printf("Error: only managers can remove reports\n");
            return 1;
        }
        if (pos_count < 2) { printf("Missing district or ID\n"); return 1; }
        delete_record(atoi(positional_args[1]), positional_args[0]);

    } else if (strcmp(command, "--update_threshold") == 0) {
        if (strcmp(role, "manager") != 0) {
            printf("Error: only managers can update threshold\n");
            return 1;
        }
        if (pos_count < 2) { printf("Missing district or value\n"); return 1; }
        update_threshold(atoi(positional_args[1]), positional_args[0]);

    } else if (strcmp(command, "--filter") == 0) {
        if (pos_count < 2) { printf("Missing district or condition\n"); return 1; }
        char *district = positional_args[0];

        char filename[150];
        snprintf(filename, sizeof(filename), "%s/reports.dat", district);
        FILE *f = fopen(filename, "rb");
        if (f == NULL) { printf("Could not open reports.dat\n"); return 1; }

        char field[30], op[4], value[30];
        report aux;
        while (fread(&aux, sizeof(report), 1, f) == 1) {
            int match = 1;
            for (int i = 1; i < pos_count; i++) {
                if (!parse_condition(positional_args[i], field, op, value) ||
                    !match_condition(&aux, field, op, value)) {
                    match = 0;
                    break;
                }
            }
            if (match) {
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
        }
        fclose(f);

    } else if (strcmp(command, "--create_district") == 0) {
        if (strcmp(role, "manager") != 0) {
            printf("Error: only managers can create districts\n");
            return 1;
        }
        if (pos_count < 1) { printf("Missing district name\n"); return 1; }
        create_district(positional_args[0]);

    } else {
        printf("Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}