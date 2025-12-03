#define _DEFAULT_SOURCE 
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include "cleanup.h"

// 1. 현재 시간을 문자열로 반환하는 내부 함수
void get_current_time_str(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// 2. 로그 파일(cleanup.log)에 내용을 적는 함수
void write_log(const char *format, ...) {
    FILE *fp = fopen("cleanup.log", "a");
    if (fp == NULL) {
        perror("로그 파일 열기 실패");
        return;
    }

    char time_str[64];
    get_current_time_str(time_str, sizeof(time_str));

    // [시간] 내용 형식으로 작성
    fprintf(fp, "[%s] ", time_str);

    va_list args;
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);

    fprintf(fp, "\n");
    fclose(fp);
}

// 3. [NEW] GUI로 폴더 선택 창을 띄우는 함수
// 성공하면 1, 취소하거나 실패하면 0 반환
int pick_folder_with_gui(char *buffer, size_t size) {
    printf(">> 📂 윈도우 다운로드 폴더를 찾는 중...\n");

    char win_user[256] = {0};
    
    // 윈도우 CMD를 호출하여 사용자 이름 추출
    FILE *name_fp = popen("cmd.exe /c echo %USERNAME% 2>/dev/null", "r");
    
    if (name_fp) {
        if (fgets(win_user, sizeof(win_user), name_fp) != NULL) {
            // 윈도우는 줄바꿈이 \r\n 이라 둘 다 제거
            win_user[strcspn(win_user, "\r\n")] = 0; 
        }
        pclose(name_fp);
    }

    // 이름을 못 찾았을 경우 기본값 설정
    if (strlen(win_user) == 0) {
        printf(">> ⚠️ 윈도우 사용자 이름을 찾지 못했습니다. 기본 경로로 엽니다.\n");
        strcpy(win_user, "Public"); // 비상시 공용 폴더로
    } else {
        printf(">> ✅ 감지된 윈도우 사용자: %s\n", win_user);
    }

    // 3. zenity 실행 (자동으로 찾은 이름 사용)
    char command[1024];
    snprintf(command, sizeof(command), 
             "zenity --file-selection --directory --title='다운로드 폴더 정리' --filename='/mnt/c/Users/%s/Downloads/' 2>/dev/null", 
             win_user);

    printf(">> 📂 폴더 선택 창을 띄웁니다... (화면을 확인하세요)\n");
    FILE *fp = popen(command, "r");
    
    if (fp == NULL) {
        printf(">> ⚠️ GUI 창을 띄울 수 없습니다. (zenity 설치 필요)\n");
        return 0;
    }

    if (fgets(buffer, size, fp) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        pclose(fp);
        return 1;
    }

    pclose(fp);
    return 0; 
}

int mkdir_p(const char *path) {
    char temp[MAX_PATH];
    char *p = NULL;
    size_t len;

    snprintf(temp, sizeof(temp), "%s", path);
    len = strlen(temp);

    if (temp[len - 1] == '/')
        temp[len - 1] = 0;

    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(temp, 0755) != 0) {
                if (errno != EEXIST) return -1;
            }
            *p = '/';
        }
    }
    if (mkdir(temp, 0755) != 0) {
        if (errno != EEXIST) return -1;
    }
    return 0;
}
