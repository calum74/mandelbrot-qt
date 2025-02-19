#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "RenderingMetrics.hpp"
#include "fractal.hpp"
#include "view_parameters.hpp"
#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QFile>
#include <QMainWindow>

class Bookmark;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct SharedBookmarks {
  std::vector<fractals::view_parameters> bookmarks;
  std::vector<fractals::view_parameters> builtinBookmarks;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
  MainWindow(const std::shared_ptr<SharedBookmarks> &bookmarks = {},
             QWidget *parent = nullptr);
  ~MainWindow();
  void closeEvent(QCloseEvent *) override;

public slots:
  void startCalculating(double d, int i);
  void completed(const fractals::RenderingMetrics *metrics);
  void openGoToDialog();
  void changeFractal(class ChangeFractalAction *src,
                     const fractals::pointwise_fractal &fractal);
  void fractalChanged(const char *name);
  void cancelAnimations();
  void addBookmark();
  void newWindow();

  void showBookmarks(bool checked);

private:
  Ui::MainWindow *ui;
  QActionGroup fractalsActionGroup;
  QActionGroup zoomSpeedActionGroup;
  QActionGroup threadingActionGroup;

  void doAddBookmark(const fractals::view_parameters &params, bool isUser,
                     bool isBuiltin);
  std::shared_ptr<SharedBookmarks> bookmarks;

  void loadBookmarks(QFile &&file, bool isUser, bool isBuiltin);
  void saveBookmarks();

  QFile getBookmarksFile();
};

class ChangeFractalAction : public QAction {
  Q_OBJECT
public:
  ChangeFractalAction(const char *name,
                      const fractals::pointwise_fractal &fractal, bool checked);

signals:
  void changeFractal(ChangeFractalAction *src,
                     const fractals::pointwise_fractal &);

private slots:
  void select(bool checked);

private:
  const fractals::pointwise_fractal &fractal;
};

class Bookmark : public QAction {
  Q_OBJECT
public:
  Bookmark(const fractals::view_parameters &);

private slots:
  void triggered(bool);

signals:
  void selected(const fractals::view_parameters *);

private:
  fractals::view_parameters params;
};

#endif // MAINWINDOW_H
