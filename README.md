# readFAT

### Functions

#### print.asm

```C
void printStr(char * str, int length);
/*
打印字符串，首地址在str，长度为length（传参不包括最后的\0,这个函数自动加上去）
*/
```

#### readFAT.c

```C
typedef unsigned char uint8_t; //1bytes
typedef unsigned short uint16_t; //2bytes
typedef unsigned int uint32_t; //4bytes
```

##### 数据结构

```c
typedef struct BPB{
    uint16_t BPB_BytesPerSec;//每扇区字节数
    uint8_t BPB_SecPerClus;//每簇扇区数
    uint16_t BPB_RsvdSecCnt;//Boot记录占用多少扇区
    uint8_t BPB_NumFATs;//总共多少FAT表
    uint16_t BPB_RootEntCnt;//根目录文件数最大值。
    uint16_t BPB_TotSec16;//扇区总数
    uint8_t BPB_Media;//介质描述符
    uint16_t BPB_FATSz16;//每FAT扇区数
    uint16_t BPB_SecPerTrk;//每磁道扇区数
    uint16_t BPB_NumHeads;//磁头数（面数）
    uint32_t BPB_HiddSec;//隐藏扇区数
    uint32_t BPB_TotSec32;//如果BPB_TotSec16为0则在这里记录扇区总数
}BPB;
```

