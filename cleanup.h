/* cleanup.h : 구조체 및 함수 선언 */

#ifndef CLEANUP_H
#define CLEANUP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

// 상수 정의
#define MAX_PATH 1024        // 파일 경로 최대 길이
#define MAX_NAME 256         // 파일 이름 최대 길이
#define FILE_BUFFER 4096     // 파일 읽기/쓰기용 버퍼 크기
#define CMD_BUFFER 2048      // 시스템 명령어용 버퍼 크기

// 파일 정보를 담을 구조체 (연결 리스트)
typedef struct FileInfo {
    char path[1024];      // 절대 경로
    char name[256];       // 파일 이름
    long size;            // 파일 크기 (Byte)
    time_t last_access;   // 마지막 접근 시간
    struct FileInfo *next;// 다음 파일
} FileInfo;

// --- [Scanner] scanner.c ---
FileInfo* scan_directory(const char *dir_path);
void free_file_list(FileInfo *head);
void print_file_list(FileInfo *head);

// --- [Actions] actions.c ---
void move_file(FileInfo *file, const char *dest_folder);   
int check_duplicate(FileInfo *file1, FileInfo *file2);
void remove_copy_files(FileInfo *head);
void classify_files_by_extension(FileInfo *head, const char *base_dest_folder);
void archive_files(FileInfo *head, int days);

// 원클릭 전체 정리 함수
void run_full_cleanup(FileInfo *head, int days, const char *final_dest);

// --- [Utils] utils.c ---
void write_log(const char *format, ...);
int pick_folder_with_gui(char *buffer, size_t size);
int mkdir_p(const char *path);

// 스레드 관련 함수
pthread_t start_loading_thread();
void stop_loading_thread(pthread_t thread_id);

#endif
