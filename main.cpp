// v14 - Dynamic Island with Page Layout, Rotation & Night Mode
#include <QApplication>
#include <QMainWindow>
#include <QStackedWidget>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
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

// ─────────────────────────────────────────────
// 1. زر السهم لطي وإظهار الجزيرة الديناميكية
// ─────────────────────────────────────────────
class IslandToggleButton : public QPushButton {
    Q_OBJECT
public:
    IslandToggleButton(QWidget *parent = nullptr) 
        : QPushButton(parent), m_collapsed(false) 
    {
        setFixedSize(28, 12);
        setCursor(Qt::PointingHandCursor);
        setStyleSheet(
            "QPushButton { background: #252526; border: 1px solid #3d3d3d; border-top: none; "
            "border-bottom-left-radius: 6px; border-bottom-right-radius: 6px; }"
            "QPushButton:hover { background-color: #007acc; border-color: #007acc; }"
        );
    }

    void setCollapsed(bool collapsed) {
        m_collapsed = collapsed;
        update();
    }

    bool isCollapsed() const { return m_collapsed; }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPushButton::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::white, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

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
};

// ─────────────────────────────────────────────
// 2. الجزيرة الديناميكية المعلقة (Dynamic Island)
// ─────────────────────────────────────────────
class DynamicIsland : public QWidget {
    Q_OBJECT
public:
    enum ViewMode { Continuous = 0, SinglePage = 1, TwoPages = 2 };

    DynamicIsland(QWidget *parent = nullptr) 
        : QWidget(parent), m_currentViewMode(Continuous), m_rotationAngle(0), m_nightMode(false) 
    {
        setFixedHeight(32);
        setStyleSheet(
            "QWidget#IslandBody { "
            "  background-color: #222222; "
            "  border: 1px solid #383838; "
            "  border-radius: 16px; "
            "}"
            "QLabel { color: #cccccc; font-size: 11px; font-weight: bold; font-family: 'Segoe UI'; }"
            "QPushButton { background: transparent; border: none; color: #aaaaaa; font-size: 12px; font-weight: bold; border-radius: 8px; padding: 2px 5px; }"
            "QPushButton:hover { background-color: #333333; color: white; }"
        );

        setObjectName("IslandBody");

        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setContentsMargins(10, 0, 10, 0);
        layout->setSpacing(6);

        // --- قسم أزرار الصفحات العمودية ---
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

        // مؤشر الصفحات
        pageLabel = new QLabel("1 / 10", this);
        layout->addWidget(pageLabel);

        layout->addWidget(createSeparator());

        // --- قسم الزوم ---
        QPushButton *btnZoomOut = new QPushButton("—", this);
        QPushButton *btnZoomIn = new QPushButton("+", this);
        btnZoomOut->setFixedSize(20, 20);
        btnZoomIn->setFixedSize(20, 20);

        zoomLabel = new QLabel("100%", this);

        layout->addWidget(btnZoomOut);
        layout->addWidget(zoomLabel);
        layout->addWidget(btnZoomIn);

        layout->addWidget(createSeparator());

        // --- 1. زر نمط عرض الصفحات (عادي -> صفحة -> صفحتين) ---
        btnViewMode = new QPushButton("📜 Scroll", this);
        btnViewMode->setToolTip("Change Page View Mode");
        connect(btnViewMode, &QPushButton::clicked, this, &DynamicIsland::toggleViewMode);
        layout->addWidget(btnViewMode);

        // --- 2. زر التدوير بجهة واحدة (90 درجة باستمرار) ---
        btnRotate = new QPushButton("🔄", this);
        btnRotate->setFixedSize(24, 24);
        btnRotate->setToolTip("Rotate Page Clockwise (90°)");
        connect(btnRotate, &QPushButton::clicked, this, &DynamicIsland::rotateClockwise);
        layout->addWidget(btnRotate);

        // --- 3. زر القراءة الليلية ---
        btnNightMode = new QPushButton("🌙", this);
        btnNightMode->setFixedSize(24, 24);
        btnNightMode->setToolTip("Toggle Night Reading Mode");
        connect(btnNightMode, &QPushButton::clicked, this, &DynamicIsland::toggleNightMode);
        layout->addWidget(btnNightMode);

        adjustSize();
    }

