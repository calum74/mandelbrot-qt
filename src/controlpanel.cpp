#include "controlpanel.h"
#include "ui_controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent)
    : QDialog(parent), ui(new Ui::ControlPanel) {
  ui->setupUi(this);
  changeShading(true);
  ui->colourSeedSpin->setRange(0,1000000);

  connect(ui->shadingCheck, &QCheckBox::toggled, this, &ControlPanel::shadingChanged);
  connect(ui->colourSeedSpin, &QSpinBox::valueChanged, this, &ControlPanel::colourSeedChanged);
}

ControlPanel::~ControlPanel() { delete ui; }

void ControlPanel::changeShading(bool shading) {
  ui->shadingCheck->setChecked(shading);
}

void ControlPanel::changeColourSeed(int s) {
    ui->colourSeedSpin->setValue(s);
}

void ControlPanel::changeColourGradient(double) {}

void ControlPanel::changeColourOffset(double) {}
