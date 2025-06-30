// Tic-Tac-Toe
// Made by ColorProgrammy
// v1.0 (Github)
//
// Created in Visual Studio C++ 6.0 (Windows XP)
//
// For operating systems Windows 98+,
// but we recommend using Windows XP

#include <windows.h>
#include <tchar.h>
#include "resource.h"

// Global Variables
HINSTANCE g_hInstance;
HWND hButtons[3][3] = { 0 };
TCHAR board[3][3] = { 
    {_T(' '), _T(' '), _T(' ')},
    {_T(' '), _T(' '), _T(' ')},
    {_T(' '), _T(' '), _T(' ')} 
};

int buttonSize = 50;

TCHAR currentPlayer = _T('O');
int winsX = 0, winsO = 0, draws = 0;
bool gameOver = false;
HFONT hButtonFont = NULL;
HWND hStatusBar = NULL;
bool inputBlocked = false;

// Bot Settings
int gameMode = 0;          // 0 = 2 players, 1 = vs Bot
int botDifficulty = 1;     // 0 = easy, 1 = medium, 2 = hard
TCHAR playerSide = _T('O');// Which side player controls (X/O) - default to O
bool botFirstMove = false; // Flag for bot's first move

// Function Prototypes
void CreateGameButtons(HWND hWnd);
void ResetGame();
bool CheckWin(TCHAR player);
bool CheckDraw();
void SaveStats();
void LoadStats();
void UpdateStatusBar();
void DrawButton(LPDRAWITEMSTRUCT pdis);
void BotMove();
bool CheckWinPossible(TCHAR player, int& winRow, int& winCol);
int EvaluateBoard(TCHAR botSide, TCHAR humanSide);
int MiniMax(int depth, bool isMax, TCHAR botSide, TCHAR humanSide, int alpha, int beta);
void MakeBestMove(TCHAR botSide, TCHAR humanSide);
void ShowBotSettingsDialog(HWND hWndParent);
BOOL CALLBACK BotSettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void PlayResourceSound(int soundID);

