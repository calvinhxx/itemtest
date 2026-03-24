#include <gtest/gtest.h>
#include <QAbstractItemView>
#include <QApplication>
#include <QFontDatabase>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMetaEnum>
#include <QStandardItemModel>
#include <QStringListModel>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QEnterEvent>
#else
#include <QEvent>
#endif
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

#include "FluentListItemDelegate.h"
#include "view/collections/ListView.h"
#include "view/textfields/TextBlock.h"
#include "view/basicinput/Button.h"
#include "view/QMLPlus.h"
#include "common/Spacing.h"
#include "common/Typography.h"

using namespace view::collections;
using namespace view::textfields;
using namespace view::basicinput;
using namespace view;

namespace {

int defaultListRowHeight() {
    return Spacing::ControlHeight::Standard + Spacing::Gap::Tight;
}

/** 业务组装：为 ListView 挂上 Fluent 行高代理（主题来自 ListView 的 FluentElement）。 */
void attachFluentDelegate(ListView* lv, int rowHeight = defaultListRowHeight()) {
    lv->setItemDelegate(new listview_test::FluentListItemDelegate(
        static_cast<FluentElement*>(lv), rowHeight, lv, lv));
    lv->setUniformItemSizes(true);
}

/** 创建 QStringListModel，setModel + attachFluentDelegate。 */
QStringListModel* attachStringListModel(ListView* lv, const QStringList& rows = {}) {
    auto* m = new QStringListModel(rows, lv);
    lv->setModel(m);
    attachFluentDelegate(lv);
    return m;
}

int itemCount(ListView* lv) {
    const auto* m = lv->model();
    return m ? m->rowCount() : 0;
}

QString itemText(ListView* lv, int index) {
    const auto* m = lv->model();
    if (!m || index < 0 || index >= m->rowCount()) return {};
    return m->index(index, 0).data(Qt::DisplayRole).toString();
}

void addItem(ListView* lv, const QString& text) {
    auto* slm = qobject_cast<QStringListModel*>(lv->model());
    ASSERT_NE(slm, nullptr);
    QStringList list = slm->stringList();
    list.append(text);
    slm->setStringList(list);
}

void addItems(ListView* lv, const QStringList& texts) {
    auto* slm = qobject_cast<QStringListModel*>(lv->model());
    ASSERT_NE(slm, nullptr);
    QStringList list = slm->stringList();
    list.append(texts);
    slm->setStringList(list);
}

void insertItem(ListView* lv, int index, const QString& text) {
    auto* slm = qobject_cast<QStringListModel*>(lv->model());
    ASSERT_NE(slm, nullptr);
    QStringList list = slm->stringList();
    ASSERT_GE(index, 0);
    ASSERT_LE(index, list.size());
    list.insert(index, text);
    slm->setStringList(list);
}

void removeItem(ListView* lv, int index) {
    auto* slm = qobject_cast<QStringListModel*>(lv->model());
    ASSERT_NE(slm, nullptr);
    QStringList list = slm->stringList();
    ASSERT_GE(index, 0);
    ASSERT_LT(index, list.size());
    list.removeAt(index);
    slm->setStringList(list);
}

void clearItems(ListView* lv) {
    auto* slm = qobject_cast<QStringListModel*>(lv->model());
    ASSERT_NE(slm, nullptr);
    slm->setStringList({});
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

class ListViewTest : public ::testing::Test {
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
        window->setWindowTitle("Fluent ListView Test");
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

// ── 业务层：QStringListModel + Fluent delegate + 数据操作 ─────────────────────

TEST_F(ListViewTest, AddAndRemoveItems) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv);

    addItem(lv, "Apple");
    addItem(lv, "Banana");
    addItem(lv, "Cherry");
    EXPECT_EQ(itemCount(lv), 3);
    EXPECT_EQ(itemText(lv, 0), "Apple");
    EXPECT_EQ(itemText(lv, 1), "Banana");
    EXPECT_EQ(itemText(lv, 2), "Cherry");

    removeItem(lv, 1);
    EXPECT_EQ(itemCount(lv), 2);
    EXPECT_EQ(itemText(lv, 0), "Apple");
    EXPECT_EQ(itemText(lv, 1), "Cherry");

    clearItems(lv);
    EXPECT_EQ(itemCount(lv), 0);
}

TEST_F(ListViewTest, AddItemsBatch) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv);
    addItems(lv, {"A", "B", "C", "D"});
    EXPECT_EQ(itemCount(lv), 4);
    EXPECT_EQ(itemText(lv, 3), "D");
}

