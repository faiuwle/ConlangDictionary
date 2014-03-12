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
  languageBox->addItem (db.getValue (LANGUAGE_NAME));
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

void WordPage::setChanged ()  {
//  cout << "setChanged" << endl;
  
  submitButton->setEnabled (true);
}

void WordPage::addWord ()  {
  if (addWordEdit->text () != "")
    db.addWord (addWordEdit->text ());
  
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
//  cout << "displayWord" << endl;
  
  if (!wordView->selectionModel ()->currentIndex ().isValid ())
    return;
  
  int row = wordView->selectionModel ()->currentIndex ().row ();
  int currentID = wordModel->record (row).value ("id").toInt ();
  
  mapper->setCurrentIndex (currentID);
  
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
//  cout << "submitChanges" << endl;
  
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
  
  editPhonologyDialog = new EditPhonologyDialog (db, mapper->currentIndex ());
  
  connect (editPhonologyDialog, SIGNAL (done ()), this, SLOT (displayWord ()));
  
  editPhonologyDialog->show ();
}

void WordPage::parseWord ()  {
  submitChanges ();
  parseWord (mapper->currentIndex ());
  displayWord ();
}

void WordPage::parseWord (int id)  {
  if (dirty)  {
    phonemes = db.getPhonemesAndSpellings ();/*
    onsets = db.getPhonotacticSequenceList (ONSET);
    peaks = db.getPhonotacticSequenceList (PEAK);
    codas = db.getPhonotacticSequenceList (CODA);*/
    diacriticSupras = db.getDiacriticSupras ();
    beforeSupras = db.getBeforeSupras ();
    afterSupras = db.getAfterSupras ();
    doubledSupras = db.getDoubledSupras ();

    QList<Rule> ruleList = db.getParsingGrammar ();
    parser.setRules (ruleList);
    
//    QTextStream terminal (stdout);
    
//    db.setPhonology (1, convertTree (parser.parse (db.getWordName (1))));
    
//    terminal << db.getRepresentation (1) << endl;
    
    dirty = false;
  }
  
  QString word = db.getWordName (id);
  db.setPhonology (id, convertTree (parser.parse (word)));
  
  emit wordParsed ();
  
  QTextStream terminal (stdout);
//  terminal << word << ", " << db.getRepresentation (id) << endl;
  
/*
  if (db.getValue (USE_PHONOTACTICS) == "false")
    return;
  
  QString unparsed = db.getWordName (id);
  bool onsetRequired = (db.getValue (ONSET_REQUIRED) == "true");
  tempSyllList.clear ();
  int currentSyll = 0;
  
  QString ignored = db.getValue (IGNORED_CHARACTERS);
  for (int x = 0; x < ignored.size (); x++)
    unparsed.remove (ignored[x]);
  
  while (unparsed != "")  {
    if (tempSyllList.size () == currentSyll)  {
      Syllable s;
      s.beforeText = "";
      s.onsetText.clear ();
      s.peakText.clear ();
      s.codaText.clear ();
      s.afterText = "";
      s.onsetNum = -1;
      s.peakNum = -1;
      s.codaNum = -1;
      tempSyllList.append (s);
    }
    
    for (int x = 0; x < beforeSupras.size (); x++)  {
      if (beforeSupras[x].domain == SUPRA_DOMAIN_PHON)
        continue;
      
      if (unparsed.startsWith (beforeSupras[x].text))  {
        tempSyllList[currentSyll].supras.append (beforeSupras[x].name);
        tempSyllList[currentSyll].beforeText = beforeSupras[x].text;
        unparsed.remove (0, beforeSupras[x].text.size ());
        break;
      }
    }
    
    if (!parseOnset (unparsed, currentSyll, false))
      if (onsetRequired || unparsed == "")
        break;
      
    if (!parsePeak (unparsed, currentSyll, false))
      break;
    
    for (int x = 0; x < afterSupras.size (); x++)  {
      if (afterSupras[x].domain == SUPRA_DOMAIN_PHON)
        continue;
      
      if (unparsed.startsWith (afterSupras[x].text))  {
        tempSyllList[x].supras.append (afterSupras[x].name);
        tempSyllList[x].afterText = afterSupras[x].text;
        unparsed.remove (0, afterSupras[x].text.size ());
        break;
      }
    }
    
    currentSyll++;
  }
  
  db.setPhonology (id, tempSyllList);
*/
}

