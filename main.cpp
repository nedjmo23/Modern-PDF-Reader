#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QFileInfo>
#include <QTabBar>
#include <QPainter>
#include <QLabel>
#include <QPdfDocument>
#include <QPdfView>
#include <QWheelEvent>
#include <QPainterPath>

// ─────────────────────────────────────────────
// 1. زر الإغلاق الدائري للتبويبات
// ─────────────────────────────────────────────
class CleanCloseButton : public QPushButton {
    Q_OBJECT
public:
    CleanCloseButton(QWidget *parent = nullptr) : QPushButton(parent) {
        setFixedSize(16, 16);
        setCursor(Qt::PointingHandCursor);
        setStyleSheet(
            "QPushButton { background: transparent; border: none; border-radius: 8px; }"
            "QPushButton:hover { background-color: #e81123; }"
        );
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        QPushButton::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(underMouse() ? QPen(Qt::white, 2) : QPen(QColor(150, 150, 150), 1.5));
        int m = 4;
        painter.drawLine(m, m, width() - m, height() - m);
        painter.drawLine(width() - m, m, m, height() - m);
    }
};

// ─────────────────────────────────────────────
// 2. أيقونة المنزل المرسومة برمجياً
// ─────────────────────────────────────────────
class HomeIconWidget : public QWidget {
    Q_OBJECT
public:
    HomeIconWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(20, 20);
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        QPainterPath path;
        path.moveTo(2, 10);
        path.lineTo(10, 3);
        path.lineTo(18, 10);

        path.moveTo(5, 9);
        path.lineTo(5, 17);
        path.lineTo(15, 17);
        path.lineTo(15, 9);

        path.moveTo(8, 17);
        path.lineTo(8, 12);
        path.lineTo(12, 12);
        path.lineTo(12, 17);

        painter.drawPath(path);
    }
};

// ─────────────────────────────────────────────
// 3. عارض PDF مع Zoom انسيابي
// ─────────────────────────────────────────────
class ZoomablePdfView : public QPdfView {
    Q_OBJECT
public:
    ZoomablePdfView(QWidget *parent = nullptr) : QPdfView(parent) {
        setPageMode(QPdfView::PageMode::MultiPage);
        setZoomMode(QPdfView::ZoomMode::Custom);
        setZoomFactor(1.0);

        setStyleSheet(
            "QPdfView { background-color: #141414; border: none; }"
            "QAbstractScrollArea { background-color: #141414; }"
            "QScrollBar:vertical { background: #141414; width: 10px; margin: 0px; border: none; }"
            "QScrollBar::handle:vertical { background: #2d2d2d; min-height: 40px; border-radius: 5px; }"
            "QScrollBar::handle:vertical:hover { background: #444444; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { background: none; height: 0px; }"
            "QScrollBar:horizontal { background: #141414; height: 10px; margin: 0px; border: none; }"
            "QScrollBar::handle:horizontal { background: #2d2d2d; min-width: 40px; border-radius: 5px; }"
            "QScrollBar::handle:horizontal:hover { background: #444444; }"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { background: none; width: 0px; }"
        );
        // إجبار الخلفية الداكنة على viewport أيضاً
        viewport()->setStyleSheet("background-color: #141414;");
    }

protected:
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier ||
            event->source() == Qt::MouseEventSynthesizedBySystem) {

            double angle  = event->angleDelta().y();
            double factor = zoomFactor();

            if (angle > 0) factor += 0.05;
            else if (angle < 0) factor -= 0.05;

            factor = qBound(0.4, factor, 4.0);
            setZoomFactor(factor);
            event->accept();
        } else {
            QPdfView::wheelEvent(event);
        }
    }
};

