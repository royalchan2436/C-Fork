#include "psort.h"

/* Return File Size By Struct */
int get_file_size(char *filename) {

    struct stat sbuf;

    if((stat(filename, &sbuf)) == -1) {
       perror("stat");
       exit(1);
    }

    return sbuf.st_size / sizeof(struct rec);
}

/* A Comparison Function To Use For Qsort */
int compare_freq(const void *rec1, const void *rec2) {

    struct rec *r1 = (struct rec *)rec1;
    struct rec *r2 = (struct rec *)rec2;

    if(r1->freq == r2->freq) {
        return 0;
    } else if(r1->freq > r2->freq) {
        return 1;
    } else {
        return -1;
    }
}

/* Given A Struct Array And Length
 * Move The 0 Position Element To The End
 * 0 Position Element Word Will Be Set To "NULL"
 * Other Position Element Move Forward One Position
 */
void change_position(struct rec *head, int len){
    struct rec temp;
    temp = head[0];
    strcpy(temp.word, "NULL");
    temp.freq = -1;
    int i;
    for(i = 0; i < len; i++){
        if(i == (len-1)){
            head[i] = temp;
            break;
        }
        head[i] = head[i+1];
    }
}

/* Given A Struct Array With Each 0 Pisition
 * Element From Each Child
 * If All Child Element Been Set NULL Return -1 
 * Otherwise Return 1
 */
int check_null(struct rec *head, int len){
    int i;
    int count = 0;
    for(i = 0; i < len; i++){
        if(strcmp(head[i].word, "NULL") == 0){
            count++;
        }
    }
    if(count == len){
        return -1;
    }
    else{
        return 1;
    }
}

/* Sort 0 Pisition Element From Each Child
 * Return -1 If All Element Already Been Set NULL
 * Return Poisition For Match Child
*/
int merge_sort(struct rec *head, int len){
    
    int check = check_null(head, len);
    int i;
    struct rec temp[len];
    for(i = 0; i < len; i++){
        temp[i] = head[i];
    }
    if(check == -1){
        return -1;
    }
    qsort(head, len, sizeof(struct rec), compare_freq);
    for(i = 0; i < len; i++){
        if(strcmp(head[i].word, "NULL") != 0){
            break;
        }
    }
    int temp_val = head[i].freq;
    char temp_word[44];
    strcpy(temp_word, head[i].word);
    for(i = 0; i < len; i++){
        if(temp_val == temp[i].freq){
            if(strcmp(temp_word, temp[i].word) == 0){
                break;
            }
        }
    }
    return i;
}