// Min/Max macros
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// Main Window Procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE: {
            LoadStats();
            
            // Create status bar
            hStatusBar = CreateWindow(
                _T("STATIC"),
                _T("Turn: O"),
                WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_LEFT,
                0, 0, 0, 0,
                hWnd,
                NULL,
                ((LPCREATESTRUCT)lParam)->hInstance,
                NULL
            );
            
            // Create game buttons
            CreateGameButtons(hWnd);
            break;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);
            
            // Handle button clicks
            if (wmId >= IDC_BUTTON1 && wmId <= IDC_BUTTON9 && !gameOver && !inputBlocked) {
				inputBlocked = true;
                int buttonIndex = wmId - IDC_BUTTON1;
                int row = buttonIndex / 3;
                int col = buttonIndex % 3;
                
                if (board[row][col] == _T(' ')) {
                    PlayResourceSound(IDR_CLICK);
                    
                    board[row][col] = currentPlayer;
                    InvalidateRect(hButtons[row][col], NULL, TRUE);
                    UpdateWindow(hButtons[row][col]);
                    
                    if (CheckWin(currentPlayer)) {
                        gameOver = true;
                        if (currentPlayer == _T('X')) winsX++;
                        else winsO++;
                        UpdateStatusBar();
                        
                        InvalidateRect(hWnd, NULL, TRUE);
                        UpdateWindow(hWnd);
                        
                        // Play win sound
                        PlayResourceSound(IDR_TADA);
                        
                        TCHAR winMsg[32];
                        wsprintf(winMsg, _T("%c wins!"), currentPlayer);
                        MessageBox(hWnd, winMsg, _T("Game Over"), MB_OK);
                    } 
                    else if (CheckDraw()) {
                        gameOver = true;
                        draws++;
                        UpdateStatusBar();
                        MessageBox(hWnd, _T("Draw!"), _T("Game Over"), MB_OK);
                    } 
                    else {
                        currentPlayer = (currentPlayer == _T('X')) ? _T('O') : _T('X');
                        UpdateStatusBar();

                        if (gameMode == 1 && currentPlayer != playerSide) {
                            BotMove();
                        }
						else {
							inputBlocked = false; 
						}
                    }
                }
            }
            else if (wmId == IDM_NEWGAME) {
                ResetGame();
            }
            else if (wmId == IDM_EXIT) {
                SaveStats();
                DestroyWindow(hWnd);
            }
            else if (wmId == IDM_STATS) {
                TCHAR statsMsg[128];
                wsprintf(statsMsg, _T("X wins: %d\r\nO wins: %d\r\nDraws: %d"), winsX, winsO, draws);
                MessageBox(hWnd, statsMsg, _T("Statistics"), MB_OK);
            }
            else if (wmId == IDM_TWOPLAYERS) {
                gameMode = 0;
                ResetGame();
            }
            else if (wmId == IDM_VSBOT) {
                ShowBotSettingsDialog(hWnd);
            }
            else if (wmId == IDM_ABOUT) {
                MessageBox(hWnd, _T("Made by ColorProgrammy\nv1.0 (Github)"), 
                           _T("About"), MB_OK | MB_ICONINFORMATION);
            }
			else if (wmId == IDM_HOWTO) {
				MessageBox(hWnd, 
					_T("HOW TO PLAY\n\n")
					_T("Tic-Tac-Toe is a classic game where two players take turns marking\n")
					_T("spaces in a 3x3 grid. The player who succeeds in placing three\n")
					_T("of their marks in a row wins.\n\n")
					_T("GAME RULES:\n")
					_T("1. Player 'O' always goes first\n")
					_T("2. Players alternate placing their symbol in empty squares\n")
					_T("3. Win condition: 3 symbols in a row (horizontal, vertical or diagonal)\n")
					_T("4. If all squares are filled with no winner, the game is a draw\n\n")
					_T("CONTROLS:\n")
					_T("- Click: Place your symbol in an empty square\n")
					_T("- F2: Start a new game\n")
					_T("- Game menu: Switch between players, change bot settings\n")
					_T("- Help menu: View statistics and game information\n\n")
					_T("GAME MODES:\n")
					_T("Two Players:\n")
					_T("   This is a classic mode where you play together on one computer.\n")
					_T("vs Bot:\n")
					_T("   This is a mode where you play with a bot.\n\nThere are 3 difficulty levels:\n")
					_T("   - Easy: Makes random moves\n")
					_T("   - Medium: Blocks obvious wins/loses\n")
					_T("   - Hard: Uses advanced strategy"),
					_T("How to Play"), MB_OK | MB_ICONINFORMATION);
			}
				break;
        }
        
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            if (pdis->CtlType == ODT_BUTTON) {
                DrawButton(pdis);
                return TRUE;
            }
            break;
        }
        
        case WM_KEYDOWN: {
            if (wParam == VK_F2) {
                ResetGame();
            }
            break;
        }
        
        case WM_SIZE: {
            if (hStatusBar) {
                RECT rc;
                GetClientRect(hWnd, &rc);
                int statusHeight = 20;
                SetWindowPos(hStatusBar, NULL, 
                    0, rc.bottom - statusHeight, 
                    rc.right, statusHeight, 
                    SWP_NOZORDER);
            }
            
            RECT rc;
            GetClientRect(hWnd, &rc);
            int spacing = 5;
            int gridSize = 3 * buttonSize + 2 * spacing;
            int startX = (rc.right - gridSize) / 2;
            int startY = (rc.bottom - gridSize - 30) / 2;
            
            for (int row = 0; row < 3; row++) {
                for (int col = 0; col < 3; col++) {
                    if (hButtons[row][col]) {
                        int x = startX + col * (buttonSize + spacing);
                        int y = startY + row * (buttonSize + spacing);
                        SetWindowPos(hButtons[row][col], NULL, x, y, buttonSize, buttonSize, SWP_NOZORDER);
                    }
                }
            }
            break;
        }
        
        case WM_DESTROY: {
            if (hButtonFont) DeleteObject(hButtonFont);
            SaveStats();
            PostQuitMessage(0);
            break;
        }
        
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Entry Point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInstance = hInstance;
    
    // Register window class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wcex.lpszClassName = _T("TicTacToeXPClass");
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));

    if (!RegisterClassEx(&wcex)) {
        MessageBox(NULL, _T("Window class registration failed!"), _T("Error"), MB_ICONERROR);
        return 1;
    }

    // Create main window
    HWND hWnd = CreateWindow(
        _T("TicTacToeXPClass"),
        _T("Tic Tac Toe"),
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        190, 260,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        MessageBox(NULL, _T("Window creation failed!"), _T("Error"), MB_ICONERROR);
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// Create game buttons
void CreateGameButtons(HWND hWndParent) {

    // Font
    hButtonFont = CreateFont(
        42, 0, 0, 0, FW_NORMAL,
        FALSE, FALSE, FALSE, 
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
        DEFAULT_PITCH | FF_DONTCARE, _T("Tahoma") // We'll use the Tahoma font
    );

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            int id = IDC_BUTTON1 + (row * 3 + col);

            hButtons[row][col] = CreateWindow(
                _T("BUTTON"),
                _T(""),
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                0, 0, buttonSize, buttonSize,
                hWndParent,
                (HMENU)id,
                GetModuleHandle(NULL),
                NULL
            );

            // Set button font
            if (hButtonFont) {
                SendMessage(hButtons[row][col], WM_SETFONT, (WPARAM)hButtonFont, TRUE);
            }
        }
    }
}

