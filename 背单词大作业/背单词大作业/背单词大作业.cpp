#include <graphics.h>
#include <conio.h>
#include <string>
#include <Windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include "json.hpp"
#include <sstream>

using json = nlohmann::json;

// 窗口大小
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

namespace Colors {
    constexpr COLORREF Background = 0x00FFFFFF;  // 白色 (BBGGRR格式)
    constexpr COLORREF ButtonNormal = 0x00D3D3D3;  // 淡灰色
    constexpr COLORREF ButtonHover = 0x00FFC8C8;  // 浅蓝色
    constexpr COLORREF Text = 0x00000000;  // 黑色
    constexpr COLORREF Title = 0x00960000;  // 深蓝色 (RGB(0,0,150))
}





// 字符串转换函数（提前定义以避免未定义错误）
std::wstring ansiToWstring(const std::string& str);
std::wstring utf8ToWstring(const std::string& str);

// 按钮基类
class Button {
protected:
    int x, y, width, height;
    std::string text;
    bool isHovered;
    COLORREF normalColor, hoverColor;

public:
    Button(int x, int y, int width, int height, const std::string& text,
        COLORREF normalColor = Colors::ButtonNormal,
        COLORREF hoverColor = Colors::ButtonHover)
        : x(x), y(y), width(width), height(height), text(text),
        isHovered(false), normalColor(normalColor), hoverColor(hoverColor) {}

    virtual void draw() {
        setfillcolor(isHovered ? hoverColor : normalColor);
        fillrectangle(x, y, x + width, y + height);

        settextcolor(Colors::Text);
        settextstyle(24, 0, _T("微软雅黑"));

        std::wstring wtext = ansiToWstring(text);
        int textX = x + (width - wtext.length() * 12) / 2;
        int textY = y + (height - 24) / 2;

        outtextxy(textX, textY, wtext.c_str());
    }

    virtual bool isClicked(int mx, int my) {
        return mx >= x && mx <= x + width && my >= y && my <= y + height;
    }

    virtual void checkHover(int mx, int my) {
        isHovered = (mx >= x && mx <= x + width && my >= y && my <= y + height);
    }
};

// 单词结构体
struct Word {
    std::string word;
    std::string meaning;
    int familiarity; // 熟悉度: 0-不熟悉, 1-一般, 2-熟悉
    bool learned;    // 是否已学习

    Word() : familiarity(0), learned(false) {}
};

// 全局单词库
std::vector<Word> wordLibrary;
std::vector<int> unlearnedWords;
std::vector<int> learnedWords;

// 随机数生成器
std::random_device rd;
std::mt19937 gen(rd());

// 字符串转换函数实现
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
bool loadWordLibraryFromJSON() {
    try {
        // 以二进制模式打开文件以避免编码转换问题
        std::ifstream file("words.json", std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("无法打开 words.json 文件");
        }

        json jsonData;
        file >> jsonData;

        wordLibrary.clear();
        unlearnedWords.clear();
        learnedWords.clear();

        for (auto& item : jsonData) {
            Word word;
            word.word = item["word"].get<std::string>();

            if (item.contains("translations") && !item["translations"].empty()) {
                word.meaning = item["translations"][0]["translation"].get<std::string>();
            }
            else {
                word.meaning = "暂无翻译";
            }

            // 检查是否有熟悉度信息
            if (item.contains("familiarity")) {
                word.familiarity = item["familiarity"].get<int>();
                if (word.familiarity > 0) {
                    word.learned = true;
                    learnedWords.push_back(wordLibrary.size());
                }
                else {
                    unlearnedWords.push_back(wordLibrary.size());
                }
            }
            else {
                unlearnedWords.push_back(wordLibrary.size());
            }

            wordLibrary.push_back(word);
        }

        std::cout << "已加载 " << wordLibrary.size() << " 个单词" << std::endl;
        std::cout << "未学习: " << unlearnedWords.size() << ", 已学习: " << learnedWords.size() << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "加载单词库错误: " << e.what() << std::endl;
        return false;
    }
}

// 随机选择一个未学习的单词
int getRandomUnlearnedWord() {
    if (unlearnedWords.empty()) {
        return -1; // 没有未学习的单词
    }

    std::uniform_int_distribution<> dis(0, unlearnedWords.size() - 1);
    return unlearnedWords[dis(gen)];
}

// 随机选择一个已学习的单词用于复习
int getRandomLearnedWord() {
    if (learnedWords.empty()) {
        return -1; // 没有已学习的单词
    }

    // 根据熟悉度进行加权随机选择，熟悉度越低越容易被选中
    std::vector<int> weightedPool;
    for (int idx : learnedWords) {
        int weight = 3 - wordLibrary[idx].familiarity; // 熟悉度0的单词权重为3，熟悉度2的单词权重为1
        for (int i = 0; i < weight; i++) {
            weightedPool.push_back(idx);
        }
    }

    std::uniform_int_distribution<> dis(0, weightedPool.size() - 1);
    return weightedPool[dis(gen)];
}

