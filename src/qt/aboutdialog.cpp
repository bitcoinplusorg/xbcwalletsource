#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "clientmodel.h"
#include "util.h"
#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    if ((mapArgs["-torconnect"] == "1")){
        ui->about_lbl->setPixmap(QPixmap(":/images/about_dark"));
    } else {
       ui->about_lbl->setPixmap(QPixmap(":/images/about"));
    }


}

void AboutDialog::setModel(ClientModel *model)
{
    if(model)
    {
        ui->versionLabel->setText(model->formatFullVersion());
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_buttonBox_accepted()
{
    close();
}
