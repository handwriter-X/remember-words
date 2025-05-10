#include <graphics.h>
#include <conio.h>
#include <string>
#include <Windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

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
};

// 全局单词库
std::vector<Word> wordLibrary;

// 将 ANSI 编码的 std::string 转换为 std::wstring
std::wstring ansiToWstring(const std::string& str) {
    int len;
    int slength = static_cast<int>(str.length()) + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

// 将 UTF-8 编码的 std::string 转换为 std::wstring
std::wstring utf8ToWstring(const std::string& str) {
    int len;
    int slength = static_cast<int>(str.length()) + 1;
    len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

// 从JSON文件加载单词库
void loadWordLibraryFromJSON() {
    try {
        // 以二进制模式打开文件以避免编码转换问题
        std::ifstream file("words.json", std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开 words.json 文件");
        }

        json jsonData;
        file >> jsonData;

        wordLibrary.clear();

        for (auto& item : jsonData) {
            Word word;
            word.word = item["word"].get<std::string>();

            if (item.contains("translations") && !item["translations"].empty()) {
                word.meaning = item["translations"][0]["translation"].get<std::string>();
            }
            else {
                word.meaning = "暂无翻译";
            }

            wordLibrary.push_back(word);
        }

        std::cout << "Loaded " << wordLibrary.size() << " words" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading word library: " << e.what() << std::endl;
    }
}

// 绘制按钮函数
void drawButton(int x, int y, const std::string& text) {
    setfillcolor(LIGHTGRAY);
    fillrectangle(x, y, x + BUTTON_WIDTH, y + BUTTON_HEIGHT);
    settextcolor(BLACK);
    settextstyle(24, 0, _T("微软雅黑"));
    int textX = x + (BUTTON_WIDTH - text.length() * 12) / 2;
    int textY = y + (BUTTON_HEIGHT - 24) / 2;

    // 对于源代码中的硬编码字符串，使用ANSI转换
    std::wstring wtext = ansiToWstring(text);
    outtextxy(textX, textY, wtext.c_str());
}

// 显示单词信息
void showWordInfo(const Word& word) {
    setbkcolor(WHITE);
    cleardevice();
    settextcolor(BLACK);
    settextstyle(36, 0, _T("微软雅黑"));

    // 对于从JSON读取的字符串，使用UTF-8转换
    std::wstring wWord = utf8ToWstring(word.word);
    outtextxy(200, 100, wWord.c_str());

    settextstyle(24, 0, _T("微软雅黑"));
    // 对于从JSON读取的字符串，使用UTF-8转换
    std::wstring wMeaning = utf8ToWstring(word.meaning);
    outtextxy(200, 150, wMeaning.c_str());

    // 对于源代码中的硬编码字符串，使用ANSI转换
    drawButton(150, 500, "返回");
    drawButton(500, 500, "下一词");
}

// 绘制主界面
void drawMainMenu() {
    setbkcolor(WHITE);
    cleardevice();
    // 对于源代码中的硬编码字符串，使用ANSI转换
    drawButton(BUTTON_X, BUTTON_Y, "背新词");
    drawButton(BUTTON_X + BUTTON_WIDTH + 20, BUTTON_Y, "复习");
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    loadWordLibraryFromJSON();

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
                    currentWordIndex = 0;
                    showWordInfo(wordLibrary[currentWordIndex]);
                    isMainMenu = false;
                }
            }
            else {
                if (msg.x >= 150 && msg.x <= 150 + BUTTON_WIDTH &&
                    msg.y >= 500 && msg.y <= 500 + BUTTON_HEIGHT) {
                    drawMainMenu();
                    isMainMenu = true;
                }
                else if (msg.x >= 500 && msg.x <= 500 + BUTTON_WIDTH &&
                    msg.y >= 500 && msg.y <= 500 + BUTTON_HEIGHT) {
                    currentWordIndex = (currentWordIndex + 1) % wordLibrary.size();
                    showWordInfo(wordLibrary[currentWordIndex]);
                }
            }
        }
        if (msg.uMsg == WM_CLOSE) {
            break;
        }
    }

    closegraph();
    return 0;
}