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
#include <QPainter>

// فئة مخصصة لرسم زر إغلاق (✕) احترافي وثابت بدون الحاجة لصور خارجية
class CleanCloseButton : public QPushButton {
    Q_OBJECT
public:
    CleanCloseButton(QWidget *parent = nullptr) : QPushButton(parent) {
        setFixedSize(18, 18);
        setCursor(Qt::PointingHandCursor);
        // جعل الزر شفافاً في الحالة العادية ويتحول للأحمر عند تمرير الفأرة
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
        
        // تغيير لون علامة X (أبيض عند الوقوف عليه، ورمادي خفيف في العادي)
        if (underMouse()) {
            painter.setPen(QPen(Qt::white, 2));
        } else {
            painter.setPen(QPen(QColor(150, 150, 150), 2));
        }
        
        // رسم خطوط علامة ✕ بدقة في منتصف الزر
        int m = 5;
        painter.drawLine(m, m, width() - m, height() - m);
        painter.drawLine(width() - m, m, m, height() - m);
    }
};

class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader() {
        // إزالة شريط العنوان والحدود تماماً
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
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

        // الحاوية الرئيسية النظيفة خالصة السواد وبدون حدود
        QWidget *centralWidget = new QWidget(this);
        centralWidget->setStyleSheet("background-color: #1c1c1c; border: none;");
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // إنشاء شريط علوي مخصص نقي ومتناسق الهوامش (يمنع الخط الصغير)
        QWidget *customTitleBar = new QWidget(this);
        customTitleBar->setStyleSheet("background-color: #1c1c1c; border: none;");
        QHBoxLayout *titleLayout = new QHBoxLayout(customTitleBar);
        titleLayout->setContentsMargins(5, 0, 10, 0);
        titleLayout->setSpacing(0);

        // شريط القوائم
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

        // التبويبات - إزالة الخط الفاصل الطويل تماماً عبر ضبط الـ pane إلى 0px
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

        // إضافة التبويب الترحيبي الأول
        QWidget *welcomeWidget = new QWidget();
        welcomeWidget->setStyleSheet("background-color: #141414; border: none;");
        QLabel *welcomeLabel = new QLabel("مرحباً بك! اذهب إلى ملف -> فتح ملف PDF لبدء القراءة سريعة السلاسة.", welcomeWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet("color: #666666; font-size: 15px; font-family: 'Segoe UI'; border: none;");
        auto *layout = new QVBoxLayout(welcomeWidget);
        layout->addWidget(welcomeLabel);
        
        tabWidget->addTab(welcomeWidget, "صفحة جديدة");
        
        // إخفاء زر الإغلاق تماماً من التبويب الترحيبي الأول
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
            
            // حقن زر الإغلاق المخصص المرسوم برمجياً ليعمل ويظهر بوضوح ثبات دائم
            CleanCloseButton *closeBtn = new CleanCloseButton(this);
            connect(closeBtn, &QPushButton::clicked, this, [this, index]() { closeTab(index); });
            tabWidget->tabBar()->setTabButton(index, QTabBar::RightSide, closeBtn);

            tabWidget->setCurrentIndex(index);
        }
    }

    void closeTab(int index) {
        if (tabWidget->count() > 1) {
            QWidget *w = tabWidget->widget(index);
            tabWidget->removeTab(index);
            delete w;
            
            // تحديث مؤشرات الأزرار المتبقية لتعمل بشكل سليم بعد حذف تبويب
            for (int i = 1; i < tabWidget->count(); ++i) {
                QWidget *b = tabWidget->tabBar()->tabButton(i, QTabBar::RightSide);
                if (b) {
                    b->disconnect();
                    connect(static_cast<CleanCloseButton*>(b), &QPushButton::clicked, this, [this, i]() { closeTab(i); });
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
