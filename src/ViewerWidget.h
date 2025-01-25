#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QImage>
#include <QTimer>
#include <QWidget>

#include "AnimatedRenderer.hpp"
#include "ColourMap.hpp"
#include "Renderer.hpp"
#include "Viewport.hpp"
#include "fractal.hpp"
#include "registry.hpp"

class ViewerWidget : public QWidget {
  Q_OBJECT
  QImage image;
  QTimer renderingTimer;

  struct MyViewport : public fractals::Viewport {
    ViewerWidget &widget;
    MyViewport(ViewerWidget &);
    void updated() override;
    void finished(double width, int min_depth, int max_depth, double avg,
                  double skipped, double render_time) override;
    void discovered_depth(int points, double discovered_depth,
                          double time_per_pixel) override;
    void calculation_started(double log_radius, int iterations) override;
    void schedule_next_calculation() override;
    void start_timer() override;
    void stop_timer() override;
  } viewport;

  AnimatedRenderer renderer;

  // Track the previous position of the mouse cursor
  int press_x, press_y, move_x, move_y;

  void calculate();
  void draw();

  std::atomic<int> pending_redraw;

  void setSpeedEstimate(double secondsPerPixel);

public:
  explicit ViewerWidget(QWidget *parent = nullptr);

  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

  void increaseIterations();
  void decreaseIterations();

  void getCoords(fractals::view_parameters &params) const;
  bool setCoords(const fractals::view_parameters &params);

  std::vector<std::pair<std::string, const fractals::PointwiseFractal &>>
  listFractals();

  void saveToFile(const QString &image_filename);

public slots:
  void copyCoords();
  void recolourPalette();
  void resetCurrentFractal();
  void changeFractal(const fractals::PointwiseFractal &fractal);
  void enableThreading(bool checked);
  void singleThreaded(bool checked);
  void maxThreading(bool checked);

  void enableAutoDepth(bool checked);
  void quickSave();
  void scalePalette();
  void open();
  void save();
  void zoomIn();
  void zoomOut();
  void autoZoom();
  void renderingFinishedSlot();

  void smoothZoomIn();
  void updateFrame();

  void animateToHere();
  void zoomAtCursor();
  void stopAnimations();

  void setQualityAnimation();
  void setFastAnimation();
  void setFastestAnimation();

signals:
  void startCalculating(double width, int maxIterations);
  void completed(double width, int min_depth, int max_depth, double, double,
                 double time);
  void renderingFinishedSignal();
  void fractalChanged(const char *name);
};

#endif // VIEWERWIDGET_H
