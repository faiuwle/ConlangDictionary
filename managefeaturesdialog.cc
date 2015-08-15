#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QMessageBox>

#include "const.h"
#include "managefeaturesdialog.h"
#include "editablequerymodel.h"

ManageFeaturesDialog::ManageFeaturesDialog (int d, CDICDatabase b)  {
  domain = d;
  db = b;
  
  featureListModel = db.getFeatureListModel (domain);
  featureBoxModel = db.getFeatureListModel (domain, ALLSUBS);
  subfeatureListModel = db.getSubfeatureModel (domain, "");
  subfeatureBoxModel = db.getSubfeatureModel (domain, "");
  
  addFeaturesButton = new QPushButton ("Add");
  addFeaturesButton->setMaximumSize (addFeaturesButton->sizeHint ());
  addFeaturesEdit = new QLineEdit;
  addFeaturesBox = new QComboBox;
  addFeaturesBox->addItem ("as univalent feature(s)");
  addFeaturesBox->addItem ("as binary feature(s)");
  addFeaturesBox->addItem ("as feature group(s)");
  
  addFeaturesLayout = new QHBoxLayout;
  addFeaturesLayout->addWidget (addFeaturesButton);
  addFeaturesLayout->addWidget (addFeaturesEdit);
  addFeaturesLayout->addWidget (addFeaturesBox);
  
  QLabel *displayLabel = new QLabel ("Display ");
  displayLabel->setMaximumSize (displayLabel->sizeHint ());
  featureDisplayBox = new QComboBox;
  featureDisplayBox->addItem ("Univalent Features");
  featureDisplayBox->addItem ("Binary Features");
  featureDisplayBox->addItem ("Feature Groups");
  
  displayFeaturesLayout = new QHBoxLayout;
  displayFeaturesLayout->addWidget (displayLabel);
  displayFeaturesLayout->addWidget (featureDisplayBox);
  
  featureListView = new QListView;
  featureListView->setSelectionBehavior (QAbstractItemView::SelectRows);
  featureListView->setEditTriggers (QAbstractItemView::DoubleClicked |
                                    QAbstractItemView::SelectedClicked);
  featureListView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  featureListView->setModel (featureListModel);

  connect (featureListModel, SIGNAL (itemChanged (QVariant, QVariant)), this,
           SLOT (replaceFeature (QVariant, QVariant)));
  
  parentLabel = new QLabel ("Parent Feature: ");
  parentButton = new QPushButton ("Set Parent Feature:");
  parentButton->setMaximumSize (parentButton->sizeHint ());
  parentFeatureBox = new QComboBox;
  parentFeatureBox->setModel (featureBoxModel);
  parentSubfeatureBox = new QComboBox;
  parentSubfeatureBox->setModel (subfeatureBoxModel);
  clearParentButton = new QPushButton ("Clear");
  clearParentButton->setMaximumSize (clearParentButton->sizeHint ());
  
  parentLayout = new QHBoxLayout;
  parentLayout->addWidget (parentButton);
  parentLayout->addWidget (parentFeatureBox);
  parentLayout->addWidget (parentSubfeatureBox);
  parentLayout->addWidget (clearParentButton);
  
  leftLayout = new QVBoxLayout;
  leftLayout->addLayout (addFeaturesLayout);
  leftLayout->addLayout (displayFeaturesLayout);
  leftLayout->addWidget (featureListView);
  leftLayout->addWidget (parentLabel);
  leftLayout->addLayout (parentLayout);
  
  deleteButton = new QPushButton ("Delete");
  deleteButton->setMaximumSize (deleteButton->sizeHint ());
  
  addSubfeaturesButton = new QPushButton ("Add Subfeatures");
  addSubfeaturesButton->setMaximumSize (addSubfeaturesButton->sizeHint ());
  addSubfeaturesEdit = new QLineEdit;
  
  addSubfeaturesLayout = new QHBoxLayout;
  addSubfeaturesLayout->addWidget (addSubfeaturesButton);
  addSubfeaturesLayout->addWidget (addSubfeaturesEdit);
  
  QLabel *displayTypeLabel = new QLabel ("Subfeature Display Format:");
  displayTypeBox = new QComboBox;
  
  displayTypeLayout = new QHBoxLayout;
  displayTypeLayout->addWidget (displayTypeLabel);
  displayTypeLayout->addWidget (displayTypeBox);
  
  subfeatureListView = new QListView;
  subfeatureListView->setModel (subfeatureListModel);
  subfeatureListView->setSelectionBehavior (QAbstractItemView::SelectRows);
  subfeatureListView->setEditTriggers (QAbstractItemView::DoubleClicked |
                                       QAbstractItemView::SelectedClicked);
  subfeatureListView->setSelectionMode (QAbstractItemView::ExtendedSelection);

  connect (subfeatureListModel, SIGNAL (itemChanged (QVariant, QVariant)), this,
           SLOT (replaceSubfeature (QVariant, QVariant)));
  
  rightLayout = new QVBoxLayout;
  rightLayout->addLayout (addSubfeaturesLayout);
  rightLayout->addLayout (displayTypeLayout);
  rightLayout->addWidget (subfeatureListView);
  
  centralLayout = new QHBoxLayout;
  centralLayout->addLayout (leftLayout);
  centralLayout->addWidget (deleteButton);
  centralLayout->addLayout (rightLayout);
  
  submitButton = new QPushButton ("Done");
  submitButton->setMaximumSize (submitButton->sizeHint ());
  
  mainLayout = new QVBoxLayout;
  mainLayout->addLayout (centralLayout);
  mainLayout->addWidget (submitButton);
  mainLayout->setAlignment (submitButton, Qt::AlignCenter);
  
  setLayout (mainLayout);
  
  connect (addFeaturesButton, SIGNAL (clicked ()), this, SLOT (addFeatures ()));
  connect (featureDisplayBox, SIGNAL (currentIndexChanged (const QString&)), this,
           SLOT (updateModels ()));
  connect (deleteButton, SIGNAL (clicked ()), this, SLOT (deleteFeatures ()));
  connect (featureListView->selectionModel (), 
           SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)), this,
           SLOT (displaySubfeatures ()));
  connect (addSubfeaturesButton, SIGNAL (clicked ()), this, SLOT (addSubfeatures ()));
  connect (displayTypeBox, SIGNAL (currentIndexChanged (int)), this,
           SLOT (setDisplayType (int)));
  connect (parentButton, SIGNAL (clicked ()), this, SLOT (setParent ()));
  connect (parentFeatureBox, SIGNAL (currentIndexChanged (const QString&)), this,
           SLOT (populateParentSubs (QString)));
  connect (clearParentButton, SIGNAL (clicked ()), this, SLOT (clearParent ()));
           
  connect (submitButton, SIGNAL (clicked ()), this, SLOT (close ()));
}

