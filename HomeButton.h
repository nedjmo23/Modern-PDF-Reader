#pragma once

#include "Common.h"

class HomeButton : public QPushButton {
    Q_OBJECT
public:
    explicit HomeButton(QWidget *parent = nullptr) : QPushButton(parent), m_theme(ThemeDark) {
        setFixedSize(28, 28);
        setCursor(Qt::PointingHandCursor);
        updateStyle();
    }

    void setTheme(ReadingTheme theme) {
        m_theme = theme;
        updateStyle();
        update();
    }

private:
    void updateStyle() {
        if (m_theme == ThemeDark || m_theme == ThemeNord) {
            setStyleSheet("QPushButton { background: transparent; border: none; border-radius: 4px; }"
                          "QPushButton:hover { background-color: #2d2d2d; }");
        } else {
            setStyleSheet("QPushButton { background: transparent; border: none; border-radius: 4px; }"
                          "QPushButton:hover { background-color: #d0d0d0; }");
        }
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPushButton::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        bool isDark = (m_theme == ThemeDark || m_theme == ThemeNord);
        p.setPen(QPen(isDark ? Qt::white : QColor(40, 40, 40), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p.setBrush(Qt::NoBrush);
        int cx = width() / 2, cy = height() / 2;
        QPainterPath path;
        path.moveTo(cx-7, cy+1); path.lineTo(cx, cy-6); path.lineTo(cx+7, cy+1);
        path.moveTo(cx-5, cy);   path.lineTo(cx-5, cy+7);
        path.lineTo(cx+5, cy+7); path.lineTo(cx+5, cy);
        path.moveTo(cx-2, cy+7); path.lineTo(cx-2, cy+3);
        path.lineTo(cx+2, cy+3); path.lineTo(cx+2, cy+7);
        p.drawPath(path);
    }

private:
    ReadingTheme m_theme;
};
