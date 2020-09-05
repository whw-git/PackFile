#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int judgeStr(char * s,char * result,int star,int end){    /* 判断指定位置的子串是否回文 */
    for (int i = 0; i < (end-star)/2+1; i++){
        if (s[star+i]!=s[end-i]){
            return 0;
        }
    }
    for (int i = star; i <= end; i++){
        result[i-star]=s[i];
    }
    return 1;
}

char * longestPalindrome(char * s){
    int strLen= strlen(s);
    char *result=(char *)malloc(sizeof(char)*strLen+1);
    memset(result,'\0',sizeof(char)*strLen+1);
    for (int subStr = strLen; subStr > 1; subStr--){    /* 子串长度 */
        for (int star = 0; star <= strLen-subStr; star++){    /* 起始地址 */
            if(judgeStr(s,result,star,star+subStr-1))
                return result;
        }
    }
    result[0]=s[0];     /* 没有 */
    return result;
}

int main(){
    char *s = "cbbd";
    printf("%s\n" ,longestPalindrome(s));
    system("pause");
}