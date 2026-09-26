#ifndef TASK2_LINES_H
#define TASK2_LINES_H

#include <QWidget>
#include <QImage>
#include <QPoint>

class LineCanvas : public QWidget {
    Q_OBJECT
public:
    enum Algo { Bresenham, Wu };

    explicit LineCanvas(QWidget* parent = nullptr);

    void setAlgo(Algo a);
    void setColor(const QColor& c);
    void clearAll();

signals:
    void statusChanged(const QString&);
    void coordsChanged(const QString&);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void drawBresenham(int x0, int y0, int x1, int y1, const QColor& c);
    void drawWu(int x0, int y0, int x1, int y1, const QColor& c);

    QImage img;
    Algo   algo = Bresenham;
    QColor color = Qt::black;
    QPoint p0;
    bool   pending = false;
};

class Task2Window : public QWidget {
    Q_OBJECT
public:
    Task2Window();
};

#endif // TASK2_LINES_H
