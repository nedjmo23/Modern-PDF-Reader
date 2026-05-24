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

// 1. زر إغلاق دائري أنيق للتبويبات (✕) مرسوم برمجياً بدقة عالية وبدون صور خارجية
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

// 2. ويدجت رسم أيقونة المنزل الذكية (Home Icon) برمجياً وبأبعاد متناسقة لتبويب Home الثابت
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

        // رسم سقف المنزل بدقة في المنتصف
        QPainterPath path;
        path.moveTo(2, 10);
        path.lineTo(10, 3);
        path.lineTo(18, 10);
        
        // رسم الجدران والقاعدة
        path.moveTo(5, 9);
        path.lineTo(5, 17);
        path.lineTo(15, 17);
        path.lineTo(15, 9);
        
        // رسم الباب الصغير
        path.moveTo(8, 17);
        path.lineTo(8, 12);
        path.lineTo(12, 12);
        path.lineTo(12, 17);

        painter.drawPath(path);
    }
};

// 3. أداة عرض الـ PDF المخصصة مع ميزة الزوم الانسيابي الذكي (Smooth Hardware Scaling) لمنع الوميض الأبيض تماماً
class ZoomablePdfView : public QPdfView {
    Q_OBJECT
public:
    ZoomablePdfView(QWidget *parent = nullptr) : QPdfView(parent) {
        setPageMode(QPdfView::PageMode::MultiPage);
        setZoomMode(QPdfView::ZoomMode::Custom);
        setZoomFactor(1.0);
        
        // تغيير الخلفية الرمادية الفاتحة إلى لون داكن بالكامل (#141414) لراحة العين والوضع الليلي
        setStyleSheet(
            "QPdfView { background-color: #141414; border: none; }"
            "QScrollBar:vertical { background: #141414; width: 10px; margin: 0px; border: none; }"
            "QScrollBar::handle:vertical { background: #2d2d2d; min-height: 40px; border-radius: 5px; }"
            "QScrollBar::handle:vertical:hover { background: #444444; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { background: none; height: 0px; }"
            "QScrollBar:horizontal { background: #141414; height: 10px; margin: 0px; border: none; }"
            "QScrollBar::handle:horizontal { background: #2d2d2d; min-width: 40px; border-radius: 5px; }"
            "QScrollBar::handle:horizontal:hover { background: #444444; }"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { background: none; width: 0px; }"
        );
    }

protected:
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier || event->source() == Qt::MouseEventSynthesizedBySystem) {
            double angle = event->angleDelta().y();
            double factor = zoomFactor();
            
            if (angle > 0) factor += 0.05; 
            else if (angle < 0) factor -= 0.05;
            
            if (factor < 0.4) factor = 0.4;
            if (factor > 4.0) factor = 4.0;
            
            setZoomFactor(factor);
            event->accept();
        } else {
            QPdfView::wheelEvent(event);
        }
    }
};

