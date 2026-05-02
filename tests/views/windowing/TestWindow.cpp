#include <gtest/gtest.h>

#include <QApplication>
#include <QColor>
#include <QDirIterator>
#include <QFile>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QPainter>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QSignalSpy>
#include <QTest>
#include <QVBoxLayout>

#include "compatibility/WindowChromeCompat.h"
#include "design/Breakpoints.h"
#include "design/Typography.h"
#include "view/FluentElement.h"
#include "view/QMLPlus.h"
#include "view/basicinput/Button.h"
#include "view/textfields/AutoSuggestBox.h"
#include "view/textfields/Label.h"
#include "view/windowing/TitleBar.h"
#include "view/windowing/Window.h"

using compatibility::WindowChromeCompat;
using view::AnchorLayout;
using view::basicinput::Button;
using view::textfields::AutoSuggestBox;
using view::textfields::Label;
using view::windowing::TitleBar;
using view::windowing::Window;

namespace {

using Edge = AnchorLayout::Edge;

constexpr int TitleBarIconSize = 24;
constexpr int TitleBarAppIconSize = 14;
constexpr int TitleBarAvatarSize = 24;
constexpr int TitleBarSearchWidth = 220;
constexpr int TitleBarSearchHeight = 24;

struct WindowVisualLauncher {
    QWidget* window = nullptr;
    Button* showButton = nullptr;
};

Button* createTitleBarIconButton(const QString& glyph, QWidget* parent) {
    auto* button = new Button(parent);
    button->setFluentStyle(Button::Subtle);
    button->setFluentLayout(Button::IconOnly);
    button->setFluentSize(Button::Small);
    button->setIconGlyph(glyph, 12);
    button->setFixedSize(TitleBarIconSize, TitleBarIconSize);
    return button;
}

QIcon createWindowAppIcon() {
    const qreal dpr = qApp && qApp->primaryScreen()
                          ? qApp->primaryScreen()->devicePixelRatio()
                          : qreal(1);

    QPixmap pixmap(QSize(qRound(TitleBarAppIconSize * dpr), qRound(TitleBarAppIconSize * dpr)));
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#0078D4"));
    painter.drawRoundedRect(QRectF(0.75, 0.75, TitleBarAppIconSize - 1.5, TitleBarAppIconSize - 1.5), 2.5, 2.5);

    QFont iconFont(Typography::FontFamily::SegoeFluentIcons);
    iconFont.setPixelSize(8);
    painter.setFont(iconFont);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(0, 0, TitleBarAppIconSize, TitleBarAppIconSize),
                     Qt::AlignCenter,
                     Typography::Icons::AppIconDefault);

    return QIcon(pixmap);
}

void anchorFromLeft(AnchorLayout* layout,
                    QWidget* widget,
                    TitleBar* titleBar,
                    QWidget* target,
                    Edge edge,
                    int offset) {
    AnchorLayout::Anchors anchors;
    anchors.left = {target, edge, offset};
    anchors.verticalCenter = {titleBar, Edge::VCenter, 0};
    layout->addAnchoredWidget(widget, anchors);
}

void anchorFromRight(AnchorLayout* layout,
                     QWidget* widget,
                     TitleBar* titleBar,
                     QWidget* target,
                     Edge edge,
                     int offset) {
    AnchorLayout::Anchors anchors;
    anchors.right = {target, edge, offset};
    anchors.verticalCenter = {titleBar, Edge::VCenter, 0};
    layout->addAnchoredWidget(widget, anchors);
}

Label* createTitleBarTitle(TitleBar* titleBar) {
    auto* title = new Label("Fluent Window", titleBar);
    title->setObjectName(QStringLiteral("titleBarWindowTitle"));
    title->setFluentTypography(Typography::FontRole::Caption);
    title->setStyleSheet("#titleBarWindowTitle { font-weight: 600; }");
    title->setFixedHeight(20);
    return title;
}

