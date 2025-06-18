// ===============================================================

/* 컴파일러 옵션에서 최적화 끄기(예상치 못한 동작 방지) */

#pragma GCC optimize("O0")

// ===============================================================

/* 기본 라이브러리 include */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#if !defined(__WIN64) && (defined(__W64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(__WIN32) || defined(_WIN32) || defined(_MSC_VER))
#define __WIN64 1
#endif


#ifdef __WIN64
#include <conio.h>
#else // 리눅스/유닉스용 라이브러리를 include 하는 부분입니다. 윈도우에서는 해당 라이브러리들을 사용하지 않습니다.
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#endif

// ===============================================================

/* 윈도우용 소켓 통신 정의 및 DLL 로딩 부분입니다. 해당 부분 (36~114줄)은 Copyright (c) 2025 Ju Seungmin에게 저작권이 있으며, MIT 라이센스에 따라 공개되어 있습니다. */

#ifdef __WIN64
#include <winsock2.h>
#include <windows.h>
#define DLL_NAME "ws2_32.dll"
#define RESOLVE(fn)  (fn##_t)GetProcAddress(hMod, #fn)

#undef htons
#undef htonl
#undef inet_addr

HMODULE hMod;
typedef int  (WINAPI *WSAStartup_t)(WORD, LPWSADATA);
typedef int  (WINAPI *WSACleanup_t)(void);
typedef int  (WINAPI *connect_t)(SOCKET,const struct sockaddr*,int);
typedef SOCKET (WINAPI *socket_t)(int,int,int);
typedef int  (WINAPI *send_t)(SOCKET,const char*,int,int);
typedef int  (WINAPI *recv_t)(SOCKET,char*,int,int);
typedef int  (WINAPI *shutdown_t)(SOCKET,int);
typedef int  (WINAPI *closesocket_t)(SOCKET);
typedef int (WINAPI *WSAGetLastError_t)(void);

WSAStartup_t   pWSAStartup;
WSACleanup_t   pWSACleanup;
socket_t       psocket;
connect_t      pconnect;
send_t         psend;
recv_t         precv;
shutdown_t     pshutdown;
closesocket_t  pclosesocket;
WSAGetLastError_t pWSAGetLastError;
SOCKET s;

typedef u_short (WINAPI *htons_t)(u_short);
typedef u_long  (WINAPI *htonl_t)(u_long);
typedef u_long (WINAPI *inet_addr_t)(const char*);
htons_t phtons;
htonl_t phtonl;
inet_addr_t pinet_addr;

unsigned short htons16(unsigned short v){ return (v >> 8) | (v << 8); }

unsigned long htonl32(unsigned long v) { return (v>>24)|((v>>8)&0x0000FF00)|((v<<8)&0x00FF0000)|(v<<24);}

/// 윈도우용 소켓 통신 라이브러리 다이나믹 로딩
int winsock_dynload(void){
    hMod = LoadLibraryA(DLL_NAME);
    if (!hMod) { fprintf(stderr,"LoadLibrary failed\n"); return 0; }

    pWSAStartup   = RESOLVE(WSAStartup);
    pWSACleanup   = RESOLVE(WSACleanup);
    psocket       = RESOLVE(socket);
    pconnect      = RESOLVE(connect);
    psend         = RESOLVE(send);
    precv         = RESOLVE(recv);
    pshutdown     = RESOLVE(shutdown);
    pclosesocket  = RESOLVE(closesocket);
    pWSAGetLastError = RESOLVE(WSAGetLastError);
    phtons  = RESOLVE(htons);
    phtonl  = RESOLVE(htonl);
    pinet_addr = RESOLVE(inet_addr);
    if (!phtons || !phtonl) { fputs("htons/htonl load fail\n", stderr); return 0; }

    if (!pWSAStartup||!psocket||!pconnect) {
        fprintf(stderr,"GetProcAddress failed\n");
        FreeLibrary(hMod);
        return 0;
    }

    WSADATA wsa;
    if (pWSAStartup(MAKEWORD(2,2), &wsa)!=0){
        fprintf(stderr,"WSAStartup err\n");
        FreeLibrary(hMod);
        return 0;
    }
    return 1;
}
void winsock_unload(void){
    if (pWSACleanup) pWSACleanup();
    if (hMod) FreeLibrary(hMod);
}

/* 위의 코드들은 윈도우에서 소켓 통신을 위한 라이브러리를 동적으로 로드하고, 필요한 함수들을 가져오는 부분입니다.
   이 코드는 MIT 라이센스에 따라 공개되어 있으며, (c) 주승민에게 저작권이 있습니다. */

// ===============================================================

/* 리눅스/유닉스용 소켓 통신 라이브러리를 include 하는 부분입니다. 윈도우에서는 해당 라이브러리들이 없으며, 사용하지 않습니다. */

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
int s;
#endif

// ===============================================================

/* 색상 관련 매크로 정의 */

#ifdef __WIN64
#define stdHandle GetStdHandle(STD_OUTPUT_HANDLE)
#define SetColor(color) SetConsoleTextAttribute(stdHandle, color)

#define Red 4
#define Green 2
#define Blue 1
#define Yellow 6
#define LightGreen 10
#define LightRed 12
#define LightBlue 9
#define LightYellow 14
#define White 7
#else
#define Red 31
#define Green 32
#define Blue 34
#define Yellow 33
#define LightGreen 92
#define LightRed 91
#define LightBlue 94
#define LightYellow 93
#define White 0
#define SetColor(color) printf("\033[%dm", color)
#endif

