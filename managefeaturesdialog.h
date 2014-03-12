#ifndef MANAGEFEATURESDIALOG_H
#define MANAGEFEATURESDIALOG_H

#include <QDialog>
#include <QVariant>

#include "cdicdatabase.h"

class QLabel;
class QString;
class EditableQueryModel;
class QListView;
class QPushButton;
class QLineEdit;
class QComboBox;
class QVBoxLayout;
class QHBoxLayout;

class ManageFeaturesDialog : public QDialog  {
  Q_OBJECT
  
  public:
    ManageFeaturesDialog (int, CDICDatabase);

  private slots:
    void addFeatures ();
    void addSubfeatures ();
    
    void deleteFeatures ();
    
    void displaySubfeatures ();
    void populateParentSubs (QString);
    
    void setParent ();
    void clearParent ();
    void setDisplayType (int);
    
    void replaceFeature (QVariant, QVariant);
    void replaceSubfeature (QVariant, QVariant);

    void updateModels ();

  private:
    int domain;
    CDICDatabase db;

    EditableQueryModel *featureListModel;
    EditableQueryModel *featureBoxModel;
    EditableQueryModel *subfeatureListModel;
    EditableQueryModel *subfeatureBoxModel;

    QPushButton *addFeaturesButton;
    QLineEdit *addFeaturesEdit;
    QComboBox *addFeaturesBox;
    QListView *featureListView;
    QComboBox *featureDisplayBox;

    QPushButton *deleteButton;

    QListView *subfeatureListView;
    QPushButton *addSubfeaturesButton;
    QLineEdit *addSubfeaturesEdit;
    QComboBox *displayTypeBox;

    QLabel *parentLabel;
    QPushButton *parentButton;
    QComboBox *parentFeatureBox;
    QComboBox *parentSubfeatureBox;
    QPushButton *clearParentButton;
    
    QPushButton *submitButton;

    QHBoxLayout *addFeaturesLayout;
    QHBoxLayout *displayFeaturesLayout;
    QHBoxLayout *parentLayout;
    QVBoxLayout *leftLayout;
    QHBoxLayout *addSubfeaturesLayout;
    QHBoxLayout *displayTypeLayout;
    QVBoxLayout *rightLayout;
    QHBoxLayout *centralLayout;
    QVBoxLayout *mainLayout;
};

#endif