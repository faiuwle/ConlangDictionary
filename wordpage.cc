#include <QPushButton>
#include <QLineEdit>
#include <QTreeView>
#include <QComboBox>
#include <QLabel>
#include <QTextEdit>

#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QDataWidgetMapper>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QMessageBox>
#include <QTextStream>

#include <iostream>
using namespace std;

#include "const.h"

#include "wordpage.h"
#include "cdicdatabase.h"
#include "managefeaturesdialog.h"
#include "featurebundlesdialog.h"
#include "editphonologydialog.h"

WordPage::WordPage ()  {
  featuresDialog = NULL;
  featureBundlesDialog = NULL;
  editPhonologyDialog = NULL;
  
  wordModel = NULL;
  displayModel = NULL;
  mapper = NULL;
  
  dirty = false;
  
  addWordButton = new QPushButton ("Add Word");
  addWordEdit = new QLineEdit;
  deleteWordButton = new QPushButton ("Delete");
  addWordLayout = new QHBoxLayout;
  addWordLayout->addWidget (addWordButton);
  addWordLayout->addWidget (addWordEdit);
  addWordLayout->addWidget (deleteWordButton);
  
  searchButton = new QPushButton ("Search");
  searchEdit = new QLineEdit;
  naturalClassBox = new QComboBox;
  naturalClassBox->setSizeAdjustPolicy (QComboBox::AdjustToContents);
  naturalClassBox->addItem ("Any Word Type");
  languageBox = new QComboBox;
  languageBox->setSizeAdjustPolicy (QComboBox::AdjustToContents);
  languageBox->addItem ("English");
  searchLayout = new QHBoxLayout;
  searchLayout->addWidget (searchButton);
  searchLayout->addWidget (searchEdit);
  searchLayout->addWidget (naturalClassBox);
  searchLayout->addWidget (languageBox);
  
  wordView = new QTreeView;
  wordView->setSelectionBehavior (QAbstractItemView::SelectRows);
  wordView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  wordView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  
  manageFeaturesButton = new QPushButton ("Manage Word Features");
  manageNaturalClassesButton = new QPushButton ("Manage Word Classes");
  dialogButtonsLayout = new QHBoxLayout;
  dialogButtonsLayout->addWidget (manageFeaturesButton);
  dialogButtonsLayout->addWidget (manageNaturalClassesButton);

  assignNaturalClassButton = new QPushButton ("Assign to Class:");
  assignNaturalClassBox = new QComboBox;
  assignNaturalClassLayout = new QHBoxLayout;
  assignNaturalClassLayout->addWidget (assignNaturalClassButton);
  assignNaturalClassLayout->addWidget (assignNaturalClassBox);

  listLayout = new QVBoxLayout;
  listLayout->addLayout (addWordLayout);
  listLayout->addLayout (searchLayout);
  listLayout->addWidget (wordView);
  listLayout->addLayout (dialogButtonsLayout);
  listLayout->addLayout (assignNaturalClassLayout);
  
  QLabel *spellingLabel = new QLabel ("Spelling: ");
  spellingEdit = new QLineEdit;
  changeButton = new QPushButton ("Update Phonology");
  spellingLayout = new QHBoxLayout;
  spellingLayout->addWidget (spellingLabel);
  spellingLayout->addWidget (spellingEdit);
  spellingLayout->addWidget (changeButton);
  
  QLabel *phonologyLabel = new QLabel ("Phonology: ");
  phonologyDisplay = new QLabel ("");
  editPhonologyButton = new QPushButton ("Edit");
  phonologyLayout = new QHBoxLayout;
  phonologyLayout->addWidget (phonologyLabel);
  phonologyLayout->addWidget (phonologyDisplay);
  phonologyLayout->addWidget (editPhonologyButton);
  phonologyLayout->setAlignment (editPhonologyButton, Qt::AlignRight);
  
  naturalClassDisplay = new QLabel ("");
  featuresDisplay = new QLabel ("Features:\n\n");
  editFeaturesButton = new QPushButton ("Add/Edit Features");
  QLabel *definitionLabel = new QLabel ("Definition:");
  definitionEdit = new QTextEdit;
  submitButton = new QPushButton ("Submit");
  submitButton->setEnabled (false);
  
  wordInfoLayout = new QVBoxLayout;
  wordInfoLayout->addLayout (spellingLayout);
  wordInfoLayout->addLayout (phonologyLayout);
  wordInfoLayout->addWidget (naturalClassDisplay);
  wordInfoLayout->addWidget (featuresDisplay);
  wordInfoLayout->addWidget (editFeaturesButton);
  wordInfoLayout->setAlignment (editFeaturesButton, Qt::AlignCenter);
  wordInfoLayout->addWidget (definitionLabel);
  wordInfoLayout->addWidget (definitionEdit);
  wordInfoLayout->addWidget (submitButton);
  wordInfoLayout->setAlignment (submitButton, Qt::AlignCenter);
  
  mainLayout = new QHBoxLayout;
  mainLayout->addLayout (listLayout);
  mainLayout->addLayout (wordInfoLayout);
  
  setLayout (mainLayout);
  
  connect (addWordButton, SIGNAL (clicked ()),  this, SLOT (addWord ()));
  connect (deleteWordButton, SIGNAL (clicked ()), this, SLOT (deleteWord ()));
  connect (searchButton, SIGNAL (clicked ()), this, SLOT (search ()));
  
  connect (manageFeaturesButton, SIGNAL (clicked ()), this, 
           SLOT (launchFeaturesDialog ()));
  connect (manageNaturalClassesButton, SIGNAL (clicked ()), this, 
           SLOT (launchNaturalClassesDialog ()));
  connect (editFeaturesButton, SIGNAL (clicked ()), this,
           SLOT (launchEditFeaturesDialog ()));
  connect (assignNaturalClassButton, SIGNAL (clicked ()), this,
           SLOT (applyNaturalClass ()));
  connect (editPhonologyButton, SIGNAL (clicked ()), this,
           SLOT (launchEditPhonologyDialog ()));
  
  connect (changeButton, SIGNAL (clicked ()), this, SLOT (parseWord ()));
           
  connect (spellingEdit, SIGNAL (textEdited (QString)), this, SLOT (setChanged ()));
  connect (definitionEdit, SIGNAL (textChanged ()), this, SLOT (setChanged ()));
  
  connect (submitButton, SIGNAL (clicked ()), this, SLOT (submitChanges ()));
}

