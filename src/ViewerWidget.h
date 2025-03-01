#ifndef VIEWERWIDGET_H
#define VIEWERWIDGET_H

#include <QImage>
#include <QTimer>
#include <QWidget>

#include "AnimatedRenderer.hpp"
#include "ColourMap.hpp"
#include "Renderer.hpp"
#include "Viewport.hpp"
#include "controlpanel.h"
#include "fractal.hpp"
#include "registry.hpp"
#include "view_coords.hpp"

class ViewerWidget : public QWidget, fractals::view_listener {
  Q_OBJECT
  QImage image;
  QTimer renderingTimer;

  // We can increase the resolution, perhaps to native screen resolution, but
  // it's slower.
  double imageScale = 1.0; // Oversample

  fractals::AnimatedRenderer renderer;

  // Track the previous position of the mouse cursor
  int press_x, press_y, move_x, move_y, start_x, start_y;
  bool release_can_start_zooming;
  bool pending_resize;

  void calculate();
  void draw();

  std::atomic<int> pending_redraw;

  void setSpeedEstimate(double secondsPerPixel);

  ControlPanel controlPanel;
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

  std::vector<std::pair<std::string, const fractals::fractal &>> listFractals();

  void saveToFile(const QString &image_filename);

  void doResize(int w, int h);
  void updateColourControls();

  void calculation_started(double ln_r, int max_iterations) override;
  void values_changed() override;
  void calculation_finished(const fractals::calculation_metrics &) override;
  void animation_finished(const fractals::calculation_metrics &) override;

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
  void enableAutoGradient(bool checked);
  void enableShading(bool checked);

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
  void showOptions();

  void shadingParametersChanged(const fractals::shader_parameters *params);

signals:
  void startCalculating(double width, int maxIterations);
  void completed(const fractals::calculation_metrics *
                     metrics); // References silently fail with Qt signals/slots
  void renderingFinishedSignal();
  void fractalChanged(const char *name);
  void shadingChanged(bool);
};

#endif // VIEWERWIDGET_H
