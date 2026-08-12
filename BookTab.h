#pragma once

#include "Common.h"

class BookTab : public QWidget {
    Q_OBJECT
public:
    static const int TAB_WIDTH   = 180;
    static const int TAB_HEIGHT  = 30;
    static const int TAB_SPACING = 3;
    static const int TAB_Y       = 4;

    BookTab(const QString &title, const QString &filePath, QWidget *parent = nullptr)
        : QWidget(parent), m_title(title), m_filePath(filePath), m_selected(false),
          m_hovered(false), m_dragging(false), m_pinned(false), m_theme(ThemeDark)
    {
        setFixedSize(TAB_WIDTH, TAB_HEIGHT);
        setCursor(Qt::PointingHandCursor);

        closeBtn = new QPushButton("✕", this);
        closeBtn->setFixedSize(14, 14);
        closeBtn->setStyleSheet(
            "QPushButton { background: transparent; border: none; color: #888888; font-size: 10px; }"
            "QPushButton:hover { color: #ffffff; }"
        );
        closeBtn->move(TAB_WIDTH - 18, (TAB_HEIGHT - 14) / 2);
        connect(closeBtn, &QPushButton::clicked, this, &BookTab::closeRequested);
    }

    QString filePath() const { return m_filePath; }
    bool isPinned() const { return m_pinned; }
    void setPinned(bool pinned) {
        m_pinned = pinned;
        closeBtn->setVisible(!m_pinned);
        update();
    }

    void setSelected(bool s) { m_selected = s; update(); }
    void setTheme(ReadingTheme theme) { m_theme = theme; update(); }

    void stopAnimations() {
        for (QObject *child : children()) {
            if (auto *anim = qobject_cast<QPropertyAnimation*>(child))
                anim->stop();
        }
    }

signals:
    void clicked();
    void closeRequested();
    void pinToggled(BookTab *tab);
    void closeUnpinnedRequested();
    void dragStarted(BookTab *tab, int globalX);
    void dragMoved(BookTab *tab, int globalX);
    void dragEnded(BookTab *tab);

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QColor bg;
        switch (m_theme) {
            case ThemeLight: bg = m_selected ? QColor(255, 255, 255) : (m_hovered ? QColor(225, 225, 225) : QColor(210, 210, 210)); break;
            case ThemeDark:  bg = m_selected ? QColor(30, 30, 30) : (m_hovered ? QColor(50, 50, 50) : QColor(38, 38, 38)); break;
            case ThemeSepia: bg = m_selected ? QColor(245, 230, 210) : (m_hovered ? QColor(235, 222, 195) : QColor(225, 210, 180)); break;
            case ThemeNord:  bg = m_selected ? QColor(46, 52, 64) : (m_hovered ? QColor(67, 76, 94) : QColor(59, 66, 82)); break;
        }

        QPainterPath path;
        path.addRoundedRect(1, 1, width() - 2, height() - 1, 7, 7);
        p.fillPath(path, bg);

        if (m_selected) {
            p.setPen(QPen(QColor(0, 122, 204), 2));
            p.drawLine(8, height() - 1, width() - 8, height() - 1);
        }

        p.setPen((m_theme == ThemeDark || m_theme == ThemeNord) ? (m_selected ? Qt::white : QColor(170, 170, 170))
                                                               : (m_selected ? Qt::black : QColor(80, 80, 80)));
        QFont font = p.font();
        font.setPointSize(9);
        p.setFont(font);

        int textLeft = m_pinned ? 22 : 8;
        int textWidth = m_pinned ? (width() - 28) : (width() - 28);
        QRect textRect(textLeft, 0, textWidth, height());

        if (m_pinned) {
            p.drawText(6, height() / 2 + 4, "📌");
        }

        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
            p.fontMetrics().elidedText(m_title, Qt::ElideRight, textRect.width()));
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            m_pressPos = event->globalPosition().toPoint();
            m_dragging = false;
            emit clicked();
        } else if (event->button() == Qt::RightButton) {
            QMenu contextMenu(this);
            QAction *pinAction = contextMenu.addAction(m_pinned ? "📌 Unpin Tab" : "📌 Pin Tab");
            QAction *closeUnpinnedAction = contextMenu.addAction("🧹 Close Unpinned Tabs");
            QAction *closeAction = contextMenu.addAction("✕ Close Tab");

            // تنفيذ الأوامر بأمان بعد اختفاء القائمة المنبثقة لتجنب انهيار الذاكرة
            connect(pinAction, &QAction::triggered, this, [this]() { 
                QTimer::singleShot(0, this, [this]() { emit pinToggled(this); });
            });
            connect(closeUnpinnedAction, &QAction::triggered, this, [this]() { 
                QTimer::singleShot(0, this, [this]() { emit closeUnpinnedRequested(); });
            });
            connect(closeAction, &QAction::triggered, this, [this]() { 
                QTimer::singleShot(0, this, [this]() { emit closeRequested(); });
            });

            contextMenu.exec(event->globalPosition().toPoint());
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (!(event->buttons() & Qt::LeftButton) || m_pinned) return;
        int dx = (event->globalPosition().toPoint() - m_pressPos).manhattanLength();
        if (!m_dragging && dx > 6) {
            m_dragging = true;
            emit dragStarted(this, event->globalPosition().toPoint().x());
        }
        if (m_dragging) emit dragMoved(this, event->globalPosition().toPoint().x());
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && m_dragging) {
            m_dragging = false;
            emit dragEnded(this);
        }
    }

    void enterEvent(QEnterEvent *) override { m_hovered = true;  update(); }
    void leaveEvent(QEvent *)       override { m_hovered = false; update(); }

private:
    QString      m_title;
    QString      m_filePath;
    bool         m_selected;
    bool         m_hovered;
    bool         m_dragging;
    bool         m_pinned;
    ReadingTheme m_theme;
    QPoint       m_pressPos;
    QPushButton *closeBtn;
};
