#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QString>

#include "items/ResponsiveLabel.h"
#include "viewmodel/VM_ResponsiveLabel.h"

class ResponsiveLabelTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
    }

    void SetUp() override {
        window = new QWidget();
        window->setFixedSize(600, 600);
        window->setWindowTitle("ResponsiveLabel UT Visual Check");
        label = new ResponsiveLabel(window);
        vm = new VM_ResponsiveLabel(window);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    ResponsiveLabel* label;
    VM_ResponsiveLabel* vm;
};

TEST_F(ResponsiveLabelTest, BindingUpdate) {
    // 1. 执行绑定
    label->bind(vm, "text");

    // 2. 验证初始值 (VM 初始化为 "就绪")
    EXPECT_EQ(label->text(), "就绪");

    // 3. 外部更新 VM 的属性
    vm->setText("Hello MVVM");

    // 4. 验证 Label 是否自动同步
    EXPECT_EQ(label->text(), "Hello MVVM");

    window->show();
    qApp->exec();
}
