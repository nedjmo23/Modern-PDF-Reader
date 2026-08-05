// main.cpp - Fixed GitHub Actions CMake AUTOMOC & C2039 Build Errors
#include <QApplication>
#include <QMainWindow>
#include <QStackedWidget>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QFileInfo>
#include <QPainter>
#include <QLabel>
#include <QPainterPath>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QVector>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSettings>
#include <QDateTime>
#include <QScrollArea>
#include <QScroller>

// ─────────────────────────────────────────────
// 1. زر السهم لطي وإظهار الجزيرة الديناميكية
// ─────────────────────────────────────────────
class IslandToggleButton : public QPushButton {
    Q_OBJECT
public:
    explicit IslandToggleButton(QWidget *parent = nullptr) 
        : QPushButton(parent), m_collapsed(false), m_isDark(true)
    {
        setFixedSize(28, 12);
        setCursor(Qt::PointingHandCursor);
        updateStyle();
    }

    void setCollapsed(bool collapsed) {
        m_collapsed = collapsed;
        update();
    }

    bool isCollapsed() const { 
        return m_collapsed; 
    }

    void updateTheme(bool isDark) {
        m_isDark = isDark;
        updateStyle();
    }

private:
    void updateStyle() {
        if (m_isDark) {
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
        p.setPen(QPen(m_isDark ? Qt::white : QColor(30, 30, 30), 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        int cx = width() / 2;
        int cy = height() / 2;

        QPainterPath path;
        if (m_collapsed) {
            path.moveTo(cx - 4, cy - 2);
            path.lineTo(cx, cy + 2);
            path.lineTo(cx + 4, cy - 2);
        } else {
            path.moveTo(cx - 4, cy + 2);
            path.lineTo(cx, cy - 2);
            path.lineTo(cx + 4, cy + 2);
        }
        p.drawPath(path);
    }

private:
    bool m_collapsed;
    bool m_isDark;
};

// ─────────────────────────────────────────────
// 2. الجزيرة الديناميكية (Dynamic Island)
// ─────────────────────────────────────────────
class DynamicIsland : public QWidget {
    Q_OBJECT
public:
    enum ViewMode { Continuous = 0, SinglePage = 1, TwoPages = 2 };

    explicit DynamicIsland(QWidget *parent = nullptr) 
        : QWidget(parent), m_currentViewMode(Continuous), m_rotationAngle(0), m_nightMode(false), m_dimMode(false) 
    {
        setFixedHeight(32);
        setObjectName("IslandBody");
        updateThemeStyle(true);

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

    void updateThemeStyle(bool isDark) {
        if (isDark) {
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

// ─────────────────────────────────────────────
// 3. تبويبة الكتاب
// ─────────────────────────────────────────────
class BookTab : public QWidget {
    Q_OBJECT
public:
    static const int TAB_WIDTH   = 180;
    static const int TAB_HEIGHT  = 30;
    static const int TAB_SPACING = 3;
    static const int TAB_Y       = 4;

    BookTab(const QString &title, const QString &filePath, QWidget *parent = nullptr)
        : QWidget(parent), m_title(title), m_filePath(filePath), m_selected(false),
          m_hovered(false), m_dragging(false), m_isDark(true)
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
    void setSelected(bool s) { m_selected = s; update(); }
    void setTheme(bool isDark) { m_isDark = isDark; update(); }

    void stopAnimations() {
        for (QObject *child : children()) {
            if (auto *anim = qobject_cast<QPropertyAnimation*>(child))
                anim->stop();
        }
    }

signals:
    void clicked();
    void closeRequested();
    void dragStarted(BookTab *tab, int globalX);
    void dragMoved(BookTab *tab, int globalX);
    void dragEnded(BookTab *tab);

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QColor bg;
        if (m_isDark) {
            bg = m_selected ? QColor(30, 30, 30) : (m_hovered ? QColor(50, 50, 50) : QColor(38, 38, 38));
        } else {
            bg = m_selected ? QColor(255, 255, 255) : (m_hovered ? QColor(225, 225, 225) : QColor(210, 210, 210));
        }

        QPainterPath path;
        path.addRoundedRect(1, 1, width() - 2, height() - 1, 7, 7);
        p.fillPath(path, bg);

        if (m_selected) {
            p.setPen(QPen(m_isDark ? QColor(200, 200, 200) : QColor(0, 122, 204), 2));
            p.drawLine(8, height() - 1, width() - 8, height() - 1);
        }

        p.setPen(m_isDark ? (m_selected ? Qt::white : QColor(170, 170, 170))
                          : (m_selected ? Qt::black : QColor(80, 80, 80)));
        QFont font = p.font();
        font.setPointSize(9);
        p.setFont(font);
        QRect textRect(8, 0, width() - 28, height());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft,
            p.fontMetrics().elidedText(m_title, Qt::ElideRight, textRect.width()));
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            m_pressPos = event->globalPosition().toPoint();
            m_dragging = false;
            emit clicked();
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (!(event->buttons() & Qt::LeftButton)) return;
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
    bool         m_isDark;
    QPoint       m_pressPos;
    QPushButton *closeBtn;
};

// ─────────────────────────────────────────────
// 4. بطاقة الكتاب العريضة (Recent Card)
// ─────────────────────────────────────────────
class RecentCard : public QFrame {
    Q_OBJECT
public:
    RecentCard(const QString &filePath, const QString &lastOpened, QWidget *parent = nullptr)
        : QFrame(parent), m_filePath(filePath), m_isDark(true)
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

        updateTheme(true);
    }

    QString filePath() const { return m_filePath; }

    void updateTheme(bool isDark) {
        m_isDark = isDark;
        if (m_isDark) {
            setStyleSheet(
                "QFrame#RecentCard { background-color: #222222; border: 1px solid #333333; border-radius: 8px; }"
                "QFrame#RecentCard:hover { background-color: #2a2a2a; border-color: #007acc; }"
            );
            coverLabel->setStyleSheet("background-color: #2a2a2a; border-radius: 6px; color: #888888; font-size: 42px;");
            titleLabel->setStyleSheet("color: #ffffff;");
            dateLabel->setStyleSheet("color: #888888;");
            deleteBtn->setStyleSheet(
                "QPushButton { background: #333333; color: #aaaaaa; border-radius: 11px; border: none; font-size: 11px; }"
                "QPushButton:hover { background: #e81123; color: white; }"
            );
        } else {
            setStyleSheet(
                "QFrame#RecentCard { background-color: #ffffff; border: 1px solid #e0e0e0; border-radius: 8px; }"
                "QFrame#RecentCard:hover { background-color: #f8f8f8; border-color: #007acc; }"
            );
            coverLabel->setStyleSheet("background-color: #e9e9e9; border-radius: 6px; color: #555555; font-size: 42px;");
            titleLabel->setStyleSheet("color: #222222;");
            dateLabel->setStyleSheet("color: #666666;");
            deleteBtn->setStyleSheet(
                "QPushButton { background: #e0e0e0; color: #555555; border-radius: 11px; border: none; font-size: 11px; }"
                "QPushButton:hover { background: #e81123; color: white; }"
            );
        }
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
    bool         m_isDark;
};

// ─────────────────────────────────────────────
// 5. زر المنزل
// ─────────────────────────────────────────────
class HomeButton : public QPushButton {
    Q_OBJECT
public:
    explicit HomeButton(QWidget *parent = nullptr) : QPushButton(parent), m_isDark(true) {
        setFixedSize(28, 28);
        setCursor(Qt::PointingHandCursor);
        updateStyle();
    }

    void setTheme(bool isDark) {
        m_isDark = isDark;
        updateStyle();
        update();
    }

private:
    void updateStyle() {
        if (m_isDark) {
            setStyleSheet(
                "QPushButton { background: transparent; border: none; border-radius: 4px; }"
                "QPushButton:hover { background-color: #2d2d2d; }"
            );
        } else {
            setStyleSheet(
                "QPushButton { background: transparent; border: none; border-radius: 4px; }"
                "QPushButton:hover { background-color: #d0d0d0; }"
            );
        }
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPushButton::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(m_isDark ? Qt::white : QColor(40, 40, 40), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
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
    bool m_isDark;
};

// ─────────────────────────────────────────────
// 6. النافذة الرئيسية (ModernPDFReader)
// ─────────────────────────────────────────────
class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader()
        : m_draggingWindow(false), m_currentIndex(-1),
          m_draggedTab(nullptr), m_dragOffsetX(0), m_islandVisible(false), m_isDarkMode(true)
    {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        resize(1100, 800);
        setMinimumSize(800, 600);

        centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // ── الشريط العلوي ─────────────────────────────────────
        header = new QWidget(this);
        header->setFixedHeight(38);
        QHBoxLayout *headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(6, 4, 4, 0);
        headerLayout->setSpacing(4);

        menuBtn = new QPushButton("⋮", this);
        menuBtn->setFixedSize(28, 28);
        menuBtn->setCursor(Qt::PointingHandCursor);

        mainMenu = new QMenu(this);
        QAction *openAction = mainMenu->addAction("📁 Open PDF");
        connect(openAction, &QAction::triggered, this, &ModernPDFReader::openPDFFileDialog);

        settingsMenu = mainMenu->addMenu("⚙️ Settings");
        themeAction = settingsMenu->addAction("☀️ Light Mode");
        connect(themeAction, &QAction::triggered, this, &ModernPDFReader::toggleTheme);

        QAction *dirAction = settingsMenu->addAction("📂 Default Open Directory");
        connect(dirAction, &QAction::triggered, this, &ModernPDFReader::selectDefaultDirectory);

        connect(menuBtn, &QPushButton::clicked, this, [this]() {
            mainMenu->exec(menuBtn->mapToGlobal(QPoint(0, menuBtn->height())));
        });
        headerLayout->addWidget(menuBtn);

        btnHome = new HomeButton(this);
        connect(btnHome, &QPushButton::clicked, this, &ModernPDFReader::showHomePage);
        headerLayout->addWidget(btnHome);

        headerSep = new QFrame(this);
        headerSep->setFrameShape(QFrame::VLine);
        headerSep->setFixedSize(2, 22);
        headerLayout->addWidget(headerSep);
        headerLayout->addSpacing(2);

        tabsContainer = new QWidget(this);
        tabsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        tabsContainer->setFixedHeight(38);
        headerLayout->addWidget(tabsContainer, 1);

        btnMin   = new QPushButton("–",  this);
        btnMax   = new QPushButton("⬜", this);
        btnClose = new QPushButton("✕",  this);

        connect(btnMin,   &QPushButton::clicked, this, &ModernPDFReader::showMinimized);
        connect(btnMax,   &QPushButton::clicked, this, [this]() { isMaximized() ? showNormal() : showMaximized(); });
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);
        
        headerLayout->addWidget(btnMin);
        headerLayout->addWidget(btnMax);
        headerLayout->addWidget(btnClose);

        mainLayout->addWidget(header);

        // ── منطقة العرض والصفحة الرئيسية ──────────────────────
        stackedWidget = new QStackedWidget(this);
        
        setupHomePage();

        stackedWidget->addWidget(homePageWidget);
        stackedWidget->setCurrentIndex(0);
        mainLayout->addWidget(stackedWidget, 1);
        setCentralWidget(centralWidget);

        // ── الجزيرة الديناميكية ──
        dynamicIsland = new DynamicIsland(this);
        islandToggleBtn = new IslandToggleButton(this);

        dynamicIsland->hide();
        islandToggleBtn->hide();

        connect(islandToggleBtn, &QPushButton::clicked, this, &ModernPDFReader::toggleIslandState);

        applyTheme();
        loadRecentHistory();
    }

private slots:
    void setupHomePage() {
        homePageWidget = new QWidget();
        QVBoxLayout *homeLayout = new QVBoxLayout(homePageWidget);
        homeLayout->setContentsMargins(40, 30, 40, 30);

        QHBoxLayout *recentHeader = new QHBoxLayout();
        QLabel *recentTitle = new QLabel("Recent Books", homePageWidget);
        recentTitle->setFont(QFont("Segoe UI", 16, QFont::Bold));

        btnClearHistory = new QPushButton("🗑️ Clear History", homePageWidget);
        btnClearHistory->setCursor(Qt::PointingHandCursor);
        btnClearHistory->setFixedSize(120, 28);
        connect(btnClearHistory, &QPushButton::clicked, this, &ModernPDFReader::clearAllHistory);

        recentHeader->addWidget(recentTitle);
        recentHeader->addStretch();
        recentHeader->addWidget(btnClearHistory);
        homeLayout->addLayout(recentHeader);

        QScrollArea *scrollArea = new QScrollArea(homePageWidget);
        scrollArea->setWidgetResizable(true);
        scrollArea->setFrameShape(QFrame::NoFrame);
        
        QScroller::grabGesture(scrollArea->viewport(), QScroller::LeftMouseButtonGesture);

        cardsContainerWidget = new QWidget(scrollArea);
        recentGrid = new QGridLayout(cardsContainerWidget);
        recentGrid->setSpacing(20);
        recentGrid->setContentsMargins(0, 10, 0, 0);

        scrollArea->setWidget(cardsContainerWidget);
        homeLayout->addWidget(scrollArea);
    }

    void selectDefaultDirectory() {
        QSettings settings("ModernPDFReader", "Settings");
        QString currentDir = settings.value("defaultDir", "").toString();
        QString dir = QFileDialog::getExistingDirectory(this, "Select Default Directory", currentDir);
        if (!dir.isEmpty()) {
            settings.setValue("defaultDir", dir);
        }
    }

    void toggleTheme() {
        m_isDarkMode = !m_isDarkMode;
        themeAction->setText(m_isDarkMode ? "☀️ Light Mode" : "🌙 Dark Mode");
        applyTheme();
    }

    void applyTheme() {
        if (m_isDarkMode) {
            centralWidget->setStyleSheet("background-color: #141414; border: none;");
            header->setStyleSheet("background-color: #1c1c1c; border: none;");
            headerSep->setStyleSheet("background-color: #3a3a3a; border: none;");
            
            menuBtn->setStyleSheet(
                "QPushButton { background: transparent; border: none; color: #ffffff; font-size: 18px; font-weight: bold; border-radius: 4px; }"
                "QPushButton:hover { background-color: #2d2d2d; }"
            );
            mainMenu->setStyleSheet(
                "QMenu { background-color: #2d2d2d; color: #ffffff; border: 1px solid #3d3d3d; padding: 4px; font-size: 13px; }"
                "QMenu::item { padding: 5px 20px; border-radius: 3px; }"
                "QMenu::item:selected { background-color: #007acc; }"
            );

            QString btnStyle = "QPushButton { background: transparent; color: #aaaaaa; border: none; font-size: 12px; width: 38px; height: 28px; }"
                               "QPushButton:hover { background-color: #2d2d2d; color: white; }";
            btnMin->setStyleSheet(btnStyle);
            btnMax->setStyleSheet(btnStyle);
            btnClose->setStyleSheet("QPushButton { background: transparent; color: #aaaaaa; border: none; font-size: 12px; width: 38px; height: 28px; }"
                                   "QPushButton:hover { background-color: #e81123; color: white; }");

            btnClearHistory->setStyleSheet(
                "QPushButton { background-color: #252526; color: #cccccc; border: 1px solid #3d3d3d; border-radius: 6px; font-size: 11px; }"
                "QPushButton:hover { background-color: #e81123; color: white; border-color: #e81123; }"
            );
        } else {
            centralWidget->setStyleSheet("background-color: #f3f3f3; border: none;");
            header->setStyleSheet("background-color: #e5e5e5; border: none;");
            headerSep->setStyleSheet("background-color: #cccccc; border: none;");
            
            menuBtn->setStyleSheet(
                "QPushButton { background: transparent; border: none; color: #222222; font-size: 18px; font-weight: bold; border-radius: 4px; }"
                "QPushButton:hover { background-color: #d0d0d0; }"
            );
            mainMenu->setStyleSheet(
                "QMenu { background-color: #ffffff; color: #222222; border: 1px solid #cccccc; padding: 4px; font-size: 13px; }"
                "QMenu::item { padding: 5px 20px; border-radius: 3px; }"
                "QMenu::item:selected { background-color: #007acc; color: white; }"
            );

            QString btnStyle = "QPushButton { background: transparent; color: #444444; border: none; font-size: 12px; width: 38px; height: 28px; }"
                               "QPushButton:hover { background-color: #d0d0d0; color: black; }";
            btnMin->setStyleSheet(btnStyle);
            btnMax->setStyleSheet(btnStyle);
            btnClose->setStyleSheet("QPushButton { background: transparent; color: #444444; border: none; font-size: 12px; width: 38px; height: 28px; }"
                                   "QPushButton:hover { background-color: #e81123; color: white; }");

            btnClearHistory->setStyleSheet(
                "QPushButton { background-color: #ffffff; color: #333333; border: 1px solid #cccccc; border-radius: 6px; font-size: 11px; }"
                "QPushButton:hover { background-color: #e81123; color: white; border-color: #e81123; }"
            );
        }

        btnHome->setTheme(m_isDarkMode);
        dynamicIsland->updateThemeStyle(m_isDarkMode);
        islandToggleBtn->updateTheme(m_isDarkMode);

        for (auto *tab : m_tabs) tab->setTheme(m_isDarkMode);
        for (auto *card : m_recentCards) card->updateTheme(m_isDarkMode);
    }

   void loadRecentHistory() {
    QSettings settings("ModernPDFReader", "History");
    QStringList recentFiles = settings.value("recentFiles").toStringList();

    for (auto *card : m_recentCards) {
        recentGrid->removeWidget(card);
        card->deleteLater(); // تم استبدال delete بـ deleteLater للوقاية من الخروج
    }
    m_recentCards.clear();

    int row = 0, col = 0;
    for (const QString &path : recentFiles) {
        if (!QFile::exists(path)) continue;

        QString timeStr = settings.value("time_" + path, "Recently").toString();
        RecentCard *card = new RecentCard(path, timeStr, cardsContainerWidget);
        card->updateTheme(m_isDarkMode);

        // استخدام Qt::QueuedConnection يضمن تنفيذ الفتح بعد انتهاء حدث الضغط بسلام
        connect(card, &RecentCard::clicked, this, &ModernPDFReader::openPDFFilePath, Qt::QueuedConnection);
        connect(card, &RecentCard::deleteRequested, this, &ModernPDFReader::deleteRecentCard, Qt::QueuedConnection);

        recentGrid->addWidget(card, row, col);
        m_recentCards.append(card);

        col++;
        if (col >= 4) { col = 0; row++; }
        if (m_recentCards.size() >= 8) break;
    }
}

    void saveToRecentHistory(const QString &filePath) {
        QSettings settings("ModernPDFReader", "History");
        QStringList recentFiles = settings.value("recentFiles").toStringList();
        recentFiles.removeAll(filePath);
        recentFiles.prepend(filePath);

        while (recentFiles.size() > 8) recentFiles.removeLast();

        settings.setValue("recentFiles", recentFiles);
        settings.setValue("time_" + filePath, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));

        loadRecentHistory();
    }

    void deleteRecentCard(const QString &filePath) {
        QSettings settings("ModernPDFReader", "History");
        QStringList recentFiles = settings.value("recentFiles").toStringList();
        recentFiles.removeAll(filePath);
        settings.setValue("recentFiles", recentFiles);
        settings.remove("time_" + filePath);

        loadRecentHistory();
    }

    void clearAllHistory() {
        QSettings settings("ModernPDFReader", "History");
        settings.clear();
        loadRecentHistory();
    }

    void openPDFFileDialog() {
        QSettings settings("ModernPDFReader", "Settings");
        QString defaultDir = settings.value("defaultDir", "").toString();

        QString filePath = QFileDialog::getOpenFileName(this, "Open PDF", defaultDir, "PDF Files (*.pdf)");
        if (!filePath.isEmpty()) openPDFFilePath(filePath);
    }

    void openPDFFilePath(const QString &filePath) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i]->filePath() == filePath) {
                selectTab(i);
                return;
            }
        }

        saveToRecentHistory(filePath);

        QWidget *viewContainer = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(viewContainer);
        layout->setContentsMargins(0, 0, 0, 0);

        QLabel *label = new QLabel("MuPDF Engine view for: " + QFileInfo(filePath).fileName(), viewContainer);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(m_isDarkMode ? "color: white; font-size: 20px;" : "color: black; font-size: 20px;");
        layout->addWidget(label);

        stackedWidget->addWidget(viewContainer);

        BookTab *tab = new BookTab(QFileInfo(filePath).fileName(), filePath, tabsContainer);
        tab->setTheme(m_isDarkMode);
        tab->show();

        m_tabs.append(tab);
        m_views.append(viewContainer);

        connect(tab, &BookTab::clicked, this, [this, tab]() { selectTab(m_tabs.indexOf(tab)); });
        connect(tab, &BookTab::closeRequested, this, [this, tab]() { closeTabByWidget(tab); });

        connect(tab, &BookTab::dragStarted, this, &ModernPDFReader::onDragStarted);
        connect(tab, &BookTab::dragMoved, this, &ModernPDFReader::onDragMoved);
        connect(tab, &BookTab::dragEnded, this, &ModernPDFReader::onDragEnded);

        repositionTabs(false);
        selectTab(m_tabs.size() - 1);
    }

    void selectTab(int index) {
        if (index < 0 || index >= m_tabs.size()) return;
        m_currentIndex = index;
        for (int i = 0; i < m_tabs.size(); i++)
            m_tabs[i]->setSelected(i == index);
        stackedWidget->setCurrentWidget(m_views[index]);
        setIslandVisible(true);
    }

    void closeTabByWidget(BookTab *tab) {
        int index = m_tabs.indexOf(tab);
        if (index < 0) return;

        int activeIndex = m_currentIndex;
        tab->stopAnimations();

        QWidget *view = m_views[index];
        stackedWidget->removeWidget(view);
        m_tabs.removeAt(index);
        m_views.removeAt(index);
        delete tab;
        delete view;

        if (m_tabs.isEmpty()) {
            m_currentIndex = -1;
            stackedWidget->setCurrentWidget(homePageWidget);
            setIslandVisible(false);
        } else if (index == activeIndex) {
            selectTab(qMin(activeIndex, m_tabs.size() - 1));
        } else {
            if (activeIndex > index) activeIndex--;
            m_currentIndex = activeIndex;
            for (int i = 0; i < m_tabs.size(); i++)
                m_tabs[i]->setSelected(i == m_currentIndex);
        }
        repositionTabs(true);
    }

    void showHomePage() {
        stackedWidget->setCurrentWidget(homePageWidget);
        for (auto *tab : m_tabs) tab->setSelected(false);
        m_currentIndex = -1;
        setIslandVisible(false);
    }

    void toggleIslandState() {
        bool isCollapsed = islandToggleBtn->isCollapsed();
        islandToggleBtn->setCollapsed(!isCollapsed);
        animateIslandPosition(!isCollapsed);
    }

    void onDragStarted(BookTab *tab, int globalX) {
        m_draggedTab     = tab;
        m_dragOffsetX    = globalX - tabsContainer->mapToGlobal(tab->pos()).x();
        m_draggingWindow = false;
        tab->raise();
    }

    void onDragMoved(BookTab *tab, int globalX) {
        if (!m_draggedTab || m_draggedTab != tab) return;

        int localX = tabsContainer->mapFromGlobal(QPoint(globalX, 0)).x() - m_dragOffsetX;
        localX = qBound(0, localX, tabsContainer->width() - BookTab::TAB_WIDTH);
        tab->move(localX, BookTab::TAB_Y);

        int step     = BookTab::TAB_WIDTH + BookTab::TAB_SPACING;
        int centerX  = localX + BookTab::TAB_WIDTH / 2;
        int newIndex = qBound(0, centerX / step, m_tabs.size() - 1);
        int oldIndex = m_tabs.indexOf(tab);

        if (newIndex != oldIndex) {
            m_tabs.removeAt(oldIndex);
            m_views.insert(newIndex, m_views.takeAt(oldIndex));
            m_tabs.insert(newIndex, tab);
            m_currentIndex = newIndex;
            animateTabsExcept(tab);
        }
    }

    void onDragEnded(BookTab *tab) {
        if (!m_draggedTab) return;
        m_draggedTab = nullptr;
        int index   = m_tabs.indexOf(tab);
        int targetX = index * (BookTab::TAB_WIDTH + BookTab::TAB_SPACING);
        animateTab(tab, targetX);
    }

    void animateTab(BookTab *tab, int targetX) {
        if (tab->x() == targetX) return;
        QPropertyAnimation *anim = new QPropertyAnimation(tab, "pos", tab);
        anim->setDuration(130);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->setStartValue(tab->pos());
        anim->setEndValue(QPoint(targetX, BookTab::TAB_Y));
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void animateTabsExcept(BookTab *except) {
        int step = BookTab::TAB_WIDTH + BookTab::TAB_SPACING;
        for (int i = 0; i < m_tabs.size(); i++) {
            if (m_tabs[i] == except) continue;
            animateTab(m_tabs[i], i * step);
        }
    }

    void repositionTabs(bool animated) {
        int step = BookTab::TAB_WIDTH + BookTab::TAB_SPACING;
        for (int i = 0; i < m_tabs.size(); i++) {
            if (animated) animateTab(m_tabs[i], i * step);
            else m_tabs[i]->move(i * step, BookTab::TAB_Y);
        }
    }

    void setIslandVisible(bool visible) {
        m_islandVisible = visible;
        if (visible) {
            dynamicIsland->show();
            islandToggleBtn->show();
            dynamicIsland->raise();
            islandToggleBtn->raise();
            islandToggleBtn->setCollapsed(false);
            animateIslandPosition(false);
        } else {
            dynamicIsland->hide();
            islandToggleBtn->hide();
        }
    }

    void updateIslandPosition(bool collapsed) {
        if (!m_islandVisible) return;
        int centerX = width() / 2;
        int islandX = centerX - (dynamicIsland->width() / 2);
        int targetY = collapsed ? (-dynamicIsland->height() + 2) : 42;

        dynamicIsland->move(islandX, targetY);
        int toggleY = collapsed ? 38 : (dynamicIsland->y() + dynamicIsland->height() - 1);
        islandToggleBtn->move(centerX - (islandToggleBtn->width() / 2), toggleY);
        dynamicIsland->raise();
        islandToggleBtn->raise();
    }

    void animateIslandPosition(bool collapsed) {
        if (!m_islandVisible) return;

        int centerX = width() / 2;
        int islandX = centerX - (dynamicIsland->width() / 2);
        int endY = collapsed ? (-dynamicIsland->height() + 2) : 42;

        QPropertyAnimation *animIsland = new QPropertyAnimation(dynamicIsland, "pos", this);
        animIsland->setDuration(220);
        animIsland->setEasingCurve(QEasingCurve::OutCubic);
        animIsland->setStartValue(dynamicIsland->pos());
        animIsland->setEndValue(QPoint(islandX, endY));

        int toggleEndY = collapsed ? 38 : (endY + dynamicIsland->height() - 1);
        QPropertyAnimation *animBtn = new QPropertyAnimation(islandToggleBtn, "pos", this);
        animBtn->setDuration(220);
        animBtn->setEasingCurve(QEasingCurve::OutCubic);
        animBtn->setStartValue(islandToggleBtn->pos());
        animBtn->setEndValue(QPoint(centerX - (islandToggleBtn->width() / 2), toggleEndY));

        animIsland->start(QAbstractAnimation::DeleteWhenStopped);
        animBtn->start(QAbstractAnimation::DeleteWhenStopped);
    }

protected:
    void resizeEvent(QResizeEvent *event) override {
        QMainWindow::resizeEvent(event);
        updateIslandPosition(false);
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && event->position().y() < 38) {
            m_windowDragStart = event->globalPosition().toPoint() - frameGeometry().topLeft();
            m_draggingWindow  = true;
            event->accept();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (m_draggingWindow && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPosition().toPoint() - m_windowDragStart);
            event->accept();
        }
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) m_draggingWindow = false;
    }

private:
    QWidget                   *centralWidget;
    QWidget                   *header;
    QFrame                    *headerSep;
    QStackedWidget            *stackedWidget;
    QWidget                   *homePageWidget;
    QWidget                   *cardsContainerWidget;
    QGridLayout               *recentGrid;
    QPushButton               *btnClearHistory;
    QWidget                   *tabsContainer;
    QPushButton               *menuBtn;
    QMenu                     *mainMenu;
    QMenu                     *settingsMenu;
    QAction                   *themeAction;
    HomeButton                *btnHome;
    QPushButton               *btnMin;
    QPushButton               *btnMax;
    QPushButton               *btnClose;
    DynamicIsland             *dynamicIsland;
    IslandToggleButton        *islandToggleBtn;
    
    bool                       m_islandVisible;
    bool                       m_isDarkMode;

    QVector<BookTab*>          m_tabs;
    QVector<QWidget*>          m_views;
    QVector<RecentCard*>       m_recentCards;
    int                        m_currentIndex;
    QPoint                     m_windowDragStart;
    bool                       m_draggingWindow;
    BookTab                   *m_draggedTab;
    int                        m_dragOffsetX;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModernPDFReader viewer;
    viewer.show();
    return app.exec();
}

// السطر السحري لحل مشكلة Qt AUTOMOC في ملفات الكود الواحد
#include "main.moc"
