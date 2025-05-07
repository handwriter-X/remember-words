#include <graphics.h>
#include <conio.h>
#include <string>
#include <Windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>

// 窗口大小
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// 选项按钮的大小和位置
const int BUTTON_WIDTH = 150;
const int BUTTON_HEIGHT = 50;
const int BUTTON_X = (WINDOW_WIDTH - 2 * BUTTON_WIDTH - 20) / 2;
const int BUTTON_Y = (WINDOW_HEIGHT - BUTTON_HEIGHT) / 2;

// 单词结构体
struct Word {
    std::string word;
    std::string meaning;
    std::string example;
};

// 模拟词库
std::vector<Word> wordLibrary = {
    {"abandon", "放弃；遗弃", "He had to abandon his plan."},
    {"ability", "能力；才能", "She has the ability to solve difficult problems."},
    {"able", "能够；有能力的", "I'll be able to come tomorrow."}
    
};

// 将 std::string 转换为 std::wstring
std::wstring stringToWstring(const std::string& str) {
    int len;
    int slength = static_cast<int>(str.length()) + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

// 绘制按钮函数
void drawButton(int x, int y, const std::string& text) {
    setfillcolor(LIGHTGRAY);
    fillrectangle(x, y, x + BUTTON_WIDTH, y + BUTTON_HEIGHT);
    settextcolor(BLACK);
    settextstyle(24, 0, _T("微软雅黑"));
    int textX = x + (BUTTON_WIDTH - text.length() * 12) / 2;
    int textY = y + (BUTTON_HEIGHT - 24) / 2;
    std::wstring wtext = stringToWstring(text);
    outtextxy(textX, textY, wtext.c_str());
}

// 显示单词信息
void showWordInfo(const Word& word) {
    setbkcolor(WHITE);
    cleardevice();
    settextcolor(BLACK);
    settextstyle(36, 0, _T("微软雅黑"));
    std::wstring wWord = stringToWstring(word.word);
    outtextxy(200, 100, wWord.c_str());

    settextstyle(24, 0, _T("微软雅黑"));
    std::wstring wMeaning = stringToWstring(word.meaning);
    outtextxy(200, 150, wMeaning.c_str());

    std::wstring wExample = stringToWstring(word.example);
    outtextxy(200, 200, wExample.c_str());

    // 绘制返回按钮
    drawButton(150, 500, "返回");
    // 绘制下一词按钮
    drawButton(500, 500, "下一词");
}

// 绘制主界面
void drawMainMenu() {
    setbkcolor(WHITE);
    cleardevice();
    // 绘制“背新词”按钮
    drawButton(BUTTON_X, BUTTON_Y, "背新词");
    // 绘制“复习”按钮
    drawButton(BUTTON_X + BUTTON_WIDTH + 20, BUTTON_Y, "复习");
}

int main() {
    // 初始化随机数种子
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 初始化图形窗口
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);

    drawMainMenu();

    MOUSEMSG msg;
    bool isMainMenu = true;
    int currentWordIndex = 0;

    while (true) {
        msg = GetMouseMsg();
        if (msg.uMsg == WM_LBUTTONDOWN) {
            if (isMainMenu) {
                if (msg.x >= BUTTON_X && msg.x <= BUTTON_X + BUTTON_WIDTH &&
                    msg.y >= BUTTON_Y && msg.y <= BUTTON_Y + BUTTON_HEIGHT) {
                    // 点击“背新词”按钮
                    currentWordIndex = 0;
                    showWordInfo(wordLibrary[currentWordIndex]);
                    isMainMenu = false;
                }
            }
            else {
                if (msg.x >= 150 && msg.x <= 150 + BUTTON_WIDTH &&
                    msg.y >= 500 && msg.y <= 500 + BUTTON_HEIGHT) {
                    // 点击“返回”按钮
                    drawMainMenu();
                    isMainMenu = true;
                }
                else if (msg.x >= 500 && msg.x <= 500 + BUTTON_WIDTH &&
                    msg.y >= 500 && msg.y <= 500 + BUTTON_HEIGHT) {
                    // 点击“下一词”按钮
                    currentWordIndex = (currentWordIndex + 1) % wordLibrary.size();
                    showWordInfo(wordLibrary[currentWordIndex]);
                }
            }
        }
        if (msg.uMsg == WM_CLOSE) {
            break;
        }
    }

    // 关闭图形窗口
    closegraph();

    return 0;
}