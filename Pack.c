#include<stdio.h>
#include<string.h>
#include<malloc.h>

#define FILE_TYPE_LEN 10
#define MAX_FILE_TLV_SIZE 10
#define BUFFER_SIZE 10*1024*1024
/* 文件信息，包含文件名和文件内容两部分 */
#define FILE_INFO_NUM 2
#define WRITE_FILE_PATH "d:\\output\\output.dat"


#define U32 unsigned int
#define U64 unsigned long int
#define S32 int
#define U8 char
char *typeEmun[] = {
    "exe",
    "jpg",
    "mp3",
    "txt"
};
U32 typeEmunNum = 4;

typedef struct fileInfo
{
    U32 fileInfoType;
    U32 fileInfoLen;
    U8 *fileInfoValue;
}FileInfo;

typedef struct fileTlv
{
    U32 fileType;
    U32 fileLen;
    FileInfo *fileValue;
}FileTlv;

U32 GetFileType(U8 *filePath, U32 fileNameSize, U32 *fileType){
    U8 *pType;
    U8 i;
    U32 j;
    for (i = 0; i < fileNameSize; i++) {
        if(filePath[i] == '.') {
            pType = &filePath[i+1];
            break;
        }
    }
    for (j = 0; j < typeEmunNum; j++) {
        if (strcmp(pType, typeEmun[j]) == 0) {
            *fileType = j;
            break;
        }
    }
    return 0;
}

U32 FilesInfoShow(FileTlv *fileTlvs, U32 *fileNum) {
    int fileIndex;
    int fileInfoIndex;
    U32 fileValueIndex;
    
    for (fileIndex = 0; fileIndex < *fileNum; fileIndex++) {
        printf("fileType : %d\tfileLen : %d\n", fileTlvs[fileIndex].fileType, fileTlvs[fileIndex].fileLen);
        for (fileInfoIndex = 0; fileInfoIndex < FILE_INFO_NUM; fileInfoIndex++) {
            printf("fileInfoType : %d\tfileInfoLen : %d\n", fileTlvs[fileIndex].fileValue[fileInfoIndex].fileInfoType,
                                                             fileTlvs[fileIndex].fileValue[fileInfoIndex].fileInfoLen);
            for (fileValueIndex = 0; fileValueIndex < fileTlvs[fileIndex].fileValue[fileInfoIndex].fileInfoLen; fileValueIndex++) {
                printf("fileInfoValue : %c\n", fileTlvs[fileIndex].fileValue[fileInfoIndex].fileInfoValue[fileValueIndex]);
            }
        }
    }
}

U32 PhaseFile(U8 *filePath, U32 fileNameSize, FileTlv *fileTlvs, U32 *fileNum) {

    U8 *fileBuffer = (U8 *)malloc(BUFFER_SIZE);
    FileInfo *fileInfo = (FileInfo *) malloc(sizeof(FileInfo) * 2);
    U32 fileType;
    U32 fileLen = 0;
    
    GetFileType(filePath, fileNameSize, &fileType);
    
    /* 填充文件名 */
    fileInfo[0].fileInfoType = 0;
    fileInfo[0].fileInfoLen = fileNameSize;
    fileInfo[0].fileInfoValue = filePath;

    /* 填充文件信息 */
    fileInfo[1].fileInfoType = 1;
    //读文件到缓冲区
    FILE *fileRead = fopen(filePath,"r+");
    U8 *pFile=fileBuffer;
    while (!feof(fileRead))
    {
        *pFile = fgetc(fileRead);
        pFile++;
        fileLen++;
    }
    pFile = NULL;
    fclose(fileRead);
    fileInfo[1].fileInfoLen = fileLen;
    fileInfo[1].fileInfoValue = fileBuffer;
    
    fileTlvs[*fileNum].fileType = fileType;
    fileTlvs[*fileNum].fileLen = 4 * sizeof(U32) + fileInfo[0].fileInfoLen + fileInfo[0].fileInfoLen;
    fileTlvs[*fileNum].fileValue =  fileInfo;
    *fileNum = *fileNum + 1;
    return 0;
}

