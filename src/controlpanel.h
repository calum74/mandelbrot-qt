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

public slots:
    void changeShading(bool);
};

#endif // CONTROLPANEL_H