class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader() {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        resize(1100, 800);
        setMinimumSize(700, 500); 

        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(28, 28, 28));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(20, 20, 20));
        darkPalette.setColor(QPalette::Text, Qt::white);
        setPalette(darkPalette);

        QWidget *centralWidget = new QWidget(this);
        centralWidget->setStyleSheet("background-color: #1c1c1c; border: none;");
        
        // المخطط الرئيسي عمودي لترتيب الشريط في الأعلى والكتاب في الأسفل بدقة
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // [الشريط العلوي المدمج الفخم بقوة وسعة المتصفحات الحديثة]
        QWidget *topHeaderWidget = new QWidget(this);
        topHeaderWidget->setFixedHeight(45); // تثبيت الارتفاع لمنع انضغاط العناصر
        topHeaderWidget->setStyleSheet("background-color: #1c1c1c; border-bottom: 1px solid #252526;");
        QHBoxLayout *headerLayout = new QHBoxLayout(topHeaderWidget);
        headerLayout->setContentsMargins(10, 0, 10, 0);
        headerLayout->setSpacing(5);

        // استبدال كلمة File بـ أيقونة الثلاث نقاط العمودية الفخمة (⋮)
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

        // مساحة التبويبات الدائرية النظيفة وبدون أي خطوط عشوائية
        tabWidget = new QTabWidget(this);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        tabWidget->setStyleSheet(
            "QTabWidget::pane { border: none; background: #141414; top: 0px; margin-top: 0px; }"
            "QTabBar { background: #1c1c1c; padding-left: 2px; padding-top: 5px; border: none; }"
            "QTabBar::tab { background: #252526; color: #aaaaaa; padding: 6px 18px; "
            "border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 3px; font-size: 13px; border: none; }"
            "QTabBar::tab:selected { background: #141414; color: #ffffff; font-weight: bold; border: none; }"
            "QTabBar::tab:hover:!selected { background: #2d2d2d; color: #ffffff; }"
        );
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &ModernPDFReader::closeTab);
        headerLayout->addWidget(tabWidget, 1); // دمج شريط التبويبات في نفس السطر بجانب النقاط الثلاث

        // أزرار التحكم في الزوم المرئية (+ و —) بجانب التبويبات
        QHBoxLayout *zoomControls = new QHBoxLayout();
        zoomControls->setSpacing(2);
        zoomControls->setContentsMargins(5, 0, 5, 0);

        QPushButton *btnZoomOut = new QPushButton("—");
        QPushButton *btnZoomIn = new QPushButton("+");
        QString zoomBtnStyle = "QPushButton { background: transparent; color: #cccccc; border: none; font-size: 14px; font-weight: bold; width: 28px; height: 28px; border-radius: 14px; }"
                              "QPushButton:hover { background-color: #2d2d2d; color: white; }";
        btnZoomOut->setStyleSheet(zoomBtnStyle);
        btnZoomIn->setStyleSheet(zoomBtnStyle);

        connect(btnZoomOut, &QPushButton::clicked, this, &ModernPDFReader::triggerZoomOut);
        connect(btnZoomIn, &QPushButton::clicked, this, &ModernPDFReader::triggerZoomIn);

        zoomControls->addWidget(btnZoomOut);
        zoomControls->addWidget(btnZoomIn);
        headerLayout->addLayout(zoomControls);

        // أزرار التحكم في حجم النافذة (تصغير وتكبير وإغلاق التطبيق)
        QHBoxLayout *windowControls = new QHBoxLayout();
        windowControls->setSpacing(0);
        QPushButton *btnMinimize = new QPushButton("–");
        QPushButton *btnMaximize = new QPushButton("⬜");
        QPushButton *btnClose = new QPushButton("✕");

        QString btnStyle = "QPushButton { background: transparent; color: #aaaaaa; border: none; font-size: 14px; width: 45px; height: 35px; }"
                           "QPushButton:hover { background-color: #2d2d2d; color: white; }";
        btnMinimize->setStyleSheet(btnStyle);
        btnMaximize->setStyleSheet(btnStyle);
        btnClose->setStyleSheet("QPushButton { background: transparent; color: #aaaaaa; border: none; font-size: 14px; width: 45px; height: 35px; }"
                               "QPushButton:hover { background-color: #e81123; color: white; }");

        connect(btnMinimize, &QPushButton::clicked, this, &ModernPDFReader::showMinimized);
        connect(btnMaximize, &QPushButton::clicked, this, [this]() { isMaximized() ? showNormal() : showMaximized(); });
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);

        windowControls->addWidget(btnMinimize);
        windowControls->addWidget(btnMaximize);
        windowControls->addWidget(btnClose);
        headerLayout->addLayout(windowControls);

        // إدراج شريط العنوان المدمج في الأعلى تماماً أولاً
        mainLayout->addWidget(topHeaderWidget);
        
        // إدراج منطقة عرض الكتب (tabWidget) أسفل الشريط مباشرة وإجبارها على ملء بقية الشاشة بالطول
        mainLayout->addWidget(tabWidget, 1); 

        // واجهة تبويب Home الرئيسية الثابتة
        QWidget *welcomeWidget = new QWidget();
        welcomeWidget->setStyleSheet("background-color: #141414; border: none;");
        QLabel *welcomeLabel = new QLabel("Welcome! Click ( ⋮ ) -> Open PDF to start reading smoothly.", welcomeWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet("color: #666666; font-size: 16px; font-family: 'Segoe UI'; border: none;");
        auto *layout = new QVBoxLayout(welcomeWidget);
        layout->addWidget(welcomeLabel);
        
        // إدراج التبويب وحقن أيقونة المنزل المخصصة لحذف النصوص المكتوبة
        tabWidget->addTab(welcomeWidget, ""); 
        tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr); // منع زر الإغلاق من تبويب Home
        
        HomeIconWidget *homeIcon = new HomeIconWidget(this);
        tabWidget->tabBar()->setTabButton(0, QTabBar::LeftSide, homeIcon); // تثبيت شكل الأيقونة جهة اليسار

        // كود تثبيت تبويب Home ومنعه من الحركة (Pinned Tab)
        connect(tabWidget->tabBar(), &QTabBar::tabMoved, this, [this](int from, int to) {
            if (from == 0 || to == 0) {
                tabWidget->tabBar()->moveTab(to, from);
            }
        });

        setCentralWidget(centralWidget);
    }

