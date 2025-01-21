#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QImage>
#include <QTimer>
#include <QWidget>

#include "ColourMap.hpp"
#include "Renderer.hpp"
#include "Viewport.hpp"
#include "fractal.hpp"
#include "registry.hpp"

class ViewerWidget : public QWidget {
  Q_OBJECT
  QImage image, computedImage, previousImage;

  bool zooming = false;
  std::atomic<bool> calculationFinished = false;
  std::atomic<bool> zoomTimeout = false;
  std::chrono::time_point<std::chrono::system_clock> zoom_start;
  std::chrono::duration<double> zoom_duration;
  int zoom_x, zoom_y;

  enum class AnimationType {
    none,
    autozoom,
    depthzoom
  } current_animation = AnimationType::none;

  double lastRenderTime = 1.0;
  double estimatedSecondsPerPixel = 0;
  QTimer renderingTimer;

  std::unique_ptr<fractals::Registry> registry;

  // Note destruction order - renderer must be destroyed after viewport
  std::unique_ptr<fractals::Renderer> renderer;

  struct MyViewport : public fractals::Viewport {
    ViewerWidget *widget;
    void region_updated(int x, int y, int w, int h) override;
    void finished(double width, int min_depth, int max_depth, double avg,
                  double skipped, double render_time) override;
    void discovered_depth(int points, double discovered_depth,
                          double time_per_pixel) override;

  } viewport;

  struct BackgroundViewport : public fractals::Viewport {
    ViewerWidget *widget;
    void region_updated(int x, int y, int w, int h) override;
    void finished(double width, int min_depth, int max_depth, double avg,
                  double skipped, double render_time) override;
    void discovered_depth(int points, double discovered_depth,
                          double time) override;
  } background_viewport;

  std::unique_ptr<fractals::ColourMap> colourMap;

  // Track the previous position of the mouse cursor
  int press_x, press_y, move_x, move_y;

  void calculate();
  void draw();

  std::atomic<int> pending_redraw;

  void cancelAnimations();
  void setSpeedEstimate(double secondsPerPixel);
  void beginNextAnimation();
  void smoothZoomTo(int x, int y);

public:
  explicit ViewerWidget(QWidget *parent = nullptr);
  ~ViewerWidget() override;

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
  void renderFinished2();

public slots:
  void copyCoords();
  void recolourPalette();
  void resetCurrentFractal();
  void changeFractal(const fractals::PointwiseFractal &fractal);
  void enableThreading(bool checked);
  void enableAutoDepth(bool checked);
  void quickSave();
  void scalePalette();
  void open();
  void save();
  void center();
  void zoomIn();
  void zoomOut();
  void autoZoom();
  void autoZoomContinue();
  void renderingFinishedSlot();

  void smoothZoomIn();
  void backgroundRenderFinished();
  void updateFrame();

  void animateToHere();

signals:
  void startCalculating(double width, int maxIterations);
  void completed(double width, int min_depth, int max_depth, double, double,
                 double time);
  void renderingFinishedSignal();
};

#endif // VIEWERWIDGET_H
