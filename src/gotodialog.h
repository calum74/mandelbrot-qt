#ifndef GOTODIALOG_H
#define GOTODIALOG_H

#include <QDialog>

namespace Ui {
class GoToDialog;
}

class GoToDialog : public QDialog {
  Q_OBJECT

public:
  explicit GoToDialog(QWidget *parent = nullptr);
  ~GoToDialog();

  void getCoords(QString &x, QString &y, QString &r, QString &i) const;
  void setCoords(const QString &x, const QString &y, const QString &r,
                 const QString &i);

private:
  Ui::GoToDialog *ui;
  void accept() override;
};

#endif // GOTODIALOG_H
