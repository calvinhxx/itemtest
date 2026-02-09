#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontDatabase>
#include "view/menus_toolbars/Menu.h"
#include "view/basicinput/ToggleSplitButton.h"
#include "view/FluentElement.h"
#include "common/Typography.h"

using namespace view;
using namespace view::basicinput;
using namespace view::menus_toolbars;

class ToggleSplitButtonTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;

    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class ToggleSplitButtonTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char** argv = nullptr;
        if (!qApp) new QApplication(argc, argv);
        QApplication::setStyle("Fusion");

        Q_INIT_RESOURCE(resources);
        QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }

    void SetUp() override {
        window = new ToggleSplitButtonTestWindow();
        window->setFixedSize(600, 400);
        window->setWindowTitle("Fluent ToggleSplitButton Visual Test");

        auto* layout = new QVBoxLayout(window);
        layout->setContentsMargins(40, 40, 40, 40);
        layout->setSpacing(20);

        // 1. Basic ToggleSplitButton
        layout->addWidget(new QLabel("1. ToggleSplitButton (Icon + Text):", window));
        auto* toggleSplit = new ToggleSplitButton("List Options", window);
        
        // 设置图标和布局
        toggleSplit->setIconGlyph(Typography::Icons::List, Typography::FontSize::Body);
        toggleSplit->setFluentLayout(Button::IconBefore);
        toggleSplit->setFixedSize(160, 32);

        // 设置菜单
        FluentMenu* menu = new FluentMenu("Styles", toggleSplit);
        menu->addAction(new FluentMenuItem("None", menu));
        menu->addAction(new FluentMenuItem("Bulleted", menu));
        menu->addAction(new FluentMenuItem("Numbered", menu));
        toggleSplit->setMenu(menu);
        
        QLabel* status = new QLabel("State: Unchecked", window);
        QObject::connect(toggleSplit, &ToggleSplitButton::toggled, [status](bool checked) {
            status->setText(QString("State: %1").arg(checked ? "Checked" : "Unchecked"));
        });
        
        layout->addWidget(toggleSplit);
        layout->addWidget(status);

        // 2. Icon-only ToggleSplitButton
        layout->addWidget(new QLabel("2. Icon-only ToggleSplitButton:", window));
        auto* iconOnly = new ToggleSplitButton("", window);
        iconOnly->setIconGlyph(Typography::Icons::Settings, Typography::FontSize::Body);
        iconOnly->setFluentLayout(Button::IconOnly);
        iconOnly->setFixedSize(64, 32);
        layout->addWidget(iconOnly);

        layout->addStretch();

        // Theme switch button
        auto* themeBtn = new Button("Switch Theme", window);
        themeBtn->setFixedSize(120, 32);
        layout->addWidget(themeBtn);
        QObject::connect(themeBtn, &Button::clicked, []() {
            FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light 
                                    ? FluentElement::Dark 
                                    : FluentElement::Light);
        });

        window->onThemeUpdated();
    }

    void TearDown() override {
        delete window;
    }

    ToggleSplitButtonTestWindow* window = nullptr;
};

TEST_F(ToggleSplitButtonTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") && 
        qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    window->show();
    qApp->exec();
}
