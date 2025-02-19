#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QImage>
#include <QTimer>
#include <QWidget>

#include <mutex>

#include "AnimatedRenderer.hpp"
#include "ColourMap.hpp"
#include "Renderer.hpp"
#include "Viewport.hpp"
#include "fractal.hpp"
#include "view_coords.hpp"
#include "registry.hpp"

class ViewerWidget : public QWidget {
  Q_OBJECT
  QImage image;
  QTimer renderingTimer;

  // We can increase the resolution, perhaps to native screen resolution, but
  // it's slower.
  double imageScale = 1.0; // Oversample

  struct MyViewport : public fractals::Viewport {
    ViewerWidget &widget;
    MyViewport(ViewerWidget &);
    void updated() override;
    void finished(const fractals::RenderingMetrics &) override;
    void calculation_started(double log_radius, int iterations) override;
    void schedule_next_calculation() override;
    void start_timer() override;
    void stop_timer() override;
  } viewport;

  fractals::AnimatedRenderer renderer;

  // Track the previous position of the mouse cursor
  int press_x, press_y, move_x, move_y, start_x, start_y;
  bool release_can_start_zooming;

  void calculate();
  void draw();

  std::mutex bookmarksMutex;
  std::atomic<int> pending_redraw;

  void setSpeedEstimate(double secondsPerPixel);

  struct flag_location {
    int x, y, size;
  };

  struct BookmarkToDraw {
    std::string algorithm;
    fractals::view_coords coords;
  };

  std::vector<BookmarkToDraw> bookmarksToDraw;
  std::vector<flag_location> flagsToDraw;
  void doUpdate();

public:
  explicit ViewerWidget(QWidget *parent = nullptr);

  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

  void increaseIterations();
  void decreaseIterations();

  void getCoords(fractals::view_parameters &params) const;
  bool setCoords(const fractals::view_parameters &params);

  std::vector<std::pair<std::string, const fractals::fractal &>>
  listFractals();

  void saveToFile(const QString &image_filename);

  void showBookmarks(const fractals::view_parameters *params, int size);
  void hideBookmarks();

  void calculateFlagLocations();

public slots:
  void copyCoords();
  void pasteCoords();
  void recolourPalette();
  void resetCurrentFractal();
  void changeFractal(const fractals::fractal &fractal);
  void enableThreading(bool checked);
  void singleThreaded(bool checked);
  void maxThreading(bool checked);
  void enableOversampling(bool checked);

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

  void openBookmark(const fractals::view_parameters *params);

signals:
  void startCalculating(double width, int maxIterations);
  void completed(const fractals::RenderingMetrics *
                     metrics); // References silently fail with Qt signals/slots
  void renderingFinishedSignal();
  void fractalChanged(const char *name);
};

#endif // VIEWERWIDGET_H