AutoSuggestBox* createTitleBarSearch(TitleBar* titleBar) {
    auto* search = new AutoSuggestBox(titleBar);
    search->setPlaceholderText("Search...");
    search->setSuggestions(QStringList{
        QStringLiteral("TitleBar"),
        QStringLiteral("WindowChromeCompat"),
        QStringLiteral("AutoSuggestBox")
    });
    search->setQueryIconVisible(false);
    search->setFontRole(Typography::FontRole::Caption);
    search->setSuggestionFontRole(Typography::FontRole::Caption);
    search->setSuggestionItemHeight(24);
    search->setInputHeight(TitleBarSearchHeight);
    search->setQueryButtonSize(16);
    search->setClearButtonSize(16);
    search->setFixedSize(TitleBarSearchWidth, TitleBarSearchHeight);
    return search;
}

Label* createTitleBarAvatar(TitleBar* titleBar) {
    auto* avatar = new Label("JD", titleBar);
    avatar->setObjectName(QStringLiteral("titleBarAvatar"));
    avatar->setAlignment(Qt::AlignCenter);
    avatar->setFixedSize(TitleBarAvatarSize, TitleBarAvatarSize);
    avatar->setStyleSheet("#titleBarAvatar { background: #E1DFDD; color: #323130; border-radius: 12px; font-weight: 600; font-size: 11px; }");
    return avatar;
}

void addWindowsCaptionButtons(Window* window, AnchorLayout* layout, TitleBar* titleBar) {
    if (!WindowChromeCompat::platformPrefersCustomWindowChrome())
        return;

    auto* close = createTitleBarIconButton(Typography::Icons::ChromeClose, titleBar);
    auto* maximize = createTitleBarIconButton(Typography::Icons::ChromeMaximize, titleBar);
    auto* minimize = createTitleBarIconButton(Typography::Icons::ChromeMinimize, titleBar);
    close->setFixedSize(40, titleBar->titleBarHeight());
    maximize->setFixedSize(40, titleBar->titleBarHeight());
    minimize->setFixedSize(40, titleBar->titleBarHeight());

    QObject::connect(minimize, &QPushButton::clicked, window, &Window::minimizeWindow);
    QObject::connect(maximize, &QPushButton::clicked, window, &Window::toggleMaximizeRestore);
    QObject::connect(close, &QPushButton::clicked, window, &Window::closeWindow);

    anchorFromRight(layout, close, titleBar, titleBar, Edge::Right, 0);
    anchorFromRight(layout, maximize, titleBar, close, Edge::Left, 0);
    anchorFromRight(layout, minimize, titleBar, maximize, Edge::Left, 0);
}

void createTitleBarContent(Window* window) {
    auto* titleBar = window->titleBar();
    auto* layout = qobject_cast<AnchorLayout*>(titleBar->layout());
    if (!layout)
        return;

    auto* appIcon = new Label(titleBar);
    appIcon->setFixedSize(TitleBarAppIconSize, TitleBarAppIconSize);
    appIcon->setPixmap(createWindowAppIcon().pixmap(QSize(TitleBarAppIconSize, TitleBarAppIconSize)));
    anchorFromLeft(layout, appIcon, titleBar, titleBar, Edge::Left, titleBar->systemReservedLeadingWidth() + 8);

    auto* pane = createTitleBarIconButton(Typography::Icons::GlobalNav, titleBar);
    anchorFromLeft(layout, pane, titleBar, appIcon, Edge::Right, 10);

    auto* back = createTitleBarIconButton(Typography::Icons::TitleBarBack, titleBar);
    anchorFromLeft(layout, back, titleBar, pane, Edge::Right, 2);

    auto* title = createTitleBarTitle(titleBar);
    anchorFromLeft(layout, title, titleBar, back, Edge::Right, 8);

    auto* search = createTitleBarSearch(titleBar);
    anchorFromLeft(layout, search, titleBar, title, Edge::Right, 12);

    auto* avatar = createTitleBarAvatar(titleBar);
    anchorFromRight(layout, avatar, titleBar, titleBar, Edge::Right, -8);

    auto* favorite = createTitleBarIconButton(Typography::Icons::FavoriteStar, titleBar);
    anchorFromRight(layout, favorite, titleBar, avatar, Edge::Left, -6);

    auto* share = createTitleBarIconButton(Typography::Icons::Share, titleBar);
    anchorFromRight(layout, share, titleBar, favorite, Edge::Left, -6);

    auto* link = createTitleBarIconButton(Typography::Icons::Link, titleBar);
    anchorFromRight(layout, link, titleBar, share, Edge::Left, -6);

    addWindowsCaptionButtons(window, layout, titleBar);
}

