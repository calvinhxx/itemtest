#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QtTest/QSignalSpy>
#include "view/textfields/Label.h"
#include "view/basicinput/Button.h"
#include "view/QMLPlus.h"

using namespace view::textfields;
using namespace view::basicinput;
using namespace view;

class LabelTest : public ::testing::Test {
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
        window->setFixedSize(800, 600);
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

TEST_F(LabelTest, DefaultConstructor) {
    Label* label = new Label(window);
    EXPECT_EQ(label->text(), "");
    EXPECT_EQ(label->fluentTypography(), "Body");
    EXPECT_NE(dynamic_cast<QLabel*>(label), nullptr);
}

TEST_F(LabelTest, TextConstructor) {
    Label* label = new Label("Static label text", window);
    EXPECT_EQ(label->text(), "Static label text");
    EXPECT_EQ(label->fluentTypography(), "Body");
}

TEST_F(LabelTest, FluentTypographyChange) {
    Label* label = new Label(window);
    QSignalSpy spy(label, SIGNAL(typographyChanged()));

    label->setFluentTypography("Title");
    EXPECT_EQ(label->fluentTypography(), "Title");
    EXPECT_EQ(spy.count(), 1);

    label->setFluentTypography("Title");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(LabelTest, ThemeUpdatePreservesTypography) {
    Label* label = new Label("Theme text", window);
    label->setFluentTypography("Caption");

    label->onThemeUpdated();

    EXPECT_EQ(label->fluentTypography(), "Caption");
    EXPECT_TRUE(label->palette().color(QPalette::WindowText).isValid());
}

TEST_F(LabelTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    using Edge = AnchorLayout::Edge;

    auto createTypographyLabel = [&](const QString& text, const QString& styleName, QWidget* anchor, int margin = 20) {
        Label* l = new Label(text + " (" + styleName + ")", window);
        // --- 核心修复：使用属性接口，内部会自动记忆并在切换主题时持久化 ---
        l->setFluentTypography(styleName);
        
        l->anchors()->top = {anchor, Edge::Bottom, margin};
        l->anchors()->left = {window, Edge::Left, 40};
        layout->addWidget(l);
        return l;
    };

    // 1. Display (最顶层锚定)
    Label* display = new Label("Fluent UI (Display)", window);
    display->setFluentTypography("Display");
    display->anchors()->top = {window, Edge::Top, 30};
    display->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(display);
    
    // 2. 其余阶梯
    Label* titleLarge = createTypographyLabel("Large Title", "TitleLarge", display);
    Label* title = createTypographyLabel("Standard Title", "Title", titleLarge);
    Label* subtitle = createTypographyLabel("Subtitle Text", "Subtitle", title);
    Label* bodyStrong = createTypographyLabel("Strong Body Text", "BodyStrong", subtitle);
    
    Label* body = new Label("Standard Body Text (Default)", window);
    body->anchors()->top = {bodyStrong, Edge::Bottom, 20};
    body->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(body);
    
    Label* caption = createTypographyLabel("Small Caption Text", "Caption", body);

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
