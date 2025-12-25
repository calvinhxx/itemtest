#include "VM_ResponsiveDialog.h"

VM_ResponsiveDialog::VM_ResponsiveDialog(QObject *parent) 
    : QObject(parent) 
{}

void VM_ResponsiveDialog::setTitle(const QString &t) {
    if (m_title == t) return;
    m_title = t;
    emit titleChanged(m_title);
}

void VM_ResponsiveDialog::setVisible(bool v) {
    if (m_visible == v) return;
    m_visible = v;
    emit visibleChanged(m_visible);
}

