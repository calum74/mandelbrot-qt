#ifndef ADDBOOKMARK_H
#define ADDBOOKMARK_H

#include <QDialog>

namespace Ui {
class AddBookmark;
}

class AddBookmark : public QDialog
{
    Q_OBJECT

public:
    explicit AddBookmark(QWidget *parent = nullptr);
    ~AddBookmark();
    QString getName() const;

  private:
    Ui::AddBookmark *ui;
};

#endif // ADDBOOKMARK_H