// ─────────────────────────────────────────────
// 4. النافذة الرئيسية
// ─────────────────────────────────────────────
class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader() : m_dragging(false) {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        resize(1100, 800);
        setMinimumSize(700, 500);

        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window,     QColor(28, 28, 28));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base,       QColor(20, 20, 20));
        darkPalette.setColor(QPalette::Text,       Qt::white);
        setPalette(darkPalette);

        QWidget *centralWidget = new QWidget(this);
        centralWidget->setStyleSheet("background-color: #141414; border: none;");

        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // ── الشريط العلوي ────────────────────────────────────
        QWidget *topHeaderWidget = new QWidget(this);
        topHeaderWidget->setFixedHeight(45);
        topHeaderWidget->setStyleSheet("background-color: #1c1c1c; border: none;");

        QHBoxLayout *headerLayout = new QHBoxLayout(topHeaderWidget);
        headerLayout->setContentsMargins(10, 0, 10, 0);
        headerLayout->setSpacing(5);

        // قائمة النقاط الثلاث
        menuBarCustom = new QMenuBar(this);
        menuBarCustom->setStyleSheet(
            "QMenuBar { background-color: #1c1c1c; color: #ffffff; font-size: 22px; border: none; margin: 0px; padding: 0px; }"
            "QMenuBar::item { background: transparent; padding: 2px 12px; color: #ffffff; border-radius: 4px; font-weight: bold; }"
            "QMenuBar::item:selected { background-color: #2d2d2d; color: #ffffff; }"
            "QMenu { background-color: #2d2d2d; color: #ffffff; border: 1px solid #3d3d3d; padding: 5px; font-size: 14px; }"
            "QMenu::item { padding: 6px 25px; border-radius: 3px; }"
            "QMenu::item:selected { background-color: #007acc; color: white; }"
        );
        QMenu *fileMenu = menuBarCustom->addMenu("⋮");
        QAction *openAction = fileMenu->addAction("Open PDF");
        connect(openAction, &QAction::triggered, this, &ModernPDFReader::openPDF);
        headerLayout->addWidget(menuBarCustom);

        // شريط التبويبات في الهيدر (بدون setTabBar المحمية)
        headerTabBar = new QTabBar(this);
        headerTabBar->setExpanding(false);
        headerTabBar->setMovable(true);
        headerTabBar->setStyleSheet(
            "QTabBar { background: transparent; padding-left: 2px; padding-top: 5px; border: none; }"
            "QTabBar::tab { background: #252526; color: #aaaaaa; padding: 6px 18px; "
            "border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 3px; font-size: 13px; border: none; }"
            "QTabBar::tab:first-child { padding: 6px 6px; min-width: 30px; max-width: 30px; }"
            "QTabBar::tab:selected { background: #141414; color: #ffffff; font-weight: bold; border: none; }"
            "QTabBar::tab:hover:!selected { background: #2d2d2d; color: #ffffff; }"
        );
        // ربط headerTabBar بتغيير صفحة tabWidget
        connect(headerTabBar, &QTabBar::currentChanged, this, [this](int index) {
            tabWidget->setCurrentIndex(index);
        });
        headerLayout->addWidget(headerTabBar, 1);

        // أزرار الزوم
        QPushButton *btnZoomOut = new QPushButton("—");
        QPushButton *btnZoomIn  = new QPushButton("+");
        QString zoomBtnStyle =
            "QPushButton { background: transparent; color: #cccccc; border: none; font-size: 14px;"
            " font-weight: bold; width: 28px; height: 28px; border-radius: 14px; }"
            "QPushButton:hover { background-color: #2d2d2d; color: white; }";
        btnZoomOut->setStyleSheet(zoomBtnStyle);
        btnZoomIn->setStyleSheet(zoomBtnStyle);
        connect(btnZoomOut, &QPushButton::clicked, this, &ModernPDFReader::triggerZoomOut);
        connect(btnZoomIn,  &QPushButton::clicked, this, &ModernPDFReader::triggerZoomIn);

        QHBoxLayout *zoomControls = new QHBoxLayout();
        zoomControls->setSpacing(2);
        zoomControls->setContentsMargins(5, 0, 5, 0);
        zoomControls->addWidget(btnZoomOut);
        zoomControls->addWidget(btnZoomIn);
        headerLayout->addLayout(zoomControls);

        // أزرار التحكم في النافذة
        QPushButton *btnMinimize = new QPushButton("–");
        QPushButton *btnMaximize = new QPushButton("⬜");
        QPushButton *btnClose    = new QPushButton("✕");

        QString btnStyle =
            "QPushButton { background: transparent; color: #aaaaaa; border: none;"
            " font-size: 14px; width: 45px; height: 35px; }"
            "QPushButton:hover { background-color: #2d2d2d; color: white; }";
        btnMinimize->setStyleSheet(btnStyle);
        btnMaximize->setStyleSheet(btnStyle);
        btnClose->setStyleSheet(
            "QPushButton { background: transparent; color: #aaaaaa; border: none;"
            " font-size: 14px; width: 45px; height: 35px; }"
            "QPushButton:hover { background-color: #e81123; color: white; }");

        connect(btnMinimize, &QPushButton::clicked, this, &ModernPDFReader::showMinimized);
        connect(btnMaximize, &QPushButton::clicked, this,
                [this]() { isMaximized() ? showNormal() : showMaximized(); });
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);

        QHBoxLayout *windowControls = new QHBoxLayout();
        windowControls->setSpacing(0);
        windowControls->addWidget(btnMinimize);
        windowControls->addWidget(btnMaximize);
        windowControls->addWidget(btnClose);
        headerLayout->addLayout(windowControls);

        mainLayout->addWidget(topHeaderWidget);

        // ── منطقة عرض المحتوى ────────────────────────────────
        tabWidget = new QTabWidget(this);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        // إخفاء شريط التبويبات الداخلي بالكامل بدون أي مساحة
        tabWidget->tabBar()->hide();
        tabWidget->setStyleSheet(
            "QTabWidget::pane { border: none; background: #141414; margin: 0px; padding: 0px; }"
            "QTabWidget { border: none; background: #141414; }"
        );
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &ModernPDFReader::closeTab);
        mainLayout->addWidget(tabWidget, 1);

        // ── تبويب Home ───────────────────────────────────────
        QWidget *welcomeWidget = new QWidget();
        welcomeWidget->setStyleSheet("background-color: #141414; border: none;");
        QLabel *welcomeLabel = new QLabel(
            "Welcome! Click ( ⋮ ) → Open PDF to start reading.", welcomeWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet(
            "color: #666666; font-size: 16px; font-family: 'Segoe UI'; border: none;");
        auto *wLayout = new QVBoxLayout(welcomeWidget);
        wLayout->addWidget(welcomeLabel);

        // إضافة تبويب Home في tabWidget و headerTabBar معاً
        tabWidget->addTab(welcomeWidget, "");
        int homeIdx = headerTabBar->addTab("");
        tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
        headerTabBar->setTabButton(homeIdx, QTabBar::RightSide, nullptr);

        HomeIconWidget *homeIcon = new HomeIconWidget(this);
        headerTabBar->setTabButton(homeIdx, QTabBar::LeftSide, homeIcon);

        // تثبيت تبويب Home ومنع تحريكه نهائياً
        connect(headerTabBar, &QTabBar::tabMoved, this, [this](int from, int to) {
            if (from == 0 || to == 0) {
                // منع الحركة بإعادته فوراً بدون تشغيل السيغنال مرة ثانية
                headerTabBar->blockSignals(true);
                headerTabBar->moveTab(to, from);
                headerTabBar->blockSignals(false);
            }
        });

        setCentralWidget(centralWidget);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && event->position().y() < 45) {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            m_dragging = true;
            event->accept();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (m_dragging && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton)
            m_dragging = false;
    }

