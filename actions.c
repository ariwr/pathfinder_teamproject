/* actions.c : 파일 정리, 이동, 압축, 삭제 구현 */

#include "cleanup.h"
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>

// 파일 확장자 추출
const char* get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return ""; // 확장자 없음
    return dot + 1;
}

// 확장자별 카테고리 결정
void get_category_name(const char *filename, char *category_buf) {
    const char *ext = get_filename_ext(filename);
    
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "png") == 0 || 
        strcasecmp(ext, "jpeg") == 0 || strcasecmp(ext, "gif") == 0) {
        strcpy(category_buf, "Images");
    }
    else if (strcasecmp(ext, "mp4") == 0 || strcasecmp(ext, "avi") == 0 || 
             strcasecmp(ext, "mov") == 0 || strcasecmp(ext, "mkv") == 0 ||
             strcasecmp(ext, "mp3") == 0 || strcasecmp(ext, "wav") == 0) {
        strcpy(category_buf, "Media");
    }
    else if (strcasecmp(ext, "pdf") == 0 || strcasecmp(ext, "doc") == 0 || 
             strcasecmp(ext, "docx") == 0 || strcasecmp(ext, "ppt") == 0 || 
             strcasecmp(ext, "pptx") == 0 || strcasecmp(ext, "txt") == 0 ||
             strcasecmp(ext, "hwp") == 0 || strcasecmp(ext, "xls") == 0) {
        strcpy(category_buf, "Documents");
    }
    else if (strcasecmp(ext, "c") == 0 || strcasecmp(ext, "cpp") == 0 || 
             strcasecmp(ext, "h") == 0 || strcasecmp(ext, "py") == 0 || 
             strcasecmp(ext, "java") == 0 || strcasecmp(ext, "js") == 0 ||
             strcasecmp(ext, "html") == 0 || strcasecmp(ext, "css") == 0) {
        strcpy(category_buf, "Codes");
    }
    else if (strcasecmp(ext, "zip") == 0 || strcasecmp(ext, "tar") == 0 || 
             strcasecmp(ext, "gz") == 0 || strcasecmp(ext, "7z") == 0) {
        strcpy(category_buf, "Archives");
    }
    else if (strcasecmp(ext, "exe") == 0 || strcasecmp(ext, "msi") == 0 || 
             strcasecmp(ext, "sh") == 0) {
        strcpy(category_buf, "Programs");
    }
    else {
        strcpy(category_buf, "Others");
    }
}

//파일 이동 함수
void move_file(FileInfo *file, const char *dest_folder){
    //목표 경로 문자열 생성
    char dest_path[MAX_PATH];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_folder, file->name);

    //파일 이동 
    if(rename(file->path, dest_path) == 0){
        // printf("[이동 완료] %s -> %s\n", file->name, dest_folder);
        write_log("파일 이동: %s -> %s\n", file->name, dest_path);

        strcpy(file->path, dest_path);  //경로 정보 업데이트
    } else {
        perror("\033[2K\r⚠️ [이동 실패]");
        write_log("이동 실패: %s (Error: %s)", file->name, strerror(errno));
    }
}

// 파일 내용 비교 (중복 검사)
int check_duplicate(FileInfo *file1, FileInfo *file2){
    //파일 크기 비교
    if(file1 -> size != file2 -> size){
        return 0;   //다름
    }

    //파일 내용 비교
    FILE *f1 = fopen(file1->path, "rb");
    FILE *f2 = fopen(file2->path, "rb");
    if(!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }

    int same = 1;   //같다고 가정
    char buf1[FILE_BUFFER], buf2[FILE_BUFFER];
    size_t n1, n2;
    
    while((n1 = fread(buf1, 1, sizeof(buf1), f1)) > 0){
        n2 = fread(buf2, 1, sizeof(buf2), f2);
        //길이, 메모리 비교
        if(n1 != n2 || memcmp(buf1, buf2, n1) != 0){
            same = 0; //다르면 루프 탈출
            break;
        }
    }

    fclose(f1);
    fclose(f2);
    return same;
}