QWidget* createWindowContent() {
    auto* content = new QWidget();
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 28, 24, 24);
    contentLayout->setSpacing(16);

    auto* heading = new Label("Native macOS traffic lights + custom TitleBar content", content);
    heading->setFluentTypography(Typography::FontRole::Title);
    contentLayout->addWidget(heading);

    auto* body = new Label(
        "On macOS this Window uses a full-size native titlebar: the system traffic lights stay on the left, while back, pane toggle, search, actions, and avatar are Qt widgets in the titlebar area.",
        content);
    body->setFluentTypography(Typography::FontRole::Body);
    body->setWordWrap(true);
    contentLayout->addWidget(body);
    contentLayout->addStretch(1);

    return content;
}

WindowVisualLauncher createWindowVisualLauncher() {
    WindowVisualLauncher ui;

    auto* launcher = new QWidget();
    launcher->setAttribute(Qt::WA_DeleteOnClose);
    launcher->setWindowTitle("Window VisualCheck");
    launcher->resize(280, 120);

    auto* root = new QVBoxLayout(launcher);
    root->setContentsMargins(24, 24, 24, 24);
    root->setSpacing(0);

    auto* showButton = new Button("Show window", launcher);
    showButton->setIconGlyph(Typography::Icons::BackToWindow, Typography::FontSize::Body);
    showButton->setFluentLayout(Button::IconBefore);
    showButton->setFixedSize(150, 34);
    root->addWidget(showButton, 0, Qt::AlignCenter);

    ui.window = launcher;
    ui.showButton = showButton;
    return ui;
}

void showOrActivateVisualWindow(QPointer<Window>& appWindow, QWidget* launcher) {
    if (!appWindow) {
        appWindow = new Window();
        appWindow->setAttribute(Qt::WA_DeleteOnClose);
        appWindow->setWindowTitle("Fluent Window");
        createTitleBarContent(appWindow);
        appWindow->setContentWidget(createWindowContent());
    }

    appWindow->resize(860, 420);
    appWindow->move(launcher->geometry().center() - QPoint(430, 210));
    appWindow->show();
    appWindow->raise();
    appWindow->activateWindow();
}

} // namespace

class WindowTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char** argv = nullptr;
        if (!qApp)
            new QApplication(argc, argv);

        QApplication::setStyle("Fusion");
        Q_INIT_RESOURCE(resources);
        QFontDatabase::addApplicationFont(":/res/Segoe Fluent Icons.ttf");
        QFontDatabase::addApplicationFont(":/res/SegoeUI-VF.ttf");
    }
};

TEST_F(WindowTest, DefaultConstructionCreatesChromeAndContentHost) {
    Window window;

    ASSERT_NE(window.titleBar(), nullptr);
    ASSERT_NE(window.contentHost(), nullptr);
    EXPECT_TRUE(window.isWindow());
    EXPECT_GE(window.minimumWidth(), Breakpoints::MinWindowWidth);
    EXPECT_GE(window.minimumHeight(), Breakpoints::MinWindowHeight);
    EXPECT_EQ(window.titleBar()->titleBarHeight(), TitleBar::defaultTitleBarHeight());
    EXPECT_EQ(window.titleBar()->sizeHint().height(), window.titleBar()->titleBarHeight());
    EXPECT_EQ(window.graphicsEffect(), nullptr);
}

TEST_F(WindowTest, TitleBarHeightIsConfigurable) {
    TitleBar titleBar;
    QSignalSpy heightSpy(&titleBar, &TitleBar::titleBarHeightChanged);
    QSignalSpy chromeSpy(&titleBar, &TitleBar::chromeGeometryChanged);

    EXPECT_EQ(titleBar.titleBarHeight(), TitleBar::defaultTitleBarHeight());

    titleBar.setTitleBarHeight(40);
    EXPECT_EQ(titleBar.titleBarHeight(), 40);
    EXPECT_EQ(titleBar.height(), 40);
    EXPECT_EQ(titleBar.sizeHint().height(), 40);
    EXPECT_EQ(titleBar.minimumSizeHint().height(), 40);
    EXPECT_EQ(heightSpy.count(), 1);
    EXPECT_EQ(chromeSpy.count(), 1);

    titleBar.setTitleBarHeight(40);
    EXPECT_EQ(heightSpy.count(), 1);
    EXPECT_EQ(chromeSpy.count(), 1);

    titleBar.setTitleBarHeight(0);
    EXPECT_EQ(titleBar.titleBarHeight(), 1);
    EXPECT_EQ(titleBar.height(), 1);
    EXPECT_EQ(heightSpy.count(), 2);
    EXPECT_EQ(chromeSpy.count(), 2);
}

