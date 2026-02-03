#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
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
        window->setFixedSize(400, 300);
        window->setWindowTitle("Label Visual Test");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    AnchorLayout* layout;
};

TEST_F(LabelTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    // 1. 不同层级的字体展示
    Label* title = new Label("Fluent Typography", window);
    title->setFont(title->themeFont("Title").toQFont());
    title->anchors()->top = {window, AnchorLayout::Edge::Top, 20};
    title->anchors()->horizontalCenter = {window, AnchorLayout::Edge::HCenter, 0};
    layout->addWidget(title);

    Label* body = new Label("This is a standard body text using Fluent tokens.", window);
    body->anchors()->top = {title, AnchorLayout::Edge::Bottom, 10};
    body->anchors()->horizontalCenter = {window, AnchorLayout::Edge::HCenter, 0};
    layout->addWidget(body);

    Label* caption = new Label("Caption text", window);
    caption->setFont(caption->themeFont("Caption").toQFont());
    caption->anchors()->top = {body, AnchorLayout::Edge::Bottom, 10};
    caption->anchors()->horizontalCenter = {window, AnchorLayout::Edge::HCenter, 0};
    layout->addWidget(caption);

    // 2. 主题切换
    Button* themeBtn = new Button("Toggle Theme", window);
    themeBtn->setFixedSize(120, 32);
    themeBtn->anchors()->bottom = {window, AnchorLayout::Edge::Bottom, -20};
    themeBtn->anchors()->horizontalCenter = {window, AnchorLayout::Edge::HCenter, 0};
    layout->addWidget(themeBtn);

    QObject::connect(themeBtn, &Button::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light ? FluentElement::Dark : FluentElement::Light);
    });

    window->show();
    qApp->exec();
}
