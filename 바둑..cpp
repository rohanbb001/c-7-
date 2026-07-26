#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>      // 기본 입출력 함수 (printf, scanf_s 등)
#include <stdlib.h>     // 문자열 변환 함수 (atoi 등)

#define BOARD_SIZE 9    // 바둑판 크기를 9x9로 정의합니다.

// [1] 바둑판의 각 칸이 가질 수 있는 4가지 상태를 열거형(enum)으로 정의
typedef enum {
    EMPTY = 0,     // [0] 빈 공간 (돌이 놓이지 않음)
    BLACK = 1,     // [1] 흑돌이 놓인 상태
    WHITE = 2,     // [2] 백돌이 놓인 상태
    FORBIDDEN = 3  // [3] 상대 돌을 따낸 직후 발생하는 착수 금지 구역
} Stone;

// [2] 게임 전체 진행 상황 및 바둑판 데이터를 담는 구조체
typedef struct {
    Stone board[BOARD_SIZE][BOARD_SIZE]; // 9x9 바둑판의 상태를 저장하는 2차원 배열
    int blackScore;                      // 흑이 상대 돌을 따낸 개수 (점수)
    int whiteScore;                      // 백이 상대 돌을 따낸 개수 (점수)
    Stone currentTurn;                   // 현재 턴 (BLACK: 흑돌 차례 / WHITE: 백돌 차례)
} GameState;

// --- [자체 구현 1] string.h의 strcmp를 대체하는 항복(gg/GG) 확인 함수 ---
int isSurrender(const char* str) {
    // "gg" 또는 "GG"인지 직접 검사 (1: 항복 맞음, 0: 항복 아님)
    if ((str[0] == 'g' || str[0] == 'G') &&
        (str[1] == 'g' || str[1] == 'G') &&
        str[2] == '\0') {
        return 1;
    }
    return 0;
}

// --- [기능 1] 바둑판에 더 이상 둘 빈자리가 없는지 검사하는 함수 (stdbool 대신 int 사용) ---
int isBoardFull(GameState* state) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            // 빈칸(EMPTY)이 하나라도 있다면 full 상태가 아님 (0 반환)
            if (state->board[i][j] == EMPTY) {
                return 0;
            }
        }
    }
    return 1; // 모든 자리가 가득 차 있음 (1 반환)
}

// --- [기능 2] 착수한 위치(x, y) 주변의 상대방 돌이 포위되었는지 검사하고 따내는 함수 ---
void checkAndCapture(GameState* state, int x, int y) {
    Stone myColor = state->board[x][y];                   // 방금 내가 둔 돌의 색상
    Stone enemyColor = (myColor == BLACK) ? WHITE : BLACK; // 상대방 돌의 색상

    // 상, 하, 좌, 우 4방향 탐색을 위한 시프트 좌표 배열
    int dx[] = { -1, 1, 0, 0 };
    int dy[] = { 0, 0, -1, 1 };

    // 1단계: 내가 둔 돌의 사방(상하좌우)에 상대방 돌이 있는지 확인
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        // 바둑판 경계선 내부 좌표인지 확인
        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
            // 인접한 위치에 상대방 돌이 존재하는 경우
            if (state->board[nx][ny] == enemyColor) {
                int isSurrounded = 1; // 포위 여부 플래그 (1: true, 0: false)

                // 2단계: 찾은 상대방 돌 주변의 4방향이 모두 내 돌로 막혔는지 다시 검사
                for (int j = 0; j < 4; j++) {
                    int nnx = nx + dx[j];
                    int nny = ny + dy[j];

                    if (nnx < 0 || nnx >= BOARD_SIZE || nny < 0 || nny >= BOARD_SIZE) continue;

                    // 사방 중 내 돌이 아닌 공간(빈칸 등)이 하나라도 있다면 포위 실패
                    if (state->board[nnx][nny] != myColor) {
                        isSurrounded = 0;
                        break;
                    }
                }

                // 3단계: 완전히 포위되었다면 상대 돌을 제거하고 점수 획득
                if (isSurrounded) {
                    state->board[nx][ny] = FORBIDDEN; // 따낸 자리는 재착수 방지를 위해 FORBIDDEN 처리
                    if (myColor == BLACK) state->blackScore += 1; // 흑 점수 +1
                    else state->whiteScore += 1;                  // 백 점수 +1

                    printf(">> [%s] Captured enemy stone! (+1 Point)\n",
                        (myColor == BLACK) ? "Black" : "White");
                }
            }
        }
    }
}

