#include "cleanup.h"

// 1. 멋진 로고 출력 함수
void print_logo() {
    printf("\n");
    printf("==============================================================\n");
    printf("       📂 SMART CLEANUP AGENT (Team 10) 📂       \n");
    printf("       System Programming Project - File Organizer    \n");
    printf("==============================================================\n");
    printf("\n");
}

// 2. 파일 통계 계산 및 출력 함수 (A파트의 센스!)
void print_statistics(FileInfo *head) {
    int count = 0;
    long total_size = 0;
    FileInfo *current = head;

    while (current != NULL) {
        count++;
        total_size += current->size;
        current = current->next;
    }

    printf("\n[📊 스캔 요약 보고서]\n");
    printf("------------------------------------------\n");
    printf("  - 발견된 파일 수 : %d 개\n", count);
    printf("  - 총 사용 용량   : %.2f MB\n", total_size / (1024.0 * 1024.0));
    printf("------------------------------------------\n");
}

// 3. 메인 메뉴
int main(int argc, char *argv[]) {
    int choice;
    char target_dir[1024] = "."; // 기본값은 현재 폴더
    FileInfo *file_list = NULL;

    print_logo();

    while (1) {
        printf("\n[ 메인 메뉴 ]\n");
        printf("1. 🔍 디렉토리 스캔 설정 및 실행\n");
        printf("2. 🧹 파일 정리 시작 (Action)\n");
        printf("3. ❌ 프로그램 종료\n");
        printf("선택 >> ");
        
        if (scanf("%d", &choice) != 1) {
            // 문자가 들어왔을 때 무한루프 방지
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: // 스캔 실행
                printf("\n스캔할 디렉토리 경로를 입력하세요 (현재: %s): ", target_dir);
                printf("\n(입력 없이 엔터를 누르면 현재 폴더 스캔 불가능, 경로를 입력해주세요) >> ");
                scanf("%s", target_dir);

                // 기존 리스트가 있다면 메모리 해제 후 다시 스캔
                if (file_list != NULL) free_file_list(file_list);
                
                printf("\n>> '%s' 디렉토리를 스캔 중입니다...\n", target_dir);
                file_list = scan_directory(target_dir);

                if (file_list != NULL) {
                    print_statistics(file_list);
                    
                    // 스캔 직후 목록을 볼지 물어보기
                    char show_list;
                    printf("상세 파일 목록을 출력하시겠습니까? (y/n): ");
                    scanf(" %c", &show_list); // 공백을 넣어 엔터 키 무시
                    if (show_list == 'y' || show_list == 'Y') {
                        print_file_list(file_list);
                    }
                } else {
                    printf(">> ⚠️ 파일을 찾지 못했거나 경로가 잘못되었습니다.\n");
                }
                break;

            case 2: // 정리 작업 (B파트 연결 구간)
                if (file_list == NULL) {
                    printf("\n>> ⚠️ 먼저 [1. 디렉토리 스캔]을 수행해주세요!\n");
                    break;
                }
                
                printf("\n[ 🧹 정리 옵션 선택 ]\n");
                printf("1. 오래된 파일 자동 아카이브 (Archive)\n");
                printf("2. 키워드/종류별 분류 이동 (Move)\n");
                printf("3. 중복 파일 찾기 (Duplicate)\n");
                printf("0. 뒤로 가기\n");
                printf("선택 >> ");
                
                int sub_choice;
                scanf("%d", &sub_choice);
                
                if (sub_choice == 1) {
                    char archive_name[MAX_NAME];
                    printf("생성할 압축 파일명 입력 (예: backup.tar) >> ");
                    scanf("%s", archive_name);
                    
                    printf("\n[System] 아카이빙 기능을 실행합니다...\n");
                    archive_files(file_list, archive_name);
                } else if (sub_choice == 2) {
                    char dest_folder[MAX_PATH];
                    printf("파일들을 이동할 목적지 폴더 입력 >> ");
                    scanf("%s", dest_folder);

                    //목적지 폴더가 없으면 생성 (mkdir -p)
                    char cmd[MAX_PATH + 10];
                    sprintf(cmd, "mkdir -p %s", dest_folder);
                    system(cmd);

                    printf("\n[System] 파일 분류 기능을 실행합니다...\n");
                    FileInfo *curr = file_list;
                    while (curr != NULL) {
                        move_file(curr, dest_folder);
                        curr = curr->next;
                    }
                } else if (sub_choice == 3) {
                    printf("\n[System] 중복 파일 검사를 시작합니다...\n");
                    
                    int dup_count = 0;
                    FileInfo *p1 = file_list;
                    
                    while(p1 != NULL) {
                        FileInfo *p2 = p1->next;
                        while(p2 != NULL){
                            if(check_duplicate(p1, p2)){
                                printf("⚠️[중복 발견]:\n   원본: %s\n   중복: %s\n", p1->name, p2->name);
                                dup_count++;
                            }
                            p2 = p2->next;
                        }
                        p1 = p1->next;
                    }
                    if (dup_count == 0) printf(">> 중복된 파일이 없습니다.\n");
                    else printf(">> 총 %d개의 중복 쌍을 발견했습니다.\n", dup_count);
                }
                break;

            case 3: // 종료
                printf("\n시스템을 종료합니다. Good Bye!\n");
                if (file_list != NULL) free_file_list(file_list);
                return 0;

            default:
                printf("\n>> ⚠️ 잘못된 입력입니다. 다시 선택해주세요.\n");
        }
    }

    return 0;
}
