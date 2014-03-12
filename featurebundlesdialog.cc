#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QListView>

#include <QStringListModel>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include "const.h"
#include "editablequerymodel.h"
#include "featurebundlesdialog.h"

FeatureBundlesDialog::FeatureBundlesDialog (int d, int i, CDICDatabase data)  {
  domain = d;
  id = i;
  db = data;
  
  if (i >= 0) initFeatureSetDialog ();
  else initNaturalClassDialog ();
}

void FeatureBundlesDialog::initNaturalClassDialog ()  {
  naturalClassModel = db.getNaturalClassModel (domain);
  featureListModel = new QStringListModel ();
  featureBoxModel = db.getFeatureListModel (domain, ALLSUBS);
  subfeatureBoxModel = db.getSubfeatureModel (domain, QString ());
  
  QString label = (domain == WORD) ? "Word Natural Classes" : "Phoneme Natural Classes";
  
  titleLabel = new QLabel ();
  titleLabel->setText ("<big><b>" + label + "</b></big>");
  titleLabel->setAlignment (Qt::AlignCenter);
  
  addNaturalClassButton = new QPushButton ("Add Class:");
  addNaturalClassButton->setMaximumSize (addNaturalClassButton->sizeHint ());
  addNaturalClassEdit = new QLineEdit;
  
  addNaturalClassLayout = new QHBoxLayout;
  addNaturalClassLayout->addWidget (addNaturalClassButton);
  addNaturalClassLayout->addWidget (addNaturalClassEdit);
  
  naturalClassView = new QListView;
  naturalClassView->setSelectionBehavior (QAbstractItemView::SelectRows);
  naturalClassView->setEditTriggers (QAbstractItemView::DoubleClicked |
                                     QAbstractItemView::SelectedClicked);
  naturalClassView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  naturalClassView->setModel (naturalClassModel);
  
  deleteButton = new QPushButton ("Delete Class(es)");
  deleteButton->setMaximumSize (deleteButton->sizeHint ());
  
  leftLayout = new QVBoxLayout;
  leftLayout->addLayout (addNaturalClassLayout);
  leftLayout->addWidget (naturalClassView);
  leftLayout->addWidget (deleteButton);
  leftLayout->setAlignment (deleteButton, Qt::AlignCenter);
  
  addFeatureButton = new QPushButton ("Add/Set Feature:");
  addFeatureButton->setMaximumSize (addFeatureButton->sizeHint ());
  addFeatureBox = new QComboBox;
  addFeatureBox->setModel (featureBoxModel);
  addSubfeatureBox = new QComboBox;
  addSubfeatureBox->setModel (subfeatureBoxModel);
  
  db.updateSubfeatureModel (domain, subfeatureBoxModel, addFeatureBox->currentText ());
  
  addFeatureLayout = new QHBoxLayout;
  addFeatureLayout->addWidget (addFeatureButton);
  addFeatureLayout->addWidget (addFeatureBox);
  addFeatureLayout->addWidget (addSubfeatureBox);
  
  featureView = new QListView;
  featureView->setSelectionBehavior (QAbstractItemView::SelectRows);
  featureView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  featureView->setModel (featureListModel);
  
  removeFeatureButton = new QPushButton ("Remove Feature");
  removeFeatureBox = new QComboBox;
  removeFeatureBox->setModel (featureBoxModel);
  
  removeFeatureLayout = new QHBoxLayout;
  removeFeatureLayout->addWidget (removeFeatureButton);
  removeFeatureLayout->setAlignment (removeFeatureButton, Qt::AlignRight);
  removeFeatureLayout->addWidget (removeFeatureBox);
  removeFeatureLayout->setAlignment (removeFeatureBox, Qt::AlignLeft);
  
  clearButton = new QPushButton ("Clear Features");
  clearButton->setMaximumSize (clearButton->sizeHint ());
  
  rightLayout = new QVBoxLayout;
  rightLayout->addLayout (addFeatureLayout);
  rightLayout->addWidget (featureView);
  rightLayout->addLayout (removeFeatureLayout);
  rightLayout->addWidget (clearButton);
  rightLayout->setAlignment (clearButton, Qt::AlignCenter);
  
  centralLayout = new QHBoxLayout;
  centralLayout->addLayout (leftLayout);
  centralLayout->addLayout (rightLayout);
  
  doneButton = new QPushButton ("Done");
  doneButton->setMaximumSize (doneButton->sizeHint ());
  
  mainLayout = new QVBoxLayout;
  mainLayout->addWidget (titleLabel);
  mainLayout->addLayout (centralLayout);
  mainLayout->addWidget (doneButton);
  mainLayout->setAlignment (doneButton, Qt::AlignCenter);
  
  setLayout (mainLayout);
  
  connect (addNaturalClassButton, SIGNAL (clicked ()), this, SLOT (addNaturalClass ()));
  connect (addNaturalClassButton, SIGNAL (clicked ()), addNaturalClassEdit, SLOT (clear ()));
  connect (deleteButton, SIGNAL (clicked ()), this, SLOT (deleteClass ()));
  connect (naturalClassView->selectionModel (), 
           SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)),
           this, SLOT (displayFeatures ()));
  connect (naturalClassModel, SIGNAL (itemChanged (QVariant, QVariant)), this,
           SLOT (renameClass (QVariant, QVariant)));
  
  connect (addFeatureButton, SIGNAL (clicked ()), this, SLOT (addFeature ()));
  connect (addFeatureBox, SIGNAL (currentIndexChanged (const QString&)), this,
           SLOT (updateFeatureBox ()));
  connect (removeFeatureButton, SIGNAL (clicked ()), this, SLOT (removeFeature ()));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (clearFeatures ()));
  
  connect (doneButton, SIGNAL (clicked ()), this, SIGNAL (done ()));
  connect (doneButton, SIGNAL (clicked ()), this, SLOT (close ()));
}