    QLabel *pageLabel;
    QLabel *zoomLabel;

signals:
    void viewModeChanged(DynamicIsland::ViewMode mode);
    void rotationChanged(int angle);
    void nightModeToggled(bool enabled);

private slots:
    void toggleViewMode() {
        m_currentViewMode = static_cast<ViewMode>((m_currentViewMode + 1) % 3);
        switch (m_currentViewMode) {
            case Continuous:
                btnViewMode->setText("📜 Scroll");
                break;
            case SinglePage:
                btnViewMode->setText("📄 1-Page");
                break;
            case TwoPages:
                btnViewMode->setText("📖 2-Pages");
                break;
        }
        adjustSize();
        emit viewModeChanged(m_currentViewMode);
    }

    void rotateClockwise() {
        m_rotationAngle = (m_rotationAngle + 90) % 360;
        emit rotationChanged(m_rotationAngle);
    }

    void toggleNightMode() {
        m_nightMode = !m_nightMode;
        if (m_nightMode) {
            btnNightMode->setStyleSheet("QPushButton { background-color: #007acc; color: white; border-radius: 8px; }");
        } else {
            btnNightMode->setStyleSheet("QPushButton { background: transparent; color: #aaaaaa; border-radius: 8px; }"
                                        "QPushButton:hover { background-color: #333333; color: white; }");
        }
        emit nightModeToggled(m_nightMode);
    }

private:
    QFrame* createSeparator() {
        QFrame *sep = new QFrame(this);
        sep->setFrameShape(QFrame::VLine);
        sep->setFixedSize(1, 14);
        sep->setStyleSheet("background-color: #383838; border: none;");
        return sep;
    }

    QPushButton *btnViewMode;
    QPushButton *btnRotate;
    QPushButton *btnNightMode;

    ViewMode m_currentViewMode;
    int      m_rotationAngle;
    bool     m_nightMode;
};

// ─────────────────────────────────────────────
// 3. تبويبة كتاب مع دعم السحب الانسيابي
// ─────────────────────────────────────────────
class BookTab : public QWidget {
    Q_OBJECT
public:
    static const int TAB_WIDTH   = 180;
    static const int TAB_HEIGHT  = 30;
    static const int TAB_SPACING = 3;
    static const int TAB_Y       = 4;

    BookTab(const QString &title, QWidget *parent = nullptr)
        : QWidget(parent), m_title(title), m_selected(false),
          m_hovered(false), m_dragging(false)
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

    void setSelected(bool s) { m_selected = s; update(); }
    bool isSelected() const  { return m_selected; }

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

        QColor bg = m_selected
            ? QColor(30, 30, 30)
            : (m_hovered ? QColor(50, 50, 50) : QColor(38, 38, 38));

        QPainterPath path;
        path.addRoundedRect(1, 1, width() - 2, height() - 1, 7, 7);
        p.fillPath(path, bg);

        if (m_selected) {
            p.setPen(QPen(QColor(200, 200, 200), 2));
            p.drawLine(8, height() - 1, width() - 8, height() - 1);
        }

        p.setPen(m_selected ? Qt::white : QColor(170, 170, 170));
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
        if (m_dragging)
            emit dragMoved(this, event->globalPosition().toPoint().x());
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
    bool         m_selected;
    bool         m_hovered;
    bool         m_dragging;
    QPoint       m_pressPos;
    QPushButton *closeBtn;
};

