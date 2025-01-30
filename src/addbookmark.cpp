#include "addbookmark.h"
#include "ui_addbookmark.h"

AddBookmark::AddBookmark(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddBookmark)
{
    ui->setupUi(this);
}

AddBookmark::~AddBookmark()
{
    delete ui;
}

QString AddBookmark::getName() const { return ui->nameEdit->text(); }