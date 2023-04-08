#include "stdior.h"
#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX  512
#endif
// I pledge my honor that I have abided by the Stevens honor system.
//  - Robertson Romero

int len(int n){ //finds the numbers of digits
    int digits = 0;
    if(n == 0){
        return 1;
    }
    if(n < 0){
        n = -n;
    }
    while(n > 0){
        n /= 10;
        digits++;
    }
    return digits;
}

char* itos(int n){ //integer to string
    int count = 0;
    int flag = 0; 
    int size = len(n);
    char* str = (char*)malloc(size + 1);
    
    
    if (n < 0) { // in the case n is negative
        flag = 1; //sets the flag to 1
        n = -n; //makes n a positive to make things easier
    }

    while (n != 0) { // while the condition is true
        str[count++] = n % 10 + '0'; //take the value of the right most digit, converts to ASCII and inserts in the array, from right to left.
        n /= 10; //then remove that right most digit
    }

    if (flag) { //if the condition is true, when flag is 1 it indicates that the number was negative
        str[count++] = '-'; //appends the negative sign
    }

    str[count] = '\0'; //sets the null terminator at the end of the string

    for(int i = 0; i < count/2; i++) { //reverses the array to normal
        char tmp; 
        tmp = str[i];
        str[i] = str[count-i-1];
        str[count-i-1] = tmp;
    }
    return str;
}

int fprintfx(char *filename, char format, void *data){
    int fd;
    if (filename[0] == '\0'){
        fd = 1;
    }
    else{
        fd = open(filename, O_APPEND | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
    }

    if (data == NULL){ // checks if the base address is null
        errno = EIO;
        return -1;
    }

    size_t size; // holding place for our size
    ssize_t bytes_written;
    
    char* str = NULL;
    if (format == 's'){// checks the format entered
        size = strlen(data) + 1; // sets the size of the bytes of our buffer
        str = (char*)malloc(size + 1);
        if(str == NULL){
           errno = EIO;
           return -1;
        }
        strcpy(str, (char*)data);
        str[size - 1] = '\n';
        str[size] = '\0';
    }
    else if (format == 'd'){ // checks the format entered;
        int value = (*(int*)data);
        char* int_str = itos(value);
        size = strlen(int_str) + 1;
        str = (char*)malloc(size + 1);
        if(str == NULL){
            errno = EIO;
            free(int_str);
            return -1;
        }
        strcpy(str,int_str);
        str[size - 1] = '\n';
        str[size] = '\0';
        free(int_str);
    }
    else{
        errno  = EIO;
        return -1;
    }
    bytes_written = write(fd,str,size);
    free(str);
    if (bytes_written == -1){ // if write fails to write
        errno = EIO;
        // free(str);
        return -1;
    }
    else{
        
        return 0;
    }
}

int fscanfx(char* filename, char format, void* dst){ //(filename, format, dst)
    struct stat finfo;
    struct dirent *dirp;
    DIR *dir;
    char* fdpath = "/proc/self/fd";
    int fd = -2;
    stat(filename,&finfo);
    long fileinode = finfo.st_ino;
    long filesize = finfo.st_size;
    dir = opendir(fdpath);
    if (!dir) {
        return -1;
    }
    if(dst == NULL){
        closedir(dir);
        return 0;
    }

    if(filename[0] == '\0'){
        fd = 0;
    }
    else{
        while ((dirp = readdir(dir)) != NULL) {
            if (dirp->d_name[0] == '.'){
                continue;
            }

            char entry[_POSIX_PATH_MAX] = {0};
            char* path = NULL;

            strcat(entry,fdpath);
            strcat(entry,"/");
            strcat(entry, dirp->d_name);
            path = realpath(entry,NULL);
            
            if(stat(path,&finfo) == -1){
                free(path);
                continue;
            }

            free(path);

            if(fileinode == finfo.st_ino){
                fd = atoi(dirp->d_name);
                break;
            } 
        } 
    }
    closedir(dir);

    if(fd == -2){
        fd = open(filename, O_RDONLY);
    }

    if(fd == -1){
        errno = ENOENT;
        return -1;
    }
    
    char *buffer = malloc(128);
    if (!buffer) {
        errno = EIO;
        free(buffer);
        return -1;
    }

    char ch;
    size_t bufsize = 128;
    ssize_t num_read = 0;
    ssize_t current_read = 0;
    
    if(lseek(fd,0,SEEK_CUR) == filesize){
        free(buffer);
        return -2;
    }

    while((current_read = read(fd,&ch,1)) != -1 ){
        if(current_read == 0){
            break;
        }

         if (ch == '\n'){
            break;
        }
        
        buffer[num_read++] = ch;

        if(num_read + 1 >= bufsize){
            bufsize += 128;
            buffer = realloc(buffer,bufsize);
        }
    }

    if(buffer[num_read - 1] == '\n'){
        buffer[num_read - 1] = '\0';
    }

    if(format == 's') {
        buffer[num_read] = '\0';
        strcpy(dst,buffer);
    }

    if(format == 'd') {
        buffer[num_read] = '\0';
        *(int *)dst = atoi(buffer);
    }

    free(buffer);

    if (format != 's' && format != 'd'){
        errno = EIO;
        return -1;
    }
    else{
        return 0;
    }
}

int clean(){
    struct dirent *dirp;
    DIR *dir;
    int fd;
    char* fdpath = "/proc/self/fd";
    dir = opendir(fdpath);
    int cln;
    

    if (!dir) {
        return -1;
    }

    while((dirp = readdir(dir)) != NULL){
        if(dirp->d_name[0] == '.'){
            continue;
        }
        if((strcmp(dirp->d_name,"2") != 0) && (strcmp(dirp->d_name,"1") != 0) && (strcmp(dirp->d_name,"0") != 0)){
            fd = atoi(dirp->d_name); 
            cln = close(fd);
            if(cln == -1){
                errno = EIO;
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);
    return 0;
}

int main(){

    //imput test code here


    return 0;
}
