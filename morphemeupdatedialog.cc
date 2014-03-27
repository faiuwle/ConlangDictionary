#include <QPushButton>
#include <QListView>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "morphemeupdatedialog.h"
#include "cdicdatabase.h"

MorphemeUpdateDialog::MorphemeUpdateDialog (QStringList words, QList<int> wordIDs)  {
  wordList = words;
  wordIDList = wordIDs;
  
  morphemeView = new QListView;
  QLabel *morphemeLabel = new QLabel ("<b>Morphemes</b>");
  morphemeLabel->setAlignment (Qt::AlignCenter);
  morphemeLayout = new QVBoxLayout;
  morphemeLayout->addWidget (morphemeLabel);
  morphemeLayout->setAlignment (morphemeLabel, Qt::AlignCenter);
  morphemeLayout->addWidget (morphemeView);
  
  wordView = new QListView;
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
  mainLayout->addLayout (viewLayout);
  mainLayout->addWidget (submitButton);
  mainLayout->setAlignment (submitButton, Qt::AlignCenter);
  
  setLayout (mainLayout);
  
  connect (submitButton, SIGNAL (clicked ()), this, SLOT (close ()));
}