#include "task2_lines.h"

#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QColorDialog>
#include <cmath>
#include <algorithm>

static const char* kBg     = "#ffffff";
static const char* kBorder = "#d0d7de";
static const char* kTxt    = "#1f2328";
static const char* kMuted  = "#6e7781";
static const char* kAccent = "#0f9d6f";
static const char* kAccent2= "#d1395c";

// ============================================================
//  LineCanvas
// ============================================================
LineCanvas::LineCanvas(QWidget* parent) : QWidget(parent) {
    setFixedSize(720, 600);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setStyleSheet("background:#ffffff; border:1px solid " + QString(kBorder) + ";");
    img = QImage(size(), QImage::Format_ARGB32);
    img.fill(Qt::white);
}

void LineCanvas::setAlgo(Algo a) { algo = a; }
void LineCanvas::setColor(const QColor& c) { color = c; }
void LineCanvas::clearAll() { img.fill(Qt::white); pending = false; update(); }

void LineCanvas::mousePressEvent(QMouseEvent* e) {
    if (!pending) {
        p0 = e->pos(); pending = true;
        emit statusChanged(QString("Точка 1: (%1,%2). Кликните 2-ю точку.").arg(p0.x()).arg(p0.y()));
    } else {
        QPoint p1 = e->pos();
        if (algo == Bresenham)
            drawBresenham(p0.x(), p0.y(), p1.x(), p1.y(), color);
        else
            drawWu(p0.x(), p0.y(), p1.x(), p1.y(), color);
        pending = false;
        emit statusChanged(QString("Отрезок: (%1,%2) → (%3,%4)")
                               .arg(p0.x()).arg(p0.y()).arg(p1.x()).arg(p1.y()));
        update();
    }
}

void LineCanvas::mouseMoveEvent(QMouseEvent* e) {
    emit coordsChanged(QString("x=%1 y=%2").arg(e->pos().x()).arg(e->pos().y()));
}

void LineCanvas::paintEvent(QPaintEvent*) {
    QPainter g(this);
    g.drawImage(0, 0, img);
    g.setPen(QPen(QColor(0,0,0,10), 1));
    for (int x = 0; x < width();  x += 40) g.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) g.drawLine(0, y, width(), y);
    if (pending) {
        g.setPen(QPen(QColor(kAccent2), 1, Qt::DashLine));
        g.drawEllipse(p0, 4, 4);
    }
}

