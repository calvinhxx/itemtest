#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include "view/menus_toolbars/MenuBar.h"
#include "view/menus_toolbars/Menu.h"
#include "view/FluentElement.h"
#include "view/basicinput/Button.h"

using namespace view;
using namespace view::basicinput;
using namespace view::menus_toolbars;

class MenuBarTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;

    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class MenuBarTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char** argv = nullptr;
        if (!qApp) new QApplication(argc, argv);
        QApplication::setStyle("Fusion");
    }

    void SetUp() override {
        window = new MenuBarTestWindow();
        window->setFixedSize(600, 400);
        window->setWindowTitle("Fluent MenuBar Visual Test");

        auto* layout = new QVBoxLayout(window);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        // 顶部 MenuBar
        auto* menuBar = new FluentMenuBar(window);

        // 状态标签
        auto* statusLabel = new QLabel("You clicked: (none)", window);
        statusLabel->setContentsMargins(24, 16, 24, 16);

        // File 菜单（使用 FluentMenu + FluentMenuItem）
        auto* fileMenu = new FluentMenu("File", menuBar);
        auto* actionNew = new FluentMenuItem("New", fileMenu);
        auto* actionOpen = new FluentMenuItem("Open", fileMenu);
        auto* actionExit = new FluentMenuItem("Exit", fileMenu);
        fileMenu->addAction(actionNew);
        fileMenu->addAction(actionOpen);
        fileMenu->addAction(actionExit);
        menuBar->addMenu(fileMenu);

        // Edit 菜单
        auto* editMenu = new FluentMenu("Edit", menuBar);
        auto* actionUndo = new FluentMenuItem("Undo", editMenu);
        auto* actionRedo = new FluentMenuItem("Redo", editMenu);
        editMenu->addSeparator();
        auto* actionCut = new FluentMenuItem("Cut", editMenu);
        auto* actionCopy = new FluentMenuItem("Copy", editMenu);
        auto* actionPaste = new FluentMenuItem("Paste", editMenu);
        editMenu->addAction(actionUndo);
        editMenu->addAction(actionRedo);
        editMenu->addAction(actionCut);
        editMenu->addAction(actionCopy);
        editMenu->addAction(actionPaste);
        menuBar->addMenu(editMenu);

        // Help 菜单
        auto* helpMenu = new FluentMenu("Help", menuBar);
        auto* actionAbout = new FluentMenuItem("About", helpMenu);
        helpMenu->addAction(actionAbout);
        menuBar->addMenu(helpMenu);

        auto updateStatus = [statusLabel](const QString& text) {
            statusLabel->setText("You clicked: " + text);
        };

        QObject::connect(actionNew, &QAction::triggered, [=]() { updateStatus("New"); });
        QObject::connect(actionOpen, &QAction::triggered, [=]() { updateStatus("Open"); });
        QObject::connect(actionExit, &QAction::triggered, [=]() { updateStatus("Exit"); });
        QObject::connect(actionUndo, &QAction::triggered, [=]() { updateStatus("Undo"); });
        QObject::connect(actionRedo, &QAction::triggered, [=]() { updateStatus("Redo"); });
        QObject::connect(actionCut, &QAction::triggered, [=]() { updateStatus("Cut"); });
        QObject::connect(actionCopy, &QAction::triggered, [=]() { updateStatus("Copy"); });
        QObject::connect(actionPaste, &QAction::triggered, [=]() { updateStatus("Paste"); });
        QObject::connect(actionAbout, &QAction::triggered, [=]() { updateStatus("About"); });

        // 主题切换按钮
        auto* themeBtn = new Button("Switch Theme", window);
        themeBtn->setFixedSize(120, 32);
        themeBtn->setFluentStyle(Button::Subtle);

        auto* bottomLayout = new QHBoxLayout();
        bottomLayout->addStretch();
        bottomLayout->addWidget(themeBtn);
        bottomLayout->setContentsMargins(24, 16, 24, 16);

        QWidget* bottomWidget = new QWidget(window);
        bottomWidget->setLayout(bottomLayout);

        layout->setMenuBar(menuBar);
        layout->addWidget(statusLabel);
        layout->addStretch();
        layout->addWidget(bottomWidget);

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

    MenuBarTestWindow* window = nullptr;
};

TEST_F(MenuBarTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") &&
        qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    window->show();
    qApp->exec();
}

