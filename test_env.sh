#!/bin/bash

# 테스트용 디렉토리 생성
mkdir -p test_dir/downloads
mkdir -p test_dir/documents
mkdir -p test_dir/images

# 1. 오래된 파일 생성 (날짜 조작)
touch -d "2023-01-01" test_dir/downloads/old_report.pdf
touch -d "2022-05-20" test_dir/downloads/old_photo.jpg
touch -d "2024-01-10" test_dir/downloads/unused_setup.exe

# 2. 최근 파일 생성
touch test_dir/downloads/new_assignment.docx
touch test_dir/downloads/screenshot.png

# 3. 중복 파일 생성 (내용이 같아야 함)
echo "This is important data" > test_dir/downloads/data1.txt
echo "This is important data" > test_dir/downloads/copy_data1.txt
echo "Different data" > test_dir/downloads/data2.txt

echo "테스트 환경 생성 완료! (test_dir 폴더 확인)"