void LineCanvas::drawBresenham(int x0, int y0, int x1, int y1, const QColor& c) {
    int dx =  qAbs(x1-x0), sx = x0 < x1 ? 1 : -1;
    int dy = -qAbs(y1-y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        if (x0 >= 0 && y0 >= 0 && x0 < img.width() && y0 < img.height())
            img.setPixelColor(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void LineCanvas::drawWu(int x0, int y0, int x1, int y1, const QColor& c) {
    auto plot = [&](int x, int y, double k) {
        if (x < 0 || y < 0 || x >= img.width() || y >= img.height()) return;
        if (k <= 0) return;
        QColor old = img.pixelColor(x, y);
        double r = c.red()  *k + old.red()  *(1-k);
        double g = c.green()*k + old.green()*(1-k);
        double b = c.blue() *k + old.blue() *(1-k);
        img.setPixelColor(x, y, QColor(int(r), int(g), int(b)));
    };

    bool steep = qAbs(y1-y0) > qAbs(x1-x0);
    if (steep) { std::swap(x0,y0); std::swap(x1,y1); }
    if (x0 > x1) { std::swap(x0,x1); std::swap(y0,y1); }

    int dx = x1 - x0;
    int dy = y1 - y0;
    double gradient = dx == 0 ? 1.0 : double(dy) / dx;

    double y = y0 + gradient;
    for (int x = x0 + 1; x < x1; ++x) {
        int   yi = int(std::floor(y));
        double f = y - yi;
        if (steep) { plot(yi, x, 1 - f); plot(yi + 1, x, f); }
        else       { plot(x, yi, 1 - f); plot(x, yi + 1, f); }
        y += gradient;
    }
    if (steep) { plot(y0, x0, 1.0); plot(y1, x1, 1.0); }
    else       { plot(x0, y0, 1.0); plot(x1, y1, 1.0); }
}

// ============================================================
//  Task2Window
// ============================================================
Task2Window::Task2Window() {
    setWindowTitle("Task 2 — Bresenham & Wu");
    setFixedSize(920, 660);
    setStyleSheet(QString("background:%1;").arg(kBg));

    auto* canvas = new LineCanvas(this);
    canvas->move(180, 20);

    auto* panel = new QWidget(this);
    panel->setGeometry(20, 20, 150, 600);
    panel->setStyleSheet(QString("background:#ffffff; border:1px solid %1;").arg(kBorder));
    auto* pl = new QVBoxLayout(panel);
    pl->setContentsMargins(8, 14, 8, 14);
    pl->setSpacing(6);

    auto* t1 = new QLabel("TASK ②");
    t1->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:14px; font-weight:bold; border:none;").arg(kTxt));
    t1->setAlignment(Qt::AlignCenter);
    auto* t2 = new QLabel("lines");
    t2->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:8px; border:none;").arg(kMuted));
    t2->setAlignment(Qt::AlignCenter);
    pl->addWidget(t1);
    pl->addWidget(t2);
    pl->addSpacing(12);

    auto mk = [&](const QString& text, const QColor& accent) {
        auto* b = new QPushButton(text);
        b->setCursor(Qt::PointingHandCursor);
        b->setFixedHeight(38);
        b->setStyleSheet(QString(
                             "QPushButton { background:#ffffff; color:%1;"
                             "  border:1px solid %2; border-left:3px solid %1;"
                             "  font-family:Consolas; font-size:11px; font-weight:bold;"
                             "  text-align:left; padding-left:12px; }"
                             "QPushButton:hover  { background:#f6f8fa; }"
                             "QPushButton:pressed{ background:#eef1f4; }"
                             ).arg(accent.name(), kBorder));
        return b;
    };

    auto* bBr  = mk("—  Брезенхем",   QColor(kAccent));
    auto* bWu  = mk("~  Ву (сглаж.)", QColor("#2f7fd1"));
    auto* bCol = mk("🎨  Цвет",       QColor(kMuted));
    auto* bClr = mk("🗑  Очистить",   QColor(kMuted));
    pl->addWidget(bBr);
    pl->addWidget(bWu);
    pl->addSpacing(10);
    pl->addWidget(bCol);
    pl->addWidget(bClr);
    pl->addStretch();

    auto* status = new QLabel("Кликните 1-ю точку отрезка");
    status->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:10px; background:transparent;").arg(kTxt));
    status->setGeometry(180, 625, 500, 18);

    auto* coords = new QLabel("x=--- y=---");
    coords->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:10px; background:transparent;").arg(kMuted));
    coords->setGeometry(700, 625, 200, 18);
    coords->setAlignment(Qt::AlignRight);

    connect(bBr,  &QPushButton::clicked, [=]{
        canvas->setAlgo(LineCanvas::Bresenham);
        status->setText("Алгоритм: Брезенхем");
    });
    connect(bWu,  &QPushButton::clicked, [=]{
        canvas->setAlgo(LineCanvas::Wu);
        status->setText("Алгоритм: Ву");
    });
    connect(bCol, &QPushButton::clicked, [=]{
        QColor c = QColorDialog::getColor(Qt::black, this);
        if (c.isValid()) canvas->setColor(c);
    });
    connect(bClr, &QPushButton::clicked, [=]{ canvas->clearAll(); });

    connect(canvas, &LineCanvas::statusChanged, status, &QLabel::setText);
    connect(canvas, &LineCanvas::coordsChanged, coords, &QLabel::setText);
}