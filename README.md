# readFAT

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

```C
/*
fp是定位到的当前目录的目录项的文件指针
打印当前下的文件名，然后递归调用所有的子文件夹
*/
void printDir(const FILE * fp);
/*
fp是定位到的当前目录的目录项的文件指针
-l格式打印当前下的文件名（文件夹写出子文件夹和子文件数目，文件写出大小）
对于子目录会进行递归调用
*/
void printDirL(const FILE * fp);
/*
fp指向的目录项代表的目录的子文件夹和子文件数目（不算. and ..）
结果写在*dirNum 和*arcNum中
*/
void getDirSubNum(const FILE * fp, int * dirNum, int * arcNum);
/*
cat文件。clust是簇号，length是剩余的长度。递归调用
*/
void catArc(const FILE * fp, int clust, int length);
/*
从当前目录项下，找到url对应的目录的目录项。（递归调用直到url只剩一个文件名）
fp是当前目录的目录项的文件指针。通过result这个指针来返回。
返回目标对象的文件名（可能是目录也可能是文件）
如果没找到，直接返回空串。（注意是字符串，不是空。）
*/
char* findDirEntry(const FILE * fp, const char * url, FILE result);
/*
两个str是否相等
*/
int strEql(char* src, char* dest);
```

