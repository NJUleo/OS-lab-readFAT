#define DEBUG
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
using namespace std;


// #define DEBUG

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
void printStrString(string str);
extern "C"{
    void printStr(const char* str);
}
#else
void printStrString(string str);
#endif
void setBPB(FILE* fp);
void readRootEntry(FILE* fp);
void setFAT(FILE* fp);
void setDataSector();
/*
fp是文件指针，cluster定位到的当前目录的目录项
打印当前下的文件名，
*/
void printDirName(FILE * fp, int cluster);
void printDirNameL(FILE * fp, int cluster);
/*
fp是文件指针，cluster定位到的当前目录的目录项
-l格式打印当前下的文件名（文件夹写出子文件夹和子文件数目，文件写出大小）
对于子目录会进行递归调用
*/
void printDirL(FILE * fp, int cluster, string url);
/*
fp文件指针，cluster指向的目录项代表的目录的子文件夹和子文件数目（不算. and ..）
结果写在*dirNum 和*arcNum中
*/
void getDirSubNum(FILE * fp, int cluster, int * dirNum, int * arcNum);
/*
cat文件。fp文件指针，clust是簇号，length是剩余的长度。递归调用
*/
void catArc(FILE * fp, int clust, string FileName);
/*
从当前目录项下，找到url对应的目录的目录项。（递归调用直到url只剩一个文件名）
fp文件指针，cluster指向是当前目录的目录个it项的簇号。通过cluster这个簇号来返回。
返回目标对象的文件名（可能是目录也可能是文件）
如果没找到，直接返回空串。（注意是字符串，不是空。）
*/
int findDirEntry(FILE * fp, int clusterSrc, string url);
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
// void printRootDir(FILE * fp);
void catURL(FILE* fp, string url);
void printURL(FILE* fp, string url);
void printURLL(FILE* fp, string url);
string NAME2Str(char* NAME);
string getFirstNameUrl(string url);
string validateURL(string url);//处理一下url前后的“/”问题
/*
分割字符串
*/
vector<string> splitStr(string str, string s);
bool isL(string str);
bool isManyL(vector<string> src);
string getFileNameFromURL(string url);
void catFile(FILE * fp, int cluster, int size);
string getFileNameFromURL(string url);

//全局变量
BPB bpb;/*
// 字符串拼接
// */
// char* strAdd(const char* src, const string dest);
uint16_t* FAT;//指向FAT的第一个表项.用2个B来存一个12位的FAT表项。
int dataSector;