void FeatureBundlesDialog::initFeatureSetDialog ()  {
  naturalClassModel = NULL;
  featureListModel = new QStringListModel ();
  featureBoxModel = db.getFeatureListModel (domain, ALLSUBS);
  subfeatureBoxModel = db.getSubfeatureModel (domain, QString ());
  
  QString sb = db.getValue ("SquareBrackets");
  
  QString label = "Edit Features of ";
  
  if (domain == PHONEME)  {
    label += (sb == "true") ? "[" : "/";
    label += db.getPhonemeName (id);
    label += (sb == "true") ? "]" : "/";
  }
  
  titleLabel = new QLabel ("<big><b>" + label + "</b></big>");
  titleLabel->setAlignment (Qt::AlignCenter);
  
  addFeatureButton = new QPushButton ("Add/Set Feature:");
  addFeatureButton->setMaximumSize (addFeatureButton->sizeHint ());
  addFeatureBox = new QComboBox;
  addFeatureBox->setModel (featureBoxModel);
  addSubfeatureBox = new QComboBox;
  addSubfeatureBox->setModel (subfeatureBoxModel);
  
  db.updateSubfeatureModel (domain, subfeatureBoxModel, addFeatureBox->currentText ());
  
  addFeatureLayout = new QHBoxLayout;
  addFeatureLayout->addWidget (addFeatureButton);
  addFeatureLayout->addWidget (addFeatureBox);
  addFeatureLayout->addWidget (addSubfeatureBox);
  
  featureView = new QListView;
  featureView->setSelectionBehavior (QAbstractItemView::SelectRows);
  featureView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  featureView->setModel (featureListModel);
  
  removeFeatureButton = new QPushButton ("Remove Feature");
  removeFeatureBox = new QComboBox;
  removeFeatureBox->setModel (featureBoxModel);
  
  removeFeatureLayout = new QHBoxLayout;
  removeFeatureLayout->addWidget (removeFeatureButton);
  removeFeatureLayout->setAlignment (removeFeatureButton, Qt::AlignRight);
  removeFeatureLayout->addWidget (removeFeatureBox);
  removeFeatureLayout->setAlignment (removeFeatureBox, Qt::AlignLeft);
  
  clearButton = new QPushButton ("Clear Features");
  clearButton->setMaximumSize (clearButton->sizeHint ());
  
  doneButton = new QPushButton ("Done");
  doneButton->setMaximumSize (doneButton->sizeHint ());
  
  mainLayout = new QVBoxLayout;
  mainLayout->addWidget (titleLabel);
  mainLayout->addLayout (addFeatureLayout);
  mainLayout->addWidget (featureView);
  mainLayout->addLayout (removeFeatureLayout);
  mainLayout->addWidget (clearButton);
  mainLayout->setAlignment (clearButton, Qt::AlignCenter);
  mainLayout->addStretch (1);
  mainLayout->addWidget (doneButton);
  mainLayout->setAlignment (doneButton, Qt::AlignCenter);
  
  setLayout (mainLayout);
           
  connect (addFeatureButton, SIGNAL (clicked ()), this, SLOT (addFeature ()));
  connect (addFeatureBox, SIGNAL (currentIndexChanged (const QString&)), this,
           SLOT (updateFeatureBox ()));
  connect (removeFeatureButton, SIGNAL (clicked ()), this, SLOT (removeFeature ()));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (clearFeatures ()));
  
  connect (doneButton, SIGNAL (clicked ()), this, SLOT (close ()));
  connect (doneButton, SIGNAL (clicked ()), this, SIGNAL (done ()));
  
  displayFeatures ();
}
    
