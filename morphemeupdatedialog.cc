#include <QPushButton>
#include <QListView>
#include <QLabel>
#include <QStringListModel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "morphemeupdatedialog.h"
#include "cdicdatabase.h"

MorphemeUpdateDialog::MorphemeUpdateDialog (QMap<int, QString> map)  {
  QMap<int, QString>::const_iterator i;
  for (i = map.constBegin (); i != map.constEnd (); i++)  {
    int index = 0;
    for (; index < wordList.size (); index++)
      if (i.value ().compare (wordList[index]) <= 0)
        break;
      
    wordList.insert (index, i.value ());
    wordIDList.insert (index, i.key ());
  }
  
  QLabel *topLabel = new QLabel ("This update introduces morphemes to the program.  Please " +
                                 (QString)"select which of your words are actually morphemes.");
  topLabel->setAlignment (Qt::AlignCenter);
  
  morphemeView = new QListView;
  morphemeModel = new QStringListModel (this);
  morphemeView->setModel (morphemeModel);
  morphemeView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  morphemeView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  QLabel *morphemeLabel = new QLabel ("<b>Morphemes</b>");
  morphemeLabel->setAlignment (Qt::AlignCenter);
  morphemeLayout = new QVBoxLayout;
  morphemeLayout->addWidget (morphemeLabel);
  morphemeLayout->setAlignment (morphemeLabel, Qt::AlignCenter);
  morphemeLayout->addWidget (morphemeView);
  
  wordView = new QListView;
  wordModel = new QStringListModel (wordList, this);
  wordView->setModel (wordModel);
  wordView->setEditTriggers (QAbstractItemView::NoEditTriggers);
  wordView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  QLabel *wordLabel = new QLabel ("<b>Words</b>");
  wordLabel->setAlignment (Qt::AlignCenter);
  wordLayout = new QVBoxLayout;
  wordLayout->addWidget (wordLabel);
  wordLayout->setAlignment (wordLabel, Qt::AlignCenter);
  wordLayout->addWidget (wordView);
  
  leftButton = new QPushButton ("<<");
  rightButton = new QPushButton (">>");
  moveLayout = new QVBoxLayout;
  moveLayout->addStretch (1);
  moveLayout->addWidget (leftButton);
  moveLayout->addWidget (rightButton);
  moveLayout->addStretch (1);
  
  viewLayout = new QHBoxLayout;
  viewLayout->addLayout (morphemeLayout);
  viewLayout->addLayout (moveLayout);
  viewLayout->addLayout (wordLayout);
  
  submitButton = new QPushButton ("Submit");
  submitButton->setMaximumSize (submitButton->sizeHint ());
  
  mainLayout = new QVBoxLayout;
  mainLayout->addWidget (topLabel);
  mainLayout->setAlignment (topLabel, Qt::AlignCenter);
  mainLayout->addLayout (viewLayout);
  mainLayout->addWidget (submitButton);
  mainLayout->setAlignment (submitButton, Qt::AlignCenter);
  
  setLayout (mainLayout);
  
  updateModels ();
  
  connect (leftButton, SIGNAL (clicked ()), this, SLOT (left ()));
  connect (rightButton, SIGNAL (clicked ()), this, SLOT (right ()));
  connect (submitButton, SIGNAL (clicked ()), this, SLOT (submit ()));
}

void MorphemeUpdateDialog::updateModels ()  {
  morphemeModel->setStringList (morphemeList);
  wordModel->setStringList (wordList);
}

void MorphemeUpdateDialog::left ()  {
  QModelIndexList indexList = wordView->selectionModel ()->selectedRows ();
  
  QList<int> rowlist;
  for (int x = 0; x < indexList.size (); x++)
    rowlist.append (indexList[x].row ());
  
  qSort (rowlist.begin (), rowlist.end (), qGreater<int>());
  
  for (int x = 0; x < rowlist.size (); x++)  {
    QString word = wordList.takeAt (rowlist[x]);
    int id = wordIDList.takeAt (rowlist[x]);
    
    morphemeList.append (word);
    morphemeIDList.append (id);
  }
  
  updateModels ();
}

void MorphemeUpdateDialog::right ()  {
  QModelIndexList indexList = morphemeView->selectionModel ()->selectedRows ();
  
  QList<int> rowlist;
  for (int x = 0; x < indexList.size (); x++)
    rowlist.append (indexList[x].row ());
  
  qSort (rowlist.begin (), rowlist.end (), qGreater<int>());
  
  for (int x = 0; x < rowlist.size (); x++)  {
    QString word = morphemeList.takeAt (rowlist[x]);
    int id = morphemeIDList.takeAt (rowlist[x]);
    
    wordList.append (word);
    wordIDList.append (id);
  }
  
  updateModels ();
}

void MorphemeUpdateDialog::submit ()  {
  emit done (morphemeIDList);
  close ();
}