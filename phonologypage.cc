#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QListView>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QDataWidgetMapper>

#include <QTextStream>
#include <QMessageBox>

#include <QSignalMapper>
#include <QtAlgorithms>

#include <iostream>
using namespace std;

#include "const.h"

#include "phonologypage.h"
#include "cdicdatabase.h"
#include "editablequerymodel.h"

#include "managefeaturesdialog.h"
#include "featurebundlesdialog.h"

const QMap<QString, QString> PhonologyPage::ipaTable = initIPATable ();

PhonologyPage::PhonologyPage ()  {
  featuresDialog = NULL;
  featureBundlesDialog = NULL;

  phonemeModel = NULL;
  displayModel = NULL;
  mapper = NULL;

  squareBrackets = false;
  if (db.getValue (SQUARE_BRACKETS) == "true")
    squareBrackets = true;

  addPhonemeButton = new QPushButton ("Add Phoneme(s)");
  addPhonemeButton->setMaximumSize (addPhonemeButton->sizeHint ());
  addPhonemeEdit = new QLineEdit ();
  addPhonemeLayout = new QHBoxLayout;
  addPhonemeLayout->addWidget (addPhonemeButton);
  addPhonemeLayout->addWidget (addPhonemeEdit);

  phonemeListView = new QListView;
  phonemeListView->setSelectionBehavior (QAbstractItemView::SelectRows);
  phonemeListView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  phonemeListView->setSelectionMode (QAbstractItemView::ExtendedSelection);

  manageFeaturesButton = new QPushButton ("Manage Features");
//  manageFeaturesButton->setMaximumSize (manageFeaturesButton->sizeHint ());
  manageNaturalClassesButton = new QPushButton ("Manage Natural Classes");
//  manageNaturalClassesButton->setMaximumSize (manageNaturalClassesButton->sizeHint ());
  dialogButtonsLayout = new QHBoxLayout;
  dialogButtonsLayout->addWidget (manageFeaturesButton);
  dialogButtonsLayout->addWidget (manageNaturalClassesButton);

  assignNaturalClassButton = new QPushButton ("Assign to natural class:");
  assignNaturalClassBox = new QComboBox;
  assignNaturalClassLayout = new QHBoxLayout;
  assignNaturalClassLayout->addWidget (assignNaturalClassButton);
  assignNaturalClassLayout->addWidget (assignNaturalClassBox);

  listLayout = new QVBoxLayout;
  listLayout->addLayout (addPhonemeLayout);
  listLayout->addWidget (phonemeListView);
  listLayout->addLayout (dialogButtonsLayout);
  listLayout->addLayout (assignNaturalClassLayout);

  moveUpButton = new QPushButton ("Move Up");
  moveUpButton->setMaximumSize (moveUpButton->sizeHint ());
  moveDownButton = new QPushButton ("Move Down");
  moveDownButton->setMaximumSize (moveDownButton->sizeHint ());
  deleteButton = new QPushButton ("Delete");
  deleteButton->setMaximumSize (deleteButton->sizeHint ());
  moveUpDownLayout = new QVBoxLayout;
  moveUpDownLayout->addStretch (2);
  moveUpDownLayout->addWidget (moveUpButton);
  moveUpDownLayout->setAlignment (moveUpButton, Qt::AlignCenter);
  moveUpDownLayout->addStretch (1);
  moveUpDownLayout->addWidget (moveDownButton);
  moveUpDownLayout->addStretch (1);
  moveUpDownLayout->addWidget (deleteButton);
  moveUpDownLayout->setAlignment (deleteButton, Qt::AlignCenter);

  QLabel *phonemeNameLabel = new QLabel ("Phoneme Name:");
  phonemeNameEdit = new QLineEdit ();
  ipaButton = new QPushButton ("XSAMPA -> IPA");
  ipaButton->setMaximumSize (ipaButton->sizeHint ());
  phonemeNameLayout = new QHBoxLayout;
  phonemeNameLayout->addWidget (phonemeNameLabel);
  phonemeNameLayout->addWidget (phonemeNameEdit);
  phonemeNameLayout->addWidget (ipaButton);

  QLabel *phonemeSpellingLabel =
        new QLabel ("Spellings (separated by spaces):");
  phonemeSpellingEdit = new QLineEdit ();
  phonemeSpellingLayout = new QHBoxLayout;
  phonemeSpellingLayout->addWidget (phonemeSpellingLabel);
  phonemeSpellingLayout->addWidget (phonemeSpellingEdit);

  naturalClassDisplay = new QLabel ("");
  featuresDisplay = new QLabel ("Features:\n\n");
  featuresDisplay->setWordWrap (true);
  editFeaturesButton = new QPushButton ("Add/Edit Features");
  editFeaturesButton->setMaximumSize (editFeaturesButton->sizeHint ());

  QLabel *notesLabel = new QLabel ("Notes:");
  notesEdit = new QTextEdit ();

  submitButton = new QPushButton ("Submit");
  submitButton->setMaximumSize (submitButton->sizeHint ());
  submitButton->setEnabled (false);

  phonemeInfoLayout = new QVBoxLayout;
  phonemeInfoLayout->addLayout (phonemeNameLayout);
  phonemeInfoLayout->addLayout (phonemeSpellingLayout);
  phonemeInfoLayout->addWidget (naturalClassDisplay);
  phonemeInfoLayout->addWidget (featuresDisplay);
  phonemeInfoLayout->addWidget (editFeaturesButton);
  phonemeInfoLayout->setAlignment (editFeaturesButton, Qt::AlignCenter);
  phonemeInfoLayout->addWidget (notesLabel);
  phonemeInfoLayout->addWidget (notesEdit);
  phonemeInfoLayout->addWidget (submitButton);
  phonemeInfoLayout->setAlignment (submitButton, Qt::AlignCenter);

  mainLayout = new QHBoxLayout;
  mainLayout->addLayout (listLayout);
  mainLayout->addLayout (moveUpDownLayout);
  mainLayout->addLayout (phonemeInfoLayout);

  setLayout (mainLayout);

  connect (addPhonemeButton, SIGNAL (clicked ()), this, SLOT (addPhoneme ()));
  connect (moveUpButton, SIGNAL (clicked ()), this, SLOT (moveUp ()));
  connect (moveDownButton, SIGNAL (clicked ()), this, SLOT (moveDown ()));
  connect (deleteButton, SIGNAL (clicked ()), this, SLOT (deletePhoneme ()));
  connect (ipaButton, SIGNAL (clicked ()), this, SLOT (convertToIPA ()));
  connect (phonemeNameEdit, SIGNAL (textEdited (QString)), this, 
           SLOT (setChanged ()));
  connect (phonemeSpellingEdit, SIGNAL (textEdited (QString)), this,
           SLOT (setChanged ()));
  connect (editFeaturesButton, SIGNAL (clicked ()), this, SLOT (launchEditFeaturesDialog ()));
  connect (notesEdit, SIGNAL (textChanged ()), this, SLOT (setChanged ()));
  connect (manageFeaturesButton, SIGNAL (clicked ()), this, SLOT (launchFeaturesDialog ()));
  connect (manageNaturalClassesButton, SIGNAL (clicked ()), this,
           SLOT (launchNaturalClassesDialog ()));
  connect (assignNaturalClassButton, SIGNAL (clicked ()), this, SLOT (applyNaturalClass ()));
  connect (submitButton, SIGNAL (clicked ()), this, SLOT (submitChanges ()));
}