void WordPage::clearDB ()  {
  if (wordModel)
    wordModel->clear ();
  
  if (mapper)  {
    mapper->setModel (NULL);

    if (displayModel)
      displayModel->clear ();
  }
  
  naturalClassBox->clear ();
  naturalClassBox->addItem ("Any Word Type");
  assignNaturalClassBox->clear ();
  
  db.close ();
  
  dirty = true;
}

void WordPage::setDB (CDICDatabase database)  {
  QSqlQueryModel *wModel = wordModel;

  db = database;
  wordModel = db.getWordListModel ();
  wordView->setModel (wordModel);
  wordView->setColumnHidden (0, true);
  wordView->setHeaderHidden (true);

  if (wModel)  {
    disconnect (wModel);
    delete wModel;
  }
  
  connect (wordView->selectionModel (), 
           SIGNAL (currentChanged (const QModelIndex&, const QModelIndex&)), 
           this, SLOT (displayWord ()));
  
  if (!mapper) 
    mapper = new QDataWidgetMapper (this);

  QSqlTableModel *dModel = displayModel;
  
  displayModel = db.getWordDisplayModel ();  
  
  mapper->setModel (displayModel);
  mapper->setSubmitPolicy (QDataWidgetMapper::ManualSubmit);
  mapper->addMapping (spellingEdit, 1);
  mapper->addMapping (definitionEdit, 2);
  mapper->toFirst ();
  
  if (dModel)  {
    disconnect (dModel);
    delete dModel;
  }
  
  dirty = true;
  
  languageBox->clear ();
  if (db.getValue (LANGUAGE_NAME) == "")
    languageBox->addItem ("Conlang");
  else languageBox->addItem (db.getValue (LANGUAGE_NAME));
  languageBox->addItem ("English");
  
  updateModels ();
}

// Note: destroys selection and current item in listview, caller must save
void WordPage::updateModels ()  {
//  cout << "updateModels" << endl;
  
  if (!displayModel) return;
  
  db.searchWordList (wordModel, searchEdit->text (), naturalClassBox->currentText (),
                     languageBox->currentText ());
  displayModel->select ();
  naturalClassBox->clear ();
  naturalClassBox->addItem ("Any Word Type");
  naturalClassBox->addItems (db.getClassList (WORD));
  assignNaturalClassBox->clear ();
  assignNaturalClassBox->addItems (db.getClassList (WORD));

  submitButton->setEnabled (false);
}

