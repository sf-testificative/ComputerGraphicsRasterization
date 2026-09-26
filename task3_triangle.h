#ifndef TASK3_TRIANGLE_H
#define TASK3_TRIANGLE_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QVector>
#include <QColor>

class TriCanvas : public QWidget {
    Q_OBJECT
public:
    explicit TriCanvas(QWidget* parent = nullptr);

    void setColor(int i);
    void clearAll();
    QColor colorOf(int i) const { return colors[i]; }

signals:
    void statusChanged(const QString&);
    void coordsChanged(const QString&);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void rasterizeTriangle();
    void syncDisplayColors();

    QImage img;
    QVector<QPoint> pts;

    QColor colors[3];
    QColor displayColors[3];
};

class Task3Window : public QWidget {
    Q_OBJECT
public:
    Task3Window();
};

#endif // TASK3_TRIANGLE_H