// Draw colored symbols on buttons with 3D effect
void DrawButton(LPDRAWITEMSTRUCT pdis) {
    TCHAR symbol = _T(' ');
    int row = 0, col = 0;
    bool isOccupied = false;
    
    // Determine button position
    int buttonId = GetDlgCtrlID(pdis->hwndItem);
    if (buttonId >= IDC_BUTTON1 && buttonId <= IDC_BUTTON9) {
        int idx = buttonId - IDC_BUTTON1;
        row = idx / 3;
        col = idx % 3;
        symbol = board[row][col];
        isOccupied = (symbol != _T(' '));
    }
    
    // Set colors
    COLORREF bgColor = GetSysColor(COLOR_BTNFACE);
    COLORREF textColor = RGB(0, 0, 0);
    COLORREF lightColor = GetSysColor(COLOR_3DHIGHLIGHT);
    COLORREF darkColor = GetSysColor(COLOR_3DSHADOW);
    
    if (symbol == _T('X')) textColor = RGB(255, 0, 0);   // Red for X
    else if (symbol == _T('O')) textColor = RGB(0, 0, 255); // Blue for O
    
    // Draw background
    HBRUSH hBgBrush = CreateSolidBrush(bgColor);
    FillRect(pdis->hDC, &pdis->rcItem, hBgBrush);
    DeleteObject(hBgBrush);
    
    // Draw 3D border - reversed for occupied buttons
    HPEN hLightPen = CreatePen(PS_SOLID, 1, lightColor);
    HPEN hDarkPen = CreatePen(PS_SOLID, 1, darkColor);
    HPEN hOldPen = (HPEN)SelectObject(pdis->hDC, isOccupied ? hDarkPen : hLightPen);
    
    if (isOccupied) {
        // Draw sunken (pressed) effect for occupied buttons
        // Top and left edges (dark)
        MoveToEx(pdis->hDC, pdis->rcItem.left, pdis->rcItem.bottom-1, NULL);
        LineTo(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top);
        LineTo(pdis->hDC, pdis->rcItem.right-1, pdis->rcItem.top);
        
        // Bottom and right edges (light)
        SelectObject(pdis->hDC, hLightPen);
        MoveToEx(pdis->hDC, pdis->rcItem.left, pdis->rcItem.bottom-1, NULL);
        LineTo(pdis->hDC, pdis->rcItem.right-1, pdis->rcItem.bottom-1);
        LineTo(pdis->hDC, pdis->rcItem.right-1, pdis->rcItem.top-1);
    } else {
        // Draw raised effect for empty buttons
        // Top and left edges (light)
        MoveToEx(pdis->hDC, pdis->rcItem.left, pdis->rcItem.bottom-1, NULL);
        LineTo(pdis->hDC, pdis->rcItem.left, pdis->rcItem.top);
        LineTo(pdis->hDC, pdis->rcItem.right-1, pdis->rcItem.top);
        
        // Bottom and right edges (dark)
        SelectObject(pdis->hDC, hDarkPen);
        MoveToEx(pdis->hDC, pdis->rcItem.left, pdis->rcItem.bottom-1, NULL);
        LineTo(pdis->hDC, pdis->rcItem.right-1, pdis->rcItem.bottom-1);
        LineTo(pdis->hDC, pdis->rcItem.right-1, pdis->rcItem.top-1);
    }
    
    // Draw symbol
    if (isOccupied) {
        SetBkMode(pdis->hDC, TRANSPARENT);
        SetTextColor(pdis->hDC, textColor);
        RECT rc = pdis->rcItem;
        DrawText(
            pdis->hDC, 
            &symbol, 
            1, 
            &rc, 
            DT_SINGLELINE | DT_CENTER | DT_VCENTER
        );
    }
    
    // Cleanup
    SelectObject(pdis->hDC, hOldPen);
    DeleteObject(hLightPen);
    DeleteObject(hDarkPen);
}

