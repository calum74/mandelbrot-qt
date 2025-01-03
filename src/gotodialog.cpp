#include "gotodialog.h"
#include "ui_gotodialog.h"

GoToDialog::GoToDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::GoToDialog) {
  ui->setupUi(this);
}

GoToDialog::~GoToDialog() { delete ui; }

void GoToDialog::accept() { QDialog::accept(); }

void GoToDialog::getCoords(QString &x, QString &y, QString &r,
                           QString &i) const {

  x = ui->xCoord->toPlainText();
  y = ui->yCoord->toPlainText();
  r = ui->sizeCoord->toPlainText();
  i = ui->maxIterations->toPlainText();
}

void GoToDialog::setCoords(const QString &x, const QString &y, const QString &r,
                           const QString &i) {
  ui->xCoord->setPlainText(x);
  ui->yCoord->setPlainText(y);
  ui->sizeCoord->setPlainText(r);
  ui->maxIterations->setPlainText(i);
}
