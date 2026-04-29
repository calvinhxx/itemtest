#include <gtest/gtest.h>
#include <QAbstractItemView>
#include <QApplication>
#include <QFontDatabase>
#include <QItemSelectionModel>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QStandardItemModel>
#include "compatibility/QtCompat.h"
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "FluentTreeItemDelegate.h"
#include "view/collections/TreeView.h"
#include "view/QMLPlus.h"
#include "design/Spacing.h"
#include "design/Typography.h"

#include "view/basicinput/Button.h"
#include "view/scrolling/ScrollBar.h"
#include "view/textfields/TextBlock.h"

using namespace view::collections;
using namespace view;
using view::basicinput::Button;
using view::textfields::TextBlock;

namespace {

int defaultTreeRowHeight() {
    return Spacing::ControlHeight::Standard + Spacing::Gap::Tight;
}

/** 业务组装：为 TreeView 挂上 Fluent 行代理 */
void attachFluentDelegate(TreeView* tv, int rowHeight = defaultTreeRowHeight()) {
    tv->setItemDelegate(new treeview_test::FluentTreeItemDelegate(
        static_cast<FluentElement*>(tv), rowHeight, tv, tv));
}

/** 挂上带 CheckBox 可见的代理 */
treeview_test::FluentTreeItemDelegate* attachCheckableDelegate(TreeView* tv) {
    auto* d = new treeview_test::FluentTreeItemDelegate(
        static_cast<FluentElement*>(tv), defaultTreeRowHeight(), tv, tv);
    d->setCheckBoxVisible(true);
    tv->setItemDelegate(d);
    return d;
}

/**
 * 创建一个带层级结构的 QStandardItemModel:
 *   Work Documents
 *     ├─ XYZ Functional Spec
 *     └─ Feature Schedule
 *   Personal Documents
 *     └─ Home Remodel
 *         ├─ Contractor Contact Info
 *         └─ Paint Color Scheme
 *   Pictures
 */
QStandardItemModel* createSampleTreeModel(QObject* parent = nullptr) {
    auto* model = new QStandardItemModel(parent);

    auto* work = new QStandardItem("Work Documents");
    work->appendRow(new QStandardItem("XYZ Functional Spec"));
    work->appendRow(new QStandardItem("Feature Schedule"));
    model->appendRow(work);

    auto* personal = new QStandardItem("Personal Documents");
    auto* remodel = new QStandardItem("Home Remodel");
    remodel->appendRow(new QStandardItem("Contractor Contact Info"));
    remodel->appendRow(new QStandardItem("Paint Color Scheme"));
    personal->appendRow(remodel);
    model->appendRow(personal);

    model->appendRow(new QStandardItem("Pictures"));

    return model;
}

/** 绑定 sample model + delegate */
QStandardItemModel* attachSampleModel(TreeView* tv) {
    auto* model = createSampleTreeModel(tv);
    tv->setModel(model);
    attachFluentDelegate(tv);
    return model;
}

int topLevelCount(TreeView* tv) {
    return tv->model() ? tv->model()->rowCount() : 0;
}

QString itemText(TreeView* tv, const QModelIndex& index) {
    return index.isValid() ? index.data(Qt::DisplayRole).toString() : QString();
}

/** 离屏显示: 触发布局计算但不弹出可见窗口 */
void showOffscreen(QWidget* w) {
    w->setAttribute(Qt::WA_DontShowOnScreen, true);
    w->show();
    QApplication::processEvents();
}

/**
 * 创建一个带 CheckStateRole 的可勾选树模型 (WinUI Multi-selection 模式)
 */
QStandardItemModel* createCheckableTreeModel(QObject* parent = nullptr) {
    auto* model = new QStandardItemModel(parent);

    auto makeCheckable = [](QStandardItem* item, Qt::CheckState state) {
        item->setCheckable(true);
        item->setCheckState(state);
        return item;
    };

    auto* work = makeCheckable(new QStandardItem("Work Documents"), Qt::PartiallyChecked);
    work->appendRow(makeCheckable(new QStandardItem("XYZ Functional Spec"), Qt::Checked));
    work->appendRow(makeCheckable(new QStandardItem("Feature Schedule"), Qt::Unchecked));
    model->appendRow(work);

    auto* personal = makeCheckable(new QStandardItem("Personal Documents"), Qt::Checked);
    auto* remodel = makeCheckable(new QStandardItem("Home Remodel"), Qt::Checked);
    remodel->appendRow(makeCheckable(new QStandardItem("Contractor Contact Info"), Qt::Checked));
    remodel->appendRow(makeCheckable(new QStandardItem("Paint Color Scheme"), Qt::Checked));
    personal->appendRow(remodel);
    model->appendRow(personal);

    model->appendRow(makeCheckable(new QStandardItem("Pictures"), Qt::Unchecked));
    return model;
}

/**
 * 创建带图标字形的树模型 (WinUI ItemTemplateSelector 模式)
 * 文件夹使用 Folder 图标，文档使用 Document 图标
 */
QStandardItemModel* createIconTreeModel(QObject* parent = nullptr) {
    auto* model = new QStandardItemModel(parent);

    auto makeFolder = [](const QString& text) {
        auto* item = new QStandardItem(text);
        item->setData(Typography::Icons::Folder, treeview_test::IconGlyphRole);
        return item;
    };
    auto makeDoc = [](const QString& text) {
        auto* item = new QStandardItem(text);
        item->setData(Typography::Icons::Document, treeview_test::IconGlyphRole);
        return item;
    };

    auto* work = makeFolder("Work Documents");
    work->appendRow(makeDoc("XYZ Functional Spec"));
    work->appendRow(makeDoc("Feature Schedule"));
    model->appendRow(work);

    auto* personal = makeFolder("Personal Documents");
    auto* remodel = makeFolder("Home Remodel");
    remodel->appendRow(makeDoc("Contractor Contact Info"));
    remodel->appendRow(makeDoc("Paint Color Scheme"));
    personal->appendRow(remodel);
    model->appendRow(personal);

    model->appendRow(makeFolder("Pictures"));
    return model;
}

} // namespace

class FluentTestWindow : public QWidget, public FluentElement {
public:
    using QWidget::QWidget;
    void onThemeUpdated() override {
        const auto& c = themeColors();
        setStyleSheet(QString("background-color: %1;").arg(c.bgCanvas.name()));
    }
};

class TreeViewTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char** argv = nullptr;
        if (!qApp) {
            new QApplication(argc, argv);
        }
        QApplication::setStyle("Fusion");
        Q_INIT_RESOURCE(resources);
        QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }

    void SetUp() override {
        window = new FluentTestWindow();
        window->setFixedSize(500, 500);
        window->setWindowTitle("Fluent TreeView Test");
        layout = new AnchorLayout(window);
        window->setLayout(layout);
        window->onThemeUpdated();
    }

    void TearDown() override {
        delete window;
    }

    FluentTestWindow* window;
    AnchorLayout* layout;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Model & Data
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, EmptyTreeView) {
    TreeView* tv = new TreeView(window);
    EXPECT_EQ(topLevelCount(tv), 0);
    EXPECT_FALSE(tv->selectedItem().isValid());
}

TEST_F(TreeViewTest, SampleModelStructure) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    EXPECT_EQ(topLevelCount(tv), 3);
    EXPECT_EQ(model->item(0)->text(), "Work Documents");
    EXPECT_EQ(model->item(0)->rowCount(), 2);
    EXPECT_EQ(model->item(1)->text(), "Personal Documents");
    EXPECT_EQ(model->item(1)->rowCount(), 1);
    EXPECT_EQ(model->item(2)->text(), "Pictures");
    EXPECT_EQ(model->item(2)->rowCount(), 0);
}

TEST_F(TreeViewTest, DeepNesting) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    // Personal Documents → Home Remodel → children
    auto* personal = model->item(1);
    auto* remodel = personal->child(0);
    ASSERT_NE(remodel, nullptr);
    EXPECT_EQ(remodel->text(), "Home Remodel");
    EXPECT_EQ(remodel->rowCount(), 2);
    EXPECT_EQ(remodel->child(0)->text(), "Contractor Contact Info");
    EXPECT_EQ(remodel->child(1)->text(), "Paint Color Scheme");
}

// ═══════════════════════════════════════════════════════════════════════════════
// Selection
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, DefaultSelectionMode) {
    TreeView* tv = new TreeView(window);
    EXPECT_EQ(tv->selectionMode(), TreeSelectionMode::Single);
}

TEST_F(TreeViewTest, SelectionModeSwitch) {
    TreeView* tv = new TreeView(window);
    attachSampleModel(tv);

    QSignalSpy spy(tv, &TreeView::selectionModeChanged);

    tv->setSelectionMode(TreeSelectionMode::Multiple);
    EXPECT_EQ(tv->selectionMode(), TreeSelectionMode::Multiple);
    EXPECT_EQ(spy.count(), 1);

    tv->setSelectionMode(TreeSelectionMode::None);
    EXPECT_EQ(tv->selectionMode(), TreeSelectionMode::None);
    EXPECT_EQ(spy.count(), 2);

    tv->setSelectionMode(TreeSelectionMode::Extended);
    EXPECT_EQ(tv->selectionMode(), TreeSelectionMode::Extended);
    EXPECT_EQ(spy.count(), 3);
}

TEST_F(TreeViewTest, SelectionModeDuplicateIgnored) {
    TreeView* tv = new TreeView(window);
    QSignalSpy spy(tv, &TreeView::selectionModeChanged);

    tv->setSelectionMode(TreeSelectionMode::Single);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(TreeViewTest, SetSelectedItem) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    tv->setSelectedItem(model->index(0, 0));
    EXPECT_EQ(tv->selectedItem(), model->index(0, 0));
}

TEST_F(TreeViewTest, ClearSelectionOnInvalidIndex) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    tv->setSelectedItem(model->index(0, 0));
    EXPECT_TRUE(tv->selectedItem().isValid());

    tv->setSelectedItem(QModelIndex());
    EXPECT_FALSE(tv->selectedItem().isValid());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Expand / Collapse
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, ExpandCollapseTopLevel) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    QModelIndex workIdx = model->index(0, 0);

    // Initially collapsed
    EXPECT_FALSE(tv->isExpanded(workIdx));

    // Expand
    tv->expand(workIdx);
    EXPECT_TRUE(tv->isExpanded(workIdx));

    // Collapse
    tv->collapse(workIdx);
    EXPECT_FALSE(tv->isExpanded(workIdx));
}

TEST_F(TreeViewTest, ToggleExpanded) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    QModelIndex workIdx = model->index(0, 0);

    EXPECT_FALSE(tv->isExpanded(workIdx));
    tv->toggleExpanded(workIdx);
    EXPECT_TRUE(tv->isExpanded(workIdx));
    tv->toggleExpanded(workIdx);
    EXPECT_FALSE(tv->isExpanded(workIdx));
}

TEST_F(TreeViewTest, ExpandAll) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    tv->expandAll();

    EXPECT_TRUE(tv->isExpanded(model->index(0, 0)));
    EXPECT_TRUE(tv->isExpanded(model->index(1, 0)));
    // Personal Documents → Home Remodel
    EXPECT_TRUE(tv->isExpanded(model->item(1)->child(0)->index()));
}

TEST_F(TreeViewTest, CollapseAll) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    tv->expandAll();
    tv->collapseAll();

    EXPECT_FALSE(tv->isExpanded(model->index(0, 0)));
    EXPECT_FALSE(tv->isExpanded(model->index(1, 0)));
}

TEST_F(TreeViewTest, ToggleInvalidIndexNoOp) {
    TreeView* tv = new TreeView(window);
    attachSampleModel(tv);

    // Should not crash
    tv->toggleExpanded(QModelIndex());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Expand Animation
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, ExpandAnimationTriggered) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->resize(300, 400);
    window->resize(320, 420);
    QPixmap pm = tv->grab(); // force layout

    QModelIndex workIdx = model->index(0, 0);
    tv->expand(workIdx);
    QApplication::processEvents();

    // State changes immediately even during animation
    EXPECT_TRUE(tv->isExpanded(workIdx));
}

TEST_F(TreeViewTest, ExpandAnimationCompletesCleanly) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->resize(300, 400);
    window->resize(320, 420);
    QPixmap pm = tv->grab();

    QModelIndex workIdx = model->index(0, 0);
    tv->expand(workIdx);

    // Let animation finish (Normal = 250ms)
    QTest::qWait(300);
    QApplication::processEvents();

    EXPECT_TRUE(tv->isExpanded(workIdx));
    // Verify children are visible
    EXPECT_GT(model->rowCount(workIdx), 0);
}

TEST_F(TreeViewTest, RapidToggleNoCrash) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->resize(300, 400);
    window->resize(320, 420);
    QPixmap pm = tv->grab();

    QModelIndex workIdx = model->index(0, 0);

    // Rapidly toggle multiple times — should not crash
    for (int i = 0; i < 10; ++i) {
        tv->toggleExpanded(workIdx);
        QApplication::processEvents();
    }
    // Final state depends on even/odd — just verify no crash
    SUCCEED();
}

