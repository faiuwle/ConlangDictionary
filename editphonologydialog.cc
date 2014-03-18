#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QComboBox>

#include <QSignalMapper>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "editphonologydialog.h"

EditPhonologyDialog::EditPhonologyDialog (CDICDatabase data, int w)  {
  db = data;
  wordID = w;
  phonology = db.getPhonology (wordID);
  supras = db.getSupras ();
  currentSyllable = phonology.size () - 1;
  
  QStringList phonNames = db.getAllPhonemeNames ();
  
  signalMapper = new QSignalMapper (this);
  phonemeLayout = new QGridLayout;
  
  int numCols = 20;
  if (phonNames.size () < 400) numCols -= 5;
  if (phonNames.size () < 225) numCols -= 5;
  if (phonNames.size () < 100) numCols -= 5;
  if (phonNames.size () < 25) numCols -= 1;
  if (phonNames.size () < 16) numCols -= 1;
  if (phonNames.size () < 9) numCols -= 1;

  for (int x = 0; x < phonNames.size (); x++)  {
    QPushButton *button = new QPushButton (phonNames[x]);
    connect (button, SIGNAL (clicked ()), signalMapper, SLOT (map ()));
    signalMapper->setMapping (button, phonNames[x]);
    phonemeLayout->addWidget (button, x / numCols, x % numCols);
  }
  
  dotButton = new QPushButton ("New Syllable");
  dotButton->setMaximumSize (dotButton->sizeHint ());
  backButton = new QPushButton ("Back");
  backButton->setMaximumSize (backButton->sizeHint ());
  clearButton = new QPushButton ("Clear");
  clearButton->setMaximumSize (clearButton->sizeHint ());
  
  dotBackClearLayout = new QHBoxLayout;
  dotBackClearLayout->addStretch (1);
  dotBackClearLayout->addWidget (dotButton);
  dotBackClearLayout->addWidget (backButton);
  dotBackClearLayout->addWidget (clearButton);
  dotBackClearLayout->addStretch (1);
  
  prevSyllableButton = new QPushButton ("Previous Syllable");
  prevSyllableButton->setMaximumSize (prevSyllableButton->sizeHint ());
  nextSyllableButton = new QPushButton ("Next Syllable");
  nextSyllableButton->setMaximumSize (nextSyllableButton->sizeHint ());
  nextSyllableButton->setEnabled (false);
  
  syllableNavigationLayout = new QHBoxLayout;
  syllableNavigationLayout->addStretch (1);
  syllableNavigationLayout->addWidget (prevSyllableButton);
  syllableNavigationLayout->addWidget (nextSyllableButton);
  syllableNavigationLayout->addStretch (1);
  
  prevSegmentButton = new QPushButton ("<");
  prevSegmentButton->setMaximumSize (prevSegmentButton->sizeHint ());
  nextSegmentButton = new QPushButton (">");
  nextSegmentButton->setMaximumSize (nextSegmentButton->sizeHint ());
  if (phonology.size () == 0)  {
    segmentLabel = new QLabel ("Onset");
    prevSegmentButton->setEnabled (false);
  }
  else  {
    segmentLabel = new QLabel ("Coda");
    nextSegmentButton->setEnabled (false);
  }
  
  segmentNavigationLayout = new QHBoxLayout;
  segmentNavigationLayout->addStretch (1);
  segmentNavigationLayout->addWidget (prevSegmentButton);
  segmentNavigationLayout->addWidget (segmentLabel);
  segmentNavigationLayout->addWidget (nextSegmentButton);
  segmentNavigationLayout->addStretch (1);
  
  sequenceLabel = new QLabel (getRepresentation ());
  sequenceLabel->setAlignment (Qt::AlignCenter);
  
  applySupraButton = new QPushButton ("Apply Suprasegmental:");
  applySupraBox = new QComboBox;
  
  for (int x = 0; x < supras.size (); x++)
    applySupraBox->addItem (supras[x].name);
  
  applySupraLayout = new QHBoxLayout;
  applySupraLayout->addStretch (1);
  applySupraLayout->addWidget (applySupraButton);
  applySupraLayout->addWidget (applySupraBox);
  applySupraLayout->addStretch (1);
  
  submitButton = new QPushButton ("Submit");
  
  mainLayout = new QVBoxLayout;
  mainLayout->addLayout (phonemeLayout);
  mainLayout->addLayout (dotBackClearLayout);
  mainLayout->addLayout (syllableNavigationLayout);
  mainLayout->addLayout (segmentNavigationLayout);
  mainLayout->addWidget (sequenceLabel);
  mainLayout->addLayout (applySupraLayout);
  mainLayout->addStretch (1);
  mainLayout->addWidget (submitButton);
  mainLayout->setAlignment (submitButton, Qt::AlignCenter);
  
  setLayout (mainLayout);
  
  connect (signalMapper, SIGNAL (mapped (const QString&)), this,
           SLOT (addPhoneme (const QString&)));
  
  connect (prevSyllableButton, SIGNAL (clicked ()), this, SLOT (prevSyllable ()));
  connect (nextSyllableButton, SIGNAL (clicked ()), this, SLOT (nextSyllable ()));
  
  connect (dotButton, SIGNAL (clicked ()), this, SLOT (newSyllable ()));
  connect (backButton, SIGNAL (clicked ()), this, SLOT (goBack ()));
  connect (clearButton, SIGNAL (clicked ()), this, SLOT (clear ()));
  
  connect (nextSegmentButton, SIGNAL (clicked ()), this, SLOT (nextSegment ()));
  connect (prevSegmentButton, SIGNAL (clicked ()), this, SLOT (prevSegment ()));
  
  connect (applySupraButton, SIGNAL (clicked ()), this, SLOT (applySupra ()));
  
  connect (submitButton, SIGNAL (clicked ()), this, SLOT (submit ()));
}

