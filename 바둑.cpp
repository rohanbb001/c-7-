#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>

#define BOARD_SIZE 9

typedef enum {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2,
    FORBIDDEN = 3
} Stone;

typedef struct {
    Stone board[BOARD_SIZE][BOARD_SIZE];
    int blackScore;
    int whiteScore;
    Stone currentTurn;
} GameState;

typedef struct MoveNode {
    int moveNum;
    int x, y;
    Stone color;
    struct MoveNode* next;
} MoveNode;

typedef struct {
    MoveNode* front;
    MoveNode* rear;
    int totalMoves;
} MoveQueue;

MoveQueue* createQueue(void) {
    MoveQueue* q = (MoveQueue*)malloc(sizeof(MoveQueue));
    if (q == NULL) return NULL;
    q->front = NULL;
    q->rear = NULL;
    q->totalMoves = 0;
    return q;
}

void enqueueMove(MoveQueue* q, int x, int y, Stone color) {
    if (q == NULL) return;

    MoveNode* newNode = (MoveNode*)malloc(sizeof(MoveNode));
    if (newNode == NULL) return;

    newNode->moveNum = q->totalMoves + 1;
    newNode->x = x;
    newNode->y = y;
    newNode->color = color;
    newNode->next = NULL;

    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
    }
    else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->totalMoves++;
}

void saveGameAndHistory(GameState* state, MoveQueue* q, const char* filename, const char* endReason) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        printf(">> File save failed!\n");
        return;
    }

    fprintf(fp, "=== Baduk Game Result ===\n");
    fprintf(fp, "End Reason: %s\n", endReason);
    fprintf(fp, "Score - Black: %d | White: %d\n\n", state->blackScore, state->whiteScore);
    fprintf(fp, "[Move History]\n");

    MoveNode* curr = q->front;
    while (curr != NULL) {
        fprintf(fp, "%3d Move: %s (%d, %d)\n",
            curr->moveNum,
            (curr->color == BLACK) ? "Black" : "White",
            curr->x, curr->y);
        curr = curr->next;
    }

    fclose(fp);
    printf("\n>> Game record saved: %s\n", filename);
}

bool isBoardFull(GameState* state) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (state->board[i][j] == EMPTY) {
                return false;
            }
        }
    }
    return true;
}

void checkAndCapture(GameState* state, int x, int y) {
    Stone myColor = state->board[x][y];
    Stone enemyColor = (myColor == BLACK) ? WHITE : BLACK;

    int dx[] = { -1, 1, 0, 0 };
    int dy[] = { 0, 0, -1, 1 };

    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE) {
            if (state->board[nx][ny] == enemyColor) {
                bool isSurrounded = true;

                for (int j = 0; j < 4; j++) {
                    int nnx = nx + dx[j];
                    int nny = ny + dy[j];

                    if (nnx < 0 || nnx >= BOARD_SIZE || nny < 0 || nny >= BOARD_SIZE) continue;

                    if (state->board[nnx][nny] != myColor) {
                        isSurrounded = false;
                        break;
                    }
                }

                if (isSurrounded) {
                    state->board[nx][ny] = FORBIDDEN;
                    if (myColor == BLACK) state->blackScore += 1;
                    else state->whiteScore += 1;

                    printf(">> [%s] Captured enemy stone! (+1 Point)\n",
                        (myColor == BLACK) ? "Black" : "White");
                }
            }
        }
    }
}

void printBoard(GameState* state) {
    printf("\n==== [SCORE] Black: %d | White: %d ====\n", state->blackScore, state->whiteScore);

    printf("   ");
    for (int i = 0; i < BOARD_SIZE; i++) printf("%d ", i);
    printf("\n");

    for (int i = 0; i < BOARD_SIZE; i++) {
        printf("%2d ", i);
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (state->board[i][j] == EMPTY) printf("+ ");
            else if (state->board[i][j] == BLACK) printf("O ");
            else if (state->board[i][j] == WHITE) printf("X ");
            else if (state->board[i][j] == FORBIDDEN) printf(". ");
        }
        printf("\n");
    }
}

void printRules(void) {
    printf("==================================================\n");
    printf("             [ BADUK GAME RULES ]                 \n");
    printf("==================================================\n");
    printf(" 1. Input format: '0 0' (Row Column with space)\n");
    printf(" 2. Surrender: Type 'gg' to forfeit\n");
    printf(" 3. Game ends when no more stones can be placed\n");
    printf("==================================================\n\n");
}

int main(void) {
    SetConsoleOutputCP(65001);

    GameState state;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            state.board[i][j] = EMPTY;
        }
    }
    state.blackScore = 0;
    state.whiteScore = 0;
    state.currentTurn = BLACK;

    MoveQueue* historyQueue = createQueue();
    if (historyQueue == NULL) {
        printf("Memory allocation error.\n");
        return 1;
    }

    printRules();

    char inputBuf[50];
    int x, y;

    while (1) {
        if (isBoardFull(&state)) {
            printf("\n>> [GAME OVER] No empty space on board!\n");
            saveGameAndHistory(&state, historyQueue, "goban_result.txt", "Board Full");
            break;
        }

        printBoard(&state);

        printf("\n[%s Turn] Input (Row Col / Surrender: gg) -> ",
            (state.currentTurn == BLACK) ? "Black" : "White");

        if (scanf_s("%s", inputBuf, (unsigned)sizeof(inputBuf)) != 1) {
            continue;
        }

        if (strcmp(inputBuf, "gg") == 0 || strcmp(inputBuf, "GG") == 0) {
            const char* surrenderPlayer = (state.currentTurn == BLACK) ? "Black" : "White";
            const char* winnerPlayer = (state.currentTurn == BLACK) ? "White" : "Black";

            printf("\n>> [%s] surrendered (gg)!\n", surrenderPlayer);
            printf(">> Winner: %s\n", winnerPlayer);

            saveGameAndHistory(&state, historyQueue, "goban_result.txt", "Surrender (gg)");
            break;
        }

        x = atoi(inputBuf);

        if (scanf_s("%d", &y) != 1) {
            printf(">> Invalid input. Please enter '0 0'\n");
            while (getchar() != '\n');
            continue;
        }

        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
            printf(">> Out of bounds! Range: (0 ~ %d)\n", BOARD_SIZE - 1);
            continue;
        }
        if (state.board[x][y] == FORBIDDEN) {
            printf(">> [Forbidden] Cannot place on captured position!\n");
            continue;
        }
        if (state.board[x][y] != EMPTY) {
            printf(">> Position already occupied!\n");
            continue;
        }

        state.board[x][y] = state.currentTurn;
        enqueueMove(historyQueue, x, y, state.currentTurn);
        checkAndCapture(&state, x, y);

        state.currentTurn = (state.currentTurn == BLACK) ? WHITE : BLACK;
    }

    MoveNode* curr = historyQueue->front;
    while (curr != NULL) {
        MoveNode* temp = curr;
        curr = curr->next;
        free(temp);
    }
    free(historyQueue);

    return 0;
}