TEST_F(WindowTest, NativeMacModeUsesUnifiedTitleBar) {
    Window window;
    const bool customChrome = WindowChromeCompat::platformPrefersCustomWindowChrome();
    const bool nativeMacTitleBar = (WindowChromeCompat::currentPlatform() == WindowChromeCompat::Platform::MacOS);

    EXPECT_EQ(window.titleBar()->isHidden(), !(customChrome || nativeMacTitleBar));
    EXPECT_EQ(window.titleBar()->systemReservedLeadingWidth() > 0, nativeMacTitleBar);

#if defined(Q_OS_MAC) && QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    if (nativeMacTitleBar) {
        EXPECT_TRUE(window.windowFlags().testFlag(Qt::ExpandedClientAreaHint));
        EXPECT_TRUE(window.windowFlags().testFlag(Qt::NoTitleBarBackgroundHint));
        EXPECT_FALSE(window.testAttribute(Qt::WA_ContentsMarginsRespectsSafeArea));
    }
#endif
}

TEST_F(WindowTest, TopLevelShowSmoke) {
    Window window;
    window.resize(520, 560);
    window.show();
    QApplication::processEvents();

    EXPECT_TRUE(window.isWindow());
    EXPECT_TRUE(window.isVisible());

    window.close();
}

TEST_F(WindowTest, ContentWidgetInsertionAndReplacement) {
    Window window;
    auto* first = new Label("first");
    auto* second = new Label("second");

    window.setContentWidget(first);
    EXPECT_EQ(window.contentWidget(), first);
    EXPECT_EQ(first->parentWidget(), window.contentHost());
    ASSERT_NE(window.contentHost()->layout(), nullptr);
    EXPECT_EQ(window.contentHost()->layout()->count(), 1);

    window.setContentWidget(second);
    EXPECT_EQ(window.contentWidget(), second);
    EXPECT_EQ(second->parentWidget(), window.contentHost());
    EXPECT_EQ(first->parentWidget(), nullptr);
    EXPECT_EQ(window.contentHost()->layout()->count(), 1);

    delete first;
}

TEST_F(WindowTest, TitleBarHostsExternalContentAfterSystemArea) {
    TitleBar titleBar;
    titleBar.resize(720, titleBar.titleBarHeight());

    auto* anchorLayout = qobject_cast<AnchorLayout*>(titleBar.layout());
    ASSERT_NE(anchorLayout, nullptr);

    auto* title = new Label("Fluent Window", &titleBar);
    title->setFluentTypography(Typography::FontRole::Caption);
    title->setFixedSize(112, 24);
    auto* search = new AutoSuggestBox(&titleBar);
    search->setPlaceholderText("Search...");
    search->setQueryIconVisible(false);
    search->setFixedSize(160, 32);
    auto* action = new Button("Action", &titleBar);
    action->setFluentSize(Button::Small);
    action->setFixedSize(72, 28);

    using Edge = AnchorLayout::Edge;
    AnchorLayout::Anchors titleAnchors;
    titleAnchors.left = {&titleBar, Edge::Left, 86};
    titleAnchors.verticalCenter = {&titleBar, Edge::VCenter, 0};
    anchorLayout->addAnchoredWidget(title, titleAnchors);

    AnchorLayout::Anchors searchAnchors;
    searchAnchors.left = {title, Edge::Right, 12};
    searchAnchors.verticalCenter = {&titleBar, Edge::VCenter, 0};
    anchorLayout->addAnchoredWidget(search, searchAnchors);

    AnchorLayout::Anchors actionAnchors;
    actionAnchors.left = {search, Edge::Right, 8};
    actionAnchors.verticalCenter = {search, Edge::VCenter, 0};
    anchorLayout->addAnchoredWidget(action, actionAnchors);

    QSignalSpy reservedSpy(&titleBar, &TitleBar::systemReservedLeadingWidthChanged);
    titleBar.setSystemReservedLeadingWidth(78);
    titleBar.show();
    QApplication::processEvents();

    EXPECT_EQ(titleBar.systemReservedLeadingWidth(), 78);
    EXPECT_EQ(search->parentWidget(), titleBar.contentHost());
    EXPECT_GE(search->geometry().left(), titleBar.systemReservedLeadingWidth());
    EXPECT_GT(search->geometry().left(), title->geometry().right());
    EXPECT_EQ(search->geometry().center().y(), titleBar.rect().center().y());
    EXPECT_GT(action->geometry().left(), search->geometry().right());
    EXPECT_EQ(reservedSpy.count(), 1);
    EXPECT_GE(titleBar.dragExclusionRects().size(), 2);
}

