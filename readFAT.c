#include <stdio.h>
#include <stdlib.h>
typedef unsigned char uint8_t; //1bytes
typedef unsigned short uint16_t; //2bytes
typedef unsigned int uint32_t; //4bytes
extern void printKKP();
#pragma pack (1)//结构体内按一字节对齐，这样读的时候可以一次读完，不然还得一个一个读，比较麻烦。或者有更好的方法吗？
typedef struct BPB{
    uint16_t BytesPerSec;//每扇区字节数
    uint8_t SecPerClus;//每簇扇区数
    uint16_t RsvdSecCnt;//Boot记录占用多少扇区
    uint8_t NumFATs;//总共多少FAT表
    uint16_t RootEntCnt;//根目录文件数最大值。
    uint16_t TotSec16;//扇区总数
    uint8_t Media;//介质描述符
    uint16_t FATSz16;//每FAT扇区数
    uint16_t SecPerTrk;//每磁道扇区数
    uint16_t NumHeads;//磁头数（面数）
    uint32_t HiddSec;//隐藏扇区数
    uint32_t TotSec32;//如果BPB_TotSec16为0则在这里记录扇区总数
}BPB;
typedef struct entry{
    char NAME[11];//文件名8字节，拓展名3字节
    uint8_t Attr;//文件属性
    uint8_t Resv[10];//保留位
    uint16_t WrtTime;//最后写入时间
    uint16_t WrtDate;//最后写入日期
    uint16_t FstClust;//开始簇号
    uint32_t FileSize;//文件大小
}Entry;
#pragma pack ()
int main(int argc, char * argv[]){
    FILE *fp;
    if((fp = fopen(argv[1], "r")) == NULL){
        printf("can't open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    BPB bpb;    
    fseek(fp, 11, SEEK_SET);
    fread(&bpb, 1, 25, fp);

    printf("kkp\n");
    printf("kkp\n");
    
    //printKKP();
    return 0;
}