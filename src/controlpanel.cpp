#include "controlpanel.h"
#include "ui_controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent)
    : QDialog(parent), ui(new Ui::ControlPanel) {
  ui->setupUi(this);
  changeShading(true);
  ui->colourSeedSpin->setRange(0,1000000);
  ui->colourGradientSlider->setRange(1,2500);

  connect(ui->shadingCheck, &QCheckBox::toggled, this, &ControlPanel::shadingChanged);
  connect(ui->colourSeedSpin, &QSpinBox::valueChanged, this, &ControlPanel::colourSeedChanged);
  connect(ui->colourGradientSlider, &QSlider::valueChanged, this, &ControlPanel::changeColourGradient);
  connect(ui->colourGradientBox, &QLineEdit::textChanged, this, [&](QString value) {
    auto d = value.toDouble();
    if(d<2000) 
        ui->colourGradientSlider->setValue(d);
    colourGradientChanged(value.toDouble());
  });
}

ControlPanel::~ControlPanel() { delete ui; }

void ControlPanel::changeShading(bool shading) {
  ui->shadingCheck->setChecked(shading);
}

void ControlPanel::changeColourSeed(int s) {
    ui->colourSeedSpin->setValue(s);
}

void ControlPanel::changeColourGradient(double d) {
    auto s = (std::stringstream()<<d).str();
    ui->colourGradientBox->setText(s.c_str());
    ui->colourGradientSlider->setValue(d);
}

void ControlPanel::changeColourOffset(double) {}