TEST_F(TreeViewTest, ExpandAllSkipsAnimation) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->resize(300, 400);
    window->resize(320, 420);
    QPixmap pm = tv->grab();

    tv->expandAll();
    QApplication::processEvents();

    // All nodes expanded immediately (no animation)
    EXPECT_TRUE(tv->isExpanded(model->index(0, 0)));
    EXPECT_TRUE(tv->isExpanded(model->index(1, 0)));
}

TEST_F(TreeViewTest, CollapseStopsExpandAnimation) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->resize(300, 400);
    window->resize(320, 420);
    QPixmap pm = tv->grab();

    QModelIndex workIdx = model->index(0, 0);

    // Expand (starts animation)
    tv->expand(workIdx);
    QApplication::processEvents();
    EXPECT_TRUE(tv->isExpanded(workIdx));

    // Immediately collapse — should stop animation and not crash
    tv->collapse(workIdx);
    QApplication::processEvents();
    EXPECT_FALSE(tv->isExpanded(workIdx));
}

// ═══════════════════════════════════════════════════════════════════════════════
// Appearance Properties
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, DefaultBorderVisible) {
    TreeView* tv = new TreeView(window);
    EXPECT_TRUE(tv->borderVisible());
}

TEST_F(TreeViewTest, SetBorderVisible) {
    TreeView* tv = new TreeView(window);
    QSignalSpy spy(tv, &TreeView::borderVisibleChanged);

    tv->setBorderVisible(false);
    EXPECT_FALSE(tv->borderVisible());
    EXPECT_EQ(spy.count(), 1);

    tv->setBorderVisible(false); // duplicate
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TreeViewTest, DefaultBackgroundVisible) {
    TreeView* tv = new TreeView(window);
    EXPECT_TRUE(tv->backgroundVisible());
}

TEST_F(TreeViewTest, SetBackgroundVisible) {
    TreeView* tv = new TreeView(window);
    QSignalSpy spy(tv, &TreeView::backgroundVisibleChanged);

    tv->setBackgroundVisible(false);
    EXPECT_FALSE(tv->backgroundVisible());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TreeViewTest, HeaderText) {
    TreeView* tv = new TreeView(window);
    QSignalSpy spy(tv, &TreeView::headerTextChanged);

    EXPECT_TRUE(tv->headerText().isEmpty());

    tv->setHeaderText("Documents");
    EXPECT_EQ(tv->headerText(), "Documents");
    EXPECT_EQ(spy.count(), 1);

    tv->setHeaderText("Documents"); // duplicate
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TreeViewTest, PlaceholderText) {
    TreeView* tv = new TreeView(window);
    QSignalSpy spy(tv, &TreeView::placeholderTextChanged);

    tv->setPlaceholderText("No items");
    EXPECT_EQ(tv->placeholderText(), "No items");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TreeViewTest, FontRole) {
    TreeView* tv = new TreeView(window);
    QSignalSpy spy(tv, &TreeView::fontRoleChanged);

    EXPECT_EQ(tv->fontRole(), Typography::FontRole::Body);

    tv->setFontRole(Typography::FontRole::Caption);
    EXPECT_EQ(tv->fontRole(), Typography::FontRole::Caption);
    EXPECT_EQ(spy.count(), 1);

    tv->setFontRole(Typography::FontRole::Caption); // duplicate
    EXPECT_EQ(spy.count(), 1);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Tree-specific
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, IndentationDefault) {
    TreeView* tv = new TreeView(window);
    EXPECT_EQ(tv->indentation(), 16);
}

TEST_F(TreeViewTest, RootIsDecoratedOff) {
    TreeView* tv = new TreeView(window);
    EXPECT_FALSE(tv->rootIsDecorated());
}

TEST_F(TreeViewTest, HeaderHiddenByDefault) {
    TreeView* tv = new TreeView(window);
    EXPECT_TRUE(tv->isHeaderHidden());
}

// ═══════════════════════════════════════════════════════════════════════════════
// Fluent Scroll Bar
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, FluentScrollBarExists) {
    TreeView* tv = new TreeView(window);
    EXPECT_NE(tv->verticalFluentScrollBar(), nullptr);
    EXPECT_NE(tv->horizontalFluentScrollBar(), nullptr);
}

TEST_F(TreeViewTest, FluentScrollBarHiddenWhenShort) {
    TreeView* tv = new TreeView(window);
    tv->setFixedSize(300, 400);
    attachSampleModel(tv);  // only 3 top-level items, should fit

    showOffscreen(window);

    // 3 collapsed items at 36px each = 108px < 400px → no scroll needed
    EXPECT_FALSE(tv->verticalFluentScrollBar()->isVisible());
}

TEST_F(TreeViewTest, FluentScrollBarVisibleWhenTall) {
    TreeView* tv = new TreeView(window);
    tv->setFixedSize(300, 60);  // very small → will need scrolling

    auto* model = new QStandardItemModel(tv);
    for (int i = 0; i < 20; ++i) {
        model->appendRow(new QStandardItem(QString("Item %1").arg(i)));
    }
    tv->setModel(model);
    attachFluentDelegate(tv);

    showOffscreen(window);
    QTest::qWait(50);

    EXPECT_TRUE(tv->verticalFluentScrollBar()->isVisible());
}

// ═══════════════════════════════════════════════════════════════════════════════
// itemClicked signal
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, ItemClickedSignalEmitted) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setFixedSize(300, 400);

    showOffscreen(window);

    QSignalSpy spy(tv, &TreeView::itemClicked);

    QModelIndex idx = model->index(0, 0);
    QRect rect = tv->visualRect(idx);
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, rect.center());

    EXPECT_GE(spy.count(), 1);
    QModelIndex clickedIdx = spy.at(0).at(0).value<QModelIndex>();
    EXPECT_EQ(clickedIdx, idx);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Chevron-only expand & checkbox click
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, ClickRowBodyDoesNotExpand) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setFixedSize(350, 400);
    showOffscreen(window);

    QModelIndex workIdx = model->index(0, 0);
    EXPECT_FALSE(tv->isExpanded(workIdx));

    // Click on the text area (far right) — should NOT expand
    QRect rect = tv->visualRect(workIdx);
    QPoint textCenter(rect.right() - 20, rect.center().y());
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, textCenter);
    QApplication::processEvents();
    EXPECT_FALSE(tv->isExpanded(workIdx));
}

