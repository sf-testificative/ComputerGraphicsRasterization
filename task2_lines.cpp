#include "task2_lines.h"

#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QColorDialog>
#include <cmath>
#include <algorithm>

static const char* kBg = "#ffffff";
static const char* kBorder = "#d0d7de";

LineCanvas::LineCanvas(QWidget* parent) : QWidget(parent)
{
    setFixedSize(760, 600);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setStyleSheet("background:#ffffff; border:1px solid " + QString(kBorder) + ";");
    img = QImage(size(), QImage::Format_ARGB32);
    img.fill(Qt::white);
}

void LineCanvas::setAlgo(Algo a) { algo = a; }
void LineCanvas::clearAll() { img.fill(Qt::white); pending = false; update(); }

void LineCanvas::mousePressEvent(QMouseEvent* e)
{
    if (!pending) {
        p0 = e->pos(); pending = true;
        emit statusChanged(QString("Dot 1: (%1,%2). Click the 2nd.").arg(p0.x()).arg(p0.y()));
    } else {
        QPoint p1 = e->pos();
        if (algo == Bresenham)
            drawBresenham(p0.x(), p0.y(), p1.x(), p1.y());
        else
            drawWu(p0.x(), p0.y(), p1.x(), p1.y());
        pending = false;
        emit statusChanged(QString("Line: (%1,%2) → (%3,%4)").arg(p0.x()).arg(p0.y()).arg(p1.x()).arg(p1.y()));
        update();
    }
}

void LineCanvas::mouseMoveEvent(QMouseEvent* e)
{
    emit coordsChanged(QString("x=%1 y=%2").arg(e->pos().x()).arg(e->pos().y()));
}

void LineCanvas::paintEvent(QPaintEvent*)
{
    QPainter g(this);
    g.drawImage(0, 0, img);
    g.setPen(QPen(QColor(0,0,0,10), 1));
    for (int x = 0; x < width();  x += 40) g.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) g.drawLine(0, y, width(), y);
    if (pending) {
        g.setPen(QPen(Qt::black, 1, Qt::DashLine));
        g.drawEllipse(p0, 4, 4);
    }
}

void LineCanvas::drawBresenham(int x0, int y0, int x1, int y1)
{
    const int W = img.width();
    const int H = img.height();
    const QColor c = Qt::black;

    int dx =  qAbs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -qAbs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        if (x0 >= 0 && y0 >= 0 && x0 < W && y0 < H)
            img.setPixelColor(x0, y0, c);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void LineCanvas::drawWu(int x0, int y0, int x1, int y1)
{
    const int W = img.width();
    const int H = img.height();
    const QColor c = Qt::black;

    auto plot = [&](int x, int y, double k) {
        if (x < 0 || y < 0 || x >= W || y >= H) return;
        if (k <= 0.0) return;
        if (k > 1.0) k = 1.0;
        QColor old = img.pixelColor(x, y);
        double r = c.red() * k + old.red() * (1.0 - k);
        double g = c.green() * k + old.green() * (1.0 - k);
        double b = c.blue() * k + old.blue() * (1.0 - k);
        img.setPixelColor(x, y, QColor(int(r), int(g), int(b)));
    };

    const int xStart = x0, yStart = y0;
    const int xEnd = x1, yEnd = y1;

    bool steep = qAbs(y1 - y0) > qAbs(x1 - x0);
    if (steep) { std::swap(x0, y0); std::swap(x1, y1); }
    if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }

    int dx = x1 - x0;
    int dy = y1 - y0;

    if (dx == 0) {
        plot(xStart, yStart, 1.0);
        plot(xEnd, yEnd, 1.0);
        return;
    }

    double gradient = double(dy) / double(dx);

    if (steep)
        plot(y0, x0, 1.0);
    else
        plot(x0, y0, 1.0);

    double y = y0 + gradient;
    for (int x = x0 + 1; x < x1; ++x)
    {
        int yi = int(std::floor(y));
        double frac = y - yi;

        if (steep) {
            plot(yi, x, 1.0 - frac);
            plot(yi + 1, x, frac);
        } else {
            plot(x, yi, 1.0 - frac);
            plot(x, yi + 1, frac);
        }
        y += gradient;
    }

    if (steep) plot(y1, x1, 1.0);
    else plot(x1, y1, 1.0);
}

Task2Window::Task2Window()
{
    setWindowTitle("Task 2 - Bresenham & Wu");
    setFixedSize(920, 660);
    setStyleSheet(QString("background:%1;").arg(kBg));

    auto* canvas = new LineCanvas(this);
    canvas->move(140, 20);

    auto* panel = new QWidget(this);
    panel->setGeometry(20, 20, 100, 600);
    panel->setStyleSheet(QString("background:#ffffff; border:1px solid %1; border-radius:6px;").arg(kBorder));
    auto* pl = new QVBoxLayout(panel);
    pl->setContentsMargins(8, 12, 8, 12);
    pl->setSpacing(8);

    auto mkAction = [&](const QString& text, const QColor& accent) {
        auto* b = new QPushButton(text);
        b->setCursor(Qt::PointingHandCursor);
        b->setFixedHeight(38);
        b->setStyleSheet(QString(
                             "QPushButton {"
                             "  color:%1;"
                             "  border:1px solid %2;"
                             "}"
                             "QPushButton:hover {"
                             "  background:%1;"
                             "  color:#ffffff;"
                             "  border:1px solid %1;"
                             "}"
                             "QPushButton:pressed {"
                             "  background:%1;"
                             "  color:#ffffff;"
                             "  padding-top:2px;"
                             "}"
                             ).arg(accent.name(), kBorder));
        return b;
    };

    auto mkToggle = [&](const QString& text, const QColor& accent) {
        auto* b = new QPushButton(text);
        b->setCursor(Qt::PointingHandCursor);
        b->setFixedHeight(38);
        b->setCheckable(true);
        b->setAutoExclusive(true);
        b->setStyleSheet(QString(
                             "QPushButton {"
                             "  color:%1;"
                             "  border:1px solid %2;"
                             "}"
                             "QPushButton:hover {"
                             "  background:%1;"
                             "  color:#ffffff;"
                             "  border:1px solid %1;"
                             "}"
                             "QPushButton:checked {"
                             "  background:%1;"
                             "  color:#ffffff;"
                             "  border:1px solid %1;"
                             "}"
                             "QPushButton:checked:hover {"
                             "  background:%1;"
                             "  color:#ffffff;"
                             "}"
                             ).arg(accent.name(), kBorder));
        return b;
    };

    auto* bBr = mkToggle("Брезенхем", Qt::darkGreen);
    auto* bWu = mkToggle("Ву", Qt::blue);
    auto* bClr = mkAction("Очистить",  Qt::red);

    bBr->setChecked(true);

    pl->addWidget(bBr);
    pl->addWidget(bWu);
    pl->addSpacing(14);
    pl->addWidget(bClr);
    pl->addStretch();

    connect(bBr,  &QPushButton::clicked, this, [=]{
        canvas->setAlgo(LineCanvas::Bresenham);
    });
    connect(bWu,  &QPushButton::clicked, this, [=]{
        canvas->setAlgo(LineCanvas::Wu);
    });

    connect(bClr, &QPushButton::clicked, this, [=]{ canvas->clearAll(); });
}