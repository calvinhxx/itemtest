#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include "items/interface/ResponsivePushbutton.h"
#include "viewmodel/VM_ResponsivePushbutton.h"
#include "utils/PropertyBinder.h"

class ResponsivePushbuttonTest : public ::testing::Test {
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
        btn = new ResponsivePushbutton(window);
        vm = new VM_ResponsivePushbutton(window);
    }

    void TearDown() override {
        delete window;
    }

    QWidget* window;
    ResponsivePushbutton* btn;
    VM_ResponsivePushbutton* vm;
};

TEST_F(ResponsivePushbuttonTest, GenericBinding) {
    // 绑定文字和启用状态
    PropertyBinder::bind(vm, "text", btn, "text");
    PropertyBinder::bind(vm, "enabled", btn, "enabled");

    vm->setText("Click Me");
    EXPECT_EQ(btn->text(), "Click Me");

    vm->setEnabled(false);
    EXPECT_FALSE(btn->isEnabled());
}
