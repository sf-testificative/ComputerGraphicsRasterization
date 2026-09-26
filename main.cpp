#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "task1_fill.h"
#include "task2_lines.h"
#include "task3_triangle.h"

static const char* kBg     = "#ffffff";
static const char* kBorder = "#d0d7de";
static const char* kTxt    = "#1f2328";
static const char* kMuted  = "#6e7781";

static QPushButton* makeButton(const QString& text, const QColor& accent) {
    auto* b = new QPushButton(text);
    b->setCursor(Qt::PointingHandCursor);
    b->setFixedSize(360, 56);
    b->setStyleSheet(QString(
                         "QPushButton {"
                         "  background:#ffffff; color:%1;"
                         "  border:1px solid %2; border-left:4px solid %1;"
                         "  font-family:Consolas; font-size:15px; font-weight:bold;"
                         "  text-align:left; padding-left:22px;"
                         "}"
                         "QPushButton:hover  { background:#f6f8fa; }"
                         "QPushButton:pressed{ background:#eef1f4; }"
                         ).arg(accent.name(), kBorder));
    return b;
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QWidget w;
    w.setWindowTitle("Lab3");
    w.setFixedSize(440, 420);
    w.setStyleSheet(QString("background:%1;").arg(kBg));

    auto* layout = new QVBoxLayout(&w);
    layout->setContentsMargins(40, 30, 40, 30);
    layout->setSpacing(14);

    auto* b1 = makeButton("Заливка и границы",       QColor("#0f9d6f"));
    auto* b2 = makeButton("Отрезки: Брезенхем / Ву", QColor("#0f9d6f"));
    auto* b3 = makeButton("Градиентный треугольник", QColor("#0f9d6f"));
    layout->addWidget(b1);
    layout->addWidget(b2);
    layout->addWidget(b3);
    layout->addStretch();

    QObject::connect(b1, &QPushButton::clicked, [&]{
        auto* t = new Task1Window();
        t->setAttribute(Qt::WA_DeleteOnClose);
        t->show();
    });
    QObject::connect(b2, &QPushButton::clicked, [&]{
        auto* t = new Task2Window();
        t->setAttribute(Qt::WA_DeleteOnClose);
        t->show();
    });
    QObject::connect(b3, &QPushButton::clicked, [&]{
        auto* t = new Task3Window();
        t->setAttribute(Qt::WA_DeleteOnClose);
        t->show();
    });

    w.show();
    return app.exec();
}