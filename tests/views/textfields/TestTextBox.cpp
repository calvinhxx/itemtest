#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QDebug>
#include "view/textfields/TextBox.h"
#include "view/textfields/TextBlock.h"
#include "view/basicinput/Button.h"
#include "view/QMLPlus.h"

using namespace view::textfields;
using namespace view::basicinput;
using namespace view;

class FluentTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class TextBoxTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
        QApplication::setStyle("Fusion");

        // 加载资源和图标字体
        Q_INIT_RESOURCE(resources);
        int fontId = QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        if (fontId == -1) {
            qWarning() << "Failed to load Segoe Fluent Icons from resources!";
        }
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }

    void SetUp() override {
        window = new FluentTestWindow();
        window->setFixedSize(600, 800); // 增加窗口大小以容纳更多测试用例
        window->setWindowTitle("Fluent TextBox Test");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
        window->onThemeUpdated(); // 初始化背景色
    }

    void TearDown() override {
        delete window;
    }

    FluentTestWindow* window;
    AnchorLayout* layout;
};

TEST_F(TextBoxTest, FunctionalTest) {
    TextBox* textBox = new TextBox(window);
    textBox->setPlaceholderText("Enter your name");
    
    EXPECT_EQ(textBox->placeholderText(), "Enter your name");
    
    textBox->setText("John Doe");
    EXPECT_EQ(textBox->text(), "John Doe");

    // 验证多行切换
    textBox->setMultiLine(true);
    EXPECT_TRUE(textBox->isMultiLine());
    textBox->setText("Line 1\nLine 2");
    EXPECT_EQ(textBox->text(), "Line 1\nLine 2");
}

TEST_F(TextBoxTest, CustomPropertiesTest) {
    TextBox* textBox = new TextBox(window);
    
    // 1. 测试清除按钮大小
    textBox->setClearButtonSize(40);
    EXPECT_EQ(textBox->clearButtonSize(), 40);
    
    // 2. 测试内边距影响高度
    QMargins margins(10, 20, 10, 20); // 上下各 20px
    textBox->setTextMargins(margins);
    EXPECT_EQ(textBox->textMargins(), margins);
    
    // 获取当前高度
    int heightBefore = textBox->height();
    
    // 改变边距再次检查高度变化
    textBox->setTextMargins(QMargins(10, 40, 10, 40));
    EXPECT_GT(textBox->height(), heightBefore);
}

TEST_F(TextBoxTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    using Edge = AnchorLayout::Edge;

    // 1. Simple TextBox with manual Header
    TextBlock* simpleHeader = new TextBlock("User Name:", window);
    simpleHeader->anchors()->top = {window, Edge::Top, 40};
    simpleHeader->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(simpleHeader);

    TextBox* simpleBox = new TextBox(window);
    simpleBox->setPlaceholderText("A simple TextBox with clear button");
    simpleBox->setText("Initial text");
    simpleBox->anchors()->top = {simpleHeader, Edge::Bottom, 8};
    simpleBox->anchors()->left = {window, Edge::Left, 40};
    simpleBox->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(simpleBox);

    // 2. Multi-line TextBox with manual Header
    TextBlock* multiHeader = new TextBlock("Multi-line comments:", window);
    multiHeader->anchors()->top = {simpleBox, Edge::Bottom, 30};
    multiHeader->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(multiHeader);

    TextBox* multiLineBox = new TextBox(window);
    multiLineBox->setMultiLine(true);
    multiLineBox->setPlaceholderText("Type multiple lines here...");
    multiLineBox->anchors()->top = {multiHeader, Edge::Bottom, 8};
    multiLineBox->anchors()->left = {window, Edge::Left, 40};
    multiLineBox->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(multiLineBox);

    // 3. Read-only TextBox with manual Header
    TextBlock* readOnlyHeader = new TextBlock("Read-only info:", window);
    readOnlyHeader->anchors()->top = {multiLineBox, Edge::Bottom, 30};
    readOnlyHeader->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(readOnlyHeader);

    TextBox* readOnlyBox = new TextBox(window);
    readOnlyBox->setText("I am super excited to be here!");
    readOnlyBox->setReadOnly(true);
    readOnlyBox->anchors()->top = {readOnlyHeader, Edge::Bottom, 8};
    readOnlyBox->anchors()->left = {window, Edge::Left, 40};
    readOnlyBox->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(readOnlyBox);

    // 4. Custom Styling (Custom height and ClearButton size)
    TextBlock* customHeader = new TextBlock("Custom Height & Button Size:", window);
    customHeader->anchors()->top = {readOnlyBox, Edge::Bottom, 30};
    customHeader->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(customHeader);

    TextBox* customBox = new TextBox(window);
    customBox->setText("Tall TextBox with big clear button");
    customBox->setTextMargins(QMargins(12, 15, 12, 15)); // 增加上下边距增加高度
    customBox->setClearButtonSize(40); // 增大清除按钮
    customBox->anchors()->top = {customHeader, Edge::Bottom, 8};
    customBox->anchors()->left = {window, Edge::Left, 40};
    customBox->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(customBox);

    // --- 主题切换 ---
    Button* themeBtn = new Button("Switch Theme", window);
    themeBtn->setFluentStyle(Button::Accent);
    themeBtn->setFixedSize(120, 32);
    themeBtn->anchors()->bottom = {window, Edge::Bottom, -30};
    themeBtn->anchors()->right = {window, Edge::Right, -30};
    layout->addWidget(themeBtn);

    QObject::connect(themeBtn, &Button::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light ? FluentElement::Dark : FluentElement::Light);
    });

    window->show();
    qApp->exec();
}
