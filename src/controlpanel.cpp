#include "controlpanel.h"
#include "ui_controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent)
    : QDialog(parent), ui(new Ui::ControlPanel) {
  ui->setupUi(this);
  ui->colourSeedSpin->setRange(0, 1000000);
  ui->colourGradientSlider->setRange(1, 2500);
  ui->colourOffsetSlider->setRange(0, 1000);

  connect(ui->resetGradientButton, &QPushButton::clicked, this,
          &ControlPanel::rescalePalette);
  connect(ui->shadingCheck, &QCheckBox::toggled, this, [&](bool checked) {
    params.shading = checked;
    updateParameters(&params);
  });

  connect(ui->colourSeedSpin, &QSpinBox::valueChanged, this, [&](int value) {
    params.colour_scheme = value;
    updateParameters(&params);
  });
  connect(ui->colourGradientSlider, &QSlider::valueChanged, this,
          [&](double value) {
            params.colour_gradient = value;
            updateParameters(&params);
            valuesChanged(&params);
          });
  connect(ui->colourGradientBox, &QLineEdit::textChanged, this,
          [&](QString value) {
            auto d = value.toDouble();
            if (d < 2000)
              ui->colourGradientSlider->setValue(d);
            updateParameters(&params);
            valuesChanged(&params);
          });
  connect(ui->colourOffsetSlider, &QSlider::valueChanged, this, [&](double d) {
    params.colour_offset = d / 10;
    updateParameters(&params);
    valuesChanged(&params);
  });
  connect(ui->colourOffsetBox, &QLineEdit::textChanged, this,
          [&](QString value) {
            auto d = value.toDouble();
            params.colour_offset = d;
            if (d < 100)
              ui->colourOffsetSlider->setValue(d * 10.0);
            updateParameters(&params);
          });
}

ControlPanel::~ControlPanel() { delete ui; }

void ControlPanel::valuesChanged(const fractals::shader_parameters *params) {
  this->params = *params;
  ui->colourSeedSpin->setValue(params->colour_scheme);
  ui->colourGradientSlider->setValue(params->colour_gradient);
  ui->colourOffsetSlider->setValue(params->colour_offset * 10.0);
  ui->shadingCheck->setChecked(params->shading);

  auto s = (std::stringstream() << params->colour_gradient).str();
  ui->colourGradientBox->setText(s.c_str());

  s = (std::stringstream() << params->colour_offset).str();
  ui->colourOffsetBox->setText(s.c_str());
}
