#ifndef PHONOLOGYPAGE_H
#define PHONOLOGYPAGE_H

#include <QWidget>

//#include <QList>
//#include <QPointer>
#include <QMap>

#include "cdicdatabase.h"

class QSqlQueryModel;
class QDataWidgetMapper;
class QLabel;
class QPushButton;
class QListView;
class QRadioButton;
class QLineEdit;
class QTextEdit;
class QComboBox;
class QHBoxLayout;
class QVBoxLayout;
class QFont;
class ManageFeaturesDialog;
class FeatureBundlesDialog;

class PhonologyPage : public QWidget {
  Q_OBJECT
  
  public:
    PhonologyPage ();
    
    void clearDB ();
    void setDB (CDICDatabase);
    void updateModels ();
    
  signals:
    void naturalClassListUpdated ();
    void spellingsChanged ();

  private slots:
    void setChanged ();
    
    void addPhoneme ();
    void displayPhoneme ();
    void moveUp ();
    void moveDown ();
    void deletePhoneme ();
    void applyNaturalClass ();
    void updateNCModel ();
    
    void convertToIPA ();
    void submitChanges ();

    void launchFeaturesDialog ();
    void launchNaturalClassesDialog ();
    void launchEditFeaturesDialog ();

  private:
    static QMap<QString, QString> initIPATable ();
    
    static const QMap<QString, QString> ipaTable;
    
    CDICDatabase db;
    
    bool squareBrackets;

    ManageFeaturesDialog *featuresDialog;
    FeatureBundlesDialog *featureBundlesDialog;

    QSqlQueryModel *phonemeModel;
    QSqlTableModel *displayModel;
    QDataWidgetMapper *mapper;
    
    QPushButton *addPhonemeButton;
    QLineEdit *addPhonemeEdit;
    QListView *phonemeListView;
    QPushButton *manageFeaturesButton;
    QPushButton *manageNaturalClassesButton;
    QPushButton *assignNaturalClassButton;
    QComboBox *assignNaturalClassBox;
    
    QPushButton *moveUpButton;
    QPushButton *moveDownButton;
    QPushButton *deleteButton;
    
    QLineEdit *phonemeNameEdit;
    QPushButton *ipaButton;
    QLineEdit *phonemeSpellingEdit;
    QLabel *naturalClassDisplay;
    QLabel *featuresDisplay;
    QPushButton *editFeaturesButton;
    QTextEdit *notesEdit;
    QPushButton *submitButton;

    QHBoxLayout *addPhonemeLayout;
    QHBoxLayout *dialogButtonsLayout;
    QHBoxLayout *assignNaturalClassLayout;
    QVBoxLayout *listLayout;
    QVBoxLayout *moveUpDownLayout;
    QHBoxLayout *phonemeNameLayout;
    QHBoxLayout *phonemeSpellingLayout;
    QVBoxLayout *phonemeInfoLayout;
    QHBoxLayout *mainLayout;
};

#endif