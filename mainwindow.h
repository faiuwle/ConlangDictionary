#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "dictionary.h"

class QMenu;
class QAction;

class MainWindow : public QMainWindow {
  Q_OBJECT

  public:
    MainWindow ();
    void setPath (QString);
    void openDictionary (QString);

  private slots:
    void newDictionary ();
    void openDictionary ();
    
    void loadXML ();
    void loadText ();
    void loadLexique ();
    void loadPhonFeatures ();
    void loadWordFeatures ();
    
    void saveText ();
    void exportPhonFeatures ();
    void exportWordFeatures ();
    
    void clearDictionary ();
    void quit ();
    
    void setFont ();
    void setLanguageName ();
    void setBrackets (bool);
    void setUnicode (bool);
    void setPath ();
    
    void updateWindowTitle (QString);
    void updateBrackets (bool);
    void updateUnicode (bool);

  private:
    void createActions ();
    void createMenus ();
    
    Dictionary *dictionary;
    QString path;
    
    QMenu *fileMenu;
    QMenu *importMenu;
    QMenu *exportMenu;
    QMenu *settingsMenu;
    
    QAction *newAct;
    QAction *openAct;
    
    QAction *loadXMLAct;
    QAction *loadTextAct;
    QAction *loadLexiqueAct;
    QAction *loadPhonFeaturesAct;
    QAction *loadWordFeaturesAct;
    
    QAction *saveTextAct;
    QAction *exportPhonFeaturesAct;
    QAction *exportWordFeaturesAct;
    
    QAction *clearAct;
    QAction *quitAct;
    
    QAction *fontAct;
    QAction *langNameAct;
    QAction *bracketsAct;
    QAction *unicodeAct;
    QAction *pathAct;
};

#endif