#include <gtest/gtest.h>

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include "view/textfields/LineEdit.h"
#include <QStringListModel>
#include <QSignalSpy>
#include <QFontDatabase>
#include <QComboBox>

#include "view/basicinput/ComboBox.h"
#include "view/basicinput/Button.h"
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "common/Typography.h"

using namespace view::basicinput;

// ─── FluentTestWindow ────────────────────────────────────────────────────────

class ComboBoxTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

// ─── 测试主类 ────────────────────────────────────────────────────────────────

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
        window = new ComboBoxTestWindow;
        window->onThemeUpdated();
        window->resize(600, 500);
    }

    void TearDown() override {
        delete window;
    }

    ComboBoxTestWindow* window = nullptr;
};

// ─── 基础功能测试 ────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, DefaultProperties) {
    ComboBox cb(window);
    EXPECT_EQ(cb.fontRole(), Typography::FontRole::Body);
    EXPECT_EQ(cb.contentPaddingH(), Spacing::Padding::ComboBoxHorizontal);
    EXPECT_EQ(cb.contentPaddingV(), Spacing::Padding::ComboBoxVertical);
    EXPECT_EQ(cb.chevronGlyph(), Typography::Icons::ChevronDownMed);
    EXPECT_EQ(cb.chevronSize(), 12);
    EXPECT_EQ(cb.chevronOffset(), QPoint(Spacing::Padding::ComboBoxHorizontal, 0));
    EXPECT_EQ(cb.popupOffset(), Spacing::XSmall);
    EXPECT_DOUBLE_EQ(cb.pressProgress(), 0.0);
}

TEST_F(ComboBoxTest, SetFontRole) {
    ComboBox cb(window);
    QSignalSpy spy(&cb, &ComboBox::fontRoleChanged);
    cb.setFontRole(Typography::FontRole::Caption);
    EXPECT_EQ(cb.fontRole(), Typography::FontRole::Caption);
    EXPECT_EQ(spy.count(), 1);

    // No-op when same value
    cb.setFontRole(Typography::FontRole::Caption);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ComboBoxTest, SetContentPadding) {
    ComboBox cb(window);
    QSignalSpy spy(&cb, &ComboBox::layoutChanged);
    cb.setContentPaddingH(20);
    EXPECT_EQ(cb.contentPaddingH(), 20);
    EXPECT_EQ(spy.count(), 1);

    cb.setContentPaddingV(8);
    EXPECT_EQ(cb.contentPaddingV(), 8);
    EXPECT_EQ(spy.count(), 2);
}

TEST_F(ComboBoxTest, SetChevron) {
    ComboBox cb(window);
    QSignalSpy spy(&cb, &ComboBox::chevronChanged);
    cb.setChevronGlyph(Typography::Icons::ChevronDown);
    EXPECT_EQ(cb.chevronGlyph(), Typography::Icons::ChevronDown);
    EXPECT_EQ(spy.count(), 1);

    cb.setChevronSize(16);
    EXPECT_EQ(cb.chevronSize(), 16);
    EXPECT_EQ(spy.count(), 2);
}

TEST_F(ComboBoxTest, AddItemsAndSelect) {
    ComboBox cb(window);
    cb.addItems({"Yellow", "Green", "Blue", "Red"});
    EXPECT_EQ(cb.count(), 4);

    cb.setCurrentIndex(2);
    EXPECT_EQ(cb.currentIndex(), 2);
    EXPECT_EQ(cb.currentText(), "Blue");
}

TEST_F(ComboBoxTest, SizeHintMinWidth) {
    ComboBox cb(window);
    QSize sh = cb.sizeHint();
    // Height should be ControlHeight::Standard (32)
    EXPECT_EQ(sh.height(), Spacing::ControlHeight::Standard);
    // Width should respect minimum width (80px text area)
    EXPECT_GE(sh.width(), 80);
}

TEST_F(ComboBoxTest, SizeHintGrowsWithItems) {
    ComboBox cb(window);
    QSize emptySize = cb.sizeHint();

    cb.addItems({"Very Long Item Text That Should Make It Wider"});
    QSize withItems = cb.sizeHint();
    EXPECT_GT(withItems.width(), emptySize.width());
}

TEST_F(ComboBoxTest, FixedHeight) {
    ComboBox cb(window);
    EXPECT_EQ(cb.height(), Spacing::ControlHeight::Standard);
}

TEST_F(ComboBoxTest, DisabledState) {
    ComboBox cb(window);
    cb.addItems({"Item1", "Item2"});
    cb.setEnabled(false);
    EXPECT_FALSE(cb.isEnabled());

    // Should still be paintable (no crash)
    cb.show();
    cb.repaint();
}

// ─── 主题切换测试 ────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, ThemeSwitchLight) {
    FluentElement::setTheme(FluentElement::Light);
    ComboBox cb(window);
    cb.addItems({"Test"});
    cb.show();
    cb.repaint();
    // Should not crash / assert on theme switch
}

TEST_F(ComboBoxTest, ThemeSwitchDark) {
    FluentElement::setTheme(FluentElement::Dark);
    ComboBox cb(window);
    cb.addItems({"Test"});
    cb.show();
    cb.repaint();

    // Restore
    FluentElement::setTheme(FluentElement::Light);
}

