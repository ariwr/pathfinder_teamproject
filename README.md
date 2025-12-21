<div align="center">
  <img src="https://github.com/user-attachments/assets/4064c6b3-b897-4144-866c-4504fb52376e" width="600" alt="Smart Cleanup Agent Logo" />

  # 🧹 SMART CLEANUP AGENT
  > **리눅스 시스템 프로그래밍을 활용한 자동화 파일 관리 도구**

  [![](https://img.shields.io/badge/YouTube-Watch_Demo-red?style=flat-square&logo=youtube)](https://www.youtube.com/watch?v=966Vl48DIQ8)
  [![](https://img.shields.io/badge/ELEC462-KNU_Team_10-blue?style=flat-square)](https://github.com/ariwr/pathfinder_teamproject)
  [![](https://img.shields.io/badge/OS-Linux%20%7C%20WSL-orange?style=flat-square&logo=linux)](https://www.linux.org/)

  <p align="center">
    파일의 메타데이터 및 바이너리 분석을 통한<br>
    <b>중복 제거, 자동 아카이빙, 스마트 분류 솔루션</b>
  </p>
</div>

<br>

<details>
  <summary><b>📚 목차 (Table of Contents)</b></summary>
  <ol>
    <li>
      <a href="#about-the-project">프로젝트 소개</a>
      <ul>
        <li><a href="#built-with">사용 기술 및 시스템 콜</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">시작하기 (설치 방법)</a>
      <ul>
        <li><a href="#prerequisites">사전 설치 항목</a></li>
        <li><a href="#installation">빌드 및 실행</a></li>
      </ul>
    </li>
    <li><a href="#usage">사용법</a></li>
    <li><a href="#key-features">핵심 구현 기능</a></li>
    <li><a href="#contact">팀 정보</a></li>
    <li><a href="#structure">프로젝트 구조</a></li>
  </ol>
</details>

<br>

## 💻 프로젝트 소개 (About The Project) <a name="about-the-project"></a>

**Smart Cleanup Agent**는 Linux 환경(특히 WSL)에서 빈번하게 발생하는 '다운로드 폴더 난잡화'와 '불필요한 중복 파일로 인한 inode 낭비' 문제를 해결하기 위해 개발된 C 기반 시스템 프로그램입니다.

단순히 쉘 스크립트를 사용하는 것이 아니라, **System Call**을 직접 호출하여 파일의 inode 정보와 메타데이터를 정밀하게 분석합니다. 이를 통해 **바이너리 레벨의 중복 파일 제거**, **접근 시간(atime) 기반 아카이빙**, **확장자 자동 분류**를 수행하여 쾌적한 시스템 환경을 제공합니다.

### 🛠 사용 기술 및 시스템 콜 (Built With) <a name="built-with"></a>

* **Language:** C (GCC Compiler)
* **Parallelism:** POSIX Threads (`pthread`) - UI 로딩과 파일 처리를 분리
* **UI/UX:** Zenity (File Selection GUI), ANSI Escape Codes
* **Target OS:** Linux (Ubuntu 24.04), WSL2 (Windows Subsystem for Linux)

### ⚙️ 활용된 시스템 콜 (System Calls)
본 프로젝트는 다음과 같은 시스템 콜을 활용하여 커널 레벨에서 파일 시스템을 제어합니다.

| 분류 | 사용된 시스템 콜 |
| :--- | :--- |
| **파일 제어** | `open`, `read`, `write`, `unlink`, `rename`, `stat`, `lstat` |
| **디렉토리** | `opendir`, `readdir`, `closedir`, `mkdir`, `chdir`, `getcwd` |
| **프로세스/IPC** | `fork`, `execvp`, `wait`, `popen`|
| **기타** | `signal`, `pthread_create`, `pthread_join` |

<br>

##  시작하기 (Getting Started) <a name="getting-started"></a>

### 1. 사전 설치 항목 (Prerequisites) <a name="prerequisites"></a>
이 프로젝트는 **Zenity**를 사용하여 GUI 폴더 선택 창을 제공하며, 한글 파일명 처리를 위해 로케일 설정이 필요합니다. Ubuntu 환경 기준 아래 명령어를 실행해주세요.
```bash
# 1. 빌드 도구 및 필수 패키지 설치
sudo apt update
sudo apt install -y build-essential gcc make zenity

# 2. 한글 로케일 및 폰트 설치 (한글 파일명 깨짐 방지)
sudo apt install -y language-pack-ko fonts-nanum
sudo locale-gen ko_KR.UTF-8
sudo update-locale LANG=ko_KR.UTF-8
```

### 2. 빌드 및 실행 (Installation & Run) <a name="installation"></a>

#### Repository Clone

```bash
git clone https://github.com/github_username/pathfinder_teamproject.git
```
```bash
cd pathfinder_teamproject
```
#### Compile (Make)
```bash
make
```
#### Run
```bash
./sysclean
```

## 🎮 사용법 (Usage) <a name="usage"></a>
프로그램 실행 시 다음과 같은 CLI 메뉴가 제공됩니다.

### Plaintext
```bash
============================================
1. 🔍 디렉토리 스캔 (Scan)
2. 🚀 원클릭 전체 정리 (One-Click Cleanup) [⭐추천]
3. 🛠️ 고급/개별 기능 (Manual Tools)
0. ❌ 프로그램 종료
============================================
```
🔍 디렉토리 스캔: 정리할 폴더를 선택합니다. WSL 환경인 경우 Windows 사용자를 자동 감지하여 C:\Users\[User]\Downloads 경로를 기본으로 제안합니다.

🚀 원클릭 전체 정리: [중복 제거] → [오래된 파일 압축] → [폴더 분류] 과정을 한 번에 수행합니다.

🛠️ 개별 기능: 특정 기능만 골라서 수행할 수 있습니다.


---
<br>

## 🗺️ 핵심 구현 기능 (Key Features) <a name="key-features"></a>

### 1. 🗑️ 스마트 중복 제거 (Smart Deduplication)
* **패턴 인식**: 파일명 뒤에 붙는 `(1)`, `(2)` 등의 복사본 패턴을 파싱하여 원본 파일명을 추론합니다.
* **바이너리 비교**: 단순히 이름만 같은 것이 아니라, `memcmp`를 사용하여 원본과 복사본의 **파일 내용(Binary Content)이 100% 일치하는지 검증**한 후 안전하게 삭제(`unlink`)합니다. 

### 2. 📦 오래된 파일 아카이빙 (Time-based Archiving)
* **접근 시간 분석**: `stat` 구조체의 `st_atime`을 분석하여 사용자가 설정한 기간(예: 30일) 동안 접근하지 않은 파일을 식별합니다.
* **자동 압축**: `fork()`와 `execvp()`를 통해 자식 프로세스에서 `tar` 명령어를 실행, 해당 파일들을 `YYYYMMDD-YYYYMMDD_archive.tar` 형태로 압축하여 보관합니다.

### 3. 📂 확장자 자동 분류 (Auto Classification)
* 파일의 확장자를 파싱하여 `Images`, `Documents`, `Codes`, `Media`, `Archives`, `Programs` 등 6개 이상의 카테고리 폴더로 자동 이동(`rename`)시킵니다.

<br>


## 👥 팀 정보 (Contact) <a name="contact"></a>

**경북대학교 - 시스템프로그래밍 Team 10 (Pathfinder)**

| 이름 | 역할 | 담당 업무 |
| :---: | :---: | :--- |
| **이유진** | 👑 Main Dev | 구조 설계, 스캔 알고리즘, 자료구조, Main UI |
| **신예음** | 🛠️ Dev / System |  파일 처리 로직 구현, 윈도우 경로 파싱, 설정 관리 모듈 구현, 발표 |

<br>

## Project Structure <a name="structure"></a>
```bash
.
├── main.c        # 프로그램 진입점, 메뉴 및 시그널 핸들링
├── scanner.c     # 디렉토리 탐색 및 파일 리스트 생성 (Linked List)
├── actions.c     # 파일 이동, 중복 제거, 아카이빙, 분류 로직 구현
├── utils.c       # 로깅, GUI(Zenity) 연동, 스레드(로딩바) 관리
├── cleanup.h     # 헤더 파일 (구조체 및 함수 선언)
├── Makefile      # 빌드 스크립트
└── cleanup.log   # (실행 시 생성) 작업 상세 로그
```
<div align="center">
  System Programming Project @ 2025 KNU
</div>