void EditPhonologyDialog::addPhoneme (const QString &p)  {
  Phoneme phon;
  phon.name = p;
  
  if (phonology.size () == 0)  {
    Syllable s;
    phonology.append (s);
    currentSyllable = 0;
  }
  
  if (segmentLabel->text () == "Onset")
    phonology[currentSyllable].onset.append (phon);
  if (segmentLabel->text () == "Peak")
    phonology[currentSyllable].peak.append (phon);
  if (segmentLabel->text () == "Coda")
    phonology[currentSyllable].coda.append (phon);
  
  sequenceLabel->setText (getRepresentation ());
}

void EditPhonologyDialog::nextSyllable ()  {
  if (currentSyllable < (phonology.size () - 1))
    currentSyllable++;
  if (currentSyllable == phonology.size () - 1)
    nextSyllableButton->setEnabled (false);
  if (currentSyllable > 0)
    prevSyllableButton->setEnabled (true);
  
  sequenceLabel->setText (getRepresentation ());
}

void EditPhonologyDialog::prevSyllable ()  {
  if (currentSyllable > 0)
    currentSyllable--;
  if (currentSyllable == 0)
    prevSyllableButton->setEnabled (false);
  if (currentSyllable < (phonology.size () - 1))
    nextSyllableButton->setEnabled (true);
  
  sequenceLabel->setText (getRepresentation ());
}
    
void EditPhonologyDialog::newSyllable ()  {
  Syllable s;
  phonology.append (s);
  
  if (currentSyllable < 0)
    currentSyllable = 0;
  
  if (currentSyllable < (phonology.size () - 1))
    nextSyllableButton->setEnabled (true);
  
  sequenceLabel->setText (getRepresentation ());
}
    
void EditPhonologyDialog::goBack ()  {
  if (phonology.size () == 0)
    return;
  
  if (phonology[currentSyllable].coda.size () > 0 && 
      segmentLabel->text () == "Coda")
    phonology[currentSyllable].coda.removeLast ();
  else if (phonology[currentSyllable].peak.size () > 0 &&
           (segmentLabel->text () == "Peak" ||
            segmentLabel->text () == "Coda"))
    phonology[currentSyllable].peak.removeLast ();
  else if (phonology[currentSyllable].onset.size () > 0)
    phonology[currentSyllable].onset.removeLast ();
  else phonology.removeAt (currentSyllable);
  
  if (currentSyllable == phonology.size ())
    currentSyllable = phonology.size () - 1;
  if (currentSyllable == (phonology.size () - 1))
    nextSyllableButton->setEnabled (false);
  if (currentSyllable == 0)
    prevSyllableButton->setEnabled (false);
  
  sequenceLabel->setText (getRepresentation ());
}
    
void EditPhonologyDialog::clear ()  {
  phonology = QList<Syllable> ();
  sequenceLabel->setText ("");
}
    
void EditPhonologyDialog::nextSegment ()  {
  if (segmentLabel->text () == "Onset")  {
    segmentLabel->setText ("Peak");
    prevSegmentButton->setEnabled (true);
  }
    
  else if (segmentLabel->text () == "Peak")  {
    segmentLabel->setText ("Coda");
    nextSegmentButton->setEnabled (false);
  }
}
    
void EditPhonologyDialog::prevSegment ()  {
  if (segmentLabel->text () == "Peak")  {
    segmentLabel->setText ("Onset");
    prevSegmentButton->setEnabled (false);
  }
  
  else if (segmentLabel->text () == "Coda")  {
    segmentLabel->setText ("Peak");
    nextSegmentButton->setEnabled (true);
  }
}

void EditPhonologyDialog::applySupra ()  {
  if (phonology.size () == 0)
    return;
  
  int s = applySupraBox->currentIndex ();
  QString n = applySupraBox->currentText ();
  QString seg = segmentLabel->text ();
  
  if (supras[s].domain == SUPRA_DOMAIN_SYLL)
    phonology[currentSyllable].supras.append (n);
  
  else if (phonology[currentSyllable].coda.size () > 0 && seg == "Coda")
    phonology[currentSyllable].coda.last ().supras.append (n);
  
  else if (phonology[currentSyllable].peak.size () > 0 && seg == "Peak")
    phonology[currentSyllable].peak.last ().supras.append (n);
  
  else if (phonology[currentSyllable].onset.size () > 0 && seg == "Onset")
    phonology[currentSyllable].onset.last ().supras.append (n);
  
  sequenceLabel->setText (getRepresentation ());
}

