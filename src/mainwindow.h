#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "calculation_metrics.hpp"
#include "mandelbrot_fwd.hpp"
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
  std::vector<fractals::view_parameters> userAddedBookmarks;
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
  void startCalculating(numbers::radius r, int i);
  void completed(const fractals::calculation_metrics *metrics);
  void openGoToDialog();
  void changeFractal(class ChangeFractalAction *src,
                     const fractals::fractal &fractal);
  void fractalChanged(const char *name);
  void cancelAnimations();
  void addBookmark();
  void newWindow();

  void shadingChanged(bool checked);
  void reloadBookmarks();

private:
  Ui::MainWindow *ui;
  QActionGroup fractalsActionGroup;
  QActionGroup zoomSpeedActionGroup;
  QActionGroup threadingActionGroup;
  int initialBookmarksMenuSize;  // Used when constructing the bookmarks menu

  void addBookmarkToList(const fractals::view_parameters &params, bool isUser,
                     bool isBuiltin);
  void addBookmarkToMenu(const fractals::view_parameters &params);

  std::shared_ptr<SharedBookmarks> bookmarks;

  void loadBookmarks(QFile &&file, bool isUser, bool isBuiltin);
  void saveBookmarks();

  QFile getBookmarksFile();
};

class ChangeFractalAction : public QAction {
  Q_OBJECT
public:
  ChangeFractalAction(const char *name,
                      const fractals::fractal &fractal, bool checked);

signals:
  void changeFractal(ChangeFractalAction *src,
                     const fractals::fractal &);

private slots:
  void select(bool checked);

private:
  const fractals::fractal &fractal;
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
