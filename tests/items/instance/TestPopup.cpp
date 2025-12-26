#include <gtest/gtest.h>
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QWidget>
#include <QVBoxLayout>

#include "items/instance/Popup.h"
#include "viewmodel/VM_ResponsiveDialog.h"

class PopupTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
    }

    void SetUp() override {
        window = new Popup();
        window->setFixedSize(450, 350);
        window->setCornerRadius(10);
        window->setShadowWidth(10);
        window->setBorderColor(QColor(100, 100, 100, 100));
    }

    void TearDown() override {
        delete window;
    }

    Popup* window;
};

TEST_F(PopupTest, VisualCheck) {
    VM_ResponsiveDialog* vm = new VM_ResponsiveDialog(window);
    // 绑定标题和可见性
    window->bindTitle(vm, "title");
    window->bindVisible(vm, "visible");
    window->setStyleSheet("background-color: red;");

    vm->setTitle("自定义圆角弹窗");
    vm->setVisible(true);
    window->show();
    // 验证无边框标志
    EXPECT_TRUE(window->windowFlags() & Qt::FramelessWindowHint);

    qApp->exec();
}