int main(int argc, char **argv){
    
    /* Set Timer */
    struct timeval starttime, endtime;
    double timediff;

    if ((gettimeofday(&starttime, NULL)) == -1) {
        perror("gettimeofday");
        exit(1);
    }

    /* Variables used to store arguments */
    extern char *optarg;
    int ch, num_process;
    char *infile = NULL, *outfile = NULL;

    if(argc != 7){
        fprintf(stderr, "Usage:  psort -n <N processces> "
                        "-f <input file name> -o <output file name>\n");
        exit(1);
    }
    /* read in arguments */
    while ((ch = getopt(argc, argv, "n:f:o:")) != -1) {
        switch(ch) {
        case 'n':
            num_process = atoi(optarg);
            break;
        case 'f':
            infile = optarg;
            break;
        case 'o':
            outfile = optarg;
            break;
        default : 
            fprintf(stderr, "Usage:  psort -n <N processces> "
                          "-f <input file name> -o <output file name>\n");
            exit(1);
        }
    }

    /* If Input Process Less Than One */
    if(num_process < 1){
        return 0;
    }
    
    /* Set variables */
    int i, result, fd[num_process+1][2];
    int file_size = get_file_size(infile);

    /* Calculate bytes for each process, last process may have more bytes */
    int each_process_sort = file_size / num_process;
    int last_process_sort = (file_size % num_process) + each_process_sort;
    
    /* Fork */
    for(i = 1; i <= num_process; i++){
        
        if((pipe(fd[i])) == -1){ //Pipe Error Check
            perror("pipe\n");
            exit(1);
        }

        result = fork();

        if(result < 0){ //Fork Error Check
            perror("fork\n");
            exit(1);
        }else if(result == 0){ //Child Process

            /* Close All Read End Pipe To A Sibbling */
            int child_no;
            for (child_no = 1; child_no <= i; child_no++) {
                if (close(fd[child_no][0]) != 0 ) {
                    perror("closing a pipe to a sibbling");
                    exit(1);
                }
            }
            /* Open Input File */
            FILE *fp;
            if((fp = fopen(infile, "r")) == NULL) {
                perror("fopen");
                exit(1);
            }
            
            /* Set Variables */
            struct rec r;
            int start_point = (i - 1) * each_process_sort;
            int end_point = 0;
            int mode = 0;
            
            /* Switch Mode */
            if(i == num_process){
                mode = last_process_sort;
            }

            if(i != num_process){
                mode = each_process_sort;
            }

            /* Go to target position */
            fseek(fp, start_point*sizeof(struct rec), SEEK_SET);

            /* Read From File And Store In Struct Array */
            struct rec array_last[mode];
            while((end_point != mode) && (fread(&r, sizeof(struct rec), 1, fp) == 1)) {
                array_last[end_point].freq = r.freq;
                strcpy(array_last[end_point].word, r.word);
                end_point++;
            }

            /* Close File */
            fclose(fp);
            
            /* Qsort Struct Array */
            qsort(array_last, mode, sizeof(struct rec), compare_freq);

            /* Write Sort Struct Array To Parent */
            int loop_c;
            for(loop_c = 0; loop_c < mode; loop_c++){
                if(write(fd[i][1], &array_last[loop_c], sizeof(struct rec)) != sizeof(struct rec)){
                    perror("Write Fail\n");
                    exit(1);
                }
            }

            /* Close Pipe Write End And Exit Child Process */
            if(close(fd[i][1]) != 0){
                perror("Close Pipe Fail\n");
                exit(1);
            }
            exit(0);

            
        }else{   
            /* Parent Process             */
            /* Close Every Write End Pipe */
            if(close(fd[i][1]) != 0){
                perror("Close Pipe Fail\n");
                exit(1);
            }
        }
    }
    /* Only Parent Get Here */
    /* Set Variables        */
    struct rec temp;
    struct rec answer_1[num_process][each_process_sort];
    struct rec answer_2[last_process_sort];
    int control, size_control;

    /* Open Output File */
    FILE *outfp;
    if((outfp = fopen(outfile, "w")) == NULL) {
        perror("fopen");
        exit(1);
    }

    /* If Only One Child Process */
    if(num_process == 1){

        /* Read From Only Child And Write To Output File immediately */
        for(size_control = 0; size_control < last_process_sort; size_control++){
            if(read(fd[1][0], &temp, sizeof(struct rec)) != sizeof(struct rec)){
                perror("Read Fail\n");
                exit(1);
            }
            if(fwrite(&temp, sizeof(struct rec), 1, outfp) != 1){
                perror("File Write Fail\n");
                exit(1);
            }
        }
        /* Close Read End Of Pipe */
        if(close(fd[1][0]) != 0){
            perror("Close Pipe Fail\n");
            exit(1);
        }

        /* Calculate Time Consume */
        if ((gettimeofday(&endtime, NULL)) == -1) {
            perror("gettimeofday");
            exit(1);
        }
        timediff = (endtime.tv_sec - starttime.tv_sec) +
            (endtime.tv_usec - starttime.tv_usec) / 1000000.0;
        fprintf(stdout, "%.4f\n", timediff);

        return 0;
    }
    
    /* Get value from child process except last child */
    for(control = 1; control < num_process; control++){
        for(size_control = 0; size_control < each_process_sort; size_control++){
            if(read(fd[control][0], &temp, sizeof(struct rec)) != sizeof(struct rec)){
                perror("Read Fail\n");
                exit(1);
            }
            answer_1[control - 1][size_control].freq = temp.freq;
            strcpy(answer_1[control - 1][size_control].word, temp.word);
        }
        /* Close Read End Of Pipe */
        if(close(fd[control][0]) != 0){
            perror("Close Pipe Fail\n");
            exit(1);
        }
    }

    /* Get value from last child */
    for(size_control = 0; size_control < last_process_sort; size_control++){
        if(read(fd[num_process][0], &temp, sizeof(struct rec)) != sizeof(struct rec)){
            perror("Read Fail\n");
            exit(1);
        }
        answer_2[size_control].freq = temp.freq;
        strcpy(answer_2[size_control].word, temp.word);
    }
    
    /* Close Read End Of Pipe */
    if(close(fd[num_process][0]) != 0){
        perror("Close Pipe Fail\n");
        exit(1);
    }

    /* Parent Start Merge Sort 
     * Set Variables    
    */
    struct rec final_answer[file_size];
    int condition, loop_condition, flag = 0;
    struct rec temp_hold[num_process];

    while(1){
          
          /* Take Every 0 Position Element From Each Child */
          for(loop_condition = 0; loop_condition < (num_process - 1); loop_condition++){
             temp_hold[loop_condition] = answer_1[loop_condition][0];
          }
          temp_hold[loop_condition] = answer_2[0];
          
          /* Use Helper Function To Pick The Smallest Element Position
           * Match Which Child
           */
          condition = merge_sort(temp_hold, num_process);
          
          /* If Heper Function Return -1 Means All Element Been Sorted */
          if(condition == -1){
            break;
          }
          
          /* If The Position Is Not The Last Child */
          if(condition != (num_process -1)){
             final_answer[flag].freq = answer_1[condition][0].freq;
             strcpy(final_answer[flag].word, answer_1[condition][0].word);
             change_position(answer_1[condition], each_process_sort);
          }
          
          /* If The Position Is Last Child */
          if(condition == (num_process - 1)){
              final_answer[flag].freq = answer_2[0].freq;
              strcpy(final_answer[flag].word, answer_2[0].word);
              change_position(answer_2, last_process_sort);
          }

          flag++;        
    }

    /* Write To Output File */
    int write_loop;
    for(write_loop = 0; write_loop < file_size; write_loop++){
        if(fwrite(&final_answer[write_loop], sizeof(struct rec), 1, outfp) != 1){
            perror("File Write Fail\n");
            exit(1);
        }
    }
    /* Close Output File */
    fclose(outfp);

    
    /* Calculate Time Consume */
    if ((gettimeofday(&endtime, NULL)) == -1) {
        perror("gettimeofday");
        exit(1);
    }
    timediff = (endtime.tv_sec - starttime.tv_sec) +
        (endtime.tv_usec - starttime.tv_usec) / 1000000.0;
    fprintf(stdout, "%.4f\n", timediff);
    return 0;
}