// ===============================================================

/* 아키텍쳐 감지를 위한 매크로 정의입니다. 어셈블리어는 CPU 아키텍쳐에 따라 그 문법이 달라지기 때문에 해당 내용을 적용했습니다. */

#if !defined(__x86_64__) && (defined(_M_X64) || defined(_M_AMD64) || defined(__amd64__))
#define __x86_64__ 1
#endif

#if !defined(__ARM64__) && (defined(_M_ARM64) || defined(__arm64__) || defined(__aarch64__))
#define __ARM64__ 1
#endif

typedef unsigned long long int int64; // 코드 간소화 위해 타입 축약(long long은 8바이트/64비트이므로 int64라고 정의)

/* 아래 함수들은 코드의 리버스 엔지니어링을 막기 위해 unsigned long long 정수의 연산을 인라인 어셈블리로 구현한 것입니다. 실제 함수 정의는 main 함수 아래에 있습니다.
   C언어 해석은 #else로 확인하셔도 되지만, 함수 이름 따라 덧셈, NOT, 왼쪽 시프트, 오른쪽 시프트 연산을 단순히 내장 기능을 사용하지 않고 구현한 것 입니다. */

inline __attribute__((always_inline)) int64 __add_64(int64 a, int64 b);
inline __attribute__((always_inline)) int64 __not_64(int64 a);
inline __attribute__((always_inline)) int64 __left_shift_64(int64* result, int64 a, int positions);
inline __attribute__((always_inline)) int64 __right_shift_64(int64 a, int positions);

// ===============================================================
// ===============================================================

/* 이 부분부터는 32바이트(unsigned long long의 4배) 정수형인 my_int_256에 대해 각종 연산 등을 구현한 부분입니다.*/

/* my_int_256의 선언 */

typedef void* my_int_256; // 32바이트(unsigned long long의 4배) 정수형을 void 포인터로 정의, 이후 사용시 init에서 malloc으로 동적 할당
#define BIGINT_SIZE 32 // 256비트 정수형의 크기 정의 (32바이트)
#define QWORD_COUNT 4 // 256비트 정수형을 64비트 단위로 나누었을 때의 개수 (4개)

my_int_256 init(); // my_int_256 타입의 값을 초기화하는 함수
my_int_256 from_small_int(int value); // 작은 정수(int)를 my_int_256 타입으로 변환하는 함수
my_int_256 from_string(const char* str); // 문자열을 my_int_256 타입으로 변환하는 함수
char* to_string(my_int_256 num); // my_int_256 타입의 값을 문자열로 변환하는 함수
my_int_256 left_shift(my_int_256 a, int positions); // my_int_256 타입의 값을 왼쪽으로 시프트하는 함수
my_int_256 right_shift(my_int_256 a, int positions); // my_int_256 타입의 값을 오른쪽으로 시프트하는 함수
my_int_256 bitwise_xor(my_int_256 a, my_int_256 b); // my_int_256 타입의 두 값을 비트 단위로 XOR하는 함수
my_int_256 plus(my_int_256 a, my_int_256 b); // my_int_256 타입의 두 값을 더하는 함수
my_int_256 minus(my_int_256 a, my_int_256 b); // my_int_256 타입의 두 값을 빼는 함수
my_int_256 multiply(my_int_256 a, my_int_256 b); // my_int_256 타입의 두 값을 곱하는 함수
my_int_256 divide(my_int_256 a, my_int_256 b); // my_int_256 타입의 두 값을 나누는 함수
bool is_zero(my_int_256 a); // my_int_256 타입의 값이 0인지 확인하는 함수
int compare(my_int_256 a, my_int_256 b); // my_int_256 타입의 두 값을 비교하는 함수


// ===============================================================
// ===============================================================

/* 아래 함수들은 소켓 통신 관련 함수들입니다.*/

// socket_init : 소켓을 초기화하고 서버에 연결하는 함수
inline __attribute__((always_inline)) void socket_init(const char* server_ip, int server_port) {
    if (server_ip == NULL || server_port <= 0) {
        fprintf(stderr, "Invalid server IP or port\n");
        return;
    }
#ifdef __WIN64
    if (!winsock_dynload()) {
        fprintf(stderr, "Failed to load Winsock library\n");
        return;
    }
    s = psocket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = phtons(server_port);
    srv.sin_addr.s_addr = pinet_addr(server_ip);
    if (pconnect(s, (struct sockaddr*)&srv, sizeof(srv)) < 0) {
        fprintf(stderr, "connect failed: %d\n", pWSAGetLastError());
        pclosesocket(s);
        return;
    }
#else
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return;
    }
    struct sockaddr_in srv = {0};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(server_port);
    srv.sin_addr.s_addr = inet_addr(server_ip);
    if (connect(s, (struct sockaddr*)&srv, sizeof(srv)) < 0) {
        perror("connect failed");
        close(s);
        return;
    }
#endif
}

// socket_send : 소켓을 통해 데이터를 전송하는 함수
inline __attribute__((always_inline)) void socket_send(const char* data) {
    if (data == NULL) {
        fprintf(stderr, "Invalid data\n");
        return;
    }
#ifdef __WIN64
    if (psend(s, data, (int)strlen(data), 0) < 0) {
        fprintf(stderr, "send failed: %d\n", pWSAGetLastError());
    }
#else
    if (send(s, data, (int)strlen(data), 0) < 0) {
        perror("send failed");
    }
#endif
}

