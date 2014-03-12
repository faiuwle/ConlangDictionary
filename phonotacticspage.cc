#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QStringListModel>

#include <iostream>
using namespace std;

#include "const.h"

#include "phonotacticspage.h"

PhonotacticsPage::PhonotacticsPage ()  {
  currentInd = 0;
  
  onsetRequiredBox = new QCheckBox ("Onset Required");
  QLabel *ignoreLabel = new QLabel ("Ignored Characters: ");
  ignoreEdit = new QLineEdit;
  usePhonotacticsBox = new QCheckBox ("Use Phonotactics");
  
  topLayout = new QHBoxLayout;
  topLayout->addWidget (onsetRequiredBox);
  topLayout->setAlignment (onsetRequiredBox, Qt::AlignLeft);
  topLayout->addWidget (ignoreLabel);
  topLayout->setAlignment (ignoreLabel, Qt::AlignRight);
  topLayout->addWidget (ignoreEdit);
  topLayout->setAlignment (ignoreEdit, Qt::AlignLeft);
  topLayout->addWidget (usePhonotacticsBox);
  topLayout->setAlignment (usePhonotacticsBox, Qt::AlignRight);
  
  QLabel *onsetLabel = new QLabel ("<b>Onsets</b>");
  deleteOnsetButton = new QPushButton ("Delete");
//  deleteOnsetButton->setEnabled (false);
  onsetModel = new QStringListModel ();
  onsetView = new QListView;
  onsetView->setModel (onsetModel);
  onsetView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  addOnsetButton = new QPushButton ("Add As Onset");
  
  onsetTopLayout = new QHBoxLayout;
  onsetTopLayout->addWidget (onsetLabel);
  onsetTopLayout->addWidget (deleteOnsetButton);
  onsetTopLayout->setAlignment (deleteOnsetButton, Qt::AlignRight);
  onsetLayout = new QVBoxLayout;
  onsetLayout->addLayout (onsetTopLayout);
  onsetLayout->addWidget (onsetView);
  onsetLayout->addWidget (addOnsetButton);
  onsetLayout->setAlignment (addOnsetButton, Qt::AlignCenter);
  
  QLabel *peakLabel = new QLabel ("<b>Peaks</b>");
  deletePeakButton = new QPushButton ("Delete");
//  deletePeakButton->setEnabled (false);
  peakModel = new QStringListModel ();
  peakView = new QListView;
  peakView->setModel (peakModel);
  peakView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  addPeakButton = new QPushButton ("Add As Peak");
  
  peakTopLayout = new QHBoxLayout;
  peakTopLayout->addWidget (peakLabel);
  peakTopLayout->addWidget (deletePeakButton);
  peakTopLayout->setAlignment (deletePeakButton, Qt::AlignRight);
  peakLayout = new QVBoxLayout;
  peakLayout->addLayout (peakTopLayout);
  peakLayout->addWidget (peakView);
  peakLayout->addWidget (addPeakButton);
  peakLayout->setAlignment (addPeakButton, Qt::AlignCenter);
  
  QLabel *codaLabel = new QLabel ("<b>Codas</b>");
  deleteCodaButton = new QPushButton ("Delete");
//  deleteCodaButton->setEnabled (false);
  codaModel = new QStringListModel ();
  codaView = new QListView;
  codaView->setModel (codaModel);
  codaView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  addCodaButton = new QPushButton ("As As Coda");
  
  codaTopLayout = new QHBoxLayout;
  codaTopLayout->addWidget (codaLabel);
  codaTopLayout->addWidget (deleteCodaButton);
  codaTopLayout->setAlignment (deleteCodaButton, Qt::AlignRight);
  codaLayout = new QVBoxLayout;
  codaLayout->addLayout (codaTopLayout);
  codaLayout->addWidget (codaView);
  codaLayout->addWidget (addCodaButton);
  codaLayout->setAlignment (addCodaButton, Qt::AlignCenter);
  
  centralLayout = new QHBoxLayout;
  centralLayout->addLayout (onsetLayout);
  centralLayout->addLayout (peakLayout);
  centralLayout->addLayout (codaLayout);
  
  backButton = new QPushButton ("<");
  forwardButton = new QPushButton (">");
  arrowLayout = new QHBoxLayout;
  arrowLayout->addStretch (2);
  arrowLayout->addWidget (backButton);
  arrowLayout->addStretch (1);
  arrowLayout->addWidget (forwardButton);
  arrowLayout->addStretch (2);
  
  displayLabel = new QLabel;
  displayLabel->setAlignment (Qt::AlignCenter);
  
  addClassButton = new QPushButton ("Add Class:");
  addClassBox = new QComboBox;
  addClassBox->setSizeAdjustPolicy (QComboBox::AdjustToContents);
  removeClassButton = new QPushButton ("Remove Class:");
  removeClassBox = new QComboBox;
  removeClassBox->setSizeAdjustPolicy (QComboBox::AdjustToContents);
  addRemoveClassLayout = new QHBoxLayout;
  addRemoveClassLayout->addStretch (1);
  addRemoveClassLayout->addWidget (addClassButton);
  addRemoveClassLayout->addWidget (addClassBox);
  addRemoveClassLayout->addWidget (removeClassButton);
  addRemoveClassLayout->addWidget (removeClassBox);
  addRemoveClassLayout->addStretch (1);
  
  reanalyzeButton = new QPushButton ("Reanalyze Lexicon");
  progressBar = new QProgressBar;
  progressBar->setMinimum (0);
  progressBar->setMaximum (0);
  progressBar->setValue (0);
  
  mainLayout = new QVBoxLayout;
  mainLayout->addLayout (topLayout);
  mainLayout->addStretch (1);
  mainLayout->addLayout (centralLayout);
  mainLayout->addStretch (1);
  mainLayout->addLayout (arrowLayout);
  mainLayout->addWidget (displayLabel);
  mainLayout->addLayout (addRemoveClassLayout);
  mainLayout->addStretch (1);
  mainLayout->addWidget (reanalyzeButton);
  mainLayout->setAlignment (reanalyzeButton, Qt::AlignCenter);
  mainLayout->addWidget (progressBar);
  mainLayout->setAlignment (progressBar, Qt::AlignCenter);
  
  setLayout (mainLayout);
  
  connect (onsetRequiredBox, SIGNAL (toggled (bool)), this, 
           SLOT (setOnsetRequired (bool)));
  connect (ignoreEdit, SIGNAL (textEdited (QString)), this, 
           SLOT (setIgnored (QString)));
  connect (usePhonotacticsBox, SIGNAL (toggled (bool)), this,
           SLOT (setUsePhonotactics (bool)));
  
  connect (addClassButton, SIGNAL (clicked ()), this, SLOT (addClass ()));
  connect (removeClassButton, SIGNAL (clicked ()), this, SLOT (removeClass ()));
  connect (backButton, SIGNAL (clicked ()), this, SLOT (goLeft ()));
  connect (forwardButton, SIGNAL (clicked ()), this, SLOT (goRight ()));
  
  connect (addOnsetButton, SIGNAL (clicked ()), this, SLOT (addAsOnset ()));
  connect (addPeakButton, SIGNAL (clicked ()), this, SLOT (addAsPeak ()));
  connect (addCodaButton, SIGNAL (clicked ()), this, SLOT (addAsCoda ()));

  connect (deleteOnsetButton, SIGNAL (clicked ()), this, SLOT (deleteOnset ()));
  connect (deletePeakButton, SIGNAL (clicked ()), this, SLOT (deletePeak ()));
  connect (deleteCodaButton, SIGNAL (clicked ()), this, SLOT (deleteCoda ()));
  
  connect (reanalyzeButton, SIGNAL (clicked ()), this, SLOT (startReanalyze ()));
}

