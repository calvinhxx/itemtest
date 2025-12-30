#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include "items/interface/ResponsiveLabel.h"
#include "viewmodel/VM_ResponsiveLabel.h"
#include "utils/PropertyBinder.h"

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

TEST_F(ResponsiveLabelTest, GenericBindingUpdate) {
    // 使用通用绑定器
    PropertyBinder::bind(vm, "text", label, "text");

    EXPECT_EQ(label->text(), "就绪");
    vm->setText("Hello Meta Binding");
    EXPECT_EQ(label->text(), "Hello Meta Binding");
}