void PhonologyPage::clearDB ()  {
  if (phonemeModel)
    phonemeModel->clear ();

  if (mapper)  {
    mapper->setModel (NULL);

    if (displayModel)
      displayModel->clear ();
  }
  
  assignNaturalClassBox->clear ();
  
  db.close ();
}

void PhonologyPage::setDB (CDICDatabase database)  {
  QSqlQueryModel *pModel = phonemeModel;
  
  db = database;
  phonemeModel = db.getPhonemeListModel ();
  phonemeListView->setModel (phonemeModel);
  
  if (pModel)  {
    disconnect (pModel);
    delete pModel;
  }
  
  connect (phonemeListView->selectionModel (), 
           SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)), 
           this, SLOT (displayPhoneme ()));
  
  if (!mapper) 
    mapper = new QDataWidgetMapper (this);
  
  QSqlTableModel *dModel = displayModel;
  
  displayModel = db.getPhonemeDisplayModel ();  
  
  mapper->setModel (displayModel);
  mapper->setSubmitPolicy (QDataWidgetMapper::ManualSubmit);
  mapper->addMapping (phonemeNameEdit, 2);
  mapper->addMapping (notesEdit, 3);
  mapper->toFirst ();
  
  if (dModel)  {
    disconnect (dModel);
    delete dModel;
  }
  
  updateModels ();
}

