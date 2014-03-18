#ifndef EDITPHONOLOGYDIALOG_H
#define EDITPHONOLOGYDIALOG_H

#include <QDialog>

#include "const.h"
#include "cdicdatabase.h"

class QPushButton;
class QLabel;
class QString;
class QComboBox;
class QSignalMapper;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;

class EditPhonologyDialog : public QDialog  {
  Q_OBJECT
  
  public:
    EditPhonologyDialog (CDICDatabase, int);
    
  signals:
    void done ();
    
  private slots:
    void addPhoneme (const QString&);
    
    void nextSyllable ();
    void prevSyllable ();
    
    void newSyllable ();
    void goBack ();
    void clear ();
    
    void nextSegment ();
    void prevSegment ();
    
    void applySupra ();
    
    void submit ();
  
  private:
    QString getRepresentation ();
    QString applyDiacritic (int, QString);
    
    QList<Syllable> phonology;
    QList<Suprasegmental> supras;
    int currentSyllable;
    
    CDICDatabase db;
    int wordID;
    
    QSignalMapper *signalMapper;
    
    QPushButton *dotButton;
    QPushButton *backButton;
    QPushButton *clearButton;
    
    QPushButton *prevSyllableButton;
    QPushButton *nextSyllableButton;
    
    QPushButton *prevSegmentButton;
    QPushButton *nextSegmentButton;
    QLabel *segmentLabel;
    
    QLabel *sequenceLabel;
    
    QPushButton *applySupraButton;
    QComboBox *applySupraBox;
    
    QPushButton *submitButton;
    
    QGridLayout *phonemeLayout;
    QHBoxLayout *dotBackClearLayout;
    QHBoxLayout *syllableNavigationLayout;
    QHBoxLayout *segmentNavigationLayout;
    QHBoxLayout *applySupraLayout;
    QVBoxLayout *mainLayout;
};

#endif