#include <gtest/gtest.h>
#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include "view/basicinput/Button.h"
#include "view/QMLPlus.h"

using namespace view::basicinput;
using namespace view;

class ButtonTest : public ::testing::Test {
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
        window->setFixedSize(400, 300);
        window->setWindowTitle("Button Visual Test");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    AnchorLayout* layout;
};

TEST_F(ButtonTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    // 1. 基础按钮 + 锚点布局
    Button* btn = new Button("Fluent Button", window);
    btn->setFixedSize(160, 40);
    btn->anchors()->horizontalCenter = {window, AnchorLayout::Edge::HCenter, 0};
    btn->anchors()->verticalCenter   = {window, AnchorLayout::Edge::VCenter, -20};
    layout->addWidget(btn);

    // 2. 状态切换测试
    Button* stateBtn = new Button("Toggle State", window);
    stateBtn->setFixedSize(120, 32);
    stateBtn->anchors()->top = {btn, AnchorLayout::Edge::Bottom, 20};
    stateBtn->anchors()->horizontalCenter = {btn, AnchorLayout::Edge::HCenter, 0};
    layout->addWidget(stateBtn);

    QMLState busyState;
    busyState.name = "busy";
    busyState.changes = {
        { stateBtn, "text", "Please wait..." },
        { stateBtn, "enabled", false }
    };
    stateBtn->addState(busyState);

    QObject::connect(btn, &QPushButton::clicked, [stateBtn]() {
        if (stateBtn->state() == "") {
            stateBtn->setState("busy");
            QTimer::singleShot(2000, [stateBtn]() { stateBtn->setState(""); });
        }
    });

    // 3. 主题切换
    Button* themeBtn = new Button("Switch Theme", window);
    themeBtn->setFixedSize(120, 32);
    themeBtn->anchors()->bottom = {window, AnchorLayout::Edge::Bottom, -20};
    themeBtn->anchors()->right = {window, AnchorLayout::Edge::Right, -20};
    layout->addWidget(themeBtn);

    QObject::connect(themeBtn, &QPushButton::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light ? FluentElement::Dark : FluentElement::Light);
    });

    window->show();
    qApp->exec();
}
