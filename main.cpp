// main.cpp - Complete Full Code with Frameless Resizing & Position Settings
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
#include <QTimer>
#include <QCloseEvent>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <windowsx.h>
#endif

// أنماط ألوان القراءة
enum ReadingTheme { 
    ThemeLight, 
    ThemeDark, 
    ThemeSepia, 
    ThemeNord 
};

// ─────────────────────────────────────────────
// 1. زر السهم لطي وإظهار الجزيرة الديناميكية
// ─────────────────────────────────────────────
class IslandToggleButton : public QPushButton {
    Q_OBJECT

public:
    explicit IslandToggleButton(QWidget *parent = nullptr) 
        : QPushButton(parent), 
          m_collapsed(false), 
          m_theme(ThemeDark)
    {
        setFixedSize(28, 12);
        setCursor(Qt::PointingHandCursor);
        updateStyle();
    }

    void setCollapsed(bool collapsed) {
        m_collapsed = collapsed;
        updateStyle();
        update();
    }

    bool isCollapsed() const {
        return m_collapsed;
    }

    void setTheme(ReadingTheme theme) {
        m_theme = theme;
        updateStyle();
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        QColor arrowColor;
        if (m_theme == ThemeLight || m_theme == ThemeSepia) {
            arrowColor = QColor("#333333");
        } else {
            arrowColor = QColor("#FFFFFF");
        }

        p.setPen(QPen(arrowColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

        int w = width();
        int h = height();
        QPainterPath path;

        if (m_collapsed) {
            path.moveTo(w / 2 - 4, h / 2 - 2);
            path.lineTo(w / 2, h / 2 + 2);
            path.lineTo(w / 2 + 4, h / 2 - 2);
        } else {
            path.moveTo(w / 2 - 4, h / 2 + 2);
            path.lineTo(w / 2, h / 2 - 2);
            path.lineTo(w / 2 + 4, h / 2 + 2);
        }
        p.drawPath(path);
    }

private:
    void updateStyle() {
        QString bg;
        QString hoverBg;

        if (m_theme == ThemeLight || m_theme == ThemeSepia) {
            bg = "rgba(0, 0, 0, 0.08)";
            hoverBg = "rgba(0, 0, 0, 0.15)";
        } else {
            bg = "rgba(255, 255, 255, 0.12)";
            hoverBg = "rgba(255, 255, 255, 0.25)";
        }

        QString styleSheetString = QString(
            "QPushButton {"
            "   background-color: %1;"
            "   border-radius: 6px;"
            "   border: none;"
            "}"
            "QPushButton:hover {"
            "   background-color: %2;"
            "}"
        ).arg(bg, hoverBg);

        setStyleSheet(styleSheetString);
    }

    bool m_collapsed;
    ReadingTheme m_theme;
};

// ─────────────────────────────────────────────
// 2. الجزيرة الديناميكية (Dynamic Island)
// ─────────────────────────────────────────────
class DynamicIsland : public QFrame {
    Q_OBJECT

public:
    explicit DynamicIsland(QWidget *parent = nullptr) 
        : QFrame(parent), 
          m_theme(ThemeDark) 
    {
        setFixedHeight(38);
        setStyleSheet("QFrame { background-color: #1e1e2e; border-radius: 19px; }");

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(12, 0, 12, 0);
        layout->setSpacing(8);

        m_lblBookInfo = new QLabel("لم يتم فتح أي كتاب", this);
        m_lblBookInfo->setStyleSheet("color: #a6adc8; font-size: 12px; font-weight: 500;");

        m_lblPageProgress = new QLabel("الصفحة 0 / 0", this);
        m_lblPageProgress->setStyleSheet("color: #89b4fa; font-size: 11px; font-weight: bold;");

        m_btnPrev = new QPushButton("◄", this);
        m_btnNext = new QPushButton("►", this);
        m_btnZoomIn = new QPushButton("+", this);
        m_btnZoomOut = new QPushButton("-", this);

        QString btnStyle = 
            "QPushButton {"
            "   background: transparent;"
            "   color: #cdd6f4;"
            "   border: none;"
            "   font-size: 12px;"
            "   min-width: 20px;"
            "}"
            "QPushButton:hover {"
            "   color: #89b4fa;"
            "}";

        m_btnPrev->setStyleSheet(btnStyle);
        m_btnNext->setStyleSheet(btnStyle);
        m_btnZoomIn->setStyleSheet(btnStyle);
        m_btnZoomOut->setStyleSheet(btnStyle);

        layout->addWidget(m_lblBookInfo);
        layout->addStretch();
        layout->addWidget(m_btnPrev);
        layout->addWidget(m_lblPageProgress);
        layout->addWidget(m_btnNext);
        layout->addSpacing(10);
        layout->addWidget(m_btnZoomOut);
        layout->addWidget(m_btnZoomIn);
    }

    void updateInfo(const QString &title, int currentPage, int totalPages) {
        m_lblBookInfo->setText(title);
        m_lblPageProgress->setText(QString("الصفحة %1 / %2").arg(currentPage).arg(totalPages));
    }

    void setTheme(ReadingTheme theme) {
        m_theme = theme;
        switch (theme) {
            case ThemeLight:
                setStyleSheet("QFrame { background-color: #e6e9ef; border-radius: 19px; }");
                m_lblBookInfo->setStyleSheet("color: #4c4f69; font-size: 12px; font-weight: 500;");
                m_lblPageProgress->setStyleSheet("color: #1e66f5; font-size: 11px; font-weight: bold;");
                break;

            case ThemeDark:
                setStyleSheet("QFrame { background-color: #1e1e2e; border-radius: 19px; }");
                m_lblBookInfo->setStyleSheet("color: #a6adc8; font-size: 12px; font-weight: 500;");
                m_lblPageProgress->setStyleSheet("color: #89b4fa; font-size: 11px; font-weight: bold;");
                break;

            case ThemeSepia:
                setStyleSheet("QFrame { background-color: #e8dcbf; border-radius: 19px; }");
                m_lblBookInfo->setStyleSheet("color: #433422; font-size: 12px; font-weight: 500;");
                m_lblPageProgress->setStyleSheet("color: #8b5a2b; font-size: 11px; font-weight: bold;");
                break;

            case ThemeNord:
                setStyleSheet("QFrame { background-color: #3b4252; border-radius: 19px; }");
                m_lblBookInfo->setStyleSheet("color: #e5e9f0; font-size: 12px; font-weight: 500;");
                m_lblPageProgress->setStyleSheet("color: #88c0d0; font-size: 11px; font-weight: bold;");
                break;
        }
    }

private:
    QLabel *m_lblBookInfo;
    QLabel *m_lblPageProgress;
    QPushButton *m_btnPrev;
    QPushButton *m_btnNext;
    QPushButton *m_btnZoomIn;
    QPushButton *m_btnZoomOut;
    ReadingTheme m_theme;
};

// ─────────────────────────────────────────────
// 3. التبويبات (BookTab)
// ─────────────────────────────────────────────
class BookTab : public QWidget {
    Q_OBJECT

public:
    explicit BookTab(const QString &filePath, QWidget *parent = nullptr) 
        : QWidget(parent), 
          m_filePath(filePath), 
          m_isPinned(false), 
          m_isActive(false) 
    {
        QFileInfo info(filePath);
        m_title = info.fileName();

        setFixedHeight(32);
        setCursor(Qt::PointingHandCursor);

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 0, 6, 0);
        layout->setSpacing(6);

        m_lblPin = new QLabel("📌", this);
        m_lblPin->setVisible(false);
        m_lblPin->setStyleSheet("font-size: 10px;");

        m_lblTitle = new QLabel(m_title, this);
        m_lblTitle->setStyleSheet("font-size: 12px; font-weight: 500;");

        m_btnClose = new QPushButton("✕", this);
        m_btnClose->setFixedSize(16, 16);
        m_btnClose->setStyleSheet(
            "QPushButton {"
            "   background: transparent;"
            "   border: none;"
            "   color: #a6adc8;"
            "   font-size: 10px;"
            "   border-radius: 8px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #f38ba8;"
            "   color: #11111b;"
            "}"
        );

        layout->addWidget(m_lblPin);
        layout->addWidget(m_lblTitle);
        layout->addWidget(m_btnClose);

        connect(m_btnClose, &QPushButton::clicked, this, [this]() {
            emit closeRequested(this);
        });

        updateStyle();
    }

    QString filePath() const { 
        return m_filePath; 
    }

    QString title() const { 
        return m_title; 
    }

    bool isPinned() const { 
        return m_isPinned; 
    }

    void setPinned(bool pinned) {
        m_isPinned = pinned;
        m_lblPin->setVisible(pinned);
        m_btnClose->setVisible(!pinned);
        updateStyle();
    }

    void setActive(bool active) {
        m_isActive = active;
        updateStyle();
    }

signals:
    void tabClicked(BookTab *tab);
    void closeRequested(BookTab *tab);
    void pinToggled(BookTab *tab);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit tabClicked(this);
        } else if (event->button() == Qt::RightButton) {
            showContextMenu(event->globalPos());
        }
        QWidget::mousePressEvent(event);
    }