// socket_receive : 소켓을 통해 데이터를 수신하는 함수
inline __attribute__((always_inline)) void socket_receive(char* buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        fprintf(stderr, "Invalid buffer or size\n");
        return;
    }
#ifdef __WIN64
    int bytes_received = precv(s, buffer, (int)size - 1, 0);
    if (bytes_received < 0) {
        fprintf(stderr, "recv failed: %d\n", pWSAGetLastError());
        return;
    }
    buffer[bytes_received] = '\0';
#else
    ssize_t bytes_received = recv(s, buffer, size - 1, 0);
    if (bytes_received < 0) {
        perror("recv failed");
        return;
    }
    buffer[bytes_received] = '\0';
#endif
}

// socket_close : 소켓을 닫고 자원을 해제하는 함수
inline __attribute__((always_inline)) void socket_close() {
#ifdef __WIN64
    if (pshutdown(s, SD_BOTH) < 0) {
        fprintf(stderr, "shutdown failed: %d\n", pWSAGetLastError());
    }
    if (pclosesocket(s) < 0) {
        fprintf(stderr, "closesocket failed: %d\n", pWSAGetLastError());
    }
    winsock_unload();
#else
    if (shutdown(s, SHUT_RDWR) < 0) {
        perror("shutdown failed");
    }
    if (close(s) < 0) {
        perror("close failed");
    }
#endif
}

// ===============================================================

/* 게임 관련 추가 함수들 */

// xorshift128+ 난수 생성기 상태
int64 xorshift_state[2];

// 게임 시작 시간과 패널티 시간
int64 game_start_time;
int64 penalty_time;

// 나노초 단위 시간 가져오기
inline __attribute__((always_inline)) int64 get_nanoseconds() {
#ifdef __WIN64
    LARGE_INTEGER frequency, counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (int64)(counter.QuadPart * 1000000000ULL / frequency.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif
}

// xorshift128+ 초기화
inline __attribute__((always_inline)) void init_random() {
    int64 seed = get_nanoseconds();
    xorshift_state[0] = seed;
    xorshift_state[1] = seed ^ 0x123456789ABCDEFULL;
    if (xorshift_state[0] == 0) xorshift_state[0] = 1;
    if (xorshift_state[1] == 0) xorshift_state[1] = 1;
}

// xorshift128+ 난수 생성
inline __attribute__((always_inline)) int64 xorshift128plus() {
    int64 x = xorshift_state[0];
    int64 y = xorshift_state[1];
    xorshift_state[0] = y;
    x ^= x << 23;
    xorshift_state[1] = x ^ y ^ (x >> 17) ^ (y >> 26);
    return xorshift_state[1] + y;
}

// 256비트 큰 수 랜덤 생성
inline __attribute__((always_inline)) my_int_256 generate_random_bigint() {
    my_int_256 result = init();
    int64* data = (int64*)result;

    for(int i = 0; i < 4; i++) {
        data[i] = xorshift128plus();
    }

    // 최상위 비트는 0으로 만들어서 너무 큰 수가 되지 않도록 함
    data[3] &= 0x7FFFFFFFFFFFFFFFULL;

    return result;
}

// 콘솔 설정 (Raw mode)
#ifndef __WIN64
struct termios original_termios;

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}
#else
void enable_raw_mode() {
    CONSOLE_CURSOR_INFO cursorInfo = { 0, };
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(stdHandle, &cursorInfo);
}
#endif

// 키 입력 받기 (1문자씩)
inline __attribute__((always_inline)) int get_char() {
#ifdef __WIN64
    return _getch();
#else
    return getchar();
#endif
}

// Non-blocking 키 입력 받기
inline __attribute__((always_inline)) int get_char_nonblocking() {
#ifdef __WIN64
    if (_kbhit()) {
        return _getch();
    }
    return -1;
#else
    int ch;
    struct termios old_termios, new_termios;

    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VMIN] = 0;
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);

    char buffer;
    int bytes_read = read(STDIN_FILENO, &buffer, 1);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);

    if (bytes_read <= 0) {
        return -1;
    }
    return (unsigned char)buffer;
#endif
}

// 화면 지우기
inline __attribute__((always_inline)) void clear_screen() {
#ifdef __WIN64
    system("cls");
#else
    system("clear");
#endif
}

// 커서 위치 이동
inline __attribute__((always_inline)) void move_cursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

// 색상 출력 함수
inline __attribute__((always_inline)) void print_colored(const char* text, int color) {
    SetColor(color);
    printf("%s", text);
    SetColor(White);
}

