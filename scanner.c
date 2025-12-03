#define _DEFAULT_SOURCE // 리눅스 확장 기능 사용 시 필요 (혹시 모를 호환성 대비)
#include "cleanup.h"

// 1. 새로운 노드를 생성하는 도우미 함수
FileInfo* create_node(const char *dir_path, const char *name) {
    FileInfo *new_node = (FileInfo *)malloc(sizeof(FileInfo));
    if (!new_node) {
        perror("Memory allocation failed");
        return NULL;
    }

    // 경로와 이름 저장
    snprintf(new_node->path, sizeof(new_node->path), "%s/%s", dir_path, name);
    strncpy(new_node->name, name, sizeof(new_node->name));
    new_node->next = NULL;

    // stat 시스템 콜을 이용해 파일 상세 정보 가져오기
    struct stat file_stat;
    if (stat(new_node->path, &file_stat) == 0) {
        new_node->size = file_stat.st_size;
        new_node->last_access = file_stat.st_atime; // 최근 접근 시간
    } else {
        new_node->size = -1; // 에러 표시
    }

    return new_node;
}

// 2. 디렉토리를 스캔하여 리스트로 반환하는 메인 함수
FileInfo* scan_directory(const char *dir_path) {
    DIR *d;
    struct dirent *dir;
    struct stat st; // [수정] 파일 정보를 담을 구조체 선언
    FileInfo *head = NULL;
    FileInfo *current = NULL;

    d = opendir(dir_path); // 1. 디렉토리 열기
    if (!d) {
        perror("Unable to open directory");
        return NULL;
    }

    // (선택) 디버깅용: 현재 어디를 스캔 중인지 출력
    // printf("[스캔 중] %s\n", dir_path);

    // 2. 디렉토리 내부를 하나씩 읽기 (Loop)
    while ((dir = readdir(d)) != NULL) {
        // ".", ".." 같은 숨김 폴더는 건너뛰기
        if (dir->d_name[0] == '.') continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, dir->d_name);

        // [문제 해결] DT_DIR 대신 stat()과 S_ISDIR() 사용
        // stat()을 사용하여 파일의 속성을 정확하게 파악합니다.
        if (stat(full_path, &st) == -1) {
            continue; // 파일 정보를 읽지 못하면 건너뜀
        }

        // S_ISDIR 매크로: 파일 모드(st_mode)를 확인하여 디렉토리인지 판별 (표준 방법)
        if (S_ISDIR(st.st_mode)) {
            // 폴더를 만나면 무시
            continue;
        } 
        // 일반 파일인 경우에 리스트 추가
        FileInfo *new_node = create_node(dir_path, dir->d_name);
            
        if (head == NULL) {
            head = new_node;
            current = new_node;
        } else {
            current->next = new_node;
            current = new_node; // current를 항상 마지막 노드로 유지
        }
    }
    
    closedir(d); // 3. 디렉토리 닫기
    return head; // 리스트의 머리(head) 반환
}

// 3. 메모리 해제 함수
void free_file_list(FileInfo *head) {
    FileInfo *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// 4. 테스트용 출력 함수
void print_file_list(FileInfo *head) {
    FileInfo *current = head;
    int count = 0;
    printf("\n--- 전체 파일 스캔 결과 ---\n");
    while (current != NULL) {
        printf("[%d] %-30s | %ld bytes\n", 
               ++count, current->path, current->size); // path를 출력해서 경로 확인
        current = current->next;
    }
    printf("총 %d개의 파일이 검색되었습니다.\n", count);
}
