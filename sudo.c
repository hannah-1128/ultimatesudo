#include <stdio.h>
#include <stdlib.h>

// === 結構定義 ===
typedef struct {
    int numbers;    // 題目總數
    int datasize;   // 每題資料大小（byte）
} SudokuDataHeader;

typedef struct {
    int id;
    int data[9][9];
} SudokuProblem;

// === 題庫 ===
SudokuProblem sample_problems[] = {
    {1, {
        {5,3,0,0,7,0,0,0,0},
        {6,0,0,1,9,5,0,0,0},
        {0,9,8,0,0,0,0,6,0},
        {8,0,0,0,6,0,0,0,3},
        {4,0,0,8,0,3,0,0,1},
        {7,0,0,0,2,0,0,0,6},
        {0,6,0,0,0,0,2,8,0},
        {0,0,0,4,1,9,0,0,5},
        {0,0,0,0,8,0,0,7,9}
    }},
    {2, {
        {0,0,0,2,6,0,7,0,1},
        {6,8,0,0,7,0,0,9,0},
        {1,9,0,0,0,4,5,0,0},
        {8,2,0,1,0,0,0,4,0},
        {0,0,4,6,0,2,9,0,0},
        {0,5,0,0,0,3,0,2,8},
        {0,0,9,3,0,0,0,7,4},
        {0,4,0,0,5,0,0,3,6},
        {7,0,3,0,1,8,0,0,0}
    }},
    {3, {
        {0,2,0,6,0,8,0,0,0},
        {5,8,0,0,0,9,7,0,0},
        {0,0,0,0,4,0,0,0,0},
        {3,7,0,0,0,0,5,0,0},
        {6,0,0,0,0,0,0,0,4},
        {0,0,8,0,0,0,0,1,3},
        {0,0,0,0,2,0,0,0,0},
        {0,0,9,8,0,0,0,3,6},
        {0,0,0,3,0,6,0,9,0}
    }}
};

// === 全域變數 ===
int player_board[9][9];
int answer_board[9][9];
int original_board[9][9];
int error_count = 0;

// === 函式宣告 ===
void print_board(int board[][9]);
void save_to_text_file(int board[][9], const char* filename);
int read_from_binary_file(int board[][9], const char* filename, int problem_index);
void save_sudoku_problems(const char* filename, SudokuProblem* problems, int count);
void init_game(int board[9][9]);
void handle_input(int row, int col, int value);
int is_complete();
int count_empty();
void game_loop(int board[9][9]);
void clear_input();
int is_safe(int board[9][9], int row, int col, int num);
int solve_sudoku(int board[9][9]);

// === 主程式 ===
int main() {
    save_sudoku_problems("sudoku.dat", sample_problems, 3);

    int board[9][9];
    int idx;

    while (1) {
        printf("請輸入你想玩的題號 (1~3)：");
        if (scanf("%d", &idx) != 1) break;
        clear_input();

        if (idx < 1 || idx > 3) {
            printf("題號錯誤，請重新輸入。\n");
            continue;
        }

        if (!read_from_binary_file(board, "sudoku.dat", idx - 1)) {
            printf("讀取失敗，請重新輸入。");
            continue;
        }

        // 只顯示題目，不顯示解答
        // printf("\n本題如下：\n");
        // print_board(board);

        save_to_text_file(board, "exported_sudoku.txt");
        game_loop(board);

        printf("是否要再玩一場？(y/n)：");
        char choice;
        if (scanf(" %c", &choice) != 1 || choice != 'y') break;
        clear_input();
    }

    return 0;
}

// === 儲存題庫 ===
void save_sudoku_problems(const char* filename, SudokuProblem* problems, int count) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) return;

    SudokuDataHeader header = { count, sizeof(SudokuProblem) };
    fwrite(&header, sizeof(header), 1, fp);

    for (int i = 0; i < count; i++)
        fwrite(&problems[i], sizeof(SudokuProblem), 1, fp);

    fclose(fp);
}

