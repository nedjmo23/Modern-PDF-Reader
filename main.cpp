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
#include <QPdfDocument>
#include <QPdfView>
#include <QWheelEvent>
#include <QPainterPath>
#include <QFrame>
#include <QScrollArea>
#include <QMenu>
#include <QAction>
#include <QVector>

// ─────────────────────────────────────────────
// 1. عارض PDF مع Zoom وخلفية داكنة
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
// 2. تبويبة كتاب واحدة (مرسومة يدوياً)
// ─────────────────────────────────────────────
class BookTab : public QWidget {
    Q_OBJECT
public:
    BookTab(const QString &title, QWidget *parent = nullptr)
        : QWidget(parent), m_title(title), m_selected(false), m_hovered(false)
    {
        setFixedHeight(34);
        setMinimumWidth(80);
        setMaximumWidth(200);
        setCursor(Qt::PointingHandCursor);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        // زر الإغلاق
        closeBtn = new QPushButton("✕", this);
        closeBtn->setFixedSize(14, 14);
        closeBtn->setStyleSheet(
            "QPushButton { background: transparent; border: none; color: #888888; font-size: 10px; }"
            "QPushButton:hover { color: #ffffff; }"
        );
        connect(closeBtn, &QPushButton::clicked, this, &BookTab::closeRequested);
    }

    void setSelected(bool s) {
        m_selected = s;
        update();
    }

    bool isSelected() const { return m_selected; }

    void resizeEvent(QResizeEvent *event) override {
        QWidget::resizeEvent(event);
        closeBtn->move(width() - 18, (height() - closeBtn->height()) / 2);
    }

signals:
    void clicked();
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // خلفية التبويبة
        QColor bg = m_selected ? QColor(30, 30, 30) : (m_hovered ? QColor(50, 50, 50) : QColor(38, 38, 38));
        QPainterPath path;
        path.addRoundedRect(2, 2, width() - 4, height() - 2, 8, 8);
        p.fillPath(path, bg);

        // خط أبيض في الأسفل عند التحديد
        if (m_selected) {
            p.setPen(QPen(Qt::white, 2));
            p.drawLine(10, height() - 1, width() - 10, height() - 1);
        }

        // النص
        p.setPen(m_selected ? Qt::white : QColor(170, 170, 170));
        QFont font = p.font();
        font.setPointSize(9);
        p.setFont(font);
        QRect textRect(8, 0, width() - 28, height());
        QString elidedText = p.fontMetrics().elidedText(m_title, Qt::ElideRight, textRect.width());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elidedText);
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton)
            emit clicked();
    }

    void enterEvent(QEnterEvent *) override { m_hovered = true;  update(); }
    void leaveEvent(QEvent *)       override { m_hovered = false; update(); }

private:
    QString      m_title;
    bool         m_selected;
    bool         m_hovered;
    QPushButton *closeBtn;
};

// ─────────────────────────────────────────────
// 3. زر المنزل المرسوم يدوياً
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
        path.moveTo(cx - 7, cy + 1); path.lineTo(cx, cy - 6); path.lineTo(cx + 7, cy + 1);
        path.moveTo(cx - 5, cy);     path.lineTo(cx - 5, cy + 7); path.lineTo(cx + 5, cy + 7); path.lineTo(cx + 5, cy);
        path.moveTo(cx - 2, cy + 7); path.lineTo(cx - 2, cy + 3); path.lineTo(cx + 2, cy + 3); path.lineTo(cx + 2, cy + 7);
        p.drawPath(path);
    }
};

