#ifndef FEATUREBUNDLESDIALOG_H
#define FEATUREBUNDLESDIALOG_H

#include <QDialog>
#include <QVariant>

#include "cdicdatabase.h"

class QLabel;
class QPushButton;
class QLineEdit;
class QComboBox;
class QListView;
class EditableQueryModel;
class QStringListModel;
class QHBoxLayout;
class QVBoxLayout;

class FeatureBundlesDialog : public QDialog  {
  Q_OBJECT
  
  public:
    FeatureBundlesDialog (int, int, CDICDatabase);
    void initNaturalClassDialog ();
    void initFeatureSetDialog ();
    
  signals:
    void done ();
    
  private slots:
    void addNaturalClass ();
    void displayFeatures ();
    void deleteClass ();
    void renameClass (QVariant, QVariant);
    void refreshClassModel ();
    
    void addFeature ();
    void removeFeature ();
    void clearFeatures ();
    void updateFeatureBox ();
    
  private:
    int domain;
    int id;
    
    CDICDatabase db;
    
    EditableQueryModel *naturalClassModel;
    QStringListModel *featureListModel;
    EditableQueryModel *featureBoxModel;
    EditableQueryModel *subfeatureBoxModel;
    
    QLabel *titleLabel;
    
    QPushButton *addNaturalClassButton;
    QLineEdit *addNaturalClassEdit;
    QListView *naturalClassView;
    QPushButton *deleteButton;
    
    QPushButton *addFeatureButton;
    QComboBox *addFeatureBox;
    QComboBox *addSubfeatureBox;
    QPushButton *removeFeatureButton;
    QComboBox *removeFeatureBox;
    QListView *featureView;
    QPushButton *clearButton;
    
    QPushButton *doneButton;
    
    QHBoxLayout *addNaturalClassLayout;
    QVBoxLayout *leftLayout;
    QHBoxLayout *addFeatureLayout;
    QHBoxLayout *removeFeatureLayout;
    QVBoxLayout *rightLayout;
    QHBoxLayout *centralLayout;
    QVBoxLayout *mainLayout;
};

#endif