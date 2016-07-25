#include "aboutdialog.hh"
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFile>

AboutDialog::AboutDialog(QWidget *parent)
  : QDialog(parent)
{
  QLabel *text = new QLabel();
  QFile about("://about.html");
  about.open(QIODevice::ReadOnly);
  text->setTextFormat(Qt::RichText);
  text->setWordWrap(true);
  text->setText(about.readAll());
  text->setMaximumWidth(300);

  QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(text);
  layout->addWidget(bb);

  setLayout(layout);

  connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
  connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
}
