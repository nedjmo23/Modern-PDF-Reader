#pragma once

#include "Common.h"

class DynamicIsland : public QWidget {
    Q_OBJECT
public:
    enum ViewMode { Continuous = 0, SinglePage = 1, TwoPages = 2 };

    explicit DynamicIsland(QWidget *parent = nullptr) 
        : QWidget(parent), m_currentViewMode(Continuous), m_rotationAngle(0), m_nightMode(false), m_dimMode(false) 
    {
        setFixedHeight(32);
        setObjectName("IslandBody");
        updateThemeStyle(ThemeDark);

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 0, 10, 0);
        layout->setSpacing(6);

        QWidget *navContainer = new QWidget(this);
        QVBoxLayout *navLayout = new QVBoxLayout(navContainer);
        navLayout->setContentsMargins(0, 2, 0, 2);
        navLayout->setSpacing(0);

        QPushButton *btnUp = new QPushButton("▲", this);
        QPushButton *btnDown = new QPushButton("▼", this);
        btnUp->setFixedSize(16, 12);
        btnDown->setFixedSize(16, 12);
        
        QString arrowStyle = "QPushButton { font-size: 8px; color: #888888; padding:0px; }"
                            "QPushButton:hover { color: #007acc; background: transparent; }";
        btnUp->setStyleSheet(arrowStyle);
        btnDown->setStyleSheet(arrowStyle);

        navLayout->addWidget(btnUp);
        navLayout->addWidget(btnDown);
        layout->addWidget(navContainer);

        pageLabel = new QLabel("1 / 10", this);
        layout->addWidget(pageLabel);

        sep1 = createSeparator();
        layout->addWidget(sep1);

        QPushButton *btnZoomOut = new QPushButton("—", this);
        QPushButton *btnZoomIn = new QPushButton("+", this);
        btnZoomOut->setFixedSize(20, 20);
        btnZoomIn->setFixedSize(20, 20);

        zoomLabel = new QLabel("100%", this);

        layout->addWidget(btnZoomOut);
        layout->addWidget(zoomLabel);
        layout->addWidget(btnZoomIn);

        sep2 = createSeparator();
        layout->addWidget(sep2);

        btnViewMode = new QPushButton("📜 Scroll", this);
        btnViewMode->setToolTip("Change View Mode");
        connect(btnViewMode, &QPushButton::clicked, this, &DynamicIsland::toggleViewMode);
        layout->addWidget(btnViewMode);

        btnRotate = new QPushButton("🔄", this);
        btnRotate->setFixedSize(24, 24);
        btnRotate->setToolTip("Rotate Page 90°");
        connect(btnRotate, &QPushButton::clicked, this, &DynamicIsland::rotateClockwise);
        layout->addWidget(btnRotate);

        btnNightMode = new QPushButton("🌙", this);
        btnNightMode->setFixedSize(24, 24);
        btnNightMode->setToolTip("Invert PDF Colors");
        connect(btnNightMode, &QPushButton::clicked, this, &DynamicIsland::toggleNightMode);
        layout->addWidget(btnNightMode);

        btnDimMode = new QPushButton("🔆", this);
        btnDimMode->setFixedSize(24, 24);
        btnDimMode->setToolTip("Dim Background / Focus Mode");
        connect(btnDimMode, &QPushButton::clicked, this, &DynamicIsland::toggleDimMode);
        layout->addWidget(btnDimMode);

        adjustSize();
    }

    void updateThemeStyle(ReadingTheme theme) {
        if (theme == ThemeDark || theme == ThemeNord) {
            setStyleSheet(
                "QWidget#IslandBody { background-color: #222222; border: 1px solid #383838; border-radius: 16px; }"
                "QLabel { color: #cccccc; font-size: 11px; font-weight: bold; font-family: 'Segoe UI'; }"
                "QPushButton { background: transparent; border: none; color: #aaaaaa; font-size: 12px; font-weight: bold; border-radius: 8px; padding: 2px 5px; }"
                "QPushButton:hover { background-color: #333333; color: white; }"
            );
        } else {
            setStyleSheet(
                "QWidget#IslandBody { background-color: #ffffff; border: 1px solid #d0d0d0; border-radius: 16px; }"
                "QLabel { color: #333333; font-size: 11px; font-weight: bold; font-family: 'Segoe UI'; }"
                "QPushButton { background: transparent; border: none; color: #555555; font-size: 12px; font-weight: bold; border-radius: 8px; padding: 2px 5px; }"
                "QPushButton:hover { background-color: #e5e5e5; color: black; }"
            );
        }
    }

    QLabel *pageLabel;
    QLabel *zoomLabel;

private slots:
    void toggleViewMode() {
        m_currentViewMode = static_cast<ViewMode>((m_currentViewMode + 1) % 3);
        switch (m_currentViewMode) {
            case Continuous: btnViewMode->setText("📜 Scroll"); break;
            case SinglePage: btnViewMode->setText("📄 1-Page"); break;
            case TwoPages:  btnViewMode->setText("📖 2-Pages"); break;
        }
        adjustSize();
    }

    void rotateClockwise() { m_rotationAngle = (m_rotationAngle + 90) % 360; }
    void toggleNightMode() {
        m_nightMode = !m_nightMode;
        btnNightMode->setStyleSheet(m_nightMode ? "QPushButton { background-color: #007acc; color: white; border-radius: 8px; }" : "");
    }

    void toggleDimMode() {
        m_dimMode = !m_dimMode;
        btnDimMode->setStyleSheet(m_dimMode ? "QPushButton { background-color: #ff9900; color: white; border-radius: 8px; }" : "");
    }

private:
    QFrame* createSeparator() {
        QFrame *sep = new QFrame(this);
        sep->setFrameShape(QFrame::VLine);
        sep->setFixedSize(1, 14);
        sep->setStyleSheet("background-color: #383838; border: none;");
        return sep;
    }

    QFrame      *sep1;
    QFrame      *sep2;
    QPushButton *btnViewMode;
    QPushButton *btnRotate;
    QPushButton *btnNightMode;
    QPushButton *btnDimMode;

    ViewMode m_currentViewMode;
    int      m_rotationAngle;
    bool     m_nightMode;
    bool     m_dimMode;
};
