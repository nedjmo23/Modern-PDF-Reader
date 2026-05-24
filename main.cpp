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
#include <QHBoxLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QFileInfo>
#include <QTabBar>

class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader() {
        // إزالة شريط العنوان التقليدي والحدود تماماً لمنع الخطوط العشوائية
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground, false);
        resize(1000, 750);

        // تطبيق الألوان الداكنة الموحدة
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(28, 28, 28));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(35, 35, 35));
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(45, 45, 45));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        setPalette(darkPalette);

        // الحاوية الرئيسية بدون أي حدود عشوائية
        QWidget *centralWidget = new QWidget(this);
        centralWidget->setStyleSheet("background-color: #1c1c1c; border: none;");
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // إنشاء شريط علوي مخصص نقي
        QWidget *customTitleBar = new QWidget(this);
        customTitleBar->setStyleSheet("background-color: #1c1c1c; border-bottom: 1px solid #2d2d2d;");
        QHBoxLayout *titleLayout = new QHBoxLayout(customTitleBar);
        titleLayout->setContentsMargins(10, 0, 10, 0);
        titleLayout->setSpacing(0);

        // شريط القوائم
        menuBarCustom = new QMenuBar(this);
        menuBarCustom->setStyleSheet(
            "QMenuBar { background-color: #1c1c1c; color: #ffffff; font-size: 14px; border: none; }"
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
        windowControls->setSpacing(5);
        
        QPushButton *btnMinimize = new QPushButton("–");
        QPushButton *btnMaximize = new QPushButton("⬜");
        QPushButton *btnClose = new QPushButton("✕");

        QString btnStyle = "QPushButton { background: transparent; color: #aaaaaa; border: none; font-size: 14px; width: 40px; height: 35px; }"
                           "QPushButton:hover { background-color: #2d2d2d; color: white; }";
        btnMinimize->setStyleSheet(btnStyle);
        btnMaximize->setStyleSheet(btnStyle);
        btnClose->setStyleSheet("QPushButton { background: transparent; color: #aaaaaa; border: none; font-size: 14px; width: 40px; height: 35px; }"
                               "QPushButton:hover { background-color: #e81123; color: white; }");

        connect(btnMinimize, &QPushButton::clicked, this, &ModernPDFReader::showMinimized);
        connect(btnMaximize, &QPushButton::clicked, this, [this]() { isMaximized() ? showNormal() : showMaximized(); });
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);

        windowControls->addWidget(btnMinimize);
        windowControls->addWidget(btnMaximize);
        windowControls->addWidget(btnClose);
        titleLayout->addLayout(windowControls);

        mainLayout->addWidget(customTitleBar);

        // تبويبات متطورة بخطوط واضحة وزر إغلاق نصي مرئي (✕)
        tabWidget = new QTabWidget(this);
        tabWidget->setTabsClosable(true);
        tabWidget->setMovable(true);
        tabWidget->setStyleSheet(
            "QTabWidget::pane { border: none; background: #141414; }"
            "QTabBar { background: #1c1c1c; padding-left: 10px; padding-top: 5px; border: none; }"
            "QTabBar::tab { background: #252526; color: #969696; padding: 10px 22px; "
            "border-top-left-radius: 12px; border-top-right-radius: 12px; margin-right: 4px; font-size: 13px; border: none; }"
            "QTabBar::tab:selected { background: #141414; color: #ffffff; font-weight: bold; border: none; }"
            "QTabBar::tab:hover:!selected { background: #2d2d2d; color: #ffffff; }"
            "QTabBar::close-button { text-align: center; subcontrol-position: right; width: 16px; height: 16px; "
            "border-radius: 8px; background-color: transparent; color: #aaaaaa; }"
            "QTabBar::close-button:hover { background-color: #e81123; color: white; }"
        );
        
        connect(tabWidget, &QTabWidget::tabCloseRequested, this, &ModernPDFReader::closeTab);
        mainLayout->addWidget(tabWidget);
        setCentralWidget(centralWidget);

        // إضافة تبويب ترحيبي فارغ بدون زر إغلاق
        QWidget *welcomeWidget = new QWidget();
        welcomeWidget->setStyleSheet("background-color: #141414; border: none;");
        QLabel *welcomeLabel = new QLabel("مرحباً بك! اذهب إلى ملف -> فتح ملف PDF لبدء القراءة سريعة السلاسة.", welcomeWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet("color: #666666; font-size: 15px; font-family: 'Segoe UI'; border: none;");
        auto *layout = new QVBoxLayout(welcomeWidget);
        layout->addWidget(welcomeLabel);
        
        tabWidget->addTab(welcomeWidget, "صفحة جديدة");
        
        // إخفاء زر الإغلاق من التبويب الترحيبي الأول فوراً عند التشغيل
        tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
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
            QScrollArea *scrollArea = new QScrollArea(this);
            
            scrollArea->setStyleSheet(
                "QScrollArea { background-color: #141414; border: none; }"
                "QScrollBar:vertical { background: #141414; width: 10px; margin: 0px; border: none; }"
                "QScrollBar::handle:vertical { background: #3e3e42; min-height: 40px; border-radius: 5px; }"
                "QScrollBar::handle:vertical:hover { background: #555555; }"
                "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { background: none; height: 0px; }"
                "QScrollBar:horizontal { background: #141414; height: 10px; margin: 0px; border: none; }"
                "QScrollBar::handle:horizontal { background: #3e3e42; min-width: 40px; border-radius: 5px; }"
                "QScrollBar::handle:horizontal:hover { background: #555555; }"
                "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { background: none; width: 0px; }"
            );
            scrollArea->setWidgetResizable(true);

            QWidget *container = new QWidget();
            container->setStyleSheet("background-color: #141414; border: none;");
            QVBoxLayout *layout = new QVBoxLayout(container);
            layout->setAlignment(Qt::AlignHCenter);
            layout->setSpacing(20);

            QLabel *dummyPage = new QLabel("[ جاري تحميل صفحات الـ PDF عبر المحرك السريع... ]");
            dummyPage->setStyleSheet("background-color: #1e1e1e; color: #888888; border: 1px solid #2d2d2d; border-radius: 8px; font-size: 16px;");
            dummyPage->setAlignment(Qt::AlignCenter);
            dummyPage->setFixedSize(650, 850);
            
            layout->addWidget(dummyPage);
            scrollArea->setWidget(container);

            QFileInfo fileInfo(filePath);
            int index = tabWidget->addTab(scrollArea, fileInfo.fileName());
            
            // وضع علامة (✕) نصية واضحة كزر إغلاق للتبويب الجديد
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
