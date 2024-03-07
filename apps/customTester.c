#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fs.h>


void basic_test(){
    printf("%d\n", fs_mount("disk.fs"));

    fs_info();


    fs_create("file1");
    fs_ls();
    fs_create("file2");
    fs_ls();   

    int fd1 = fs_open("file1");
    printf("%d: %d\n", fd1, fs_stat(fd1));
    fs_close(fd1);


    int fd2 = fs_open("file2");
    printf("%d: %d\n", fd2, fs_stat(fd2));
    fs_close(fd2);

    fs_delete("file1");
    fs_ls();
    fs_delete("file2");
    fs_ls();      
     
    fs_umount();
}

void simple_read_write_test(){
    fs_mount("disk.fs");



    fs_create("file1");
    //fs_ls();


    int fd1 = fs_open("file1");
    int8_t *buf = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf[i]= 'e';
    }
    fs_write(fd1, buf, 4096);

    int8_t *newBuf = malloc(4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 4096);    

    printf("%s\n", newBuf);


    fs_close(fd1);

    fs_delete("file1");
    //fs_ls();      
     
    fs_umount();
}
void didnt_close_file_descriptor(){
    fs_mount("disk.fs");



    fs_create("file1");
    //fs_ls();


    int fd1 = fs_open("file1");
    int8_t *buf = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf[i]= 'e';
    }
    fs_write(fd1, buf, 4096);

    int8_t *newBuf = malloc(4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 4096);    


     
    printf("%d\n", fs_umount());
}
void simple_read_write_test_multi_block(){
    fs_mount("disk.fs");



    fs_create("file1");
    //fs_ls();


    int fd1 = fs_open("file1");
    int8_t *buf = malloc(3*4096*sizeof(int8_t));
    for (int i=0; i<3*4096; i++){
        buf[i]= 'm';
    }
    fs_write(fd1, buf, 3*4096);

    int8_t *newBuf = malloc(3*4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 3*4096);    

    printf("%s\n", newBuf);


    fs_close(fd1);

    fs_delete("file1");
    //fs_ls();      
     
    fs_umount();
}


void write_to_two_files(){
    fs_mount("disk.fs");



    fs_create("file1");


    int fd1 = fs_open("file1");
    int8_t *buf = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf[i]= 'p';
    }
    fs_write(fd1, buf, 4096);

    int8_t *newBuf = malloc(4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 4096);    
    printf("%s\n", newBuf);  



    fs_create("file2");


    int fd2 = fs_open("file2");
    int8_t *buf2 = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf2[i]= 'm';
    }
    fs_write(fd2, buf2, 4096);

    int8_t *newBuf2 = malloc(4096*sizeof(int8_t));
    fs_read(fd2, newBuf2, 4096);  
    printf("%s\n", newBuf2);  




    fs_close(fd1);
    fs_close(fd2);

    fs_delete("file1");
    fs_delete("file2");    
    //fs_ls();      
     
    fs_umount();
}

void write_to_two_files_mixmatch_order(){
    fs_mount("disk.fs");


    fs_create("file1");

    int fd1 = fs_open("file1");
    int8_t *buf = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf[i]= 'a';
    }
    fs_write(fd1, buf, 4096);


    fs_create("file2");


    int fd2 = fs_open("file2");
    int8_t *buf2 = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf2[i]= 'x';
    }
    fs_write(fd2, buf2, 4096);



    int8_t *buf3 = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf3[i]= 'b';
    }
    fs_write(fd1, buf3, 4096); 


    int8_t *buf4 = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf4[i]= 'y';
    }
    fs_write(fd2, buf4, 4096);   


    int8_t *buf5 = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf5[i]= 'c';
    }
    fs_write(fd1, buf5, 4096);     



    int8_t *buf6 = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf6[i]= 'z';
    }
    fs_write(fd2, buf6, 4096);  



    int8_t *newBuf = malloc(3*4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 3*4096);    
    printf("%s\n", newBuf);  

    int8_t *newBuf2 = malloc(3*4096*sizeof(int8_t));
    fs_read(fd2, newBuf2, 3*4096);  
    printf("%s\n", newBuf2);  


    fs_close(fd1);
    fs_close(fd2);

    fs_delete("file1");
    fs_delete("file2");    
    //fs_ls();      
     
    fs_umount();
}