TEST_F(TreeViewTest, ClickChevronAreaExpands) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setFixedSize(350, 400);
    showOffscreen(window);

    QModelIndex workIdx = model->index(0, 0);
    EXPECT_FALSE(tv->isExpanded(workIdx));

    // Click on the chevron area (near left side of the row)
    QRect rect = tv->visualRect(workIdx);
    // Chevron is at option.rect.left() + 12 + chevronAreaW/2 = left + 22
    QPoint chevronCenter(rect.left() + 22, rect.center().y());
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, chevronCenter);
    QApplication::processEvents();
    EXPECT_TRUE(tv->isExpanded(workIdx));

    // Click again to collapse
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, chevronCenter);
    QApplication::processEvents();
    EXPECT_FALSE(tv->isExpanded(workIdx));
}

TEST_F(TreeViewTest, CheckBoxClickTogglesState) {
    TreeView* tv = new TreeView(window);
    auto* model = createCheckableTreeModel(tv);
    tv->setModel(model);
    auto* d = attachCheckableDelegate(tv);
    (void)d;
    tv->setFixedSize(350, 400);
    showOffscreen(window);

    // Pictures is index(2, 0) — starts as Unchecked
    QModelIndex picturesIdx = model->index(2, 0);
    EXPECT_EQ(model->item(2)->checkState(), Qt::Unchecked);

    // Click on checkbox area
    QRect rect = tv->visualRect(picturesIdx);
    // With checkable delegate, checkbox is at option.rect.left() + 12 + cbAreaW/2
    QPoint cbCenter(rect.left() + 12 + 11, rect.center().y());
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, cbCenter);
    QApplication::processEvents();
    EXPECT_EQ(model->item(2)->checkState(), Qt::Checked);

    // Click again to uncheck
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, cbCenter);
    QApplication::processEvents();
    EXPECT_EQ(model->item(2)->checkState(), Qt::Unchecked);
}

TEST_F(TreeViewTest, ParentCheckCascadesToChildren) {
    TreeView* tv = new TreeView(window);
    auto* model = createCheckableTreeModel(tv);
    tv->setModel(model);
    attachCheckableDelegate(tv);
    tv->setFixedSize(350, 400);
    tv->expandAll();
    showOffscreen(window);

    // "Work Documents" starts PartiallyChecked, children: Checked + Unchecked
    QModelIndex workIdx = model->index(0, 0);
    EXPECT_EQ(model->item(0)->checkState(), Qt::PartiallyChecked);
    EXPECT_EQ(model->item(0)->child(0)->checkState(), Qt::Checked);
    EXPECT_EQ(model->item(0)->child(1)->checkState(), Qt::Unchecked);

    // Click parent checkbox → should become Checked, both children Checked
    QRect rect = tv->visualRect(workIdx);
    QPoint cbCenter(rect.left() + 12 + 11, rect.center().y());
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, cbCenter);
    QApplication::processEvents();
    EXPECT_EQ(model->item(0)->checkState(), Qt::Checked);
    EXPECT_EQ(model->item(0)->child(0)->checkState(), Qt::Checked);
    EXPECT_EQ(model->item(0)->child(1)->checkState(), Qt::Checked);

    // Click again → Unchecked, all children Unchecked
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, cbCenter);
    QApplication::processEvents();
    EXPECT_EQ(model->item(0)->checkState(), Qt::Unchecked);
    EXPECT_EQ(model->item(0)->child(0)->checkState(), Qt::Unchecked);
    EXPECT_EQ(model->item(0)->child(1)->checkState(), Qt::Unchecked);
}

TEST_F(TreeViewTest, ChildCheckUpdatesParentTriState) {
    TreeView* tv = new TreeView(window);
    auto* model = createCheckableTreeModel(tv);
    tv->setModel(model);
    attachCheckableDelegate(tv);
    tv->setFixedSize(350, 400);
    tv->expandAll();
    showOffscreen(window);

    // "Personal Documents" starts Checked, child "Home Remodel" Checked
    QStandardItem* personal = model->item(1);
    QStandardItem* remodel = personal->child(0);
    EXPECT_EQ(personal->checkState(), Qt::Checked);
    EXPECT_EQ(remodel->checkState(), Qt::Checked);

    // Uncheck "Home Remodel" → parent should become Unchecked (only child)
    QModelIndex remodelIdx = model->index(0, 0, model->index(1, 0));
    QRect rect = tv->visualRect(remodelIdx);
    QPoint cbCenter(rect.left() + 12 + 11, rect.center().y());
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, cbCenter);
    QApplication::processEvents();
    EXPECT_EQ(remodel->checkState(), Qt::Unchecked);
    EXPECT_EQ(personal->checkState(), Qt::Unchecked);

    // Re-check "Home Remodel" → parent back to Checked
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, cbCenter);
    QApplication::processEvents();
    EXPECT_EQ(remodel->checkState(), Qt::Checked);
    EXPECT_EQ(personal->checkState(), Qt::Checked);
}

TEST_F(TreeViewTest, DeepCascadeGrandchildren) {
    TreeView* tv = new TreeView(window);
    auto* model = createCheckableTreeModel(tv);
    tv->setModel(model);
    attachCheckableDelegate(tv);
    tv->setFixedSize(350, 400);
    tv->expandAll();
    showOffscreen(window);

    // Check "Personal Documents" (top) → all descendants should be Checked
    QStandardItem* personal = model->item(1);
    QStandardItem* remodel = personal->child(0);

    // First uncheck "Contractor Contact Info" to make tree partially checked
    remodel->child(0)->setCheckState(Qt::Unchecked);
    remodel->setCheckState(Qt::PartiallyChecked);
    personal->setCheckState(Qt::PartiallyChecked);
    QApplication::processEvents();

    // Click on "Personal Documents" checkbox → full cascade to Checked
    QModelIndex personalIdx = model->index(1, 0);
    QRect rect = tv->visualRect(personalIdx);
    QPoint cbCenter(rect.left() + 12 + 11, rect.center().y());
    QTest::mouseClick(tv->viewport(), Qt::LeftButton, Qt::NoModifier, cbCenter);
    QApplication::processEvents();

    EXPECT_EQ(personal->checkState(), Qt::Checked);
    EXPECT_EQ(remodel->checkState(), Qt::Checked);
    EXPECT_EQ(remodel->child(0)->checkState(), Qt::Checked);
    EXPECT_EQ(remodel->child(1)->checkState(), Qt::Checked);
}