TEST_F(ListViewTest, InsertItem) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv);
    addItems(lv, {"A", "C"});
    insertItem(lv, 1, "B");
    EXPECT_EQ(itemCount(lv), 3);
    EXPECT_EQ(itemText(lv, 0), "A");
    EXPECT_EQ(itemText(lv, 1), "B");
    EXPECT_EQ(itemText(lv, 2), "C");
}

TEST_F(ListViewTest, ItemTextOutOfRange) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv);
    addItem(lv, "Only");
    EXPECT_EQ(itemText(lv, -1), "");
    EXPECT_EQ(itemText(lv, 1), "");
}

// ── 视图：选择模式 ────────────────────────────────────────────────────────────

TEST_F(ListViewTest, DefaultSelectionMode) {
    ListView* lv = new ListView(window);
    EXPECT_EQ(lv->selectionMode(), ListSelectionMode::Single);
}

TEST_F(ListViewTest, DefaultEditTriggersDisabled) {
    ListView* lv = new ListView(window);
    EXPECT_EQ(lv->editTriggers(), QAbstractItemView::NoEditTriggers);
}

TEST_F(ListViewTest, ListSelectionModeRegisteredInMetaObject) {
    QMetaEnum me = QMetaEnum::fromType<ListSelectionMode>();
    ASSERT_TRUE(me.isValid());
    EXPECT_STREQ(me.key(0), "None");
    EXPECT_STREQ(me.key(1), "Single");
    EXPECT_STREQ(me.key(2), "Multiple");
    EXPECT_STREQ(me.key(3), "Extended");
}

TEST_F(ListViewTest, SelectionModeNone) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv, {"A", "B", "C"});
    lv->setSelectionMode(ListSelectionMode::None);
    EXPECT_EQ(lv->selectionMode(), ListSelectionMode::None);
}

TEST_F(ListViewTest, SelectionModeMultiple) {
    ListView* lv = new ListView(window);
    QSignalSpy spy(lv, SIGNAL(selectionModeChanged()));
    lv->setSelectionMode(ListSelectionMode::Multiple);
    EXPECT_EQ(lv->selectionMode(), ListSelectionMode::Multiple);
    EXPECT_EQ(spy.count(), 1);

    lv->setSelectionMode(ListSelectionMode::Multiple);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ListViewTest, SelectionModeExtended) {
    ListView* lv = new ListView(window);
    lv->setSelectionMode(ListSelectionMode::Extended);
    EXPECT_EQ(lv->selectionMode(), ListSelectionMode::Extended);
}

// ── 选中 API（依赖已 setModel）────────────────────────────────────────────────

TEST_F(ListViewTest, SingleSelection) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv, {"A", "B", "C"});

    EXPECT_EQ(lv->selectedIndex(), -1);

    lv->setSelectedIndex(1);
    EXPECT_EQ(lv->selectedIndex(), 1);

    lv->setSelectedIndex(-1);
    EXPECT_EQ(lv->selectedIndex(), -1);
}

TEST_F(ListViewTest, SelectedIndexOutOfRange) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv, {"A", "B"});
    lv->setSelectedIndex(1);
    EXPECT_EQ(lv->selectedIndex(), 1);

    lv->setSelectedIndex(99);
    EXPECT_EQ(lv->selectedIndex(), -1);
}

TEST_F(ListViewTest, SelectedRowsSortedAscending) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv, {"A", "B", "C", "D"});
    lv->setSelectionMode(ListSelectionMode::Multiple);

    const QModelIndex i0 = lv->model()->index(0, 0);
    const QModelIndex i2 = lv->model()->index(2, 0);
    lv->selectionModel()->select(i2, QItemSelectionModel::Select);
    lv->selectionModel()->select(i0, QItemSelectionModel::Select);

    QList<int> rows = lv->selectedRows();
    ASSERT_EQ(rows.size(), 2);
    EXPECT_EQ(rows.at(0), 0);
    EXPECT_EQ(rows.at(1), 2);
}

TEST_F(ListViewTest, ViewportHoveredSignal) {
    window->setAttribute(Qt::WA_DontShowOnScreen, true);
    ListView* lv = new ListView(window);
    lv->setGeometry(10, 10, 200, 200);

    EXPECT_FALSE(lv->viewportHovered());

    QSignalSpy spy(lv, &ListView::viewportHoveredChanged);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QEnterEvent enterEv(QPointF(5, 5), QPointF(5, 5), QPointF(5, 5));
#else
    QEnterEvent enterEv(QPoint(5, 5), QPoint(5, 5), QPoint(5, 5));
#endif
    QApplication::sendEvent(lv, &enterEv);
    EXPECT_TRUE(lv->viewportHovered());
    EXPECT_EQ(spy.count(), 1);

    QEvent leave(QEvent::Leave);
    QApplication::sendEvent(lv, &leave);
    EXPECT_FALSE(lv->viewportHovered());
    EXPECT_EQ(spy.count(), 2);
}

