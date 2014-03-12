#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QTextEdit>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QListView>
#include <QDataWidgetMapper>

#include <QTextStream>

#include <iostream>
using namespace std;

#include "const.h"

#include "suprasegmentalspage.h"

#include "choosephonemesdialog.h"

SuprasegmentalsPage::SuprasegmentalsPage ()  {
  suprasegmentalModel = NULL;
  displayModel = NULL;
  mapper = NULL;

  choosePhonemesDialog = NULL;

  addSuprasegmentalButton = new QPushButton ("Add Suprasegmental:");
  addSuprasegmentalButton->setMaximumSize (addSuprasegmentalButton->sizeHint ());
  addSuprasegmentalEdit = new QLineEdit;
  addSuprasegmentalEdit->setMinimumWidth (100);
  addSuprasegmentalLayout = new QHBoxLayout;
  deleteButton = new QPushButton ("Delete");
  deleteButton->setMaximumSize (deleteButton->sizeHint ());
//  deleteButton->setEnabled (false);
  addSuprasegmentalLayout->addWidget (addSuprasegmentalButton);
  addSuprasegmentalLayout->addWidget (addSuprasegmentalEdit);
  addSuprasegmentalLayout->addWidget (deleteButton);

  suprasegmentalListView = new QListView;
  suprasegmentalListView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  suprasegmentalListView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  suprasegmentalListLayout = new QVBoxLayout;
  suprasegmentalListLayout->addLayout (addSuprasegmentalLayout);
  suprasegmentalListLayout->addWidget (suprasegmentalListView);

  QLabel *nameLabel = new QLabel ("Name:");
  nameEdit = new QLineEdit;
  nameLayout = new QHBoxLayout;
  nameLayout->addWidget (nameLabel);
  nameLayout->addWidget (nameEdit);

  QLabel *domainLabel = new QLabel ("Applies to:");
  domainBox = new QComboBox;
  domainBox->addItems (supraDomain);
  domainLayout = new QHBoxLayout;
  domainLayout->addWidget (domainLabel);
  domainLayout->setAlignment (domainLabel, Qt::AlignRight);
  domainLayout->addWidget (domainBox);
  domainLayout->setAlignment (domainBox, Qt::AlignLeft);

  QLabel *phonemesLabel = new QLabel ("Applicable Phonemes:");
  applicablePhonemesLabel = new QLabel;
  editPhonemesButton = new QPushButton ("Edit Phonemes");
  editPhonemesButton->setMaximumSize (editPhonemesButton->sizeHint ());
//  editPhonemesButton->setEnabled (false);
  phonemesLayout = new QHBoxLayout;
  phonemesLayout->addWidget (phonemesLabel);
  phonemesLayout->addWidget (applicablePhonemesLabel);
  phonemesLayout->addWidget (editPhonemesButton);

  QLabel *spellingLabel = new QLabel ("Spelled with:");
  spellTypeBox = new QComboBox;
  spellTypeBox->addItems (supraType);
  spellTextEdit = new QLineEdit;

  QLabel *representationLabel = new QLabel ("Represented Phonemically with: ");
  repTypeBox = new QComboBox;
  repTypeBox->addItems (supraType);
  repTextEdit = new QLineEdit;

  spellRepLayout = new QGridLayout;
  spellRepLayout->addWidget (spellingLabel, 0, 0);
  spellRepLayout->addWidget (spellTypeBox, 0, 1);
  spellRepLayout->addWidget (spellTextEdit, 0, 2);
  spellRepLayout->addWidget (representationLabel, 1, 0);
  spellRepLayout->addWidget (repTypeBox, 1, 1);
  spellRepLayout->addWidget (repTextEdit, 1, 2);

  QLabel *descriptionLabel = new QLabel ("Description:");
  descriptionEdit = new QTextEdit;
  descriptionEdit->setEnabled (false);
  submitButton = new QPushButton ("Submit");
  submitButton->setMaximumSize (submitButton->sizeHint ());

  infoLayout = new QVBoxLayout;
  infoLayout->addLayout (nameLayout);
  infoLayout->addLayout (domainLayout);
  infoLayout->addLayout (phonemesLayout);
  infoLayout->addLayout (spellRepLayout);
  infoLayout->addWidget (descriptionLabel);
  infoLayout->addWidget (descriptionEdit);
  infoLayout->addWidget (submitButton);
  infoLayout->setAlignment (submitButton, Qt::AlignCenter);

  mainLayout = new QHBoxLayout;
  mainLayout->addLayout (suprasegmentalListLayout);
  mainLayout->addLayout (infoLayout);

  setLayout (mainLayout);

  connect (addSuprasegmentalButton, SIGNAL (clicked ()), this,
           SLOT (addSuprasegmental ()));
  connect (addSuprasegmentalButton, SIGNAL (clicked ()), addSuprasegmentalEdit,
           SLOT (clear ()));
  connect (deleteButton, SIGNAL (clicked ()), this,
           SLOT (deleteSuprasegmental ()));
  connect (submitButton, SIGNAL (clicked ()), this, SLOT (submitChanges ()));
           
  connect (nameEdit, SIGNAL (textEdited (QString)), this, SLOT (setChanged ()));
  connect (domainBox, SIGNAL (currentIndexChanged (int)), this, SLOT (setChanged ()));
  connect (spellTypeBox, SIGNAL (currentIndexChanged (int)), this, SLOT (setChanged ()));
  connect (spellTextEdit, SIGNAL (textEdited (QString)), this, SLOT (setChanged ()));
  connect (repTypeBox, SIGNAL (currentIndexChanged (int)), this, SLOT (setChanged ()));
  connect (repTextEdit, SIGNAL (textEdited (QString)), this, SLOT (setChanged ()));
  connect (descriptionEdit, SIGNAL (textChanged ()), this, SLOT (setChanged ()));
           
  connect (editPhonemesButton, SIGNAL (clicked ()), this,
           SLOT (launchPhonemesDialog ()));
}

