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
  read(ui->xCoord->toPlainText(), params.coords.x);
  read(ui->yCoord->toPlainText(), params.coords.y);
  read(ui->sizeCoord->toPlainText(), params.coords.r);
  read(ui->maxIterations->toPlainText(), params.coords.max_iterations);
  read(ui->colour->toPlainText(), params.colour_seed);
  read(ui->gradient->toPlainText(), params.colour_gradient);
}

void GoToDialog::setCoords(const fractals::view_parameters &params) {
  std::stringstream ss1;
  ss1 << std::setprecision(params.coords.get_precision()) << params.coords.x;
  ui->xCoord->setPlainText(ss1.str().c_str());

  std::stringstream ss2;
  ss2 << std::setprecision(params.coords.get_precision()) << params.coords.y;
  ui->yCoord->setPlainText(ss2.str().c_str());

  std::stringstream ss3;
  ss3 << std::setprecision(3);
  fractals::log_radius(ss3, params.coords.ln_r());
  ui->sizeCoord->setPlainText(ss3.str().c_str());

  ui->maxIterations->setPlainText(to_qstring(params.coords.max_iterations));
  ui->colour->setPlainText(to_qstring(params.colour_seed));
  ui->gradient->setPlainText(to_qstring(params.colour_gradient));
}
