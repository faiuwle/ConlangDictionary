#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QFontDialog>
#include <QTextStream>

#include <iostream>
using namespace std;

#include "mainwindow.h"

MainWindow::MainWindow ()  {
  dictionary = new Dictionary ();
  setCentralWidget (dictionary);
  
  path = ".";

  QIcon icon ("ConlangDic.png");
  setWindowIcon (icon);
  setWindowTitle ("Conlang Dictionary");
  
  createActions ();
  createMenus ();

  connect (dictionary, SIGNAL (languageNameUpdated (QString)), this,
           SLOT (updateWindowTitle (QString)));
  connect (dictionary, SIGNAL (bracketsUpdated (bool)), this,
           SLOT (updateBrackets (bool)));
  connect (dictionary, SIGNAL (unicodeUpdated (bool)), this,
           SLOT (updateUnicode (bool)));
}

void MainWindow::setPath (QString p)  {
  path = p;
}

void MainWindow::openDictionary (QString filename)  {
  dictionary->openDB (filename);
}

void MainWindow::newDictionary ()  {
  QString filename = QFileDialog::getSaveFileName (this, "Create Dictionary", path,
                                                   "Dictionaries (*.cdic)");
                                                   
  if (filename.isEmpty ()) return;
  
  if (!filename.endsWith (".cdic"))
    filename += ".cdic";
  
  dictionary->openDB (filename);
}

void MainWindow::openDictionary ()  {
  QString filename = QFileDialog::getOpenFileName (this, "Open Dictionary", path,
                                                   "Dictionaries (*.cdic)");
                                                   
  if (filename.isEmpty ()) return;
  
  dictionary->openDB (filename);
  
  if (dictionary->getValue (USE_UNICODE) == "true")
    unicodeAct->setChecked (true);
  if (dictionary->getValue (SQUARE_BRACKETS) == "true")
    bracketsAct->setChecked (true);
}

void MainWindow::loadXML ()  {
  if (!dictionary->isOpen ())  {
    QMessageBox::warning (this, "Error", "Please open a new dictionary to load into.");
    return;
  }
  
  if (!dictionary->clearDictionary ())
    return;
  
  QString filename = QFileDialog::getOpenFileName (this, "Open XML", path,
                                                   "XML Dictionaries (*.cdic)");
                                                   
  if (filename.isEmpty ()) return;
  
  dictionary->loadXML (filename);
}

void MainWindow::loadText ()  {
  if (!dictionary->isOpen ())  {
    QMessageBox::warning (this, "Error", "Please open a new dictionary to load into.");
    return;
  }
  
  QString filename = QFileDialog::getOpenFileName (this, "Open text file", path,
                                                   "Text Files (*.txt)");
  
  if (filename.isEmpty ()) return;
  
  QString pattern = QInputDialog::getText (this, "Text Regex", 
                                           "Enter text regex, using /w for the word, /c for the class, and /d for the definition.",
                                           QLineEdit::Normal, "/w (/c): /d");
  
  if (pattern.isEmpty ()) return;
  
  if (pattern.count ("/w") > 1)
    QMessageBox::warning (this, "Error", "Specify only one word per line.");
  else if (pattern.count ("/c") > 1)
    QMessageBox::warning (this, "Error", "Multiple classes are not supported at this time.");
  else if (pattern.count ("/d") > 1)
    QMessageBox::warning (this, "Error", "Specify only one definition per line.");                                    
  else dictionary->loadText (filename, pattern);
}

void MainWindow::loadLexique ()  {
  QMessageBox::information (this, "", "This feature isn't implemented yet.");
}

void MainWindow::loadPhonFeatures ()  {
  if (!dictionary->isOpen ())  {
    QMessageBox::warning (this, "Error", "Please open a new dictionary to load into.");
    return;
  }
  
  QString filename = QFileDialog::getOpenFileName (this, "Open features file", path,
                                                   "Features Files (*.features)");
  
  if (filename.isEmpty ()) return;
  
  dictionary->loadFeatures (PHONEME, filename);
}

void MainWindow::loadWordFeatures ()  {
  if (!dictionary->isOpen ())  {
    QMessageBox::warning (this, "Error", "Please open a new dictionary to load into.");
    return;
  }
  
  QString filename = QFileDialog::getOpenFileName (this, "Open features file", path,
                                                   "Features Files (*.features)");
  
  if (filename.isEmpty ()) return;
  
  dictionary->loadFeatures (WORD, filename);
}

void MainWindow::saveText ()  {
  if (!dictionary->isOpen ())  {
    QMessageBox::warning (this, "Error", "Dictionary not loaded.");
    return;
  }
  
  QString filename = QFileDialog::getSaveFileName (this, "Save text file", path,
                                                   "Text Files (*.txt)");
  
  if (filename.isEmpty ()) return;
  
  QString pattern = QInputDialog::getText (this, "Text Pattern", 
                                           (QString)"Enter text regex, using /w for the " +
                                           "word, /p for the phonology /c for " +
                                           "the class(es), and /d for the definition.",
                                           QLineEdit::Normal, "/w //p/ (/c): /d");
  
  if (pattern.isEmpty ()) return;
  
  dictionary->saveText (filename, pattern);
}

