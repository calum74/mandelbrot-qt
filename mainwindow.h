#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void keyPressEvent(QKeyEvent *event) override;

  public slots:
    void zoomChanged(double d);
    void completed(double zoom, int min_depth, int max_depth, double time);
    void openGoToDialog();

  private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