// 경과 시간을 포맷하여 반환하는 함수 (오버플로우 방지)
inline __attribute__((always_inline)) char* format_elapsed_time(int64 elapsed_ns) {
    char* time_str = malloc(64);

    // 음수이거나 0인 경우 처리
    if (elapsed_ns <= 0) {
        snprintf(time_str, sizeof(time_str), "00:00");
        return time_str;
    }

    // 오버플로우 방지를 위한 최대값 제한 (약 292년)
    const int64 max_ns = 9223372036854775807LL;  // int64 최대값
    if (elapsed_ns > max_ns / 1000000000LL * 1000000000LL) {
        elapsed_ns = max_ns / 1000000000LL * 1000000000LL;
    }

    // 나노초를 초로 변환
    int64 total_seconds = elapsed_ns / 1000000000LL;

    // 시간 단위로 변환 시 오버플로우 방지
    if (total_seconds > 359999) {  // 99:59:59 제한
        total_seconds = 359999;
    }

    int hours = (int)(total_seconds / 3600);
    int minutes = (int)((total_seconds % 3600) / 60);
    int seconds = (int)(total_seconds % 60);

    if (hours > 0) {
        snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", hours, minutes, seconds);
    } else {
        snprintf(time_str, sizeof(time_str), "%02d:%02d", minutes, seconds);
    }

    return time_str;
}

// 실시간 시간 표시 함수 (커서 위치 저장/복원 포함)
inline __attribute__((always_inline)) void display_current_time(int restore_x, int restore_y) {
    // 현재 커서 위치 저장을 위해 시간 표시 위치로 이동
    printf("\033[s");  // 현재 커서 위치 저장
    move_cursor(50, 1);

    int64 current_time = get_nanoseconds();
    // 오버플로우 방지를 위한 안전한 계산
    int64 elapsed_time = 0;
    if (current_time >= game_start_time) {
        elapsed_time = (current_time - game_start_time) + penalty_time;
    } else {
        elapsed_time = penalty_time;
    }

    char* elapsed_str = format_elapsed_time(elapsed_time);
    print_colored("경과 시간: ", LightBlue);
    print_colored(elapsed_str, LightYellow);

    // 패널티 시간이 있으면 표시
    if (penalty_time > 0) {
        char penalty_str[32];
        int penalty_seconds = (int)(penalty_time / 1000000000LL);
        snprintf(penalty_str, sizeof(penalty_str), " (패널티: +%d초)", penalty_seconds);
        print_colored(penalty_str, LightRed);
    }
    printf("     ");

    printf("\033[u");  // 저장된 커서 위치로 복원
    fflush(stdout);
}

// 아스키 아트 타이틀
inline __attribute__((always_inline)) void print_title() {
    clear_screen();
    print_colored("┌─────────────────────────────────────────────────────────────────────────┐\n", Blue);
    print_colored("│                                                                         │\n", Blue);
    print_colored("│                   ███████╗██████╗ ███████╗███████╗██████╗               │\n", LightGreen);
    print_colored("│                   ██╔════╝██╔══██╗██╔════╝██╔════╝██╔══██╗              │\n", LightGreen);
    print_colored("│                   ███████╗██████╔╝█████╗  █████╗  ██║  ██║              │\n", LightGreen);
    print_colored("│                   ╚════██║██╔═══╝ ██╔══╝  ██╔══╝  ██║  ██║              │\n", LightGreen);
    print_colored("│                   ███████║██║     ███████╗███████╗██████╔╝              │\n", LightGreen);
    print_colored("│                   ╚══════╝╚═╝     ╚══════╝╚══════╝╚═════╝               │\n", LightGreen);
    print_colored("│                                                                         │\n", Blue);
    print_colored("│                    ███╗   ███╗ █████╗ ████████╗██╗  ██╗                 │\n", Yellow);
    print_colored("│                    ████╗ ████║██╔══██╗╚══██╔══╝██║  ██║                 │\n", Yellow);
    print_colored("│                    ██╔████╔██║███████║   ██║   ███████║                 │\n", Yellow);
    print_colored("│                    ██║╚██╔╝██║██╔══██║   ██║   ██╔══██║                 │\n", Yellow);
    print_colored("│                    ██║ ╚═╝ ██║██║  ██║   ██║   ██║  ██║                 │\n", Yellow);
    print_colored("│                    ╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝                 │\n", Yellow);
    print_colored("│                                                                         │\n", Blue);
    print_colored("│                        256비트 큰 수 덧셈 게임                          │\n", White);
    print_colored("│                                                                         │\n", Blue);
    print_colored("└─────────────────────────────────────────────────────────────────────────┘\n", Blue);
    printf("\n");
    print_colored("빠른 계산으로 순위를 올려보세요!\n", LightYellow);
    print_colored("역순으로 입력해도 됩니다 (백스페이스로 수정 가능)\n", LightBlue);
    printf("\n");
    print_colored("Enter를 눌러 시작하세요...", LightGreen);
    fflush(stdout);
}