TEST_F(WindowTest, TitleBarDoubleClickEmitsFromNonInteractiveArea) {
    TitleBar titleBar;
    titleBar.resize(720, titleBar.titleBarHeight());
    titleBar.show();
    QApplication::processEvents();

    QSignalSpy doubleClickSpy(&titleBar, &TitleBar::doubleClicked);
    QTest::mouseDClick(&titleBar, Qt::LeftButton, Qt::NoModifier, QPoint(320, titleBar.titleBarHeight() / 2));

    EXPECT_EQ(doubleClickSpy.count(), 1);
}

TEST_F(WindowTest, WindowTitleBarDoubleClickDoesNotCrash) {
    Window window;
    window.resize(640, 420);
    window.show();
    QApplication::processEvents();

    QTest::mouseDClick(window.titleBar(), Qt::LeftButton, Qt::NoModifier, QPoint(320, window.titleBar()->titleBarHeight() / 2));
    QApplication::processEvents();

    SUCCEED();
}

TEST_F(WindowTest, ThemeSwitchNoCrash) {
    Window window;

    FluentElement::setTheme(FluentElement::Dark);
    window.onThemeUpdated();
    FluentElement::setTheme(FluentElement::Light);
    window.onThemeUpdated();

    SUCCEED();
}

TEST_F(WindowTest, ExternalTitleBarActionsCanDriveWindowSlots) {
    Window window;
    auto* content = new QWidget();
    auto* layout = new QHBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* minimizeButton = new QPushButton("Minimize", content);
    auto* maximizeButton = new QPushButton("Maximize", content);
    auto* closeButton = new QPushButton("Close", content);
    layout->addWidget(minimizeButton);
    layout->addWidget(maximizeButton);
    layout->addWidget(closeButton);
    window.titleBar()->setContentWidget(content);

    QSignalSpy minimizeSpy(&window, &Window::minimizeRequested);
    QSignalSpy maximizeSpy(&window, &Window::maximizeRequested);
    QSignalSpy restoreSpy(&window, &Window::restoreRequested);
    QSignalSpy closeSpy(&window, &Window::closeRequested);

    QObject::connect(minimizeButton, &QPushButton::clicked, &window, &Window::minimizeWindow);
    QObject::connect(maximizeButton, &QPushButton::clicked, &window, &Window::toggleMaximizeRestore);
    QObject::connect(closeButton, &QPushButton::clicked, &window, &Window::closeWindow);

    minimizeButton->click();
    EXPECT_EQ(minimizeSpy.count(), 1);

    maximizeButton->click();
    EXPECT_EQ(maximizeSpy.count(), 1);

    maximizeButton->click();
    EXPECT_EQ(restoreSpy.count(), 1);

    closeButton->click();
    EXPECT_EQ(closeSpy.count(), 1);
}

TEST_F(WindowTest, TitleBarInteractiveChildrenCreateDragExclusions) {
    TitleBar titleBar;
    auto* content = new QWidget();
    auto* layout = new QHBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(new Label("Label", content));
    layout->addWidget(new QPushButton("Search", content));
    titleBar.setContentWidget(content);
    titleBar.resize(500, titleBar.titleBarHeight());
    titleBar.show();
    QApplication::processEvents();

    const QVector<QRect> exclusions = titleBar.dragExclusionRects();
    EXPECT_FALSE(exclusions.isEmpty());
}