private:
    void showContextMenu(const QPoint &globalPos) {
        QMenu menu(this);
        QAction *actPin = menu.addAction(m_isPinned ? "إلغاء التثبيت" : "تثبيت التبويب 📌");
        QAction *actClose = menu.addAction("إغلاق التبويب");

        connect(actPin, &QAction::triggered, this, [this]() {
            setPinned(!m_isPinned);
            emit pinToggled(this);
        });

        connect(actClose, &QAction::triggered, this, [this]() {
            emit closeRequested(this);
        });

        menu.exec(globalPos);
    }

    void updateStyle() {
        if (m_isActive) {
            setStyleSheet(
                "QWidget {"
                "   background-color: #313244;"
                "   border-radius: 6px;"
                "   color: #cdd6f4;"
                "}"
            );
        } else {
            setStyleSheet(
                "QWidget {"
                "   background-color: #181825;"
                "   border-radius: 6px;"
                "   color: #a6adc8;"
                "}"
                "QWidget:hover {"
                "   background-color: #1e1e2e;"
                "}"
            );
        }
    }

    QString m_filePath;
    QString m_title;
    bool m_isPinned;
    bool m_isActive;
    QLabel *m_lblPin;
    QLabel *m_lblTitle;
    QPushButton *m_btnClose;
};

