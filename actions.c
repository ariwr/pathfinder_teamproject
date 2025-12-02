#include "cleanup.h"
#include <fcntl.h>
#include <sys/wait.h>

//파일 이동 함수
void move_file(FileInfo *file, const char *dest_folder){
    char dest_path[MAX_PATH];
    
    //목표 경로 문자열 생성
    snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_folder, file->name);

    //파일 이동
    if(rename(file->path, dest_path) == 0){
        printf("[이동 완료] %s -> %s\n", file->name, dest_path);
        strcpy(file->path, dest_path);  //경로 정보 업데이트
    } else {
        perror("[이동 실패]");
    }
}

//중복 파일 확인 함수
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

    int is_duplicate = 1;   //같다고 가정
    char buf1[FILE_BUFFER], buf2[FILE_BUFFER];
    size_t n1, n2;
    
    while((n1 = fread(buf1, 1, size(buf1), f1)) > 0){
        n2 = fread(buf2, 1, sizeof(buf2), f2);
        //길이, 메모리 비교
        if(n1 != n2 || memcmp(buf1, buf2, n1) != 0){
            is_duplicate = 0; //다르면 루프 탈출
            break;
        }
    }

    fclose(f1);
    fclose(f2);
    return is_duplicate;
}

//파일 압축 함수
void archive_files(FileInfo *head, const char *archive_name){
    if(head == NULL) return;

    char temp_dir[] = "temp_archive_zone";
    mkdir(temp_dir, 0755);

    FileInfo *current = head;
    while (current != NULL){
        //파일을 임시 폴더로 이동
        move_file(current, temp_dir);
        current = current->next;
    }

    //자식 프로세스 생성
    pid_t pid = fork();

    if(pid < 0){
        perror("Fork 실패");
    } else if (pid == 0) { //자식 프로세스
        //tar 명령어 실행
        execlp("tar", "tar", "-cvf", archive_name, temp_dir, NULL);
        exit(1);
    } else {    //부모 프로세스
        //압축 끝날 때까지 대기
        wait(NULL);
        printf("[압축 완료] %s 생성됨\n", archive_name);

        //임시 폴더 삭제
        char cmd[CMD_BUFFER]
        sprintf(cmd, "rm -rf %s", temp_dir);
        system(cmd);
    }
}
