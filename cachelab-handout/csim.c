#include "cachelab.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>

int h,v,s,E,b,S;

int hit_count,miss_count,eviction_count;

char t[1024];

typedef struct{
    int valid_bits;
    int tag;
    int stamp;
}cache_line,*cache_asso,**cache;

cache _cache_ = NULL;

void init_cache(){
    _cache_ = (cache)malloc(sizeof(cache_asso) *S);
    for(int i=0;i<S;i++){
        _cache_[i] = (cache_asso)malloc(sizeof(cache_line) * E);
        for(int j =0;j<E;j++){
            _cache_[i][j].valid_bits = 0;
            _cache_[i][j].tag = -1;
            _cache_[i][j].stamp = -1;
        }
    }
}

void update_stamp(){
    for(int i=0;i<S;i++){
        for(int j=0;j<E;j++){
            if(_cache_[i][j].valid_bits == 1){
                ++_cache_[i][j].stamp;
            }
        }
    }
}

void update(unsigned int address){
    int setindex_add = (address >> b) & ((-1U)>>(64-s));
    int tag_add = address >> (b+s);

    int max_stamp = INT_MIN;
    int max_stamp_index = -1;
    
    //check if address is loaded before
    for(int i=0;i<E;i++){
        if(_cache_[setindex_add][i].tag == tag_add){
            _cache_[setindex_add][i].stamp = 0;
            ++hit_count;
            return;
        }
    }
    
    //check if there is a empty line
    for(int i=0;i<E;i++){
        if(_cache_[setindex_add][i].valid_bits == 0){
            _cache_[setindex_add][i].valid_bits = 1;
            _cache_[setindex_add][i].tag = tag_add;
            _cache_[setindex_add][i].stamp = 0;
            ++miss_count;
            return;
        }
    }
    
    //need replace the first line
    ++eviction_count;
    ++miss_count;
    
    for(int i=0;i<E;i++){
        if(_cache_[setindex_add][i].stamp > max_stamp){
            max_stamp = _cache_[setindex_add][i].stamp;
            max_stamp_index = i;
        }
    }
    _cache_[setindex_add][max_stamp_index].tag = tag_add;
    _cache_[setindex_add][max_stamp_index].stamp = 0;
    return;
}

void parse_trace(){
    FILE* fp = fopen(t,"r");
    if(fp == NULL){
        printf("open error");
        exit(-1);
    }

    char operation;
    unsigned int address;
    int size;
    
    while(fscanf(fp," %c %xu,%d\n",&operation,&address,&size) > 0){
        switch(operation){
            case 'I':
                continue;
            case 'L':
                update(address);
                break;
            case 'M':
                update(address);
            case 'S':
                update(address);
        } 
        update_stamp();
    }

    fclose(fp);
    for(int i=0;i<S;i++){
        free(_cache_[i]);
    }
    free(_cache_);
}

int main(int argc,char* argv[])
{
    int opt = 0;
    while(-1!=(opt = (getopt(argc,argv,"hvs:E:b:t:")))){
        switch(opt){
            case 'h':
                h = 1;
                break;
            case 'v':
                v = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                strcpy(t,optarg);
                break;
            default:
                break;
        } 
    }
    
    if(s<=0 || E<=0 || b<=0 || strlen(t)==0){
        return -1;
    }
    
    S = 1<<s;
    
    FILE* fp = fopen(t,"r");
    if(fp==NULL){
        printf("open error");
        exit(-1);
    }

    init_cache();
    parse_trace();

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
