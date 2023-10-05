#include "game.h"

#include <windows.h>

#include <thread>
#include <vector>
#include <string>

namespace Game {
  std::wstring tetromino[7];
  // ------ field ------ 
  int nFieldWith = 12;
  int nFieldHeight = 18;
  uint8_t* pField;
  // ------ screen ----- 
  wchar_t* screen = nullptr;
  int nScreenWidth = 80;
  int nScreenHeight = 30;
  HANDLE hConsole = nullptr;
  HANDLE outputHandle = nullptr;

  int Rotate(int px, int py, int r) {
    switch (r % 4)
    {
      case 0: return py * 4 + px;
      case 1: return 12 + py - (px * 4);
      case 2: return 15 - (py * 4) - px;
      case 3: return 3 - py + px * 4;
    }
    return 0;
  }

  void InitTetromino() {
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L"..X.");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"..X.");
    tetromino[3].append(L"..X.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L".X..");
    tetromino[4].append(L".X..");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"....");

    tetromino[5].append(L".XX.");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"....");
    tetromino[5].append(L"....");

    tetromino[6].append(L".X..");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L"..X.");
    tetromino[6].append(L"....");
  }

  void InitField() {
    pField = new uint8_t[nFieldHeight * nFieldWith];
    for(int x=0; x < nFieldWith; x++)
      for (int y = 0; y < nFieldHeight; y++)
        pField[y * nFieldWith + x] = (x == 0 || x == nFieldWith - 1 || y == nFieldHeight - 1) ? 9 : 0;
  }

  void InitScreenBuf() {
    hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
  }

  bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
    for(int px = 0; px < 4; px++) {
      for(int py = 0; py < 4 ; py++) {
        int pi = Rotate(px, py, nRotation);

        int fi = (nPosY + py) * nFieldWith + (nPosX + px);

        if(0 <= nPosX + px && nPosX + px < nFieldWith) {
          if(0 <= nPosY + py && nPosY + py < nFieldHeight) {
            if(tetromino[nTetromino][pi] == L'X' && pField[fi] != 0) {
              return false;
            }
          }
        }
      }
    }
    return true;
  }

  void Tetris() {
    InitTetromino();
    InitField();
    InitScreenBuf();

    bool bGameOver = false;

    // running states
    int nCurrentPiece = 3;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWith / 2;
    int nCurrentY = 0;
    printf("current x %5d y %5d\n", nCurrentX, nCurrentY);

    bool bKey[4];

    screen = new wchar_t[nScreenHeight * nScreenWidth];
    for(int i=0; i< nScreenHeight* nScreenWidth; i++) screen[i] = L' ';
    // HandleScreenSize();
    DWORD nBufWritten = 0;
    bool bHoldKey = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    std::vector<int> vLines;

    while(!bGameOver) {

      // GAME TIMING ===========================
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      nSpeedCounter++;
      bForceDown = (nSpeedCounter == nSpeed);

      // INPUT ================================
      for(int k = 0; k < 4; k++) {
        // R L D Z
        bKey[k] = (0x8000 & GetAsyncKeyState((uint8_t)("\x27\x25\x28Z"[k]))) != 0;
      }

      // GAME LOGIC ==============================
      if(bKey[1])
        if(DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))
          nCurrentX = nCurrentX - 1;

      if(bKey[0])
        if(DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) 
          nCurrentX = nCurrentX + 1;

      if(bKey[2])
        if(DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX , nCurrentY + 1)) 
          nCurrentY = nCurrentY + 1;
      
      if(bKey[3]) {
        nCurrentRotation += (!bHoldKey && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
        bHoldKey = true;
      }else 
        bHoldKey = false;

      if(bForceDown) {
        if(DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
          nCurrentY++;
        }else {
          // 
          for(int x = 0; x < 4; x++)
            for(int y = 0; y < 4; y++) 
              if(tetromino[nCurrentPiece][Rotate(x, y, nCurrentRotation)] == L'X')
                pField[(nCurrentY + y) * nFieldWith + nCurrentX + x] = nCurrentPiece + 1;
          
          for(int py = 0; py < 4; py++) {
            if(nCurrentY + py < nFieldHeight - 1) {
              bool bLine = true;
              for(int px = 1; px < nFieldWith - 1; px++) {
                bLine &= (pField[(nCurrentY + py) * nFieldWith + px] != 0);
              }

              if(bLine) {
                for (int px = 1; px < nFieldWith - 1; px++) {
                  pField[(nCurrentY + py) * nFieldWith + px] = 8;
                }
                vLines.push_back(nCurrentY + py);
              }
            }
          }

          // next piece
          nCurrentX = nFieldWith / 2;
          nCurrentY = 0;
          nCurrentRotation = 0;
          nCurrentPiece = rand() % 7;

          bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
        }

        nSpeedCounter = 0;
      }


      // ---- write data ---- 
      for(int x = 0; x < nFieldWith; x++)
        for(int y = 0; y < nFieldHeight; y++) 
          screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWith + x]];
      
      for(int px = 0; px < 4; px++) {
        for(int py = 0; py < 4; py++) {
          if(tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
            screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
        }
      }

      if(!vLines.empty()) {
        #ifdef _MSC_VER
          WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, {0, 0}, &nBufWritten);
        #else 
          WriteConsoleOutputCharacterW(hConsole, screen, nScreenHeight * nScreenWidth, {0, 0}, &nBufWritten);
        #endif
        std::this_thread::sleep_for(std::chrono::milliseconds(400));

        for(auto& v: vLines) {
          for(int px=1; px < nFieldWith - 1; px++) {
            for(int py = v; py > 0; py--) {
              pField[py * nFieldWith + px] = pField[(py - 1) * nFieldWith + px];
            }
            pField[px] = 0;
          }
        }

        vLines.clear();
      }

      #ifdef _MSC_VER
        WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, {0, 0}, &nBufWritten);
      #else 
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenHeight * nScreenWidth, {0, 0}, &nBufWritten);
      #endif
    }
  }
}