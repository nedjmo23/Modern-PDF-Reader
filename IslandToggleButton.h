#pragma once

#include "Common.h"

class IslandToggleButton : public QPushButton {
    Q_OBJECT
public:
    explicit IslandToggleButton(QWidget *parent = nullptr) 
        : QPushButton(parent), m_collapsed(false), m_theme(ThemeDark)
    {
        setFixedSize(28, 12);
        setCursor(Qt::PointingHandCursor);
        updateStyle();
    }

    void setCollapsed(bool collapsed) {
        m_collapsed = collapsed;
        update();
    }

    bool isCollapsed() const { return m_collapsed; }

    void updateTheme(ReadingTheme theme) {
        m_theme = theme;
        updateStyle();
    }

private:
    void updateStyle() {
        if (m_theme == ThemeDark || m_theme == ThemeNord) {
            setStyleSheet(
                "QPushButton { background: #252526; border: 1px solid #3d3d3d; border-top: none; "
                "border-bottom-left-radius: 6px; border-bottom-right-radius: 6px; }"
                "QPushButton:hover { background-color: #007acc; border-color: #007acc; }"
            );
        } else {
            setStyleSheet(
                "QPushButton { background: #e1e1e1; border: 1px solid #cccccc; border-top: none; "
                "border-bottom-left-radius: 6px; border-bottom-right-radius: 6px; }"
                "QPushButton:hover { background-color: #007acc; border-color: #007acc; }"
            );
        }
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPushButton::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        bool isDark = (m_theme == ThemeDark || m_theme == ThemeNord);
        p.setPen(QPen(isDark ? Qt::white : QColor(30, 30, 30), 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        int cx = width() / 2, cy = height() / 2;
        QPainterPath path;
        if (m_collapsed) {
            path.moveTo(cx - 4, cy - 2); path.lineTo(cx, cy + 2); path.lineTo(cx + 4, cy - 2);
        } else {
            path.moveTo(cx - 4, cy + 2); path.lineTo(cx, cy - 2); path.lineTo(cx + 4, cy + 2);
        }
        p.drawPath(path);
    }

private:
    bool m_collapsed;
    ReadingTheme m_theme;
};
