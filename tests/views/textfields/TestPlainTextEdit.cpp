#include <gtest/gtest.h>
#include <QApplication>
#include <QFontDatabase>
#include "view/textfields/PlainTextEdit.h"
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

class PlainTextEditTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char** argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
        QApplication::setStyle("Fusion");
        Q_INIT_RESOURCE(resources);
        QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }

    void SetUp() override {
        window = new FluentTestWindow();
        window->setFixedSize(500, 400);
        window->setWindowTitle("Fluent PlainTextEdit Test");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
        window->onThemeUpdated();
    }

    void TearDown() override {
        delete window;
    }

    FluentTestWindow* window;
    AnchorLayout* layout;
};

TEST_F(PlainTextEditTest, TextAndPlaceholder) {
    PlainTextEdit* edit = new PlainTextEdit(window);
    edit->setPlaceholderText("Multi-line placeholder");
    EXPECT_EQ(edit->placeholderText(), "Multi-line placeholder");

    edit->setPlainText("line1\nline2");
    EXPECT_EQ(edit->toPlainText(), "line1\nline2");
}

TEST_F(PlainTextEditTest, ContentMargins) {
    PlainTextEdit* edit = new PlainTextEdit(window);
    QMargins margins(12, 4, 12, 4);
    edit->setContentMargins(margins);
    EXPECT_EQ(edit->contentMargins(), margins);
}

TEST_F(PlainTextEditTest, ReadOnly) {
    PlainTextEdit* edit = new PlainTextEdit(window);
    edit->setPlainText("read only content");
    edit->setReadOnly(true);
    EXPECT_TRUE(edit->isReadOnly());
    edit->setReadOnly(false);
    EXPECT_FALSE(edit->isReadOnly());
}

TEST_F(PlainTextEditTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    using Edge = AnchorLayout::Edge;

    TextBlock* header = new TextBlock("PlainTextEdit (multi-line):", window);
    header->anchors()->top = {window, Edge::Top, 30};
    header->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(header);

    PlainTextEdit* edit = new PlainTextEdit(window);
    edit->setPlaceholderText("Type multiple lines here...");
    edit->setPlainText("First line\nSecond line");
    edit->setFixedHeight(100);
    edit->setContentMargins(QMargins(8, 4, 8, 4));
    edit->anchors()->top = {header, Edge::Bottom, 8};
    edit->anchors()->left = {window, Edge::Left, 40};
    edit->anchors()->right = {window, Edge::Right, -40};
    layout->addWidget(edit);

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
