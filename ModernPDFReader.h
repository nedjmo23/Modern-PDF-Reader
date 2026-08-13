#pragma once

#include "Common.h"
#include "IslandToggleButton.h"
#include "DynamicIsland.h"
#include "BookTab.h"
#include "RecentCard.h"
#include "HomeButton.h"

class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader()
        : m_draggingWindow(false), m_currentIndex(-1),
          m_draggedTab(nullptr), m_dragOffsetX(0), m_islandVisible(false), m_currentTheme(ThemeDark)
    {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        resize(1100, 800);
        setMinimumSize(800, 600);
        restoreWindowState();

        // تفعيل استجابة شريط المهام للتصغير في ويندوز
        HWND hwnd = reinterpret_cast<HWND>(this->winId());
        DWORD style = GetWindowLong(hwnd, GWL_STYLE);
        SetWindowLong(hwnd, GWL_STYLE, style | WS_MINIMIZEBOX);
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        
        // استعادة الثيم المحفوظ مسبقاً عند الفتح
        QSettings settings("ModernPDFReader", "Settings");
        int savedTheme = settings.value("currentTheme", ThemeDark).toInt();
        m_currentTheme = static_cast<ReadingTheme>(savedTheme);
        
        // إعداد أنيميشن ظهور النافذة بسلاسة
        m_windowAnim = new QPropertyAnimation(this, "windowOpacity", this);
        m_windowAnim->setDuration(300); // مدة الأنيميشن بالميلي ثانية
        m_windowAnim->setStartValue(0.0);
        m_windowAnim->setEndValue(1.0);
        m_windowAnim->setEasingCurve(QEasingCurve::InOutQuad);
        m_windowAnim->start();

        centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // ── الشريط العلوي ─────────────────────────────────────
        header = new QWidget(this);
        header->setFixedHeight(32);
        QHBoxLayout *headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(6, 1, 4, 1);
        headerLayout->setSpacing(4);

        menuBtn = new QPushButton("⋮", this);
        menuBtn->setFixedSize(28, 28);
        menuBtn->setCursor(Qt::PointingHandCursor);

        mainMenu = new QMenu(this);
        QAction *openAction = mainMenu->addAction("📁 Open PDF");
        connect(openAction, &QAction::triggered, this, &ModernPDFReader::openPDFFileDialog);

        themeMenu = mainMenu->addMenu("🎨 Themes");
        
        QAction *actLight = themeMenu->addAction("☀️ Light Theme");
        QAction *actDark  = themeMenu->addAction("🌙 Dark Theme");
        QAction *actSepia = themeMenu->addAction("📜 Sepia Theme (Classic)");
        QAction *actNord  = themeMenu->addAction("🌲 Nord Theme (Calm)");

        connect(actLight, &QAction::triggered, this, [this]() { setReadingTheme(ThemeLight); });
        connect(actDark,  &QAction::triggered, this, [this]() { setReadingTheme(ThemeDark); });
        connect(actSepia, &QAction::triggered, this, [this]() { setReadingTheme(ThemeSepia); });
        connect(actNord,  &QAction::triggered, this, [this]() { setReadingTheme(ThemeNord); });

        settingsMenu = mainMenu->addMenu("⚙️ Settings");
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

        btnCloseUnpinned = new QPushButton("🧹", this);
        btnCloseUnpinned->setFixedSize(28, 28);
        btnCloseUnpinned->setToolTip("Close All Unpinned Tabs");
        btnCloseUnpinned->setCursor(Qt::PointingHandCursor);
        connect(btnCloseUnpinned, &QPushButton::clicked, this, &ModernPDFReader::closeAllUnpinnedTabs);
        headerLayout->addWidget(btnCloseUnpinned);

        QFont iconFont("Segoe MDL2 Assets", 9);

        btnMin = new QPushButton(this);
        btnMin->setFont(iconFont);
        btnMin->setText(QString::fromUtf8("\uE921"));

        btnMax = new QPushButton(this);
        btnMax->setFont(iconFont);
        btnMax->setText(QString::fromUtf8("\uE922"));

        btnClose = new QPushButton(this);
        btnClose->setFont(iconFont);
        btnClose->setText(QString::fromUtf8("\uE8BB"));
        
        connect(btnMin, &QPushButton::clicked, this, [this]() {
            QPropertyAnimation *minAnim = new QPropertyAnimation(this, "windowOpacity", this);
            minAnim->setDuration(150);
            minAnim->setStartValue(1.0);
            minAnim->setEndValue(0.0);
            minAnim->setEasingCurve(QEasingCurve::InOutQuad);
            connect(minAnim, &QPropertyAnimation::finished, this, [this]() {
                showMinimized();
                setWindowOpacity(1.0);
            });
            minAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
        connect(btnMax, &QPushButton::clicked, this, &ModernPDFReader::toggleMaximizedAnimated);
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);
        
        headerLayout->addWidget(btnMin);
        headerLayout->addWidget(btnMax);
        headerLayout->addWidget(btnClose);

        mainLayout->addWidget(header);

        // ── منطقة العرض ──
        stackedWidget = new QStackedWidget(this);
        setupHomePage();
        stackedWidget->addWidget(homePageWidget);
        stackedWidget->setCurrentIndex(0);
        mainLayout->addWidget(stackedWidget, 1);
        setCentralWidget(centralWidget);

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
        recentTitle = new QLabel("Recent Books", homePageWidget);
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
        if (!dir.isEmpty()) settings.setValue("defaultDir", dir);
    }

    void setReadingTheme(ReadingTheme theme) {
        m_currentTheme = theme;

        // حفظ الثيم المختار حالياً ليبقى عند إعادة الفتح
        QSettings settings("ModernPDFReader", "Settings");
        settings.setValue("currentTheme", m_currentTheme);
        
        applyTheme();
    }

    void applyTheme() {
        QString bgMain, bgHeader, textColor;
        switch (m_currentTheme) {
            case ThemeLight: bgMain = "#f3f3f3"; bgHeader = "#e5e5e5"; textColor = "#222222"; break;
            case ThemeDark:  bgMain = "#141414"; bgHeader = "#1c1c1c"; textColor = "#ffffff"; break;
            case ThemeSepia: bgMain = "#fbf0d9"; bgHeader = "#eddcb9"; textColor = "#433222"; break;
            case ThemeNord:  bgMain = "#2e3440"; bgHeader = "#3b4252"; textColor = "#eceff4"; break;
        }

        centralWidget->setStyleSheet(QString("background-color: %1; border: none;").arg(bgMain));
        header->setStyleSheet(QString("background-color: %1; border: none;").arg(bgHeader));

        menuBtn->setStyleSheet(QString(
            "QPushButton { background: transparent; border: none; color: %1; font-size: 18px; font-weight: bold; border-radius: 4px; }"
            "QPushButton:hover { background-color: rgba(150, 150, 150, 0.2); }"
        ).arg(textColor));

        mainMenu->setStyleSheet(
            "QMenu { background-color: #2d2d2d; color: #ffffff; border: 1px solid #3d3d3d; padding: 4px; font-size: 13px; }"
            "QMenu::item { padding: 5px 20px; border-radius: 3px; }"
            "QMenu::item:selected { background-color: #007acc; }"
        );

        btnCloseUnpinned->setStyleSheet(QString(
            "QPushButton { background: transparent; color: %1; border: none; font-size: 14px; border-radius: 4px; }"
            "QPushButton:hover { background-color: rgba(150, 150, 150, 0.2); }"
        ).arg(textColor));

        QString btnStyle = QString("QPushButton { background: transparent; color: %1; border: none; font-size: 12px; width: 38px; height: 28px; }"
                           "QPushButton:hover { background-color: rgba(150, 150, 150, 0.2); }").arg(textColor);
        btnMin->setStyleSheet(btnStyle);
        btnMax->setStyleSheet(btnStyle);
        btnClose->setStyleSheet("QPushButton { background: transparent; color: #aaaaaa; border: none; font-size: 12px; width: 38px; height: 28px; }"
                               "QPushButton:hover { background-color: #e81123; color: white; }");

        recentTitle->setStyleSheet(QString("color: %1;").arg(textColor));
        btnClearHistory->setStyleSheet(
            "QPushButton { background-color: rgba(150, 150, 150, 0.15); color: #cccccc; border: 1px solid rgba(150, 150, 150, 0.3); border-radius: 6px; font-size: 11px; }"
            "QPushButton:hover { background-color: #e81123; color: white; border-color: #e81123; }"
        );

        btnHome->setTheme(m_currentTheme);
        dynamicIsland->updateThemeStyle(m_currentTheme);
        islandToggleBtn->updateTheme(m_currentTheme);

        for (auto *tab : m_tabs) tab->setTheme(m_currentTheme);
        for (auto *card : m_recentCards) card->updateTheme(m_currentTheme);
    }

    void loadRecentHistory() {
        QSettings settings("ModernPDFReader", "History");
        QStringList recentFiles = settings.value("recentFiles").toStringList();

        for (auto *card : m_recentCards) {
            recentGrid->removeWidget(card);
            card->deleteLater();
        }
        m_recentCards.clear();

        int row = 0, col = 0;
        for (const QString &path : recentFiles) {
            if (!QFile::exists(path)) continue;

            QString timeStr = settings.value("time_" + path, "Recently").toString();
            RecentCard *card = new RecentCard(path, timeStr, cardsContainerWidget);
            card->updateTheme(m_currentTheme);

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
        label->setStyleSheet("font-size: 20px;");
        layout->addWidget(label);

        stackedWidget->addWidget(viewContainer);

        BookTab *tab = new BookTab(QFileInfo(filePath).fileName(), filePath, tabsContainer);
        tab->setTheme(m_currentTheme);
        tab->show();

        m_tabs.append(tab);
        m_views.append(viewContainer);

        connect(tab, &BookTab::clicked, this, [this, tab]() { selectTab(m_tabs.indexOf(tab)); });
        connect(tab, &BookTab::closeRequested, this, [this, tab]() { closeTabByWidget(tab); });
        connect(tab, &BookTab::pinToggled, this, &ModernPDFReader::togglePinTab);
        connect(tab, &BookTab::closeUnpinnedRequested, this, &ModernPDFReader::closeAllUnpinnedTabs);

        connect(tab, &BookTab::dragStarted, this, &ModernPDFReader::onDragStarted);
        connect(tab, &BookTab::dragMoved, this, &ModernPDFReader::onDragMoved);
        connect(tab, &BookTab::dragEnded, this, &ModernPDFReader::onDragEnded);

        repositionTabs(false);
        selectTab(m_tabs.size() - 1);
    }

    void togglePinTab(BookTab *tab) {
        tab->setPinned(!tab->isPinned());
        
        // إعادة ترتيب التبويبات بحيث تكون المثبتة في اليسار
        m_tabs.removeAll(tab);
        int insertIndex = 0;
        if (tab->isPinned()) {
            for (int i = 0; i < m_tabs.size(); ++i) {
                if (m_tabs[i]->isPinned()) insertIndex++;
            }
        } else {
            insertIndex = m_tabs.size();
        }
        m_tabs.insert(insertIndex, tab);

        repositionTabs(true);
    }

    void closeAllUnpinnedTabs() {
        QVector<BookTab*> tabsToClose;
        for (auto *tab : m_tabs) {
            if (!tab->isPinned()) tabsToClose.append(tab);
        }

        for (auto *tab : tabsToClose) {
            closeTabByWidget(tab);
        }
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

        if (newIndex != oldIndex && !m_tabs[newIndex]->isPinned()) {
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

    // دالة إغلاق النافذة مع أنيميشن تلاشي سلس
    void closeEvent(QCloseEvent *event) override {
        if (windowOpacity() > 0.0) {
            event->ignore(); // منع الإغلاق الفوري المؤقت
            QPropertyAnimation *closeAnim = new QPropertyAnimation(this, "windowOpacity", this);
            closeAnim->setDuration(250); // مدة التلاشي (ربع ثانية)
            closeAnim->setStartValue(1.0);
            closeAnim->setEndValue(0.0);
            closeAnim->setEasingCurve(QEasingCurve::InOutQuad);
            
            // عند انتهاء الأنيميشن، قم بإغلاق البرنامج فعلياً
            connect(closeAnim, &QPropertyAnimation::finished, this, &QWidget::close);
            closeAnim->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            QMainWindow::closeEvent(event);
        }
    }

    // دالة مراقبة تغيير حالة النافذة (مثل التصغير لشريط المهام) مع أنيميشن سلس
    void changeEvent(QEvent *event) override {
        if (event->type() == QEvent::WindowStateChange) {
            if (isMinimized()) {
                // إذا أراد المستخدم تصغير النافذة، نقوم بتخفيض الشفافية بسلاسة أولاً
                QPropertyAnimation *minAnim = new QPropertyAnimation(this, "windowOpacity", this);
                minAnim->setDuration(150);
                minAnim->setStartValue(1.0);
                minAnim->setEndValue(0.0);
                minAnim->setEasingCurve(QEasingCurve::InOutQuad);
                minAnim->start(QAbstractAnimation::DeleteWhenStopped);
            } else if (!isHidden() && windowOpacity() < 1.0) {
                // عند العودة من شريط المهام (استعادة النافذة)، نعيد الشفافية بسلاسة
                QPropertyAnimation *restoreAnim = new QPropertyAnimation(this, "windowOpacity", this);
                restoreAnim->setDuration(200);
                restoreAnim->setStartValue(0.0);
                restoreAnim->setEndValue(1.0);
                restoreAnim->setEasingCurve(QEasingCurve::InOutQuad);
                restoreAnim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
        QWidget::changeEvent(event);
    }

    void saveWindowState() {
    QSettings settings("ModernPDFReader", "WindowState");
    settings.setValue("geometry", isMaximized() ? m_normalGeometry : geometry());
    settings.setValue("maximized", isMaximized());
}

void restoreWindowState() {
    QSettings settings("ModernPDFReader", "WindowState");
    QRect savedGeometry = settings.value("geometry", QRect(100, 100, 1100, 800)).toRect();
    bool savedMaximized = settings.value("maximized", false).toBool();

    setGeometry(savedGeometry);
    m_normalGeometry = savedGeometry;

    if (savedMaximized) {
        showMaximized();
    }
}

    void toggleMaximizedAnimated() {
        QRect targetGeometry;
        QRect startGeometry = this->geometry();

        if (isMaximized()) {
            // إذا كانت مكبرة، سنحدد الحجم العادي (مثلاً 1100x800) ونظهرها بشكل طبيعي أولاً
            showNormal();
            targetGeometry = m_normalGeometry;
            
            // أنيميشن الانكماش بسلاسة للحجم العادي
            QPropertyAnimation *restoreAnim = new QPropertyAnimation(this, "geometry", this);
            restoreAnim->setDuration(250);
            restoreAnim->setStartValue(startGeometry);
            restoreAnim->setEndValue(targetGeometry);
            restoreAnim->setEasingCurve(QEasingCurve::InOutQuad);
            restoreAnim->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            // إذا كانت عادية، سنكبرها لملء الشاشة المتاحة بانيميشن سلس
            m_normalGeometry = startGeometry;
            targetGeometry = screen()->availableGeometry();
            
            QPropertyAnimation *sizeAnim = new QPropertyAnimation(this, "geometry", this);
            sizeAnim->setDuration(250);
            sizeAnim->setStartValue(startGeometry);
            sizeAnim->setEndValue(targetGeometry);
            sizeAnim->setEasingCurve(QEasingCurve::InOutQuad);
            sizeAnim->start(QAbstractAnimation::DeleteWhenStopped);
            
            connect(sizeAnim, &QPropertyAnimation::finished, this, [this]() {
                showMaximized();
            });
        }
    }

    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override {
        MSG *msg = static_cast<MSG*>(message);

        // 1. معالجة تحريك وسحب حواف النافذة لتغيير الحجم (إذا لم تكن النافذة مكبرة بالكامل)
        if (msg->message == WM_NCHITTEST && !isMaximized()) {
            short x = (short)LOWORD(msg->lParam);
            short y = (short)HIWORD(msg->lParam);
            QPoint pos = mapFromGlobal(QPoint(x, y));

            const int border = 8; // هامش حواف تغيير الحجم (8 بكسل)

            bool left   = pos.x() < border;
            bool right  = pos.x() >= width() - border;
            bool top    = pos.y() < border;
            bool bottom = pos.y() >= height() - border;

            if (top && left)     { *result = HTTOPLEFT;     return true; }
            if (top && right)    { *result = HTTOPRIGHT;    return true; }
            if (bottom && left)  { *result = HTBOTTOMLEFT;  return true; }
            if (bottom && right) { *result = HTBOTTOMRIGHT; return true; }
            if (left)            { *result = HTLEFT;        return true; }
            if (right)           { *result = HTRIGHT;       return true; }
            if (top)             { *result = HTTOP;         return true; }
            if (bottom)          { *result = HTBOTTOM;      return true; }
        }

        // 2. معالجة التلاشي والتصغير من شريط المهام
        if (msg->message == WM_SYSCOMMAND) {
            if ((msg->wParam & 0xFFF0) == SC_MINIMIZE) {
                QPropertyAnimation *minAnim = new QPropertyAnimation(this, "windowOpacity", this);
                minAnim->setDuration(150);
                minAnim->setStartValue(1.0);
                minAnim->setEndValue(0.0);
                minAnim->setEasingCurve(QEasingCurve::InOutQuad);

                connect(minAnim, &QPropertyAnimation::finished, this, [this]() {
                    showMinimized();
                    setWindowOpacity(1.0);
                });

                minAnim->start(QAbstractAnimation::DeleteWhenStopped);
                if (result) {
                    *result = 0;
                }
                return true;
            }
        }

        return QWidget::nativeEvent(eventType, message, result);
    }

private:
    QWidget                   *centralWidget;
    QWidget                   *header;
    QFrame                    *headerSep;
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
    QRect                      m_normalGeometry;

    bool                       m_wasMaximized = false;
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