// === 印出盤面 ===
void print_board(int board[][9]) {
    // 強制刷新 stdout，避免緩衝區導致終端機顯示不即時
    fflush(stdout);
    printf("\n     1  2  3   4  5  6   7  8  9\n");
    printf("   ┌─────────┬─────────┬─────────┐\n");
    for (int i = 0; i < 9; i++) {
        printf(" %d │", i + 1);
        for (int j = 0; j < 9; j++) {
            if (board[i][j])
                printf(" %d", board[i][j]);
            else
                printf(" _");
            if ((j + 1) % 3 == 0) printf(" │");
            else printf(" ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0 && i != 8)
            printf("   ├─────────┼─────────┼─────────┤\n");
    }
    printf("   └─────────┴─────────┴─────────┘\n");
    fflush(stdout);
}

// === 儲存文字檔 ===
void save_to_text_file(int board[][9], const char* filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return;
    // 輸出美化後的數獨盤面到文字檔
    fprintf(fp, "\n     1  2  3   4  5  6   7  8  9\n");
    fprintf(fp, "   ┌─────────┬─────────┬─────────┐\n");
    for (int i = 0; i < 9; i++) {
        fprintf(fp, " %d │", i + 1);
        for (int j = 0; j < 9; j++) {
            if (board[i][j])
                fprintf(fp, " %d", board[i][j]);
            else
                fprintf(fp, " _");
            if ((j + 1) % 3 == 0) fprintf(fp, " │");
            else fprintf(fp, " ");
        }
        fprintf(fp, "\n");
        if ((i + 1) % 3 == 0 && i != 8)
            fprintf(fp, "   ├─────────┼─────────┼─────────┤\n");
    }
    fprintf(fp, "   └─────────┴─────────┴─────────┘\n");
    fclose(fp);
}

// === 載入題目 ===
int read_from_binary_file(int board[][9], const char* filename, int problem_index) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return 0;

    SudokuDataHeader header;
    fread(&header, sizeof(header), 1, fp);
    if (problem_index < 0 || problem_index >= header.numbers) {
        fclose(fp);
        return 0;
    }

    fseek(fp, sizeof(header) + problem_index * header.datasize, SEEK_SET);
    SudokuProblem problem;
    fread(&problem, sizeof(problem), 1, fp);
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            board[i][j] = problem.data[i][j];
    fclose(fp);
    return 1;
}

// === 初始化 ===
void init_game(int board[9][9]) {
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++) {
            player_board[i][j] = board[i][j];
            original_board[i][j] = board[i][j];
        }
    // 正確答案應該用自動解題產生
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            answer_board[i][j] = board[i][j];
    solve_sudoku(answer_board);
    error_count = 0;
}

// === 錯誤提示 ===
void handle_input(int row, int col, int value) {
    if (original_board[row][col]) {
        printf("此格不可填寫！\n");
        return;
    }

    if (value != answer_board[row][col]) {
        printf("你填入的答案錯誤！\n");
        printf("請再輸入一次你認為正確的答案：");
        int try_value;
        if (scanf("%d", &try_value) == 1) {
            clear_input();
            if (try_value == answer_board[row][col]) {
                printf("✅ 恭喜你，這次答對了！\n");
                player_board[row][col] = try_value;
                return;
            }
        }
        printf("是否要顯示此格正確答案？(y/n)：");
        char show_ans = 0;
        if (scanf(" %c", &show_ans) == 1) {
            clear_input();
            if (show_ans == 'y' || show_ans == 'Y') {
                printf("正確答案是：%d\n", answer_board[row][col]);
                player_board[row][col] = answer_board[row][col]; // 顯示答案時直接填入正確答案
                return;
            }
        }
        // 若沒填對也沒選擇顯示答案，維持空格
        player_board[row][col] = 0;
        error_count++;
    } else {
        player_board[row][col] = value;
        printf("✅ 正確\n");
    }
}

// === 檢查是否完成 ===
int is_complete() {
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (player_board[i][j] == 0)
                return 0;
    return 1;
}

// === 空格計算 ===
int count_empty() {
    int count = 0;
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (player_board[i][j] == 0)
                count++;
    return count;
}

// === 遊戲流程 ===
void game_loop(int board[9][9]) {
    int reload = 0;
    do {
        init_game(board);
        while (!is_complete()) {
            print_board(player_board);
            printf("剩餘空格：%d\n", count_empty());
            printf("請輸入「行 列 數字」（例如 3 4 7），或輸入 0 0 0 重新載入本題：");

            int r, c, v;
            if (scanf("%d %d %d", &r, &c, &v) != 3) break;
            clear_input();

            if (r == 0 && c == 0 && v == 0) {
                reload = 1;
                break;
            }

            if (r < 1 || r > 9 || c < 1 || c > 9 || v < 1 || v > 9) {
                printf("輸入錯誤，請重新輸入 1~9\n");
                continue;
            }

            handle_input(r - 1, c - 1, v);
        }

        if (!reload) {
            print_board(player_board);
            printf("🎉 恭喜完成！總共錯誤 %d 次。\n", error_count);
        }
    } while (reload);
}

// 清空多餘輸入
void clear_input() {
    while (getchar() != '\n');
}

// === 自動解題 ===
int is_safe(int board[9][9], int row, int col, int num) {
    for (int x = 0; x < 9; x++)
        if (board[row][x] == num || board[x][col] == num)
            return 0;
    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[startRow + i][startCol + j] == num)
                return 0;
    return 1;
}

int solve_sudoku(int board[9][9]) {
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            if (board[row][col] == 0) {
                for (int num = 1; num <= 9; num++) {
                    if (is_safe(board, row, col, num)) {
                        board[row][col] = num;
                        if (solve_sudoku(board))
                            return 1;
                        board[row][col] = 0;
                    }
                }
                return 0;
            }
        }
    }
    return 1;
}