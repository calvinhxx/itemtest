/**
 * Fluent ComboBox 测试与可视化样例。
 *
 * 场景对齐：
 * - WinUI Gallery：microsoft/WinUI-Gallery — WinUIGallery/Samples/ControlPages/ComboBoxPage.xaml
 *   （Header、PlaceholderText、Width=200、可编辑字号列表等）
 * - Windows UI Kit (Figma)：ComboBox 组件 Rest 状态尺寸与内边距见设计稿
 *   https://www.figma.com/design/urqb6Xq3nerP5OrnbUAxGJ/Windows-UI-kit--Community-
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontDatabase>
#include <QLineEdit>

#include "common/Spacing.h"
#include "FluentListItemDelegate.h"
#include "view/basicinput/ComboBox.h"
#include "view/collections/ListView.h"
#include "view/basicinput/Button.h"
#include "view/FluentElement.h"

using namespace view;
using namespace view::basicinput;
using namespace view::collections;

namespace {

int comboPopupRowHeight() {
    return Spacing::ControlHeight::Standard + Spacing::Gap::Tight;
}

/**
 * 下拉层使用 QComboBox 内部 model，仅挂上测试用 FluentListItemDelegate（与 TestListView 共用）。
 */
void attachFluentDelegateToComboPopup(ComboBox* cb) {
    auto* lv = qobject_cast<ListView*>(cb->view());
    if (!lv)
        return;
    lv->setItemDelegate(new listview_test::FluentListItemDelegate(
        static_cast<FluentElement*>(lv), comboPopupRowHeight(), lv));
    lv->setUniformItemSizes(true);
}

} // namespace

class ComboBoxTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;

    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class ComboBoxTest : public ::testing::Test {
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
        window = new ComboBoxTestWindow();
        window->setFixedSize(800, 600);
        window->setWindowTitle("Fluent ComboBox Visual Test");
        window->onThemeUpdated();
    }

    void TearDown() override {
        delete window;
    }

    ComboBoxTestWindow* window = nullptr;
};

TEST_F(ComboBoxTest, PopupUsesCollectionsListView) {
    ComboBox cb;
    auto* lv = qobject_cast<ListView*>(cb.view());
    ASSERT_NE(lv, nullptr);
    cb.addItems({"x", "y"});
    ASSERT_EQ(cb.model(), lv->model());
}

TEST_F(ComboBoxTest, PopupListViewUsesFluentDelegateFromTestHelper) {
    ComboBox cb;
    cb.addItem("one");
    attachFluentDelegateToComboPopup(&cb);
    auto* lv = qobject_cast<ListView*>(cb.view());
    ASSERT_NE(lv, nullptr);
    ASSERT_NE(lv->itemDelegate(), nullptr);
}

TEST_F(ComboBoxTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    auto* root = new QVBoxLayout(window);
    root->setContentsMargins(40, 40, 40, 40);
    root->setSpacing(24);

    auto* title = new QLabel("ComboBox", window);
    title->setFont(window->themeFont("Title").toQFont());
    root->addWidget(title);

    // 1. 对齐 WinUI Gallery：Header + PlaceholderText + Width=200 + 内联项
    auto* header = new QLabel("Colors", window);
    header->setFont(window->themeFont("BodyStrong").toQFont());
    root->addWidget(header);
    root->addWidget(new QLabel(
        QStringLiteral("与 ComboBoxPage.xaml 一致：Header、PlaceholderText、固定宽度 200。"), window));

    auto* colorsCombo = new ComboBox(window);
    colorsCombo->setPlaceholderText(QStringLiteral("Pick a color"));
    colorsCombo->addItems({QStringLiteral("Blue"), QStringLiteral("Green"),
                           QStringLiteral("Red"), QStringLiteral("Yellow")});
    colorsCombo->setFixedWidth(200);
    root->addWidget(colorsCombo, 0, Qt::AlignLeft);

    // 2. 字体列表（对应 Gallery 中 ItemsSource + DisplayMemberPath 示例的简化版）
    root->addWidget(new QLabel("A ComboBox with its ItemsSource set.", window));
    auto* fontRow = new QHBoxLayout();
    fontRow->setSpacing(12);

    auto* fontLabel = new QLabel("You can set the font used for this text.", window);
    fontLabel->setFont(window->themeFont("Body").toQFont());

    auto* fontCombo = new ComboBox(window);
    fontCombo->addItems({"Segoe UI", "Courier New", "Times New Roman", "Arial"});
    fontCombo->setCurrentText("Courier New");
    fontCombo->setFixedWidth(200);
    QObject::connect(fontCombo, &QComboBox::currentTextChanged, [fontLabel](const QString& family) {
        QFont f = fontLabel->font();
        f.setFamily(family);
        fontLabel->setFont(f);
    });
    fontRow->addWidget(fontCombo, 0);
    fontRow->addWidget(fontLabel, 1);
    root->addLayout(fontRow);

    // 3. 可编辑 ComboBox（对应 Gallery IsEditable + 字号列表）
    root->addWidget(new QLabel("An editable ComboBox.", window));
    auto* sizeRow = new QHBoxLayout();
    sizeRow->setSpacing(12);

    auto* sizeCombo = new ComboBox(window);
    sizeCombo->setEditable(true);
    sizeCombo->addItems({"8", "9", "10", "11", "12", "14", "16", "18",
                         "20", "24", "28", "36", "48", "72"});
    sizeCombo->setCurrentText("10");
    sizeCombo->setFixedWidth(120);

    auto* sizeLabel = new QLabel("You can set the font size used for this text.", window);
    sizeLabel->setFont(window->themeFont("Body").toQFont());

    auto updateSize = [sizeCombo, sizeLabel]() {
        bool ok = false;
        int pt = sizeCombo->currentText().toInt(&ok);
        if (ok && pt > 0) {
            QFont f = sizeLabel->font();
            f.setPointSize(pt);
            sizeLabel->setFont(f);
        }
    };
    QObject::connect(sizeCombo, &QComboBox::currentTextChanged, window, [updateSize]() { updateSize(); });
    if (sizeCombo->lineEdit())
        QObject::connect(sizeCombo->lineEdit(), &QLineEdit::editingFinished, window, [updateSize]() { updateSize(); });
    sizeRow->addWidget(sizeCombo, 0);
    sizeRow->addWidget(sizeLabel, 1);
    root->addLayout(sizeRow);

    // 主题切换按钮
    auto* themeBtn = new Button("Switch Theme", window);
    themeBtn->setFixedSize(120, 32);
    root->addWidget(themeBtn, 0, Qt::AlignLeft);
    QObject::connect(themeBtn, &Button::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light
                                    ? FluentElement::Dark : FluentElement::Light);
    });

    attachFluentDelegateToComboPopup(colorsCombo);
    attachFluentDelegateToComboPopup(fontCombo);
    attachFluentDelegateToComboPopup(sizeCombo);

    window->show();
    qApp->exec();
}

