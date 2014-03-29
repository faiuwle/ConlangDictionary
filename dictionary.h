#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <QTabWidget>

#include <QList>
#include <QPointer>

#include "cdicdatabase.h"

class MainWindow;
class PhonologyPage;
class PhonotacticsPage;
class SuprasegmentalsPage;
class WordPage;
class MorphemeUpdateDialog;

class Dictionary : public QTabWidget {
  Q_OBJECT

  public:
    Dictionary ();
    
    bool isOpen ();

    void openDB (QString);
    void closeDB ();
    void loadXML (QString);
    void loadText (QString, QString);
    void loadLexique (QString, QStringList);
    void loadFeatures (int, QString);
    void saveText (QString, QString);
    void saveFeatures (int, QString);
    bool clearDictionary ();
    bool clearWordlist ();
    
    void setValue (QString, QString);
    QString getValue (QString);
    
  signals:
    void languageNameUpdated (QString);
    void bracketsUpdated (bool);
    void unicodeUpdated (bool);
    
  private slots:
    void setMorphemeList (QList<int>);

  private:
    void setDB ();
    void updateModels ();
    
    void launchMorphemeUpdateDialog ();

    CDICDatabase db;
    
    MorphemeUpdateDialog *morphemeUpdateDialog;

    PhonologyPage *phonologyPage;
    PhonotacticsPage *phonotacticsPage;
    SuprasegmentalsPage *suprasegmentalsPage;
    WordPage *wordPage;
};

#endif