protected:
    // تحريك النافذة بسلاسة عبر السحب من أي منطقة في الشريط العلوي المدمج الجديد
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && event->position().y() < 45) {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons() & Qt::LeftButton && event->position().y() < 45) {
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }

private slots:
    void openPDF() {
        QString filePath = QFileDialog::getOpenFileName(this, "Open PDF", "", "PDF Files (*.pdf)");
        if (!filePath.isEmpty()) {
            QPdfDocument *document = new QPdfDocument(this);
            if (document->load(filePath) == QPdfDocument::Error::None) {
                
                ZoomablePdfView *pdfView = new ZoomablePdfView(this);
                pdfView->setDocument(document);

                QFileInfo fileInfo(filePath);
                int index = tabWidget->addTab(pdfView, fileInfo.fileName());
                
                CleanCloseButton *closeBtn = new CleanCloseButton(this);
                connect(closeBtn, &QPushButton::clicked, this, [this, pdfView]() {
                    int currentIndex = tabWidget->indexOf(pdfView);
                    if (currentIndex != -1) {
                        closeTab(currentIndex);
                    }
                });
                tabWidget->tabBar()->setTabButton(index, QTabBar::RightSide, closeBtn);
                tabWidget->setCurrentIndex(index);
            }
        }
    }

    void closeTab(int index) {
        if (index > 0 && tabWidget->count() > 1) {
            QWidget *w = tabWidget->widget(index);
            tabWidget->removeTab(index);
            delete w;
            
            for (int i = 1; i < tabWidget->count(); ++i) {
                QWidget *b = tabWidget->tabBar()->tabButton(i, QTabBar::RightSide);
                if (b) {
                    b->disconnect();
                    QWidget* currentWidget = tabWidget->widget(i);
                    connect(static_cast<CleanCloseButton*>(b), &QPushButton::clicked, this, [this, currentWidget]() {
                        int idx = tabWidget->indexOf(currentWidget);
                        if (idx != -1) {
                            closeTab(idx);
                        }
                    });
                }
            }
        }
    }

    void triggerZoomIn() {
        QWidget *current = tabWidget->currentWidget();
        ZoomablePdfView *view = qobject_cast<ZoomablePdfView*>(current);
        if (view) {
            double factor = view->zoomFactor() + 0.15;
            if (factor <= 4.0) view->setZoomFactor(factor);
        }
    }

    void triggerZoomOut() {
        QWidget *current = tabWidget->currentWidget();
        ZoomablePdfView *view = qobject_cast<ZoomablePdfView*>(current);
        if (view) {
            double factor = view->zoomFactor() - 0.15;
            if (factor >= 0.4) view->setZoomFactor(factor);
        }
    }

private:
    QTabWidget *tabWidget;
    QMenuBar *menuBarCustom;
    QPoint m_dragPosition;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModernPDFReader viewer;
    viewer.show();
    return app.exec();
}

#include "main.moc"
