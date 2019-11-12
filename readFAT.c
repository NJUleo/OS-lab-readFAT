#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DEBUG
//数据结构定义
typedef unsigned char uint8_t; //1bytes
typedef unsigned short uint16_t; //2bytes
typedef unsigned int uint32_t; //4bytes
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
typedef struct entry{//32 byte
    char NAME[11];//文件名8字节，拓展名3字节
    uint8_t Attr;//文件属性 0x10 for dir;
    uint8_t Resv[10];//保留位
    uint16_t WrtTime;//最后写入时间
    uint16_t WrtDate;//最后写入日期
    uint16_t FstClust;//开始簇号
    uint32_t FileSize;//文件大小
}Entry;
#pragma pack ()

//函数声明
#ifndef DEBUG
extern void printKKP();
extern void printStr(char * str, int length);
#else
void printStr(const char * str);
#endif
void setBPB(FILE* fp);
void readRootEntry(FILE* fp);
int strEql(const char* src, const char* dest);

//全局变量
BPB bpb;
int main(int argc, char * argv[]){
    FILE *fp;
    if((fp = fopen(argv[1], "rb")) == NULL){
        printf("can't open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    setBPB(fp);
    readRootEntry(fp);
    char inputStr[100];
    printStr("\033[31mkkp\n\033[0m");
    while(1){
        scanf("%s", inputStr);
        if(!strcmp(inputStr, "ls")){
            printStr("cmd ls\n");
        }else if(!strcmp(inputStr, "cat")){
            printStr("cmd cat\n");
        }else{
            printStr("wrong command ");
            printStr(inputStr);
            printStr("\n");
        }
    }
    printStr("kkp");
    
    
    return 0;
}
void readRootEntry(FILE* fp){
    int rootEntryBase = (bpb.NumFATs * bpb.FATSz16 + 1) * bpb.BytesPerSec;//FAT扇区数+引导扇区，在算出byte，就是目录区的base
    fseek(fp, rootEntryBase, SEEK_SET);
    Entry* entries = (Entry*) malloc(2 * sizeof(Entry));
    Entry entry[8];
    fread(&entry, 8, sizeof(Entry), fp);
    printStr("Root Entry read\n");
    if(entry[0].Attr == 0x10){
        //try sub dir, find it's entry and put it to subEntry;
        Entry subEntry[3];
        int rootDirSectors = (bpb.RootEntCnt * 32 + bpb.BytesPerSec - 1) / bpb.BytesPerSec;
        int dataSector = rootDirSectors + 1 + bpb.NumFATs * bpb.FATSz16;
        int subEntrySector = dataSector + entry[0].FstClust * bpb.SecPerClus - 2;//数据区的第一个簇是簇2
        int subDirEntryBase = subEntrySector * bpb.BytesPerSec;
        fseek(fp, subDirEntryBase, SEEK_SET);
        fread(&subEntry, 3, sizeof(Entry), fp);
        printStr("try sub Entry");
    }
}
void setBPB(FILE *fp){
    int fileResult;
    if((fileResult = fseek(fp, 11, SEEK_SET)) == -1){
        printStr("img file too small\n");
        exit(EXIT_FAILURE);
    }
    if((fileResult = fread(&bpb, 1, 25, fp)) != 25){
        printStr("can't read legal BPB\n");
    }
    
}
int strEql(const char* src, const char* dest){
    int result = 1;
    while(*src != '\0' && *dest != "\0"){
        if(*src != *dest){
            return 0;
        }
        src++;
        dest++;
    }
    if(*src != *dest){
        return 0;
    }else{
        return 1;
    }
}
#ifdef DEBUG
void printStr(const char * str){
    printf("%s", str);
    fflush(stdout);
}
#endif