void MainWindow::exportPhonFeatures ()  {
  if (!dictionary->isOpen ())  {
    QMessageBox::warning (this, "Error", "Dictionary not loaded.");
    return;
  }
  
  QString filename = QFileDialog::getSaveFileName (this, "Save features file", path,
                                                   "Features Files (*.features)");
  
  if (filename.isEmpty ()) return;
  
  if (!filename.endsWith (".features"))
    filename.append (".features");
  
  dictionary->saveFeatures (PHONEME, filename);
}

void MainWindow::exportWordFeatures ()  {
  if (!dictionary->isOpen ())  {
    QMessageBox::warning (this, "Error", "Dictionary not loaded.");
    return;
  }
  
  QString filename = QFileDialog::getSaveFileName (this, "Save features file", path,
                                                   "Features Files (*.features)");
  
  if (filename.isEmpty ()) return;
      
  if (!filename.endsWith (".features"))
    filename.append (".features");
  
  dictionary->saveFeatures (WORD, filename);
}

void MainWindow::clearDictionary ()  {
  dictionary->clearDictionary ();
}

void MainWindow::clearWordlist ()  {
  dictionary->clearWordlist ();
}

void MainWindow::quit ()  {
  dictionary->closeDB ();
  close ();
}

void MainWindow::setFont ()  {
  bool ok = false;
  QFont font = QFontDialog::getFont (&ok, QFont ("DejaVu Sans", 8), this);

  if (ok)  {
    QFile settingsFile ("cd.config");
    QTextStream out (&settingsFile);

    if (settingsFile.open (QIODevice::WriteOnly | QIODevice::Text))  {
      out << "Font " << font.toString () << endl;
      out << "Path " << path << endl;
      settingsFile.close ();
      QMessageBox::information (this, "Font Set", "Font preference saved - restart CD to see effects.");
    }
  }
}

void MainWindow::setLanguageName ()  {
  QString name = QInputDialog::getText (this, "Language Name", "Enter the name of your language.");
  dictionary->setValue (LANGUAGE_NAME, name);
  updateWindowTitle (name);
}

void MainWindow::setBrackets (bool b)  {
  dictionary->setValue (SQUARE_BRACKETS, (b ? "true" : "false"));
}

void MainWindow::setUnicode (bool b)  {
  dictionary->setValue (USE_UNICODE, (b ? "true" : "false"));
}

void MainWindow::setPath ()  {
  QString p = QFileDialog::getExistingDirectory (this, "Choose directory", path);
  
  if (p.isEmpty ()) return;
  
  path = p;
  
  QFile settingsFile ("cd.config");
  QTextStream stream (&settingsFile);
  QStringList lines;
  
  if (settingsFile.open (QIODevice::ReadOnly | QIODevice::Text))  {
    while (!stream.atEnd ())
      lines.append (stream.readLine ());
    
    settingsFile.close ();
  }
  
  if (settingsFile.open (QIODevice::WriteOnly | QIODevice::Text))  {
    for (int x = 0; x < lines.size (); x++)
      if (!lines[x].startsWith ("Path"))
        stream << lines[x] << endl;
      
    stream << "Path " << path << endl;
  }
  
  QMessageBox::information (this, "Default Path Set", 
                            "The default path has been set to " + path);
}

void MainWindow::updateWindowTitle (QString languageName)  {
  setWindowTitle (languageName + " Dictionary");
}

void MainWindow::updateBrackets (bool b)  {
  bracketsAct->setChecked (b);
}

void MainWindow::updateUnicode (bool b)  {
  unicodeAct->setChecked (b);
}

