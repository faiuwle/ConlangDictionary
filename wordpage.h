#ifndef WORDPAGE_H
#define WORDPAGE_H

#include <QWidget>

#include "cdicdatabase.h"
#include "earleyparser.h"

class QPushButton;
class QLineEdit;
class QTreeView;
class QComboBox;
class QLabel;
class QTextEdit;
class QSqlQueryModel;
class QSqlTableModel;
class QDataWidgetMapper;
class QHBoxLayout;
class QVBoxLayout;

class ManageFeaturesDialog;
class FeatureBundlesDialog;
class EditPhonologyDialog;

class WordPage : public QWidget  {
  Q_OBJECT
  
  public:
    WordPage ();
    
    void clearDB ();
    void setDB (CDICDatabase);
    void updateModels ();
    
  signals:
    void wordParsed ();
    void parsingFinished ();
    
  public slots:
    void setDirty ();
    void parseWordlist ();
    void setLanguageName (QString);
    
  private slots:
    void setChanged ();
    
    void addWord ();
    void deleteWord ();
    void search ();
    void displayWord ();
    void applyNaturalClass ();
    void updateNCModel ();
    void updateAfterFeatureChange ();
    
    void submitChanges ();
    
    void launchFeaturesDialog ();
    void launchNaturalClassesDialog ();
    void launchEditFeaturesDialog ();
    void launchEditPhonologyDialog ();
    
    void parseWord ();
    
  private:
    void parseWord (int);
    
    QList<Syllable> convertTree (TreeNode);
    
    // used for word parsing
    bool dirty;
    EarleyParser parser;
    QMap<QString, QStringList> phonemes;
    QList<Suprasegmental> diacriticSupras;
    QList<Suprasegmental> beforeSupras;
    QList<Suprasegmental> afterSupras;
    QList<Suprasegmental> doubledSupras;
    
    CDICDatabase db;
    
    ManageFeaturesDialog *featuresDialog;
    FeatureBundlesDialog *featureBundlesDialog;
    EditPhonologyDialog *editPhonologyDialog;
    
    QSqlQueryModel *wordModel;
    QSqlTableModel *displayModel;
    QDataWidgetMapper *mapper;
    
    QPushButton *addWordButton;
    QLineEdit *addWordEdit;
    QPushButton *deleteWordButton;
    QPushButton *searchButton;
    QLineEdit *searchEdit;
    QComboBox *naturalClassBox;
    QComboBox *languageBox;
    QTreeView *wordView;
    QPushButton *manageFeaturesButton;
    QPushButton *manageNaturalClassesButton;
    QPushButton *assignNaturalClassButton;
    QComboBox *assignNaturalClassBox;
    
    QLineEdit *spellingEdit;
    QPushButton *changeButton;
    QLabel *phonologyDisplay;
    QPushButton *editPhonologyButton;
    QLabel *naturalClassDisplay;
    QLabel *featuresDisplay;
    QPushButton *editFeaturesButton;
    QTextEdit *definitionEdit;
    QPushButton *submitButton;
    
    QHBoxLayout *addWordLayout;
    QHBoxLayout *searchLayout;
    QHBoxLayout *dialogButtonsLayout;
    QHBoxLayout *assignNaturalClassLayout;
    QVBoxLayout *listLayout;
    QHBoxLayout *spellingLayout;
    QHBoxLayout *phonologyLayout;
    QVBoxLayout *wordInfoLayout;
    QHBoxLayout *mainLayout;
};

#endif