void WordPage::setDirty ()  {
  dirty = true;
}

void WordPage::parseWordlist ()  {
  QList<int> idList = db.getAllWordIDs ();
  
  for (int x = 0; x < idList.size (); x++)
    parseWord (idList[x]);
  
  emit parsingFinished ();
}

void WordPage::setLanguageName (QString language)  {
  languageBox->clear ();
  if (language == "")
    languageBox->addItem ("Conlang");
  else languageBox->addItem (language);
  languageBox->addItem ("English");
}

void WordPage::setChanged ()  {
  submitButton->setEnabled (true);
}

void WordPage::addWord ()  {
  if (addWordEdit->text () != "")  {
    int wordID = db.addWord (addWordEdit->text ());
    parseWord (wordID);
  }
  
  updateModels ();
  
  addWordEdit->setText ("");
}

void WordPage::deleteWord ()  {
  QModelIndexList indexList = wordView->selectionModel ()->selectedRows ();
  
  for (int x = 0; x < indexList.size (); x++)  {
    int id = wordModel->record (indexList[x].row ()).value ("id").toInt ();
    db.deleteWord (id);
  }

  updateModels ();
}

void WordPage::search ()  {
  if (naturalClassBox->currentIndex () == 0)
    db.searchWordList (wordModel, searchEdit->text (), "", languageBox->currentText ());
  else db.searchWordList (wordModel, searchEdit->text (), 
                          naturalClassBox->currentText (),
                          languageBox->currentText ());
}

