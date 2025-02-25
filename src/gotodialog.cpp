#include "gotodialog.h"
#include "ui_gotodialog.h"
#include <iomanip>
#include <sstream>

GoToDialog::GoToDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::GoToDialog) {
  ui->setupUi(this);
}

GoToDialog::~GoToDialog() { delete ui; }

void GoToDialog::accept() { QDialog::accept(); }

template <typename T> QString to_qstring(const T &t) {
  std::stringstream ss;
  ss << t;
  return {ss.str().c_str()};
}

template <typename T> void read(const QString &src, T &result) {
  std::stringstream ss(src.toStdString());
  ss >> result;
}

void GoToDialog::getCoords(fractals::view_parameters &params) const {
  read(ui->xCoord->toPlainText(), params.x);
  read(ui->yCoord->toPlainText(), params.y);
  read(ui->sizeCoord->toPlainText(), params.r);
  read(ui->maxIterations->toPlainText(), params.max_iterations);
  read(ui->colour->toPlainText(), params.shader.colour_scheme);
  read(ui->gradient->toPlainText(), params.shader.colour_gradient);
}

void GoToDialog::setCoords(const fractals::view_parameters &params) {
  ui->xCoord->setPlainText(params.x.c_str());
  ui->yCoord->setPlainText(params.y.c_str());
  ui->sizeCoord->setPlainText(params.r.c_str());

  ui->maxIterations->setPlainText(to_qstring(params.max_iterations));
  ui->colour->setPlainText(to_qstring(params.shader.colour_scheme));
  ui->gradient->setPlainText(to_qstring(params.shader.colour_gradient));
}
