#include "task3_triangle.h"

#include <QPainter>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QColorDialog>
#include <array>
#include <algorithm>
#include <cmath>

static const char* kBg = "#ffffff";
static const char* kBorder = "#d0d7de";
static const char* kMuted = "#6e7781";

TriCanvas::TriCanvas(QWidget* parent) : QWidget(parent) {
    setFixedSize(720, 600);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setStyleSheet("background:#ffffff; border:1px solid " + QString(kBorder) + ";");
    img = QImage(size(), QImage::Format_ARGB32);
    img.fill(Qt::white);

    colors[0] = QColor(15, 157, 111);
    colors[1] = QColor(209, 57, 92);
    colors[2] = QColor(47, 127, 209);

    displayColors[0] = colors[0];
    displayColors[1] = colors[1];
    displayColors[2] = colors[2];
}

void TriCanvas::clearAll() {
    img.fill(Qt::white);
    pts.clear();
    syncDisplayColors();
    update();
}

void TriCanvas::syncDisplayColors() {
    displayColors[0] = colors[0];
    displayColors[1] = colors[1];
    displayColors[2] = colors[2];
}

void TriCanvas::setColor(int i) {
    QColor c = QColorDialog::getColor(colors[i], this,
                                      QString("Цвет вершины %1").arg(i+1),
                                      QColorDialog::DontUseNativeDialog);
    if (c.isValid()) {
        colors[i] = c;
    }
}

void TriCanvas::mousePressEvent(QMouseEvent* e) {
    if (pts.size() >= 3) {
        img.fill(Qt::white);
        pts.clear();
        syncDisplayColors();
    }

    if (pts.isEmpty()) {
        syncDisplayColors();
    }

    pts.append(e->pos());
    emit statusChanged(QString("Вершин: %1 / 3").arg(pts.size()));

    if (pts.size() == 3) {
        rasterizeTriangle();
        emit statusChanged("Треугольник залит градиентом");
    }

    update();
}

void TriCanvas::mouseMoveEvent(QMouseEvent* e) {
    emit coordsChanged(QString("x=%1 y=%2").arg(e->pos().x()).arg(e->pos().y()));
}

void TriCanvas::paintEvent(QPaintEvent*) {
    QPainter g(this);
    g.drawImage(0, 0, img);
    g.setPen(QPen(QColor(0,0,0,10), 1));
    for (int x = 0; x < width();  x += 40) g.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) g.drawLine(0, y, width(), y);

    for (int i = 0; i < pts.size(); ++i) {
        g.setBrush(displayColors[i]);
        g.setPen(QPen(Qt::black, 1));
        g.drawEllipse(pts[i], 5, 5);
    }
}

void TriCanvas::rasterizeTriangle() {
    if (pts.size() != 3)
        return;

    const double Ax = pts[0].x();
    const double Ay = pts[0].y();
    const double Bx = pts[1].x();
    const double By = pts[1].y();
    const double Cx = pts[2].x();
    const double Cy = pts[2].y();

    const double a11 = Ax - Cx;
    const double a12 = Bx - Cx;
    const double a21 = Ay - Cy;
    const double a22 = By - Cy;
    const double det = a11 * a22 - a12 * a21;

    if (std::abs(det) < 1e-9)
        return;

    const int xMin = std::max(int(std::floor(std::min({Ax, Bx, Cx}))), 0);
    const int xMax = std::min(int(std::ceil(std::max({Ax, Bx, Cx}))), img.width() - 1);
    const int yMin = std::max(int(std::floor(std::min({Ay, By, Cy}))), 0);
    const int yMax = std::min(int(std::ceil(std::max({Ay, By, Cy}))), img.height() - 1);

    const QColor& cA = displayColors[0];
    const QColor& cB = displayColors[1];
    const QColor& cC = displayColors[2];

    for (int y = yMin; y <= yMax; ++y) {
        for (int x = xMin; x <= xMax; ++x) {
            const double px = x + 0.5;
            const double py = y + 0.5;

            const double dx = px - Cx;
            const double dy = py - Cy;

            const double wa = (dx * a22 - a12 * dy) / det;
            const double wb = (a11 * dy - dx * a21) / det;
            const double wc = 1.0 - wa - wb;

            if (wa < -1e-6 || wb < -1e-6 || wc < -1e-6)
                continue;

            const int r = qBound(0,
                                 int(std::lround(wa * cA.red() +
                                                 wb * cB.red() +
                                                 wc * cC.red())),
                                 255);
            const int g = qBound(0,
                                 int(std::lround(wa * cA.green() +
                                                 wb * cB.green() +
                                                 wc * cC.green())),
                                 255);
            const int b = qBound(0,
                                 int(std::lround(wa * cA.blue() +
                                                 wb * cB.blue() +
                                                 wc * cC.blue())),
                                 255);

            img.setPixelColor(x, y, QColor(r, g, b));
        }
    }
}

