#include "controlpanel.h"
#include "ui_controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent)
    : QDialog(parent), ui(new Ui::ControlPanel) {
  ui->setupUi(this);
  changeShading(true);

  connect(ui->shadingCheck, &QCheckBox::toggled, this, &ControlPanel::shadingChanged);
}

ControlPanel::~ControlPanel() { delete ui; }

void ControlPanel::changeShading(bool shading) {
  ui->shadingCheck->setChecked(shading);
}