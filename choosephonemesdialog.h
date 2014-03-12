#ifndef CHOOSEPHONEMESDIALOG_H
#define CHOOSEPHONEMESDIALOG_H

#include <QDialog>

#include <QList>

#include "cdicdatabase.h"

class QCheckBox;
class QPushButton;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;

class ChoosePhonemesDialog : public QDialog  {
  Q_OBJECT
  
  public:
    ChoosePhonemesDialog (CDICDatabase, QString);

  signals:
    void done ();
    
  private slots:
    void clear ();
    void selectAll ();
    void compileChoices ();

  private:
    CDICDatabase db;
    QString supraName;
    
    QList<QCheckBox*> phonemeBoxList;
    QPushButton *chooseButton;
    QPushButton *clearButton;
    QPushButton *selectAllButton;

    QGridLayout *phonemeLayout;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
};

#endif