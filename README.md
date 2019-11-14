# OS lab2：readFAT实验笔记

#### FAT表项的存储

由于FAT的每一项都是12位实际上比较难处理。由于不是整数byte，如果想要一个连续的存储（方便fread）实际上还是需要两个作为一个单位，之后再进行处理成单个的。

### print.asm

```C
/*
打印字符串，首地址在str，长度为length（传参不包括最后的\0,这个函数自动加上去）
*/
void printStr(char * str, int length);
```

### readFAT.c

```C
typedef unsigned char uint8_t; //1bytes
typedef unsigned short uint16_t; //2bytes
typedef unsigned int uint32_t; //4bytes
```

#### 数据结构

##### BPB

```c
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
```

DIR Entry

```C
typedef struct entry{
    char NAME[11];//文件名8字节，拓展名3字节
    uint8_t Attr;//文件属性
    uint8_t Resv[10];//保留位
    uint16_t WrtTime;//最后写入时间
    uint16_t WrtDate;//最后写入日期
    uint16_t FstClust;//开始簇号
    uint32_t FileSize;//文件大小
}Entry;
```

#### 函数调用

总共以下函数需要递归得对所有簇进行操作

1. 打印簇内每个表项的文件名称

2. 统计本簇内的文件（文件夹）数量

3. 打印本簇内，每一个表项的子文件数

4. 打印簇内容（cat）

5. 对簇内每个表项，进行ls（或者加上-l）命令

```C
//指针函数，处理多个簇的问题

void iterateCluster(FILE * fp, int cluster, int(*p)(FILE * fp, int cluster));



//1.
int printClustersFileName(void* cluster);

//2.1
int countClustersFileNum(void* cluster);

//2.2
int countClustersDirNum(void* cluster);

//3.
int printClusterFDNum(void * cluster);

//4. 返回剩余length
int catCluster(void * cluster,int length);




/*
fp是文件指针，cluster定位到的当前目录的目录项
打印当前表项的子文件名，//然后递归调用所有的簇
就认为这个文件夹表项小于512。cnm
*/
void printDirName(FILE * fp, int cluster);
/*
url包括了路径和当前文件的文件名。
*/
void printDir(FILE * fp, int cluster, char* url);
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
fp文件指针，cluster指向是当前目录（最开始的时候是根目录）的目录项的簇号。通过cluster这个簇号来返回。
返回目标对象的文件名（可能是目录也可能是文件）
如果没找到或者最后一项是一个文件，直接返回空串，cluster为-1。（注意是字符串，不是空。）
*/
char* findDirEntry(FILE * fp, int clusterSrc, const char * url, int* clusterRes);
/*
两个str是否相等
*/
int strEql(char* src, char* dest);
/*个it个it个it
字符串拼接
*/
char* strAdd(const char* src, const char* dest);
/*
打印红色字符
*/
void printRed(const char* src);
/*
新建一个子串
*/
char* subStr(const char* src, int length);
/*
打印一个文件名
*/
void printName(const char* src);
```

这里大部分位置的传递都是通过簇号，原因是就算是目录项也是通过FAT的形式存储的，必须通过簇号找到下一个簇才可以。

### 有待解决的问题

#### FAT的读取

使用位域的时候，读取的东西似乎出现了一些由于大小端产生的问题，但我不知道为什么。

![](/home/leo/Desktop/TIM图片20191112235553.png)

```C
typedef struct FATentry2{
    uint16_t entry  :12;
    uint16_t entry2 :12;
}FATEntry2;


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
        // if(i % 2 == 0){
        //     //高8位在第一个，低4位在第二个
        //     // *(FAT + i)  = ((uint16_t)(FAT2 + a)->entry2[0]) << 4 + ((uint16_t)(FAT2 + a)->entry2[0]) & 0xF;
            
        // }else{
        //     *(FAT + i)  = (uint16_t) (FAT2 + a)->entry2;
        // }
        
        // uint16_t* haha = (uint16_t)(FAT + i);
        // *haha = (b == 0) ? (FAT2 + a)->entry: (FAT2 + a)->entry2;
        *(FAT + i) = (uint16_t)(b == 0) ? (FAT2 + a)->entry: (FAT2 + a)->entry2;
    }
    printf("stznsln\n");
    free(FAT2);

}
```

改成三个byte的结构