Task3Window::Task3Window() {
    setWindowTitle("Task 3 - Gradient Triangle");
    setFixedSize(920, 660);
    setStyleSheet(QString("background:%1;").arg(kBg));

    auto* canvas = new TriCanvas(this);
    canvas->move(180, 20);

    auto* panel = new QWidget(this);
    panel->setGeometry(20, 20, 150, 600);
    panel->setStyleSheet(QString("background:#ffffff; border:1px solid %1; border-radius:6px;").arg(kBorder));
    auto* pl = new QVBoxLayout(panel);
    pl->setContentsMargins(8, 14, 8, 14);
    pl->setSpacing(8);

    auto styleFor = [](const QColor& c) {
        QColor hover = c.darker(115);
        return QString(
                   "QPushButton {"
                   "  background:%1;"
                   "  color:#ffffff;"
                   "  border:1px solid %2;"
                   "  border-radius:8px;"
                   "}"
                   "QPushButton:hover {"
                   "  background:%2;"
                   "}"
                   "QPushButton:pressed {"
                   "  background:%3;"
                   "  padding-top:2px;"
                   "}"
                   ).arg(c.name(), hover.name(), c.darker(130).name());
    };

    auto* b1 = new QPushButton("Вершина 1");
    auto* b2 = new QPushButton("Вершина 2");
    auto* b3 = new QPushButton("Вершина 3");
    b1->setCursor(Qt::PointingHandCursor); b1->setFixedHeight(38);
    b2->setCursor(Qt::PointingHandCursor); b2->setFixedHeight(38);
    b3->setCursor(Qt::PointingHandCursor); b3->setFixedHeight(38);

    b1->setStyleSheet(styleFor(canvas->colorOf(0)));
    b2->setStyleSheet(styleFor(canvas->colorOf(1)));
    b3->setStyleSheet(styleFor(canvas->colorOf(2)));

    auto* bClr = new QPushButton("Очистить");
    bClr->setCursor(Qt::PointingHandCursor);
    bClr->setFixedHeight(38);
    bClr->setStyleSheet(QString(
                            "QPushButton {"
                            "  background:#f6f8fa; color:%1;"
                            "  border:1px solid %2; border-radius:8px;"
                            "}"
                            "QPushButton:hover { background:%2; color:#ffffff; }"
                            "QPushButton:pressed { background:%2; color:#ffffff; padding-top:2px; }"
                            ).arg(kMuted, kBorder));

    pl->addWidget(b1);
    pl->addWidget(b2);
    pl->addWidget(b3);
    pl->addSpacing(10);
    pl->addWidget(bClr);
    pl->addStretch();

    auto pick = [=](int i, QPushButton* b) {
        canvas->setColor(i);
        b->setStyleSheet(styleFor(canvas->colorOf(i)));
    };

    connect(b1,  &QPushButton::clicked, [=]{ pick(0, b1); });
    connect(b2,  &QPushButton::clicked, [=]{ pick(1, b2); });
    connect(b3,  &QPushButton::clicked, [=]{ pick(2, b3); });
    connect(bClr, &QPushButton::clicked, this, [=]{ canvas->clearAll(); });
}