// 중복 파일 원본 이름 추출 함수
int get_original_name_from_copy(const char *copy_name, char *original_name_buf) {
    //확장자 찾기(뒤에서부터 . 찾기)
    const char *ext = strrchr(copy_name, '.');
    if (!ext) return 0; // 확장자 없으면 패스

    //확장자 바로 앞이 ')' 인지 확인
    if (ext == copy_name || *(ext - 1) != ')') 
        return 0;

    //괄호 열기 '(' 찾기 (뒤쪽으로 탐색)
    const char *p = ext - 2;
    while (p > copy_name && isdigit(*p)) { // 숫자 건너뛰기
        p--;
    }

    //숫자가 끝나고 '(', 그 앞은 공백" ("
    if (*p != '(' || p == copy_name || *(p - 1) != ' ') 
        return 0;

    //중복파일의 " (숫자)" 부분을 뺀 이름 생성
    // 앞부분 복사 (공백 전까지)
    int len = (p - 1) - copy_name;
    strncpy(original_name_buf, copy_name, len);
    original_name_buf[len] = '\0';
    
    // 확장자 붙이기
    strcat(original_name_buf, ext);
    
    return 1; // 성공
}


// 중복 파일 삭제 함수
void remove_copy_files(FileInfo *head) {
    if (head == NULL) {
        printf(">> 파일 리스트가 비어있습니다.\n");
        return;
    }

    pthread_t tid = start_loading_thread();

    int removed_count = 0;
    FileInfo *curr = head;

    while (curr != NULL) {
        char original_name[MAX_NAME];

        //현재 파일이 복사본 패턴인지 검사 (1), (2)
        if (get_original_name_from_copy(curr->name, original_name)) {
            // 복사본이라면 리스트 안에 '원본 이름'을 가진 파일이 있는지 검색
            FileInfo *finder = head;
            FileInfo *original_file = NULL;

            while (finder != NULL) {
                if (strcmp(finder->name, original_name) == 0) {
                    original_file = finder;
                    break;
                }
                finder = finder->next;
            }
            // 원본 파일을 찾았다면 -> 내용 비교
            if (original_file != NULL) {
                if (check_duplicate(curr, original_file)) {
                    //내용 같다면 복사본 삭제
                    if (unlink(curr->path) == 0) { // unlink는 파일 삭제 시스템 콜
                        printf("\033[2K\r[중복 삭제] %s\n", curr->name);
                        write_log("중복 삭제: %s (원본: %s)", curr->name, original_name); // 로그 기록
                        removed_count++;
                    } else {
                        printf("\033[2K\r");
                        perror("⚠️[삭제 실패]");
                    }
                } else {
                    printf("\033[2K\r⚠️[JUMP]: %s 파일명은 복사본 같지만, 원본과 내용이 다릅니다.\n", curr->name);
                }
            }
        }
        curr = curr->next;
    }
    stop_loading_thread(tid);

    printf("\033[2K\r✅ %d개의 중복 파일 삭제 완료.\n", removed_count);
    write_log("중복 정리 완료: %d개 삭제됨", removed_count);
}