/*
bool WordPage::parseOnset (QString &unparsed, int current, bool reparse)  {
  if (unparsed == "" && !reparse) return false;
  
  int currentOnset = 0;
  
  if (reparse)  {
    currentOnset = tempSyllList[current].onsetNum;
//    unparsed.prepend (tempSyllList[current].onsetText.join (""));
//    tempSyllList[current].onset.clear ();
//    tempSyllList[current].onsetText.clear ();
  }
  
  if (currentOnset < 0) currentOnset = 0;
  
  bool onsetFound = false;
  
  for (int x = currentOnset; x < onsets.size (); x++)  {
    bool sequenceReparse = (x == currentOnset ? reparse : false);
    
    if (trySequence (unparsed, current, ONSET, x, sequenceReparse))  {
      onsetFound = true;
      tempSyllList[current].onsetNum = x;
//      tempSyllList[current].onset = phonemeList;
//      tempSyllList[current].onsetText = spellingList.join ("");
//      unparsed.remove (0, spellingList.join ("").size ());
      break;
    }
  }
    
  if (!onsetFound && current == 0)
    return false;
  
  if (!onsetFound)  {
    if (parseCoda (unparsed, current - 1))
      return parseOnset (unparsed, current, false);
    else return false;
  }
  
  else return true;
}

bool WordPage::parsePeak (QString &unparsed, int current, bool reparse)  {
  if (unparsed == "" && !reparse)
    if (!parseOnset (unparsed, current, true))
      return false;
    
  int currentPeak = 0;
  
  if (reparse)  {
    currentPeak = tempSyllList[current].peakNum;
//    unparsed.prepend (tempSyllList[current].peakText.join (""));
//    tempSyllList[current].peak.clear ();
//    tempSyllList[current].peakText.clear ();
  }
  
  if (currentPeak < 0) currentPeak = 0;
  
  bool peakFound = false;
  
  for (int x = currentPeak; x < peaks.size (); x++)  {
    bool sequenceReparse = (x == currentPeak ? reparse : false);
    
    if (trySequence (unparsed, current, PEAK, x, sequenceReparse))  {
      peakFound = true;
      tempSyllList[current].peakNum = x;
//      tempSyllList[current].peak = phonemeList;
//      tempSyllList[current].peakText = spellingList.join ("");
//      unparsed.remove (0, spellingList.join ("").size ());
      break;
    }
  }
    
  if (!peakFound)  {
    if (parseOnset (unparsed, current, true))
      return parsePeak (unparsed, current, false);
    else return false;
  }
  
  else return true;
}

bool WordPage::parseCoda (QString &unparsed, int current)  {
  if (codas.size () == 0) return false;
  
  unparsed.prepend (tempSyllList[current].afterText);
  tempSyllList[current].afterText = "";
  for (int x = 0; x < afterSupras.size (); x++)
    tempSyllList[current].supras.removeAll (afterSupras[x].name);
    
  int currentCoda = 0;
  currentCoda = tempSyllList[current].codaNum;
  if (currentCoda < 0) currentCoda = 0;
//  unparsed.prepend (tempSyllList[current].codaText.join (""));
//  tempSyllList[current].coda.clear ();
//  tempSyllList[current].codaText.clear ();
  
  bool codaFound = false;
  
  for (int x = currentCoda; x < codas.size (); x++)  {
    bool sequenceReparse = (x == currentCoda);
    
    if (trySequence (unparsed, current, CODA, x, sequenceReparse))  {
      codaFound = true;
      tempSyllList[current].codaNum = x;
//      tempSyllList[current].coda = phonemeList;
//      tempSyllList[current].codaText = spellingList.join ("");
//      unparsed.remove (0, spellingList.join ("").size ());
      break;
    }
  }
    
  if (codaFound)
    for (int x = 0; x < afterSupras.size (); x++)  {
      if (afterSupras[x].domain == SUPRA_DOMAIN_PHON)
        continue;
      
      if (unparsed.startsWith (afterSupras[x].text))  {
        tempSyllList[current].supras.append (afterSupras[x].name);
        tempSyllList[current].afterText = afterSupras[x].text;
        unparsed.remove (0, afterSupras[x].text.size ());
      }
    }
    
  return codaFound;
}

bool WordPage::trySequence (QString &unparsed, int current, int loc, 
                            int index, bool reparse)  {
  QStringList previous;
  
  if (reparse)  {
    if (loc == ONSET)  {
      for (int x = 0; x < tempSyllList[current].onset.size (); x++)
        previous.append (tempSyllList[current].onset[x])
      
      unparsed.prepend (tempSyllList[current].onsetText.join (""));
      tempSyllList[current].onset.clear ();
      tempSyllList[current].onsetText.clear ();
    }
    
    else if (loc == PEAK)  {
      for (int x = 0; x < tempSyllList[current].peak.size (); x++)
        previous.append (tempSyllList[current].peak[x]);
      
      unparsed.prepend (tempSyllList[current].peakText.join (""));
      tempSyllList[current].peak.clear ();
      tempSyllList[current].peakText.clear ();
    }
    
    else  {
      for (int x = 0; x < tempSyllList[current].coda.size (); x++)
        previous.append (tempSyllList[current].coda[x]);
      
      unparsed.prepend (tempSyllList[current].codaText.join (""));
      tempSyllList[current].coda.clear ();
      tempSyllList[current].codaText.clear ();
    }
  }
  
  QList<QStringList> sequence;
  if (loc == ONSET) sequence = onsets[index];
  else if (loc == PEAK) sequence = peaks[index];
  else sequence = codas[index];
                        
  if (previous.size () != sequence.size ())
    reparse = false;
  
  for (int x = 0; x < sequence.size (); x++)  {
    QStringList possiblePhonemes = db.getPhonemesOfClass (sequence[x]);
    
    int currentPhoneme = 0;
    if (reparse)
      currentPhoneme = possiblePhonemes.indexOf (previous[x]);
    
    bool phonemeFound = false;
    
    for (int p = currentPhoneme; p < possiblePhonemes.size (); p++)  {
      QStringList spellingList = phonemes[possiblePhonemes[p]];
      
      for (int s = 0; s < spellingList.size (); s++)  {
        QStringList supras;
        QString actualSpelling = trySpelling (unparsed, spellingList[s], supras);
        
        if (actualSpelling != "")  {
          phonemeFound = true;
          
          Phoneme ph;
          ph.name = possiblePhonemes[p];
          ph.supras = supras;
          
          if (loc == ONSET)  {
            tempSyllList[current].onset.append (ph);
            tempSyllList[current].onsetText.append (actualSpelling);
          }
          
          else if (loc == PEAK)  {
            tempSyllList[current].peak.append (ph);
            tempSyllList[current].peakText.append (actualSpelling);
          }
          
          else  {
            tempSyllList[current].coda.append (ph);
            tempSyllList[current].codaText.append (actualSpelling);
          }
          
          unparsed.remove (0, actualSpelling.size ());
          break;
        }
      }
      
      if (phonemeFound) break;
    }
    
    if (!phonemeFound) return false;
    else phonemeFound = false;
  }
  
  return true;
}

QString WordPage::trySpelling (QString unparsed, QString spelling, QStringList &supras)  {
  if (unparsed == "") return "";
                        
  return "";
}*/

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
  
//  for (int x = 0; x < treeList.size (); x++)
//    terminal << parser.stringTreeNode (treeList[x]) << endl;
  
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
          
//          terminal << beforeText << endl;
      
          if (beforeText != "")
            for (int s = 0; s < beforeSupras.size (); s++)  {
//              terminal << beforeSupras[s].domain << ", " << beforeSupras[s].text << endl;
              
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
//              terminal << diacriticSupras[s].type << db.applyDiacritic (diacriticSupras[s].type, phonemes[p.name][sp]) << endl;
              
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