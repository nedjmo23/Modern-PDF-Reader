#pragma once

// main.cpp - Added Reading Themes, Pin Tabs & Close Unpinned Tabs
#include <QApplication>
#include <QMainWindow>
#include <QStackedWidget>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
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
#include <QSettings>
#include <QDateTime>
#include <QScrollArea>
#include <QScroller>
#include <QTimer>
#include <windows.h>

// أنماط ألوان القراءة
enum ReadingTheme { ThemeLight, ThemeDark, ThemeSepia, ThemeNord };
