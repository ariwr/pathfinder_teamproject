/* utils.c : 로그, GUI, 폴더 생성 도구 */

#define _DEFAULT_SOURCE 
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "cleanup.h"

volatile int is_loading = 0;

// 현재 시간 문자열 반환 함수
void get_current_time_str(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// 로그 파일에 기록 함수
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

// Zenity GUI로 폴더 선택 (윈도우 사용자 자동 감지)
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

    char command[1024];
    // Zenity 실행 명령어 (에러 메시지 숨김)
    snprintf(command, sizeof(command), 
             "zenity --file-selection --directory --title='다운로드 폴더 정리' --filename='/mnt/c/Users/%s/Downloads/' 2>/dev/null", 
             win_user);

    printf(">> 📂 폴더 선택 창을 띄웁니다...\n");
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

// 재귀적 폴더 생성 (mkdir -p 기능)
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

// 로딩 애니메이션을 담당할 작업자 스레드
void *loading_spinner(void *arg) {
    // 회전하는 막대기 모양
    const char spinner[] = {'|', '/', '-', '\\'};
    int i = 0;
    
    // is_loading이 1인 동안 계속 돌아감
    while (is_loading) {
        printf("\r[ 작업 진행 중... %c ]   ", spinner[i % 4]);
        fflush(stdout); // 화면에 즉시 표시
        usleep(100000);
        i++;
    }
    return NULL;
}

// 스레드 시작 함수
pthread_t start_loading_thread() {
    is_loading = 1;
    pthread_t thread_id;
    // 스레드 생성 (성공 시 0 반환)
    if (pthread_create(&thread_id, NULL, loading_spinner, NULL) != 0) {
        perror("스레드 생성 실패");
        is_loading = 0;
    }
    return thread_id;
}

// 스레드 종료 함수
void stop_loading_thread(pthread_t thread_id) {
    if (is_loading) {
        is_loading = 0; // 루프 종료 신호
        pthread_join(thread_id, NULL); // 스레드가 끝날 때까지 대기
        printf("\r%60s\r", " ");
    }
}