// 更新单词学习状态
void updateWordStatus(int wordIndex, int newFamiliarity) {
    Word& word = wordLibrary[wordIndex];

    // 记录之前的学习状态
    bool wasLearned = word.learned;

    // 更新熟悉度
    word.familiarity = newFamiliarity;

    // 更新学习状态
    if (newFamiliarity > 0) {
        word.learned = true;

        // 如果之前未学习，将其从未学习列表移至已学习列表
        if (!wasLearned) {
            auto it = std::find(unlearnedWords.begin(), unlearnedWords.end(), wordIndex);
            if (it != unlearnedWords.end()) {
                unlearnedWords.erase(it);
                learnedWords.push_back(wordIndex);
            }
        }
    }
    else {
        word.learned = false;

        // 如果之前已学习，将其从已学习列表移至未学习列表
        if (wasLearned) {
            auto it = std::find(learnedWords.begin(), learnedWords.end(), wordIndex);
            if (it != learnedWords.end()) {
                learnedWords.erase(it);
                unlearnedWords.push_back(wordIndex);
            }
        }
    }
}

// 主界面类
class MainMenu {
private:
    Button* btnLearnNew;
    Button* btnReview;
    std::wstring statusText;

public:
    MainMenu() {
        int btnWidth = 200;
        int btnHeight = 60;
        int centerX = (WINDOW_WIDTH - btnWidth) / 2;

        btnLearnNew = new Button(centerX, 200, btnWidth, btnHeight, "学习新词");
        btnReview = new Button(centerX, 300, btnWidth, btnHeight, "复习单词");

        updateStatusText();
    }

    ~MainMenu() {
        delete btnLearnNew;
        delete btnReview;
    }

    void updateStatusText() {
        std::wstringstream ss;
        ss << L"单词总数: " << wordLibrary.size()
            << L"   已学习: " << learnedWords.size()
            << L"   未学习: " << unlearnedWords.size();
        statusText = ss.str();
    }

    void draw() {
        setbkcolor(Colors::Background);
        cleardevice();

        // 绘制标题
        settextcolor(Colors::Title);
        settextstyle(48, 0, _T("微软雅黑"));
        std::wstring title = L"单词学习助手";
        int titleX = (WINDOW_WIDTH - title.length() * 24) / 2;
        outtextxy(titleX, 50, title.c_str());

        // 绘制状态文本
        settextcolor(Colors::Text);
        settextstyle(16, 0, _T("微软雅黑"));
        outtextxy(50, WINDOW_HEIGHT - 30, statusText.c_str());

        // 绘制按钮
        btnLearnNew->draw();
        btnReview->draw();
    }

    void checkHover(int mx, int my) {
        btnLearnNew->checkHover(mx, my);
        btnReview->checkHover(mx, my);
    }

    int handleClick(int mx, int my) {
        if (btnLearnNew->isClicked(mx, my)) {
            return 1; // 学习新词
        }
        else if (btnReview->isClicked(mx, my)) {
            return 2; // 复习单词
        }
        return 0; // 无操作
    }
};

// 单词学习界面类
class WordLearningScreen {
private:
    Button* btnBack;
    Button* btnNext;
    Button* btnFamiliarity0;
    Button* btnFamiliarity1;
    Button* btnFamiliarity2;

    int currentWordIndex;
    bool isReviewMode;
    std::wstring statusText;

public:
    WordLearningScreen(bool reviewMode = false) : isReviewMode(reviewMode) {
        btnBack = new Button(100, 500, 120, 50, "返回");
        btnNext = new Button(580, 500, 120, 50, "下一个");

        btnFamiliarity0 = new Button(250, 400, 100, 40, "不熟悉", RGB(255, 200, 200), RGB(255, 150, 150));
        btnFamiliarity1 = new Button(370, 400, 100, 40, "一般", RGB(200, 200, 255), RGB(150, 150, 255));
        btnFamiliarity2 = new Button(490, 400, 100, 40, "熟悉", RGB(200, 255, 200), RGB(150, 255, 150));

        // 初始化当前单词
        if (isReviewMode) {
            currentWordIndex = getRandomLearnedWord();
            statusText = L"复习模式";
        }
        else {
            currentWordIndex = getRandomUnlearnedWord();
            statusText = L"学习模式";
        }
    }

    ~WordLearningScreen() {
        delete btnBack;
        delete btnNext;
        delete btnFamiliarity0;
        delete btnFamiliarity1;
        delete btnFamiliarity2;
    }

