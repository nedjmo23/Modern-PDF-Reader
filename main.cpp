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

// زر إغلاق دائري أنيق للتبويبات يظهر بوضوح دائم وثابت
class CleanCloseButton : public QPushButton {
    Q_OBJECT
public:
    CleanCloseButton(QWidget *parent = nullptr) : QPushButton(parent) {
        setFixedSize(18, 18);
        setCursor(Qt::PointingHandCursor);
        setStyleSheet(
            "QPushButton { background: transparent; border: none; border-radius: 9px; }"
            "QPushButton:hover { background-color: #e81123; }"
        );
    }
protected:
    void paintEvent(QPaintEvent *event) override {
        QPushButton::paintEvent(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(underMouse() ? QPen(Qt::white, 2) : QPen(QColor(150, 150, 150), 2));
        int m = 5;
        painter.drawLine(m, m, width() - m, height() - m);
        painter.drawLine(width() - m, m, m, height() - m);
    }
};

// فئة مخصصة لعرض ملف الـ PDF مع ميزة التكبير والتصغير باللمس والفأرة المدمجة
class ZoomablePdfView : public QPdfView {
    Q_OBJECT
public:
    ZoomablePdfView(QWidget *parent = nullptr) : QPdfView(parent) {
        setPageMode(QPdfView::PageMode::MultiPage); // عرض الصفحات بشكل متتالي وسلس
        setZoomMode(QPdfView::ZoomMode::Custom);
        setZoomFactor(1.0); // الحجم الافتراضي 100%
        
        // تحسين مظهر شريط التمرير الداخلي ليتناسق مع الوضع الليلي (بدون خطوط عشوائية)
        setStyleSheet(
            "QScrollBar:vertical { background: #141414; width: 10px; margin: 0px; border: none; }"
            "QScrollBar::handle:vertical { background: #3e3e42; min-height: 40px; border-radius: 5px; }"
            "QScrollBar::handle:vertical:hover { background: #555555; }"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { background: none; height: 0px; }"
        );
    }

protected:
    // التقاط حركة عجلة الفأرة أو حركة الإصبعين على لوحة اللمس (Pinch/Scroll) للتكبير والتصغير
    void wheelEvent(QWheelEvent *event) override {
        if (event->modifiers() & Qt::ControlModifier || event->source() == Qt::MouseEventSynthesizedBySystem) {
            double angle = event->angleDelta().y();
            double factor = zoomFactor();
            
            if (angle > 0) {
                factor += 0.1; // تكبير
            } else if (angle < 0) {
                factor -= 0.1; // تصغير
            }
            
            // حدود التكبير والتصغير
            if (factor < 0.5) factor = 0.5;
            if (factor > 4.0) factor = 4.0;
            
            setZoomFactor(factor);
            event->accept();
        } else {
            QPdfView::wheelEvent(event); // التمرير العادي للأعلى والأسفل
        }
    }
};

class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader() {
        // إزالة شريط العنوان والحدود تماماً لمنع الخطوط العشوائية وحذف CMD
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        resize(1050, 780);

