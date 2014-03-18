#include <QFile>

#include <QTextStream>
#include <QMessageBox>

#include <iostream>
using namespace std;

#include "const.h"

#include "dictionary.h"
#include "mainwindow.h"

#include "phonologypage.h"
#include "phonotacticspage.h"
#include "suprasegmentalspage.h"
#include "wordpage.h"

Dictionary::Dictionary ()  {
  setMinimumWidth (700);
  setMinimumHeight (550);

  phonologyPage = new PhonologyPage ();
  phonotacticsPage = new PhonotacticsPage;
  suprasegmentalsPage = new SuprasegmentalsPage;
  wordPage = new WordPage;

  addTab (phonologyPage, "Phonemes");
  addTab (suprasegmentalsPage, "Suprasegmentals");
  addTab (phonotacticsPage, "Phonotactics");
  addTab (wordPage, "Words");

  connect (phonologyPage, SIGNAL (naturalClassListUpdated ()), phonotacticsPage,
           SLOT (updateClassList ()));
  
  connect (phonologyPage, SIGNAL (spellingsChanged ()), wordPage, 
           SLOT (setDirty ()));
  connect (suprasegmentalsPage, SIGNAL (suprasChanged ()), wordPage,
           SLOT (setDirty ()));
  connect (phonotacticsPage, SIGNAL (phonotacticsChanged ()), wordPage,
           SLOT (setDirty ()));
  
  connect (phonotacticsPage, SIGNAL (reanalyze ()), wordPage,
           SLOT (parseWordlist ()));
  
  connect (wordPage, SIGNAL (wordParsed ()), phonotacticsPage,
           SLOT (incrementProgressBar ()));
  connect (wordPage, SIGNAL (parsingFinished ()), phonotacticsPage,
           SLOT (resetProgressBar ()));
}

bool Dictionary::isOpen ()  {
  return (db.currentDB () != "");
}

void Dictionary::openDB (QString filename)  {
  closeDB ();
  
  if (!db.open (filename))
    QMessageBox::warning (this, "", "Could not open database: " + filename);
  
  else  {
    QString langName = db.getValue (LANGUAGE_NAME);
    
    if (langName.isEmpty ())
      emit languageNameUpdated ("Conlang");
    else emit languageNameUpdated (langName);
    
    emit bracketsUpdated (db.getValue (SQUARE_BRACKETS) == "true");
    emit unicodeUpdated (db.getValue (USE_UNICODE) == "true");
    
    setDB ();
  }
}

void Dictionary::closeDB ()  {
  phonologyPage->clearDB ();
  suprasegmentalsPage->clearDB ();
  phonotacticsPage->clearDB ();
  wordPage->clearDB ();
  
  db.close ();
}

void Dictionary::loadXML (QString filename)  {
  if (db.loadFromXML (filename))  {
    QString langName = db.getValue (LANGUAGE_NAME);
    
    if (langName.isEmpty ())
      emit languageNameUpdated ("Conlang");
    else emit languageNameUpdated (langName);
    
    emit bracketsUpdated (db.getValue (SQUARE_BRACKETS) == "true");
    emit unicodeUpdated (db.getValue (USE_UNICODE) == "true");
  }
  
  updateModels ();
}

void Dictionary::loadText (QString filename, QString pattern)  {
  db.loadFromText (filename, pattern);
  updateModels ();
}

void Dictionary::loadFeatures (int domain, QString filename)  {
  db.loadFeatures (domain, filename);
  updateModels ();
}

void Dictionary::loadLexique (QString filename, QStringList params)  {

}

void Dictionary::saveText (QString filename, QString pattern)  {
  db.saveToText (filename, pattern);
}

void Dictionary::saveFeatures (int domain, QString filename)  {
  db.saveFeatures (domain, filename);
}

bool Dictionary::clearDictionary ()  {
  if (db.currentDB ().isEmpty ()) return false;
  
  if (QMessageBox::question (this, "Clear Dictionary?", 
                             "Do you really want to delete all data from the dictionary?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return false;
  
  db.clear ();
  updateModels ();
  
  return true;
}

bool Dictionary::clearWordlist ()  {
  if (db.currentDB ().isEmpty ()) return false;
  
  if (QMessageBox::question (this, "Clear Wordlist?",
                             "Do you really want to delete all words in the wordlist?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return false;
  
  db.clearWordlist ();
  updateModels ();
  
  return true;
}

void Dictionary::setValue (QString key, QString value)  {
  if (value.isNull ()) value = "";
  
  db.setValue (key, value);
  
  if (key == LANGUAGE_NAME)
    wordPage->setLanguageName (value);
}

QString Dictionary::getValue (QString key)  {
  return db.getValue (key);
}

void Dictionary::setDB ()  {
  phonologyPage->setDB (db);
  suprasegmentalsPage->setDB (db);
  phonotacticsPage->setDB (db);
  wordPage->setDB (db);
}

void Dictionary::updateModels ()  {
  phonologyPage->updateModels ();
  suprasegmentalsPage->updateModels ();
  phonotacticsPage->updateModels ();
  wordPage->updateModels ();
}