void MainWindow::createActions ()  {
  newAct = new QAction ("&New Dictionary", this);
  newAct->setShortcuts (QKeySequence::New);
  newAct->setStatusTip ("Create a new dictionary");
  connect (newAct, SIGNAL (triggered ()), this, SLOT (newDictionary ()));
  
  openAct = new QAction ("&Open Dictionary", this);
  openAct->setShortcuts (QKeySequence::Open);
  openAct->setStatusTip ("Open an existing dictionary");
  connect (openAct, SIGNAL (triggered ()), this, SLOT (openDictionary ()));
  
  loadXMLAct = new QAction ("Load From XML", this);
  loadXMLAct->setStatusTip ("Load dictionary from 0.2 XML");
  connect (loadXMLAct, SIGNAL (triggered ()), this, SLOT (loadXML ()));
  
  loadTextAct = new QAction ("Load From Text", this);
  loadTextAct->setStatusTip ("Load words from a text or CSV file");
  connect (loadTextAct, SIGNAL (triggered ()), this, SLOT (loadText ()));
  
  loadLexiqueAct = new QAction ("Load from Lexique Pro File", this);
  loadLexiqueAct->setStatusTip ("Load words from Lexique Pro");
  connect (loadLexiqueAct, SIGNAL (triggered ()), this, SLOT (loadLexique ()));
  
  loadPhonFeaturesAct = new QAction ("Load Phoneme Features", this);
  loadPhonFeaturesAct->setStatusTip ("Load features for phonemes");
  connect (loadPhonFeaturesAct, SIGNAL (triggered ()), this, SLOT (loadPhonFeatures ()));
  
  loadWordFeaturesAct = new QAction ("Load Word Features", this);
  loadWordFeaturesAct->setStatusTip ("Load features for words");
  connect (loadWordFeaturesAct, SIGNAL (triggered ()), this, SLOT (loadWordFeatures ()));
  
  saveTextAct = new QAction ("Save To Text File", this);
  saveTextAct->setStatusTip ("Save wordlist to a text file");
  connect (saveTextAct, SIGNAL (triggered ()), this, SLOT (saveText ()));
  
  exportPhonFeaturesAct = new QAction ("Export Phoneme Features", this);
  exportPhonFeaturesAct->setStatusTip ("Save phoneme features for use in other dictionaries");
  connect (exportPhonFeaturesAct, SIGNAL (triggered ()), this, SLOT (exportPhonFeatures ()));
  
  exportWordFeaturesAct = new QAction ("Export Word Features", this);
  exportWordFeaturesAct->setStatusTip ("Save word features for use in other dictionaries");
  connect (exportWordFeaturesAct, SIGNAL (triggered ()), this, SLOT (exportWordFeatures ()));
  
  clearAct = new QAction ("Clear Dictionary", this);
  clearAct->setStatusTip ("Clear all dictionary data");
  connect (clearAct, SIGNAL (triggered ()), this, SLOT (clearDictionary ()));
  
  clearWordsAct = new QAction ("Clear Wordlist", this);
  clearWordsAct->setStatusTip ("Clear just the wordlist");
  connect (clearWordsAct, SIGNAL (triggered ()), this, SLOT (clearWordlist ()));
  
  quitAct = new QAction ("&Quit", this);
  quitAct->setShortcuts (QKeySequence::Quit);
  quitAct->setStatusTip ("Exit the application");
  connect (quitAct, SIGNAL (triggered ()), this, SLOT (quit ()));
  
  fontAct = new QAction ("Set Font", this);
  fontAct->setStatusTip ("Set the application font");
  connect (fontAct, SIGNAL (triggered ()), this, SLOT (setFont ()));
  
  langNameAct = new QAction ("Set Language Name", this);
  langNameAct->setStatusTip ("Set the conlang's name");
  connect (langNameAct, SIGNAL (triggered ()), this, SLOT (setLanguageName ()));
  
  bracketsAct = new QAction ("Use Square Brackets", this);
  bracketsAct->setStatusTip ("Use square brackets for phonemes instead of slashes");
  bracketsAct->setCheckable (true);
  connect (bracketsAct, SIGNAL (toggled (bool)), this, SLOT (setBrackets (bool)));
  
  unicodeAct = new QAction ("Force Unicode", this);
  unicodeAct->setStatusTip ("Force imported text files to be read as unicode");
  unicodeAct->setCheckable (true);
  connect (unicodeAct, SIGNAL (toggled (bool)), this, SLOT (setUnicode (bool)));
  
  pathAct = new QAction ("Set Default Path", this);
  pathAct->setStatusTip ("Set the default path for saving and loading");
  connect (pathAct, SIGNAL (triggered ()), this, SLOT (setPath ()));
}

void MainWindow::createMenus ()  {
  fileMenu = menuBar()->addMenu ("File");
  fileMenu->addAction (newAct);
  fileMenu->addAction (openAct);
  
  importMenu = fileMenu->addMenu ("Import");
  importMenu->addAction (loadXMLAct);
  importMenu->addAction (loadTextAct);
  importMenu->addAction (loadLexiqueAct);
  importMenu->addAction (loadPhonFeaturesAct);
  importMenu->addAction (loadWordFeaturesAct);
  
  exportMenu = fileMenu->addMenu ("Export");
  exportMenu->addAction (saveTextAct);
  exportMenu->addAction (exportPhonFeaturesAct);
  exportMenu->addAction (exportWordFeaturesAct);
  
  fileMenu->addAction (clearAct);
  fileMenu->addAction (clearWordsAct);
  fileMenu->addSeparator ();
  fileMenu->addAction (quitAct);
  
  settingsMenu = menuBar ()->addMenu ("Settings");
  settingsMenu->addAction (fontAct);
  settingsMenu->addAction (langNameAct);
  settingsMenu->addAction (bracketsAct);
  settingsMenu->addAction (unicodeAct);
  settingsMenu->addAction (pathAct);
}