U32 PackBuffer(FileTlv *fileTlvs, U32 *fileNum) {
    U32 packSize = 0;
    U32 *pointU32;
    U8 *pointCurrent;
    U32 fileIndex = 0;
    int fileInfoIndex = 0;
    U32 fileValueIndex = 0;

    packSize += (*fileNum) * (2 * sizeof(U32));
    for(fileIndex = 0; fileIndex < *fileNum; fileIndex++) {
        packSize += fileTlvs[fileIndex].fileLen;
    }
    U8 *fileHead = (U8 *) malloc(sizeof(U8) * packSize + sizeof(U32));
    printf("Totle = %d\n",sizeof(U8) * packSize + sizeof(U32));
    pointCurrent = fileHead;

    pointU32 = (U32 *) (void *)pointCurrent;
    *pointU32 = packSize;
    pointU32 ++;
    pointCurrent = (U8 *) (void *)pointU32;

    /* 第一层，pck的所有信息，包含所有文件 */
    for(fileIndex = 0; fileIndex < *fileNum; fileIndex++) {
        U32 fileLen = fileTlvs[fileIndex].fileLen;
        U32 fileType = fileTlvs[fileIndex].fileType;

        pointU32 = (U32 *) (void *)pointCurrent;
        *pointU32 = fileType;
        pointU32 ++;
        *pointU32 = fileLen;
        pointU32 ++;
        pointCurrent = (U8 *) (void *)pointU32;

        /* 第二层，单个文件的所有信息，包含文件名和文件内容 */
        for (fileInfoIndex = 0; fileInfoIndex < FILE_INFO_NUM; fileInfoIndex++) {

            pointU32 = (U32 *) (void *)pointCurrent;
            *pointU32 = fileTlvs[fileIndex].fileValue[fileInfoIndex].fileInfoType;
            pointU32 ++;
            *pointU32 = fileTlvs[fileIndex].fileValue[fileInfoIndex].fileInfoLen;
            pointU32 ++;
            pointCurrent = (U8 *) (void *)pointU32;

            /* 第三层，单个文件的单个信息，如文件名信息或文件内容信息 */
            for (fileValueIndex = 0; fileValueIndex < fileTlvs[fileIndex].fileValue[fileInfoIndex].fileInfoLen; fileValueIndex++) {
                *pointCurrent = fileTlvs[fileIndex].fileValue[fileInfoIndex].fileInfoValue[fileValueIndex];
                pointCurrent++;
            }
        }
    }
    

    FILE *fileWrite = fopen(WRITE_FILE_PATH,"w");
    //写文件类型
    fwrite(fileHead,sizeof(U8),sizeof(U8) * packSize + sizeof(U32), fileWrite);
    // //写文件名长度
    // fwrite(&fileNameSize,sizeof(U32),1, fileWrite);
    // //写文件名
    // fwrite(filePath,sizeof(U8),fileNameSize, fileWrite);

    // //写文件内容长度
    // fwrite(&fileLen,sizeof(U32),1, fileWrite);
    // //写文件
    // fwrite(fileBuffer,sizeof(U8),fileLen, fileWrite);
    fclose(fileWrite);
}

U32 UnPackFile(U8 *filePath, void *fileBuffer, U64 *fileSize) {

    FILE *fileRead = fopen(filePath,"r");
    U32 A = 99;
    fseek(fileRead, 0, SEEK_END);       
	U64 tempFileSize = ftell (fileRead);

    fseek(fileRead, 0, SEEK_SET);
    void *tempFileBuffer = (void *)malloc(sizeof(U8) * tempFileSize);
    fread(tempFileBuffer, sizeof(U8), tempFileSize, fileRead);

    //A =* (U32 *)tempFileBuffer;
    //printf("%d" ,tempFileBuffer);
    //fileBuffer = tempFileBuffer;
    
    printf("fileSize = %d\n", A);
    *fileSize = tempFileSize;
    return 0;
}


int main(){
    U8 *fileBuffer;
    U64 fileSize = 0;
    U32 *type;
    U32 fileNum = 0;
    U8 path1[] = "d:\\cc.txt";
    U8 path2[] = "d:\\d.txt";
    U8 path3[] = "d:\\map.mp3";
    /* 最多保存10个文件 */
    FileTlv *fileTlvs = (FileTlv *)malloc(sizeof(FileTlv) * MAX_FILE_TLV_SIZE);

    PhaseFile(path1, sizeof(path1)/sizeof(U8), fileTlvs, &fileNum);
    //PhaseFile(path2, sizeof(path2)/sizeof(U8), fileTlvs, &fileNum);
    PhaseFile(path3, sizeof(path3)/sizeof(U8), fileTlvs, &fileNum);
    FilesInfoShow(fileTlvs, &fileNum);
    PackBuffer(fileTlvs, &fileNum);
    //UnPackFile("D:\\output\\output.dat", fileBuffer, &fileSize);
    //type = (U32 *) fileBuffer;

    //printf("fileSize = %d", type);
    system("pause");

}