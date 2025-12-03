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
    char original_cwd[1024];     // 프로그램 시작 위치 저장용
    FileInfo *file_list = NULL;

    // 시작 위치 기억 
    if (getcwd(original_cwd, sizeof(original_cwd)) == NULL) {
        perror("getcwd error");
        return 1;
    }

    print_logo();

    while (1) {
        printf("\n============================================\n");
        printf("1. 🔍 디렉토리 스캔 (Scan)\n");
        printf("2. 🚀 원클릭 전체 정리 (One-Click Cleanup) [⭐추천]\n");
        printf("3. 🛠️ 고급/개별 기능 (Manual Tools)\n");
        printf("0. ❌ 프로그램 종료\n");
        printf("============================================\n");
        printf("선택 >> ");
        
        if (scanf("%d", &choice) != 1) {
            // 문자가 들어왔을 때 무한루프 방지
            while(getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1: // 스캔 기능
                if (file_list != NULL) 
                    free_file_list(file_list);
                
                if (pick_folder_with_gui(target_dir, sizeof(target_dir))) {
                    printf(">> 선택된 경로: %s\n", target_dir);
                } else {
                    printf(">> ⚠️ 경로 선택 취소 또는 실패. 현재 폴더를 사용합니다.\n");
                }

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
            case 2: // 원클릭 정리 기능
                if (file_list == NULL) {
                    printf("\n>> ⚠️ 먼저 [1. 디렉토리 스캔]을 수행해주세요!\n");
                    break;
                }
                int days_auto;
                char final_dest[MAX_PATH];

                printf("\n[ 🚀 원클릭 전체 정리 설정 ]\n");
                printf("1. 며칠 이상 된 파일을 아카이빙(압축) 할까요? (예: 30) >> ");
                scanf("%d", &days_auto);

                printf("2. 나머지 파일들을 분류해서 담을 상위 폴더 이름 (예: Cleaned) >> ");
                scanf("%s", final_dest);

                printf("\n>> 📂 작업 위치를 '%s'로 이동합니다...\n", target_dir);
                if (chdir(target_dir) != 0) {
                    perror(">> ❌ 경로 이동 실패");
                    break;
                }

                // 통합 함수 실행
                run_full_cleanup(file_list, days_auto, final_dest);
                
                // 정리 후 리스트가 변경되므로 안전하게 해제
                free_file_list(file_list);
                file_list = NULL;
                printf(">> 💡 파일 구조가 변경되었습니다. 최신 상태를 보려면 [1. 스캔]을 다시 해주세요.\n");
                break;

            case 3: // 개별/수동 기능
                if (file_list == NULL) {
                    printf("\n>> ⚠️ 먼저 [1. 디렉토리 스캔]을 수행해주세요!\n");
                    break;
                }

                printf("\n[ 🛠️ 고급/개별 기능 선택 ]\n");
                printf("1. 📦 오래된 파일 아카이브 (Archive)\n");
                printf("2. 📂 확장자별 분류 이동 (Move)\n");
                printf("3. 📑 중복 파일 검사 및 삭제 (Duplicate)\n");
                printf("0. 🔙 뒤로 가기\n");
                printf("선택 >> ");

                int sub_choice;
                scanf("%d", &sub_choice);

                if (sub_choice != 0) {
                    if (chdir(target_dir) != 0) {
                        perror(">> ❌ 경로 이동 실패");
                        break;
                    }
                }

                if (sub_choice == 1) {
                    int days;
                    printf("며칠 이상 사용하지 않은 파일을 정리할까요? (예: 30) >> ");
                    scanf("%d", &days);
                    printf("\n[System] %d일 이상 된 파일을 찾아 아카이빙합니다...\n", days);
                    archive_files(file_list, days);
                } else if (sub_choice == 2) {
                    char dest_folder[MAX_PATH];
                    printf("분류된 폴더들을 담을 상위 폴더 이름 입력 (예: Sorted) >> ");
                    scanf("%s", dest_folder);
                    classify_files_by_extension(file_list, dest_folder);
                    printf(">> 💡 파일 경로가 변경되었습니다. 재스캔을 권장합니다.\n");

                } else if (sub_choice == 3) {
                    printf("\n[System] 중복 파일 검사를 시작합니다...\n");
                    remove_copy_files(file_list);
                }
                // 작업 후 복귀
                if (sub_choice != 0) chdir(original_cwd);
                break;

            case 0: // 종료
                printf("\n시스템을 종료합니다. Good Bye!\n");
                if (file_list != NULL) free_file_list(file_list);
                return 0;

            default:
                printf("\n>> ⚠️ 잘못된 입력입니다. 다시 선택해주세요.\n");
        }
    }

    return 0;
}

