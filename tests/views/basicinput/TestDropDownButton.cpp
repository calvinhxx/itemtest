#include <gtest/gtest.h>
#include <QApplication>
#include <QMenu>
#include <QTimer>
#include <QFontDatabase>
#include "view/basicinput/DropDownButton.h"
#include "view/QMLPlus.h"
#include "view/FluentElement.h"
#include "view/textfields/Label.h"

using namespace view::basicinput;
using namespace view::textfields;
using namespace view;

class FluentTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class DropDownButtonTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) new QApplication(argc, argv);
        QApplication::setStyle("Fusion");

        // --- 关键修复：初始化资源和加载字体 ---
        Q_INIT_RESOURCE(resources);
        
        // 加载 Segoe Fluent Icons 确保 DropDownButton 能显示图标
        int iconFontId = QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        if (iconFontId != -1) {
            QString family = QFontDatabase::applicationFontFamilies(iconFontId).first();
            qDebug() << "Successfully loaded Icon Font:" << family;
        } else {
            qWarning() << "Failed to load Segoe Fluent Icons from resources!";
        }

        // 加载 UI 字体
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }

    void SetUp() override {
        window = new FluentTestWindow();
        window->setFixedSize(550, 450);
        window->setWindowTitle("DropDownButton Visual Test");
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

TEST_F(DropDownButtonTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    using Edge = AnchorLayout::Edge;

    Label* title = new Label("DropDownButton Visual Test", window);
    title->setFont(title->themeFont("Title").toQFont());
    title->anchors()->top = {window, Edge::Top, 30};
    title->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(title);

    // 1. Standard DropDownButton
    Label* lbl1 = new Label("Standard Style (with Menu):", window);
    lbl1->anchors()->top = {title, Edge::Bottom, 40};
    lbl1->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(lbl1);

    DropDownButton* dropBtn = new DropDownButton("Options", window);
    dropBtn->setFixedSize(140, 32);
    dropBtn->anchors()->top = {lbl1, Edge::Bottom, 10};
    dropBtn->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(dropBtn);

    QMenu* menu1 = new QMenu(dropBtn);
    menu1->addAction("Edit Profile");
    menu1->addAction("Account Settings");
    menu1->addSeparator();
    menu1->addAction("Logout");
    dropBtn->setMenu(menu1);

    // 2. Accent Style DropDownButton
    Label* lbl2 = new Label("Accent Style:", window);
    lbl2->anchors()->verticalCenter = {lbl1, Edge::VCenter, 0};
    lbl2->anchors()->left = {dropBtn, Edge::Right, 60};
    layout->addWidget(lbl2);

    DropDownButton* dropAccent = new DropDownButton("Primary Action", window);
    dropAccent->setFluentStyle(Button::Accent);
    dropAccent->setFixedSize(160, 32);
    dropAccent->anchors()->top = {lbl2, Edge::Bottom, 10};
    dropAccent->anchors()->left = {dropBtn, Edge::Right, 60};
    layout->addWidget(dropAccent);

    QMenu* menu2 = new QMenu(dropAccent);
    menu2->addAction("Confirm Selection");
    menu2->addAction("Review Changes");
    dropAccent->setMenu(menu2);

    // 3. 自定义图标属性测试（多种 Icon 对比）
    Label* lbl3 = new Label("Custom Glyph Variants:", window);
    lbl3->anchors()->top = {dropBtn, Edge::Bottom, 40};
    lbl3->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(lbl3);

    // 3.1 向上 Chevron
    DropDownButton* customUp = new DropDownButton("Chevron Up", window);
    customUp->setChevronGlyph(Typography::Icons::ChevronUp); 
    customUp->setChevronSize(16);
    customUp->setChevronPadding(20);
    customUp->setFixedSize(150, 32);
    customUp->anchors()->top = {lbl3, Edge::Bottom, 10};
    customUp->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(customUp);

    // 3.2 向下中号 Chevron
    DropDownButton* customDownMed = new DropDownButton("Chevron Down Med", window);
    customDownMed->setChevronGlyph(Typography::Icons::ChevronDownMed);
    customDownMed->setChevronSize(14);
    customDownMed->setChevronPadding(5);
    customDownMed->setFixedSize(170, 32);
    customDownMed->anchors()->top = {customUp, Edge::Bottom, 8};
    customDownMed->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(customDownMed);

    // 3.3 右箭头 Icon（演示非传统下拉箭头）
    DropDownButton* customRight = new DropDownButton("Next Step", window);
    customRight->setChevronGlyph(Typography::Icons::ChevronRight);
    customRight->setChevronSize(14);
    customRight->setChevronPadding(16);
    customRight->setFixedSize(150, 32);
    customRight->anchors()->top = {customDownMed, Edge::Bottom, 8};
    customRight->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(customRight);

    // 3.4 使用其它 Fluent Icon（例如“更多”菜单）
    DropDownButton* customMore = new DropDownButton("More Actions", window);
    customMore->setChevronGlyph(Typography::Icons::More);
    customMore->setChevronSize(14);
    customMore->setChevronPadding(16);
    customMore->setChevronOffset(3);
    customMore->setFixedSize(160, 32);
    customMore->anchors()->top = {customRight, Edge::Bottom, 8};
    customMore->anchors()->left = {window, Edge::Left, 40};
    layout->addWidget(customMore);

    // 4. Theme Switcher
    Button* themeBtn = new Button("Switch Theme", window);
    themeBtn->setFixedSize(120, 32);
    themeBtn->anchors()->bottom = {window, Edge::Bottom, -20};
    themeBtn->anchors()->right = {window, Edge::Right, -20};
    layout->addWidget(themeBtn);

    QObject::connect(themeBtn, &Button::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light ? FluentElement::Dark : FluentElement::Light);
    });

    window->show();
    qApp->exec();
}