void FeatureBundlesDialog::addNaturalClass ()  {
  if (id >= 0) return;

  db.addNaturalClass (domain, addNaturalClassEdit->text ());
  
  refreshClassModel ();
}
    
void FeatureBundlesDialog::displayFeatures ()  {
  if (id >= 0)
    featureListModel->setStringList (db.getBundledFeatures (domain, id));
  
  else  {
    if (!naturalClassView->selectionModel ()->currentIndex ().isValid ())
      return;
  
    QString currentClass = naturalClassView->selectionModel ()->currentIndex ().data ().toString ();
  
    featureListModel->setStringList (db.getBundledFeatures (domain, currentClass));
  }
}
    
void FeatureBundlesDialog::deleteClass ()  {
  if (id >= 0) return;
  
  QModelIndexList indexList = naturalClassView->selectionModel ()->selectedRows ();
  
  QStringList nameList;
  for (int x = 0; x < indexList.size (); x++)
    nameList.append (indexList[x].data ().toString ());
  
  db.deleteNaturalClass (domain, nameList);
  
  refreshClassModel ();
}

void FeatureBundlesDialog::renameClass (QVariant before, QVariant after)  {
  db.renameNaturalClass (domain, before.toString (), after.toString ());
  
  refreshClassModel ();
}

void FeatureBundlesDialog::refreshClassModel ()  {
  if (id >= 0) return;
  
  if (domain == PHONEME)
    naturalClassModel->setQuery ("select name from NaturalClassPhon");
  else naturalClassModel->setQuery ("select name from NaturalClassWord");
}
    
void FeatureBundlesDialog::addFeature ()  {
  QString feat = addFeatureBox->currentText ();
  QString sub = addSubfeatureBox->currentText ();
  
  if (id >= 0)
    db.addFeatureToSet (domain, id, feat, sub);
  
  else  {
    if (!naturalClassView->selectionModel ()->currentIndex ().isValid ())
      return;
    
    QString currentClass = naturalClassView->selectionModel ()->currentIndex ().data ().toString ();
    
    db.addFeatureToClass (domain, currentClass, feat, sub);
  }
  
  displayFeatures ();
}

void FeatureBundlesDialog::removeFeature ()  {
  if (id >= 0)
    db.removeFeatureFromSet (domain, id, removeFeatureBox->currentText ());
  
  else  {
    if (!naturalClassView->selectionModel ()->currentIndex ().isValid ())
      return;
    
    QString currentClass = naturalClassView->selectionModel ()->currentIndex ().data ().toString ();
    
    db.removeFeatureFromClass (domain, currentClass, removeFeatureBox->currentText ());
  }
  
  displayFeatures ();
}
    
void FeatureBundlesDialog::clearFeatures ()  {
  if (id >= 0)
    db.clearSet (domain, id);
  
  else  {
    if (!naturalClassView->selectionModel ()->currentIndex ().isValid ())
      return;
    
    QString currentClass = naturalClassView->selectionModel ()->currentIndex ().data ().toString ();
    
    db.clearClass (domain, currentClass);
  }
  
  displayFeatures ();
}

void FeatureBundlesDialog::updateFeatureBox ()  {
  db.updateSubfeatureModel (domain, subfeatureBoxModel, addFeatureBox->currentText ());
}