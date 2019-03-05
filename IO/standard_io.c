#include "tlpi_hdr.h"
#include <stdio.h>

//example1:以字符为单位，从标准输入读，从标准输出输出
// int main(){
//     int c;
//     while((c=getc(stdin))!=EOF){
//         if(putc(c,stdout)==EOF){
//             printf("error\n");
//         }
//     }
//     return 0;
// }

//example1:以行为单位，从标准输入读，从标准输出输出
// #define MAXLINE 100000
// int main(){
//     char buf[MAXLINE];
//     while((fgets(buf,MAXLINE,stdin))!=NULL){
//         if(fputs(buf,stdout)==EOF){
//             printf("error\n");
//         }
//     }
//     if(ferror(stdin)){
//         printf("input error\n");
//     }
//     return 0;
// }

// 将文件newfile中的内容，复制到newfile2
// #define MAXLINE 100000
// int main(){
//     FILE *f1,*f2;
//     f1=fopen("./newfile","r");
//     f2=fopen("./newfile2","w");
//     char buf[MAXLINE];
//     while((fgets(buf,MAXLINE,f1))!=NULL){
//         if(fputs(buf,f2)==EOF){
//             printf("error\n");
//         }
        
//     }
//     if(ferror(f1)){
//         printf("input error\n");
//     }
//     return 0;
// }

