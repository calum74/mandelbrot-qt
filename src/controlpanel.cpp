#include "controlpanel.h"
#include "ui_controlpanel.h"

ControlPanel::ControlPanel(QWidget *parent)
    : QDialog(parent), ui(new Ui::ControlPanel) {
  ui->setupUi(this);
  ui->colourSeedSpin->setRange(0, 1000000);
  ui->colourGradientSlider->setRange(1, 2500);
  ui->colourOffsetSlider->setRange(0, 1000);
  ui->ambientBrightnessSlider->setRange(0, 100);
  ui->sourceBrightnessSlider->setRange(0, 100);
  ui->sourceInclineSlider->setRange(0, 100);
  ui->sourceDirectionSlider->setRange(0, 100);

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
            params.colour_gradient = d;
            if (d < 2000)
              ui->colourGradientSlider->setValue(d);
            updateParameters(&params);
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

  connect(ui->ambientBrightnessSlider, &QSlider::valueChanged, this,
          [&](int value) {
            params.ambient_brightness = value / 100.0;
            updateParameters(&params);
          });

  connect(ui->sourceBrightnessSlider, &QSlider::valueChanged, this,
          [&](int value) {
            params.source_brightness = value / 100.0;
            updateParameters(&params);
          });

  connect(ui->sourceDirectionSlider, &QSlider::valueChanged, this,
          [&](double value) {
            params.source_direction_radians = value * 2 * M_PI / 100;
            updateParameters(&params);
          });

  connect(ui->sourceInclineSlider, &QSlider::valueChanged, this,
          [&](double value) {
            params.source_elevation_radians = value * M_PI / (2 * 100);
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

  ui->ambientBrightnessSlider->setValue(params->ambient_brightness * 100);
  ui->sourceBrightnessSlider->setValue(params->source_brightness * 100);

  ui->sourceDirectionSlider->setValue(params->source_direction_radians * 100 /
                                      (2 * M_PI));
  ui->sourceInclineSlider->setValue(params->source_elevation_radians * 100 /
                                    (M_PI / 2));
}