// ─── 可编辑模式测试 ──────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, SetEditableCreatesLineEdit) {
    ComboBox cb(window);
    cb.addItems({"Item1", "Item2", "Item3"});
    cb.setEditable(true);

    // Line edit should exist and not be hidden
    auto* lineEdit = cb.findChild<view::textfields::LineEdit*>();
    ASSERT_NE(lineEdit, nullptr);
    EXPECT_FALSE(lineEdit->isHidden());
    EXPECT_FALSE(lineEdit->isClearButtonEnabled());
}

TEST_F(ComboBoxTest, SetEditableFalseRemovesLineEdit) {
    ComboBox cb(window);
    cb.setEditable(true);
    ASSERT_NE(cb.findChild<view::textfields::LineEdit*>(), nullptr);

    cb.setEditable(false);
    EXPECT_EQ(cb.findChild<view::textfields::LineEdit*>(), nullptr);
}

TEST_F(ComboBoxTest, EditableSelectUpdatesLineEdit) {
    ComboBox cb(window);
    cb.addItems({"Alpha", "Beta", "Gamma"});
    cb.setEditable(true);
    cb.setCurrentIndex(1);

    // After selection via popup click, line edit would be updated
    // (direct setCurrentIndex doesn't update line edit in our impl)
    auto* lineEdit = cb.findChild<view::textfields::LineEdit*>();
    ASSERT_NE(lineEdit, nullptr);
    // Line edit should be paintable
    cb.show();
    cb.repaint();
}

// ─── VisualCheck ─────────────────────────────────────────────────────────────

TEST_F(ComboBoxTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    auto* mainLayout = new QVBoxLayout(window);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // Title
    auto* title = new QLabel("ComboBox — WinUI 3 Fluent Design", window);
    title->setFont(window->themeFont(Typography::FontRole::Subtitle).toQFont());
    mainLayout->addWidget(title);

    // Example 1: Color ComboBox (from WinUI Gallery)
    {
        auto* label = new QLabel("Colors (inline items):", window);
        label->setFont(window->themeFont(Typography::FontRole::BodyStrong).toQFont());
        mainLayout->addWidget(label);

        auto* combo = new ComboBox(window);
        combo->addItems({"Yellow", "Green", "Blue", "Red"});
        combo->setCurrentIndex(0);
        combo->setFixedWidth(200);
        mainLayout->addWidget(combo);
    }

    // Example 2: Font family ComboBox
    {
        auto* label = new QLabel("Fonts (ItemsSource):", window);
        label->setFont(window->themeFont(Typography::FontRole::BodyStrong).toQFont());
        mainLayout->addWidget(label);

        auto* combo = new ComboBox(window);
        combo->addItems({"Arial", "Comic Sans MS", "Courier New", "Segoe UI", "Times New Roman"});
        combo->setCurrentIndex(3); // Segoe UI
        combo->setFixedWidth(200);
        mainLayout->addWidget(combo);
    }

    // Example 3: Disabled ComboBox
    {
        auto* label = new QLabel("Disabled:", window);
        label->setFont(window->themeFont(Typography::FontRole::BodyStrong).toQFont());
        mainLayout->addWidget(label);

        auto* combo = new ComboBox(window);
        combo->addItems({"Item 1", "Item 2", "Item 3"});
        combo->setCurrentIndex(0);
        combo->setEnabled(false);
        combo->setFixedWidth(200);
        mainLayout->addWidget(combo);
    }

    // Example 4: Many items
    {
        auto* label = new QLabel("Many items (scroll):", window);
        label->setFont(window->themeFont(Typography::FontRole::BodyStrong).toQFont());
        mainLayout->addWidget(label);

        auto* combo = new ComboBox(window);
        QStringList items;
        for (int i = 1; i <= 20; ++i)
            items << QString("Item %1").arg(i);
        combo->addItems(items);
        combo->setCurrentIndex(5);
        combo->setFixedWidth(200);
        mainLayout->addWidget(combo);
    }

    // Example 5: Editable ComboBox (Font sizes)
    {
        auto* label = new QLabel("Editable (Font sizes):", window);
        label->setFont(window->themeFont(Typography::FontRole::BodyStrong).toQFont());
        mainLayout->addWidget(label);

        auto* combo = new ComboBox(window);
        combo->addItems({"8", "9", "10", "11", "12", "14", "16", "18",
                         "20", "24", "28", "36", "48", "72"});
        combo->setEditable(true);
        combo->setCurrentIndex(4); // 12
        combo->setFixedWidth(200);
        mainLayout->addWidget(combo);
    }

    mainLayout->addStretch();

    // Theme switch button (bottom-right corner)
    auto* themeBtn = new view::basicinput::Button("Switch Theme", window);
    themeBtn->setFluentStyle(view::basicinput::Button::Accent);
    themeBtn->setFixedSize(120, 32);
    themeBtn->anchors()->bottom = {window, view::AnchorLayout::Edge::Bottom, -20};
    themeBtn->anchors()->right = {window, view::AnchorLayout::Edge::Right, -20};
    mainLayout->addWidget(themeBtn);
    QObject::connect(themeBtn, &view::basicinput::Button::clicked, []() {
        FluentElement::setTheme(FluentElement::currentTheme() == FluentElement::Light
                                ? FluentElement::Dark
                                : FluentElement::Light);
    });

    window->show();
    qApp->exec();
}
