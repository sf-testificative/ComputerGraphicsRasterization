#include "task1_fill.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QPainter>
#include <QMouseEvent>
#include <QStack>

static const char* kBg     = "#ffffff";
static const char* kBorder = "#d0d7de";
static const char* kTxt    = "#1f2328";
static const char* kMuted  = "#6e7781";
static const char* kAccent = "#0f9d6f";
static const char* kAccent2= "#d1395c";

Canvas::Canvas(QWidget* parent) : QWidget(parent) {
    setFixedSize(720, 600);
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setStyleSheet("background:#ffffff; border:1px solid " + QString(kBorder) + ";");

    img = QImage(size(), QImage::Format_ARGB32);
    img.fill(Qt::white);
}

void Canvas::setMode(Mode m) { mode = m; }
void Canvas::setFillColor(const QColor& c) { fillColor = c; }

void Canvas::loadPattern() {
    QString path = QFileDialog::getOpenFileName(this, "Загрузить узор",
                                                "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (path.isEmpty()) return;
    QImage tmp;
    if (!tmp.load(path)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }
    pattern = tmp.convertToFormat(QImage::Format_ARGB32);
}

void Canvas::clearAll() {
    img.fill(Qt::white);
    boundaryPts.clear();
    update();
}

void Canvas::mousePressEvent(QMouseEvent* e) {
    QPoint p = e->pos();
    if (mode == Draw) {
        drawing = true;
        last = p;
    } else if (mode == FillColor) {
        floodFillScanline(p.x(), p.y(), fillColor, false);
        update();
    } else if (mode == FillPattern) {
        if (pattern.isNull()) { emit statusChanged("Сначала загрузите узор"); return; }
        floodFillScanline(p.x(), p.y(), QColor(), true);
        update();
    } else if (mode == Boundary) {
        QPoint start = findStart(p);
        if (start.isNull()) { emit statusChanged("Граница не найдена рядом с точкой"); return; }
        QVector<QPoint> pts = mooreTrace(start);
        boundaryPts = pts;
        emit statusChanged(QString("Точек границы: %1").arg(pts.size()));
        QPainter g(&img);
        g.setPen(QPen(QColor(209, 57, 92), 3));
        for (int i = 1; i < pts.size(); ++i) g.drawLine(pts[i-1], pts[i]);
        update();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent* e) {
    emit coordsChanged(QString("x=%1 y=%2").arg(e->pos().x()).arg(e->pos().y()));
    if (drawing && mode == Draw) {
        bresenham(last.x(), last.y(), e->pos().x(), e->pos().y(), QColor(30,30,30));
        last = e->pos();
        update();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent*) { drawing = false; }

void Canvas::paintEvent(QPaintEvent*) {
    QPainter g(this);
    g.drawImage(0, 0, img);
    g.setPen(QPen(QColor(0,0,0,10), 1));
    for (int x = 0; x < width();  x += 40) g.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 40) g.drawLine(0, y, width(), y);
}

void Canvas::bresenham(int x0, int y0, int x1, int y1, const QColor& c) {
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

// 1а / 1б — заливка сериями (итеративная реализация рекурсии)
void Canvas::floodFillScanline(int x, int y, const QColor& c, bool usePattern) {
    if (x < 0 || y < 0 || x >= img.width() || y >= img.height()) return;

    QRgb target = img.pixel(x, y);
    if (!usePattern && QColor(target) == c) return;

    auto pixelAt = [&](int px, int py) -> QColor {
        if (!usePattern) return c;
        int pw = pattern.width(), ph = pattern.height();
        return pattern.pixelColor(px % pw, py % ph);
    };

    QStack<QPoint> stack;
    stack.push({x, y});

    while (!stack.isEmpty()) {
        QPoint pt = stack.pop();
        int sx = pt.x(), sy = pt.y();
        if (sx < 0 || sy < 0 || sx >= img.width() || sy >= img.height()) continue;
        if (img.pixel(sx, sy) != target) continue;

        int xl = sx;
        while (xl > 0 && img.pixel(xl-1, sy) == target) --xl;
        int xr = sx;
        while (xr < img.width()-1 && img.pixel(xr+1, sy) == target) ++xr;

        for (int px = xl; px <= xr; ++px)
            img.setPixelColor(px, sy, pixelAt(px, sy));

        for (int ny : { sy-1, sy+1 }) {
            if (ny < 0 || ny >= img.height()) continue;
            bool inSpan = false;
            for (int px = xl; px <= xr; ++px) {
                if (img.pixel(px, ny) == target) {
                    if (!inSpan) { stack.push({px, ny}); inSpan = true; }
                } else inSpan = false;
            }
        }
    }
}

// 1в — обход границы по Муру
QPoint Canvas::findStart(const QPoint& p) const {
    for (int r = 0; r < 40; ++r) {
        for (int dy = -r; dy <= r; ++dy)
            for (int dx = -r; dx <= r; ++dx) {
                int nx = p.x()+dx, ny = p.y()+dy;
                if (nx < 0 || ny < 0 || nx >= img.width() || ny >= img.height()) continue;
                if (img.pixelColor(nx, ny) != Qt::white) return {nx, ny};
            }
    }
    return QPoint();
}

bool Canvas::isFg(int x, int y) const {
    if (x < 0 || y < 0 || x >= img.width() || y >= img.height()) return false;
    return img.pixelColor(x, y) != Qt::white;
}

QVector<QPoint> Canvas::mooreTrace(const QPoint& start) const {
    const int dx[8] = { 1, 1, 0,-1,-1,-1, 0, 1 };
    const int dy[8] = { 0, 1, 1, 1, 0,-1,-1,-1 };

    QVector<QPoint> contour;
    QPoint cur = start;
    contour.append(cur);
    int prevDir = 4;
    const int maxSteps = img.width() * img.height();
    int steps = 0;
    QPoint first = start;
    bool secondVisit = false;

    while (steps++ < maxSteps) {
        bool found = false;
        for (int k = 1; k <= 8; ++k) {
            int di = (prevDir + k) % 8;
            int nx = cur.x() + dx[di];
            int ny = cur.y() + dy[di];
            if (isFg(nx, ny)) {
                cur = QPoint(nx, ny);
                contour.append(cur);
                prevDir = (di + 4) % 8;
                found = true;
                break;
            }
        }
        if (!found) break;
        if (cur == first) {
            if (secondVisit) break;
            secondVisit = true;
        }
    }
    return contour;
}

//  Task1Window
static QPushButton* panelBtn(const QString& text, const QColor& accent) {
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
}

Task1Window::Task1Window() {
    setWindowTitle("Task 1 — Fill & Boundary");
    setFixedSize(920, 660);
    setStyleSheet(QString("background:%1;").arg(kBg));

    auto* canvas = new Canvas(this);
    canvas->move(180, 20);

    auto* panel = new QWidget(this);
    panel->setGeometry(20, 20, 150, 600);
    panel->setStyleSheet(QString("background:#ffffff; border:1px solid %1;").arg(kBorder));
    auto* pl = new QVBoxLayout(panel);
    pl->setContentsMargins(8, 14, 8, 14);
    pl->setSpacing(4);

    auto* t1 = new QLabel("TASK 1");
    t1->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:14px; font-weight:bold; border:none;").arg(kTxt));
    t1->setAlignment(Qt::AlignCenter);
    auto* t2 = new QLabel("fill / boundary");
    t2->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:8px; border:none;").arg(kMuted));
    t2->setAlignment(Qt::AlignCenter);
    pl->addWidget(t1);
    pl->addWidget(t2);
    pl->addSpacing(12);

    auto* bDraw = panelBtn("Рисовать",      QColor(kAccent));
    auto* bFill = panelBtn("Залить цветом", QColor("#2f7fd1"));
    auto* bPat  = panelBtn("Залить узором", QColor("#b07a10"));
    auto* bBnd  = panelBtn("Граница",       QColor(kAccent2));
    pl->addWidget(bDraw);
    pl->addWidget(bFill);
    pl->addWidget(bPat);
    pl->addWidget(bBnd);
    pl->addSpacing(8);

    auto* sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet(QString("color:%1; background:%1; max-height:1px; border:none;").arg(kBorder));
    pl->addWidget(sep);
    pl->addSpacing(6);

    auto* bColor = panelBtn("Цвет заливки",   QColor(kMuted));
    auto* bLoad  = panelBtn("Загрузить узор", QColor(kMuted));
    auto* bClear = panelBtn("Очистить",       QColor(kMuted));
    pl->addWidget(bColor);
    pl->addWidget(bLoad);
    pl->addWidget(bClear);

    auto* status = new QLabel("Режим: рисование");
    status->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:10px; background:transparent;").arg(kTxt));
    status->setGeometry(180, 625, 500, 18);

    auto* coords = new QLabel("x=--- y=---");
    coords->setStyleSheet(QString("color:%1; font-family:Consolas; font-size:10px; background:transparent;").arg(kMuted));
    coords->setGeometry(700, 625, 200, 18);
    coords->setAlignment(Qt::AlignRight);

    connect(bDraw, &QPushButton::clicked, [=]{ canvas->setMode(Canvas::Draw);
        status->setText("Режим: рисование"); });
    connect(bFill, &QPushButton::clicked, [=]{ canvas->setMode(Canvas::FillColor);
        status->setText("Режим: заливка цветом"); });
    connect(bPat,  &QPushButton::clicked, [=]{ canvas->setMode(Canvas::FillPattern);
        status->setText("Режим: заливка узором"); });
    connect(bBnd,  &QPushButton::clicked, [=]{ canvas->setMode(Canvas::Boundary);
        status->setText("Режим: обход границы"); });

    connect(bColor, &QPushButton::clicked, [=]{
        QColor c = QColorDialog::getColor(QColor(kAccent), this, "Цвет заливки");
        if (c.isValid()) canvas->setFillColor(c);
    });
    connect(bLoad,  &QPushButton::clicked, [=]{ canvas->loadPattern(); });
    connect(bClear, &QPushButton::clicked, [=]{ canvas->clearAll(); });

    connect(canvas, &Canvas::statusChanged, status, &QLabel::setText);
    connect(canvas, &Canvas::coordsChanged, coords, &QLabel::setText);
}