void WordPage::displayWord ()  {
  if (!wordView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  int row = wordView->selectionModel ()->currentIndex ().row ();
  int currentID = wordModel->record (row).value ("id").toInt ();
  
  mapper->setCurrentIndex (row);
  
  phonologyDisplay->setText (db.getRepresentation (currentID));
  naturalClassDisplay->setText ("<b>" + db.getClassList (WORD, currentID).join (", ") + "</b>");
  featuresDisplay->setText ("Features:\n[" + db.getBundledFeatures (WORD, currentID).join (", ") + "]");
  submitButton->setEnabled (false);
}

void WordPage::updateAfterFeatureChange ()  {
  QModelIndex index = wordView->selectionModel ()->currentIndex ();
  
  updateModels ();
  
  if (index.isValid ())
    wordView->selectionModel ()->setCurrentIndex (index, QItemSelectionModel::Select);
}

void WordPage::applyNaturalClass ()  {
  QModelIndexList rowList = wordView->selectionModel ()->selectedRows ();
  
  for (int x = 0; x < rowList.size (); x++)  {
    int id = wordModel->record (rowList[x].row ()).value ("id").toInt ();
    
    db.assignNaturalClass (assignNaturalClassBox->currentText (), id);
  }
  
  displayWord ();
}

void WordPage::updateNCModel ()  {
  naturalClassBox->clear ();
  naturalClassBox->addItems (db.getClassList (WORD));
  
  assignNaturalClassBox->clear ();
  assignNaturalClassBox->addItems (db.getClassList (WORD));
}

void WordPage::submitChanges ()  {
  if (!mapper) return;
  
  QModelIndex index = wordView->selectionModel ()->currentIndex ();
  
  mapper->submit ();
  displayModel->submitAll ();
  
  updateModels ();
  
  if (index.isValid ())
    wordView->selectionModel ()->setCurrentIndex (index, QItemSelectionModel::Select);
  
  submitButton->setEnabled (false);
}

void WordPage::launchFeaturesDialog ()  {
  if (featuresDialog)  {
    if (featuresDialog->isVisible ())
      return;
    
    delete featuresDialog;
  }
  
  if (db.currentDB () == "")  {
    QMessageBox::warning (this, "Error", "You need to open or create a dictionary first.");
    return;
  }

  featuresDialog = new ManageFeaturesDialog (WORD, db);
  
  featuresDialog->show ();
}

void WordPage::launchNaturalClassesDialog ()  {
  if (featureBundlesDialog)  {
    if (featureBundlesDialog->isVisible ())
      return;
    
    delete featureBundlesDialog;
  }
  
  if (db.currentDB () == "")  {
    QMessageBox::warning (this, "Error", "You need to open or create a dictionary first.");
    return;
  }
  
  featureBundlesDialog = new FeatureBundlesDialog (WORD, NATURAL_CLASS, db);
  
  connect (featureBundlesDialog, SIGNAL (done ()), this, SLOT (updateNCModel ()));
  connect (featureBundlesDialog, SIGNAL (done ()), this, SLOT (updateAfterFeatureChange ()));
  
  featureBundlesDialog->show ();
}

void WordPage::launchEditFeaturesDialog ()  {
  if (featureBundlesDialog)  {
    if (featureBundlesDialog->isVisible ())
      return;
    
    delete featureBundlesDialog;
  }
  
  if (db.currentDB () == "")  {
    QMessageBox::warning (this, "Error", "You need to open or create a dictionary first.");
    return;
  }
  
  featureBundlesDialog = new FeatureBundlesDialog (WORD, mapper->currentIndex (), db);
  
  connect (featureBundlesDialog, SIGNAL (done ()), this, SLOT (updateAfterFeatureChange ()));
  
  featureBundlesDialog->show ();
}

void WordPage::launchEditPhonologyDialog ()  {
  if (editPhonologyDialog)  {
    if (editPhonologyDialog->isVisible ())
      return;
    
    delete editPhonologyDialog;
  }
  
  if (db.currentDB () == "")  {
    QMessageBox::warning (this, "Error", "You need to open or create a dictionary first.");
    return;
  }
  
  int row = wordView->selectionModel ()->currentIndex ().row ();
  int currentID = wordModel->record (row).value ("id").toInt ();
  
  editPhonologyDialog = new EditPhonologyDialog (db, currentID);
  
  connect (editPhonologyDialog, SIGNAL (done ()), this, SLOT (displayWord ()));
  
  editPhonologyDialog->show ();
}

void WordPage::parseWord ()  {
  submitChanges ();
  int row = wordView->selectionModel ()->currentIndex ().row ();
  int currentID = wordModel->record (row).value ("id").toInt ();
  parseWord (currentID);
  displayWord ();
}

void WordPage::parseWord (int id)  {
  if (dirty)  {
    phonemes = db.getPhonemesAndSpellings ();
    diacriticSupras = db.getDiacriticSupras ();
    beforeSupras = db.getBeforeSupras ();
    afterSupras = db.getAfterSupras ();
    doubledSupras = db.getDoubledSupras ();

    QList<Rule> ruleList = db.getParsingGrammar ();
    parser.setRules (ruleList);
    parser.setIgnored (db.getValue (IGNORED_CHARACTERS));
    
    dirty = false;
  }
  
  QString word = db.getWordName (id);
  db.setPhonology (id, convertTree (parser.parse (word)));
  
  emit wordParsed ();
}

QList<Syllable> WordPage::convertTree (TreeNode node)  {
  QList<Syllable> syllList;
  QList<TreeNode> treeList;
  
  TreeNode currentS;
  
  for (int x = 0; x < node.children.size (); x++)
    if (node.children[x].label == "S")  {
      currentS = node.children[x];
      break;
    }
  
  QTextStream terminal (stdout);
  
  // break into syllables
  while (true)  {
    int x = 0;
    int children = currentS.children.size ();
    
    for (; x < children; x++)
      if (currentS.children[x].label == "Syll")  {
        treeList.append (currentS.children[x]);
        break;
      }
      
    for (x = 0; x < children; x++)
      if (currentS.children[x].label == "S")  {
        currentS = currentS.children[x];
        break;
      }
      
    if (x == children)
      break;
  }
  
  // for each syllable
  for (int x = 0; x < treeList.size (); x++)  {
    Syllable s;
    syllList.append (s);
    
    TreeNode syll = treeList[x];
    
    // deal with before and after supras
    while (true)  {
      QString beforeText = "";
      QString afterText = "";
      bool subSyllFound = false;
      
      for (int c = 0; c < syll.children.size (); c++)  {
        if (syll.children[c].label == "Syll")  {
          subSyllFound = true;
          continue;
        }
        
        if (subSyllFound && syll.children[c].label.startsWith ("Char"))
          afterText.append (syll.children[c].payload);
        else if (syll.children[c].label.startsWith ("Char"))
          beforeText.append (syll.children[c].payload);
      }
               
      if (!subSyllFound) break;
      
      if (beforeText != "")
        for (int s = 0; s < beforeSupras.size (); s++)  {    
          if (beforeSupras[s].domain == SUPRA_DOMAIN_SYLL &&
              beforeSupras[s].text == beforeText)  {
            syllList[x].supras.append (beforeSupras[s].name);
            break;
          }
        }
          
      if (afterText != "")
        for (int s = 0; s < afterSupras.size (); s++)
          if (afterSupras[s].domain == SUPRA_DOMAIN_SYLL &&
              afterSupras[s].text == afterText)  {
            syllList[x].supras.append (afterSupras[s].name);
          }
          
      for (int c = 0; c < syll.children.size (); c++)
        if (syll.children[c].label == "Syll")
          syll = syll.children[c];
    }
    
    // for each segment
    for (int loc = ONSET; loc <= CODA; loc++)  {
      QString label;
      if (loc == ONSET) label = "Onset";
      else if (loc == PEAK) label = "Peak";
      else label = "Coda";
      
      TreeNode segment;
      for (int c = 0; c < syll.children.size (); c++)
        if (syll.children[c].label == label)
          segment = syll.children[c];
        
      // for each phoneme
      for (int c = 0; c < segment.children.size (); c++)  {
        if (segment.children[c].children.size () == 0) 
          continue;
        
        TreeNode phonNode = segment.children[c].children[0];
        if (!phonNode.label.startsWith ("Phon"))
          continue;
        
        Phoneme p;
        p.name = phonNode.label.remove (0, 4);
        
        // deal with before and after supras
        while (true)  {
          QString beforeText = "";
          QString afterText = "";
          bool subPhonFound = false;
          
          for (int c = 0; c < phonNode.children.size (); c++)  { 
            if (phonNode.children[c].label.startsWith ("Phon"))  {
              subPhonFound = true;
              continue;
            }
        
            if (subPhonFound) afterText.append (phonNode.children[c].payload);
            else beforeText.append (phonNode.children[c].payload);
          }
          
          if (!subPhonFound) break;
      
          if (beforeText != "")
            for (int s = 0; s < beforeSupras.size (); s++)  {
              if (beforeSupras[s].domain == SUPRA_DOMAIN_PHON &&
                  beforeSupras[s].text == beforeText)  {
                p.supras.append (beforeSupras[s].name);
                break;
              }
            }
          
          if (afterText != "")
            for (int s = 0; s < afterSupras.size (); s++)
              if (afterSupras[s].domain == SUPRA_DOMAIN_PHON &&
                  afterSupras[s].text == afterText)  {
                p.supras.append (afterSupras[s].name);
              }
              
          for (int c = 0; c < phonNode.children.size (); c++)
            if (phonNode.children[c].label.startsWith ("Phon"))
              phonNode = phonNode.children[c];
        }
        
        QString spelling = "";
        for (int c = 0; c < phonNode.children.size (); c++)
          spelling.append (phonNode.children[c].payload);
        
        if (!phonemes[p.name].contains (spelling))  {
          for (int sp = 0; sp < phonemes[p.name].size (); sp++)
            if (spelling == phonemes[p.name][sp] + phonemes[p.name][sp])
              for (int s = 0; s < doubledSupras.size (); s++)  {
                if (doubledSupras[s].domain == SUPRA_DOMAIN_PHON)  {
                  p.supras.append (doubledSupras[s].name);
                  break;
                }
                
                else if (loc == PEAK)  {
                  syllList[x].supras.append (doubledSupras[s].name);
                  break;
                }
              }
              
          for (int sp = 0; sp < phonemes[p.name].size (); sp++)
            for (int s = 0; s < diacriticSupras.size (); s++)  {
              if (spelling == db.applyDiacritic (diacriticSupras[s].type, phonemes[p.name][sp]))  {
                if (diacriticSupras[s].domain == SUPRA_DOMAIN_PHON)  {
                  p.supras.append (diacriticSupras[s].name);
                  break;
                }
                
                else if (loc == PEAK)  {
                  syllList[x].supras.append (diacriticSupras[s].name);
                  break;
                }
              }
            }
        }
        
        if (loc == ONSET) syllList[x].onset.append (p);
        else if (loc == PEAK) syllList[x].peak.append (p);
        else syllList[x].coda.append (p);
      }
    }
  }
  
  return syllList;
}