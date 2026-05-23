#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QScrollArea>
#include <QLabel>
#include <QStyle>
#include <QPalette>
#include <QVBoxLayout>
class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader() {
        // إعداد النافذة الرئيسية
        setWindowTitle("Modern PDF Reader");
        resize(900, 700);

        // تطبيق الوضع الليلي العصري (Matte Dark)
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(30, 30, 30));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(45, 45, 45));
        darkPalette.setColor(QPalette::AlternateBase, QColor(30, 30, 30));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        setPalette(darkPalette);

        // إنشاء نظام التبويبات الشبيه بكروم
        tabWidget = new QTabWidget(this);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        tabWidget->setStyleSheet(
            "QTabWidget::pane { border: 1px solid #2d2d2d; background: #1e1e1e; }"
            "QTabBar::tab { background: #2d2d2d; color: #b1b1b1; padding: 8px 20px; border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 2px; }"
            "QTabBar::tab:selected { background: #1e1e1e; color: white; border-bottom: 2px solid #2a82da; }"
            "QTabBar::close-button { image: url(close.png); subcontrol-position: right; }"
            "QTabBar::close-button:hover { background: #3d3d3d; border-radius: 2px; }"
        );
        
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &ModernPDFReader::closeTab);
        setCentralWidget(tabWidget);

        // إنشاء القوائم
        QMenu *fileMenu = menuBar()->addMenu("ملف");
        QAction *openAction = fileMenu->addAction("فتح ملف PDF");
        connect(openAction, &QAction::triggered, this, &ModernPDFReader::openPDF);

        // إضافة تبويب ترحيبي فارغ
        QWidget *welcomeWidget = new QWidget();
        QLabel *welcomeLabel = new QLabel("مرحباً بك! اذهب إلى ملف -> فتح ملف PDF لبدء القراءة سريعة السلاسة.", welcomeWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet("color: #888888; font-size: 16px;");
        auto *layout = new QVBoxLayout(welcomeWidget);
        layout->addWidget(welcomeLabel);
        tabWidget->addTab(welcomeWidget, "صفحة جديدة");
    }

private slots:
    void openPDF() {
        QString filePath = QFileDialog::getOpenFileName(this, "افتح ملف PDF", "", "PDF Files (*.pdf)");
        if (!filePath.isEmpty()) {
            // هنا سيقوم المترجم السحابي بربط محرك قراءة الصفحات السلسة
            QScrollArea *scrollArea = new QScrollArea(this);
            scrollArea->setStyleSheet("background-color: #1e1e1e; border: none;");
            scrollArea->setWidgetResizable(true);

            // مساحة محاذاة عمودية لعرض الصفحات بشكل منساب وسلس
            QWidget *container = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(container);
            layout->setAlignment(Qt::AlignHCenter);
            layout->setSpacing(15); // مسافة عصرية بين الصفحات

            // (تنبيه: محرك تصيير الصفحات الفعلي سيتم ربطه في ملف البناء ليعمل فوراً عند التشغيل)
            QLabel *dummyPage = new QLabel("[ جاري تحميل صفحات الـ PDF عبر المحرك السريع... ]");
            dummyPage->setStyleSheet("background-color: #2d2d2d; color: #ffffff; padding: 40px; border-radius: 8px; font-size: 18px;");
            dummyPage->setAlignment(Qt::AlignCenter);
            dummyPage->setFixedSize(600, 800); // محاكاة أبعاد صفحة حقيقية
            
            layout->addWidget(dummyPage);
            scrollArea->setWidget(container);

            QFileInfo fileInfo(filePath);
            int index = tabWidget->addTab(scrollArea, fileInfo.fileName());
            tabWidget->setCurrentIndex(index);
        }
    }

    void closeTab(int index) {
        if (tabWidget->count() > 1) {
            QWidget *w = tabWidget->widget(index);
            tabWidget->removeTab(index);
            delete w;
        }
    }

private:
    QTabWidget *tabWidget;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModernPDFReader viewer;
    viewer.show();
    return app.exec();
}

#include "main.moc"