void EditPhonologyDialog::submit ()  {
  db.setPhonology (wordID, phonology);
  emit done ();
  close ();
}

QString EditPhonologyDialog::getRepresentation ()  {
  QString text = "";
  
  for (int x = 0; x < phonology.size (); x++)  {
    if (currentSyllable == x)
      text += "<b>";
    
    QList<Suprasegmental> syllSupraList;
    
    for (int s = 0; s < supras.size (); s++)
      if (phonology[x].supras.contains (supras[s].name))
        syllSupraList.append (supras[s]);
    
    // syllable before supras
    for (int s = 0; s < syllSupraList.size (); s++)
      if (syllSupraList[s].type == TYPE_BEFORE)
        text += syllSupraList[s].text;
    
    // onset
    for (int o = 0; o < phonology[x].onset.size (); o++)  {
      QList<Suprasegmental> phonSupraList;
      
      for (int s = 0; s < supras.size (); s++)
        if (phonology[x].onset[o].supras.contains (supras[s].name))
          phonSupraList.append (supras[s]);
        
      QString phonText = phonology[x].onset[o].name;
      
      for (int s = 0; s < phonSupraList.size (); s++)  {
        if (phonSupraList[s].type < TYPE_BEFORE)
          phonText = applyDiacritic (phonSupraList[s].type, phonText);
        else if (phonSupraList[s].type == TYPE_DOUBLED)
          phonText = phonText + phonText;
      }
      
      for (int s = 0; s < phonSupraList.size (); s++)  {
        if (phonSupraList[s].type == TYPE_BEFORE)
          phonText.prepend (phonSupraList[s].text);
        else if (phonSupraList[s].type == TYPE_AFTER)
          phonText.append (phonSupraList[s].text);
      }
      
      text += phonText;
    }
    
    text += " ";
    
    // peak
    for (int p = 0; p < phonology[x].peak.size (); p++)  {
      QList<Suprasegmental> phonSupraList;
      
      for (int s = 0; s < supras.size (); s++)
        if (phonology[x].peak[p].supras.contains (supras[s].name))
          phonSupraList.append (supras[s]);
        
      QString phonText = phonology[x].peak[p].name;
      
      for (int s = 0; s < phonSupraList.size (); s++)  {
        if (phonSupraList[s].type < TYPE_BEFORE)
          phonText = applyDiacritic (phonSupraList[s].type, phonText);
        else if (phonSupraList[s].type == TYPE_DOUBLED)
          phonText = phonText + phonText;
      }
      
      for (int s = 0; s < phonSupraList.size (); s++)  {
        if (phonSupraList[s].type == TYPE_BEFORE)
          phonText.prepend (phonSupraList[s].text);
        else if (phonSupraList[s].type == TYPE_AFTER)
          phonText.append (phonSupraList[s].text);
      }
      
      // apply syllable-level peak supras
      for (int s = 0; s < syllSupraList.size (); s++)  {
        if (syllSupraList[s].type < TYPE_BEFORE)
          phonText = applyDiacritic (syllSupraList[s].type, phonText);
        else if (syllSupraList[s].type == TYPE_DOUBLED)
          phonText = phonText + phonText;
      }
      
      text += phonText;
    }
    
    text += " ";
    
    // coda
    for (int c = 0; c < phonology[x].coda.size (); c++)  {
      QList<Suprasegmental> phonSupraList;
      
      for (int s = 0; s < supras.size (); s++)
        if (phonology[x].coda[c].supras.contains (supras[s].name))
          phonSupraList.append (supras[s]);
        
      QString phonText = phonology[x].coda[c].name;
      
      for (int s = 0; s < phonSupraList.size (); s++)  {
        if (phonSupraList[s].type < TYPE_BEFORE)
          phonText = applyDiacritic (phonSupraList[s].type, phonText);
        else if (phonSupraList[s].type == TYPE_DOUBLED)
          phonText = phonText + phonText;
      }
      
      for (int s = 0; s < phonSupraList.size (); s++)  {
        if (phonSupraList[s].type == TYPE_BEFORE)
          phonText.prepend (phonSupraList[s].text);
        else if (phonSupraList[s].type == TYPE_AFTER)
          phonText.append (phonSupraList[s].text);
      }
      
      text += phonText;
    }
    
    // syllable after supras
    for (int s = 0; s < syllSupraList.size (); s++)
      if (syllSupraList[s].type == TYPE_AFTER)
        text += syllSupraList[s].text;
      
    if (currentSyllable == x)
      text += "</b>";
     
    if (x < (phonology.size () - 1))
      text += ".";
  }
  
  return text;
}

QString EditPhonologyDialog::applyDiacritic (int diacritic, QString old)  {
  if (diacritic >= 5 || diacritic < 0) 
    return old;
  
  QString newText = "";
  
  for (int x = 0; x < old.size (); x++)  {
    int index = plainChars.indexOf ((QString)old[x]);
    
    if (index >= 0)
      newText += diacriticChars[diacritic*12 + index];
    
    else newText += old[x];
  }
  
  return newText;
}