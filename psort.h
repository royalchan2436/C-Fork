#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>

#define SIZE 44

struct rec {
    int freq;
    char word[SIZE];
};

int get_file_size(char *filename);
int compare_freq(const void *rec1, const void *rec2);
void change_position(struct rec *head, int len);
int check_null(struct rec *head, int len);
int merge_sort(struct rec *head, int len);

