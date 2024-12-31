#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QImage>
#include <QTimer>
#include <QWidget>

#include "ColourMap.hpp"
#include "Renderer.hpp"
#include "Viewport.hpp"
#include "fractal.hpp"

class ViewerWidget : public QWidget {
  Q_OBJECT
  QImage image;

  // Note destruction order - renderer must be destroyed after viewport
  std::unique_ptr<fractals::Renderer> renderer;

  struct MyViewport : public fractals::Viewport {
    ViewerWidget *widget;
    void region_updated(int x, int y, int w, int h) override;
    void finished(double width, int min_depth, int max_depth, double avg,
                  double skipped, double render_time) override;
    virtual void discovered_depth(int points, double discovered_depth) override;

  } viewport;

  std::unique_ptr<fractals::ColourMap> colourMap;

  // Track the previous position of the mouse cursor
  int press_x, press_y, move_x, move_y;

  void calculate();
  void draw();

  std::atomic<int> pending_redraw;

  QTimer timer;

public:
  explicit ViewerWidget(QWidget *parent = nullptr);
  ~ViewerWidget() override;

  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void timerEvent(QTimerEvent *event) override;

  void increaseIterations();
  void decreaseIterations();
  void toggleAutoMode();

  void getCoords(QString &x, QString &y, QString &r, QString &i) const;
  bool setCoords(const QString &x, const QString &y, const QString &r,
                 const QString &i);

  std::vector<std::pair<std::string, const fractals::PointwiseFractal &>>
  listFractals();

public slots:
  void timer2();
  void copyCoords();
  void randomizePalette();
  void resetCurrentFractal();
  void changeFractal(const fractals::PointwiseFractal &fractal);

signals:
  void startCalculating(double width, int maxIterations);
  void completed(double width, int min_depth, int max_depth, double, double,
                 double time);
};

#endif // VIEWERWIDGET_H
