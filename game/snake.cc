#include <windows.h>
#include <list>
#include <thread>

#include "game.h"

namespace Game {

  // ------ screen ----- 
  static wchar_t* screen = nullptr;
  static int nScreenWidth = 120;
  static int nScreenHeight = 40;
  static HANDLE hConsole = nullptr;

  struct sSnakeSegment 
  {
    int X;
    int Y;
  };

  typedef std::list<sSnakeSegment> snake_t ;

  void Snake() {

    // create screen buffer
    screen = new wchar_t[nScreenHeight * nScreenWidth];
    for(int i=0; i< nScreenHeight* nScreenWidth; i++) screen[i] = L' ';
    hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    snake_t snake = {{60, 15}, {61, 15}, {62, 15}, {63, 15}};
    int nFoodX = 30;
    int nFoodY = 15;
    int nScore = 0;
    int nSnakeDirection = 3;
    bool bDead = false;

    bool bKeyRight = false, bKeyLeft = false, bKeyLeftOld = false, bKeyRightOld = false; 

    while(true) {

      // Timing & Input
      auto time = std::chrono::system_clock::now();
      while((std::chrono::system_clock::now() - time) < std::chrono::milliseconds(200)) {
        bKeyRight = (0x8000 & GetAsyncKeyState((uint8_t)('\x27'))) != 0;
        bKeyLeft  = (0x8000 & GetAsyncKeyState((uint8_t)('\x25'))) != 0;

        if(bKeyRight && !bKeyRightOld) {
          nSnakeDirection++;
          if(nSnakeDirection == 4) nSnakeDirection = 0;
        }

        if(bKeyLeft && !bKeyLeftOld) {
          nSnakeDirection--;
          if(nSnakeDirection == -1) nSnakeDirection = 3;
        }
      }

      bKeyLeftOld   = bKeyLeft;
      bKeyRightOld  = bKeyRight;

      switch(nSnakeDirection) {
        case 0: // up
          snake.push_front({snake.front().X, snake.front().Y - 1});
          break;
        case 1: // RIGHT
          snake.push_front({snake.front().X + 1, snake.front().Y});
          break;
        case 2: // down
          snake.push_front({snake.front().X, snake.front().Y + 1});
          break;
        case 3: // left
          snake.push_front({snake.front().X - 1, snake.front().Y });
          break;
      };

      if(snake.front().X < 0 || snake.front().X >= nScreenWidth) {
        bDead = true;
      }
      if(snake.front().Y < 3 || snake.front().Y >= nScreenHeight) {
        bDead = true;
      }

      if(snake.front().X == nFoodX && snake.front().Y == nFoodY) {
        nScore++;
        while(screen[nFoodY * nScreenWidth + nFoodX] != L' ') {
          nFoodX = rand() % nScreenWidth;
          nFoodY = rand() % (nScreenHeight -3) + 3;
        }

        for(int i=0; i < 5; i++) {
          snake.push_back({snake.back().X, snake.back().Y});
        }
      }

      for(snake_t::iterator iter = snake.begin() ; iter != snake.end(); iter++)
        if(iter != snake.begin() && iter->X == snake.front().X && iter->Y == snake.front().Y) {
          bDead = true;
          break;
        }

      snake.pop_back();

      // Display to player 
      // Clear Screen 
      for(int i=0; i< nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

      // Draw Stats & Border
      for(int i =0; i < nScreenWidth; i++) {
        screen[i] = L'=';
        screen[2 * nScreenWidth + i] = L'=';
      }

      #ifdef _MSC_VER
        wsprintf(&screen[nScreenWidth + 5], L"www.OneLoneCoder.com - S N A K E ! !        score: %d food(%d, %d)", nScore, nFoodX, nFoodY);
      #else 
        wsprintfW(&screen[nScreenWidth + 5], L"www.OneLoneCoder.com - S N A K E ! !        score: %d food(%d, %d)", nScore, nFoodX, nFoodY);
      #endif

      // Draw Snake Body
      for (auto s : snake) 
        screen[s.Y * nScreenWidth + s.X] = bDead ? L'+' : L'O';
      
      screen[snake.front().Y * nScreenWidth + snake.front().X] = bDead ? L'X' : L'@';

      screen[nFoodY * nScreenWidth + nFoodX] = L'%';

      #ifdef _MSC_VER
        WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, {0, 0}, &dwBytesWritten);
      #else 
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenHeight * nScreenWidth, {0, 0}, &dwBytesWritten);
      #endif
    }

  };
}