void PhonotacticsPage::clearDB ()  {
  db.clear ();
  
  onsetRequiredBox->setChecked (false);
  ignoreEdit->setText ("");
  usePhonotacticsBox->setChecked (true);
  addClassBox->clear ();
  clearCurrentSequence ();
  
  onsetModel->setStringList (QStringList ());
  peakModel->setStringList (QStringList ());
  codaModel->setStringList (QStringList ());
}

void PhonotacticsPage::setDB (CDICDatabase data)  {
  db = data;
  
  onsetRequiredBox->setChecked (db.getValue (ONSET_REQUIRED) == "true");
  ignoreEdit->setText (db.getValue (IGNORED_CHARACTERS));
  usePhonotacticsBox->setChecked (db.getValue (USE_PHONOTACTICS) != "false");
  addClassBox->addItems (db.getClassList (PHONEME));
  
  onsetModel->setStringList (db.getSequenceList (ONSET));
  peakModel->setStringList (db.getSequenceList (PEAK));
  codaModel->setStringList (db.getSequenceList (CODA));
}

void PhonotacticsPage::updateModels ()  {
  onsetModel->setStringList (db.getSequenceList (ONSET));
  peakModel->setStringList (db.getSequenceList (PEAK));
  codaModel->setStringList (db.getSequenceList (CODA));
  
  addClassBox->clear ();
  addClassBox->addItems (db.getClassList (PHONEME));
}

void PhonotacticsPage::updateClassList ()  {
  addClassBox->clear ();
  addClassBox->addItems (db.getClassList (PHONEME));
}

