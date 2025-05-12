#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#pragma execution_character_set("utf-8")
#include <windows.h>
#include <graphics.h>
#include <conio.h>
#include <string>
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
const int WINDOW_WIDTH = 500;
const int WINDOW_HEIGHT = 600;

// 更新后的颜色定义
namespace Colors {
    constexpr COLORREF Background = 0x00F8F9FA;    // 浅灰色背景
    constexpr COLORREF Title = 0x002B2D42;    // 深蓝灰色
    constexpr COLORREF Text = 0x004A4A4A;    // 深灰色
    constexpr COLORREF ButtonNormal = 0x00E9ECEF;    // 浅灰色按钮
    constexpr COLORREF ButtonHover = 0x00CED4DA;    // 中灰色悬停
    constexpr COLORREF ButtonText = 0x00FFFFFF;    // 白色按钮文字
    constexpr COLORREF CardBg = 0x00FFFFFF;    // 白色卡片
    constexpr COLORREF Progress = 0x0076C893;    // 绿色进度条
    constexpr COLORREF Familiar0 = 0x00FF6B6B;    // 珊瑚红
    constexpr COLORREF Familiar1 = 0x00FFD93D;    // 活力黄
    constexpr COLORREF Familiar2 = 0x006CBF84;    // 森林绿
    constexpr COLORREF Subtitle = 0x006C757D;    // 中性灰
}

// 字符串转换函数（提前定义以避免未定义错误）
std::wstring utf8ToWstring(const std::string& str);

// 修改后的按钮基类（增加圆角半径和文本颜色参数）
class Button {
protected:
    int x, y, width, height;
    std::string text; // 保留原始字符串
    std::wstring wtext; // 缓存宽字符版本
    bool isHovered;
    COLORREF normalColor, hoverColor, textColor;
    int radius;

public:
    // 构造函数重载 - 接受窄字符字符串
    Button(int x, int y, int width, int height, const std::string& text,
        COLORREF normalColor = Colors::ButtonNormal,
        COLORREF hoverColor = Colors::ButtonHover,
        COLORREF textColor = Colors::ButtonText,
        int radius = 8)
        : x(x), y(y), width(width), height(height), text(text),
        wtext(utf8ToWstring(text)), // 立即转换为宽字符
        isHovered(false), normalColor(normalColor),
        hoverColor(hoverColor), textColor(textColor),
        radius(radius) {}

    // 构造函数重载 - 接受宽字符字符串
    Button(int x, int y, int width, int height, const std::wstring& wtext,
        COLORREF normalColor = Colors::ButtonNormal,
        COLORREF hoverColor = Colors::ButtonHover,
        COLORREF textColor = Colors::ButtonText,
        int radius = 8)
        : x(x), y(y), width(width), height(height), wtext(wtext),
        isHovered(false), normalColor(normalColor),
        hoverColor(hoverColor), textColor(textColor),
        radius(radius) {}

    virtual void draw() {
        // 使用圆角参数绘制按钮
        setfillcolor(isHovered ? hoverColor : normalColor);
        fillroundrect(x, y, x + width, y + height, radius, radius);

        // 设置字体样式
        settextstyle(16, 0, _T("微软雅黑"));
        setbkmode(TRANSPARENT); // 设置背景透明

        // 绘制文字
        int textWidth = textwidth(wtext.c_str());
        int textHeight = textheight(wtext.c_str());
        int textX = x + (width - textWidth) / 2;
        int textY = y + (height - textHeight) / 2;

        // 确保文本颜色可见
        settextcolor(textColor);
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
std::wstring utf8ToWstring(const std::string& str) {
    if (str.empty()) return L"";

    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (len == 0) return L"";

    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], len);

    return wstr;
}

