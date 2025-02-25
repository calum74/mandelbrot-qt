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
};

#endif // CONTROLPANEL_H