// --- [기능 3] 현재 스코어 및 9x9 바둑판 화면 출력 함수 ---
void printBoard(GameState* state) {
    // 점수판 출력
    printf("\n==== [SCORE] Black: %d | White: %d ====\n", state->blackScore, state->whiteScore);

    // 열 번호 (0 ~ 8) 출력
    printf("   ");
    for (int i = 0; i < BOARD_SIZE; i++) printf("%d ", i);
    printf("\n");

    // 행 번호 및 각 칸의 돌 상태 출력
    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i); // 행 번호
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (state->board[i][j] == EMPTY) printf("+ ");           // 빈칸
            else if (state->board[i][j] == BLACK) printf("O ");      // 흑돌
            else if (state->board[i][j] == WHITE) printf("X ");      // 백돌
            else if (state->board[i][j] == FORBIDDEN) printf(". ");  // 착수 금지 구역
        }
        printf("\n");
    }
}

// --- [기능 4] 게임 사용 설명 및 규칙 출력 함수 ---
void printRules(void) {
    printf("==================================================\n");
    printf("             [ BADUK GAME RULES ]                 \n");
    printf("==================================================\n");
    printf(" 1. Input format: '0 0' (Row Column with space)\n");
    printf(" 2. Surrender: Type 'gg' to forfeit\n");
    printf(" 3. Game ends when no more stones can be placed\n");
    printf("==================================================\n\n");
}

// --- [메인 함수] 프로그램의 시작점 ---
int main(void) {
    GameState state;
    // 바둑판 초기화: 처음엔 모든 칸을 빈칸(EMPTY)으로 세팅
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            state.board[i][j] = EMPTY;
        }
    }
    state.blackScore = 0;     // 흑 점수 초기화
    state.whiteScore = 0;     // 백 점수 초기화
    state.currentTurn = BLACK; // 흑돌 선공

    // 게임 시작 시 안내 규칙 출력
    printRules();

    char inputBuf[50]; // 사용자 입력용 임시 문자열 버퍼
    int x, y;          // 입력받은 X, Y 좌표 변수

    // 무한 루프: 게임 진행
    while (1) {
        // [종료 조건 1] 바둑판에 더 이상 돌을 둘 공간이 없으면 자동 종료
        if (isBoardFull(&state)) {
            printf("\n>> [GAME OVER] No empty space on board!\n");
            break; // 루프 탈출
        }

        // 바둑판 화면 출력
        printBoard(&state);

        // 현재 턴 플레이어 안내 및 입력 유도
        printf("\n[%s Turn] Input (Row Col / Surrender: gg) -> ",
            (state.currentTurn == BLACK) ? "Black" : "White");

        // 첫 번째 입력 단어를 읽음 (gg 입력 판단용)
        if (scanf_s("%s", inputBuf, (unsigned)sizeof(inputBuf)) != 1) {
            continue;
        }

        // [종료 조건 2] 입력값이 'gg' 또는 'GG'인 경우 항복으로 판단하여 게임 종료 (자체 함수 사용)
        if (isSurrender(inputBuf)) {
            const char* surrenderPlayer = (state.currentTurn == BLACK) ? "Black" : "White";
            const char* winnerPlayer = (state.currentTurn == BLACK) ? "White" : "Black";

            printf("\n>> [%s] surrendered (gg)!\n", surrenderPlayer);
            printf(">> Winner: %s\n", winnerPlayer);
            break; // 루프 탈출
        }

        // 항복이 아닌 경우: 첫 번째 입력 단어를 정수 X(행) 좌표로 변환
        x = atoi(inputBuf);

        // 두 번째 입력값을 정수 Y(열) 좌표로 읽어옴
        if (scanf_s("%d", &y) != 1) {
            printf(">> Invalid input. Please enter '0 0'\n");
            while (getchar() != '\n'); // 잘못된 입력 문자열 버퍼 비우기
            continue;
        }

        // [착수 예외 처리 1] 좌표가 바둑판 범위를 벗어난 경우
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
            printf(">> Out of bounds! Range: (0 ~ %d)\n", BOARD_SIZE - 1);
            continue;
        }
        // [착수 예외 처리 2] 상대 돌을 따낸 착수 금지 구역인 경우
        if (state.board[x][y] == FORBIDDEN) {
            printf(">> [Forbidden] Cannot place on captured position!\n");
            continue;
        }
        // [착수 예외 처리 3] 이미 다른 돌이 놓여있는 자리의 경우
        if (state.board[x][y] != EMPTY) {
            printf(">> Position already occupied!\n");
            continue;
        }

        // --- 정상 착수 과정 ---
        state.board[x][y] = state.currentTurn; // 배열에 돌 놓기
        checkAndCapture(&state, x, y);         // 포위 따내기 검사 수행

        // 턴 교체 (Black -> White / White -> Black)
        state.currentTurn = (state.currentTurn == BLACK) ? WHITE : BLACK;
    }

    return 0; // 프로그램 정상 종료
}