// ─────────────────────────────────────────────
// 4. زر الصفحة الرئيسية (HomeButton)
// ─────────────────────────────────────────────
class HomeButton : public QPushButton {
    Q_OBJECT

public:
    explicit HomeButton(QWidget *parent = nullptr) 
        : QPushButton("🏠 الرئيسية", parent) 
    {
        setFixedHeight(32);
        setCursor(Qt::PointingHandCursor);
        setStyleSheet(
            "QPushButton {"
            "   background-color: #181825;"
            "   color: #cdd6f4;"
            "   border-radius: 6px;"
            "   padding: 0 12px;"
            "   font-weight: bold;"
            "   border: none;"
            "}"
            "QPushButton:hover {"
            "   background-color: #313244;"
            "   color: #89b4fa;"
            "}"
        );
    }
};

// ─────────────────────────────────────────────
// 5. بطاقات الكتب الأخيرة (RecentCard)
// ─────────────────────────────────────────────
class RecentCard : public QFrame {
    Q_OBJECT

public:
    explicit RecentCard(const QString &filePath, QWidget *parent = nullptr)
        : QFrame(parent), 
          m_filePath(filePath)
    {
        setFixedSize(180, 140);
        setCursor(Qt::PointingHandCursor);
        setStyleSheet(
            "QFrame {"
            "   background-color: #1e1e2e;"
            "   border-radius: 12px;"
            "   border: 1px solid #313244;"
            "}"
            "QFrame:hover {"
            "   border-color: #89b4fa;"
            "   background-color: #252538;"
            "}"
        );

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(12, 12, 12, 12);

        QLabel *iconLabel = new QLabel("📑", this);
        iconLabel->setStyleSheet("font-size: 32px; border: none; background: transparent;");
        iconLabel->setAlignment(Qt::AlignCenter);

        QFileInfo info(filePath);
        QLabel *titleLabel = new QLabel(info.fileName(), this);
        titleLabel->setStyleSheet("color: #cdd6f4; font-size: 12px; font-weight: bold; border: none; background: transparent;");
        titleLabel->setWordWrap(true);
        titleLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(iconLabel);
        layout->addWidget(titleLabel);
    }

