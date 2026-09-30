#ifndef TASK1_FILL_H
#define TASK1_FILL_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QVector>


class Canvas : public QWidget {
    Q_OBJECT
public:
    enum Mode { Draw, FillColor, FillPattern, Boundary };

    explicit Canvas(QWidget* parent = nullptr);

    void setMode(Mode m);
    void setFillColor(const QColor& c);
    void loadPattern();
    void clearAll();

signals:
    void statusChanged(const QString&);
    void coordsChanged(const QString&);

protected:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void floodFillScanline(int x, int y, const QColor& c, bool usePattern);
    QPoint findStart(const QPoint&) const;
    bool isFg(int , int) const;
    QVector<QPoint> mooreTrace(const QPoint&) const;

    int fillOriginX = 0;
    int fillOriginY = 0;
    QImage img;
    QImage pattern;
    Mode   mode = Draw;
    QColor fillColor = QColor(15, 157, 111);
    bool   drawing = false;
    QPoint last;
    QVector<QPoint> boundaryPts;
};


class Task1Window : public QWidget {
    Q_OBJECT
public:
    Task1Window();
};

#endif // TASK1_FILL_H