// Note: destroys selection and current item in listview, caller must save
void PhonologyPage::updateModels ()  {
  if (!displayModel) return;
  
  phonemeModel->setQuery ("select name from Phoneme order by alpha");
  displayModel->select ();
  assignNaturalClassBox->clear ();
  assignNaturalClassBox->addItems (db.getClassList (PHONEME));

  submitButton->setEnabled (false);
}

void PhonologyPage::setChanged ()  {
  submitButton->setEnabled (true);
}

void PhonologyPage::addPhoneme ()  {
  if (addPhonemeEdit->text () == "") return;

  QStringList newPhonemes = addPhonemeEdit->text ().split (' ');

  for (int x = 0; x < newPhonemes.size (); x++)
    db.addPhoneme (newPhonemes[x]);

  updateModels ();
  
  addPhonemeEdit->setText ("");
  
  emit spellingsChanged ();
}

void PhonologyPage::displayPhoneme ()  {
  if (!phonemeListView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  mapper->setCurrentIndex (phonemeListView->selectionModel ()->currentIndex ().row ());
  
  int currentPhonID = displayModel->record (mapper->currentIndex ()).value ("id").toInt ();
  
  phonemeSpellingEdit->setText (db.getSpellingText (phonemeNameEdit->text ()));
  naturalClassDisplay->setText ("<b>" + db.getClassList (PHONEME, currentPhonID).join (", ") + "</b>");
  featuresDisplay->setText ("Features:\n[" + db.getBundledFeatures (PHONEME, currentPhonID).join (", ") + "]");
  submitButton->setEnabled (false);
}

void PhonologyPage::moveUp ()  {
  QModelIndexList indexList = phonemeListView->selectionModel ()->selectedRows ();
  
  QList<int> rowList;
  for (int x = 0; x < indexList.size (); x++)
    rowList.append (indexList[x].row ());
  
  qSort (rowList);

  for (int x = 0; x < rowList.size (); x++)
    db.movePhonemeUp (rowList[x]);
  
  updateModels ();

  for (int x = 0; x < rowList.size (); x++)
    phonemeListView->selectionModel ()->select
      (phonemeModel->index (rowList[x] - 1, 0), QItemSelectionModel::Toggle);
}

void PhonologyPage::moveDown ()  {
  QModelIndexList indexList = phonemeListView->selectionModel ()->selectedRows ();
  
  QList<int> rowList;
  for (int x = 0; x < indexList.size (); x++)
    rowList.append (indexList[x].row ());
  
  qSort (rowList.begin (), rowList.end (), qGreater<int>());

  for (int x = 0; x < rowList.size (); x++)
    db.movePhonemeDown (rowList[x]);

  updateModels ();

  for (int x = 0; x < rowList.size (); x++)
    phonemeListView->selectionModel ()->select
      (phonemeModel->index (rowList[x] + 1, 0), QItemSelectionModel::Toggle);
}

void PhonologyPage::deletePhoneme ()  {
  QModelIndexList indexList = phonemeListView->selectionModel ()->selectedRows ();
  
  for (int x = 0; x < indexList.size (); x++)
    db.deletePhoneme (indexList[x].data ().toString ());

  updateModels ();
  emit spellingsChanged ();
}

void PhonologyPage::applyNaturalClass ()  {
  QModelIndexList rowList = phonemeListView->selectionModel ()->selectedRows ();
  
  for (int x = 0; x < rowList.size (); x++)
    db.assignNaturalClass (assignNaturalClassBox->currentText (), rowList[x].data ().toString ());
  
  displayPhoneme ();
  
  emit spellingsChanged ();
}

void PhonologyPage::updateNCModel ()  {
  assignNaturalClassBox->clear ();
  assignNaturalClassBox->addItems (db.getClassList (PHONEME));
}

void PhonologyPage::convertToIPA ()  {
  QString xs = phonemeNameEdit->text ();
  QStringList tokens;
  QString conversion = "";
  
  tokens.append ("");
  int currToken = 0;
  
  for (int x = 0; x < xs.size (); x++)  {
    if ((xs[x] == '`' || xs[x] == '\\') && currToken > 0)
      tokens[currToken-1] += xs[x];
    
    else if (xs[x] == '_' && (x + 1) < xs.size ())  {
      if (xs[x+1] == '<' && currToken > 0)
        tokens[currToken-1] += "_<";
      
      else  {
        tokens[currToken] = "_" + xs[x+1];
        tokens.append ("");
        currToken++;
      }
      
      x++;
    }
    
    else  {
      tokens[currToken] = xs[x];
      tokens.append ("");
      currToken++;
    }
  }
  
  for (int x = 0; x < tokens.size (); x++)  {
    if (ipaTable.contains (tokens[x]))
      conversion += ipaTable[tokens[x]];
    else conversion += tokens[x];
  }
  
  phonemeNameEdit->setText (conversion);
  setChanged ();
}

void PhonologyPage::submitChanges ()  {
  if (!mapper) return;
  
  QModelIndex index = phonemeListView->selectionModel ()->currentIndex ();
  
  mapper->submit ();
  displayModel->submitAll ();
  db.setSpellings (phonemeNameEdit->text (), phonemeSpellingEdit->text ());
  
  updateModels ();
  
  if (index.isValid ())
    phonemeListView->selectionModel ()->setCurrentIndex (index, QItemSelectionModel::Select);
  
  emit spellingsChanged ();
}

void PhonologyPage::launchFeaturesDialog ()  {
  if (featuresDialog)  {
    if (featuresDialog->isVisible ())
      return;
    
    delete featuresDialog;
  }
  
  if (db.currentDB () == "")  {
    QMessageBox::warning (this, "Error", "You need to open or create a dictionary first.");
    return;
  }

  featuresDialog = new ManageFeaturesDialog (PHONEME, db);
  
  featuresDialog->show ();
}

void PhonologyPage::launchNaturalClassesDialog ()  {
  if (featureBundlesDialog)  {
    if (featureBundlesDialog->isVisible ())
      return;
    
    delete featureBundlesDialog;
  }
  
  if (db.currentDB () == "")  {
    QMessageBox::warning (this, "Error", "You need to open or create a dictionary first.");
    return;
  }
  
  featureBundlesDialog = new FeatureBundlesDialog (PHONEME, NATURAL_CLASS, db);
  
  connect (featureBundlesDialog, SIGNAL (done ()), this, SLOT (updateNCModel ()));
  connect (featureBundlesDialog, SIGNAL (done ()), this, SIGNAL (naturalClassListUpdated ()));
  connect (featureBundlesDialog, SIGNAL (done ()), this, SIGNAL (spellingsChanged ()));
  
  featureBundlesDialog->show ();
}

void PhonologyPage::launchEditFeaturesDialog ()  {
  if (featureBundlesDialog)  {
    if (featureBundlesDialog->isVisible ())
      return;
    
    delete featureBundlesDialog;
  }
  
  if (db.currentDB () == "")  {
    QMessageBox::warning (this, "Error", "You need to open or create a dictionary first.");
    return;
  }
  
  int currentPhonID = displayModel->record (mapper->currentIndex ()).value ("id").toInt ();
  
  featureBundlesDialog = new FeatureBundlesDialog (PHONEME, currentPhonID, db);
  
  connect (featureBundlesDialog, SIGNAL (done ()), this, SLOT (displayPhoneme ()));
  connect (featureBundlesDialog, SIGNAL (done ()), this, SIGNAL (spellingsChanged ()));
  
  featureBundlesDialog->show ();
}

QMap<QString, QString> PhonologyPage::initIPATable ()  {
  QMap<QString, QString> m;
  
  if (xsampa.size () != ipa.size ())
    QMessageBox::warning (NULL, "", "XSAMPA list is a different length than IPA list; aborting");
  
  else for (int x = 0; x < xsampa.size (); x++)
    m[xsampa[x]] = ipa[x];
  
  return m;
}