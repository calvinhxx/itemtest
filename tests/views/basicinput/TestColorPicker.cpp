#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontDatabase>
#include "view/basicinput/ColorPicker.h"
#include "view/basicinput/Button.h"
#include "view/basicinput/CheckBox.h"
#include "view/FluentElement.h"
#include "utils/DebugOverlay.h"

using namespace view;
using namespace view::basicinput;

class ColorPickerTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;

    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class ColorPickerTest : public ::testing::Test {
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
        window = new ColorPickerTestWindow();
        window->setFixedSize(900, 650);
        window->setWindowTitle("Fluent ColorPicker Visual Test");

        auto* root = new QVBoxLayout(window);
        root->setContentsMargins(40, 40, 40, 40);
        root->setSpacing(24);

        auto* title = new QLabel("ColorPicker - Basic input", window);
        QFont titleFont = window->themeFont("Title").toQFont();
        title->setFont(titleFont);
        root->addWidget(title);

        // 主内容区域：左侧为 ColorPicker，右侧为属性面板（参考 WinUI3 Gallery）
        auto* mainLayout = new QHBoxLayout();
        mainLayout->setSpacing(32);
        root->addLayout(mainLayout, 1);

        // 左侧：ColorPicker + 状态
        auto* leftLayout = new QVBoxLayout();
        leftLayout->setSpacing(16);
        mainLayout->addLayout(leftLayout, 2);

        leftLayout->addWidget(new QLabel("1. Basic ColorPicker:", window));

        auto* picker = new ColorPicker(window);
        new DebugOverlay(picker); // 辅助调试布局
        leftLayout->addWidget(picker, 1);

        auto* status = new QLabel("Color: #FFFFFFFF", window);
        leftLayout->addWidget(status);

        QObject::connect(picker, &ColorPicker::colorChanged, [status](const QColor& c) {
            status->setText(QString("Color: #%1%2%3%4")
                                .arg(c.red(), 2, 16, QLatin1Char('0'))
                                .arg(c.green(), 2, 16, QLatin1Char('0'))
                                .arg(c.blue(), 2, 16, QLatin1Char('0'))
                                .arg(c.alpha(), 2, 16, QLatin1Char('0')).toUpper());
        });

        // 右侧：属性面板（简化版）
        auto* rightLayout = new QVBoxLayout();
        rightLayout->setSpacing(12);
        mainLayout->addLayout(rightLayout, 1);

        auto* propsTitle = new QLabel("ColorPicker Properties", window);
        propsTitle->setFont(window->themeFont("Subtitle").toQFont());
        rightLayout->addWidget(propsTitle);

        rightLayout->addSpacing(8);

        // Alpha Enabled
        auto* cbAlpha = new CheckBox("Alpha Enabled", window);
        cbAlpha->setChecked(true);
        QObject::connect(cbAlpha, &CheckBox::stateChanged, [picker](int state) {
            picker->setAlphaEnabled(state == Qt::Checked);
        });
        rightLayout->addWidget(cbAlpha);

        // 预留属性：Hex 输入、颜色通道文本输入等（当前 demo 中不生效，仅作为布局占位）
        auto* cbHexVisible = new CheckBox("IsHexInputVisible (demo only)", window);
        cbHexVisible->setEnabled(false);
        rightLayout->addWidget(cbHexVisible);

        auto* cbMoreButton = new CheckBox("IsMoreButtonVisible (demo only)", window);
        cbMoreButton->setEnabled(false);
        rightLayout->addWidget(cbMoreButton);

        rightLayout->addStretch();

        // 底部主题切换按钮
        auto* themeBtn = new Button("Switch Theme", window);
        themeBtn->setFixedSize(120, 32);
        root->addWidget(themeBtn, 0, Qt::AlignLeft);
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

    ColorPickerTestWindow* window = nullptr;
};

TEST_F(ColorPickerTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("QT_QPA_PLATFORM") &&
        qEnvironmentVariable("QT_QPA_PLATFORM") == "offscreen") {
        GTEST_SKIP() << "Skipping visual test in offscreen mode";
    }

    window->show();
    qApp->exec();
}

