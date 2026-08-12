#pragma once

#include "Common.h"

class RecentCard : public QFrame {
    Q_OBJECT
public:
    RecentCard(const QString &filePath, const QString &lastOpened, QWidget *parent = nullptr)
        : QFrame(parent), m_filePath(filePath), m_theme(ThemeDark)
    {
        setFixedSize(220, 260);
        setCursor(Qt::PointingHandCursor);
        setObjectName("RecentCard");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSpacing(6);

        coverLabel = new QLabel(this);
        coverLabel->setFixedSize(204, 170);
        coverLabel->setAlignment(Qt::AlignCenter);
        coverLabel->setText("📄");
        layout->addWidget(coverLabel);

        QFileInfo info(filePath);
        titleLabel = new QLabel(info.fileName(), this);
        titleLabel->setFont(QFont("Segoe UI", 10, QFont::Bold));
        titleLabel->setToolTip(info.fileName());
        layout->addWidget(titleLabel);

        dateLabel = new QLabel(lastOpened, this);
        dateLabel->setFont(QFont("Segoe UI", 8));
        layout->addWidget(dateLabel);

        deleteBtn = new QPushButton("✕", this);
        deleteBtn->setFixedSize(22, 22);
        deleteBtn->move(width() - 26, 6);
        deleteBtn->setCursor(Qt::PointingHandCursor);
        connect(deleteBtn, &QPushButton::clicked, this, [this](bool){
            emit deleteRequested(m_filePath);
        });

        updateTheme(ThemeDark);
    }

    QString filePath() const { return m_filePath; }

    void updateTheme(ReadingTheme theme) {
        m_theme = theme;
        if (m_theme == ThemeDark || m_theme == ThemeNord) {
            setStyleSheet(
                "QFrame#RecentCard { background-color: #222222; border: 1px solid #333333; border-radius: 8px; }"
                "QFrame#RecentCard:hover { background-color: #2a2a2a; border-color: #007acc; }"
            );
            coverLabel->setStyleSheet("background-color: #2a2a2a; border-radius: 6px; color: #888888; font-size: 42px;");
            titleLabel->setStyleSheet("color: #ffffff;");
            dateLabel->setStyleSheet("color: #888888;");
        } else {
            setStyleSheet(
                "QFrame#RecentCard { background-color: #ffffff; border: 1px solid #e0e0e0; border-radius: 8px; }"
                "QFrame#RecentCard:hover { background-color: #f8f8f8; border-color: #007acc; }"
            );
            coverLabel->setStyleSheet("background-color: #e9e9e9; border-radius: 6px; color: #555555; font-size: 42px;");
            titleLabel->setStyleSheet("color: #222222;");
            dateLabel->setStyleSheet("color: #666666;");
        }
        deleteBtn->setStyleSheet(
            "QPushButton { background: #333333; color: #aaaaaa; border-radius: 11px; border: none; font-size: 11px; }"
            "QPushButton:hover { background: #e81123; color: white; }"
        );
    }

signals:
    void clicked(const QString &filePath);
    void deleteRequested(const QString &filePath);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && !deleteBtn->underMouse()) {
            emit clicked(m_filePath);
        }
    }

private:
    QString      m_filePath;
    QLabel      *coverLabel;
    QLabel      *titleLabel;
    QLabel      *dateLabel;
    QPushButton *deleteBtn;
    ReadingTheme m_theme;
};