// Play embedded sound resource
void PlayResourceSound(int soundID) {
    PlaySound(MAKEINTRESOURCE(soundID), g_hInstance, 
             SND_RESOURCE | SND_ASYNC | SND_NODEFAULT);
}

// ***Game Logic*** //

// Checking win
bool CheckWin(TCHAR player) {
    // Check rows
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player) 
            return true;
    }
    
    // Check columns
    for (int j = 0; j < 3; j++) {
        if (board[0][j] == player && board[1][j] == player && board[2][j] == player) 
            return true;
    }
    
    // Check diagonals
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player) 
        return true;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player) 
        return true;
    
    return false;
}

// Checking draw
bool CheckDraw() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == _T(' ')) 
                return false;
        }
    }
    return true;
}

// Reset the game
void ResetGame() {
	inputBlocked = false;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            board[i][j] = _T(' ');
        }
    }
    currentPlayer = _T('O');  // O goes first
    gameOver = false;
    
    // Handle first move if playing against bot
    if (gameMode == 1) {
        if (playerSide == _T('O')) {
            // Player is O, bot is X -> player goes first
            botFirstMove = false;
        }
        else {
            // Player is X, bot is O -> bot goes first
            botFirstMove = true;
            BotMove();
        }
    }
    
    // Redraw all buttons
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            InvalidateRect(hButtons[r][c], NULL, TRUE);
        }
    }
    
    UpdateStatusBar();
}

// Save statistics to registry
void SaveStats() {
    HKEY hKey;
    DWORD dwDisposition;
    
    if (RegCreateKeyEx(
        HKEY_CURRENT_USER,
        _T("Software\\TicTacToeXP"),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        &dwDisposition) == ERROR_SUCCESS) {
        
        RegSetValueEx(hKey, _T("WinsX"), 0, REG_DWORD, (BYTE*)&winsX, sizeof(winsX));
        RegSetValueEx(hKey, _T("WinsO"), 0, REG_DWORD, (BYTE*)&winsO, sizeof(winsO));
        RegSetValueEx(hKey, _T("Draws"), 0, REG_DWORD, (BYTE*)&draws, sizeof(draws));
        RegCloseKey(hKey);
    }
}

// Load statistics to registry
void LoadStats() {
    HKEY hKey;
    DWORD dwType, dwSize = sizeof(DWORD);
    
    if (RegOpenKeyEx(
        HKEY_CURRENT_USER,
        _T("Software\\TicTacToeXP"),
        0,
        KEY_READ,
        &hKey) == ERROR_SUCCESS) {
        
        if (RegQueryValueEx(hKey, _T("WinsX"), NULL, &dwType, (LPBYTE)&winsX, &dwSize) != ERROR_SUCCESS)
            winsX = 0;
        
        dwSize = sizeof(DWORD);
        if (RegQueryValueEx(hKey, _T("WinsO"), NULL, &dwType, (LPBYTE)&winsO, &dwSize) != ERROR_SUCCESS)
            winsO = 0;
        
        dwSize = sizeof(DWORD);
        if (RegQueryValueEx(hKey, _T("Draws"), NULL, &dwType, (LPBYTE)&draws, &dwSize) != ERROR_SUCCESS)
            draws = 0;
        
        RegCloseKey(hKey);
    }
}

// Update status bar
void UpdateStatusBar() {
    TCHAR statusText[50];
    if (gameOver) {
        wsprintf(statusText, _T("Game Over"));
    } else {
        wsprintf(statusText, _T("Turn: %c"), currentPlayer);
    }
    SetWindowText(hStatusBar, statusText);
}