void PhonotacticsPage::incrementProgressBar ()  {
  progressBar->setValue (progressBar->value () + 1);
}

void PhonotacticsPage::resetProgressBar ()  {
  progressBar->setValue (0);
}

void PhonotacticsPage::setOnsetRequired (bool o)  {
  o ? db.setValue (ONSET_REQUIRED, "true") 
    : db.setValue (ONSET_REQUIRED, "false");
}

void PhonotacticsPage::setIgnored (QString i)  {
  db.setValue (IGNORED_CHARACTERS, i);
}

void PhonotacticsPage::setUsePhonotactics (bool u)  {
  u ? db.setValue (USE_PHONOTACTICS, "true")
    : db.setValue (USE_PHONOTACTICS, "false");
}

void PhonotacticsPage::addClass ()  {
  if (currentSequence.isEmpty ())
    currentSequence.append (QStringList ());
  
  if (!currentSequence[currentInd].contains (addClassBox->currentText ()))
    currentSequence[currentInd].append (addClassBox->currentText ());
  
  displayCurrentSequence ();
}

void PhonotacticsPage::removeClass ()  {
  if (currentSequence.isEmpty () || removeClassBox->currentText () == "") 
    return;
  
  currentSequence[currentInd].removeAll (removeClassBox->currentText ());
  
  displayCurrentSequence ();
}

void PhonotacticsPage::goLeft ()  {
  if (currentInd != 0)
    currentInd--;
  
  displayCurrentSequence ();
}

void PhonotacticsPage::goRight ()  {
  while (currentInd >= (currentSequence.length () - 1))
    currentSequence.append (QStringList ());
    
  currentInd++;
  
  displayCurrentSequence ();
}

void PhonotacticsPage::addAsOnset ()  {
  db.addSequence (ONSET, currentSequence);
  
  onsetModel->setStringList (db.getSequenceList (ONSET));
  
  clearCurrentSequence ();
  emit phonotacticsChanged ();
}

void PhonotacticsPage::addAsPeak ()  {
  db.addSequence (PEAK, currentSequence);
  
  peakModel->setStringList (db.getSequenceList (PEAK));

  clearCurrentSequence ();
  emit phonotacticsChanged ();
}

void PhonotacticsPage::addAsCoda ()  {
  db.addSequence (CODA, currentSequence);
  
  codaModel->setStringList (db.getSequenceList (CODA));
  
  clearCurrentSequence ();
  emit phonotacticsChanged ();
}

void PhonotacticsPage::deleteOnset ()  {
  if (!onsetView->selectionModel()->currentIndex ().isValid ())
    return;
  
  int index = onsetView->selectionModel ()->currentIndex ().row ();
  
  db.removeSequence (ONSET, index + 1);
  
  onsetModel->setStringList (db.getSequenceList (ONSET));
  emit phonotacticsChanged ();
}

void PhonotacticsPage::deletePeak ()  {
  if (!peakView->selectionModel()->currentIndex ().isValid ())
    return;
  
  int index = peakView->selectionModel ()->currentIndex ().row ();
  
  db.removeSequence (PEAK, index + 1);
  
  peakModel->setStringList (db.getSequenceList (PEAK));
  emit phonotacticsChanged ();
}

void PhonotacticsPage::deleteCoda ()  {
  if (!codaView->selectionModel()->currentIndex ().isValid ())
    return;
  
  int index = codaView->selectionModel ()->currentIndex ().row ();
  
  db.removeSequence (CODA, index + 1);
  
  codaModel->setStringList (db.getSequenceList (CODA));
  emit phonotacticsChanged ();
}

void PhonotacticsPage::startReanalyze ()  {
  cout << db.getNumberOfWords ();
  progressBar->setMaximum (db.getNumberOfWords ());
  emit reanalyze ();
}

void PhonotacticsPage::displayCurrentSequence ()  {
  if (currentSequence.length () == 0)  {
    displayLabel->setText ("");
    return;
  }
  
  QString text = "";
  
  for (int x = 0; x < currentSequence.length (); x++)  {
    if (currentInd == x)
      text += "<b>";
    
    for (int c = 0; c < currentSequence[x].length (); c++)  {
      text += currentSequence[x][c];
      
      if (c != (currentSequence[x].length () - 1))
        text += " / ";
    }
    
    if (currentInd == x)
      text += "</b>";
    
    if (x != (currentSequence.length () - 1))
      text += " + ";
  }
  
  removeClassBox->clear ();
  removeClassBox->addItems (currentSequence[currentInd]);
  
  displayLabel->setText (text);
}

void PhonotacticsPage::clearCurrentSequence ()  {
  currentSequence.clear ();
  currentInd = 0;
  displayLabel->setText ("");
  removeClassBox->clear ();
}