#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

#include <QGridLayout>
#include <QVBoxLayout>

#include "choosephonemesdialog.h"

ChoosePhonemesDialog::ChoosePhonemesDialog (CDICDatabase data, QString sup) {
  db = data;
  supraName = sup;
  
  QStringList phonemeList = db.getAllPhonemeNames ();
  QStringList applies = db.getSupraApplies (supraName);

  for (int x = 0; x < phonemeList.size (); x++)  {
    phonemeBoxList.append (new QCheckBox (phonemeList[x]));
    phonemeBoxList[x]->setChecked (applies.contains (phonemeList[x]));
  }

  int numCols = 4;
  if (phonemeBoxList.size () < 16) numCols = 3;
  if (phonemeBoxList.size () < 9) numCols = 2;
  
  phonemeLayout = new QGridLayout;
  
  for (int x = 0; x < phonemeBoxList.size (); x++)
    phonemeLayout->addWidget (phonemeBoxList[x], x/numCols, x % numCols);

  QLabel *choosePhonemesLabel = new QLabel ("Choose phonemes:");

  clearButton = new QPushButton ("Clear");
  selectAllButton = new QPushButton ("Select All");
  chooseButton = new QPushButton ("Done");
  buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget (clearButton);
  buttonLayout->addWidget (selectAllButton);
  buttonLayout->addWidget (chooseButton);
  
  mainLayout = new QVBoxLayout;
  mainLayout->addWidget (choosePhonemesLabel);
  mainLayout->setAlignment (choosePhonemesLabel, Qt::AlignCenter);
  mainLayout->addLayout (phonemeLayout);
  mainLayout->addLayout (buttonLayout);

  setLayout (mainLayout);

  connect (clearButton, SIGNAL (clicked ()), this, SLOT (clear ()));
  connect (selectAllButton, SIGNAL (clicked ()), this, SLOT (selectAll ()));
  connect (chooseButton, SIGNAL (clicked ()), this, SLOT (compileChoices ()));
  connect (chooseButton, SIGNAL (clicked ()), this, SLOT (hide ()));
}

void ChoosePhonemesDialog::clear ()  {
  for (int x = 0; x < phonemeBoxList.size (); x++)
    phonemeBoxList[x]->setChecked (false);
}

void ChoosePhonemesDialog::selectAll ()  {
  for (int x = 0; x < phonemeBoxList.size (); x++)
    phonemeBoxList[x]->setChecked (true);
}

void ChoosePhonemesDialog::compileChoices ()  {
  QStringList choices;
  
  for (int x = 0; x < phonemeBoxList.size (); x++)
    if (phonemeBoxList[x]->isChecked ())
      choices.append (phonemeBoxList[x]->text ());
  
  db.setSupraApplies (supraName, choices);
  
  emit done ();
}