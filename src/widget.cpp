#include "widget.h"

#include <QPushButton>
#include "layout/AnchorLayout.h"
#include "items/DebugOverlay.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(500, 400);
    AnchorLayout *layout = new AnchorLayout(this);

    // button 1: top-left to parent
    QPushButton* btn1 = new QPushButton("TopLeft", this);
    btn1->setObjectName("btnTopLeft");
    AnchorSpec s1;
    s1.leftToParent = true; s1.leftOffset = 10;
    s1.topToParent = true; s1.topOffset = 10;
    s1.preferredSize = QSize(100, 30);
    layout->addAnchoredWidget(btn1, s1);

    // button 2: demonstrate left-to-left (align left edges)
    QPushButton* btn2 = new QPushButton("Align Left", this);
    btn2->setObjectName("btnAlignLeft");
    AnchorSpec s2;
    s2.leftToWidget = btn1; s2.leftToWidgetEdge = AnchorSpec::LeftEdge; // left-to-left
    s2.topToParent = true; s2.topOffset = 50;
    s2.preferredSize = QSize(120, 30);
    layout->addAnchoredWidget(btn2, s2);

    // button 3: center of parent
    QPushButton* btn3 = new QPushButton("Center", this);
    btn3->setObjectName("btnCenter");
    AnchorSpec s3;
    s3.horizontalCenter = true; s3.hCenterOffset = 0;
    s3.verticalCenter = true; s3.vCenterOffset = 0;
    s3.preferredSize = QSize(140, 40);
    layout->addAnchoredWidget(btn3, s3);

    // button 4: fill parent with margins (anchors.fill equivalent)
    QPushButton* btn4 = new QPushButton("Fill (demo)", this);
    btn4->setObjectName("btnFill");
    AnchorSpec s4;
    s4.fillParent = true;
    s4.fillMargins = QMargins(12, 12, 12, 80);
    layout->addAnchoredWidget(btn4, s4);

    setLayout(layout);

    // Debug overlay: 绘制边框并支持点击选中子控件
    DebugOverlay *dbg = new DebugOverlay(this);
    dbg->setOutlineEnabled(true);
    dbg->setLabelsEnabled(true);
    dbg->setSelectionEnabled(true); // 点击父窗口时会选中子控件并高亮
    dbg->raise();
}

Widget::~Widget() {}