// ── 视图默认属性 / 业务 delegate 行高 ──────────────────────────────────────────

TEST_F(ListViewTest, DefaultFontRole) {
    ListView* lv = new ListView(window);
    EXPECT_EQ(lv->fontRole(), Typography::FontRole::Body);
}

TEST_F(ListViewTest, FluentDelegateDefaultRowHeight) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv);
    auto* del = qobject_cast<listview_test::FluentListItemDelegate*>(lv->itemDelegate());
    ASSERT_NE(del, nullptr);
    EXPECT_EQ(del->rowHeight(), defaultListRowHeight());
}

TEST_F(ListViewTest, FluentDelegateSetRowHeight) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv);
    auto* del = qobject_cast<listview_test::FluentListItemDelegate*>(lv->itemDelegate());
    ASSERT_NE(del, nullptr);
    del->setRowHeight(48);
    lv->doItemsLayout();
    EXPECT_EQ(del->rowHeight(), 48);
}

TEST_F(ListViewTest, SetFontRole) {
    ListView* lv = new ListView(window);
    QSignalSpy spy(lv, SIGNAL(fontRoleChanged()));
    lv->setFontRole(Typography::FontRole::Subtitle);
    EXPECT_EQ(lv->fontRole(), Typography::FontRole::Subtitle);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ListViewTest, ItemClickedSignal) {
    ListView* lv = new ListView(window);
    attachStringListModel(lv, {"A", "B", "C"});
    QSignalSpy spy(lv, SIGNAL(itemClicked(int)));

    QModelIndex idx = lv->model()->index(0, 0);
    emit lv->clicked(idx);
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);
}

TEST_F(ListViewTest, FluentScrollBarExists) {
    ListView* lv = new ListView(window);
    EXPECT_NE(lv->verticalFluentScrollBar(), nullptr);
}

TEST_F(ListViewTest, CustomModelWithFluentDelegate) {
    ListView* lv = new ListView(window);
    auto* stdModel = new QStandardItemModel(lv);
    stdModel->appendRow(new QStandardItem("Row0"));
    stdModel->appendRow(new QStandardItem("Row1"));
    lv->setModel(stdModel);
    attachFluentDelegate(lv);

    EXPECT_EQ(lv->model()->rowCount(), 2);
    EXPECT_EQ(lv->model()->index(0, 0).data(Qt::DisplayRole).toString(), "Row0");

    lv->setSelectedIndex(1);
    EXPECT_EQ(lv->selectedIndex(), 1);
}

TEST_F(ListViewTest, ViewDoesNotProvideModelByDefault) {
    ListView* lv = new ListView(window);
    EXPECT_EQ(lv->model(), nullptr);
    // Qt 会为 QAbstractItemView 提供默认 itemDelegate()，故不断言 delegate 为空。
}

// ── 新增属性: borderVisible / headerText / placeholderText ────────────────────

TEST_F(ListViewTest, DefaultBorderVisible) {
    ListView* lv = new ListView(window);
    EXPECT_TRUE(lv->borderVisible());
}

TEST_F(ListViewTest, SetBorderVisible) {
    ListView* lv = new ListView(window);
    QSignalSpy spy(lv, &ListView::borderVisibleChanged);
    lv->setBorderVisible(false);
    EXPECT_FALSE(lv->borderVisible());
    EXPECT_EQ(spy.count(), 1);

    // 重复设置不触发信号
    lv->setBorderVisible(false);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ListViewTest, DefaultHeaderText) {
    ListView* lv = new ListView(window);
    EXPECT_TRUE(lv->headerText().isEmpty());
}

TEST_F(ListViewTest, SetHeaderText) {
    ListView* lv = new ListView(window);
    QSignalSpy spy(lv, &ListView::headerTextChanged);
    lv->setHeaderText("My Header");
    EXPECT_EQ(lv->headerText(), "My Header");
    EXPECT_EQ(spy.count(), 1);

    // 重复设置不触发信号
    lv->setHeaderText("My Header");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ListViewTest, DefaultPlaceholderText) {
    ListView* lv = new ListView(window);
    EXPECT_TRUE(lv->placeholderText().isEmpty());
}