    QString filePath() const { 
        return m_filePath; 
    }

signals:
    void cardClicked(const QString &filePath);

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            emit cardClicked(m_filePath);
        }
        QFrame::mousePressEvent(event);
    }

private:
    QString m_filePath;
};

// ─────────────────────────────────────────────
// 6. النافذة الرئيسية (ModernPDFReader)
// ─────────────────────────────────────────────
class ModernPDFReader : public QMainWindow {
    Q_OBJECT

public:
    ModernPDFReader(QWidget *parent = nullptr)
        : QMainWindow(parent), 
          m_islandVisible(true), 
          m_currentTheme(ThemeDark),
          m_currentIndex(-1), 
          m_draggingWindow(false), 
          m_draggedTab(nullptr)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
        setMinimumSize(800, 500);

        loadSettings();
        initUI();
        setReadingTheme(ThemeDark);
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        saveSettings();
        QMainWindow::closeEvent(event);
    }

    bool nativeEvent(const QByteArray &eventType, void *message, qint64 *result) override {
        #if defined(Q_OS_WIN)
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_NCHITTEST) {
            if (isMaximized()) {
                return QMainWindow::nativeEvent(eventType, message, result);
            }

            int border_width = 8;
            int x = GET_X_LPARAM(msg->lParam);
            int y = GET_Y_LPARAM(msg->lParam);

            QPoint localPos = mapFromGlobal(QPoint(x, y));
            int lx = localPos.x();
            int ly = localPos.y();
            int w = width();
            int h = height();

            bool left   = lx < border_width;
            bool right  = lx >= w - border_width;
            bool top    = ly < border_width;
            bool bottom = ly >= h - border_width;

            if (left && top)      { *result = HTTOPLEFT;     return true; }
            if (right && top)     { *result = HTTOPRIGHT;    return true; }
            if (left && bottom)   { *result = HTBOTTOMLEFT;  return true; }
            if (right && bottom)  { *result = HTBOTTOMRIGHT; return true; }
            if (left)             { *result = HTLEFT;        return true; }
            if (right)            { *result = HTRIGHT;       return true; }
            if (top)              { *result = HTTOP;         return true; }
            if (bottom)           { *result = HTBOTTOM;      return true; }
        }
        #endif
        return QMainWindow::nativeEvent(eventType, message, result);
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && event->pos().y() <= 46) {
            m_draggingWindow = true;
            m_windowDragStart = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        } else {
            QMainWindow::mousePressEvent(event);
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (m_draggingWindow && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPos() - m_windowDragStart);
            event->accept();
        } else {
            QMainWindow::mouseMoveEvent(event);
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        m_draggingWindow = false;
        QMainWindow::mouseReleaseEvent(event);
    }

