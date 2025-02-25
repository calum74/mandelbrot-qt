#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QDialog>
#include "shader_parameters.hpp"

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
    fractals::shader_parameters params;

signals:
    void rescalePalette();
    void updateParameters(const fractals::shader_parameters*params);

public slots:
    void valuesChanged(const fractals::shader_parameters*params);
};

#endif // CONTROLPANEL_H
