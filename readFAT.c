#include <stdio.h>
#include <stdlib.h>
typedef unsigned char uint8_t; //1bytes
typedef unsigned short uint16_t; //2bytes
typedef unsigned int uint32_t; //4bytes
extern void printKKP();
typedef struct BPB{
    uint16_t BPB_BytesPerSec;//每扇区字节数
    uint8_t BPB_SecPerClus;//每簇扇区数
    uint16_t BPB_RsvdSecCnt;//Boot记录占用多少扇区
    uint8_t BPB_NumFATs;//总共多少FAT表
}BPB;
int main(int argc, char * argv[]){
    FILE *fp;
    if((fp = fopen(argv[1], "r")) == NULL){
        printf("can't open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    BPB bpb;
    fseek(fp, 11, SEEK_SET);
    fread(&bpb, 1, 6, fp);
    printf("kkp\n");
    printf("kkp\n");
    
    //printKKP();
    return 0;
}