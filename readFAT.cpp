
#include <iostream>
#include <stdlib.h>
#include <string>
using namespace std;


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
typedef struct FATentry2{
    // uint16_t entry  :12;
    // uint16_t entry2 :12;
    uint8_t entry2[3];
}FATEntry2;
#pragma pack ()

//函数声明
#ifndef DEBUG
extern void printKKP();
extern void printStr(string str, int length);
#else
void printStr(string str);
#endif
void setBPB(FILE* fp);
void readRootEntry(FILE* fp);
// int strEql(const char* src, const char* dest){
//     int result = 1;
//     while(*src != '\0' && *dest != '\0'){
//         if(*src != *dest){
//             return 0;
//         }
//         src++;
//         dest++;
//     }
//     if(*src != *dest){
//         return 0;
//     }else{
//         return 1;
//     }
// }
// int strEql(const char* src, const char* dest);
void setFAT(FILE* fp);
void setDataSector();
/*
fp是文件指针，cluster定位到的当前目录的目录项
打印当前下的文件名，
*/
void printDirName(FILE * fp, int cluster);
/*
fp是文件指针，cluster定位到的当前目录的目录项
-l格式打印当前下的文件名（文件夹写出子文件夹和子文件数目，文件写出大小）
对于子目录会进行递归调用
*/
void printDirL(FILE * fp, int cluster);
/*
fp文件指针，cluster指向的目录项代表的目录的子文件夹和子文件数目（不算. and ..）
结果写在*dirNum 和*arcNum中
*/
void getDirSubNum(FILE * fp, int cluster, int * dirNum, int * arcNum);
/*
cat文件。fp文件指针，clust是簇号，length是剩余的长度。递归调用
*/
void catArc(FILE * fp, int clust, int length);
/*
从当前目录项下，找到url对应的目录的目录项。（递归调用直到url只剩一个文件名）
fp文件指针，cluster指向是当前目录的目录项的簇号。通过cluster这个簇号来返回。
返回目标对象的文件名（可能是目录也可能是文件）
如果没找到，直接返回空串。（注意是字符串，不是空。）
*/
char* findDirEntry(FILE * fp, int clusterSrc, const string url, int* clusterRes);
// /*
// 字符串拼接
// */
// char* strAdd(const char* src, const string dest);
/*
打印红色字符
*/
void printRed(const string src);
// void printName(const string src);
/*
url包括了路径和当前文件的文件名。
*/
void printDir(FILE * fp, int cluster, string url);
//由于根目录的cluster的号和数据区的不一样，单独处理。
void printRootDir(FILE * fp);
string NAME2Str(char* NAME);

//全局变量
BPB bpb;/*
// 字符串拼接
// */
// char* strAdd(const char* src, const string dest);
uint16_t* FAT;//指向FAT的第一个表项.用2个B来存一个12位的FAT表项。
int dataSector;