// 문자열 뒤집기 (역순 입력 허용)
inline __attribute__((always_inline)) void reverse_string(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

// 게임 입력 처리
inline __attribute__((always_inline)) int input_number_digit_by_digit(const char* correct_answer, char* user_input) {
    int len = strlen(correct_answer);
    int pos = 0;
    int ch;

    // 시간 표시를 위한 타이머 변수
    int64 last_time_update = 0;
    int input_start_x, input_start_y;
    short mode = 0;

    while (pos <= len) {
        // 100ms마다 시간 업데이트 (커서 위치 복원 포함)
        int64 current_ns = get_nanoseconds();
        if (current_ns - last_time_update > 100000000LL) { // 100ms
            display_current_time(0, 0);  // 커서 위치는 자동으로 저장/복원됨
            last_time_update = current_ns;
        }

        ch = get_char_nonblocking();
        if (ch == -1) {
            // 키 입력이 없으면 10ms 대기 후 계속
#ifdef __WIN64
            Sleep(10);
#else
            usleep(10000);
#endif
            continue;
        }

        if (ch == '\n' || ch == '\r') {
            if (pos == len) break;
            user_input[pos] = '\0';

            // 정방향 검사
            if (strcmp(correct_answer, user_input) == 0) {
                return 1;
            }

            // 역방향 검사
            char reversed[256];
            strcpy(reversed, user_input);
            reverse_string(reversed);
            if (strcmp(correct_answer, reversed) == 0) {
                return 1;
            }

            if (pos == len) break;
            continue;
        }

        if (ch == 127 || ch == 8) { // 백스페이스
            if (pos > 0) {
                pos--;
                printf("\b \b");
                fflush(stdout);
            } else mode = 0;
            continue;
        }

        if (ch >= '0' && ch <= '9' && pos < len) {
            user_input[pos] = ch;

            // 정방향 또는 역방향으로 맞는지 확인
            char temp_input[256];
            strncpy(temp_input, user_input, pos + 1);
            temp_input[pos + 1] = '\0';

            char temp_reversed[256];
            strcpy(temp_reversed, temp_input);
            reverse_string(temp_reversed);

            bool correct_forward = (pos < len && ch == correct_answer[pos]);
            bool correct_backward = (pos < len && ch == correct_answer[len - 1 - pos]);

            mode = mode == 0 ? correct_forward - correct_backward : mode; // 1: 정방향, -1: 역방향, 0: 둘 다 맞거나 둘 다 맞지 않음

            if (mode >= 0 && correct_forward || mode <= 0 && correct_backward) {
                char ch_str[2] = {ch, '\0'};
                print_colored(ch_str, LightGreen);
            } else {
                char ch_str[2] = {ch, '\0'};
                print_colored(ch_str, LightRed);

                printf("\033[s");  // 커서 위치 저장

                // 패널티 5초 추가
                penalty_time += 5000000000LL; // 5초를 나노초로 변환

                // 패널티 추가 후 즉시 시간 업데이트
                move_cursor(50, 1);
                int64 current_time = get_nanoseconds();
                int64 elapsed_time = 0;
                if (current_time >= game_start_time) {
                    elapsed_time = (current_time - game_start_time) + penalty_time;
                } else {
                    elapsed_time = penalty_time;
                }
                char* elapsed_str = format_elapsed_time(elapsed_time);
                print_colored("경과 시간: ", LightBlue);
                print_colored(elapsed_str, LightYellow);
                if (penalty_time > 0) {
                    char penalty_str[32];
                    int penalty_seconds = (int)(penalty_time / 1000000000LL);
                    snprintf(penalty_str, sizeof(penalty_str), " (패널티: +%d초)", penalty_seconds);
                    print_colored(penalty_str, LightRed);
                }
                printf("     ");

                printf("\033[u");  // 커서 위치 복원
            }
            pos++;
            fflush(stdout);
        }
    }

    user_input[pos] = '\0';

    // 최종 검사 (정방향)
    if (strcmp(correct_answer, user_input) == 0) {
        return 1;
    }

    // 최종 검사 (역방향)
    char reversed[256];
    strcpy(reversed, user_input);
    reverse_string(reversed);
    if (strcmp(correct_answer, reversed) == 0) {
        return 1;
    }

    return 0;
}

// 게임 메인 로직
inline __attribute__((always_inline)) void play_game() {
    init_random();

    // 두 개의 큰 수 생성
    my_int_256 a = generate_random_bigint();
    my_int_256 b = generate_random_bigint();
    my_int_256 sum = plus(a, b);

    char* a_str = to_string(a);
    char* b_str = to_string(b);
    char* sum_str = to_string(sum);

    clear_screen();

    // 경과 시간 표시 (패널티 포함)
    int64 current_time = get_nanoseconds();
    int64 elapsed_time = current_time - game_start_time + penalty_time;
    char* elapsed_str = format_elapsed_time(elapsed_time);

    printf("문제:                                               ");
    print_colored("경과 시간: ", LightBlue);
    print_colored(elapsed_str, LightYellow);
    printf("\n\n");

    // 두 수의 자릿수를 맞추기 위해 패딩 추가
    int a_len = strlen(a_str);
    int b_len = strlen(b_str);
    int max_len = (a_len > b_len) ? a_len : b_len;

    // a_str 패딩
    printf("  ");
    for(int i = 0; i < max_len - a_len; i++) {
        printf(" ");
    }
    printf("%s\n", a_str);

    // b_str 패딩
    printf("+ ");
    for(int i = 0; i < max_len - b_len; i++) {
        printf(" ");
    }
    printf("%s\n", b_str);

    printf("────────────────────────────────────────────────────────────────────────────\n");
    printf("= ");

    char user_input[256] = {0};

    int64 start_time = get_nanoseconds();
    int correct = input_number_digit_by_digit(sum_str, user_input);
    int64 end_time = get_nanoseconds();

    double elapsed_ms = (end_time - start_time) / 1000000.0;

    printf("\n\n");
    if (correct) {
        print_colored("🎉 정답입니다!\n", LightGreen);
        printf("⏱️  소요 시간: %.2f ms\n", elapsed_ms);

        // 서버에 결과 전송
        char result_msg[512];
        snprintf(result_msg, sizeof(result_msg), "SCORE:%.2f", elapsed_ms);
        socket_send(result_msg);

        // 서버에서 순위 받기
        char rank_buffer[256];
        socket_receive(rank_buffer, sizeof(rank_buffer));
        printf("🏆 현재 순위: %s\n", rank_buffer);

    }

    free(a_str);
    free(b_str);
    free(sum_str);
    free(a);
    free(b);
    free(sum);
}

// ===============================================================

int main(void) {
    // 한글 인코딩 설정
#ifdef __WIN64
    system("chcp 65001 > nul");
#endif // __WIN64

    // 서버 연결
    socket_init("103.244.118.110", 13579);
    enable_raw_mode();

    // 타이틀 화면 표시
    print_title();

    // Enter 키 대기
    get_char();

    // 게임 시작 시간 초기화
    game_start_time = get_nanoseconds();
    penalty_time = 0;

    // 게임 시작
    while (1) {
        play_game();

        printf("\n");
        print_colored("다시 하시겠습니까? (y/n): ", White);
        int ch = get_char();
        if (ch != 'y' && ch != 'Y') {
            break;
        }
    }

    // 정리
#ifndef __WIN64
    disable_raw_mode();
#endif

    socket_close();
    clear_screen();
    print_colored("🎮 게임을 종료합니다. 감사합니다!\n", LightGreen);

    return 0;
}

// ===============================================================
// ===============================================================

/* my_int_256의 선언 */

// init : malloc을 통해 동적으로 공간을 할당하고 모두 0으로 초기화
inline __attribute__((always_inline)) my_int_256 init() {
    my_int_256 result = malloc(BIGINT_SIZE);
    memset(result, 0, BIGINT_SIZE);
    return result;
}

// ================================================================

/* my_int_256과 다른 타입 간의 변환 */

// from_small_int : int 타입의 값을 my_int_256 타입으로 변환하는 함수 (작은 수로 초기화를 위해 사용)
inline __attribute__((always_inline)) my_int_256 from_small_int(int value) {
    my_int_256 result = init();
    int64* qwords = (int64*)result;
    qwords[0] = (int64)value;
    return result;
}

// from_string : 문자열을 my_int_256 타입으로 변환하는 함수 (입력, 큰 수로 초기화 등을 위해 사용)
inline __attribute__((always_inline)) my_int_256 from_string(const char* str) {
    my_int_256 result = init();
    my_int_256 ten = from_small_int(10);
    int i=-1;
loop:
    if(i++==strlen(str)) goto end;
    if(str[i] < '0' || str[i] > '9') goto loop;
    my_int_256 temp = multiply(result, ten);
    free(result);
    result = temp;

    my_int_256 digit = from_small_int(str[i] - '0');
    temp = plus(result, digit);
    free(result);
    free(digit);
    result = temp;
    goto loop;
end:
    free(ten);
    return result;
}

// to_string : my_int_256 타입의 값을 문자열로 변환하는 함수 (출력 등을 위해 사용)
inline __attribute__((always_inline)) char* to_string(my_int_256 num) {
    char* buffer = malloc(100);
    if(is_zero(num)) {
        strcpy(buffer, "0");
        return buffer;
    }

    my_int_256 temp = init(); memcpy(temp, num, BIGINT_SIZE);
    my_int_256 ten = from_small_int(10);
    char result[100] = {0};
    int pos = 0, result_len;
loop:
    if(is_zero(temp)) goto next;
    my_int_256 quotient = divide(temp, ten);
    my_int_256 product = multiply(quotient, ten);
    my_int_256 remainder = minus(temp, product);

    result[pos++] = '0' + (int)*(int64*)remainder;

    free(temp);
    free(product);
    free(remainder);
    temp = quotient;
    goto loop;
next:
    result_len = pos;
    for(int i = 0; i < result_len; i++) {
        buffer[i] = result[result_len - 1 - i];
    }
    buffer[result_len] = '\0';

    free(temp);
    free(ten);
    return buffer;
}

// ===============================================================

/* my_int_256에 대한 비트 연산 */

// left_shift : my_int_256 타입의 값을 왼쪽으로 시프트하는 함수 (곱셈, 나눗셈 구현을 위해 필요)
inline __attribute__((always_inline)) my_int_256 left_shift(my_int_256 a, int positions) {
    my_int_256 result = init();
    int64* qa = (int64*)a;
    int64* qr = (int64*)result;

    if(positions >= 256) {
        return result;
    }

    if(positions >= 64) {
        int qword_shift = positions / 64;
        int bit_shift = positions % 64;

        for(int i = QWORD_COUNT - 1; i >= qword_shift; i--) {
            qr[i] = qa[i - qword_shift];
        }

        if(bit_shift > 0) {
            int64 carry = 0;
            for(int i = qword_shift; i < QWORD_COUNT; i++) {
                int64 new_carry = __left_shift_64(&qr[i], qr[i], bit_shift);
                qr[i] |= carry;
                carry = new_carry;
            }
        }
    } else {
        memcpy(qr, qa, BIGINT_SIZE);
        int64 carry = 0;

        for(int i = 0; i < QWORD_COUNT; i++) {
            int64 new_carry = __left_shift_64(&qr[i], qr[i], positions);
            qr[i] |= carry;
            carry = new_carry;
        }
    }

    return result;
}

// right_shift : my_int_256 타입의 값을 오른쪽으로 시프트하는 함수
inline __attribute__((always_inline)) my_int_256 right_shift(my_int_256 a, int positions) {
    my_int_256 result = init();
    int64* qa = (int64*)a;
    int64* qr = (int64*)result;

    if(positions >= 256) {
        return result;
    }

    if(positions >= 64) {
        int qword_shift = positions / 64;
        int bit_shift = positions % 64;

        for(int i = 0; i < QWORD_COUNT - qword_shift; i++) {
            qr[i] = qa[i + qword_shift];
        }

        if(bit_shift > 0) {
            int64 carry = 0;
            for(int i = QWORD_COUNT - qword_shift - 1; i >= 0; i--) {
                int64 temp = qr[i];
                qr[i] = __right_shift_64(qr[i], bit_shift);
                qr[i] |= carry;
                carry = temp << (64 - bit_shift);
            }
        }
    } else {
        memcpy(qr, qa, BIGINT_SIZE);
        int64 carry = 0;

        for(int i = QWORD_COUNT - 1; i >= 0; i--) {
            int64 temp = qr[i];
            qr[i] = __right_shift_64(qr[i], positions);
            qr[i] |= carry;
            if(positions < 64) {
                carry = temp << (64 - positions);
            } else {
                carry = 0;
            }
        }
    }

    return result;
}

// bitwise_xor : my_int_256 타입의 두 값을 비트 단위로 XOR하는 함수
inline __attribute__((always_inline)) my_int_256 bitwise_xor(my_int_256 a, my_int_256 b) {
    my_int_256 result = init();
    int64* qa = (int64*)a;
    int64* qb = (int64*)b;
    int64* qr = (int64*)result;

    for(int i = 0; i < QWORD_COUNT; i++) {
        qr[i] = qa[i] ^ qb[i];
    }

    return result;
}

// ===============================================================

/* my_int_256를 활용한 산술 연산 */

// plus : my_int_256 타입의 두 값을 더하는 함수
inline __attribute__((always_inline)) my_int_256 plus(my_int_256 a, my_int_256 b) {
    my_int_256 result = init();
    int64* qa = (int64*)a;
    int64* qb = (int64*)b;
    int64* qr = (int64*)result;

    int64 carry = 0;

    for(int i = 0; i < QWORD_COUNT; i++) {
        int64 sum = __add_64(qa[i], qb[i]);
        if(carry) {
            sum = __add_64(sum, carry);
        }
        qr[i] = sum;

        carry = 0;
        if(carry == 0) {
            if(sum < qa[i] || sum < qb[i]) {
                carry = 1;
            }
        } else {
            if(sum <= qa[i] || sum <= qb[i]) {
                carry = 1;
            }
        }
    }

    return result;
}

// minus : my_int_256 타입의 두 값을 빼는 함수
inline __attribute__((always_inline)) my_int_256 minus(my_int_256 a, my_int_256 b) {
    my_int_256 neg_b = init();
    int64* qb = (int64*)b;
    int64* qnb = (int64*)neg_b;

    for(int i = 0; i < QWORD_COUNT; i++) {
        qnb[i] = __not_64(qb[i]);
    }

    my_int_256 one = from_small_int(1);
    my_int_256 temp = plus(neg_b, one);
    free(neg_b);
    free(one);
    neg_b = temp;

    my_int_256 result = plus(a, neg_b);
    free(neg_b);
    return result;
}

// multiply : my_int_256 타입의 두 값을 곱하는 함수
inline __attribute__((always_inline)) my_int_256 multiply(my_int_256 a, my_int_256 b) {
    my_int_256 result = init();
    int64* qb = (int64*)b;

    for(int i = 0; i < 256; i++) {
        int qword_idx = i / 64;
        int bit_idx = i % 64;

        if(qword_idx >= QWORD_COUNT) break;

        int64 bit_mask = 1ULL << bit_idx;
        if(qb[qword_idx] & bit_mask) {
            my_int_256 shifted_a = left_shift(a, i);
            my_int_256 new_result = plus(result, shifted_a);
            free(result);
            free(shifted_a);
            result = new_result;
        }
    }

    return result;
}

// divide : my_int_256 타입의 두 값을 나누는 함수
inline __attribute__((always_inline)) my_int_256 divide(my_int_256 a, my_int_256 b) {
    my_int_256 quotient = init();
    my_int_256 remainder = init();

    if(is_zero(b)) return quotient;

    for(int i = 255; i >= 0; i--) {
        my_int_256 temp_remainder = left_shift(remainder, 1);
        free(remainder);
        remainder = temp_remainder;

        int qword_idx = i / 64;
        int bit_idx = i % 64;
        int64* qa = (int64*)a;
        int64* qrem = (int64*)remainder;

        if(qa[qword_idx] & (1ULL << bit_idx)) {
            qrem[0] |= 1;
        }

        if(compare(remainder, b) >= 0) {
            my_int_256 temp_remainder = minus(remainder, b);
            free(remainder);
            remainder = temp_remainder;

            int64* qq = (int64*)quotient;
            qq[qword_idx] |= (1ULL << bit_idx);
        }
    }

    free(remainder);
    return quotient;
}

// ===============================================================

/* my_int_256에 대한 비교 연산 */

inline __attribute__((always_inline)) int compare(my_int_256 a, my_int_256 b) {
    int64* qa = (int64*)a;
    int64* qb = (int64*)b;

    for(int i = QWORD_COUNT - 1; i >= 0; i--) {
        if(qa[i] > qb[i]) return 1;
        if(qa[i] < qb[i]) return -1;
    }
    return 0;
}

// is_zero : my_int_256 타입 변수의 값이 0인지 확인하는 함수
inline __attribute__((always_inline)) bool is_zero(my_int_256 a) {
    int64* qa = (int64*)a;
    for(int i = 0; i < QWORD_COUNT; i++) {
        if(qa[i] != 0) return false;
    }
    return true;
}


// ===============================================================
// ===============================================================

/* 아래 코드는 코드의 리버스 엔지니어링을 막기 위해 unsigned long long 정수의 연산을 인라인 어셈블리로 구현한 것입니다.
   C언어 해석은 각 함수의 #else에서 확인하실 수 있습니다. (동작은 동일) */

// __add_64 : 두 64비트 정수를 더하는 함수
inline __attribute__((always_inline)) int64 __add_64(int64 a, int64 b) {
    int64 result;
#ifdef __x86_64__
    __asm__ __volatile__(
        "movq %1, %%rax\n"
        "movq %2, %%rbx\n"
        "1:\n"
        "movq %%rax, %%rcx\n"
        "xorq %%rbx, %%rax\n"
        "andq %%rbx, %%rcx\n"
        "shlq $1, %%rcx\n"
        "movq %%rcx, %%rbx\n"
        "testq %%rbx, %%rbx\n"
        "jnz 1b\n"
        "movq %%rax, %0\n"
        : "=m"(result)
        : "r"(a), "r"(b)
        : "%rax", "%rbx", "%rcx", "cc"
    );
#elif defined(__aarch64__)
    __asm__ __volatile__(
        "mov x0, %1\n"
        "mov x1, %2\n"
        "1:\n"
        "mov x2, x0\n"
        "eor x0, x0, x1\n"
        "and x1, x2, x1\n"
        "lsl x1, x1, #1\n"
        "cbnz x1, 1b\n"
        "str x0, %0\n"
        : "=m"(result)
        : "r"(a), "r"(b)
        : "x0", "x1", "x2"
    );
#else
    while (b != 0) {
        int64 carry = a & b;
        a = a ^ b;
        b = carry << 1;
    }
    result = a;
#endif
    return result;
}

// __not_64 : 64비트 정수의 비트를 반전시키는 함수
inline __attribute__((always_inline)) int64 __not_64(int64 a) {
    int64 result;
#ifdef __x86_64__
    __asm__ __volatile__(
        "movq %1, %%rax\n"
        "notq %%rax\n"
        "movq %%rax, %0\n"
        : "=m"(result)
        : "r"(a)
        : "%rax"
    );
#elif defined(__aarch64__)
    __asm__ __volatile__(
        "mov x0, %1\n"
        "mvn x0, x0\n"
        "str x0, %0\n"
        : "=m"(result)
        : "r"(a)
        : "x0"
    );
#else
    result = ~a;
#endif
    return result;
}

// __left_shift_64 : 64비트 정수를 왼쪽으로 시프트하는 함수
inline __attribute__((always_inline)) int64 __left_shift_64(int64* result, int64 a, int positions) {
    if(positions >= 64) {
        *result = 0;
        return a;
    }

    int64 carry;
#ifdef __x86_64__
    __asm__ __volatile__(
        "movq %3, %%rax\n"
        "movq %2, %%rcx\n"
        "movq %%rax, %%rbx\n"
        "shlq %%cl, %%rax\n"
        "movq %%rax, %1\n"
        "movq $64, %%rcx\n"
        "subq %2, %%rcx\n"
        "shrq %%cl, %%rbx\n"
        "movq %%rbx, %0\n"
        : "=m"(carry), "=m"(*result)
        : "r"((int64)positions), "r"(a)
        : "%rax", "%rbx", "%rcx", "cc"
    );
#elif defined(__aarch64__)
    __asm__ __volatile__(
        "mov x0, %2\n"
        "mov x1, %3\n"
        "mov x2, #64\n"
        "sub x2, x2, x0\n"
        "mov x3, x1\n"
        "lsr x3, x3, x2\n"
        "str x3, %0\n"
        "lsl x1, x1, x0\n"
        "str x1, %1\n"
        : "=m"(carry), "=m"(*result)
        : "r"((int64)positions), "r"(a)
        : "x0", "x1", "x2", "x3"
    );
#else
    carry = a >> (64 - positions);
    *result = a << positions;
#endif
    return carry;
}

// __right_shift_64 : 64비트 정수를 오른쪽으로 시프트하는 함수
inline __attribute__((always_inline)) int64 __right_shift_64(int64 a, int positions) {
    if(positions >= 64) return 0;

    int64 result;
#ifdef __x86_64__
    __asm__ __volatile__(
        "movq %1, %%rax\n"
        "movq %2, %%rcx\n"
        "shrq %%cl, %%rax\n"
        "movq %%rax, %0\n"
        : "=m"(result)
        : "r"(a), "r"((int64)positions)
        : "%rax", "%rcx"
    );
#elif defined(__aarch64__)
    __asm__ __volatile__(
        "mov x0, %1\n"
        "mov x1, %2\n"
        "lsr x0, x0, x1\n"
        "str x0, %0\n"
        : "=m"(result)
        : "r"(a), "r"((int64)positions)
        : "x0", "x1"
    );
#else
    result = a >> positions;
#endif
    return result;
}