void ManageFeaturesDialog::addFeatures ()  {
  QStringList featNames = addFeaturesEdit->text ().split (",", QString::SkipEmptyParts);
  
  for (int x = 0; x < featNames.size (); x++)
    featNames[x] = featNames[x].trimmed ();
  
  featNames.removeAll ("");
  
  QStringList sub;
  
  if (addFeaturesBox->currentIndex () == UNIVALENT)
    sub.append ("");
  
  else if (addFeaturesBox->currentIndex () == BINARY)  {
    sub.append ("+");
    sub.append (MINUS_SIGN);
  }
  
  db.addFeature (domain, featNames, sub);
  
  addFeaturesEdit->clear ();
  updateModels ();
}

void ManageFeaturesDialog::addSubfeatures ()  {
  if (!featureListView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  QModelIndex currentFeat = featureListView->selectionModel ()->currentIndex ();
  
  QStringList subNames = addSubfeaturesEdit->text ().split (",", QString::SkipEmptyParts);
  
  for (int x = 0; x < subNames.size (); x++)
    subNames[x] = subNames[x].trimmed ();
  
  subNames.removeAll ("");
  
  db.addSubfeature (domain, currentFeat.data ().toString (), subNames);
  
  addSubfeaturesEdit->clear ();
  updateModels ();
  
  featureListView->selectionModel ()->setCurrentIndex (currentFeat, QItemSelectionModel::SelectCurrent);
}
    
void ManageFeaturesDialog::deleteFeatures ()  {
  QModelIndexList indexList;
  QModelIndex currentFeature = featureListView->selectionModel ()->currentIndex ();
  
  if (currentFeature.isValid () && subfeatureListView->selectionModel ()->hasSelection ())  {
    indexList = subfeatureListView->selectionModel ()->selectedRows ();
    
    QStringList subfeatNames;
    for (int x = 0; x < indexList.size (); x++)
      subfeatNames.append (indexList[x].data ().toString ());
    
    db.deleteSubfeature (domain, currentFeature.data ().toString (), subfeatNames);
    
    updateModels ();
    
    featureListView->selectionModel ()->setCurrentIndex (currentFeature, QItemSelectionModel::SelectCurrent);
  }
  
  else  {
    indexList = featureListView->selectionModel ()->selectedRows ();
  
    QStringList featNames;
    for (int x = 0; x < indexList.size (); x++)
      featNames.append (indexList[x].data ().toString ());
  
    db.deleteFeature (domain, featNames);
    
    updateModels ();
  }
}
    
void ManageFeaturesDialog::displaySubfeatures ()  {
  if (!featureListView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  QString currentFeat = featureListView->selectionModel ()->currentIndex ().data ().toString ();
  
  db.updateSubfeatureModel (domain, subfeatureListModel, currentFeat);
    
  parentLabel->setText ("Parent Feature: " + db.getParent (domain, currentFeat));
  
  QStringList displayList = db.getDisplayType (domain, currentFeat);
  QString currentDisplay = displayList.size () ? displayList.takeLast() : "";
  
  displayTypeBox->clear ();
  displayTypeBox->addItems (displayList);
  
  if (currentDisplay != "")
    displayTypeBox->setCurrentIndex (displayList.indexOf (currentDisplay));
}
    
void ManageFeaturesDialog::populateParentSubs (QString feat)  {
  db.updateSubfeatureModel (domain, subfeatureBoxModel, feat);
}
    
void ManageFeaturesDialog::setParent ()  {
  if (!featureListView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  QString featName = featureListView->selectionModel ()->currentIndex ().data ().toString ();
  QString parentName = parentFeatureBox->currentText ();
  QString parentSub = parentSubfeatureBox->currentText ();
  
  if (featName == parentName)
    QMessageBox::warning (this, "Error", "You can't make a feature it's own parent!");
  
  else db.setFeatureParent (domain, featName, parentName, parentSub);
                      
  displaySubfeatures ();
}

void ManageFeaturesDialog::clearParent ()  {
  if (!featureListView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  db.setFeatureParent (domain, featureListView->selectionModel ()->currentIndex ().data ().toString ());
  
  displaySubfeatures ();
}

void ManageFeaturesDialog::setDisplayType (int type)  {
  if (!featureListView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  db.setDisplay (domain, featureListView->selectionModel ()->currentIndex ().data ().toString (), type);
}
    
void ManageFeaturesDialog::replaceFeature (QVariant before, QVariant after)  {
  db.renameFeature (domain, before.toString (), after.toString ());
  
  updateModels ();
}
    
void ManageFeaturesDialog::replaceSubfeature (QVariant before, QVariant after)  {
  QModelIndex currentFeat = featureListView->selectionModel ()->currentIndex ();
  
  if (!currentFeat.isValid ())
    return;
  
  db.renameSubfeature (domain, currentFeat.data ().toString (), before.toString (), 
                       after.toString ());
  
  updateModels ();
  
  featureListView->selectionModel ()->setCurrentIndex (currentFeat, QItemSelectionModel::SelectCurrent);
}

void ManageFeaturesDialog::updateModels ()  {
  db.filterFeatureModel (domain, featureListModel, featureDisplayBox->currentIndex ());
  db.filterFeatureModel (domain, featureBoxModel, ALLSUBS);
  
  db.updateSubfeatureModel (domain, subfeatureListModel, "");
  db.updateSubfeatureModel (domain, subfeatureBoxModel, parentFeatureBox->currentText ());
  
  displayTypeBox->clear ();
}