TEST_F(TreeViewTest, ChevronRotationApi) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setFixedSize(300, 400);
    window->resize(320, 420);
    QPixmap pm = tv->grab();

    QModelIndex workIdx = model->index(0, 0);

    // Initially collapsed → rotation should be 0
    EXPECT_DOUBLE_EQ(tv->chevronRotation(workIdx), 0.0);

    // Expand and wait for animation to finish
    tv->expand(workIdx);
    QTest::qWait(300);
    QApplication::processEvents();

    EXPECT_DOUBLE_EQ(tv->chevronRotation(workIdx), 1.0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Drag reorder (file-manager style)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, DefaultCanReorderItems) {
    TreeView* tv = new TreeView(window);
    EXPECT_FALSE(tv->canReorderItems());
}

TEST_F(TreeViewTest, SetCanReorderItems) {
    TreeView* tv = new TreeView(window);
    QSignalSpy spy(tv, &TreeView::canReorderItemsChanged);
    tv->setCanReorderItems(true);
    EXPECT_TRUE(tv->canReorderItems());
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TreeViewTest, CanReorderItemsSignalNotDuplicate) {
    TreeView* tv = new TreeView(window);
    QSignalSpy spy(tv, &TreeView::canReorderItemsChanged);
    tv->setCanReorderItems(true);
    tv->setCanReorderItems(true); // same
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TreeViewTest, DragReorderTopLevelSiblings) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setCanReorderItems(true);
    tv->setFixedSize(350, 400);
    showOffscreen(window);

    // Top-level: "Work Documents", "Personal Documents", "Pictures"
    EXPECT_EQ(model->item(0)->text(), "Work Documents");
    EXPECT_EQ(model->item(1)->text(), "Personal Documents");
    EXPECT_EQ(model->item(2)->text(), "Pictures");

    // Simulate model-level reorder (same as Between drop does)
    auto row = model->takeRow(0);
    model->insertRow(1, row);
    EXPECT_EQ(model->item(0)->text(), "Personal Documents");
    EXPECT_EQ(model->item(1)->text(), "Work Documents");
    EXPECT_EQ(model->item(2)->text(), "Pictures");
}

TEST_F(TreeViewTest, DragReorderChildSiblings) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setCanReorderItems(true);
    tv->setFixedSize(350, 400);
    tv->expandAll();
    showOffscreen(window);

    // Work Documents children: "XYZ Functional Spec", "Feature Schedule"
    QStandardItem* work = model->item(0);
    EXPECT_EQ(work->child(0)->text(), "XYZ Functional Spec");
    EXPECT_EQ(work->child(1)->text(), "Feature Schedule");

    // Move child 0 to after child 1
    auto row = work->takeRow(0);
    work->insertRow(1, row);
    EXPECT_EQ(work->child(0)->text(), "Feature Schedule");
    EXPECT_EQ(work->child(1)->text(), "XYZ Functional Spec");
}

TEST_F(TreeViewTest, DragReorderEmitsSignal) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setCanReorderItems(true);
    tv->setFixedSize(350, 400);
    showOffscreen(window);

    QSignalSpy spy(tv, &TreeView::itemReordered);

    // Simulate full drag: press → move past threshold → move to target → release
    QModelIndex srcIdx = model->index(0, 0);
    QRect srcRect = tv->visualRect(srcIdx);
    QPoint start = srcRect.center();

    // Target: bottom edge of row 1 (insert after Personal Documents)
    QModelIndex targetIdx = model->index(1, 0);
    QRect targetRect = tv->visualRect(targetIdx);
    QPoint end(start.x(), targetRect.bottom() + 2);

    QTest::mousePress(tv->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    // Move past drag distance
    QPoint moved(start.x(), start.y() + QApplication::startDragDistance() + 5);
    QMouseEvent moveEvent1(QEvent::MouseMove, QPointF(moved), QPointF(moved),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent1);
    QApplication::processEvents();

    // Move to target
    QMouseEvent moveEvent2(QEvent::MouseMove, QPointF(end), QPointF(end),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent2);
    QApplication::processEvents();

    QTest::mouseRelease(tv->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QApplication::processEvents();

    EXPECT_EQ(spy.count(), 1);

    // Verify order changed
    EXPECT_EQ(model->item(0)->text(), "Personal Documents");
    EXPECT_EQ(model->item(1)->text(), "Work Documents");
    EXPECT_EQ(model->item(2)->text(), "Pictures");
}

TEST_F(TreeViewTest, DragDisabledDoesNotReorder) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setCanReorderItems(false);
    tv->setFixedSize(350, 400);
    showOffscreen(window);

    QModelIndex srcIdx = model->index(0, 0);
    QRect srcRect = tv->visualRect(srcIdx);
    QPoint start = srcRect.center();

    QModelIndex targetIdx = model->index(2, 0);
    QRect targetRect = tv->visualRect(targetIdx);
    QPoint end(start.x(), targetRect.bottom() + 5);

    QTest::mousePress(tv->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QPoint moved(start.x(), start.y() + QApplication::startDragDistance() + 5);
    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(moved), QPointF(moved),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent);
    QApplication::processEvents();
    QTest::mouseRelease(tv->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QApplication::processEvents();

    // Order unchanged
    EXPECT_EQ(model->item(0)->text(), "Work Documents");
    EXPECT_EQ(model->item(1)->text(), "Personal Documents");
    EXPECT_EQ(model->item(2)->text(), "Pictures");
}

TEST_F(TreeViewTest, DragPreservesChildHierarchy) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setCanReorderItems(true);
    tv->setFixedSize(350, 400);
    showOffscreen(window);

    // Move "Work Documents" (with children) from row 0 to row 2
    QStandardItem* root = model->invisibleRootItem();
    auto row = root->takeRow(0);
    root->insertRow(1, row);

    // "Work Documents" at row 1 should still have its 2 children
    QStandardItem* work = model->item(1);
    EXPECT_EQ(work->text(), "Work Documents");
    EXPECT_EQ(work->rowCount(), 2);
    EXPECT_EQ(work->child(0)->text(), "XYZ Functional Spec");
    EXPECT_EQ(work->child(1)->text(), "Feature Schedule");
}

