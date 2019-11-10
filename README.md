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

