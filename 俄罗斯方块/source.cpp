#include<iostream>
#include<thread>
#include<vector>
#include<fstream>
#include<string>
using namespace std;

#include<Windows.h>

wstring tetromino[7];
int nFieldWidth = 20;
int nFieldHeight = 30;
unsigned char* GameView = nullptr; // 整个游戏界面

int nScreenWidth = 45;
int nScreenHeight = 30;

int Rotate(int px, int py, int r)
{
    switch (r % 4)
    {
    case 0:return py * 4 + px;         // 旋转0度
    case 1:return 12 + py - (px * 4);  // 旋转90度
    case 2:return 15 - (py * 4) - px;  // 旋转180度
    case 3:return 3 - py + (px * 4);   // 旋转270度
    }
    return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
    for(int px = 0;px<4;px++)
        for (int py = 0;py < 4;py++)
        {
            // Get index into piece
            int pi = Rotate(px, py, nRotation);

            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);
        
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
            {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
                {
                    if (tetromino[nTetromino][pi] == L'X' && GameView[fi] != 0)
                        return false; // fail on first hit
                }
            }
        }


    return true;
}

int main(void)
{
    int nScore = 0;
    // 打开文件
    ifstream ifs;
    ifs.open("Score.txt", ios::in); // 打开文件
    string s;
    vector<int> Score;
    while (getline(ifs, s)) {
        Score.push_back(stoi(s));
    }

    // 创建七种形状
    // 长条：英雄
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    // Z:克利夫兰Z
    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    // S:罗德岛S
    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    // 方块:粉碎男孩
    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    // T:小T
    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"....");

    // 蓝色瑞克
    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");

    // 橘色瑞克
    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");

    GameView = new unsigned char[nFieldWidth * nFieldHeight]; // 一个一维数组存储整个游戏界面
    // 这个二重循环将边界存入GameView里
    for (int x = 0;x < nFieldWidth;x++)
        for (int y = 0;y < nFieldHeight;y++)
            // x==0 左边界,x==nFieldWidth-1 有边界,y==nFieldHeight-1 下边界
            GameView[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0;i < nScreenWidth * nScreenHeight;i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;


    // 游戏逻辑
    bool bGameOver = false;

    int nCurrentPiece = 1;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;
    
    bool bKey[4];
    bool bRotateHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;

    vector<int> vLines;

    while (!bGameOver)
    {
        // GAME TIMING ================================
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);

        // INPUT ======================================
        for (int k = 0;k < 4;k++)
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x26"[k]))) != 0;

        // GAME LOGIC =================================
        nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

        if (bKey[3])
        {
            nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else
            bRotateHold = false;


        if (bForceDown)
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                nCurrentY++; // It can , so do it!
            else
            {
                // Lock the current piece in the field
                for (int px = 0;px < 4;px++)
                    for (int py = 0;py < 4;py++)
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                            GameView[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

                nPieceCount++;
                if (nPieceCount % 10 == 0)
                    if (nSpeed >= 10) nSpeed--;

                // Check have we got any lines
                for (int py=0;py<4;py++)
                    if (nCurrentY + py < nFieldHeight - 1)
                    {
                        bool bLine = true;
                        for (int px = 1;px < nFieldWidth - 1;px++)
                            bLine &= (GameView[(nCurrentY + py) * nFieldWidth + px]) != 0;
                        if (bLine)
                        {
                            // Remove Line,set to =
                            for (int px = 1;px < nFieldWidth - 1;px++)
                                GameView[(nCurrentY + py) * nFieldWidth + px] = 8;
                            
                            vLines.push_back(nCurrentY + py);
                        }
                    }

                if (!vLines.empty()) nScore += (1 << vLines.size()) * 1;

                // Choose next piece 
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;

                // if piece does not fit
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

            }

            nSpeedCounter = 0;
        }

        // RENDER OUTPUT ==============================

        // Draw Field
        for (int x = 0;x < nFieldWidth;x++)
            for (int y = 0;y < nFieldHeight;y++)
                screen[(y + 0) * nScreenWidth + (x + 0)] = L"\u3000红橙黄绿青蓝紫一墙"[GameView[y * nFieldWidth + x]];

        // Draw Current Piece
        for (int px = 0;px < 4;px++)
            for (int py = 0;py < 4;py++)
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                    screen[(nCurrentY + py + 0)*nScreenWidth + (nCurrentX + px + 0)] = L"红橙黄绿青蓝紫一墙"[nCurrentPiece];
        
        // Draw Score
        swprintf_s(& screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"score: %8d", nScore);
        swprintf_s(& screen[6 * nScreenWidth + nFieldWidth + 11], 11, L"HIGH SCORE");
        
        char l, r;
        for (int i = 0;i < 10;i++) {
            if      (i == 0) { l = 's';r = 't'; }
            else if (i == 1) { l = 'n';r = 'd'; }
            else if (i == 2) { l = 'r';r = 'd'; }
            else             { l = 't';r = 'h'; }
            swprintf_s(&screen[(8 + 2 * i) * nScreenWidth + nFieldWidth + 6], 16, L"%3d%c%c: %8d", i + 1, l, r, Score.size()>i? Score[i]:0);
        }

        if (!vLines.empty())
        {
            // Display Frame (cheekily to draw lines)
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
            this_thread::sleep_for(400ms); // Delay a bit
            
            for(auto &v : vLines)
                for (int px = 1;px < nFieldWidth - 1;px++)
                {
                    for (int py = v;py > 0;py--)
                        GameView[py*nFieldWidth+px] = GameView[(py-1) * nFieldWidth + px];
                    GameView[px] = 0;
                }
            
            vLines.clear();

        }

        // Display Frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }
    // 将分数添入资源文件
    if (nScore != 0) {
        Score.insert(find(Score.begin(),Score.end(),nScore), nScore);
    }

    ifs.close();

    ofstream ofs;
    ofs.open("Score.txt",ios::out);
    for (int i = 0;i < Score.size();i++) {
        ofs << Score[i] << endl;
    }
    ofs.close();


    // Oh Dear
    CloseHandle(hConsole);
    cout << "Game Over!! Score:" << nScore << endl;
    system("pause");

    return 0;
}