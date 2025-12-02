# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -g -pthread
# -Wall: 모든 경고 출력
# -g: 디버깅 정보 포함 (gdb 사용 시 필요)
# -pthread: 스레드 라이브러리 사용

# 대상 실행 파일 이름
TARGET = sysclean

# 소스 파일 및 오브젝트 파일 정의
SRCS = main.c scanner.c actions.c utils.c
OBJS = $(SRCS:.c=.o)

# 기본 타겟
all: $(TARGET)

# 실행 파일 생성 규칙
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# .c 파일을 .o 파일로 컴파일하는 규칙
%.o: %.c cleanup.h
	$(CC) $(CFLAGS) -c $< -o $@

# 정리(clean) 규칙
clean:
	rm -f $(OBJS) $(TARGET)

# 테스트 실행
run: $(TARGET)
	./$(TARGET)