TEST_F(WindowTest, WindowingSourcesDoNotContainPlatformMacrosOrNativeHeaders) {
#ifndef PROJECT_SOURCE_DIR
#error PROJECT_SOURCE_DIR must be defined for this test target
#endif

    const QString sourceDir = QString::fromUtf8(PROJECT_SOURCE_DIR) + "/src/view/windowing";
    const QStringList forbidden = {
        "Q_OS_WIN",
        "Q_OS_MAC",
        "_WIN32",
        "__APPLE__",
        "QT_VERSION_CHECK",
        "windows.h",
        "dwmapi.h",
        "Cocoa",
        "AppKit"
    };

    QDirIterator it(sourceDir, QStringList() << "*.h" << "*.cpp",
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFile file(it.next());
        ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text))
            << file.fileName().toStdString();
        const QString text = QString::fromUtf8(file.readAll());
        for (const QString& pattern : forbidden) {
            EXPECT_FALSE(text.contains(pattern))
                << pattern.toStdString() << " found in " << file.fileName().toStdString();
        }
    }
}

TEST_F(WindowTest, WindowChromeCompatClassifiesHitTestAreas) {
    compatibility::WindowChromeOptions options;
    options.titleBarRect = QRect(0, 0, 500, 48);
    options.resizeBorderWidth = 8;
    options.dragExclusionRects << QRect(360, 0, 140, 32);

    EXPECT_EQ(WindowChromeCompat::classifyHitTest(options, QSize(500, 400), QPoint(2, 2)),
              WindowChromeCompat::HitTest::TopLeft);
    EXPECT_EQ(WindowChromeCompat::classifyHitTest(options, QSize(500, 400), QPoint(20, 20)),
              WindowChromeCompat::HitTest::Caption);
    EXPECT_EQ(WindowChromeCompat::classifyHitTest(options, QSize(500, 400), QPoint(380, 16)),
              WindowChromeCompat::HitTest::Client);
    EXPECT_EQ(WindowChromeCompat::classifyHitTest(options, QSize(500, 400), QPoint(498, 200)),
              WindowChromeCompat::HitTest::Right);
}

TEST_F(WindowTest, WindowChromeCompatFallbackIsSafe) {
    QWidget widget;
    WindowChromeCompat chrome(&widget);
    compatibility::FluentNativeEventResult result = 0;

    EXPECT_FALSE(chrome.handleNativeEvent(QByteArray(), nullptr, &result));
    EXPECT_FALSE(chrome.beginSystemMove(QPoint(0, 0)));
    EXPECT_FALSE(chrome.beginSystemResize(Qt::LeftEdge, QPoint(0, 0)));
}

TEST_F(WindowTest, WindowsHitTestClassificationSkippedOffWindows) {
#ifdef Q_OS_WIN
    compatibility::WindowChromeOptions options;
    options.titleBarRect = QRect(0, 0, 640, 48);
    options.resizeBorderWidth = 8;
    options.useCustomWindowChrome = true;
    options.dragExclusionRects << QRect(502, 0, 138, 32);

    EXPECT_EQ(WindowChromeCompat::classifyHitTest(options, QSize(640, 480), QPoint(100, 20)),
              WindowChromeCompat::HitTest::Caption);
    EXPECT_EQ(WindowChromeCompat::classifyHitTest(options, QSize(640, 480), QPoint(620, 16)),
              WindowChromeCompat::HitTest::Client);
#else
    GTEST_SKIP() << "Windows hit-test native classification is only required on Windows";
#endif
}

TEST_F(WindowTest, VisualCheck) {
    if (qEnvironmentVariableIsSet("SKIP_VISUAL_TEST")) {
        GTEST_SKIP() << "Set SKIP_VISUAL_TEST=1 to skip visual tests";
    }

    const WindowVisualLauncher launcher = createWindowVisualLauncher();
    QPointer<Window> appWindow;

    QObject::connect(launcher.showButton, &QPushButton::clicked, launcher.window, [&]() {
        showOrActivateVisualWindow(appWindow, launcher.window);
    });

    QObject::connect(launcher.window, &QObject::destroyed, qApp, [&]() {
        if (appWindow)
            appWindow->close();
        qApp->quit();
    });

    launcher.window->show();
    qApp->exec();
}
