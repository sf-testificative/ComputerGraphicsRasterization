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
    void clearAll();

signals:
    void statusChanged(const QString&);
    void coordsChanged(const QString&);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void drawBresenham(int x0, int y0, int x1, int y1);
    void drawWu(int x0, int y0, int x1, int y1);

    static constexpr int kScale = 2;    // коэффициент увеличения

    QImage img;                 // буфер в "логических" пикселях
    Algo   algo = Bresenham;
    QPoint p0;
    bool   pending = false;
};

class Task2Window : public QWidget {
    Q_OBJECT
public:
    Task2Window();
};

#endif // TASK2_LINES_H