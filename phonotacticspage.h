#ifndef PHONOTACTICSPAGE_H
#define PHONOTACTICSPAGE_H

#include <QWidget>
#include <QList>

#include "cdicdatabase.h"

class QCheckBox;
class QLineEdit;
class QListView;
class QStringListModel;
class QPushButton;
class QLabel;
class QComboBox;
class QProgressBar;
class QHBoxLayout;
class QVBoxLayout;

class PhonotacticsPage : public QWidget  {
  Q_OBJECT

  public:
    PhonotacticsPage ();
    
    void clearDB ();
    void setDB (CDICDatabase);
    void updateModels ();
    
  signals:
    void phonotacticsChanged ();
    void reanalyze ();

  public slots:
    void updateClassList ();
    void incrementProgressBar ();
    void resetProgressBar ();
    
  private slots:
    void setOnsetRequired (bool);
    void setIgnored (QString);
    void setUsePhonotactics (bool);
    
    void addClass ();
    void removeClass ();
    void goLeft ();
    void goRight ();
    
    void addAsOnset ();
    void addAsPeak ();
    void addAsCoda ();
    
    void deleteOnset ();
    void deletePeak ();
    void deleteCoda ();
    
    void startReanalyze ();

  private:
    void displayCurrentSequence ();
    void clearCurrentSequence ();
    
    CDICDatabase db;
    
    QList<QStringList> currentSequence;
    int currentInd;
    
    QCheckBox *onsetRequiredBox;
    QLineEdit *ignoreEdit;
    QCheckBox *usePhonotacticsBox;
    
    QPushButton *deleteOnsetButton;
    QPushButton *deletePeakButton;
    QPushButton *deleteCodaButton;
    QStringListModel *onsetModel;
    QStringListModel *peakModel;
    QStringListModel *codaModel;
    QListView *onsetView;
    QListView *peakView;
    QListView *codaView;
    QPushButton *addOnsetButton;
    QPushButton *addPeakButton;
    QPushButton *addCodaButton;
    
    QLabel *displayLabel;
    QPushButton *backButton;
    QPushButton *forwardButton;
    QPushButton *addClassButton;
    QComboBox *addClassBox;
    QPushButton *removeClassButton;
    QComboBox *removeClassBox;
    
    QPushButton *reanalyzeButton;
    QProgressBar *progressBar;
    
    QHBoxLayout *topLayout;
    
    QHBoxLayout *onsetTopLayout;
    QHBoxLayout *peakTopLayout;
    QHBoxLayout *codaTopLayout;
    QVBoxLayout *onsetLayout;
    QVBoxLayout *peakLayout;
    QVBoxLayout *codaLayout;
    QHBoxLayout *centralLayout;
    
    QHBoxLayout *arrowLayout;
    QHBoxLayout *addRemoveClassLayout;
    
    QVBoxLayout *mainLayout;
};

#endif