#include "ViewModel.h"

ViewModel::ViewModel(QObject *parent)
    : QObject(parent) {
}

void ViewModel::setText(const QString &t) {
    if (m_text != t) {
        m_text = t;
        emit textChanged(t);
    }
}

void ViewModel::setEnabled(bool e) {
    if (m_enabled != e) {
        m_enabled = e;
        emit enabledChanged(e);
    }
}

void ViewModel::setTitle(const QString &t) {
    if (m_title != t) {
        m_title = t;
        emit titleChanged(t);
    }
}

void ViewModel::setVisible(bool v) {
    if (m_visible != v) {
        m_visible = v;
        emit visibleChanged(v);
    }
}