// 오래된 파일 압축 함수
void archive_files(FileInfo *head, int days){
    if (head == NULL) {
        printf(">> 파일 리스트가 비어있습니다.\n");
        return;
    }

    //기준 시간 계산
    time_t now = time(NULL);
    double limit_seconds = days * 86400.0;

    // 임시 폴더 생성
    char temp_dir[] = "temp_archive_zone";
    if (mkdir_p(temp_dir) == -1) { 
        perror("임시 폴더 생성 실패");
        write_log("에러: 아카이브 임시 폴더 생성 실패");
        return;
    }
    pthread_t tid = start_loading_thread();
    
    printf("\033[2K\r>> %d일 동안 접근하지 않은 파일 압축 중...\n", days);
    
    int count = 0;
    FileInfo *curr = head;

    // 날짜 범위 기록
    time_t min_time = 0;
    time_t max_time = 0;
    int first_entry = 1; // 첫 번째 파일인지 확인용 플래그

    while (curr != NULL){
        // 시간 차이 계산
        double diff = difftime(now, curr->last_access);

        // 기준보다 오래되었다면 이동
        if (diff >= limit_seconds) {

            //최대/최소 시간 갱신
            if (first_entry) {
                min_time = curr->last_access;
                max_time = curr->last_access;
                first_entry = 0;
            } else {
                if (curr->last_access < min_time) min_time = curr->last_access;
                if (curr->last_access > max_time) max_time = curr->last_access;
            }

            printf("\033[2K\r[오래된 파일 발견] %s (%.1f일 전 접근)\n", curr->name, diff / 86400.0);
            
            // 확장자별 폴더로 분류해서 이동
            char cat[64];
            get_category_name(curr->name, cat);
            char target_folder[MAX_PATH];
            snprintf(target_folder, sizeof(target_folder), "%s/%s", temp_dir, cat);
            mkdir_p(target_folder); // 폴더 생성

            // 파일 이동
            move_file(curr, target_folder);
            count++;
        }
        curr = curr->next;
    }

    if (count == 0) {
        stop_loading_thread(tid);
        printf("\033[2K\r❌ 아카이브 대상 파일이 없습니다.\n");
        // 빈 폴더 삭제
        rmdir(temp_dir);
        return;
    }

    // 압축파일명 생성 (예: 20230101-20231231.tar) -------------------------
    char min_str[20], max_str[20];
    char auto_name[MAX_NAME];

    //최소시간
    struct tm *tm_min = localtime(&min_time);
    strftime(min_str, sizeof(min_str), "%Y%m%d", tm_min);
    //최대시간
    struct tm *tm_max = localtime(&max_time);
    strftime(max_str, sizeof(max_str), "%Y%m%d", tm_max);

    // 최종 파일명 결정
    snprintf(auto_name, sizeof(auto_name), "%s-%s_archive.tar", min_str, max_str);

    printf(">> 📂 압축 파일명: [ %s ]\n", auto_name);
    write_log("아카이브 시작: %d일 이상 파일 %d개 -> %s", days, count, auto_name);

    // tar 압축 실행------------------------------------------------------
    pid_t pid = fork(); //자식 프로세스 생성

    if(pid < 0){
        perror("Fork 실패");
    } else if (pid == 0) { //자식 프로세스
        //tar 명령어 실행
        execlp("tar", "tar", "-cvf", auto_name, temp_dir, NULL);
        exit(1);
    } else {    //부모 프로세스
        //압축 끝날 때까지 대기
        int status;
        wait(&status);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("\n✅ [성공] 아카이브 파일 생성 완료!\n");
            write_log("아카이브 성공: %s 생성 완료", auto_name);
            // 임시 폴더 삭제
            char cmd[CMD_BUFFER];
            snprintf(cmd, sizeof(cmd), "rm -rf %s", temp_dir);
            system(cmd);
        } else {
            printf("❌ [오류] 압축 과정에서 문제가 발생했습니다.\n");
            write_log("아카이브 실패: tar 명령어 오류");
        }
    }
    stop_loading_thread(tid);
}

// 나머지 파일 분류
void classify_files_by_extension(FileInfo *head, const char *base_dest_folder) {
    if (head == NULL) {
        printf(">> 분류할 파일 리스트가 없습니다.\n");
        return;
    }
    pthread_t tid = start_loading_thread();
    printf("\033[2K\r>> 파일 확장자를 분석하여 자동 분류를 시작합니다...\n");
    
    write_log("분류 시작: 대상 폴더 [%s]", base_dest_folder);

    // 베이스 폴더 생성
    if (mkdir_p(base_dest_folder) == -1) {
        perror("베이스 폴더 생성 실패");
        write_log("에러: 베이스 폴더(%s) 생성 실패", base_dest_folder);
        return;
    }

    FileInfo *curr = head;
    int count = 0;

    while (curr != NULL) {
        // 삭제되거나 아카이브로 이동된 파일 건너뜀
        if (access(curr->path, F_OK) != 0) {
            curr = curr->next;
            continue;
        }
        char cat[64];
        get_category_name(curr->name, cat);
        char category_folder[MAX_PATH];
        snprintf(category_folder, sizeof(category_folder), "%s/%s", base_dest_folder, cat);
        mkdir_p(category_folder);

        move_file(curr, category_folder);
        count++;

        curr = curr->next;
    }
    stop_loading_thread(tid);

    printf("\033[2K\r✅ 총 %d개의 파일이 종류별로 분류되었습니다.\n", count);
    write_log("분류 완료: 총 %d개 파일 정리됨", count);
}

// 원클릭 전체 정리 함수
void run_full_cleanup(FileInfo *head, int days, const char *final_dest) {
    printf("\n============================================\n");
    printf("       🚀 원클릭 전체 정리 시스템 가동 \n");
    printf("============================================\n");
    write_log("=== 원클릭 정리 시작 (기준: %d일, 타겟: %s) ===", days, final_dest);
    
    // 1단계: 중복 삭제
    remove_copy_files(head);
    
    // 2단계: 오래된 파일 아카이빙 (분류 포함)
    archive_files(head, days);
    
    // 3단계: 나머지 파일 분류
    classify_files_by_extension(head, final_dest);
    
    printf("\033[2K\r✅ 모든 작업이 완료되었습니다!\n");
    write_log("=== 원클릭 정리 종료 ===");
}
