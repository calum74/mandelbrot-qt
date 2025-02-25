#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QDialog>

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QDialog
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = nullptr);
    ~ControlPanel();

private:
    Ui::ControlPanel *ui;

signals:
    void shadingChanged(bool);
    void autoGradientChanged(bool);
    void colourSeedChanged(int);
    void colourGradientChanged(double);
    void colourOffsetChanged(double);

public slots:
    void changeShading(bool);
    void changeColourSeed(int);
    void changeColourGradient(double);
    void changeColourOffset(double);
};

#endif // CONTROLPANEL_H