int main(int argc, char * argv[]){

    FILE *fp;
    if((fp = fopen(argv[1], "rb")) == NULL){
        printf("can't open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    setBPB(fp);
    setFAT(fp);
    setDataSector();
    readRootEntry(fp);
    char inputStr[100];
    printStr("\033[31mkkp  k\n\033[0m");
    // while(1){
    //     scanf("%s", inputStr);
    //     if(!strcmp(inputStr, "ls")){
    //         printStr("cmd ls\n");
    //     }else if(!strcmp(inputStr, "cat")){
    //         printStr("cmd cat\n");
    //     }else{
    //         printStr("wrong command ");
    //         printStr(inputStr);
    //         printStr("\n");
    //     }
    // }
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
    //test
    //printDir(fp, entry[0].FstClust, "/" + NAME2Str(entry[0].NAME));
    //printDir(fp, 19 -31, "/");
    printRootDir(fp);
    int a = 0;
    int b = 0;
    getDirSubNum(fp, entry[1].FstClust, &a, &b);
    // if(entry[0].Attr == 0x10){
    //     //try sub dir, find it's entry and put it to subEntry;
    //     Entry subEntry[3];
    //     int subEntrySector = dataSector + entry[0].FstClust * bpb.SecPerClus ;//数据区的第一个簇是簇2
    //     int subDirEntryBase = subEntrySector * bpb.BytesPerSec;
    //     fseek(fp, subDirEntryBase, SEEK_SET);
    //     fread(&subEntry, 3, sizeof(Entry), fp);
    //     printStr("try sub Entry");
    // }
}
void setDataSector(){
    //设定全局变量：数据区簇号。
    int rootDirSectors = (bpb.RootEntCnt * 32 + bpb.BytesPerSec - 1) / bpb.BytesPerSec;
    dataSector = rootDirSectors + 1 + bpb.NumFATs * bpb.FATSz16 - 2;//数据区第一个簇（簇2）的绝对簇号。由于数据区的第一个簇是2，所以计算绝对簇号的时候要减去2。这几个分别是，根目录的簇数量，引导扇区，FAT占据的簇。使用的时候，用数据簇号加上这个就行了。
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
// int strEql(const char* src, const char* dest){
//     int result = 1;
//     while(*src != '\0' && *dest != '\0'){
//         if(*src != *dest){
//             return 0;
//         }
//         src++;
//         dest++;
//     }
//     if(*src != *dest){
//         return 0;
//     }else{
//         return 1;
//     }
// }
void setFAT(FILE* fp){
    FATEntry2* FAT2 = (FATEntry2*) malloc(bpb.FATSz16 * bpb.BytesPerSec);//获取FAT表那么大的空间
    int fileResult;
    if((fileResult = fseek(fp, bpb.BytesPerSec, SEEK_SET)) == -1){
        printStr("can't find FAT\n");
        exit(EXIT_FAILURE);
    }
    if((fileResult = fread(FAT2, 1, bpb.FATSz16 * bpb.BytesPerSec, fp)) != bpb.FATSz16 * bpb.BytesPerSec){
        printStr("can't read FAT\n");
    }//读取的时候是连续的，所以每两个项是在3个Byte里面

    int entryNum = 0;//FAT表项的数量
    entryNum = bpb.FATSz16 * bpb.BytesPerSec * 2 / 3;//byte / 1.5是entry的总数
    FAT = (uint16_t*) malloc(entryNum * 2);//给每个表项分配两个Byte
    int a = 0, b = 0;
    for(int i = 0; i < entryNum; i++){
        a = i / 2;
        b = i % 2;
        if(i % 2 == 0){
            //高8位在第一个，低4位在第二个
            *(FAT + i)  = (((uint16_t)(FAT2 + a)->entry2[0]) << 4) + ((((uint16_t)(FAT2 + a)->entry2[1]) & 0xF0) >> 4);
            
        }else{
            *(FAT + i)  = ((((uint16_t) (FAT2 + a)->entry2[1]) & 0xF) << 8) + ((uint16_t)(FAT2 + a)->entry2[2]);
        }
        
        // uint16_t* haha = (uint16_t)(FAT + i);
        // *haha = (b == 0) ? (FAT2 + a)->entry: (FAT2 + a)->entry2;
        // *(FAT + i) = (uint16_t)(b == 0) ? (FAT2 + a)->entry: (FAT2 + a)->entry2;
        
    }
    
    free(FAT2);

}
void getDirSubNum(FILE * fp, int cluster, int * dirNum, int * arcNum){
    Entry subEntry[16];//cnm，假设文件夹里最多16个文件
    fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    fread(subEntry, bpb.BytesPerSec, 1, fp);
    *dirNum = 0;
    *arcNum = 0;
    for(int i = 2; i < 16; i++){//不算. and ..
        if(subEntry[i].Attr == 0x10){
            //is Dir
            (*dirNum)++;
        }else if(subEntry[i].NAME[0] == 0){
            //notFile
        }else{
            //FILE
            (*arcNum)++;
        }
    }
}
void printDir(FILE * fp, int cluster, string url){
    Entry subEntry[16];//cnm，假设文件夹里最多16个文件
    fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    fread(subEntry, bpb.BytesPerSec, 1, fp);
    printStr(url + "/");
    printStr(":\n");
    printDirName(fp, cluster);
    printStr("\n");
    for(int i = 0; i < 16; i++){

        if(subEntry[i].Attr == 0x10){
            //is Dir
            if(NAME2Str(subEntry[i].NAME) != "." && NAME2Str(subEntry[i].NAME) != ".."){
                printDir(fp, subEntry[i].FstClust, url + "/" + NAME2Str(subEntry[i].NAME));
                //char* newUrl = strAdd(url, "/");
                //newUrl = strAdd(newUrl, subEntry[i].NAME);
                //string newUrl = url + "//";
                //printDir(fp, subEntry[i].FstClust, newUrl);
                //free(newUrl);
            }
        }
    }
}
void printDirName(FILE * fp, int cluster){
    Entry subEntry[16];//cnm，假设文件夹里最多16个文件
    fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    fread(subEntry, bpb.BytesPerSec, 1, fp);
    for(int i = 0; i < 16; i++){
        if(i != 0){
            printStr("  ");
        }
        if(subEntry[i].Attr == 0x10){
            //is Dir
            printRed(NAME2Str(subEntry[i].NAME));
        }else if(subEntry[i].NAME[0] == 0){
            //notFile
        }else{
            //FILE
            // printName(subEntry[i].NAME);
            printStr(NAME2Str(subEntry[i].NAME));
        }
    }
    printStr("\n");
    

    // Entry * subEntries = malloc(bpb.BytesPerSec);//Entry总共占有一个簇那么多的空间。
    // int entryNum = bpb.BytesPerSec / sizeof(Entry);//共有几个表单项
    // fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    // fread(subEntries, bpb.BytesPerSec, 1, fp);
    // //printStr(":\n");
    // char sname[4];//后缀名的字符串
    // //打印当前的簇中所有文件名。
    // for(int i = 0; i < entryNum; i++){
    //     if(!strEql((subEntries + i)->NAME, "")){//如果不是空的
    //         if(subEntries[i].Attr == 0x10){
    //             //is dir
    //             printRedName((subEntries + i) -> NAME);const char *
    //         }else{
    //             printStrName((subEntries + i) -> NAME);
    //             printStr(".");
    //             sname[3] = '\0';
    //             sname[0] = *((subEntries + i) -> NAME + 8);
    //             sname[1] = *((subEntries + i) -> NAME + 9);
    //             sname[2] = *((subEntries + i) -> NAME + 10);
    //             printStr(sname);

    //         }
    //         printStr("  ");
    //     }
        
    // }
    // free(subEntries);
    //TODO:暂时假设一个文件夹最多就是16个子文件
    // //如果还有下一簇，则递归调用
    // if(FAT[cluster] == 0xFF7){
    //     printStr("\nbad cluster\n");
    // }else if(FAT[cluster] >= 0xFF8){
    //     //最后一个表项文件名都已经打印完毕
    //     printStr("\n");
    // }else{
    //     printDir(fp, FAT[cluster]);
    // }
}
void printRed(const string src){
    // printStr("\033[31m");
    // printStr(src);
    // printStr("\033[0m");
    printStr("\033[31m" + src + "\033[0m");
}
// void printRedName(const string src){
//     char name[9];
//     int i = 0;
//     for(i = 0; i < 8;i++){
//         if(*(src + i) == ' '){
//             name[i] = '\0';
//         }else{
//             name[i] = *(src + i);
//         }
//     }
//     name[8] = '\0';
//     printRed(name);
// }
// void printName(const char* src){
//     // char str[12];
//     // int i = 0;
//     // while(*(src + i) != ' '){
//     //     str[i] = *(src + i);
//     //     i++;
//     // }
//     // str[i] = '.';
//     // str[i + 1] = *(src + 8);
//     // str[i + 2] = *(src + 9);
//     // str[i + 3] = *(src + 10);
//     // str[i + 4] = '\0';
//     // printStr(str);


// }
string NAME2Str(char* NAME){
    string result = "";
    for(int i = 0; i < 8;i++){
        if(NAME[i] != ' '){
            result += NAME[i];
        }
    }
    if(NAME[8] != ' '){
        result += ".";
    }
    for(int i = 8; i < 11; i++){
        if(NAME[i] != ' '){
            result += NAME[i];
        }
    }
    return result;
}

void printRootDir(FILE * fp){
    printDir(fp, 19 - 31, "");
}

#ifdef DEBUG
void printStr(string str){
    cout << str;
    //printf("%s", str);
    // char* ptr = str;
    // while(*ptr != 32){
    //     printf("%c", ptr);
    //     ptr++;
    // }
    // fflush(stdout);
}
#endif