// Bot's move
void BotMove() {
    TCHAR botSide = currentPlayer;
    TCHAR humanSide = (botSide == _T('X')) ? _T('O') : _T('X');

	Sleep(500);
	UpdateWindow(GetParent(hStatusBar));
	
    
    // For first move use special logic
    if (botFirstMove) {
        // Center is best for first move
        if (board[1][1] == _T(' ')) {
            board[1][1] = botSide;
            InvalidateRect(hButtons[1][1], NULL, TRUE);
            UpdateWindow(hButtons[1][1]);
        }
        // If center taken, move to corner
        else {
            int corners[4][2] = {{0,0}, {0,2}, {2,0}, {2,2}};
            for (int i = 0; i < 4; i++) {
                int row = corners[i][0];
                int col = corners[i][1];
                if (board[row][col] == _T(' ')) {
                    board[row][col] = botSide;
                    InvalidateRect(hButtons[row][col], NULL, TRUE);
                    UpdateWindow(hButtons[row][col]);
                    break;
                }
            }
        }
        botFirstMove = false;
    } 
    else {
        // Choose algorithm based on difficulty
        switch (botDifficulty) {
            case 0: // Easy - random moves
                {
                    int available[9][2];
                    int count = 0;

                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                            if (board[i][j] == _T(' ')) {
                                available[count][0] = i;
                                available[count][1] = j;
                                count++;
                            }
                        }
                    }
                    
                    if (count > 0) {
                        int index = rand() % count;
                        int row = available[index][0];
                        int col = available[index][1];
                        board[row][col] = botSide;
                        InvalidateRect(hButtons[row][col], NULL, TRUE);
                        UpdateWindow(hButtons[row][col]);
                    }
                }
                break;
                
            case 1: // Medium - basic strategy
                {
                    int winRow = -1, winCol = -1;
                    
                    // 1. Try to win
                    if (CheckWinPossible(botSide, winRow, winCol)) {
                        board[winRow][winCol] = botSide;
                        InvalidateRect(hButtons[winRow][winCol], NULL, TRUE);
                        UpdateWindow(hButtons[winRow][winCol]);
                    }
                    // 2. Block human win
                    else if (CheckWinPossible(humanSide, winRow, winCol)) {
                        board[winRow][winCol] = botSide;
                        InvalidateRect(hButtons[winRow][winCol], NULL, TRUE);
                        UpdateWindow(hButtons[winRow][winCol]);
                    }
                    // 3. Random move
                    else {
                        int available[9][2];
                        int count = 0;
                        
                        for (int i = 0; i < 3; i++) {
                            for (int j = 0; j < 3; j++) {
                                if (board[i][j] == _T(' ')) {
                                    available[count][0] = i;
                                    available[count][1] = j;
                                    count++;
                                }
                            }
                        }
                        
                        if (count > 0) {
                            int index = rand() % count;
                            int row = available[index][0];
                            int col = available[index][1];
                            board[row][col] = botSide;
                            InvalidateRect(hButtons[row][col], NULL, TRUE);
                            UpdateWindow(hButtons[row][col]);
                        }
                    }
                }
                break;
                
            case 2: // Hard - minimax algorithm
                MakeBestMove(botSide, humanSide);
                break;
        }
    }

    PlayResourceSound(IDR_CLICK);

    // Check win/draw conditions
    if (CheckWin(botSide)) {
        gameOver = true;
        if (botSide == _T('X')) winsX++;
        else winsO++;
        UpdateStatusBar();

        HWND hWnd = GetParent(hStatusBar);
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
        
        // Play win sound
        PlayResourceSound(IDR_TADA);
        
        TCHAR winMsg[32];
        wsprintf(winMsg, _T("%c wins!"), botSide);
        MessageBox(hWnd, winMsg, _T("Game Over"), MB_OK);
    } 
    else if (CheckDraw()) {
        gameOver = true;
        draws++;
        UpdateStatusBar();
        MessageBox(GetParent(hStatusBar), _T("Draw!"), _T("Game Over"), MB_OK);
    } 
    else {
        // Pass turn to player
        currentPlayer = playerSide;
        UpdateStatusBar();
    }
	inputBlocked = false;
}