        // واجهة داكنة بالكامل فخمة
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(28, 28, 28));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(20, 20, 20));
        darkPalette.setColor(QPalette::Text, Qt::white);
        setPalette(darkPalette);

        QWidget *centralWidget = new QWidget(this);
        centralWidget->setStyleSheet("background-color: #1c1c1c; border: none;");
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // شريط أدوات علوي نقي وبدون خطوط جانبية
        QWidget *customTitleBar = new QWidget(this);
        customTitleBar->setStyleSheet("background-color: #1c1c1c; border: none;");
        QHBoxLayout *titleLayout = new QHBoxLayout(customTitleBar);
        titleLayout->setContentsMargins(5, 0, 10, 0);
        titleLayout->setSpacing(0);

        menuBarCustom = new QMenuBar(this);
        menuBarCustom->setStyleSheet(
            "QMenuBar { background-color: #1c1c1c; color: #ffffff; font-size: 14px; border: none; margin: 0px; padding: 0px; }"
            "QMenuBar::item { background: transparent; padding: 8px 15px; color: #ffffff; border-radius: 4px; }"
            "QMenuBar::item:selected { background-color: #2d2d2d; color: #ffffff; }"
            "QMenu { background-color: #2d2d2d; color: #ffffff; border: 1px solid #3d3d3d; padding: 5px; }"
            "QMenu::item { padding: 6px 25px; border-radius: 3px; }"
            "QMenu::item:selected { background-color: #007acc; color: white; }"
        );
        QMenu *fileMenu = menuBarCustom->addMenu("ملف");
        QAction *openAction = fileMenu->addAction("فتح ملف PDF");
        connect(openAction, &QAction::triggered, this, &ModernPDFReader::openPDF);
        titleLayout->addWidget(menuBarCustom);

        titleLayout->addStretch();

        // أزرار التحكم بالنافذة
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
        titleLayout->addLayout(windowControls);
        mainLayout->addWidget(customTitleBar);

        // التبويبات بنمط كروم الدائري وبدون أي خط فاصل طويل في الأسفل
        tabWidget = new QTabWidget(this);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        tabWidget->setStyleSheet(
            "QTabWidget::pane { border: none; background: #141414; top: 0px; margin-top: 0px; }"
            "QTabBar { background: #1c1c1c; padding-left: 10px; padding-top: 5px; border: none; }"
            "QTabBar::tab { background: #252526; color: #969696; padding: 8px 20px; "
            "border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 4px; font-size: 13px; border: none; }"
            "QTabBar::tab:selected { background: #141414; color: #ffffff; font-weight: bold; border: none; }"
            "QTabBar::tab:hover:!selected { background: #2d2d2d; color: #ffffff; }"
        );
        
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &ModernPDFReader::closeTab);
        mainLayout->addWidget(tabWidget);
        setCentralWidget(centralWidget);

        // التبويب الترحيبي الأول النظيف
        QWidget *welcomeWidget = new QWidget();
        welcomeWidget->setStyleSheet("background-color: #141414; border: none;");
        QLabel *welcomeLabel = new QLabel("مرحباً بك! اذهب إلى ملف -> فتح ملف PDF لبدء القراءة سريعة السلاسة.", welcomeWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet("color: #666666; font-size: 15px; font-family: 'Segoe UI'; border: none;");
        auto *layout = new QVBoxLayout(welcomeWidget);
        layout->addWidget(welcomeLabel);
        
        tabWidget->addTab(welcomeWidget, "صفحة جديدة");
        tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr); // حذف زر الإغلاق من الصفحة الرئيسية
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && event->position().y() < 40) {
            m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons() & Qt::LeftButton && event->position().y() < 40) {
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }

private slots:
    void openPDF() {
        QString filePath = QFileDialog::getOpenFileName(this, "افتح ملف PDF", "", "PDF Files (*.pdf)");
        if (!filePath.isEmpty()) {
            // إنشاء مستند PDF وتحميل الملف داخله برمجياً عبر المحرك
            QPdfDocument *document = new QPdfDocument(this);
            if (document->load(filePath) == QPdfDocument::Error::None) {
                
                // تشغيل أداة العرض المخصصة والمستجيبة للتكبير بإصبعين
                ZoomablePdfView *pdfView = new ZoomablePdfView(this);
                pdfView->setDocument(document);

                QFileInfo fileInfo(filePath);
                int index = tabWidget->addTab(pdfView, fileInfo.fileName());
                
                // ربط زر الإغلاق الذكي المخصص الذي قمنا برسمه بالتبويب الجديد
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
        if (tabWidget->count() > 1) {
            QWidget *w = tabWidget->widget(index);
            tabWidget->removeTab(index);
            delete w;
            
            // إعادة ضبط أزرار الإغلاق للتبويبات المتبقية لتعمل بشكل صحيح
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
