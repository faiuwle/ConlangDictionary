#ifndef MORPHEMEUPDATEDIALOG_H
#define MORPHEMEUPDATEDIALOG_H

#include <QDialog>
#include <QList>
#include <QMap>

class QPushButton;
class QListView;
class QHBoxLayout;
class QVBoxLayout;
class CDICDatabase;
class QStringList;
class QStringListModel;

class MorphemeUpdateDialog : public QDialog  {
  Q_OBJECT
  
  public:
    MorphemeUpdateDialog (QMap<int, QString>);
    
    void updateModels ();
    
  signals:
    void done (QList<int>);
    
  private slots:
    void left ();
    void right ();
    
    void submit ();
    
  private:
    QStringList wordList;
    QStringList morphemeList;
    QList<int> wordIDList;
    QList<int> morphemeIDList;
    
    QListView *morphemeView;
    QStringListModel *morphemeModel;
    QListView *wordView;
    QStringListModel *wordModel;
    
    QPushButton *rightButton;
    QPushButton *leftButton;
    
    QPushButton *submitButton;
    
    QVBoxLayout *morphemeLayout;
    QVBoxLayout *wordLayout;
    QVBoxLayout *moveLayout;
    QHBoxLayout *viewLayout;
    QVBoxLayout *mainLayout;
};

#endif