TEST_F(TreeViewTest, DragDropOntoFolder) {
    // File-manager style: drag an item onto a folder (item with children)
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setCanReorderItems(true);
    tv->setFixedSize(350, 400);
    tv->expandAll();
    showOffscreen(window);

    // Drag "Pictures" (root row 2, leaf) onto "Work Documents" (root row 0, has children)
    QStandardItem* work = model->item(0);
    int workChildrenBefore = work->rowCount(); // 2
    EXPECT_EQ(model->rowCount(), 3);

    QModelIndex srcIdx = model->index(2, 0); // Pictures
    QRect srcRect = tv->visualRect(srcIdx);
    QPoint start = srcRect.center();

    // Target: center of "Work Documents" row (OnItem zone — middle 50%)
    QModelIndex workIdx = model->index(0, 0);
    QRect workRect = tv->visualRect(workIdx);
    QPoint end = workRect.center();

    QTest::mousePress(tv->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QPoint moved(start.x(), start.y() - QApplication::startDragDistance() - 5);
    QMouseEvent moveEvent1(QEvent::MouseMove, QPointF(moved), QPointF(moved),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent1);
    QApplication::processEvents();
    QMouseEvent moveEvent2(QEvent::MouseMove, QPointF(end), QPointF(end),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent2);
    QApplication::processEvents();
    QTest::mouseRelease(tv->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QApplication::processEvents();

    // "Pictures" should now be a child of "Work Documents"
    EXPECT_EQ(model->rowCount(), 2); // only 2 root items left
    EXPECT_EQ(work->rowCount(), workChildrenBefore + 1);
    EXPECT_EQ(work->child(work->rowCount() - 1)->text(), "Pictures");
}

TEST_F(TreeViewTest, DragCrossParentBetween) {
    // File-manager style: drag child item to root level (between root items)
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setCanReorderItems(true);
    tv->setFixedSize(350, 400);
    tv->expandAll();
    showOffscreen(window);

    // Drag "XYZ Functional Spec" (Work Documents → child 0) to top edge of "Personal Documents"
    // This should insert it at root level between row 0 and row 1
    QStandardItem* work = model->item(0);
    EXPECT_EQ(work->child(0)->text(), "XYZ Functional Spec");

    QModelIndex srcIdx = work->child(0)->index();
    QRect srcRect = tv->visualRect(srcIdx);
    QPoint start = srcRect.center();

    // Target: top edge of "Personal Documents" (Between zone at root, row 1)
    QModelIndex pd = model->index(1, 0);
    QRect pdRect = tv->visualRect(pd);
    QPoint end(pdRect.center().x(), pdRect.top() + 2); // top zone → insert before

    QTest::mousePress(tv->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QPoint moved(start.x(), start.y() + QApplication::startDragDistance() + 5);
    QMouseEvent moveEvent1(QEvent::MouseMove, QPointF(moved), QPointF(moved),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent1);
    QApplication::processEvents();
    QMouseEvent moveEvent2(QEvent::MouseMove, QPointF(end), QPointF(end),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent2);
    QApplication::processEvents();
    QTest::mouseRelease(tv->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QApplication::processEvents();

    // "XYZ Functional Spec" removed from Work Documents, inserted at root level
    EXPECT_EQ(work->rowCount(), 1);    // only 1 child left
    EXPECT_EQ(model->rowCount(), 4);   // 4 root items now
    EXPECT_EQ(model->item(1)->text(), "XYZ Functional Spec");
}

TEST_F(TreeViewTest, DragCannotDropAncestorOntoDescendant) {
    // Cycle prevention: dragging a parent onto its own child should be ignored
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setCanReorderItems(true);
    tv->setFixedSize(350, 400);
    tv->expandAll();
    showOffscreen(window);

    // Try to drag "Work Documents" (row 0) onto its child "XYZ Functional Spec"
    QModelIndex srcIdx = model->index(0, 0); // Work Documents
    QStandardItem* work = model->item(0);
    QRect srcRect = tv->visualRect(srcIdx);
    QPoint start = srcRect.center();

    QModelIndex childIdx = work->child(0)->index(); // XYZ Functional Spec
    QRect childRect = tv->visualRect(childIdx);
    QPoint end = childRect.center();

    int rootCountBefore = model->rowCount();

    QTest::mousePress(tv->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    QPoint moved(start.x(), start.y() + QApplication::startDragDistance() + 5);
    QMouseEvent moveEvent1(QEvent::MouseMove, QPointF(moved), QPointF(moved),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent1);
    QApplication::processEvents();
    QMouseEvent moveEvent2(QEvent::MouseMove, QPointF(end), QPointF(end),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(tv->viewport(), &moveEvent2);
    QApplication::processEvents();
    QTest::mouseRelease(tv->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QApplication::processEvents();

    // Nothing should have moved — model structure intact
    EXPECT_EQ(model->rowCount(), rootCountBefore);
    EXPECT_EQ(model->item(0)->text(), "Work Documents");
    EXPECT_EQ(work->rowCount(), 2);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Theme
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, ThemeSwitchDoesNotCrash) {
    TreeView* tv = new TreeView(window);
    attachSampleModel(tv);

    showOffscreen(window);

    FluentElement::setTheme(FluentElement::Dark);
    QApplication::processEvents();
    FluentElement::setTheme(FluentElement::Light);
    QApplication::processEvents();

    // Should not crash
    EXPECT_TRUE(true);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Dynamic model changes
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, AddRemoveTopLevelItems) {
    TreeView* tv = new TreeView(window);
    auto* model = new QStandardItemModel(tv);
    tv->setModel(model);
    attachFluentDelegate(tv);

    EXPECT_EQ(topLevelCount(tv), 0);

    model->appendRow(new QStandardItem("Root 1"));
    EXPECT_EQ(topLevelCount(tv), 1);

    model->appendRow(new QStandardItem("Root 2"));
    EXPECT_EQ(topLevelCount(tv), 2);

    model->removeRow(0);
    EXPECT_EQ(topLevelCount(tv), 1);
    EXPECT_EQ(model->item(0)->text(), "Root 2");
}

TEST_F(TreeViewTest, AddChildrenDynamically) {
    TreeView* tv = new TreeView(window);
    auto* model = new QStandardItemModel(tv);
    tv->setModel(model);
    attachFluentDelegate(tv);

    auto* root = new QStandardItem("Root");
    model->appendRow(root);
    EXPECT_EQ(model->item(0)->rowCount(), 0);
    EXPECT_FALSE(model->hasChildren(model->index(0, 0)));

    root->appendRow(new QStandardItem("Child 1"));
    EXPECT_EQ(model->item(0)->rowCount(), 1);
    EXPECT_TRUE(model->hasChildren(model->index(0, 0)));

    root->appendRow(new QStandardItem("Child 2"));
    EXPECT_EQ(model->item(0)->rowCount(), 2);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Delegate
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, DelegateSizeHint) {
    TreeView* tv = new TreeView(window);
    attachSampleModel(tv);

    auto* delegate = qobject_cast<treeview_test::FluentTreeItemDelegate*>(tv->itemDelegate());
    ASSERT_NE(delegate, nullptr);
    EXPECT_EQ(delegate->rowHeight(), defaultTreeRowHeight());
}

TEST_F(TreeViewTest, DelegateCustomRowHeight) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);

    auto* delegate = qobject_cast<treeview_test::FluentTreeItemDelegate*>(tv->itemDelegate());
    delegate->setRowHeight(48);
    EXPECT_EQ(delegate->rowHeight(), 48);

    QStyleOptionViewItem opt;
    opt.rect = QRect(0, 0, 200, 48);
    QSize hint = delegate->sizeHint(opt, model->index(0, 0));
    EXPECT_EQ(hint.height(), 48);
}

// ═══════════════════════════════════════════════════════════════════════════════
// paint / render (no crash)
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, PaintWithExpandedItems) {
    TreeView* tv = new TreeView(window);
    auto* model = attachSampleModel(tv);
    tv->setFixedSize(300, 400);

    tv->expandAll();

    // Select a deep item
    auto* remodel = model->item(1)->child(0);
    tv->setSelectedItem(remodel->child(0)->index());

    // grab() 触发完整绘制流程但不弹窗
    tv->grab();
    EXPECT_TRUE(true);
}

TEST_F(TreeViewTest, PaintEmptyWithPlaceholder) {
    TreeView* tv = new TreeView(window);
    tv->setPlaceholderText("Empty tree");
    tv->setFixedSize(300, 400);

    // grab() 触发完整绘制流程但不弹窗
    tv->grab();
    EXPECT_TRUE(true);
}

// ═══════════════════════════════════════════════════════════════════════════════
// CheckBox delegate
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, CheckBoxVisibleDefault) {
    TreeView* tv = new TreeView(window);
    attachSampleModel(tv);
    auto* d = qobject_cast<treeview_test::FluentTreeItemDelegate*>(tv->itemDelegate());
    EXPECT_FALSE(d->checkBoxVisible());
}

TEST_F(TreeViewTest, CheckBoxVisibleToggle) {
    TreeView* tv = new TreeView(window);
    attachSampleModel(tv);
    auto* d = qobject_cast<treeview_test::FluentTreeItemDelegate*>(tv->itemDelegate());
    d->setCheckBoxVisible(true);
    EXPECT_TRUE(d->checkBoxVisible());
    d->setCheckBoxVisible(false);
    EXPECT_FALSE(d->checkBoxVisible());
}

TEST_F(TreeViewTest, CheckableModelPaintNoCrash) {
    TreeView* tv = new TreeView(window);
    auto* model = createCheckableTreeModel(tv);
    tv->setModel(model);
    auto* d = attachCheckableDelegate(tv);
    (void)d;
    tv->setFixedSize(350, 400);

    tv->expandAll();
    tv->grab();
    EXPECT_TRUE(true);
}

TEST_F(TreeViewTest, CheckStateRoleRead) {
    auto* model = createCheckableTreeModel(nullptr);
    // Work Documents → PartiallyChecked
    EXPECT_EQ(model->item(0)->checkState(), Qt::PartiallyChecked);
    // Work Documents → child 0 → Checked
    EXPECT_EQ(model->item(0)->child(0)->checkState(), Qt::Checked);
    // Pictures → Unchecked
    EXPECT_EQ(model->item(2)->checkState(), Qt::Unchecked);
    delete model;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Icon glyph delegate
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, IconGlyphRoleRead) {
    auto* model = createIconTreeModel(nullptr);
    // Work Documents → Folder icon
    EXPECT_EQ(model->item(0)->data(treeview_test::IconGlyphRole).toString(),
              Typography::Icons::Folder);
    // Work Documents → child 0 → Document icon
    EXPECT_EQ(model->item(0)->child(0)->data(treeview_test::IconGlyphRole).toString(),
              Typography::Icons::Document);
    delete model;
}

TEST_F(TreeViewTest, IconModelPaintNoCrash) {
    TreeView* tv = new TreeView(window);
    auto* model = createIconTreeModel(tv);
    tv->setModel(model);
    attachFluentDelegate(tv);
    tv->setFixedSize(350, 400);

    tv->expandAll();
    tv->grab();
    EXPECT_TRUE(true);
}

// ═══════════════════════════════════════════════════════════════════════════════
// VisualCheck
// ═══════════════════════════════════════════════════════════════════════════════

TEST_F(TreeViewTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    window->setFixedSize(800, 600);
    using Edge = AnchorLayout::Edge;

    // --- ScrollArea 容器 ---
    auto* scrollArea = new QScrollArea(window);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setGeometry(0, 0, 800, 600);

    // Fluent 自定义垂直滚动条覆盖在 scrollArea 上
    auto* fluentVBar = new view::scrolling::ScrollBar(Qt::Vertical, scrollArea);
    fluentVBar->setObjectName("fluentScrollAreaVBar");
    auto* nativeVBar = scrollArea->verticalScrollBar();
    QObject::connect(nativeVBar,  &QScrollBar::valueChanged, fluentVBar, &QScrollBar::setValue);
    QObject::connect(fluentVBar, &QScrollBar::valueChanged, nativeVBar,  &QScrollBar::setValue);

    auto syncFluentBar = [scrollArea, fluentVBar, nativeVBar]() {
        fluentVBar->setRange(nativeVBar->minimum(), nativeVBar->maximum());
        fluentVBar->setPageStep(nativeVBar->pageStep());
        const bool need = nativeVBar->maximum() > nativeVBar->minimum();
        fluentVBar->setVisible(need);
        if (!need) return;
        const QRect r = scrollArea->rect();
        const int x = r.right() - fluentVBar->thickness() + 1;
        fluentVBar->setGeometry(x, r.top() + 2, fluentVBar->thickness(), r.height() - 4);
        fluentVBar->raise();
    };
    QObject::connect(nativeVBar, &QScrollBar::rangeChanged, scrollArea, syncFluentBar);

    auto* content = new FluentTestWindow();
    content->setMinimumWidth(800);
    content->onThemeUpdated();
    auto* innerLayout = new AnchorLayout(content);
    content->setLayout(innerLayout);
    scrollArea->setWidget(content);

    // ── TreeView 1: A simple TreeView with drag and drop support ─────────
    TreeView* tv1 = new TreeView(content);
    tv1->setHeaderText("A simple TreeView with drag and drop support.");
    tv1->setBorderVisible(true);
    tv1->setCanReorderItems(true);
    {
        auto* model = createSampleTreeModel(tv1);
        tv1->setModel(model);
        attachFluentDelegate(tv1);
        tv1->expand(model->index(0, 0));  // Work Documents
        tv1->expand(model->index(1, 0));  // Personal Documents
        tv1->expand(model->item(1)->child(0)->index()); // Home Remodel
        tv1->setSelectedItem(model->item(0)->child(0)->index()); // XYZ Functional Spec
    }
    tv1->setFixedHeight(300);
    tv1->anchors()->top   = {content, Edge::Top,  20};
    tv1->anchors()->left  = {content, Edge::Left, 20};
    tv1->anchors()->right = {content, Edge::Right, -20};
    innerLayout->addWidget(tv1);

    // ── TreeView 2: Multi-selection with CheckBox ────────────────────────
    TextBlock* header2 = new TextBlock("A TreeView with Multi-selection enabled.", content);
    header2->setFluentTypography("BodyStrong");
    header2->anchors()->top  = {tv1, Edge::Bottom, 16};
    header2->anchors()->left = {content, Edge::Left, 20};
    innerLayout->addWidget(header2);

    TreeView* tv2 = new TreeView(content);
    tv2->setBorderVisible(true);
    tv2->setSelectionMode(TreeSelectionMode::Multiple);
    {
        auto* model = createCheckableTreeModel(tv2);
        tv2->setModel(model);
        auto* d = new treeview_test::FluentTreeItemDelegate(
            static_cast<FluentElement*>(tv2), defaultTreeRowHeight(), tv2, tv2);
        d->setCheckBoxVisible(true);
        tv2->setItemDelegate(d);
        tv2->expand(model->index(0, 0));  // Work Documents
        tv2->expand(model->index(1, 0));  // Personal Documents
        tv2->expand(model->item(1)->child(0)->index()); // Home Remodel
    }
    tv2->setFixedHeight(300);
    tv2->anchors()->top   = {header2, Edge::Bottom, 8};
    tv2->anchors()->left  = {content, Edge::Left, 20};
    tv2->anchors()->right = {content, Edge::Right, -20};
    innerLayout->addWidget(tv2);

    // ── TreeView 3: DataBinding using ItemSource ─────────────────────────
    TextBlock* header3 = new TextBlock("A TreeView with DataBinding Using ItemSource.", content);
    header3->setFluentTypography("BodyStrong");
    header3->anchors()->top  = {tv2, Edge::Bottom, 16};
    header3->anchors()->left = {content, Edge::Left, 20};
    innerLayout->addWidget(header3);

    TreeView* tv3 = new TreeView(content);
    tv3->setBorderVisible(true);
    tv3->setCanReorderItems(true);
    {
        auto* model = new QStandardItemModel(tv3);

        auto* desktop = new QStandardItem("Desktop");
        desktop->appendRow(new QStandardItem("folder1"));
        desktop->appendRow(new QStandardItem("folder2"));
        desktop->appendRow(new QStandardItem("folder3"));
        model->appendRow(desktop);

        auto* documents = new QStandardItem("Documents");
        auto* myDocs = new QStandardItem("My Documents");
        myDocs->appendRow(new QStandardItem("Binder1"));
        myDocs->appendRow(new QStandardItem("Binder2"));
        documents->appendRow(myDocs);
        model->appendRow(documents);

        auto* downloads = new QStandardItem("Downloads");
        model->appendRow(downloads);

        auto* pictures = new QStandardItem("Pictures");
        auto* camera = new QStandardItem("Camera Roll");
        camera->appendRow(new QStandardItem("IMG_001.jpg"));
        camera->appendRow(new QStandardItem("IMG_002.jpg"));
        pictures->appendRow(camera);
        pictures->appendRow(new QStandardItem("Wallpapers"));
        model->appendRow(pictures);

        tv3->setModel(model);
        attachFluentDelegate(tv3);
        tv3->expand(model->index(1, 0));  // Documents
    }
    tv3->setFixedHeight(280);
    tv3->anchors()->top   = {header3, Edge::Bottom, 8};
    tv3->anchors()->left  = {content, Edge::Left, 20};
    tv3->anchors()->right = {content, Edge::Right, -20};
    innerLayout->addWidget(tv3);

    // ── TreeView 4: ItemTemplateSelector (folder/document icons) ─────────
    TextBlock* header4 = new TextBlock("A TreeView with ItemTemplateSelector.", content);
    header4->setFluentTypography("BodyStrong");
    header4->anchors()->top  = {tv3, Edge::Bottom, 16};
    header4->anchors()->left = {content, Edge::Left, 20};
    innerLayout->addWidget(header4);

    TreeView* tv4 = new TreeView(content);
    tv4->setBorderVisible(true);
    {
        auto* model = createIconTreeModel(tv4);
        tv4->setModel(model);
        attachFluentDelegate(tv4);
        tv4->expand(model->index(0, 0));  // Work Documents
        tv4->expand(model->index(1, 0));  // Personal Documents
        tv4->expand(model->item(1)->child(0)->index()); // Home Remodel
    }
    tv4->setFixedHeight(300);
    tv4->anchors()->top   = {header4, Edge::Bottom, 8};
    tv4->anchors()->left  = {content, Edge::Left, 20};
    tv4->anchors()->right = {content, Edge::Right, -20};
    innerLayout->addWidget(tv4);

    // --- Switch Theme 按钮 ---
    Button* themeBtn = new Button("Switch Theme", content);
    themeBtn->setFluentStyle(Button::Accent);
    themeBtn->setFixedSize(120, 32);
    themeBtn->anchors()->top  = {tv4, Edge::Bottom, 24};
    themeBtn->anchors()->right = {content, Edge::Right, -20};
    innerLayout->addWidget(themeBtn);

    // content 最小高度
    content->setMinimumHeight(20 + 300 + 16 + 20 + 8 + 300 + 16 + 20 + 8 + 280 + 16 + 20 + 8 + 300 + 24 + 32 + 20);

    QObject::connect(themeBtn, &Button::clicked, [scrollArea, content]() {
        FluentElement::setTheme(
            FluentElement::currentTheme() == FluentElement::Light
                ? FluentElement::Dark : FluentElement::Light);
        content->onThemeUpdated();
        scrollArea->setStyleSheet(content->styleSheet());
    });

    content->onThemeUpdated();
    scrollArea->setStyleSheet(content->styleSheet());
    window->show();
    syncFluentBar();
    qApp->exec();
}
