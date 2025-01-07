#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include "view_parameters.hpp"
#include <QDialog>

namespace Ui {
class GoToDialog;
}

class GoToDialog : public QDialog {
  Q_OBJECT

public:
  explicit GoToDialog(QWidget *parent = nullptr);
  ~GoToDialog();

  void getCoords(fractals::view_parameters &params) const;
  void setCoords(const fractals::view_parameters &params);

private:
  Ui::GoToDialog *ui;
  void accept() override;
};

#endif // GOTODIALOG_H
