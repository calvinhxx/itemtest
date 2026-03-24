/**
 * Fluent ComboBox 测试与可视化样例。
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFontDatabase>
#include <QLineEdit>
#include <QSignalSpy>

#include "common/Spacing.h"
#include "common/Typography.h"
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

void attachFluentDelegateToComboPopup(ComboBox* cb) {
    auto* lv = qobject_cast<ListView*>(cb->view());
    if (!lv)
        return;
    lv->setItemDelegate(new listview_test::FluentListItemDelegate(
        static_cast<FluentElement*>(lv), comboPopupRowHeight(), lv, lv));
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

// ── 基础属性 ──────────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, DefaultPropertyValues) {
    ComboBox cb;
    EXPECT_EQ(cb.fontRole(), "Body");
    EXPECT_EQ(cb.contentPaddingH(), Spacing::Padding::ComboBoxHorizontal);
    EXPECT_EQ(cb.arrowWidth(), 24);
    EXPECT_EQ(cb.chevronGlyph(), Typography::Icons::ChevronDown);
    EXPECT_EQ(cb.chevronSize(), Typography::FontSize::Caption);
    EXPECT_EQ(cb.chevronOffset(), QPoint());
    EXPECT_DOUBLE_EQ(cb.pressProgress(), 0.0);
}

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

// ── ChevronOffset ─────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, ChevronOffsetDefaultZero) {
    ComboBox cb;
    EXPECT_EQ(cb.chevronOffset(), QPoint(0, 0));
}

TEST_F(ComboBoxTest, SetChevronOffsetEmitsSignal) {
    ComboBox cb;
    QSignalSpy spy(&cb, &ComboBox::chevronChanged);
    cb.setChevronOffset(QPoint(8, 2));
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(cb.chevronOffset(), QPoint(8, 2));
}

TEST_F(ComboBoxTest, SetSameChevronOffsetNoSignal) {
    ComboBox cb;
    cb.setChevronOffset(QPoint(8, 2));
    QSignalSpy spy(&cb, &ComboBox::chevronChanged);
    cb.setChevronOffset(QPoint(8, 2));
    EXPECT_EQ(spy.count(), 0);
}

// ── Editable ──────────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, EditableModeHasLineEdit) {
    ComboBox cb;
    EXPECT_EQ(cb.lineEdit(), nullptr);
    cb.setEditable(true);
    ASSERT_NE(cb.lineEdit(), nullptr);
}

TEST_F(ComboBoxTest, EditableLineEditTransparentBackground) {
    ComboBox cb;
    cb.setEditable(true);
    auto* le = cb.lineEdit();
    ASSERT_NE(le, nullptr);
    EXPECT_FALSE(le->autoFillBackground());
}

// ── Selection ─────────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, SelectionIndex) {
    ComboBox cb;
    cb.addItems({"Blue", "Green", "Red", "Yellow"});
    EXPECT_EQ(cb.currentIndex(), 0);
    cb.setCurrentIndex(2);
    EXPECT_EQ(cb.currentText(), "Red");
}

TEST_F(ComboBoxTest, PlaceholderTextWhenNoSelection) {
    ComboBox cb;
    cb.setPlaceholderText("Pick a color");
    EXPECT_EQ(cb.placeholderText(), "Pick a color");
    EXPECT_EQ(cb.currentIndex(), -1);
}

// ── Disabled ──────────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, DisabledState) {
    ComboBox cb;
    cb.addItems({"A", "B"});
    cb.setEnabled(false);
    EXPECT_FALSE(cb.isEnabled());
}

// ── VisualCheck ───────────────────────────────────────────────────────────────

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

    // 1. Colors + PlaceholderText + Width=200
    auto* colorsLabel = new QLabel(QStringLiteral("Colors"), window);
    colorsLabel->setFont(window->themeFont("BodyStrong").toQFont());
    root->addWidget(colorsLabel);

    auto* colorsCombo = new ComboBox(window);
    colorsCombo->setPlaceholderText(QStringLiteral("Pick a color"));
    colorsCombo->addItems({QStringLiteral("Blue"), QStringLiteral("Green"),
                           QStringLiteral("Red"), QStringLiteral("Yellow")});
    colorsCombo->setFixedWidth(200);
    root->addWidget(colorsCombo, 0, Qt::AlignLeft);

    // 2. Font + ItemsSource + SelectedIndex=2
    auto* fontHeader = new QLabel("Font", window);
    fontHeader->setFont(window->themeFont("BodyStrong").toQFont());
    root->addWidget(fontHeader);

    auto* fontRow = new QHBoxLayout();
    fontRow->setSpacing(12);

    auto* fontLabel = new QLabel("You can set the font used for this text.", window);
    fontLabel->setFont(window->themeFont("Body").toQFont());

    auto* fontCombo = new ComboBox(window);
    fontCombo->addItems({"Segoe UI", "Courier New", "Times New Roman", "Arial"});
    fontCombo->setCurrentIndex(2);
    fontCombo->setFixedWidth(200);
    QObject::connect(fontCombo, &QComboBox::currentTextChanged, [fontLabel](const QString& family) {
        QFont f = fontLabel->font();
        f.setFamily(family);
        fontLabel->setFont(f);
    });
    fontRow->addWidget(fontCombo, 0);
    fontRow->addWidget(fontLabel, 1);
    root->addLayout(fontRow);

    // 3. Font Size + IsEditable + 字号列表
    auto* sizeHeader = new QLabel("Font Size", window);
    sizeHeader->setFont(window->themeFont("BodyStrong").toQFont());
    root->addWidget(sizeHeader);

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
        if (ok && pt >= 8 && pt <= 100) {
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

    // 4. Disabled 状态
    auto* disabledHeader = new QLabel("Disabled", window);
    disabledHeader->setFont(window->themeFont("BodyStrong").toQFont());
    root->addWidget(disabledHeader);

    auto* disabledCombo = new ComboBox(window);
    disabledCombo->addItems({"Option A", "Option B"});
    disabledCombo->setCurrentIndex(0);
    disabledCombo->setEnabled(false);
    disabledCombo->setFixedWidth(200);
    root->addWidget(disabledCombo, 0, Qt::AlignLeft);

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
    attachFluentDelegateToComboPopup(disabledCombo);

    window->show();
    qApp->exec();
}