private slots:
    void openPDF() {
        QString filePath = QFileDialog::getOpenFileName(
            this, "Open PDF", "", "PDF Files (*.pdf)");
        if (filePath.isEmpty()) return;

        ZoomablePdfView *pdfView = new ZoomablePdfView(this);
        QPdfDocument *document   = new QPdfDocument(pdfView);

        if (document->load(filePath) == QPdfDocument::Error::None) {
            pdfView->setDocument(document);

            QFileInfo fileInfo(filePath);
            QString title = fileInfo.fileName();

            // إضافة في tabWidget و headerTabBar معاً
            int index = tabWidget->addTab(pdfView, title);
            int barIdx = headerTabBar->addTab(title);
            Q_UNUSED(barIdx);

            // زر الإغلاق في headerTabBar
            CleanCloseButton *closeBtn = new CleanCloseButton(this);
            connect(closeBtn, &QPushButton::clicked, this, [this, pdfView]() {
                int idx = tabWidget->indexOf(pdfView);
                if (idx != -1) closeTab(idx);
            });
            headerTabBar->setTabButton(index, QTabBar::RightSide, closeBtn);

            // تزامن التحديد
            headerTabBar->setCurrentIndex(index);
            tabWidget->setCurrentIndex(index);
        } else {
            delete pdfView;
        }
    }

    void closeTab(int index) {
        if (index <= 0) return;

        QWidget *w = tabWidget->widget(index);
        tabWidget->removeTab(index);
        headerTabBar->removeTab(index);
        delete w;
    }

    void triggerZoomIn() {
        if (auto *view = qobject_cast<ZoomablePdfView*>(tabWidget->currentWidget())) {
            view->setZoomFactor(qBound(0.4, view->zoomFactor() + 0.15, 4.0));
        }
    }

    void triggerZoomOut() {
        if (auto *view = qobject_cast<ZoomablePdfView*>(tabWidget->currentWidget())) {
            view->setZoomFactor(qBound(0.4, view->zoomFactor() - 0.15, 4.0));
        }
    }

private:
    QTabWidget *tabWidget;
    QTabBar    *headerTabBar;
    QMenuBar   *menuBarCustom;
    QPoint      m_dragPosition;
    bool        m_dragging;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModernPDFReader viewer;
    viewer.show();
    return app.exec();
}

#include "main.moc"
