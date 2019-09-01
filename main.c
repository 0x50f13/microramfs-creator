//
//  main.cpp
//  microramfs
//
//  Created by Andre Zay on 14/08/2019.
//  Copyright © 2019 Andre Zay. All rights reserved.
//

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>


#define CMD_DIR  0x01
#define CMD_FILE 0x02

uint64_t gsz=0;
uint64_t files=0;
uint64_t dirs=0;
void _add_str(uint8_t *str, size_t sz){
    size_t i=0;
    for(;i<sz;++i){
        if(gsz%16==0){
            printf("\n");
        }
        printf("0x%02x,",str[i]);
        ++gsz;
    }
}
void _add_sz(size_t sz){
    uint64_t _sz=sizeof(size_t)/sizeof(uint8_t);
    uint8_t *sza=(uint8_t *)malloc(sizeof(size_t));
    memcpy(sza, &sz, sizeof(size_t));
    uint64_t i=0;
    for(;i<_sz;++i){
        if(gsz%16==0){
            printf("\n");
        }
        printf("0x%02x,",sza[i]);
        ++gsz;
    }
}
void add_dir(char *dirname){
    if(gsz%16==0){
        printf("\n");
    }
    printf("0x%02x,",CMD_DIR);
    ++gsz;
    uint64_t sz=(strlen(dirname)+1)*sizeof(char);
    _add_sz(sz);
    _add_str((uint8_t*)dirname,sz);
    ++dirs;
}

void add_file(char *path,uint8_t *content, size_t sz_c){
    if(gsz%16==0){
        printf("\n");
    }
    printf("0x%02x,",CMD_FILE);
    ++gsz;
    uint64_t sz_p=(strlen(path)+1)*sizeof(char);
    _add_sz(sz_p);
    _add_str((uint8_t*)path,sz_p);
    _add_sz(sz_c);
    _add_str((uint8_t*)content,sz_c+sizeof(char));
    ++files;
}

void listdir(const char *name)
{
    DIR *dir;
    struct dirent *entry;
    
    if (!(dir = opendir(name)))
        return;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            if(!strcmp(entry->d_name, ".")){
                ++name;
            }
            char *_name=(char *)malloc(sizeof(char)*(strlen(entry->d_name)+strlen(name)+2));
            char *__name=_name;
            strcpy(_name, name);
            strcat(_name,"/");
            strcat(_name, entry->d_name);
            if(!strcmp(_name,"./")){
                strcpy(_name,"/");
            }else{
                _name+=2*sizeof(char);
            }
            //printf("Creating directory %s\n",_name);
            add_dir(_name);
            listdir(path);
            free(__name);
        } else {
            if(!strcmp(entry->d_name, ".DS_Store")){
                continue;
            }
            if(!strcmp(entry->d_name, ".")){
                ++name;
            }
            char * buffer = 0;
            long length;
            char *_name=(char *)malloc(sizeof(char)*(strlen(entry->d_name)+strlen(name)+2));
            strcpy(_name, name);
            strcat(_name, "/");
            strcat(_name, entry->d_name);
            FILE * f = fopen (_name, "rb");
            if(!strcmp(_name,"./")){
                strcpy(_name,"/");
            }else{
                _name+=2*sizeof(char);
            }

            if (f)
            {
                fseek (f, 0, SEEK_END);
                length = ftell (f);
                fseek (f, 0, SEEK_SET);
                buffer = (char*)malloc (length);
                if (buffer)
                {
                    fread (buffer, 1, length, f);
                }
                fclose (f);
            }else{
                printf("%s(%d)\n",strerror(errno),errno);
                continue;
            }
            
            assert(buffer);
            //buffer[length]=0;
            add_file(_name, (uint8_t *)buffer,length);
        }
    }
    closedir(dir);
}



int main(int argc, const char * argv[]) {
    if(argc<2){
        printf("Usage: %s [PATH TO RAMFS DIR]\n",argv[0]);
        return -1;
    }
    chdir(argv[1]);
    printf("{");
    listdir("./");
    printf("}\nTotal size: %llu bytes\nFiles count: %llu\n Dirs count:%llu\n",gsz,files,dirs);
    /*size_t i=0;
    while (i<initramfs_sz) {
        if(init_img[i]==CMD_FILE){
            ++i;
            uint8_t *name=NULL;
            uint8_t *content=NULL;
            uint64_t name_size=0;
            memcpy(&name_size, init_img+i, sizeof(uint64_t));
            printf("Name size: %llu\n",name_size);
            name=(uint8_t *)malloc(name_size*sizeof(uint8_t));
            i+=sizeof(uint64_t);
            memcpy((uint8_t *)name, init_img+i, name_size*sizeof(uint8_t));
            i+=name_size;
            uint64_t file_size=0;
            memcpy(&file_size, init_img+i, sizeof(uint64_t));
            printf("File size: %llu\n",file_size);
            i+=sizeof(uint64_t);
            content=(uint8_t *)malloc((file_size+1)*sizeof(uint8_t));
            memcpy((uint8_t *)content, init_img+i, file_size*sizeof(uint8_t));
            content[file_size]=0;
            printf("%s\n---\n%s\n",name,content);
            i+=file_size;
        }else if(init_img[i]==CMD_DIR){
            ++i;
            uint8_t *name=NULL;
            uint64_t name_size=0;
            memcpy(&name_size, init_img+i, sizeof(uint64_t));
            printf("Dir name size: %llu\n",name_size);
            name=(uint8_t *)malloc(name_size*sizeof(uint8_t));
            i+=sizeof(uint64_t);
            //++i;
            memcpy((uint8_t *)name, init_img+i, name_size*sizeof(uint8_t));
            printf("Dir name: %s\n",(char*)name);
            i+=name_size-1;
            size_t j=strlen((char *)name);
            while(j>0){
                j--;
                if(name[j]=='/'){
                    break;
                }
            }
            char *dirname=(char *)malloc(sizeof(char)*(strlen((char *)name)-j+1));
            strcpy(dirname, (char*)(name+j+1));
            printf("DIR:%s\n",dirname);
            name[j+1]=0;
            printf("BASE:%s\n",name);
        }else{
            assert(false);
        }
        ++i;
    }*/
    return 0;
}
