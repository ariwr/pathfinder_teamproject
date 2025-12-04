# 컴파일러 설정
CC = gcc

# CFLAGS: 컴파일 옵션
# -Wall: 모든 경고 출력
# -g: 디버깅 정보 포함
# -pthread: 스레드 사용을 위한 전처리기 및 컴파일러 설정
CFLAGS = -Wall -g -pthread

# LDLIBS: 링크할 라이브러리 목록
# -lpthread: POSIX 스레드 라이브러리를 명시적으로 링크
LDLIBS = -lpthread

# 대상 실행 파일 이름 
TARGET = sysclean

# 소스 파일 및 오브젝트 파일 정의
SRCS = main.c scanner.c actions.c utils.c
OBJS = $(SRCS:.c=.o)

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

# .c 파일을 .o 파일로 컴파일하는 규칙
# 헤더파일(cleanup.h) 변경 시 재컴파일되도록 의존성 추가
%.o: %.c cleanup.h
	$(CC) $(CFLAGS) -c $< -o $@

# 정리(clean) 규칙
clean:
	rm -f $(OBJS) $(TARGET)

# 테스트 실행
run: $(TARGET)
	./$(TARGET)
