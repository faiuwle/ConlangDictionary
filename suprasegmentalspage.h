#ifndef SUPRASEGMENTALSPAGE_H
#define SUPRASEGMENTALSPAGE_H

#include <QWidget>

#include "cdicdatabase.h"

class QSqlQueryModel;
class QSqlTableModel;
class QDataWidgetMapper;
class QListView;
class QPushButton;
class QLineEdit;
class QGroupBox;
class QRadioButton;
class QComboBox;
class QTextEdit;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;

class ChoosePhonemesDialog;

class SuprasegmentalsPage : public QWidget {
  Q_OBJECT

  public:
    SuprasegmentalsPage ();
    
    void clearDB ();
    void setDB (CDICDatabase);
    void updateModels ();
    
  signals:
    void suprasChanged ();

  private slots:
    void addSuprasegmental ();
    void deleteSuprasegmental ();
    void displaySuprasegmental ();
    void setChanged ();
    void launchPhonemesDialog ();
    void submitChanges ();

  private:
    CDICDatabase db;

    ChoosePhonemesDialog *choosePhonemesDialog;

    QSqlQueryModel *suprasegmentalModel;
    QSqlTableModel *displayModel;
    QDataWidgetMapper *mapper;

    QPushButton *addSuprasegmentalButton;
    QLineEdit *addSuprasegmentalEdit;
    QListView *suprasegmentalListView;
    QPushButton *deleteButton;

    QLineEdit *nameEdit;
    QComboBox *domainBox;
    QLabel *applicablePhonemesLabel;
    QPushButton *editPhonemesButton;
    
    QComboBox *spellTypeBox;
    QLineEdit *spellTextEdit;
    
    QComboBox *repTypeBox;
    QLineEdit *repTextEdit;

    QTextEdit *descriptionEdit;
    QPushButton *submitButton;

    QHBoxLayout *addSuprasegmentalLayout;
    QVBoxLayout *suprasegmentalListLayout;
    QHBoxLayout *nameLayout;
    QHBoxLayout *domainLayout;
    QHBoxLayout *phonemesLayout;
    QGridLayout *spellRepLayout;
    QVBoxLayout *infoLayout;
    QHBoxLayout *mainLayout;
};

#endif