// Check if win is possible with one move
bool CheckWinPossible(TCHAR player, int& winRow, int& winCol) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == _T(' ')) {
                board[i][j] = player;
                bool win = CheckWin(player);
                board[i][j] = _T(' ');
                
                if (win) {
                    winRow = i;
                    winCol = j;
                    return true;
                }
            }
        }
    }
    return false;
}

// Evaluate board for minimax
int EvaluateBoard(TCHAR botSide, TCHAR humanSide) {
    // Check bot win
    if (CheckWin(botSide)) 
        return +10;
    
    // Check human win
    if (CheckWin(humanSide)) 
        return -10;
    
    // Draw
    return 0;
}

// Minimax algorithm for hard difficulty
int MiniMax(int depth, bool isMax, TCHAR botSide, TCHAR humanSide, int alpha, int beta) {
    int score = EvaluateBoard(botSide, humanSide);
    
    // If game over, return score
    if (score != 0 || CheckDraw())
        return score;
    
    if (isMax) {
        int best = -1000;
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == _T(' ')) {
                    board[i][j] = botSide;
                    best = max(best, MiniMax(depth+1, !isMax, botSide, humanSide, alpha, beta));
                    board[i][j] = _T(' ');
                    
                    alpha = max(alpha, best);
                    if (beta <= alpha)
                        return best;
                }
            }
        }
        return best;
    } 
    else {
        int best = 1000;
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == _T(' ')) {
                    board[i][j] = humanSide;
                    best = min(best, MiniMax(depth+1, !isMax, botSide, humanSide, alpha, beta));
                    board[i][j] = _T(' ');
                    
                    beta = min(beta, best);
                    if (beta <= alpha)
                        return best;
                }
            }
        }
        return best;
    }
}

// Make best move using minimax
void MakeBestMove(TCHAR botSide, TCHAR humanSide) {
    int bestVal = -1000;
    int bestRow = -1;
    int bestCol = -1;
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[i][j] == _T(' ')) {
                board[i][j] = botSide;
                int moveVal = MiniMax(0, false, botSide, humanSide, -1000, 1000);
                board[i][j] = _T(' ');
                
                if (moveVal > bestVal) {
                    bestRow = i;
                    bestCol = j;
                    bestVal = moveVal;
                }
            }
        }
    }
    
    if (bestRow != -1 && bestCol != -1) {
        board[bestRow][bestCol] = botSide;
        InvalidateRect(hButtons[bestRow][bestCol], NULL, TRUE);
        UpdateWindow(hButtons[bestRow][bestCol]);
    }
}

// Show bot settings dialog
void ShowBotSettingsDialog(HWND hWndParent) {
    if (DialogBox(
        GetModuleHandle(NULL),
        MAKEINTRESOURCE(IDD_BOT_SETTINGS),
        hWndParent,
        (DLGPROC)BotSettingsDlgProc) == IDOK) {

        gameMode = 1;
        ResetGame();
    }
}

// Bot settings dialog procedure
BOOL CALLBACK BotSettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG:
            // Set difficulty
            SendDlgItemMessage(hDlg, IDC_DIFFICULTY, CB_ADDSTRING, 0, (LPARAM)_T("Easy"));
            SendDlgItemMessage(hDlg, IDC_DIFFICULTY, CB_ADDSTRING, 0, (LPARAM)_T("Medium"));
            SendDlgItemMessage(hDlg, IDC_DIFFICULTY, CB_ADDSTRING, 0, (LPARAM)_T("Hard"));
            SendDlgItemMessage(hDlg, IDC_DIFFICULTY, CB_SETCURSEL, botDifficulty, 0);

            if (playerSide == _T('O')) 
                CheckDlgButton(hDlg, IDC_PLAYER_O, BST_CHECKED);
            else 
                CheckDlgButton(hDlg, IDC_PLAYER_X, BST_CHECKED);
            return TRUE;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    botDifficulty = SendDlgItemMessage(hDlg, IDC_DIFFICULTY, CB_GETCURSEL, 0, 0);

                    if (IsDlgButtonChecked(hDlg, IDC_PLAYER_O) == BST_CHECKED) {
                        playerSide = _T('O');
                    } else {
                        playerSide = _T('X');
                    }
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                    
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}