void SuprasegmentalsPage::clearDB ()  {
  if (suprasegmentalModel)
    suprasegmentalModel->clear ();
  
  if (mapper)  {
    mapper->setModel (NULL);
    
    if (displayModel)  {
      displayModel->clear ();
      delete displayModel;
    }
  }
  
  db.close ();
}

void SuprasegmentalsPage::setDB (CDICDatabase database)  {
//  clearDB ();
  
  QAbstractItemModel *oldModel = suprasegmentalModel;
  
  db = database;
  suprasegmentalModel = db.getSupraListModel ();
  suprasegmentalListView->setModel (suprasegmentalModel);
  
  if (oldModel) 
    delete oldModel;
  
  connect (suprasegmentalListView->selectionModel (),
           SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
           this, SLOT (displaySuprasegmental ()));
           
  if (!mapper)
    mapper = new QDataWidgetMapper (this);
  
  displayModel = db.getSupraDisplayModel ();
  
  mapper->setModel (displayModel);
  mapper->setSubmitPolicy (QDataWidgetMapper::ManualSubmit);
  mapper->addMapping (nameEdit, 1);
  mapper->addMapping (domainBox, 2, "currentIndex");
  mapper->addMapping (repTypeBox, 3, "currentIndex");
  mapper->addMapping (repTextEdit, 4);
  mapper->addMapping (spellTypeBox, 5, "currentIndex");
  mapper->addMapping (spellTextEdit, 6);
  mapper->addMapping (descriptionEdit, 7);
  mapper->toFirst ();
  
  submitButton->setEnabled (false);
}

void SuprasegmentalsPage::updateModels ()  {
  if (!displayModel || !suprasegmentalModel || !mapper)
    return;
  
  suprasegmentalModel->setQuery ("select name from Suprasegmental order by id");
  displayModel->select ();
  submitButton->setEnabled (false);
}

void SuprasegmentalsPage::addSuprasegmental ()  {
  if (addSuprasegmentalEdit->text () == "") return;

  db.addSupra (addSuprasegmentalEdit->text ());
  
  updateModels ();
  emit suprasChanged ();
}

void SuprasegmentalsPage::deleteSuprasegmental ()  {
  QModelIndexList indexList = suprasegmentalListView->selectionModel ()->selectedRows ();
  
  for (int x = 0; x < indexList.size (); x++)
    db.deleteSupra (indexList[x].data ().toString ());
  
  updateModels ();
  emit suprasChanged ();
}

void SuprasegmentalsPage::displaySuprasegmental ()  {
  if (!suprasegmentalListView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  mapper->setCurrentIndex (suprasegmentalListView->selectionModel ()->currentIndex ().row ());
  applicablePhonemesLabel->setText (db.getSupraApplies (nameEdit->text ()).join (" "));
  
  submitButton->setEnabled (false);
}

void SuprasegmentalsPage::setChanged ()  {
  submitButton->setEnabled (true);
  
  if (domainBox->currentIndex () == SUPRA_DOMAIN_SYLL)
    editPhonemesButton->setEnabled (false);
  else editPhonemesButton->setEnabled (true);
}

void SuprasegmentalsPage::launchPhonemesDialog ()  {
  if (!mapper || !displayModel) return;
  
  if (choosePhonemesDialog)
    delete choosePhonemesDialog;
  
  choosePhonemesDialog = new ChoosePhonemesDialog (db, nameEdit->text ());
  
  connect (choosePhonemesDialog, SIGNAL (done ()), this, SLOT (displaySuprasegmental ()));
  connect (choosePhonemesDialog, SIGNAL (done ()), this, SIGNAL (suprasChanged ()));
  
  choosePhonemesDialog->show ();
}

void SuprasegmentalsPage::submitChanges ()  {
  if (!mapper) return;
  
  QModelIndex index = suprasegmentalListView->currentIndex ();
  
  mapper->submit ();
  displayModel->submitAll ();
  
  updateModels ();
  
  if (index.isValid ())
    suprasegmentalListView->selectionModel ()->setCurrentIndex (index, QItemSelectionModel::Select);
  
  emit suprasChanged ();
}