TEST_F(ListViewTest, SetPlaceholderText) {
    ListView* lv = new ListView(window);
    QSignalSpy spy(lv, &ListView::placeholderTextChanged);
    lv->setPlaceholderText("No items");
    EXPECT_EQ(lv->placeholderText(), "No items");
    EXPECT_EQ(spy.count(), 1);

    lv->setPlaceholderText("No items");
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ListViewTest, HeaderVisibleWhenTextSet) {
    window->setAttribute(Qt::WA_DontShowOnScreen, true);
    ListView* lv = new ListView(window);
    lv->setGeometry(10, 10, 300, 200);
    lv->setHeaderText("Header");
    window->show();
    QTest::qWait(50);
    auto* headerLabel = lv->findChild<QLabel*>();
    ASSERT_NE(headerLabel, nullptr);
    EXPECT_TRUE(headerLabel->isVisible());
    EXPECT_EQ(headerLabel->text(), "Header");
}

TEST_F(ListViewTest, HeaderHiddenWhenTextEmpty) {
    ListView* lv = new ListView(window);
    lv->setHeaderText("Header");
    lv->setHeaderText("");
    auto* headerLabel = lv->findChild<QLabel*>();
    ASSERT_NE(headerLabel, nullptr);
    EXPECT_FALSE(headerLabel->isVisible());
}

// ── 可视化测试（业务组装与上面一致）───────────────────────────────────────────

TEST_F(ListViewTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    window->setFixedSize(600, 700);
    using Edge = AnchorLayout::Edge;

    // --- ListView 1: 带 header + border 的单选列表 ---
    ListView* lv1 = new ListView(window);
    lv1->setHeaderText("Fruits (Single Selection)");
    lv1->setBorderVisible(true);
    attachStringListModel(lv1, {"Apricot", "Banana", "Cherry", "Date", "Elderberry",
                                 "Fig", "Grape", "Honeydew"});
    lv1->setSelectedIndex(2);
    lv1->setFixedHeight(250);
    lv1->anchors()->top   = {window, Edge::Top,  20};
    lv1->anchors()->left  = {window, Edge::Left, 20};
    lv1->anchors()->right = {window, Edge::Right, -20};
    layout->addWidget(lv1);

    // --- ListView 2: 多选模式，无 border ---
    TextBlock* header2 = new TextBlock("Multiple Selection (no border):", window);
    header2->anchors()->top  = {lv1, Edge::Bottom, 16};
    header2->anchors()->left = {window, Edge::Left, 20};
    layout->addWidget(header2);

    ListView* lv2 = new ListView(window);
    lv2->setSelectionMode(ListSelectionMode::Multiple);
    lv2->setBorderVisible(false);
    attachStringListModel(lv2, {"Item A", "Item B", "Item C", "Item D"});
    lv2->setFixedHeight(160);
    lv2->anchors()->top   = {header2, Edge::Bottom, 8};
    lv2->anchors()->left  = {window, Edge::Left, 20};
    lv2->anchors()->right = {window, Edge::Right, -20};
    layout->addWidget(lv2);

    // --- ListView 3: 空列表，显示 placeholder ---
    TextBlock* header3 = new TextBlock("Empty list with placeholder:", window);
    header3->anchors()->top  = {lv2, Edge::Bottom, 16};
    header3->anchors()->left = {window, Edge::Left, 20};
    layout->addWidget(header3);

    ListView* lv3 = new ListView(window);
    lv3->setHeaderText("Empty List");
    lv3->setPlaceholderText("No items to display");
    lv3->setBorderVisible(true);
    attachStringListModel(lv3);
    lv3->setFixedHeight(100);
    lv3->anchors()->top   = {header3, Edge::Bottom, 8};
    lv3->anchors()->left  = {window, Edge::Left, 20};
    lv3->anchors()->right = {window, Edge::Right, -20};
    layout->addWidget(lv3);

    Button* themeBtn = new Button("Switch Theme", window);
    themeBtn->setFluentStyle(Button::Accent);
    themeBtn->setFixedSize(120, 32);
    themeBtn->anchors()->bottom = {window, Edge::Bottom, -20};
    themeBtn->anchors()->right  = {window, Edge::Right,  -20};
    layout->addWidget(themeBtn);

    QObject::connect(themeBtn, &Button::clicked, []() {
        FluentElement::setTheme(
            FluentElement::currentTheme() == FluentElement::Light
                ? FluentElement::Dark : FluentElement::Light);
    });

    window->show();
    qApp->exec();
}