void write_to_two_files_mixmatch_order_v2(){
    fs_mount("disk.fs");



    fs_create("file1");


    int fd1 = fs_open("file1");
    int8_t *buf = malloc(3*4096*sizeof(int8_t));
    for (int i=0; i<3*4096; i++){
        buf[i]= 'a';
    }
    fs_write(fd1, buf, 3*4096);



    fs_create("file2");


    int fd2 = fs_open("file2");
    int8_t *buf2 = malloc(3*4096*sizeof(int8_t));
    for (int i=0; i<3*4096; i++){
        buf2[i]= 'x';
    }
    fs_write(fd2, buf2, 3*4096);



    int8_t *buf3 = malloc(2*4096*sizeof(int8_t));
    for (int i=0; i<2*4096; i++){
        buf3[i]= 'b';
    }
    fs_write(fd1, buf3, 2*4096); 


    int8_t *buf4 = malloc(2*4096*sizeof(int8_t));
    for (int i=0; i<2*4096; i++){
        buf4[i]= 'y';
    }
    fs_write(fd2, buf4, 2*4096);   


    int8_t *buf5 = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf5[i]= 'c';
    }
    fs_write(fd1, buf5, 4096);     



    int8_t *buf6 = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf6[i]= 'z';
    }
    fs_write(fd2, buf6, 4096);  



    int8_t *newBuf = malloc(6*4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 6*4096);    
    printf("%s\n", newBuf);  

    int8_t *newBuf2 = malloc(6*4096*sizeof(int8_t));
    fs_read(fd2, newBuf2, 6*4096);  
    printf("%s\n", newBuf2);  


    fs_close(fd1);
    fs_close(fd2);

    fs_delete("file1");
    fs_delete("file2");    
    //fs_ls();      
     
    fs_umount();
}

void other_write_test(){
    fs_mount("disk.fs");


    fs_create("file1");
    //fs_ls();


    int fd1 = fs_open("file1");
    
    int8_t *buf = malloc(2*4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf[i]= 'x';
    }
    for (int i=4096; i<2*4096; i++){
        buf[i]= 'y';
    }    
    fs_write(fd1, buf, 2*4096);

    int8_t *newBuf = malloc(2*4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 2*4096);    

    printf("%s\n", newBuf);


    fs_close(fd1);

    fs_delete("file1");
    //fs_ls();      
     
    fs_umount();
}

void size_change_test(){
    fs_mount("disk.fs");


    fs_ls();
    fs_create("file1");
    fs_ls();


    int fd1 = fs_open("file1");
    
    int8_t *buf = malloc(2*4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf[i]= 'x';
    }
    for (int i=4096; i<2*4096; i++){
        buf[i]= 'y';
    }    
    fs_write(fd1, buf, 2*4096);

    int8_t *newBuf = malloc(2*4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 2*4096);    

    //printf("%s\n", newBuf);

    fs_ls();

    fs_close(fd1);

    fs_delete("file1");
    fs_ls();
     
    fs_umount();
}


void size_change_test_2_files(){
    fs_mount("disk.fs");


    fs_ls();
    fs_create("file1");
    fs_ls();


    int fd1 = fs_open("file1");
    
    int8_t *buf = malloc(4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf[i]= 'x';
    }
    fs_write(fd1, buf, 4096);

    int8_t *newBuf = malloc(4096*sizeof(int8_t));
    fs_read(fd1, newBuf, 4096);    

    fs_create("file2");
    fs_ls();


    int fd2 = fs_open("file2");
    
    int8_t *buf2 = malloc(2*4096*sizeof(int8_t));
    for (int i=0; i<4096; i++){
        buf2[i]= 'a';
    }
    for (int i=4096; i<4096; i++){
        buf2[i]= 'b';
    }    
    fs_write(fd2, buf2, 2*4096);

    int8_t *newBuf2 = malloc(2*4096*sizeof(int8_t));
    fs_read(fd1, newBuf2, 2*4096); 

    fs_ls();

    fs_close(fd1);

    fs_delete("file1");

    fs_close(fd2);

    fs_delete("file2");    
    fs_ls();
     
    fs_umount();
}

void create_simple(){
    fs_mount("disk.fs");

    fs_create("test-file-1");



    fs_ls();


    fs_delete("test-file-1");


    fs_umount();
}



int main()
{
    //basic_test();
    //simple_read_write_test();
    //didnt_close_file_descriptor();
    //simple_read_write_test_multi_block();
    //write_to_two_files();
    //write_to_two_files_mixmatch_order();
    //write_to_two_files_mixmatch_order_v2();
    //other_write_test();
    //size_change_test();
    //size_change_test_2_files();
    create_simple();

	return 0;
}
