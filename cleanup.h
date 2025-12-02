#ifndef CLEANUP_H
#define CLEANUP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h> // 스레드 사용 시 필요

// 1. [공통] 파일 정보를 담을 구조체 (이걸 주고받습니다)
typedef struct FileInfo {
    char path[1024];      // 파일의 절대 경로 (예: /home/user/down/a.txt)
    char name[256];       // 파일 이름 (예: a.txt)
    long size;            // 파일 크기 (Byte)
    time_t last_access;   // 마지막 접근 시간
    struct FileInfo *next;// 다음 파일 (링크드 리스트)
} FileInfo;

// 2. [A파트 담당] 데이터 수집 함수들
FileInfo* scan_directory(const char *dir_path); // 폴더 스캔
void free_file_list(FileInfo *head);            // 메모리 해제
void print_file_list(FileInfo *head);           // 목록 출력 (테스트용)

// 3. [B파트 담당] 액션 함수들 (이걸 구현해 줘!)
// 파일을 dest_folder로 이동시키는 함수
void move_file(FileInfo *file, const char *dest_folder);

// 리스트에 있는 파일들을 tar로 압축하고 원본 삭제하는 함수
void archive_files(FileInfo *head, const char *archive_name);

// 중복 파일인지 확인하는 함수 (중복이면 1, 아니면 0 리턴)
int check_duplicate(FileInfo *file1, FileInfo *file2);

#endif