int main(int argc, char * argv[]){

    FILE *fp;
    if((fp = fopen("a.img", "rb")) == NULL){
        printf("can't open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    setBPB(fp);
    setFAT(fp);
    setDataSector();

    string input;
    vector<string> inputSplit;
    while(1){
        printStrString(">");
        getline(cin, input);
        inputSplit = splitStr(input, " ");
        if(inputSplit.at(0) == "exit"){
            break;
        }else if(inputSplit.at(0) == "ls"){
            if(inputSplit.size() == 1){
                printURL(fp, "/");
            }else if(inputSplit.size() == 2){
                printURL(fp, inputSplit.at(1));
            }else{
                if(inputSplit.size() == 3){
                    if(isL(inputSplit.at(2))){
                        printURLL(fp, inputSplit.at(1));
                        continue;
                    }else if(isL(inputSplit.at(1))){
                        printURLL(fp, inputSplit.at(2));
                        continue;
                    }
                }else{
                    if(isL(inputSplit.at(1))){
                        if(isManyL(inputSplit)){
                            printURLL(fp, inputSplit.at(2));
                            continue;
                        }
                    }else if(isL(inputSplit.at(2))){
                        if(isManyL(inputSplit)){
                            printURLL(fp, inputSplit.at(1));
                            continue;
                        }
                    }
                }
                printStrString("invalid param for ls\n");
            }
        }else if(inputSplit.at(0) == "cat"){
            catURL(fp, inputSplit.at(1));
        }else{
            printStrString("invalid command\n");
            continue;
        }
        
    }
    printStrString("\033[31mGOODBYE AND GOOD LUCK\nNEVER GIVE UP\n\033[0m");

    
    fclose(fp);
    return 0;
}
void readRootEntry(FILE* fp){
    int rootEntryBase = (bpb.NumFATs * bpb.FATSz16 + 1) * bpb.BytesPerSec;//FAT扇区数+引导扇区，在算出byte，就是目录区的base
    fseek(fp, rootEntryBase, SEEK_SET);
    Entry* entries = (Entry*) malloc(2 * sizeof(Entry));
    Entry entry[8];
    fread(&entry, 8, sizeof(Entry), fp);

    printStrString("Root Entry read\n");
    //test
    //printDir(fp, entry[0].FstClust, "/" + NAME2Str(entry[0].NAME));
    //printDir(fp, 19 -31, "/");
    //printRootDir(fp);
    int resultCluster = 0;
    resultCluster = findDirEntry(fp, 19 - 31, "/ABC/");
    printDir(fp, resultCluster, "/ABC/SUBABC/");
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
    //     printStrString("try sub Entry");
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
        printStrString("img file too small\n");
        exit(EXIT_FAILURE);
    }
    if((fileResult = fread(&bpb, 1, 25, fp)) != 25){
        printStrString("can't read legal BPB\n");
    }
    
}
void setFAT(FILE* fp){
    FATEntry2* FAT2 = (FATEntry2*) malloc(bpb.FATSz16 * bpb.BytesPerSec);//获取FAT表那么大的空间
    int fileResult;
    if((fileResult = fseek(fp, bpb.BytesPerSec, SEEK_SET)) == -1){
        printStrString("can't find FAT\n");
        exit(EXIT_FAILURE);
    }
    if((fileResult = fread(FAT2, 1, bpb.FATSz16 * bpb.BytesPerSec, fp)) != bpb.FATSz16 * bpb.BytesPerSec){
        printStrString("can't read FAT\n");
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
            //*(FAT + i)  = (((uint16_t)(FAT2 + a)->entry2[0]) << 4) + ((((uint16_t)(FAT2 + a)->entry2[1]) & 0xF0) >> 4);
            //第一个字节是低8位，第二个字节的低4位是高4位
            *(FAT + i) = ((uint16_t)(FAT2 + a)->entry2[0]) + ((((uint16_t)(FAT2 + a)->entry2[1]) & 0x0F) << 8);
            
        }else{
            //第二个字节的高4位是低4位，第三个字节是高8位
            *(FAT + i)  = ((((uint16_t) (FAT2 + a)->entry2[1]) & 0xF0) >> 4) + ((uint16_t)(FAT2 + a)->entry2[2] << 4);
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
    for(int i = 0; i < 16; i++){//不算. and ..
        if(NAME2Str(subEntry[i].NAME) == "." || NAME2Str(subEntry[i].NAME) == ".."){
            continue;
        }
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
    if(url.at(url.length() - 1) == '/'){
        url = url.substr(0, url.length() - 1);
    }
    Entry subEntry[16];//cnm，假设文件夹里最多16个文件
    fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    fread(subEntry, bpb.BytesPerSec, 1, fp);
    printStrString(url + "/");
    printStrString(":\n");
    printDirName(fp, cluster);
    printStrString("\n");
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
            printStrString("  ");
        }
        if(subEntry[i].Attr == 0x10){
            //is Dir
            printRed(NAME2Str(subEntry[i].NAME));
        }else if(subEntry[i].NAME[0] == 0){
            //notFile
        }else{
            //FILE
            // printName(subEntry[i].NAME);
            printStrString(NAME2Str(subEntry[i].NAME));
        }
    }
    //printStrString("\n");
    
}
void printRed(const string src){
    // printStrString("\033[31m");
    // printStrString(src);
    // printStrString("\033[0m");
    printStrString("\033[31m" + src + "\033[0m");
}

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


// void printRootDir(FILE * fp){
//     printDir(fp, 19 - 31, "/");
// }


int findDirEntry(FILE * fp, int clusterSrc, string url){
    if(url == "/"){
        return 19 - 31;
    }
    if(url.at(url.length() - 1) != '/'){
        url = url + "/";
    }
    string nowName = getFirstNameUrl(url);//最前面的一个文件夹。形式是“/kkp” 
    string subUrl = url.substr(nowName.length(), url.length() - nowName.length());
    Entry subEntry[16];//cnm，假设文件夹里最多16个文件
    fseek(fp, (dataSector + clusterSrc * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    fread(subEntry, bpb.BytesPerSec, 1, fp);
    bool isLast = false;
    int clusterRes;
    //如果到最后了，就在找到之后返回，不在递归
    if(subUrl == "/"){
        //子目录中只有头尾有“/”
        isLast = true;
    }
    
    bool haveNowName = false;
    for(int i = 0;i < 16;i++){
         if("/" + NAME2Str(subEntry[i].NAME) == nowName){
            haveNowName = true;
            clusterRes = subEntry[i].FstClust;
            if(!isLast){
                clusterRes = findDirEntry(fp, clusterRes, subUrl);
            }
            break;
         }
    }
    if(!haveNowName){
        clusterRes = -1;
    }
    return clusterRes;
}
string getFirstNameUrl(string url){
    for(int i = 1; i < url.length(); i++){
        if(url.at(i) == '/'){
            return url.substr(0, i);
        }
    }
    return "";
}
void printURL(FILE* fp, string url){
    url = validateURL(url);
    int cluster = findDirEntry(fp, 19 - 31, url);
    if(cluster == -1){
        printStrString("path error.\n");
        return;
    }
    printDir(fp, cluster, url);
}
void printURLL(FILE* fp, string url){
    url = validateURL(url);
    int cluster = findDirEntry(fp, 19 - 31, url);
    if(cluster == -1){
        printStrString("path error.\n");
        return;
    }
    printDirL(fp, cluster, url);
}
void catURL(FILE* fp, string url){
    if(url.at(0) != '/'){
        url = "/" + url;
    }
    string FileName = getFileNameFromURL(url);
    string DirURL = url.substr(0, url.length() - FileName.length());
    int DirCluster = findDirEntry(fp, 19 - 31, DirURL);
    if(DirCluster == -1){
        printStrString("path error.\n");
        return;
    }
    catArc(fp, DirCluster, FileName);
    return;
}
void catArc(FILE * fp, int cluster, string FileName){
    Entry subEntry[16];//cnm，假设文件夹里最多16个文件
    fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    fread(subEntry, bpb.BytesPerSec, 1, fp);
    for(int i = 0; i < 16; i++){
        if(NAME2Str(subEntry[i].NAME) == FileName){
            catFile(fp, subEntry[i].FstClust, subEntry[i].FileSize);
            return;
        }
    }
    printStrString("can't find this file\n");
}
void catFile(FILE * fp, int cluster, int size){
    char buf[513];
    if(size <= bpb.BytesPerSec){
        fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
        fread(buf, size, 1, fp);
        buf[size] = '\0';
        printStrString(buf);
    }else{
        fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
        fread(buf, bpb.BytesPerSec, 1, fp);
        buf[512] = '\0';
        printStrString(buf);
        catFile(fp, *(FAT + cluster), size - bpb.BytesPerSec);
    }
}
string validateURL(string url){
    if(url.at(0) != '/'){
        url = "/" + url;
    }
    if(url.at(url.length() - 1) != '/'){
        url = url + "/";
    }
    return url;
}
void printDirL(FILE * fp, int cluster, string url){
    if(url.at(url.length() - 1) == '/'){
        url = url.substr(0, url.length() - 1);
    }
    Entry subEntry[16];//cnm，假设文件夹里最多16个文件
    fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    fread(subEntry, bpb.BytesPerSec, 1, fp);
    printStrString(url + "/ ");
    int dirNum = 0, arcNum = 0;
    getDirSubNum(fp, cluster, &dirNum, &arcNum);
    printStrString(to_string(dirNum) + " " + to_string(arcNum) + ":\n");
    printDirNameL(fp, cluster);
    //printStrString("\n");
    for(int i = 0; i < 16; i++){

        if(subEntry[i].Attr == 0x10){
            //is Dir
            if(NAME2Str(subEntry[i].NAME) != "." && NAME2Str(subEntry[i].NAME) != ".."){
                printDirL(fp, subEntry[i].FstClust, url + "/" + NAME2Str(subEntry[i].NAME));
            }
        }
    }
}
void printDirNameL(FILE * fp, int cluster){
    Entry subEntry[16];//cnm，假设文件夹里最多16个文件
    fseek(fp, (dataSector + cluster * bpb.SecPerClus) * bpb.BytesPerSec, SEEK_SET);
    fread(subEntry, bpb.BytesPerSec, 1, fp);
    int dirNum = 0, arcNum = 0;
    for(int i = 0; i < 16; i++){
        if(i != 0){
            printStrString("\n");
        }
        if(subEntry[i].Attr == 0x10){
            //is Dir
            printRed(NAME2Str(subEntry[i].NAME));
            if(NAME2Str(subEntry[i].NAME) != "." && NAME2Str(subEntry[i].NAME) != ".."){
                //如果不是这两个，就打印哪个数值
                getDirSubNum(fp, subEntry[i].FstClust, &dirNum, &arcNum);
                printStrString(" " + to_string(dirNum) + " " + to_string(arcNum));
            }
        }else if(subEntry[i].NAME[0] == 0){
            //notFile
            break;
        }else{
            //FILE
            // printName(subEntry[i].NAME);
            printStrString(NAME2Str(subEntry[i].NAME));
            printStrString(" " + to_string(subEntry[i].FileSize));
        }
    }
    printStrString("\n");
}
vector<string> splitStr(string str, string s){
    int start = 0;
    vector<string> result = vector<string>();
    for(int i = 1; i  + s.length() <= str.length(); i++){
        if(str.substr(i, s.length()) == s){
            if(i != start){
                result.push_back(str.substr(start, i - start));
            }
            start = i + s.length();
        }
    }
    if(start != str.length()){
        result.push_back(str.substr(start));
    }
    return result;
}
bool isL(string str){
    if(str.length() < 2){
        return false;
    }
    for(int i = 1; i < str.length(); i++){
        if(str.at(i) != 'l'){
            return false;
        }
    }
    return true;
}
bool isManyL(vector<string> src){
    for(int i = 3; i < src.size(); i++){
        if(!isL(src.at(i))){
            return false;
        }
    }
    return true;
}
string getFileNameFromURL(string url){
    int start = 0;
    for(int i = url.length() - 1; i >=0; i--){
        if(url.at(i) == '/'){
            start = i + 1;
            break;
        }
    }
    return url.substr(start);
}

#ifdef DEBUG
void printStrString(string str){
    cout << str;
}
#else
void printStrString(string str){
    const char* haha = str.c_str();
    
    printStr(haha);
}
#endif