// ─────────────────────────────────────────────
// 4. النافذة الرئيسية
// ─────────────────────────────────────────────
class ModernPDFReader : public QMainWindow {
    Q_OBJECT
public:
    ModernPDFReader() : m_dragging(false), m_currentIndex(-1) {
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
            "QMenu { background-color: #2d2d2d; color: #ffffff; border: 1px solid #3d3d3d; padding: 4px; font-size: 13px; }"
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

        // منطقة التبويبات (تمتد لتملأ المساحة)
        tabsContainer = new QWidget(this);
        tabsContainer->setStyleSheet("background: transparent;");
        tabsLayout = new QHBoxLayout(tabsContainer);
        tabsLayout->setContentsMargins(0, 0, 0, 0);
        tabsLayout->setSpacing(3);
        tabsLayout->addStretch();
        headerLayout->addWidget(tabsContainer, 1);

        // أزرار النافذة
        QString btnStyle =
            "QPushButton { background: transparent; color: #aaaaaa; border: none;"
            " font-size: 12px; width: 38px; height: 28px; }"
            "QPushButton:hover { background-color: #2d2d2d; color: white; }";
        QPushButton *btnMin = new QPushButton("–", this);
        QPushButton *btnMax = new QPushButton("⬜", this);
        QPushButton *btnClose = new QPushButton("✕", this);
        btnMin->setStyleSheet(btnStyle);
        btnMax->setStyleSheet(btnStyle);
        btnClose->setStyleSheet(
            "QPushButton { background: transparent; color: #aaaaaa; border: none;"
            " font-size: 12px; width: 38px; height: 28px; }"
            "QPushButton:hover { background-color: #e81123; color: white; }");
        connect(btnMin,   &QPushButton::clicked, this, &ModernPDFReader::showMinimized);
        connect(btnMax,   &QPushButton::clicked, this, [this]() { isMaximized() ? showNormal() : showMaximized(); });
        connect(btnClose, &QPushButton::clicked, this, &ModernPDFReader::close);
        headerLayout->addWidget(btnMin);
        headerLayout->addWidget(btnMax);
        headerLayout->addWidget(btnClose);

        mainLayout->addWidget(header);

        // ── منطقة العرض ──────────────────────────────────────
        stackedWidget = new QStackedWidget(this);
        stackedWidget->setStyleSheet("background-color: #141414; border: none;");

        // الصفحة الرئيسية
        homePageWidget = new QWidget();
        homePageWidget->setStyleSheet("background-color: #141414; border: none;");
        QLabel *welcomeLabel = new QLabel("Welcome!\n\nClick ( ⋮ ) → Open PDF to start reading.", homePageWidget);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setStyleSheet("color: #555555; font-size: 18px; font-family: 'Segoe UI'; border: none;");
        QVBoxLayout *homeLayout = new QVBoxLayout(homePageWidget);
        homeLayout->addWidget(welcomeLabel);

        stackedWidget->addWidget(homePageWidget); // index 0
        stackedWidget->setCurrentIndex(0);

        mainLayout->addWidget(stackedWidget, 1);
        setCentralWidget(central);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && event->position().y() < 38) {
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
        if (event->button() == Qt::LeftButton) m_dragging = false;
    }

private slots:
    void showHomePage() {
        stackedWidget->setCurrentWidget(homePageWidget);
        // إلغاء تحديد جميع التبويبات
        for (auto *tab : m_tabs) tab->setSelected(false);
        m_currentIndex = -1;
    }

    void openPDF() {
        QString filePath = QFileDialog::getOpenFileName(this, "Open PDF", "", "PDF Files (*.pdf)");
        if (filePath.isEmpty()) return;

        ZoomablePdfView *pdfView = new ZoomablePdfView(this);
        QPdfDocument *document   = new QPdfDocument(pdfView);

        if (document->load(filePath) == QPdfDocument::Error::None) {
            pdfView->setDocument(document);
            QFileInfo fileInfo(filePath);

            // إضافة في stackedWidget
            int stackIndex = stackedWidget->addWidget(pdfView);

            // إنشاء التبويبة اليدوية
            BookTab *tab = new BookTab(fileInfo.fileName(), tabsContainer);
            int tabIndex = m_tabs.size();
            m_tabs.append(tab);
            m_views.append(pdfView);

            // إدراج قبل الـ stretch
            tabsLayout->insertWidget(tabsLayout->count() - 1, tab);

            connect(tab, &BookTab::clicked, this, [this, tab]() {
                selectTab(m_tabs.indexOf(tab));
            });
            connect(tab, &BookTab::closeRequested, this, [this, tab]() {
                closeTabByWidget(tab);
            });

            selectTab(tabIndex);
            Q_UNUSED(stackIndex);
        } else {
            delete pdfView;
        }
    }

    void selectTab(int index) {
        if (index < 0 || index >= m_tabs.size()) return;
        m_currentIndex = index;
        for (int i = 0; i < m_tabs.size(); i++)
            m_tabs[i]->setSelected(i == index);
        stackedWidget->setCurrentWidget(m_views[index]);
    }

    void closeTabByWidget(BookTab *tab) {
        int index = m_tabs.indexOf(tab);
        if (index < 0) return;

        ZoomablePdfView *view = m_views[index];
        stackedWidget->removeWidget(view);
        tabsLayout->removeWidget(tab);
        m_tabs.removeAt(index);
        m_views.removeAt(index);
        delete tab;
        delete view;

        if (m_tabs.isEmpty()) {
            m_currentIndex = -1;
            stackedWidget->setCurrentWidget(homePageWidget);
        } else {
            int newIndex = qMin(index, m_tabs.size() - 1);
            selectTab(newIndex);
        }
    }

private:
    QStackedWidget       *stackedWidget;
    QWidget              *homePageWidget;
    QWidget              *tabsContainer;
    QHBoxLayout          *tabsLayout;
    QPushButton          *menuBtn;
    QVector<BookTab*>     m_tabs;
    QVector<ZoomablePdfView*> m_views;
    int                   m_currentIndex;
    QPoint                m_dragPosition;
    bool                  m_dragging;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModernPDFReader viewer;
    viewer.show();
    return app.exec();
}

#include "main.moc"
