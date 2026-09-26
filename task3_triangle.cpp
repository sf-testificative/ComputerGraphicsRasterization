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

static const char* kBg     = "#ffffff";
static const char* kBorder = "#d0d7de";
static const char* kTxt    = "#1f2328";
static const char* kMuted  = "#6e7781";
static const char* kAccent = "#0f9d6f";
static const char* kAccent2= "#d1395c";

// ============================================================
//  TriCanvas
// ============================================================
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
}

void TriCanvas::clearAll() { img.fill(Qt::white); pts.clear(); update(); }

void TriCanvas::setColor(int i) {
    QColor c = QColorDialog::getColor(colors[i], this,
                                      QString("Цвет вершины %1").arg(i+1));
    if (c.isValid()) colors[i] = c;
}

void TriCanvas::mousePressEvent(QMouseEvent* e) {
    if (pts.size() >= 3) { img.fill(Qt::white); pts.clear(); }
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
        g.setBrush(colors[i]);
        g.setPen(QPen(Qt::black, 1));
        g.drawEllipse(pts[i], 5, 5);
    }
}

// Растеризация: барицентрические координаты + интерполяция цвета
void TriCanvas::rasterizeTriangle() {
    if (pts.size() != 3) return;

    struct V { int x, y; QColor c; };
    V v[3] = {
               { pts[0].x(), pts[0].y(), colors[0] },
               { pts[1].x(), pts[1].y(), colors[1] },
               { pts[2].x(), pts[2].y(), colors[2] },
               };
    std::sort(v, v+3, [](const V& a, const V& b){ return a.y < b.y; });

    auto bary = [&](double px, double py,
                    const V& A, const V& B, const V& C) -> std::array<double,3>
    {
        double d = double((B.y - C.y)*(A.x - C.x) + (C.x - B.x)*(A.y - C.y));
        if (qFuzzyIsNull(d)) return {1.0/3, 1.0/3, 1.0/3};
        double wa = ((B.y - C.y)*(px - C.x) + (C.x - B.x)*(py - C.y)) / d;
        double wb = ((C.y - A.y)*(px - C.x) + (A.x - C.x)*(py - C.y)) / d;
        double wc = 1.0 - wa - wb;
        return {wa, wb, wc};
    };

    int yMin = std::max(v[0].y, 0);
    int yMax = std::min(std::max({v[0].y, v[1].y, v[2].y}), img.height()-1);
    int xMin = std::max(std::min({v[0].x, v[1].x, v[2].x}), 0);
    int xMax = std::min(std::max({v[0].x, v[1].x, v[2].x}), img.width()-1);

    for (int y = yMin; y <= yMax; ++y) {
        QVector<QPair<double,double>> xs;
        auto addEdge = [&](const V& a, const V& b) {
            if ((a.y <= y && y <= b.y) || (b.y <= y && y <= a.y)) {
                if (a.y == b.y) return;
                double t = double(y - a.y) / double(b.y - a.y);
                double x = a.x + t * (b.x - a.x);
                xs.append({x, t});
            }
        };
        addEdge(v[0], v[1]);
        addEdge(v[1], v[2]);
        addEdge(v[0], v[2]);

        if (xs.size() < 2) continue;
        std::sort(xs.begin(), xs.end(),
                  [](const auto& p, const auto& q){ return p.first < q.first; });

        int xL = std::max(int(std::ceil(xs.first().first)), xMin);
        int xR = std::min(int(std::floor(xs.last().first)),  xMax);

        for (int x = xL; x <= xR; ++x) {
            auto w = bary(x, y,
                          {pts[0].x(), pts[0].y(), colors[0]},
                          {pts[1].x(), pts[1].y(), colors[1]},
                          {pts[2].x(), pts[2].y(), colors[2]});
            double wa = w[0], wb = w[1], wc = w[2];
            if (wa < -0.001 || wb < -0.001 || wc < -0.001) continue;
            double s = wa + wb + wc;
            if (s > 0) { wa/=s; wb/=s; wc/=s; }

            int r = int(wa*colors[0].red()   + wb*colors[1].red()   + wc*colors[2].red());
            int g = int(wa*colors[0].green() + wb*colors[1].green() + wc*colors[2].green());
            int b = int(wa*colors[0].blue()  + wb*colors[1].blue()  + wc*colors[2].blue());
            img.setPixelColor(x, y, QColor(qBound(0,r,255),
                                           qBound(0,g,255),
                                           qBound(0,b,255)));
        }
    }
}

// ============================================================
//  Task3Window
// ============================================================
Task3Window::Task3Window() {
    setWindowTitle("Task 3 — Gradient Triangle");
    setFixedSize(920, 660);
    setStyleSheet(QString("background:%1;").arg(kBg));

    auto* canvas = new TriCanvas(this);
    canvas->move(180, 20);

    auto* panel = new QWidget(this);
    panel->setGeometry(20, 20, 150, 600);
    panel->setStyleSheet(QString("background:#ffffff; border:1px solid %1;").arg(kBorder));
    auto* pl = new QVBoxLayout(panel);
    pl->setContentsMargins(8, 14, 8, 14);
    pl->setSpacing(6);

    auto* t1 = new QLabel("TASK ③");
    t1->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:14px; font-weight:bold; border:none;").arg(kTxt));
    t1->setAlignment(Qt::AlignCenter);
    auto* t2 = new QLabel("gradient triangle");
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

    auto* b1  = mk("◆  Цвет вершины 1", QColor(kAccent));
    auto* b2  = mk("◆  Цвет вершины 2", QColor(kAccent2));
    auto* b3  = mk("◆  Цвет вершины 3", QColor("#2f7fd1"));
    auto* bCl = mk("🗑  Очистить",       QColor(kMuted));
    pl->addWidget(b1);
    pl->addWidget(b2);
    pl->addWidget(b3);
    pl->addSpacing(10);
    pl->addWidget(bCl);
    pl->addStretch();

    auto* status = new QLabel("Кликните 1-ю вершину треугольника");
    status->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:10px; background:transparent;").arg(kTxt));
    status->setGeometry(180, 625, 500, 18);

    auto* coords = new QLabel("x=--- y=---");
    coords->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:10px; background:transparent;").arg(kMuted));
    coords->setGeometry(700, 625, 200, 18);
    coords->setAlignment(Qt::AlignRight);

    connect(b1,  &QPushButton::clicked, [=]{ canvas->setColor(0); });
    connect(b2,  &QPushButton::clicked, [=]{ canvas->setColor(1); });
    connect(b3,  &QPushButton::clicked, [=]{ canvas->setColor(2); });
    connect(bCl, &QPushButton::clicked, [=]{ canvas->clearAll(); });

    connect(canvas, &TriCanvas::statusChanged, status, &QLabel::setText);
    connect(canvas, &TriCanvas::coordsChanged, coords, &QLabel::setText);
}