#include "VM_ResponsiveLabel.h"

VM_ResponsiveLabel::VM_ResponsiveLabel(QObject *parent) 
    : QObject(parent) 
{}

void VM_ResponsiveLabel::setText(const QString &t) {
    if (m_text == t) return;
    m_text = t;
    emit textChanged(m_text);
}
