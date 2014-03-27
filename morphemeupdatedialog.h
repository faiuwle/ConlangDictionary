#ifndef MORPHEMEUPDATEDIALOG_H
#define MORPHEMEUPDATEDIALOG_H

#include <QDialog>
#include <QList>

class QPushButton;
class QListView;
class QHBoxLayout;
class QVBoxLayout;
class CDICDatabase;
class QStringList;

class MorphemeUpdateDialog : public QDialog  {
  Q_OBJECT
  
  public:
    MorphemeUpdateDialog (QStringList, QList<int>);
    
  private:
    QStringList wordList;
    QStringList morphemeList;
    QList<int> wordIDList;
    QList<int> morphemeIDList;
    
    QListView *morphemeView;
    QListView *wordView;
    
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