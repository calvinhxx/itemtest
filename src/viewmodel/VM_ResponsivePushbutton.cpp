#include "VM_ResponsivePushbutton.h"

VM_ResponsivePushbutton::VM_ResponsivePushbutton(QObject *parent) 
    : QObject(parent) 
{}

void VM_ResponsivePushbutton::setText(const QString &t) {
    if (m_text == t) return;
    m_text = t;
    emit textChanged(m_text);
}

void VM_ResponsivePushbutton::setEnabled(bool e) {
    if (m_enabled == e) return;
    m_enabled = e;
    emit enabledChanged(m_enabled);
}