private:
    void saveSettings() {
        QSettings settings("ModernPDFReaderApp", "WindowSettings");
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
    }

    void loadSettings() {
        QSettings settings("ModernPDFReaderApp", "WindowSettings");
        if (settings.contains("geometry")) {
            restoreGeometry(settings.value("geometry").toByteArray());
        } else {
            resize(1100, 700);
        }
        if (settings.contains("windowState")) {
            restoreState(settings.value("windowState").toByteArray());
        }
    }

    void initUI() {
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        QWidget *titleBar = new QWidget(this);
        titleBar->setFixedHeight(46);
        QHBoxLayout *titleLayout = new QHBoxLayout(titleBar);
        titleLayout->setContentsMargins(10, 0, 10, 0);
        titleLayout->setSpacing(8);

        btnHome = new HomeButton(this);
        connect(btnHome, &QPushButton::clicked, this, &ModernPDFReader::showHomePage);

        menuBtn = new QPushButton("☰", this);
        menuBtn->setFixedSize(32, 32);
        menuBtn->setCursor(Qt::PointingHandCursor);
        setupMenus();

        tabsContainer = new QWidget(this);
        QHBoxLayout *tabsLayout = new QHBoxLayout(tabsContainer);
        tabsLayout->setContentsMargins(0, 0, 0, 0);
        tabsLayout->setSpacing(6);
        tabsLayout->addStretch();

        btnCloseUnpinned = new QPushButton("إغلاق التبويبات غير المثبتة", this);
        btnCloseUnpinned->setFixedHeight(28);
        btnCloseUnpinned->setCursor(Qt::PointingHandCursor);
        connect(btnCloseUnpinned, &QPushButton::clicked, this, &ModernPDFReader::closeUnpinnedTabs);

        dynamicIsland = new DynamicIsland(this);
        dynamicIsland->setVisible(m_islandVisible);

        islandToggleBtn = new IslandToggleButton(this);
        connect(islandToggleBtn, &QPushButton::clicked, this, &ModernPDFReader::toggleDynamicIsland);

        btnMin = new QPushButton("—", this);
        btnMax = new QPushButton("▢", this);
        btnClose = new QPushButton("✕", this);

        QString winBtnStyle = 
            "QPushButton {"
            "   background: transparent;"
            "   color: #cdd6f4;"
            "   border: none;"
            "   font-size: 12px;"
            "   width: 30px;"
            "   height: 30px;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #313244;"
            "}";

        btnMin->setStyleSheet(winBtnStyle);
        btnMax->setStyleSheet(winBtnStyle);

        btnClose->setStyleSheet(
            "QPushButton {"
            "   background: transparent;"
            "   color: #cdd6f4;"
            "   border: none;"
            "   font-size: 12px;"
            "   width: 30px;"
            "   height: 30px;"
            "   border-radius: 4px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #f38ba8;"
            "   color: #11111b;"
            "}"
        );

        connect(btnMin, &QPushButton::clicked, this, &ModernPDFReader::showMinimized);
        connect(btnMax, &QPushButton::clicked, this, [this]() {
            if (isMaximized()) {
                showNormal();
            } else {
                showMaximized();
            }
        });
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);

        titleLayout->addWidget(btnHome);
        titleLayout->addWidget(menuBtn);
        titleLayout->addWidget(tabsContainer, 1);
        titleLayout->addWidget(btnCloseUnpinned);
        titleLayout->addWidget(dynamicIsland);
        titleLayout->addWidget(islandToggleBtn);
        titleLayout->addWidget(btnMin);
        titleLayout->addWidget(btnMax);
        titleLayout->addWidget(btnClose);

        stackedWidget = new QStackedWidget(this);

        setupHomePage();
        stackedWidget->addWidget(homePageWidget);

        mainLayout->addWidget(titleBar);
        mainLayout->addWidget(stackedWidget, 1);
    }

    void setupMenus() {
        mainMenu = new QMenu(this);
        QAction *actOpen = mainMenu->addAction("فتح ملف PDF...");
        themeMenu = mainMenu->addMenu("نمط القراءة (الثيمات)");

        QAction *actLight = themeMenu->addAction("فاتح (Light)");
        QAction *actDark = themeMenu->addAction("داكن (Dark)");
        QAction *actSepia = themeMenu->addAction("سيپيا (Sepia)");
        QAction *actNord = themeMenu->addAction("نورد (Nord)");

        connect(actOpen, &QAction::triggered, this, &ModernPDFReader::openFileDialog);
        connect(actLight, &QAction::triggered, this, [this]() { setReadingTheme(ThemeLight); });
        connect(actDark, &QAction::triggered, this, [this]() { setReadingTheme(ThemeDark); });
        connect(actSepia, &QAction::triggered, this, [this]() { setReadingTheme(ThemeSepia); });
        connect(actNord, &QAction::triggered, this, [this]() { setReadingTheme(ThemeNord); });

        menuBtn->setMenu(mainMenu);
    }

    void setupHomePage() {
        homePageWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(homePageWidget);
        layout->setContentsMargins(20, 20, 20, 20);

        QHBoxLayout *headerLayout = new QHBoxLayout();
        recentTitle = new QLabel("الكتب الأخيرة", homePageWidget);
        recentTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: #cdd6f4;");

        btnClearHistory = new QPushButton("مسح السجل", homePageWidget);
        btnClearHistory->setStyleSheet(
            "QPushButton {"
            "   background-color: #313244;"
            "   color: #f38ba8;"
            "   border-radius: 6px;"
            "   padding: 6px 12px;"
            "   font-weight: bold;"
            "   border: none;"
            "}"
            "QPushButton:hover {"
            "   background-color: #f38ba8;"
            "   color: #11111b;"
            "}"
        );
        connect(btnClearHistory, &QPushButton::clicked, this, &ModernPDFReader::clearHistory);

        headerLayout->addWidget(recentTitle);
        headerLayout->addStretch();
        headerLayout->addWidget(btnClearHistory);

        QScrollArea *scrollArea = new QScrollArea(homePageWidget);
        scrollArea->setWidgetResizable(true);
        scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

        cardsContainerWidget = new QWidget(scrollArea);
        recentGrid = new QGridLayout(cardsContainerWidget);
        recentGrid->setSpacing(16);

        cardsContainerWidget->setLayout(recentGrid);
        scrollArea->setWidget(cardsContainerWidget);

        layout->addLayout(headerLayout);
        layout->addWidget(scrollArea, 1);
    }

    void showHomePage() {
        stackedWidget->setCurrentIndex(0);
        for (auto tab : m_tabs) {
            tab->setActive(false);
        }
    }

    void openFileDialog() {
        QString fileName = QFileDialog::getOpenFileName(this, "فتح كتاب PDF", "", "PDF Files (*.pdf)");
        if (!fileName.isEmpty()) {
            openPdfFile(fileName);
        }
    }

    void openPdfFile(const QString &filePath) {
        for (int i = 0; i < m_tabs.size(); ++i) {
            if (m_tabs[i]->filePath() == filePath) {
                activateTab(i);
                return;
            }
        }

        BookTab *tab = new BookTab(filePath, this);
        m_tabs.append(tab);

        QWidget *view = new QWidget(this);
        QVBoxLayout *vLayout = new QVBoxLayout(view);
        QLabel *lbl = new QLabel("محتوى ملف الـ PDF:\n" + filePath, view);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("font-size: 16px; color: #cdd6f4; font-weight: bold;");
        vLayout->addWidget(lbl);

        m_views.append(view);
        stackedWidget->addWidget(view);

        QHBoxLayout *tabsLayout = qobject_cast<QHBoxLayout*>(tabsContainer->layout());
        if (tabsLayout) {
            tabsLayout->insertWidget(tabsLayout->count() - 1, tab);
        }

        connect(tab, &BookTab::tabClicked, this, [this, tab]() {
            int idx = m_tabs.indexOf(tab);
            if (idx != -1) {
                activateTab(idx);
            }
        });

        connect(tab, &BookTab::closeRequested, this, &ModernPDFReader::closeTab);

        activateTab(m_tabs.size() - 1);
        addRecentCard(filePath);
    }

    void activateTab(int index) {
        if (index < 0 || index >= m_tabs.size()) return;
        m_currentIndex = index;
        for (int i = 0; i < m_tabs.size(); ++i) {
            m_tabs[i]->setActive(i == index);
        }
        stackedWidget->setCurrentWidget(m_views[index]);
        dynamicIsland->updateInfo(m_tabs[index]->title(), 1, 100);
    }

    void closeTab(BookTab *tab) {
        int idx = m_tabs.indexOf(tab);
        if (idx == -1) return;

        stackedWidget->removeWidget(m_views[idx]);
        delete m_views[idx];
        m_views.removeAt(idx);

        delete m_tabs[idx];
        m_tabs.removeAt(idx);

        if (m_tabs.isEmpty()) {
            showHomePage();
        } else {
            activateTab(qMin(idx, m_tabs.size() - 1));
        }
    }

    void closeUnpinnedTabs() {
        for (int i = m_tabs.size() - 1; i >= 0; --i) {
            if (!m_tabs[i]->isPinned()) {
                closeTab(m_tabs[i]);
            }
        }
    }

    void addRecentCard(const QString &filePath) {
        for (auto card : m_recentCards) {
            if (card->filePath() == filePath) return;
        }

        RecentCard *card = new RecentCard(filePath, cardsContainerWidget);
        connect(card, &RecentCard::cardClicked, this, &ModernPDFReader::openPdfFile);

        int row = m_recentCards.size() / 4;
        int col = m_recentCards.size() % 4;
        recentGrid->addWidget(card, row, col);
        m_recentCards.append(card);
    }

    void clearHistory() {
        for (auto card : m_recentCards) {
            recentGrid->removeWidget(card);
            delete card;
        }
        m_recentCards.clear();
    }

    void toggleDynamicIsland() {
        m_islandVisible = !m_islandVisible;
        dynamicIsland->setVisible(m_islandVisible);
        islandToggleBtn->setCollapsed(!m_islandVisible);
    }

    void setReadingTheme(ReadingTheme theme) {
        m_currentTheme = theme;
        islandToggleBtn->setTheme(theme);
        dynamicIsland->setTheme(theme);

        switch (theme) {
            case ThemeLight:
                setStyleSheet("QMainWindow { background-color: #eff1f5; }");
                break;
            case ThemeDark:
                setStyleSheet("QMainWindow { background-color: #11111b; }");
                break;
            case ThemeSepia:
                setStyleSheet("QMainWindow { background-color: #f4ebd9; }");
                break;
            case ThemeNord:
                setStyleSheet("QMainWindow { background-color: #2e3440; }");
                break;
        }
    }

    QStackedWidget            *stackedWidget;
    QWidget                   *homePageWidget;
    QWidget                   *cardsContainerWidget;
    QGridLayout               *recentGrid;
    QLabel                    *recentTitle;
    QPushButton               *btnClearHistory;
    QWidget                   *tabsContainer;
    QPushButton               *menuBtn;
    QMenu                     *mainMenu;
    QMenu                     *themeMenu;
    QMenu                     *settingsMenu;
    HomeButton                *btnHome;
    QPushButton               *btnCloseUnpinned;
    QPushButton               *btnMin;
    QPushButton               *btnMax;
    QPushButton               *btnClose;
    DynamicIsland             *dynamicIsland;
    IslandToggleButton        *islandToggleBtn;

    bool                       m_islandVisible;
    ReadingTheme               m_currentTheme;

    QVector<BookTab*>          m_tabs;
    QVector<QWidget*>          m_views;
    QVector<RecentCard*>       m_recentCards;
    int                        m_currentIndex;
    QPoint                     m_windowDragStart;
    bool                       m_draggingWindow;
    BookTab                   *m_draggedTab;
    int                        m_dragOffsetX;
    QPropertyAnimation        *m_windowAnim;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModernPDFReader viewer;
    viewer.show();
    return app.exec();
}

#include "main.moc"