// ─────────────────────────────────────────────
// 4. زر المنزل
// ─────────────────────────────────────────────
class HomeButton : public QPushButton {
    Q_OBJECT
public:
    HomeButton(QWidget *parent = nullptr) : QPushButton(parent) {
        setFixedSize(28, 28);
        setCursor(Qt::PointingHandCursor);
        setStyleSheet(
            "QPushButton { background: transparent; border: none; border-radius: 4px; }"
            "QPushButton:hover { background-color: #2d2d2d; }"
            "QPushButton:pressed { background-color: #007acc; }"
        );
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        QPushButton::paintEvent(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(Qt::white, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
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
};

// ─────────────────────────────────────────────
// 5. النافذة الرئيسية
// ─────────────────────────────────────────────
class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader()
        : m_draggingWindow(false), m_currentIndex(-1),
          m_draggedTab(nullptr), m_dragOffsetX(0), m_islandVisible(false)
    {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        resize(1100, 800);
        setMinimumSize(700, 500);

        QPalette pal;
        pal.setColor(QPalette::Window,     QColor(20, 20, 20));
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Base,       QColor(20, 20, 20));
        pal.setColor(QPalette::Text,       Qt::white);
        setPalette(pal);

        QWidget *central = new QWidget(this);
        central->setStyleSheet("background-color: #141414; border: none;");
        QVBoxLayout *mainLayout = new QVBoxLayout(central);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // ── الشريط العلوي ─────────────────────────────────────
        QWidget *header = new QWidget(this);
        header->setFixedHeight(38);
        header->setStyleSheet("background-color: #1c1c1c; border: none;");
        QHBoxLayout *headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(6, 4, 4, 0);
        headerLayout->setSpacing(4);

        // زر النقاط الثلاث
        menuBtn = new QPushButton("⋮", this);
        menuBtn->setFixedSize(28, 28);
        menuBtn->setCursor(Qt::PointingHandCursor);
        menuBtn->setStyleSheet(
            "QPushButton { background: transparent; border: none; color: #ffffff;"
            " font-size: 18px; font-weight: bold; border-radius: 4px; }"
            "QPushButton:hover { background-color: #2d2d2d; }"
        );
        QMenu *fileMenu = new QMenu(this);
        fileMenu->setStyleSheet(
            "QMenu { background-color: #2d2d2d; color: #ffffff; border: 1px solid #3d3d3d;"
            " padding: 4px; font-size: 13px; }"
            "QMenu::item { padding: 5px 20px; border-radius: 3px; }"
            "QMenu::item:selected { background-color: #007acc; }"
        );
        QAction *openAction = fileMenu->addAction("Open PDF");
        connect(openAction, &QAction::triggered, this, &ModernPDFReader::openPDF);
        connect(menuBtn, &QPushButton::clicked, this, [this, fileMenu]() {
            fileMenu->exec(menuBtn->mapToGlobal(QPoint(0, menuBtn->height())));
        });
        headerLayout->addWidget(menuBtn);

        // زر المنزل
        HomeButton *btnHome = new HomeButton(this);
        connect(btnHome, &QPushButton::clicked, this, &ModernPDFReader::showHomePage);
        headerLayout->addWidget(btnHome);

        // خط فاصل
        QFrame *sep = new QFrame(this);
        sep->setFrameShape(QFrame::VLine);
        sep->setFixedSize(2, 22);
        sep->setStyleSheet("background-color: #3a3a3a; border: none;");
        headerLayout->addWidget(sep);
        headerLayout->addSpacing(2);

        // منطقة التبويبات
        tabsContainer = new QWidget(this);
        tabsContainer->setStyleSheet("background: transparent;");
        tabsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        tabsContainer->setFixedHeight(38);
        headerLayout->addWidget(tabsContainer, 1);

        // أزرار النافذة
        QString btnStyle =
            "QPushButton { background: transparent; color: #aaaaaa; border: none;"
            " font-size: 12px; width: 38px; height: 28px; }"
            "QPushButton:hover { background-color: #2d2d2d; color: white; }";
        QPushButton *btnMin   = new QPushButton("–",  this);
        QPushButton *btnMax   = new QPushButton("⬜", this);
        QPushButton *btnClose = new QPushButton("✕",  this);
        btnMin->setStyleSheet(btnStyle);
        btnMax->setStyleSheet(btnStyle);
        btnClose->setStyleSheet(
            "QPushButton { background: transparent; color: #aaaaaa; border: none;"
            " font-size: 12px; width: 38px; height: 28px; }"
            "QPushButton:hover { background-color: #e81123; color: white; }");
        connect(btnMin,   &QPushButton::clicked, this, &ModernPDFReader::showMinimized);
        connect(btnMax,   &QPushButton::clicked, this,
                [this]() { isMaximized() ? showNormal() : showMaximized(); });
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);
        headerLayout->addWidget(btnMin);
        headerLayout->addWidget(btnMax);
        headerLayout->addWidget(btnClose);

        mainLayout->addWidget(header);

        // ── منطقة العرض ──────────────────────────────────────
        stackedWidget = new QStackedWidget(this);
        stackedWidget->setStyleSheet("background-color: #141414; border: none;");

        homePageWidget = new QWidget();
        homePageWidget->setStyleSheet("background-color: #141414; border: none;");
        QLabel *welcomeLabel = new QLabel(
            "Welcome!\n\nClick ( ⋮ ) → Open PDF to start reading.",
            homePageWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet(
            "color: #555555; font-size: 18px; font-family: 'Segoe UI'; border: none;");
        QVBoxLayout *homeLayout = new QVBoxLayout(homePageWidget);
        homeLayout->addWidget(welcomeLabel);

        stackedWidget->addWidget(homePageWidget);
        stackedWidget->setCurrentIndex(0);
        mainLayout->addWidget(stackedWidget, 1);
        setCentralWidget(central);

        // ── إنشاء الجزيرة الديناميكية وزر التبديل ──
        dynamicIsland = new DynamicIsland(this);
        islandToggleBtn = new IslandToggleButton(this);

        dynamicIsland->hide();
        islandToggleBtn->hide();

        connect(islandToggleBtn, &QPushButton::clicked, this, &ModernPDFReader::toggleIslandState);
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

private slots:
    void toggleIslandState() {
        bool isCollapsed = islandToggleBtn->isCollapsed();
        islandToggleBtn->setCollapsed(!isCollapsed);
        animateIslandPosition(!isCollapsed);
    }

    void showHomePage() {
        stackedWidget->setCurrentWidget(homePageWidget);
        for (auto *tab : m_tabs) tab->setSelected(false);
        m_currentIndex = -1;
        setIslandVisible(false);
    }

    void openPDF() {
        QString filePath = QFileDialog::getOpenFileName(
            this, "Open PDF", "", "PDF Files (*.pdf)");
        if (filePath.isEmpty()) return;

        QWidget *view = new QWidget(this);
        view->setStyleSheet("background-color: #141414;");
        QVBoxLayout *layout = new QVBoxLayout(view);

        QLabel *label = new QLabel("MuPDF engine will be added soon.", view);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: white; font-size: 22px;");
        layout->addWidget(label);

        stackedWidget->addWidget(view);

        BookTab *tab = new BookTab(
            QFileInfo(filePath).fileName(),
            tabsContainer
        );
        tab->show();

        m_tabs.append(tab);
        m_views.append(view);

        connect(tab, &BookTab::clicked, this, [this, tab]() {
            selectTab(m_tabs.indexOf(tab));
        });

        connect(tab, &BookTab::closeRequested, this, [this, tab]() {
            closeTabByWidget(tab);
        });

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
            if (animated)
                animateTab(m_tabs[i], i * step);
            else
                m_tabs[i]->move(i * step, BookTab::TAB_Y);
        }
    }

    // --- التحكم بالجزيرة وموضعها ---
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

private:
    QStackedWidget            *stackedWidget;
    QWidget                   *homePageWidget;
    QWidget                   *tabsContainer;
    QPushButton               *menuBtn;
    DynamicIsland             *dynamicIsland;
    IslandToggleButton        *islandToggleBtn;
    bool                       m_islandVisible;

    QVector<BookTab*>          m_tabs;
    QVector<QWidget*>          m_views;
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

#include "main.moc"