// 从JSON文件加载单词库
bool loadWordLibraryFromJSON() {
    try {
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

    std::vector<int> weightedPool;
    for (int idx : learnedWords) {
        int weight = 3 - wordLibrary[idx].familiarity; // 熟悉度越低权重越高
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

    bool wasLearned = word.learned;
    word.familiarity = newFamiliarity;

    if (newFamiliarity > 0) {
        word.learned = true;
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
        if (wasLearned) {
            auto it = std::find(learnedWords.begin(), learnedWords.end(), wordIndex);
            if (it != learnedWords.end()) {
                learnedWords.erase(it);
                unlearnedWords.push_back(wordIndex);
            }
        }
    }
}

// 修改后的主菜单界面
// 修改后的主菜单界面
class MainMenu {
private:
    Button* btnLearnNew;
    Button* btnReview;
    std::wstring statusText;

public:
    MainMenu() {
        int btnWidth = 280;  // 加宽按钮
        int btnHeight = 80;  // 增加高度
        int centerX = (WINDOW_WIDTH - btnWidth) / 2;

        // 垂直排列按钮，增加间距
        btnLearnNew = new Button(centerX, 320, btnWidth, btnHeight, "学习新词", 
            Colors::Familiar2, Colors::Familiar1, WHITE, 15);
        btnReview = new Button(centerX, 420, btnWidth, btnHeight, "复习单词",
            Colors::Familiar1, Colors::Familiar0, WHITE, 15);

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
        std::wstring title = L"词汇大师";
        int titleWidth = textwidth(title.c_str());
        outtextxy((WINDOW_WIDTH - titleWidth)/2, 80, title.c_str());

        // 绘制副标题
        settextcolor(Colors::Subtitle);
        settextstyle(24, 0, _T("微软雅黑"));
        std::wstring subtitle = L"每日进步一点点";
        int subtitleWidth = textwidth(subtitle.c_str());
        outtextxy((WINDOW_WIDTH - subtitleWidth)/2, 150, subtitle.c_str());

        // 绘制状态文本
        settextcolor(Colors::Text);
        settextstyle(18, 0, _T("微软雅黑"));
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

// 修改后的单词学习界面
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
        // 创建返回和下一个按钮
        btnBack = new Button(50, 500, 120, 50, "返回", 
            Colors::ButtonNormal, Colors::ButtonHover, WHITE, 8);
        btnNext = new Button(330, 500, 120, 50, "下一个", 
            Colors::ButtonNormal, Colors::ButtonHover, WHITE, 8);

        // 创建熟悉度按钮
        int btnWidth = 160;
        int btnHeight = 60;
        int startX = (WINDOW_WIDTH - btnWidth*3 - 40)/2;

        btnFamiliarity0 = new Button(startX, 400, btnWidth, btnHeight, "不熟悉",
            Colors::Familiar0, RGB(255, 100, 100), WHITE, 10);
        btnFamiliarity1 = new Button(startX + btnWidth + 20, 400, btnWidth, btnHeight, "一般",
            Colors::Familiar1, RGB(255, 180, 50), WHITE, 10);
        btnFamiliarity2 = new Button(startX + btnWidth*2 + 40, 400, btnWidth, btnHeight, "熟悉",
            Colors::Familiar2, RGB(100, 200, 100), WHITE, 10);

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
        settextcolor(Colors::Title);
        settextstyle(24, 0, _T("微软雅黑"));
        outtextxy(50, 30, statusText.c_str());

        if (currentWordIndex >= 0 && currentWordIndex < wordLibrary.size()) {
            Word& word = wordLibrary[currentWordIndex];

            // 绘制卡片背景
            setfillcolor(Colors::CardBg);
            fillroundrect(100, 120, WINDOW_WIDTH - 100, 340, 20, 20);

            // 绘制装饰线
            setlinecolor(0xE9ECEF);
            setlinestyle(PS_SOLID, 3);
            line(100, 460, WINDOW_WIDTH - 100, 460);

            // 绘制单词
            settextcolor(Colors::Title);
            settextstyle(64, 0, _T("微软雅黑"));
            std::wstring wWord = utf8ToWstring(word.word);
            int wordWidth = textwidth(wWord.c_str());
            int wordX = (WINDOW_WIDTH - wordWidth) / 2;
            outtextxy(wordX, 140, wWord.c_str());

            // 绘制释义
            settextcolor(Colors::Text);
            settextstyle(24, 0, _T("微软雅黑"));
            std::wstring wMeaning = utf8ToWstring(word.meaning);
            int meaningWidth = textwidth(wMeaning.c_str());
            int meaningX = (WINDOW_WIDTH - meaningWidth) / 2;
            outtextxy(meaningX, 240, wMeaning.c_str());

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
            int msgWidth = textwidth(message.c_str());
            outtextxy((WINDOW_WIDTH - msgWidth)/2, 200, message.c_str());
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

            mainMenu->updateStatusText();
        }

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

int main() {
    SetConsoleOutputCP(65001);
    // 尝试加载单词库
    if (!loadWordLibraryFromJSON()) {
        std::cout << "使用示例单词库..." << std::endl;

        Word w1; w1.word = "apple"; w1.meaning = "苹果";
        Word w2; w2.word = "banana"; w2.meaning = "香蕉";
        Word w3; w3.word = "cherry"; w3.meaning = "樱桃";
        Word w4; w4.word = "dog"; w4.meaning = "狗";
        Word w5; w5.word = "elephant"; w5.meaning = "大象";

        wordLibrary = { w1, w2, w3, w4, w5 };

        for (size_t i = 0; i < wordLibrary.size(); i++) {
            unlearnedWords.push_back(i);
        }
    }

    // 初始化图形窗口
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    BeginBatchDraw();

    // 创建界面对象
    MainMenu mainMenu;
    WordLearningScreen* currentLearningScreen = nullptr;

    int currentScreen = 0; // 0-主菜单，1-学习，2-复习
    bool running = true;

    while (running) {
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();

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

        // 绘制当前界面
        cleardevice();

        if (currentScreen == 0) {
            mainMenu.draw();
        }
        else {
            if (currentLearningScreen) currentLearningScreen->draw();
        }

        FlushBatchDraw();
        Sleep(10);
    }

    // 清理资源
    delete currentLearningScreen;
    EndBatchDraw();
    closegraph();
    return 0;
}