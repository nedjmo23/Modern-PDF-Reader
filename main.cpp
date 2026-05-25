#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QStackedWidget>
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
#include <QFrame>

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
// 2. زر المنزل بنفس حجم زر النقاط الثلاث
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
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::white, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        int cx = width() / 2;
        int cy = height() / 2;

        QPainterPath path;
        path.moveTo(cx - 7, cy + 1);
        path.lineTo(cx,     cy - 6);
        path.lineTo(cx + 7, cy + 1);
        path.moveTo(cx - 5, cy);
        path.lineTo(cx - 5, cy + 7);
        path.lineTo(cx + 5, cy + 7);
        path.lineTo(cx + 5, cy);
        path.moveTo(cx - 2, cy + 7);
        path.lineTo(cx - 2, cy + 3);
        path.lineTo(cx + 2, cy + 3);
        path.lineTo(cx + 2, cy + 7);
        painter.drawPath(path);
    }
};

// ─────────────────────────────────────────────
// 3. عارض PDF مع Zoom وخلفية داكنة
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
        viewport()->setStyleSheet("background-color: #141414;");
    }

protected:
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier) {
            double factor = zoomFactor();
            factor += (event->angleDelta().y() > 0) ? 0.05 : -0.05;
            setZoomFactor(qBound(0.4, factor, 4.0));
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

        // ── الشريط العلوي (أنحف: 36px) ───────────────────────
        QWidget *header = new QWidget(this);
        header->setFixedHeight(36);
        header->setStyleSheet("background-color: #1c1c1c; border: none;");

        QHBoxLayout *headerLayout = new QHBoxLayout(header);
        headerLayout->setContentsMargins(4, 0, 4, 0);
        headerLayout->setSpacing(2);

        // زر النقاط الثلاث ( ⋮ )
        menuBarCustom = new QMenuBar(this);
        menuBarCustom->setFixedHeight(28);
        menuBarCustom->setStyleSheet(
            "QMenuBar { background: transparent; color: #ffffff; font-size: 20px; border: none; padding: 0px; margin: 0px; }"
            "QMenuBar::item { background: transparent; padding: 2px 8px; border-radius: 4px; font-weight: bold; }"
            "QMenuBar::item:selected { background-color: #2d2d2d; }"
            "QMenu { background-color: #2d2d2d; color: #ffffff; border: 1px solid #3d3d3d; padding: 4px; font-size: 13px; }"
            "QMenu::item { padding: 5px 20px; border-radius: 3px; }"
            "QMenu::item:selected { background-color: #007acc; }"
        );
        QMenu *fileMenu = menuBarCustom->addMenu("⋮");
        QAction *openAction = fileMenu->addAction("Open PDF");
        connect(openAction, &QAction::triggered, this, &ModernPDFReader::openPDF);
        headerLayout->addWidget(menuBarCustom);

        // زر المنزل بنفس حجم النقاط الثلاث
        HomeButton *btnHome = new HomeButton(this);
        connect(btnHome, &QPushButton::clicked, this, &ModernPDFReader::showHomePage);
        headerLayout->addWidget(btnHome);
        headerLayout->addSpacing(3);

        // خط فاصل داكن
        QFrame *sep = new QFrame(this);
        sep->setFrameShape(QFrame::VLine);
        sep->setFixedSize(2, 22);
        sep->setStyleSheet("background-color: #3a3a3a; border: none;");
        headerLayout->addWidget(sep);
        headerLayout->addSpacing(3);

        // تبويبات الكتب
        tabWidget = new QTabWidget(this);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        tabWidget->setStyleSheet(
            "QTabWidget::pane { border: none; background: #141414; margin: 0px; padding: 0px; }"
            "QTabWidget { border: none; }"
            "QTabBar { background: transparent; border: none; }"
            "QTabBar::tab { background: #252526; color: #aaaaaa; padding: 4px 14px; "
            "border-top-left-radius: 8px; border-top-right-radius: 8px; "
            "margin-right: 2px; font-size: 12px; border: none; }"
            "QTabBar::tab:selected { background: #141414; color: #ffffff; font-weight: bold; }"
            "QTabBar::tab:hover:!selected { background: #2d2d2d; color: #ffffff; }"
        );
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &ModernPDFReader::closeTab);
        headerLayout->addWidget(tabWidget->tabBar(), 1);

        // أزرار التحكم في النافذة
        QString btnStyle =
            "QPushButton { background: transparent; color: #aaaaaa; border: none;"
            " font-size: 13px; width: 40px; height: 28px; }"
            "QPushButton:hover { background-color: #2d2d2d; color: white; }";

        QPushButton *btnMinimize = new QPushButton("–", this);
        QPushButton *btnMaximize = new QPushButton("⬜", this);
        QPushButton *btnClose    = new QPushButton("✕", this);
        btnMinimize->setStyleSheet(btnStyle);
        btnMaximize->setStyleSheet(btnStyle);
        btnClose->setStyleSheet(
            "QPushButton { background: transparent; color: #aaaaaa; border: none;"
            " font-size: 13px; width: 40px; height: 28px; }"
            "QPushButton:hover { background-color: #e81123; color: white; }");

        connect(btnMinimize, &QPushButton::clicked, this, &ModernPDFReader::showMinimized);
        connect(btnMaximize, &QPushButton::clicked, this,
                [this]() { isMaximized() ? showNormal() : showMaximized(); });
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);

        headerLayout->addWidget(btnMinimize);
        headerLayout->addWidget(btnMaximize);
        headerLayout->addWidget(btnClose);

        mainLayout->addWidget(header);

        // ── منطقة العرض ──────────────────────────────────────
        stackedWidget = new QStackedWidget(this);
        stackedWidget->setStyleSheet("background-color: #141414; border: none;");

        // الصفحة الرئيسية (index 0)
        homePageWidget = new QWidget();
        homePageWidget->setStyleSheet("background-color: #141414; border: none;");
        QLabel *welcomeLabel = new QLabel(
            "Welcome!\n\nClick ( ⋮ ) → Open PDF to start reading.",
            homePageWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet("color: #555555; font-size: 18px; font-family: 'Segoe UI'; border: none;");
        QVBoxLayout *homeLayout = new QVBoxLayout(homePageWidget);
        homeLayout->addWidget(welcomeLabel);

        stackedWidget->addWidget(homePageWidget); // index 0
        stackedWidget->addWidget(tabWidget);       // index 1
        stackedWidget->setCurrentIndex(0);

        mainLayout->addWidget(stackedWidget, 1);
        setCentralWidget(central);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && event->position().y() < 36) {
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
    void showHomePage() {
        stackedWidget->setCurrentIndex(0);
    }

    void openPDF() {
        QString filePath = QFileDialog::getOpenFileName(
            this, "Open PDF", "", "PDF Files (*.pdf)");
        if (filePath.isEmpty()) return;

        ZoomablePdfView *pdfView = new ZoomablePdfView(tabWidget);
        QPdfDocument *document   = new QPdfDocument(pdfView);

        if (document->load(filePath) == QPdfDocument::Error::None) {
            pdfView->setDocument(document);
            QFileInfo fileInfo(filePath);
            int index = tabWidget->addTab(pdfView, fileInfo.fileName());

            CleanCloseButton *closeBtn = new CleanCloseButton(tabWidget->tabBar());
            connect(closeBtn, &QPushButton::clicked, this, [this, pdfView]() {
                int idx = tabWidget->indexOf(pdfView);
                if (idx != -1) closeTab(idx);
            });
            tabWidget->tabBar()->setTabButton(index, QTabBar::RightSide, closeBtn);

            stackedWidget->setCurrentIndex(1);
            tabWidget->setCurrentIndex(index);
        } else {
            delete pdfView;
        }
    }

    void closeTab(int index) {
        QWidget *w = tabWidget->widget(index);
        tabWidget->removeTab(index);
        delete w;
        if (tabWidget->count() == 0)
            stackedWidget->setCurrentIndex(0);
    }

private:
    QStackedWidget *stackedWidget;
    QWidget        *homePageWidget;
    QTabWidget     *tabWidget;
    QMenuBar       *menuBarCustom;
    QPoint          m_dragPosition;
    bool            m_dragging;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModernPDFReader viewer;
    viewer.show();
    return app.exec();
}

#include "main.moc"