    void draw() {
        setbkcolor(Colors::Background);
        cleardevice();

        // 绘制状态文本
        settextcolor(Colors::Text);
        settextstyle(16, 0, _T("微软雅黑"));
        outtextxy(50, 30, statusText.c_str());

        if (currentWordIndex >= 0 && currentWordIndex < wordLibrary.size()) {
            Word& word = wordLibrary[currentWordIndex];

            // 绘制单词
            settextcolor(Colors::Title);
            settextstyle(48, 0, _T("微软雅黑"));
            std::wstring wWord = utf8ToWstring(word.word);
            int wordX = (WINDOW_WIDTH - wWord.length() * 24) / 2;
            outtextxy(wordX, 120, wWord.c_str());

            // 绘制释义
            settextcolor(Colors::Text);
            settextstyle(28, 0, _T("微软雅黑"));
            std::wstring wMeaning = utf8ToWstring(word.meaning);
            int len = static_cast<int>(wMeaning.length());
            int displayLen = (len > 30) ? 30 : len;
            int meaningX = (WINDOW_WIDTH - displayLen * 14) / 2;
            outtextxy(meaningX, 200, wMeaning.c_str());

            // 绘制熟悉度按钮
            btnFamiliarity0->draw();
            btnFamiliarity1->draw();
            btnFamiliarity2->draw();
        }
        else {
            // 没有可用单词的提示
            settextcolor(Colors::Text);
            settextstyle(28, 0, _T("微软雅黑"));
            std::wstring message = isReviewMode ? L"没有需要复习的单词" : L"没有新单词可学习";
            int msgX = (WINDOW_WIDTH - message.length() * 14) / 2;
            outtextxy(msgX, 200, message.c_str());
        }

        // 绘制返回和下一个按钮
        btnBack->draw();
        btnNext->draw();
    }

    void checkHover(int mx, int my) {
        btnBack->checkHover(mx, my);
        btnNext->checkHover(mx, my);

        if (currentWordIndex >= 0 && currentWordIndex < wordLibrary.size()) {
            btnFamiliarity0->checkHover(mx, my);
            btnFamiliarity1->checkHover(mx, my);
            btnFamiliarity2->checkHover(mx, my);
        }
    }

    int handleClick(int mx, int my, MainMenu* mainMenu) {
        if (btnBack->isClicked(mx, my)) {
            return 0; // 返回主菜单
        }

        if (currentWordIndex >= 0 && currentWordIndex < wordLibrary.size()) {
            if (btnFamiliarity0->isClicked(mx, my)) {
                updateWordStatus(currentWordIndex, 0);
            }
            else if (btnFamiliarity1->isClicked(mx, my)) {
                updateWordStatus(currentWordIndex, 1);
            }
            else if (btnFamiliarity2->isClicked(mx, my)) {
                updateWordStatus(currentWordIndex, 2);
            }

            // 更新主菜单状态文本
            mainMenu->updateStatusText();
        }

        // 切换到下一个单词
        if (btnNext->isClicked(mx, my)) {
            if (isReviewMode) {
                currentWordIndex = getRandomLearnedWord();
            }
            else {
                currentWordIndex = getRandomUnlearnedWord();
            }
        }

        return 1; // 继续在当前界面
    }
};

// ...前面的头文件和定义保持不变...

int main() {
    // 初始化随机数生成器
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 尝试加载单词库
    if (!loadWordLibraryFromJSON()) {
        // 如果加载失败，创建一个简单的示例单词库
        std::cout << "使用示例单词库..." << std::endl;

        Word w1; w1.word = "apple"; w1.meaning = "苹果";
        Word w2; w2.word = "banana"; w2.meaning = "香蕉";
        Word w3; w3.word = "cherry"; w3.meaning = "樱桃";
        Word w4; w4.word = "dog"; w4.meaning = "狗";
        Word w5; w5.word = "elephant"; w5.meaning = "大象";

        wordLibrary = { w1, w2, w3, w4, w5 };

        // 初始化未学习单词列表
        for (size_t i = 0; i < wordLibrary.size(); i++) {
            unlearnedWords.push_back(i);
        }
    }
    // 初始化图形窗口
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    BeginBatchDraw();  // 启用双缓冲

    // 创建界面对象
    MainMenu mainMenu;
    WordLearningScreen* currentLearningScreen = nullptr;

    int currentScreen = 0; // 0-主菜单，1-学习，2-复习
    bool running = true;

    while (running) {
        // 处理消息
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();

            // 处理点击事件
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (currentScreen == 0) { // 主菜单
                    int action = mainMenu.handleClick(msg.x, msg.y);
                    if (action == 1) { // 学习新词
                        delete currentLearningScreen;
                        currentLearningScreen = new WordLearningScreen(false);
                        currentScreen = 1;
                    }
                    else if (action == 2) { // 复习
                        delete currentLearningScreen;
                        currentLearningScreen = new WordLearningScreen(true);
                        currentScreen = 2;
                    }
                }
                else { // 学习/复习界面
                    int result = currentLearningScreen->handleClick(msg.x, msg.y, &mainMenu);
                    if (result == 0) currentScreen = 0;
                }
            }
        }

        // 开始绘制
        cleardevice();

        // 绘制当前界面
        if (currentScreen == 0) {
            mainMenu.draw();
        }
        else {
            if (currentLearningScreen) currentLearningScreen->draw();
        }

        // 提交绘制
        FlushBatchDraw();
        Sleep(10);  // 降低CPU占用
    }

    // 清理资源
    delete currentLearningScreen;
    EndBatchDraw();
    closegraph();
    return 0;
}