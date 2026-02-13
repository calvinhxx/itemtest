#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include "view/textfields/TextBlock.h"
#include "view/basicinput/Button.h"
#include "view/QMLPlus.h"

using namespace view::textfields;
using namespace view::basicinput;
using namespace view;

class TextBlockTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
        QApplication::setStyle("Fusion");
    }

    void SetUp() override {
        window = new QWidget();
        window->setFixedSize(500, 600);
        window->setWindowTitle("Fluent Typography Persistence Test");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    AnchorLayout* layout;
};

TEST_F(TextBlockTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    using Edge = AnchorLayout::Edge;

    auto createTypographyTextBlock = [&](const QString& text, const QString& styleName, QWidget* anchor, int margin = 20) {
        TextBlock* l = new TextBlock(text + " (" + styleName + ")", window);
        // --- 核心修复：使用属性接口，内部会自动记忆并在切换主题时持久化 ---
        l->setFluentTypography(styleName);
        
        l->anchors()->top = {anchor, Edge::Bottom, margin};
        l->anchors()->left = {window, Edge::Left, 40};
        layout->addWidget(l);
        return l;
    };

    // 1. Display (最顶层锚定)
    TextBlock* display = new TextBlock("Fluent UI (Display)", window);
    display->setFluentTypography("Display");
    display->anchors()->top = {window, Edge::Top, 30};
    display->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(display);
    
    // 2. 其余阶梯
    TextBlock* titleLarge = createTypographyTextBlock("Large Title", "TitleLarge", display);
    TextBlock* title = createTypographyTextBlock("Standard Title", "Title", titleLarge);
    TextBlock* subtitle = createTypographyTextBlock("Subtitle Text", "Subtitle", title);
    TextBlock* bodyStrong = createTypographyTextBlock("Strong Body Text", "BodyStrong", subtitle);
    
    TextBlock* body = new TextBlock("Standard Body Text (Default)", window);
    body->anchors()->top = {bodyStrong, Edge::Bottom, 20};
    body->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(body);
    
    TextBlock* caption = createTypographyTextBlock("Small Caption Text", "Caption", body);

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
