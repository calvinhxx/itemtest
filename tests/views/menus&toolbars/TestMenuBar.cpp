#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>

#include "view/menus&toolbars/MenuBar.h"
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

        // File 菜单
        QMenu* fileMenu = new QMenu("File", menuBar);
        QAction* actionNew = fileMenu->addAction("New");
        QAction* actionOpen = fileMenu->addAction("Open");
        QAction* actionExit = fileMenu->addAction("Exit");
        menuBar->addMenu(fileMenu);

        // Edit 菜单
        QMenu* editMenu = new QMenu("Edit", menuBar);
        QAction* actionUndo = editMenu->addAction("Undo");
        QAction* actionRedo = editMenu->addAction("Redo");
        editMenu->addSeparator();
        QAction* actionCut = editMenu->addAction("Cut");
        QAction* actionCopy = editMenu->addAction("Copy");
        QAction* actionPaste = editMenu->addAction("Paste");
        menuBar->addMenu(editMenu);

        // Help 菜单
        QMenu* helpMenu = new QMenu("Help", menuBar);
        QAction* actionAbout = helpMenu->addAction("About");
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

