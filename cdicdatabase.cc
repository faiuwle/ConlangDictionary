#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QSqlTableModel>

#include <QDomDocument>
#include <QDomElement>

#include <QFile>
#include <QTextStream>

#include <QMessageBox>

#include <iostream>
using namespace std;

#include "cdicdatabase.h"
#include "mainwindow.h"
#include "editablequerymodel.h"

#define QUERY_ERROR(v) QMessageBox::warning (NULL, "Database Error", v.executedQuery () + "\n" + v.lastError ().text ());
#define DATA_ERROR(m) QMessageBox::warning (NULL, "Database Error", m);

CDICDatabase::CDICDatabase ()  {
}

CDICDatabase::CDICDatabase (QString name)  {
  open (name);
}

bool CDICDatabase::open (QString name)  {
  if (db.isOpen ())
    db.close ();
  
  if (db.connectionName ().isEmpty ())
    db = QSqlDatabase::addDatabase ("QSQLITE");
  
  if (QFile::exists (name))  {
    db.setHostName ("localhost");
    db.setDatabaseName (name);
    
    if (!db.open ())
      QMessageBox::warning (NULL, "Database Error", db.lastError ().text ());
  }
  
  else  {
    db.setHostName ("localhost");
    db.setDatabaseName (name);
    
    if (!db.open ())
      QMessageBox::warning (NULL, "Database Error", db.lastError ().text ());
    
    else  {
      db.transaction ();
      
      if (readSQLFile ("schema.sql"))
        db.commit ();
      else db.rollback ();
    }
  }
  
  if (db.isOpen ())  {
    QSqlQuery query (db);
    
    if (!query.exec ("pragma foreign_keys = ON"))
      QUERY_ERROR(query)
      
    query.finish ();
      
    if (!query.exec ("pragma recursive_triggers = ON"))
      QUERY_ERROR(query)
    
    query.finish ();
    
    if (!query.exec ("pragma case_sensitive_like = ON"))
      QUERY_ERROR(query)
      
    query.finish ();
  }
  
  return db.isOpen ();
}

void CDICDatabase::close ()  {
  if (db.isOpen ())
    db.close ();
}

void CDICDatabase::clear ()  {
  if (!db.isOpen ()) return;
  
  db.transaction ();
  
  if (!readSQLFile ("drop.sql"))
    db.rollback ();
  
  else if (!readSQLFile ("schema.sql"))
    db.rollback ();
  
  db.commit ();
}

QString CDICDatabase::currentDB ()  {
  if (db.isOpen ()) return db.databaseName ();
  else return "";
}

bool CDICDatabase::loadFromXML (QString filename)  {
  QDomDocument doc ("ConlangML");
  QFile file (filename);

  if (!file.open (QFile::ReadOnly))  {
    QMessageBox::warning (NULL, "", "Cannot read file.");
    return false;
  }

  if (!doc.setContent (&file))  {
    QMessageBox::warning (NULL, "", "Cannot read XML.");
    file.close ();
    return false;
  }
  
  if (!db.isOpen ())  {
    QMessageBox::warning (NULL, "", "Please open a dictionary to load into first.");
    file.close ();
    return false;
  }

  file.close ();
  
  db.transaction ();

  QString languageName = doc.documentElement ().attribute ("name");
  if (languageName != "")
    setValue (LANGUAGE_NAME, languageName);

  if (!doc.documentElement ().firstChildElement ("squareBrackets").isNull ())
    setValue (SQUARE_BRACKETS, "true");
  else setValue (SQUARE_BRACKETS, "false");

  if (!loadInventory (doc.documentElement ().firstChildElement ("inventory")))
    db.rollback ();

  if (!loadSupras (doc.documentElement ().firstChildElement ("suprasegmentals")))
    db.rollback ();
    
  if (!loadWordList (doc.documentElement ().firstChildElement ("wordList")))
    db.rollback ();

  db.commit ();

  return true;
}

bool CDICDatabase::loadFromText (QString filename, QString pattern)  {
  int wordLoc = pattern.indexOf ("/w");
  int classLoc = pattern.indexOf ("/c");
  int definitionLoc = pattern.indexOf ("/d");
  putInOrder (&wordLoc, &classLoc, &definitionLoc);

  pattern.replace ("(", "\\(");
  pattern.replace (")", "\\)");
  pattern.replace ("/w", "(.+)");
  pattern.replace ("/c", "(.+)");
  pattern.replace ("/d", "(.+)");
  QRegExp regExp (pattern);

  QFile file (filename);
  QTextStream in (&file);
  if (getValue (USE_UNICODE) == "true")
    in.setCodec ("UTF-8");
  file.open (QIODevice::ReadOnly | QIODevice::Text);
  QString line = "";

  while (!in.atEnd ())  {
    line = in.readLine ();

    if (line == "") continue;

    if (!regExp.exactMatch (line)) continue;
    
    int wordID = 0;
    
    QSqlQuery query (db);
    query.prepare ("select max(id) from Word");
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      continue;
    }
    
    if (query.next ())
      if (query.value (0).toInt () >= 1)
        wordID = query.value (0).toInt () + 1;
    query.finish ();
    
    query.prepare ("insert into Word values (NULL, :name, :def)");
    query.bindValue (":name", regExp.cap (wordLoc));
    query.bindValue (":def", regExp.cap (definitionLoc));
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      continue;
    }
    
    query.finish ();
    
    assignNaturalClass (regExp.cap (classLoc), wordID);
  };
  
  return true;
}

bool CDICDatabase::loadLexique (QString filename, QStringList params)  {
  return false;
}

bool CDICDatabase::loadFeatures (int domain, QString filename)  {
  if (!QFile::exists (filename))
    return false;
  
  QSqlDatabase featuresDB = QSqlDatabase::addDatabase ("QSQLITE", "featuresDB");
  featuresDB.setHostName ("localhost");
  featuresDB.setDatabaseName (filename);
  
  if (!featuresDB.open ())  {
    DATA_ERROR(featuresDB.lastError ().text ())
    return false;
  }
  
  if (QMessageBox::warning (NULL, "Load Features", (QString)"This will delete all existing " + 
                             (domain == PHONEME ? "Phoneme" : "Word") + " features.  Continue?",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return false;
  
  
  db.transaction ();
    
  QSqlQuery query (db);
  
  QString tableName = (domain == PHONEME ? "FeatureBundlePhon" : "FeatureBundleWord");
  
  query.prepare ("delete from " + tableName);
    
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    db.rollback ();
    return false;
  }
    
  query.finish ();
    
  tableName = (domain == PHONEME ? "NaturalClassPhon" : "NaturalClassWord");
    
  query.prepare ("delete from " + tableName);
    
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    db.rollback ();
    return false;
  }
    
  query.finish ();
    
  tableName = (domain == PHONEME ? "PhonemeSubfeature" : "WordSubfeature");
    
  query.prepare ("delete from " + tableName);
    
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    db.rollback ();
    return false;
  }
    
  query.finish ();
    
  tableName = (domain == PHONEME ? "PhonemeFeatureDef" : "WordFeatureDef");
    
  query.prepare ("delete from " + tableName);
    
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    db.rollback ();
    return false;
  }
    
  query.finish ();
  
  QSqlQuery fquery (featuresDB);
  fquery.prepare ("select name, parentName, parentValue, displayType from " + 
                  (QString)"FeatureDef");
  
  if (!fquery.exec ())  {
    QUERY_ERROR(fquery)
    fquery.finish ();
    db.rollback ();
    return false;
  }
  
  QMap< QString, QList<QString> > parentValuePairs;
  
  tableName = (domain == PHONEME ? "PhonemeFeatureDef" : "WordFeatureDef");
  
  while (fquery.next ())  {
    query.prepare ("insert into " + tableName + " values (:n, NULL, NULL, :d)");
    query.bindValue (":n", fquery.value (0).toString ());
    query.bindValue (":d", fquery.value (3).toString ());
    
    QList<QString> pv;
    pv << fquery.value (1).toString () << fquery.value (2).toString ();
    parentValuePairs[fquery.value (0).toString ()] = pv;
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      db.rollback ();
      return false;
    }
    
    query.finish ();
  }
  
  fquery.finish ();
  fquery.prepare ("select name, value from Subfeature");
  
  if (!fquery.exec ())  {
    QUERY_ERROR(fquery)
    fquery.finish ();
    db.rollback ();
    return false;
  }
  
  tableName = (domain == PHONEME ? "PhonemeSubfeature" : "WordSubfeature");
  
  while (fquery.next ())  {
    query.prepare ("insert into " + tableName + " values (:n, :v)");
    query.bindValue (":n", fquery.value (0).toString ());
    query.bindValue (":v", fquery.value (1).toString ());
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      db.rollback ();
      return false;
    }
    
    query.finish ();
  }
  
  QMap< QString, QList<QString> >::const_iterator i = parentValuePairs.begin ();
  while (i != parentValuePairs.constEnd ())  {
    tableName = (domain == PHONEME ? "PhonemeFeatureDef" : "WordFeatureDef");
    
    query.prepare ("update " + tableName + 
                   " set parentName = :pn, parentValue = :pv where name == :n");
    query.bindValue (":pn", i.value ()[0]);
    query.bindValue (":pv", i.value ()[1]);
    query.bindValue (":n", i.key ());
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      db.rollback ();
      return false;
    }
    
    ++i;
  }
  
  fquery.finish ();
  fquery.prepare ("select bundleID, name from NaturalClass");
  
  if (!fquery.exec ())  {
    QUERY_ERROR(fquery)
    fquery.finish ();
    db.rollback ();
    return false;
  }
  
  tableName = (domain == PHONEME ? "NaturalClassPhon" : "NaturalClassWord");
  
  while (fquery.next ())  {
    query.prepare ("insert into " + tableName + " values (:id, :n)");
    query.bindValue (":id", fquery.value (0).toInt ());
    query.bindValue (":n", fquery.value (1).toString ());
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      db.rollback ();
      return false;
    }
    
    query.finish ();
  }
  
  fquery.finish ();
  fquery.prepare ("select id, feature, value from FeatureBundle");
  
  if (!fquery.exec ())  {
    QUERY_ERROR(fquery)
    fquery.finish ();
    db.rollback ();
    return false;
  }
  
  tableName = (domain == PHONEME ? "FeatureBundlePhon" : "FeatureBundleWord");
  
  while (fquery.next ())  {
    query.prepare ("insert into " + tableName + " values (:id, :f, :v)");
    query.bindValue (":id", fquery.value (0).toInt ());
    query.bindValue (":f", fquery.value (1).toString ());
    query.bindValue (":v", fquery.value (2).toString ());
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      db.rollback ();
      return false;
    }
    
    query.finish ();
  }
  // copy features.sql tables into main tables
  
  fquery.finish ();
  featuresDB.close ();
  db.commit ();
  
  return true;
}

bool CDICDatabase::saveToText (QString filename, QString pattern)  {
  QSqlQuery query (db);
  query.prepare ("select id, name, definition from Word");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return false;
  }
  
  QFile file (filename);
  QTextStream out (&file);
  file.open (QIODevice::WriteOnly | QIODevice::Text);
  
  while (query.next ())  {
    QString line = pattern;
    line.replace ("/w", query.value (1).toString ());
    line.replace ("/d", query.value (2).toString ());
    line.replace ("/p", getRepresentation (query.value (0).toInt ()));
    
    QSqlQuery classQuery (db);
    classQuery.prepare ("select classlist from WordPageTable where id == :w");
    classQuery.bindValue (":w", query.value (0).toInt ());
    
    if (!classQuery.exec ())  {
      QUERY_ERROR(classQuery)
      classQuery.finish ();
      continue;
    }
    
    if (classQuery.next ())
      line.replace ("/c", classQuery.value (0).toString ());
    
    out << line << endl;
  }
  
  file.close ();
  query.finish ();
  return true;
}

bool CDICDatabase::saveFeatures (int domain, QString filename)  {
  if (QFile::exists (filename))
    QFile::remove (filename);
  
  QSqlDatabase featuresDB = QSqlDatabase::addDatabase ("QSQLITE", "featuresDB");
  featuresDB.setHostName ("localhost");
  featuresDB.setDatabaseName (filename);
  
  if (!featuresDB.open ())  {
    DATA_ERROR(featuresDB.lastError ().text ())
    return false;
  }
  
  featuresDB.transaction ();
  
  QFile file ("features.sql");
  QTextStream in (&file);
  file.open (QIODevice::ReadOnly | QIODevice::Text);
      
  QString line = in.readLine ();
      
  while (!in.atEnd ())  {
    QString queryText = "";
        
    while (line.trimmed () == "" || line.startsWith ("--"))
      line = in.readLine ();
        
    while (line.trimmed () != "" && !in.atEnd ())  {
      queryText += line;
      line = in.readLine ();
    };
        
    if (line != "")
      queryText += line;
        
    queryText.chop (1);
        
    QSqlQuery query (featuresDB);
        
    if (!query.exec (queryText))  {
      QUERY_ERROR(query);
      query.finish ();
      featuresDB.rollback ();
      file.close ();
      return false;
    }
        
    query.finish ();
    queryText = "";
  };
      
  file.close ();
  
  QString tableName = (domain == PHONEME ? "PhonemeFeatureDef" : "WordFeatureDef");
  
  QSqlQuery query (db);
  query.prepare ("select name, parentName, parentValue, displayType from " + 
                 tableName);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    featuresDB.rollback ();
    return false;
  }
  
  while (query.next ())  {
    QSqlQuery fquery (featuresDB);
    fquery.prepare ("insert into FeatureDef values (:n, :pn, :pv, :d)");
    fquery.bindValue (":n", query.value (0).toString ());
    fquery.bindValue (":pn", query.value (1).toString ());
    fquery.bindValue (":pv", query.value (2).toString ());
    fquery.bindValue (":d", query.value (3).toString ());
    
    if (!fquery.exec ())  {
      QUERY_ERROR(fquery)
      fquery.finish ();
      featuresDB.rollback ();
      return false;
    }
    
    fquery.finish ();
  }
  
  query.finish ();
  
  tableName = (domain == PHONEME ? "PhonemeSubfeature" : "WordSubfeature");
  
  query.prepare ("select name, value from " + tableName);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    featuresDB.rollback ();
    return false;
  }
  
  while (query.next ())  {
    QSqlQuery fquery (featuresDB);
    fquery.prepare ("insert into Subfeature values (:n, :v)");
    fquery.bindValue (":n", query.value (0).toString ());
    fquery.bindValue (":v", query.value (1).toString ());
    
    if (!fquery.exec ())  {
      QUERY_ERROR(fquery)
      fquery.finish ();
      featuresDB.rollback ();
      return false;
    }
  }
  
  query.finish ();
  
  tableName = (domain == PHONEME ? "NaturalClassPhon" : "NaturalClassWord");
  
  query.prepare ("select bundleID, name from " + tableName);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    featuresDB.rollback ();
    return false;
  }
  
  while (query.next ())  {
    QSqlQuery fquery (featuresDB);
    fquery.prepare ("insert into NaturalClass values (:id, :n)");
    fquery.bindValue (":id", query.value (0).toInt ());
    fquery.bindValue (":n", query.value (1).toString ());
    
    if (!fquery.exec ())  {
      QUERY_ERROR(fquery)
      fquery.finish ();
      featuresDB.rollback ();
      return false;
    }
  }
  
  query.finish ();
  
  tableName = (domain == PHONEME ? "FeatureBundlePhon" : "FeatureBundleWord");
  
  query.prepare ("select id, feature, value from " + tableName);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    featuresDB.rollback ();
    return false;
  }
  
  while (query.next ())  {
    QSqlQuery fquery (featuresDB);
    fquery.prepare ("insert into FeatureBundle values (:id, :f, :v)");
    fquery.bindValue (":id", query.value (0).toInt ());
    fquery.bindValue (":f", query.value (1).toString ());
    fquery.bindValue (":v", query.value (2).toString ());
    
    if (!fquery.exec ())  {
      QUERY_ERROR(fquery)
      fquery.finish ();
      featuresDB.rollback ();
      return false;
    }
  }
  
  query.finish ();
  featuresDB.commit ();
  featuresDB.close ();
  
  return true;
}

bool CDICDatabase::settingDefined (QString setting)  {
  if (!db.isOpen ()) return false;
  
  QSqlQuery query (db);
  query.prepare ("select value from Settings where name == :setting");
  query.bindValue (":setting", setting);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return false;
  }
  
  if (query.next ())  {
    query.finish ();
    return true;
  }
  
  else  {
    query.finish ();
    return false;
  }
}

QString CDICDatabase::getValue (QString setting)  {
  if (!db.isOpen ()) return "";
  
  QString value = "";
  QSqlQuery query (db);
  query.prepare ("select value from Settings where name == :setting");
  query.bindValue (":setting", setting);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return "";
  }
  
  if (query.next ())
    value = query.value (0).toString ();
  
  query.finish ();
  return value;
}

void CDICDatabase::setValue (QString setting, QString value)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  
  if (settingDefined (setting))  {
    query.prepare ("update Settings set value = :value where name == :setting");
    query.bindValue (":value", value);
    query.bindValue (":setting", setting);
    
    if (!query.exec ())
      QUERY_ERROR(query)
  }
  
  else  {
    query.prepare ("insert into Settings values (:setting, :value)");
    query.bindValue (":setting", setting);
    query.bindValue (":value", value);
    
    if (!query.exec ())
      QUERY_ERROR(query)
  }
  
  query.finish ();
}

QSqlQueryModel *CDICDatabase::getPhonemeListModel ()  {
  if (!db.isOpen ()) return NULL;
  
  QSqlQueryModel *model = new QSqlQueryModel (NULL);
  model->setQuery ("select name from Phoneme order by alpha", db);
  
  return model;
}

QSqlTableModel *CDICDatabase::getPhonemeDisplayModel ()  {
  if (!db.isOpen ()) return NULL;
  
  QSqlTableModel *model = new QSqlTableModel (NULL, db);
  model->setTable ("Phoneme");
  model->setSort (1, Qt::AscendingOrder);
  model->setEditStrategy (QSqlTableModel::OnManualSubmit);
  model->select ();
  
  return model;
}

void CDICDatabase::addPhoneme (QString name, QString notes)  {
  if (!db.isOpen ()) return;

  QSqlQuery query (db);
  
  int nextAlpha = 0;

  if (!query.exec ("select count(alpha) from Phoneme"))  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (query.next ())
    nextAlpha = query.value (0).toInt ();
  
  query.finish ();
  
  QTextStream terminal (stdout);
  
  terminal << "Adding phoneme: " << nextAlpha << ", " << name << ", " << notes << endl;
  
  query.prepare ("insert into Phoneme values (null, :alpha, :name, :notes)");
  query.bindValue (":alpha", nextAlpha);
  query.bindValue (":name", name);
  query.bindValue (":notes", notes);
  
  if (!query.exec ())
    QUERY_ERROR(query)
  
  query.finish ();
  return;
}

void CDICDatabase::deletePhoneme (QString name)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  query.prepare ("delete from Phoneme where name == :name");
  query.bindValue (":name", name);
  
  if (!query.exec ())
    QUERY_ERROR(query)
  
  query.finish ();
}

void CDICDatabase::movePhonemeUp (int alpha)  {
  if (!db.isOpen ()) return;
  if (alpha < 1) return;
  
  QSqlQuery query (db);
  query.prepare ("update Phoneme set alpha = :new where alpha == :old");
  query.bindValue (":new", alpha-1);
  query.bindValue (":old", alpha);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}

void CDICDatabase::movePhonemeDown (int alpha)  {
  movePhonemeUp (alpha+1);
}

void CDICDatabase::setSpellings (QString phon, QString s)  {
  if (!db.isOpen ()) return;
  
  QStringList spellings = s.split (' ', QString::SkipEmptyParts);
  int phonID = -1;
  
  QSqlQuery query (db);
  query.prepare ("select id from Phoneme where name == :name");
  query.bindValue (":name", phon);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (!query.next ())  {
    DATA_ERROR("No such phoneme: " + phon)
    query.finish ();
    return;
  }
  
  else phonID = query.value (0).toInt ();
  
  query.finish ();
  
  db.transaction ();
  
  for (int x = 0; x < spellings.size (); x++)  {
    query.prepare ("insert into PhonemeSpelling values (:id, :spell)");
    query.bindValue (":id", phonID);
    query.bindValue (":spell", spellings[x]);
      
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      db.rollback ();
      return;
    }
    
    query.finish ();
  }
  
  QStringList placeholders;
  for (int x = 0; x < spellings.size (); x++)
    placeholders.append ("?");
  
  query.prepare ((QString)"delete from PhonemeSpelling " +
                 (QString)"where phonemeID = ? and spelling not in (" +
                 placeholders.join (", ") + ")");
  query.bindValue (0, phonID);
  for (int x = 0; x < spellings.size (); x++)
    query.bindValue (x+1, spellings[x]);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    db.rollback ();
    return;
  }
  
  query.finish ();
  db.commit ();
}

void CDICDatabase::assignNaturalClass (QString className, QString phonName)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  query.prepare ((QString)"insert into PhonemeFeatureSet " + 
                          "select Phoneme.id, feature, value " +
                          "from Phoneme, NaturalClassPhon, FeatureBundlePhon " +
                          "where Phoneme.name == :pn and " + 
                                "NaturalClassPhon.name == :cn and " +
                                "FeatureBundlePhon.id == bundleID");
  query.bindValue (":pn", phonName);
  query.bindValue (":cn", className);
  
  if (!query.exec ())
    QUERY_ERROR(query)

  query.finish ();
}

QString CDICDatabase::getPhonemeName (int id)  {
  if (!db.isOpen ()) return "";
  
  QSqlQuery query (db);
  query.prepare ("select name from Phoneme where id == :id");
  query.bindValue (":id", id);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return "";
  }
  
  QString name = "";
  
  if (query.next ())
    name = query.value (0).toString ();
  
  query.finish ();
  return name;
}

QString CDICDatabase::getSpellingText (QString phon)  {
  if (!db.isOpen ()) return "";
  
  int phonID = -1;
  
  QSqlQuery query (db);
  query.prepare ("select id from Phoneme where name == :name");
  query.bindValue (":name", phon);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return "";
  }
  
  if (!query.next ())  {
    DATA_ERROR("No such phoneme: " + phon)
    query.finish ();
    return "";
  }
  
  phonID = query.value (0).toInt ();
  query.finish ();
  
  query.prepare ("select spelling from PhonemeSpelling where phonemeID == :id");
  query.bindValue (":id", phonID);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return "";
  }
  
  QString text = "";
  
  while (query.next ())
    text += query.value (0).toString () + " ";
  
  if (text != "") text.chop (1);
  
  query.finish ();
  return text;
}

QStringList CDICDatabase::getAllPhonemeNames ()  {
  QStringList list;
  
  if (!db.isOpen ()) return list;
  
  QSqlQuery query (db);
  
  if (!query.exec ("select name from Phoneme"))  {
    QUERY_ERROR(query)
    query.finish ();
    return list;
  }
  
  while (query.next ())
    list.append (query.value (0).toString ());
  
  return list;
}

QSqlQueryModel *CDICDatabase::getSupraListModel ()  {
  if (!db.isOpen ()) return NULL;
  
  QSqlQueryModel *model = new QSqlQueryModel (NULL);
  model->setQuery ("select name from Suprasegmental order by id", db);
  
  return model;
}
    
QSqlTableModel *CDICDatabase::getSupraDisplayModel ()  {
  if (!db.isOpen ()) return NULL;
  
  QSqlTableModel *model = new QSqlTableModel (NULL, db);
  model->setTable ("Suprasegmental");
  model->setSort (0, Qt::AscendingOrder);
  model->setEditStrategy (QSqlTableModel::OnManualSubmit);
  model->select ();
  
  return model;
}
    
void CDICDatabase::addSupra (QString name)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  query.prepare ("insert into Suprasegmental(name) values (:name)");
  query.bindValue (":name", name);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
void CDICDatabase::deleteSupra (QString name)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  query.prepare ("delete from Suprasegmental where name == :name");
  query.bindValue (":name", name);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
void CDICDatabase::setSupraApplies (QString name, QStringList phonList)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  
  QStringList placeholders;
  for (int x = 0; x < phonList.size (); x++)
    placeholders.append ("?");
  
  query.prepare ("select id from Suprasegmental where name == :name");
  query.bindValue (":name", name);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (!query.next ())  {
    DATA_ERROR("No such suprasegmental: " + name)
    query.finish ();
    return;
  }
  
  int supraID = query.value (0).toInt ();
  query.finish ();
  
  QList<int> phonID;
  query.prepare ("select id from Phoneme where name in (" + placeholders.join (", ") + ")");
  for (int x = 0; x < phonList.size (); x++)
    query.bindValue (x, phonList[x]);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  while (query.next ())
    phonID.append (query.value (0).toInt ());
  
  db.transaction ();
  
  query.prepare ((QString)"insert into SupraApplies " +
                          "select ?, id from Phoneme where name in " +
                          "(" + placeholders.join (", ") + ")");
  query.bindValue (0, supraID);
  for (int x = 0; x < phonList.size (); x++)
    query.bindValue (x+1, phonList[x]);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    db.rollback ();
    return;
  }
  
  while (phonID.size () < placeholders.size ())
    placeholders.removeLast ();
  
  query.prepare ((QString)"delete from SupraApplies where supraID == ? and " +
                          "phonemeID not in (" + placeholders.join (", ") + ")");
  query.bindValue (0, supraID);
  for (int x = 0; x < phonID.size (); x++)
    query.bindValue (x+1, phonID[x]);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    db.rollback ();
    return;
  }
  
  query.finish ();
  db.commit ();
}

QStringList CDICDatabase::getSupraApplies (QString name)  {
  QStringList applies;
  
  if (!db.isOpen ()) return applies;
  
  QSqlQuery query (db);
  query.prepare ((QString)"select Phoneme.name from Phoneme, SupraApplies, Suprasegmental " +
                 "where Suprasegmental.name == :n and supraID == Suprasegmental.id " +
                       "and phonemeID == Phoneme.id");
  query.bindValue (":n", name);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return applies;
  }
  
  while (query.next ())
    applies.append (query.value (0).toString ());
  
  query.finish ();
  
  return applies;
}

QList<Suprasegmental> CDICDatabase::getSupras ()  {
  if (!db.isOpen ()) return QList<Suprasegmental> ();
  
  QList<Suprasegmental> supraList;
  
  QSqlQuery query (db);
  query.prepare ("select id, name, domain, repType, repText from Suprasegmental");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return supraList;
  }
  
  while (query.next ())  {
    Suprasegmental s;
    s.name = query.value (1).toString ();
    s.domain = query.value (2).toInt ();
    s.type = query.value (3).toInt ();
    s.text = query.value (4).toString ();
    
    QSqlQuery appliesQuery (db);
    appliesQuery.prepare ((QString)"select name from Phoneme, SupraApplies " +
                          "where id == phonemeID and supraID == :sid");
    appliesQuery.bindValue (":sid", query.value (0).toInt ());
    
    if (!appliesQuery.exec ())  {
      QUERY_ERROR(appliesQuery)
      appliesQuery.finish ();
      supraList.append (s);
      continue;
    }
    
    QStringList appliesList;
    
    while (appliesQuery.next ())
      appliesList.append (appliesQuery.value (0).toString ());
    
    appliesQuery.finish ();
    
    s.applicablePhonemes = appliesList;
    
    supraList.append (s);
  }
  
  return supraList;
}

QStringList CDICDatabase::getSequenceList (int loc)  {
  if (!db.isOpen ()) return QStringList ();
  
  QStringList list;
  
  QString tableName;
  
  if (loc == ONSET)
    tableName = "LegalOnset";
  else if (loc == PEAK)
    tableName = "LegalPeak";
  else tableName = "LegalCoda";
  
  QSqlQuery query (db);
  query.prepare ("select id, ind, name from " + tableName + ", NaturalClassPhon " +
                 "where class == bundleID");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return list;
  }
  
  QList< QList<QStringList> > data;
  
  while (query.next ())  {
    int id = query.value(0).toInt ();
    int ind = query.value(1).toInt ();
    QString className = query.value(2).toString ();
    
    while (data.size () <= id)
      data.append (QList<QStringList> ());
    
    while (data[id].size () <= ind)
      data[id].append (QStringList ());
    
    data[id][ind].append (className);
  }
  
  for (int x = 1; x < data.size (); x++)  {
    QString text = "";
    
    for (int i = 0; i < data[x].size (); i++)  {
      for (int c = 0; c < data[x][i].size (); c++)  {
        text += data[x][i][c];
        
        if (c != (data[x][i].size () - 1))
          text += " / ";
      }
      
      if (i != (data[x].size () - 1))
        text += " + ";
    }
    
    list.append (text);
    text = "";
  }
  
  return list;
}

void CDICDatabase::addSequence (int loc, QList<QStringList> sequence)  {
  if (!db.isOpen ()) return;
  
  QString tableName;
  
  if (loc == ONSET)
    tableName = "LegalOnset";
  else if (loc == PEAK)
    tableName = "LegalPeak";
  else tableName = "LegalCoda";
  
  QSqlQuery query (db);
  query.prepare ("select max(id) from " + tableName);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  int nextID = 1;
  
  if (query.next ())
    if (query.value(0).toInt () >= 1)
      nextID = query.value(0).toInt () + 1;
  
  query.finish ();
  
  for (int ind = 0; ind < sequence.size (); ind++)  {
    for (int c = 0; c < sequence[ind].size (); c++)  {
      int classID = 0;
      
      query.prepare ("select bundleID from NaturalClassPhon where name == :name");
      query.bindValue (":name", sequence[ind][c]);
      
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        continue;
      }
      
      if (!query.next ())  {
        query.finish ();
        continue;
      }
      
      classID = query.value(0).toInt ();
      
      query.finish ();
      
      query.prepare ("insert into " + tableName + " values (:id, :ind, :class)");
      query.bindValue (":id", nextID);
      query.bindValue (":ind", ind);
      query.bindValue (":class", classID);
      
      if (!query.exec ())
        QUERY_ERROR(query)
        
      query.finish ();
    }
  }
}

void CDICDatabase::removeSequence (int loc, int id)  {
  if (!db.isOpen ()) return;
  
  QString tableName;
  
  if (loc == ONSET)
    tableName = "LegalOnset";
  else if (loc == PEAK)
    tableName = "LegalPeak";
  else tableName = "LegalCoda";
  
  QSqlQuery query (db);
  query.prepare ("delete from " + tableName + " where id == :id");
  query.bindValue (":id", id);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
    
  query.finish ();
  
  query.prepare ("update " + tableName + " set id = id - 1 where id > :id");
  query.bindValue (":id", id);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();  
  return;
}

QSqlQueryModel *CDICDatabase::getWordListModel ()  {
  if (!db.isOpen ()) return NULL;
  
  QSqlQueryModel *model = new QSqlQueryModel (NULL);
  model->setQuery ("select id, name, classlist from WordPageTable", db);
  
  return model;
}
    
void CDICDatabase::searchWordList (QSqlQueryModel *model, QString search,
                                   QString className, QString languageName)  {
  if (!model) return;
  
  if (search == "" && (className == "" || className == "Any Word Type"))  {
    model->setQuery ("select id, name, classlist from WordPageTable");
    return;
  }
  
  search.prepend ("%");
  search.append ("%");

  QSqlQuery query (db);
  
  if (search == "")  {
    query.prepare ("select id, name, classlist from WordPageTable " +
                   (QString)"where exists " +
                     "(select * from WordClassList " +
                      "where WordClassList.id == WordPageTable.id and " +
                            "WordClassList.class = :class)");
    query.bindValue (":class", className);
  }
  
  else if ((className == "" || className == "Any Word Type") && 
           languageName != "English")  {
    query.prepare ("select id, name, classlist from WordPageTable " +
                  (QString)"where name like :search");
    query.bindValue (":search", search);
  }
                  
  else if (languageName != "English")  {
    query.prepare ((QString)"select id, name, classlist from WordPageTable " +
                   "where name like :search and exists " +
                     "(select * from WordClassList " +
                      "where WordClassList.id == WordPageTable.id and " +
                            "WordClassList.class == :class)");
    query.bindValue (":search", search);
    query.bindValue (":class", className);
  }
  
  else if ((className == "" || className == "Any Word Type") &&
           languageName == "English")  {
    query.prepare ("select Word.id, Word.name, classlist from Word, WordPageTable " +
                   (QString)"where Word.id == WordPageTable.id and " +
                   "definition like :search");
    query.bindValue (":search", search);
  }
  
  else  {
    query.prepare ("select Word.id, Word.name, classlist from Word, WordPageTable " +
                   (QString)"where Word.id == WordPageTable.id and " +
                   "definition like :search and exists " +
                     "(select * from WordClassList " +
                      "where WordClassList.id == WordPageTable.id and " +
                            "WordClassList.class == :class)");
    query.bindValue (":search", search);
    query.bindValue (":class", className);
  }
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  model->setQuery (query);
  
  return;
}
    
QSqlTableModel *CDICDatabase::getWordDisplayModel ()  {
  if (!db.isOpen ()) return NULL;
  
  QSqlTableModel *model = new QSqlTableModel (NULL, db);
  model->setTable ("Word");
  model->setSort (0, Qt::AscendingOrder);
  model->setEditStrategy (QSqlTableModel::OnManualSubmit);
  model->select ();
  
  return model;
}
    
void CDICDatabase::addWord (QString name, QString definition)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  query.prepare ("insert into Word values (null, :name, :def)");
  query.bindValue (":name", name);
  query.bindValue (":def", definition);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
void CDICDatabase::deleteWord (int wordID)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  query.prepare ("delete from Word where id == :id");
  query.bindValue (":id", wordID);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
void CDICDatabase::assignNaturalClass (QString className, int wordID)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  query.prepare ((QString)"insert into WordFeatureSet " + 
                          "select Word.id, feature, value " +
                          "from Word, NaturalClassWord, FeatureBundleWord " +
                          "where Word.id == :wi and " + 
                                "NaturalClassWord.name == :cn and " +
                                "FeatureBundleWord.id == bundleID");
  query.bindValue (":wi", wordID);
  query.bindValue (":cn", className);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}

void CDICDatabase::setPhonology (int wordID, QList<Syllable> phonology)  {
  if (!db.isOpen ()) return;
  
  QSqlQuery query (db);
  
  db.transaction ();
  
  for (int x = 0; x < 7; x++)  {
    QString tableName = "OnsetSupra";
    if (x == 1) tableName = "PeakSupra";
    if (x == 2) tableName = "CodaSupra";
    if (x == 3) tableName = "Onset";
    if (x == 4) tableName = "Peak";
    if (x == 5) tableName = "Coda";
    if (x == 6) tableName = "SyllableSupra";
    
    query.prepare ("delete from " + tableName + " where wordID == :wID");
    query.bindValue (":wID", wordID);
  
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      db.rollback ();
      return;
    }
    
    query.finish ();
  }
  
  for (int syll = 0; syll < phonology.size (); syll++)  {
    // syllable supras
    for (int sup = 0; sup < phonology[syll].supras.size (); sup++)  {
      query.prepare ("select id from Suprasegmental where name == :n");
      query.bindValue (":n", phonology[syll].supras[sup]);
      
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        db.rollback ();
        return;
      }
      
      if (!query.next ())  {
        DATA_ERROR("Could not find suprasegmental " + phonology[syll].supras[sup])
        query.finish ();
        db.rollback ();
        return;
      }
      
      int supID = query.value (0).toInt ();
      
      query.finish ();
      query.prepare ("insert into SyllableSupra values (:wID, :sNum, :sID)");
      query.bindValue (":wID", wordID);
      query.bindValue (":sNum", syll);
      query.bindValue (":sID", supID);
      
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        db.rollback ();
        return;
      }
      
      query.finish ();
    }
    
    for (int x = 0; x < 3; x++)  {
      QString tableName = "Onset";
      if (x == 1) tableName = "Peak";
      if (x == 2) tableName = "Coda";
      
      QList<Phoneme> currList = phonology[syll].onset;
      if (x == 1) currList = phonology[syll].peak;
      if (x == 2) currList = phonology[syll].coda;
      
      for (int p = 0; p < currList.size (); p++)  {
        query.prepare ("select id from Phoneme where name == :n");
        query.bindValue (":n", currList[p].name);
        
        if (!query.exec ())  {
          QUERY_ERROR(query)
          query.finish ();
          db.rollback ();
          return;
        }
        
        if (!query.next ())  {
          DATA_ERROR("Could not find phoneme " + currList[p].name)
          query.finish ();
          db.rollback ();
          return;
        }
        
        int pID = query.value (0).toInt ();
        
        query.finish ();
        query.prepare ("insert into " + tableName + " values (:wID, :sNum, :ind, :pID)");
        query.bindValue (":wID", wordID);
        query.bindValue (":sNum", syll);
        query.bindValue (":ind", p);
        query.bindValue (":pID", pID);
        
        if (!query.exec ())  {
          QUERY_ERROR(query)
          query.finish ();
          db.rollback ();
          return;
        }
        
        query.finish ();
        
        for (int s = 0; s < currList[p].supras.size (); s++)  {
          query.prepare ("select id from Suprasegmental where name == :n");
          query.bindValue (":n", currList[p].supras[s]);
          
          if (!query.exec ())  {
            QUERY_ERROR(query)
            query.finish ();
            db.rollback ();
            return;
          }
          
          if (!query.next ())  {
            DATA_ERROR("Could not find suprasegmental " + currList[p].supras[s])
            query.finish ();
            db.rollback ();
            return;
          }
          
          int sID = query.value (0).toInt ();
          
          query.finish ();
          query.prepare ("insert into " + tableName + "Supra values (:wID, :sNum, :ind, :sID)");
          query.bindValue (":wID", wordID);
          query.bindValue (":sNum", syll);
          query.bindValue (":ind", p);
          query.bindValue (":sID", sID);
          
          if (!query.exec ())  {
            QUERY_ERROR(query)
            query.finish ();
            db.rollback ();
            return;
          }
          
          query.finish ();
        }
      }
    }
  }
  
  db.commit ();
}
     
QString CDICDatabase::getRepresentation (int wordID)  {
  if (!db.isOpen ()) return "";
  
  QSqlQuery query (db);
  QString text = "";
  int syllNum = 0;
  
  while (true)  {
    QString syllText = "";
    
    if (syllNum > 0)
      syllText += ".";
    
    // load before supra
    query.prepare ((QString)"select repText from Suprasegmental, SyllableSupra " +
                   "where wordID == :wi and syllNum == :sn and supraID == id and " +
                         "repType == :rt");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    query.bindValue (":rt", TYPE_BEFORE);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    if (query.next ())
      syllText += query.value (0).toString ();
    
    query.finish ();
    
    QStringList tempList;
    
    // get onset phonemes
    query.prepare ((QString)"select name from Onset, Phoneme " +
                   "where wordID == :wi and syllNum == :sn and phonemeID == id " +
                   "order by ind");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    while (query.next ())
      tempList.append (query.value (0).toString ());
    
    query.finish ();
    
    // get onset supras
    query.prepare ("select ind, repType, repText from OnsetSupra, Suprasegmental " +
                   (QString)"where wordID == :wi and syllNum == :sn and supraID == id");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    while (query.next ())  {
      int ind = query.value (0).toInt ();
      int rty = query.value (1).toInt ();
      QString rtx = query.value (2).toString ();
      
      if (rty == TYPE_BEFORE) tempList[ind].prepend (rtx);
      else if (rty == TYPE_AFTER) tempList[ind].append (rtx);
      else if (rty == TYPE_DOUBLED) tempList[ind].append (tempList[ind]);
      else tempList[ind] = applyDiacritic (rty, tempList[ind]);
    }
    
    syllText += tempList.join ("");
    tempList.clear ();
    
    query.finish ();
    
    // get peak phonemes
    query.prepare ((QString)"select name from Peak, Phoneme " +
                   "where wordID == :wi and syllNum == :sn and phonemeID == id " +
                   "order by ind");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    while (query.next ())
      tempList.append (query.value (0).toString ());
    
    query.finish ();
    
    // get peak-based syllable supra
    query.prepare ((QString)"select repType from SyllableSupra, Suprasegmental " +
                   "where wordID == :wi and syllNum == :sn and supraID == id " +
                     "and (repType < :before or repType == :doubled)");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    query.bindValue (":before", TYPE_BEFORE);
    query.bindValue (":doubled", TYPE_DOUBLED);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    if (query.next ())  {
      if (query.value (0) == TYPE_DOUBLED)  {
        int l = tempList.length ();
        for (int x = 0; x < l; x++)
          tempList.append (tempList[x]);
      }
      
      else for (int x = 0; x < tempList.length (); x++)
        tempList[x] = applyDiacritic (query.value (0).toInt (), tempList[x]);
    }
    
    query.finish ();
    
    // get peak supras
    query.prepare ("select ind, repType, repText from PeakSupra, Suprasegmental " +
                   (QString)"where wordID == :wi and syllNum == :sn and supraID == id");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    while (query.next ())  {
      int ind = query.value (0).toInt ();
      int rty = query.value (1).toInt ();
      QString rtx = query.value (2).toString ();
      
      if (rty == TYPE_BEFORE) tempList[ind].prepend (rtx);
      else if (rty == TYPE_AFTER) tempList[ind].append (rtx);
      else if (rty == TYPE_DOUBLED) tempList[ind].append (tempList[ind]);
      else tempList[ind] = applyDiacritic (rty, tempList[ind]);
    }
    
    query.finish ();
    
    syllText += tempList.join ("");
    tempList.clear ();
    
    // get coda phonemes
    query.prepare ((QString)"select name from Coda, Phoneme " +
                   "where wordID == :wi and syllNum == :sn and phonemeID == id " +
                   "order by ind");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    while (query.next ())
      tempList.append (query.value (0).toString ());
    
    query.finish ();
    
    // get coda supras
    query.prepare ("select ind, repType, repText from CodaSupra, Suprasegmental " +
                   (QString)"where wordID == :wi and syllNum == :sn and supraID == id");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    while (query.next ())  {
      int ind = query.value (0).toInt ();
      int rty = query.value (1).toInt ();
      QString rtx = query.value (2).toString ();
      
      if (rty == TYPE_BEFORE) tempList[ind].prepend (rtx);
      else if (rty == TYPE_AFTER) tempList[ind].append (rtx);
      else if (rty == TYPE_DOUBLED) tempList[ind].append (tempList[ind]);
      else tempList[ind] = applyDiacritic (rty, tempList[ind]);
    }
    
    query.finish ();
    
    syllText += tempList.join ("");
    tempList.clear ();
    
    // get after supra
    query.prepare ((QString)"select repText from Suprasegmental, SyllableSupra " +
                   "where wordID == :wi and syllNum == :sn and supraID == id and " +
                         "repType == :rt");
    query.bindValue (":wi", wordID);
    query.bindValue (":sn", syllNum);
    query.bindValue (":rt", TYPE_AFTER);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      break;
    }
    
    if (query.next ())
      syllText += query.value (0).toString ();
    
    query.finish ();
    
    if (syllText == "" || syllText == ".")
      break;
    
    text += syllText;
    syllText = "";
    
    syllNum++;
  }
  
  return text;
}

QList<Syllable> CDICDatabase::getPhonology (int wordID)  {
  if (!db.isOpen ()) return QList<Syllable> ();
  
  QList<Syllable> syllList;
  QSqlQuery query (db);
  
  // onsets, peaks, codas
  for (int x = 0; x <= 2; x++)  {
    QString tableName = "Onset";
    if (x == 1) tableName = "Peak";
    if (x == 2) tableName = "Coda";
    
    query.prepare ("select syllNum, ind, name from "+ tableName +", Phoneme " +
                   "where id == phonemeID and wordID == :wID");
    query.bindValue (":wID", wordID);
  
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      return syllList;
    }
  
    while (query.next ())  {
      int syllNum = query.value (0).toInt ();
      int ind = query.value (1).toInt ();
      QString phon = query.value (2).toString ();
    
      while (syllList.size () <= syllNum)  {
	Syllable syll;
	syllList.append (syll);
      }
    
      if (x == 0)  {
	while (syllList[syllNum].onset.size () <= ind)  {
	  Phoneme p;
	  syllList[syllNum].onset.append (p);
	}
	  
	syllList[syllNum].onset[ind].name = phon;
      }
      
      else if (x == 1)  {
	while (syllList[syllNum].peak.size () <= ind)  {
	  Phoneme p;
	  syllList[syllNum].peak.append (p);
	}
	  
	syllList[syllNum].peak[ind].name = phon;
      }
      
      else  {
	while (syllList[syllNum].coda.size () <= ind)  {
	  Phoneme p;
	  syllList[syllNum].coda.append (p);
	}
	
	syllList[syllNum].coda[ind].name = phon;
      }
    }
    
    query.finish ();
  }
  
  // phoneme supras
  for (int x = 0; x <= 2; x++)  {
    QString tableName = "OnsetSupra";
    if (x == 1) tableName = "PeakSupra";
    if (x == 2) tableName = "CodaSupra";
    
    query.prepare ("select syllNum, ind, name from " + tableName + ", Suprasegmental " +
		    "where id == supraID and wordID == :wID");
    query.bindValue (":wID", wordID);
  
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      return syllList;
    }
  
    while (query.next ())  {
      int syllNum = query.value (0).toInt ();
      int ind = query.value (1).toInt ();
      QString supra = query.value (2).toString ();
    
      while (syllList.size () <= syllNum)  {
	Syllable syll;
	syllList.append (syll);
      }
    
      if (x == 0)  {
        while (syllList[syllNum].onset.size () <= ind)  {
	  Phoneme p;
	  syllList[syllNum].onset.append (p);
        }
    
        syllList[syllNum].onset[ind].supras.append (supra);
      }
      
      else if (x == 1)  {
        while (syllList[syllNum].peak.size () <= ind)  {
	  Phoneme p;
	  syllList[syllNum].peak.append (p);
        }
    
        syllList[syllNum].peak[ind].supras.append (supra);	
      }
      
      else  {
        while (syllList[syllNum].coda.size () <= ind)  {
	  Phoneme p;
	  syllList[syllNum].coda.append (p);
        }
    
        syllList[syllNum].coda[ind].supras.append (supra);	
      }
    }
  
    query.finish ();
  }
  
  // syllable supras
  query.prepare ("select syllNum, name from SyllableSupra, Suprasegmental " +
		 (QString)"where id == supraID and wordID == :wID");
  query.bindValue (":wID", wordID);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return syllList;
  }
  
  while (query.next ())  {
    int syllNum = query.value (0).toInt ();
    QString supra = query.value (1).toString ();
    
//    DATA_ERROR("Found supra: " + supra)
    
    while (syllList.size () <= syllNum)  {
      Syllable syll;
      syllList.append (syll);
    }
    
    syllList[syllNum].supras.append (supra);
  }
  
  query.finish ();
  
  return syllList;
}

int CDICDatabase::getNumberOfWords ()  {
  if (!db.isOpen ()) return 0;
  
  QSqlQuery query (db);
  query.prepare ("select count(id) from Word");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return 0;
  }
  
  query.next ();
  int num = query.value (0).toInt ();
  query.finish ();
  return num;
}

QList< QList<QStringList> > CDICDatabase::getPhonotacticSequenceList (int loc)  {
  if (!db.isOpen ()) return QList< QList<QStringList> > ();
  
  QList< QList<QStringList> > list;
  
  QString tableName;
  if (loc == ONSET)
    tableName = "LegalOnset";
  else if (loc == PEAK)
    tableName = "LegalPeak";
  else tableName = "LegalCoda";
  
  QSqlQuery query (db);
  query.prepare ("select id, ind, class from " + tableName);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return list;
  }
  
  while (query.next ())  {
    int id = query.value (0).toInt ();
    int index = query.value (1).toInt ();
    QString className = query.value (2).toString ();
    
    while (list.size () <= id)
      list.append (QList<QStringList> ());
    
    while (list[id].size () <= index)
      list[id].append (QStringList ());
    
    list[id][index].append (className);
  }
  
  query.finish ();
  
  return list;
}

QList<Suprasegmental> CDICDatabase::getDiacriticSupras ()  {
  if (!db.isOpen ()) return QList<Suprasegmental> ();
  
  QList<Suprasegmental> supraList;
  for (int x = 0; x < TYPE_BEFORE; x++)  {
    Suprasegmental s;
    s.name = "";
    s.domain = -1;
    supraList.append (s);
  }
  
  QSqlQuery query (db);
  query.prepare ("select id, name, domain, spellType from Suprasegmental");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return supraList;
  }
  
  while (query.next ())  {
    int spellType = query.value (3).toInt ();
    
    if (spellType >= TYPE_BEFORE) continue;
    
    supraList[spellType].name = query.value (1).toString ();
    supraList[spellType].domain = query.value (2).toInt ();
    supraList[spellType].type = spellType;
    
    if (supraList[spellType].domain == SUPRA_DOMAIN_SYLL) continue;
    
    QSqlQuery pquery (db);
    pquery.prepare ((QString)"select name from Phoneme, SupraApplies " +
                    "where phonemeID == id and supraID == :id");
    pquery.bindValue (":id", query.value (0).toInt ());
    
    if (!pquery.exec ())  {
      QUERY_ERROR(pquery)
      pquery.finish ();
      continue;
    }
    
    QStringList applies;
    while (pquery.next ())
      applies.append (pquery.value (0).toString ());
    
    supraList[spellType].applicablePhonemes = applies;
    
    pquery.finish ();
  }
  
  return supraList;
}

QList<Suprasegmental> CDICDatabase::getBeforeSupras ()  {
  if (!db.isOpen ()) return QList<Suprasegmental> ();
  
  QList<Suprasegmental> supraList;
  
  QSqlQuery query (db);
  query.prepare ("select id, name, domain, spellText from Suprasegmental " +
                 (QString)"where spellType == :t");
  query.bindValue (":t", TYPE_BEFORE);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return supraList;
  }
  
  while (query.next ())  {
    Suprasegmental s;
    s.name = query.value (1).toString ();
    s.domain = query.value (2).toInt ();
    s.text = query.value (3).toString ();
    
    if (s.domain == SUPRA_DOMAIN_SYLL)  {
      supraList.append (s);
      continue;
    }
    
    QSqlQuery pquery (db);
    pquery.prepare ((QString)"select name from Phoneme, SupraApplies " +
                    "where phonemeID == id and supraID == :id");
    pquery.bindValue (":id", query.value (0).toInt ());
    
    if (!pquery.exec ())  {
      QUERY_ERROR(pquery)
      pquery.finish ();
      continue;
    }
    
    QStringList applies;
    while (pquery.next ())
      applies.append (pquery.value (0).toString ());
    
    s.applicablePhonemes = applies;
    supraList.append (s);
    
    pquery.finish ();
  }
  
  query.finish ();
  
  return supraList;
}

QList<Suprasegmental> CDICDatabase::getAfterSupras ()  {
  if (!db.isOpen ()) return QList<Suprasegmental> ();
  
  QList<Suprasegmental> supraList;
  
  QSqlQuery query (db);
  query.prepare ("select id, name, domain, spellText from Suprasegmental " +
                 (QString)"where spellType == :t");
  query.bindValue (":t", TYPE_AFTER);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return supraList;
  }
  
  while (query.next ())  {
    Suprasegmental s;
    s.name = query.value (1).toString ();
    s.domain = query.value (2).toInt ();
    s.text = query.value (3).toString ();
    
    if (s.domain == SUPRA_DOMAIN_SYLL)  {
      supraList.append (s);
      continue;
    }
    
    QSqlQuery pquery (db);
    pquery.prepare ((QString)"select name from Phoneme, SupraApplies " +
                    "where phonemeID == id and supraID == :id");
    pquery.bindValue (":id", query.value (0).toInt ());
    
    if (!pquery.exec ())  {
      QUERY_ERROR(pquery)
      pquery.finish ();
      continue;
    }
    
    QStringList applies;
    while (pquery.next ())
      applies.append (pquery.value (0).toString ());
    
    s.applicablePhonemes = applies;
    supraList.append (s);
    
    pquery.finish ();
  }
  
  query.finish ();
  
  return supraList;
}

QList<Suprasegmental> CDICDatabase::getDoubledSupras ()  {
  if (!db.isOpen ()) return QList<Suprasegmental> ();
  
  QList<Suprasegmental> supraList;
  
  QSqlQuery query (db);
  query.prepare ((QString)"select id, name, domain from Suprasegmental " +
                 "where spellType == :t");
  query.bindValue (":t", TYPE_DOUBLED);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return supraList;
  }
  
  while (query.next ())  {
    Suprasegmental s;
    s.name = query.value (1).toString ();
    s.domain = query.value (2).toInt ();
    
    if (s.domain == SUPRA_DOMAIN_SYLL)  {
      supraList.append (s);
      continue;
    }
    
    QSqlQuery pquery (db);
    pquery.prepare ((QString)"select name from Phoneme, SupraApplies " +
                    "where phonemeID == id and supraID == :id");
    pquery.bindValue (":id", query.value (0).toInt ());
    
    if (!pquery.exec ())  {
      QUERY_ERROR(pquery)
      pquery.finish ();
      continue;
    }
    
    QStringList applies;
    while (pquery.next ())
      applies.append (pquery.value (0).toString ());
      
    s.applicablePhonemes = applies;
    supraList.append (s);
    
    pquery.finish ();
  }
  
  query.finish ();
  return supraList;
}
/*
QStringList CDICDatabase::getAllSpellings ()  {
  QMessageBox::information (NULL, "", "getAllSpellings");
  
  if (!db.isOpen ()) return QStringList ();
  
  QList<bool> supraExists;
  for (int x = 0; x < TYPE_BEFORE; x++)  {
    supraExists.append (false);
    
    QSqlQuery squery (db);
    squery.prepare ("select name from Suprasegmental where spellType == :t");
    squery.bindValue (":t", x);
    
    if (!squery.exec ())  {
      QUERY_ERROR(squery)
      squery.finish ();
      continue;
    }
    
    supraExists[x] = squery.next ();
    squery.finish ();
  }
  
  QStringList list;
  
  QSqlQuery query (db);
  query.prepare ((QString)"select distinct spelling from Phoneme, " +
                  "PhonemeSpelling where phonemeID == id order by alpha");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return list;
  }
  
  while (query.next ())  {
    QString spelling = query.value (0).toString ();
    list.append (spelling);
    
    if (plainChars.contains (spelling))
      for (int x = 0; x < supraExists.size (); x++)
        if (supraExists[x]) 
          list.append (applyDiacritic (x, spelling));
  }
  
  query.finish ();
  return list;
}
*/
QMap<QString, QStringList> CDICDatabase::getPhonemesAndSpellings ()  {
  if (!db.isOpen ()) return QMap<QString, QStringList> ();
  
  QMap<QString, QStringList> map;
  
  QSqlQuery query (db);
  query.prepare ("select name, spelling from Phoneme, PhonemeSpelling " +
                 (QString)"where phonemeID == id");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return map;
  }
  
//  int currentPhoneme = 0;
//  list.append (QStringList ());
  
  while (query.next ())  {
    if (!map.contains (query.value (0).toString ()))
      map[query.value (0).toString ()] = QStringList ();
    
    map[query.value (0).toString ()].append (query.value (1).toString ());
  }
  
  query.finish ();
  
  return map;
}

QList<int> CDICDatabase::getAllWordIDs ()  {
  if (!db.isOpen ()) return QList<int> ();
  
  QSqlQuery query (db);
  query.prepare ("select id from Word");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return QList<int> ();
  }
  
  QList<int> idList;
  while (query.next ())
    idList.append (query.value (0).toInt ());
  
  query.finish ();
  
  return idList;
}

QString CDICDatabase::getWordName (int id)  {
  if (!db.isOpen ()) return "";
  
  QSqlQuery query (db);
  query.prepare ("select name from Word where id == :id");
  query.bindValue (":id", id);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return "";
  }
  
  QString name = "";
  if (query.next ())
    name = query.value (0).toString ();
  
  query.finish ();
  
  return name;
}
/*
QStringList CDICDatabase::getPossiblePhonemes (QString spelling, QStringList classNames)  {
  if (!db.isOpen ()) return QStringList ();
  
  QStringList list;
  
  QStringList placeholders;
  for (int x = 0; x < classNames.size (); x++)
    placeholders.append ("?");
  
  QSqlQuery query (db);
  query.prepare ("select name from Phoneme, PhonemeSpelling, PhonClassList " +
                 (QString)"where phonemeID == Phoneme.id and Phoneme.id == " +
                 "PhonClassList.id and spelling == ? and class in (" +
                 placeholders.join (", ") + ") order by alpha");
  query.bindValue (0, spelling);
  for (int x = 0; x < classNames.size (); x++)
    query.bindValue (x+1, classNames[x]);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return list;
  }
  
  while (query.next ())
    list.append (query.value (0).toString ());

  return list;
}
*/
QStringList CDICDatabase::getPhonemesOfClass (QStringList classNames)  {
  if (!db.isOpen ()) return QStringList ();
  
  QStringList list;
  
  QStringList placeholders;
  for (int x = 0; x < classNames.size (); x++)
    placeholders.append ("?");
  
  QSqlQuery query (db);
  query.prepare ((QString)"select name from Phoneme, PhonClassList " +
                 "where Phoneme.id == PhonClassList.id and class in (" +
                 placeholders.join (", ") + ") order by alpha");
  for (int x = 0; x < classNames.size (); x++)
    query.bindValue (x, classNames[x]);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return list;
  }
  
  while (query.next ())
    list.append (query.value (0).toString ());
  
  query.finish ();
  
  return list;
}

QList<Rule> CDICDatabase::getParsingGrammar ()  {
  QList<Rule> ruleList;
  
  // put supras into relevant lists
  QList<Suprasegmental> diacriticSupraList;
  QList<Suprasegmental> beforePhonSupraList;
  QList<Suprasegmental> beforeSyllSupraList;
  QList<Suprasegmental> afterPhonSupraList;
  QList<Suprasegmental> afterSyllSupraList;
  QList<Suprasegmental> doubledSupraList;
  
  QSqlQuery query (db);
  query.prepare ("select id, name, domain, spellType, spellText from Suprasegmental");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return ruleList;
  }
  
  while (query.next ())  {
    Suprasegmental s;
    s.name = query.value (1).toString ();
    s.domain = query.value (2).toInt ();
    s.type = query.value (3).toInt ();
    s.text = query.value (4).toString ();
    s.applicablePhonemes = QStringList ();
    
    QSqlQuery phonQuery (db);
    phonQuery.prepare ((QString)"select name from Phoneme, SupraApplies " +
                       "where supraID == :id and phonemeID == Phoneme.id");
    phonQuery.bindValue (":id", query.value (0).toInt ());
    
    if (!phonQuery.exec ())  {
      QUERY_ERROR(phonQuery)
      phonQuery.finish ();
      continue;
    }
    
    while (phonQuery.next ())
      s.applicablePhonemes.append (phonQuery.value (0).toString ());
    
    phonQuery.finish ();
    
    if (s.domain == SUPRA_DOMAIN_PHON)  {
      if (s.type < TYPE_BEFORE)
        diacriticSupraList.append (s);
      else if (s.type == TYPE_BEFORE)
        beforePhonSupraList.append (s);
      else if (s.type == TYPE_AFTER)
        afterPhonSupraList.append (s);
      else doubledSupraList.append (s);
    }
    
    else  {
      if (s.type < TYPE_BEFORE)
        diacriticSupraList.append (s);
      else if (s.type == TYPE_BEFORE)
        beforeSyllSupraList.append (s);
      else if (s.type == TYPE_AFTER)
        afterSyllSupraList.append (s);
      else doubledSupraList.append (s);
    }
  }
 
  query.finish ();
  
  // add basic rules
  Rule r1;
  r1.lhs = "S";
  r1.rhs = QStringList ("Syll");
  ruleList.append (r1);
  
  Rule r2;
  r2.lhs = "S";
  r2.rhs = QStringList () << "Syll" << "S";
  ruleList.append (r2);
  
  Rule r3;
  r3.lhs = "Syll";
  r3.rhs = QStringList () << "Onset" << "Peak";
  ruleList.append (r3);
  
  Rule r4;
  r4.lhs = "Syll";
  r4.rhs = QStringList () << "Onset" << "Peak" << "Coda";
  ruleList.append (r4);
  
  if (getValue (ONSET_REQUIRED) != "true")  {
    Rule r5;
    r5.lhs = "Syll";
    r5.rhs = QStringList ("Peak");
    ruleList.append (r5);
    
    Rule r6;
    r6.lhs = "Syll";
    r6.rhs = QStringList () << "Peak" << "Coda";
    ruleList.append (r6);
  }
  
  // add syllable before/after supra rules
  for (int s = 0; s < beforeSyllSupraList.size (); s++)  {
    QString chars = beforeSyllSupraList[s].text;
    QStringList splitChars;
    for (int c = 0; c < chars.size (); c++)
      splitChars.append ("Char" + chars[c]);
    
    Rule rule;
    rule.lhs = "Syll";
    rule.rhs = splitChars;
    rule.rhs.append ("Syll");
    ruleList.append (rule);
  }
  
  for (int s = 0; s < afterSyllSupraList.size (); s++)  {
    QString chars = afterSyllSupraList[s].text;
    QStringList splitChars = QStringList ("Syll");
    for (int c = 0; c < chars.size (); c++)
      splitChars.append ("Char" + chars[c]);
    
    Rule rule;
    rule.lhs = "Syll";
    rule.rhs = splitChars;
    ruleList.append (rule);
  }
  
  // add onset/peak/coda rules
  for (int loc = ONSET; loc <= CODA; loc++)  {
    QString lhs;
    QString tableName;
    QList< QList<QStringList> > sequences;
    
    if (loc == ONSET)  {
      lhs = "Onset";
      tableName = "LegalOnset";
    }
    
    else if (loc == PEAK)  {
      lhs = "Peak";
      tableName = "LegalPeak";
    }
    
    else  {
      lhs = "Coda";
      tableName = "LegalCoda";
    }
    
    query.prepare ("select id, ind, name from NaturalClassPhon, " + tableName +
                   " where class == bundleID");
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      continue;
    }
    
    while (query.next ())  {
      int id = query.value (0).toInt ();
      int ind = query.value (1).toInt ();
      QString className = query.value (2).toString ();
      
      while (sequences.size () <= id)
        sequences.append (QList<QStringList> ());
      while (sequences[id].size () <= ind)
        sequences[id].append (QStringList ());
      
      sequences[id][ind].append (className);
    }
    
    query.finish ();
    
    for (int x = 0; x < sequences.size (); x++)  {
      QList<int> currentCombo;
      for (int y = 0; y < sequences[x].size (); y++)
          currentCombo.append (0);
      
      while (true)  {
        QStringList classList;
        for (int c = 0; c < currentCombo.size (); c++)
          if (sequences[x][c].size () > 0)
            classList.append ("Class" + sequences[x][c][currentCombo[c]]);
        
        if (classList.size () > 0)  {
          Rule rule;
          rule.lhs = lhs;
          rule.rhs = classList;
          ruleList.append (rule);
        }
        
        int c = 0;
        for (; c < currentCombo.size (); c++)
          if (currentCombo[c] < (sequences[x][c].size () - 1))  {
            currentCombo[c]++;
            break;
          }
          
        if (c == currentCombo.size ())
          break;
      }
    }
  }
  
  // add class rules
  query.prepare ((QString)"select name, class from PhonClassList, Phoneme " +
                 "where PhonClassList.id == Phoneme.id");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return ruleList;
  }
  
  while (query.next ())  {
    Rule rule;
    rule.lhs = "Class" + query.value (1).toString ();
    rule.rhs = QStringList ("Phon" + query.value (0).toString ());
    ruleList.append (rule);
  }
  
  query.finish ();
  
  // add phoneme rules
  query.prepare ("select name from Phoneme");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return ruleList;
  }
  
  while (query.next ())  {
    QString phoneme = query.value (0).toString ();
    
    for (int x = 0; x < beforePhonSupraList.size (); x++)
      if (beforePhonSupraList[x].applicablePhonemes.contains (phoneme))  {
        QStringList rhs;
        for (int c = 0; c < beforePhonSupraList[x].text.size (); c++)
          rhs.append ("Char" + beforePhonSupraList[x].text[c]);
      
        Rule rule;
        rule.lhs = "Phon" + phoneme;
        rule.rhs = rhs;
        rule.rhs.append ("Phon" + phoneme);
        ruleList.append (rule);
      }
    
    for (int x = 0; x < afterPhonSupraList.size (); x++)
      if (afterPhonSupraList[x].applicablePhonemes.contains (phoneme))  {
        QStringList rhs = QStringList ("Phon" + phoneme);
        for (int c = 0; c < afterPhonSupraList[x].text.size (); c++)
          rhs.append ("Char" + afterPhonSupraList[x].text[c]);
      
        Rule rule;
        rule.lhs = "Phon" + phoneme;
        rule.rhs = rhs;
        ruleList.append (rule);
      }
  }
  
  query.finish ();
  
  query.prepare ("select name, spelling from Phoneme, PhonemeSpelling " +
                 (QString)"where id == phonemeID");
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return ruleList;
  }
  
  while (query.next ())  {
    QString phoneme = query.value (0).toString ();
    QString spelling = query.value (1).toString ();
    
    for (int x = 0; x < diacriticSupraList.size (); x++)
      if (diacriticSupraList[x].applicablePhonemes.contains (phoneme) ||
          (diacriticSupraList[x].domain == SUPRA_DOMAIN_SYLL &&
           plainChars.contains (spelling)))  {
        QString diacriticChars = applyDiacritic (diacriticSupraList[x].type, spelling);
        QStringList rhs;
        for (int c = 0; c < diacriticChars.size (); c++)
          rhs.append ("Char" + diacriticChars[c]);
        
        Rule rule;
        rule.lhs = "Phon" + phoneme;
        rule.rhs = rhs;
        ruleList.append (rule);
      }
      
    for (int x = 0; x < doubledSupraList.size (); x++)
      if (doubledSupraList[x].applicablePhonemes.contains (phoneme) ||
          doubledSupraList[x].domain == SUPRA_DOMAIN_SYLL)  {
        QStringList rhs;
        for (int c = 0; c < spelling.size (); c++)
          rhs.append ("Char" + spelling[c]);
        for (int c = 0; c < spelling.size (); c++)
          rhs.append ("Char" + spelling[c]);
        
        Rule rule;
        rule.lhs = "Phon" + phoneme;
        rule.rhs = rhs;
        ruleList.append (rule);
      }
      
    QStringList rhs;
    
    for (int x = 0; x < spelling.size (); x++)
      rhs.append ("Char" + spelling[x]);
      
    Rule rule;
    rule.lhs = "Phon" + phoneme;
    rule.rhs = rhs;
    ruleList.append (rule);
  }
  
  query.finish ();
  
  // add character rules
  QString addedCharacters = "";
  
  for (int x = 0; x < ruleList.size (); x++)
    for (int r = 0; r < ruleList[x].rhs.size (); r++)
      if (ruleList[x].rhs[r].startsWith ("Char") && 
          !addedCharacters.contains (ruleList[x].rhs[r][4]))  {
        Rule rule;
        rule.lhs = ruleList[x].rhs[r];
        rule.rhs = QStringList ("\"" + ruleList[x].rhs[r][4] + "\"");
        ruleList.append (rule);
        addedCharacters.append (ruleList[x].rhs[r][4]);
      }
  
  return ruleList;
}

EditableQueryModel *CDICDatabase::getFeatureListModel (int domain, int type)  {
  if (!db.isOpen ()) return NULL;
  
  QString tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
  EditableQueryModel *model = new EditableQueryModel (NULL);
  filterFeatureModel (domain, model, type);
  
  return model;
}

void CDICDatabase::filterFeatureModel (int domain, EditableQueryModel *model, int type)  {
  if (!db.isOpen ()) return;

  QString defTable = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  QString subTable = (domain == WORD) ? "WordSubfeature" : "PhonemeSubfeature";
  
  QSqlQuery query (db);
  
  switch (type)  {
    case UNIVALENT: 
      query.prepare ("select name as featName from " + defTable + " " +
                     "where " + 
                       "(exists (select value from " + subTable + 
                               " where name == featName and value == \"\")) " +
                       "and (not exists (select value from " + subTable + 
                                       " where name == featName and value != \"\"))");
      break;
    case BINARY:
      query.prepare ("select name as featName from " + defTable + " " +
                     "where " +
                       "(exists (select value from " + subTable +
                               " where name == featName and value == \"+\")) " +
                       "and (exists (select value from " + subTable +
                                   " where name == featName and value == \"" + MINUS_SIGN + "\")) " +
                       "and (not exists (select value from " + subTable +
                                       " where name == featName and value != \"+\" and value != \"" + MINUS_SIGN + "\"))");
      break;
    case GROUPS:
       query.prepare ("select name as featName from " + defTable + " " +
                      "where " +
                        "(not exists (select name from " + subTable + " where name == featName)) " +
                        "or ((exists (select value from " + subTable + 
                                    " where name == featName and value != \"+\" and value != \"" + MINUS_SIGN + "\")) " +
                            "and (exists (select value from " + subTable +
                                        " where name == featName and value != \"\")))");
        break;
    default:
      query.prepare ("select distinct name from " + subTable);
  }
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  model->setQuery (query);
}

EditableQueryModel *CDICDatabase::getSubfeatureModel (int domain, QString feat)  {
  if (!db.isOpen ()) return NULL;
  
  EditableQueryModel *model = new EditableQueryModel (NULL);
  updateSubfeatureModel (domain, model, feat);
  
  return model;
}

void CDICDatabase::updateSubfeatureModel (int domain, EditableQueryModel *model,
                                          QString feat)  {
  if (!db.isOpen () || !model) return;
  
  QString tableName = (domain == WORD) ? "WordSubfeature" : "PhonemeSubfeature";
  
  QSqlQuery query (db);
  
  if (!feat.isEmpty ())  {
    query.prepare ("select value from " + tableName + " where name == :name");
    query.bindValue (":name", feat);
  }
  
  else query.prepare ("select value from " + tableName + " where name != name");

  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  model->setQuery (query);
}

EditableQueryModel *CDICDatabase::getNaturalClassModel (int domain)  {
  if (!db.isOpen ()) return NULL;
  
  EditableQueryModel *model = new EditableQueryModel;
  
  if (domain == WORD)
    model->setQuery ("select name from NaturalClassWord", db);
  else model->setQuery ("select name from NaturalClassPhon where bundleID > 0", db);
  
  return model;
}
    
QStringList CDICDatabase::getBundledFeatures (int domain, QString className)  {
  if (!db.isOpen ()) return QStringList ();
  
  QString ncTable = (domain == WORD) ? "NaturalClassWord" : "NaturalClassPhon";
  QString featTable = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  QString bundleTable = (domain == WORD) ? "FeatureBundleWord" : "FeatureBundlePhon";
  
  QSqlQuery query (db);
  query.prepare ((QString)"select feature, value, displayType "  +
                 "from " + bundleTable + ", " + featTable + ", " + ncTable +
                " where " + ncTable + ".name == :n and id == bundleID and " +
                 "feature == " + featTable + ".name");
  query.bindValue (":n", className);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return QStringList ();
  }
  
  QStringList featureList;
  
  while (query.next ())  {
    QString f = query.value (0).toString ();
    QString s = query.value (1).toString ();
    QString d = query.value (2).toString ();
    
    if (d == displayType[DISPLAY_COLON])
      featureList.append (f + ": " + s);
    else if (d == displayType[DISPLAY_PREFIX])
      featureList.append (s + f);
    else if (d == displayType[DISPLAY_SUFFIX])
      featureList.append (f + s);
    else if (d == displayType[DISPLAY_BEFORE])
      featureList.append (s + " " + f);
    else if (d == displayType[DISPLAY_AFTER])
      featureList.append (f + " " + s);
    else featureList.append (s);
  }
  
  query.finish ();
  return featureList;
}

QStringList CDICDatabase::getBundledFeatures (int domain, int id)  {
  if (!db.isOpen ()) return QStringList ();
  
  QString setTable = (domain == WORD) ? "WordFeatureSet" : "PhonemeFeatureSet";
  QString defTable = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  QString idName = (domain == WORD) ? "wordID" : "phonemeID";
  
  QSqlQuery query (db);
  query.prepare ((QString)"select feature, value, displayType " +
                 "from " + setTable + ", " + defTable + " " +
                 "where " + idName + " == :id and name == feature");
  query.bindValue (":id", id);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return QStringList ();
  }
  
  QStringList featureList;
  
  while (query.next ())  {
    QString f = query.value (0).toString ();
    QString s = query.value (1).toString ();
    QString d = query.value (2).toString ();
    
    if (d == displayType[DISPLAY_COLON])
      featureList.append (f + ": " + s);
    else if (d == displayType[DISPLAY_PREFIX])
      featureList.append (s + f);
    else if (d == displayType[DISPLAY_SUFFIX])
      featureList.append (f + s);
    else if (d == displayType[DISPLAY_BEFORE])
      featureList.append (s + " " + f);
    else if (d == displayType[DISPLAY_AFTER])
      featureList.append (f + " " + s);
    else featureList.append (s); 
  }
  
  query.finish ();
  return featureList;
}
    
void CDICDatabase::addFeature (int domain, QStringList featList, QStringList sub)  {
  if (!db.isOpen ()) return;
  if (!featList.size ()) return;
  
  QString tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
  QString display = displayType[DISPLAY_COLON];
  if (sub.size () == 1 && sub.contains ("")) 
    display = displayType[DISPLAY_SUFFIX];
  if (sub.size () == 2 && sub.contains ("+") && sub.contains (MINUS_SIGN))
    display = displayType[DISPLAY_PREFIX];
  
  QSqlQuery query (db);
  
  for (int f = 0; f < featList.size (); f++)  {
    db.transaction ();
    tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
    query.prepare ("insert into " + tableName + " values (:name, null, null, :disp)");
    query.bindValue (":name", featList[f]);
    query.bindValue (":disp", display);
  
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      db.rollback ();
      continue;
    }
  
    query.finish ();
  
    tableName = (domain == WORD) ? "WordSubfeature" : "PhonemeSubfeature";
  
    for (int s = 0; s < sub.size (); s++)  {
      query.prepare ("insert into " + tableName + " values (:name, :value)");
      query.bindValue (":name", featList[f]);
      query.bindValue (":value", sub[s]);
    
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        db.rollback ();
        break;
      }
    
      query.finish ();
    }
  
    db.commit ();
  }
}
    
void CDICDatabase::deleteFeature (int domain, QStringList featList)  {
  if (!db.isOpen ()) return;
  if (!featList.size ()) return;
  
  QString tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
  QStringList placeholders;
  for (int x = 0; x < featList.size (); x++)
    placeholders.append ("?");
  
  QSqlQuery query (db);
  query.prepare ("delete from " + tableName + " where name in (" + 
                 placeholders.join (", ") + ")");
  for (int x = 0; x < featList.size (); x++)
    query.bindValue (x, featList[x]);
  
  if (!query.exec ())
    QUERY_ERROR(query)
  
  query.finish ();
}
    
void CDICDatabase::addSubfeature (int domain, QString feat, QStringList subList)  {
  if (!db.isOpen ()) return;
  if (!subList.size ()) return;
  
  QString tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
  QSqlQuery query (db);
  query.prepare ("select name from " + tableName + " where name == :name");
  query.bindValue (":name", feat);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (!query.next ())  {
    DATA_ERROR("No such feature: " + feat)
    query.finish ();
    return;
  }
  
  tableName = (domain == WORD) ? "WordSubfeature" : "PhonemeSubfeature";
  
  for (int s = 0; s < subList.size (); s++)  {
    query.prepare ("insert into " + tableName + " values (:name, :value)");
    query.bindValue (":name", feat);
    query.bindValue (":value", subList[s]);
  
    if (!query.exec ())
      QUERY_ERROR(query)
    
    query.finish ();
  }
}
    
void CDICDatabase::deleteSubfeature (int domain, QString feat, QStringList subList)  {
  if (!db.isOpen ()) return;
  if (!subList.size ()) return;
  
  QString tableName = (domain == WORD) ? "WordSubfeature" : "PhonemeSubfeature";
  
  QStringList placeholders;
  for (int x = 0; x < subList.size (); x++)
    placeholders.append ("?");
  
  QSqlQuery query (db);
  query.prepare ("delete from " + tableName + " where name == ? and value in (" +
                 placeholders.join (", ") + ")");
  query.bindValue (0, feat);
  for (int x = 0; x < subList.size (); x++)
    query.bindValue (x+1, subList[x]);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}

void CDICDatabase::renameFeature (int domain, QString before, QString after)  {
  if (!db.isOpen ()) return;
  if (before == after) return;
  
  QString tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
  QSqlQuery query (db);
  query.prepare ("select name from " + tableName + " where name == :n");
  query.bindValue (":n", after);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (query.next ())  {
    QMessageBox::warning (NULL, "Error", "There is already a feature with that name.");
    query.finish ();
    return;
  }
  
  query.finish ();
  
  query.prepare ("update " + tableName + " set name = :new where name == :old");
  query.bindValue (":new", after);
  query.bindValue (":old", before);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}

void CDICDatabase::renameSubfeature (int domain, QString feat, QString before, QString after)  {
  if (!db.isOpen ()) return;
  if (before == after) return;
  
  QString tableName = (domain == WORD) ? "WordSubfeature" : "PhonemeSubfeature";
  
  QSqlQuery query (db);
  query.prepare ("select name from " + tableName + " where name == :f and value == :s");
  query.bindValue (":f", feat);
  query.bindValue (":s", after);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (query.next ())  {
    QMessageBox::warning (NULL, "Error", "There is already a subfeature with that name.");
    query.finish ();
    return;
  }
  
  query.finish ();
  
  query.prepare ("update " + tableName + " set value = :new where name == :feat and value == :old");
  query.bindValue (":new", after);
  query.bindValue (":old", before);
  query.bindValue (":feat", feat);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
void CDICDatabase::setDisplay (int domain, QString feat, int disp)  {
  if (!db.isOpen ()) return;
  if (disp < 0 || disp >= displayType.size ()) return;
  
  QString tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
  QSqlQuery query (db);
  query.prepare ("select name from " + tableName + " where name == :name");
  query.bindValue (":name", feat);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (!query.next ())  {
    query.finish ();
    return;
  }
  
  query.finish ();
  
  query.prepare ("update " + tableName + " set displayType = :d where name == :n");
  query.bindValue (":d", displayType[disp]);
  query.bindValue (":n", feat);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
void CDICDatabase::setFeatureParent (int domain, QString feat, QString parent,
                                     QString parentSub)  {
  if (!db.isOpen ()) return;
  
  QString tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
  QSqlQuery query (db);
  query.prepare ("update " + tableName + " set parentName = :n, parentValue = :v"
                 + " where name == :f");
  query.bindValue (":n", parent);
  query.bindValue (":v", parentSub);
  query.bindValue (":f", feat);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
QString CDICDatabase::getParent (int domain, QString feat)  {
  if (!db.isOpen ()) return "";
  
  QString tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  
  QSqlQuery query (db);
  query.prepare ("select parentName, parentValue from " + tableName + 
                 " where name == :name");
  query.bindValue (":name", feat);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return "";
  }
  
  if (!query.next ())  {
    query.finish ();
    return "";
  }
  
  QString pfeat = query.value (0).toString ();
  QString psub = query.value (1).toString ();
  query.finish ();
  
  if (pfeat.isEmpty ())  {
    query.finish ();
    return "None";
  }
  
  query.prepare ("select displayType from " + tableName + " where name == :pn");
  query.bindValue (":pn", pfeat);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return "";
  }
  
  if (!query.next ())  {
    query.finish ();
    return "";
  }
  
  QString text = "";
  
  if (query.value (0) == displayType[DISPLAY_COLON])
    text = pfeat + ": " + psub;
  else if (query.value (0) == displayType[DISPLAY_PREFIX])
    text = psub + pfeat;
  else if (query.value (0) == displayType[DISPLAY_SUFFIX])
    text = pfeat + psub;
  else if (query.value (0) == displayType[DISPLAY_BEFORE])
    text = psub + " " + pfeat;
  else if (query.value (0) == displayType[DISPLAY_AFTER])
    text = pfeat + " " + psub;
  else if (query.value (0) == displayType[DISPLAY_SOLO])
    text = psub;
  
  query.finish ();
  return text;
}
    
QStringList CDICDatabase::getDisplayType (int domain, QString feat)  {
  if (!db.isOpen ()) return QStringList ();
  
  QStringList displayList;
  QString tableName = (domain == WORD) ? "WordSubfeature" : "PhonemeSubfeature";
  
  QSqlQuery query (db);
  query.prepare ("select value from " + tableName + " where name == :n");
  query.bindValue (":n", feat);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return displayList;
  }
  
  if (!query.next ())  {
    query.finish ();
    return displayList;
  }
  
  QString subfeature = query.value (0).toString ();
  
  query.finish ();
  
  displayList.append (feat + ": " + subfeature);
  displayList.append (subfeature + feat);
  displayList.append (feat + subfeature);
  displayList.append (subfeature + " " + feat);
  displayList.append (feat + " " + subfeature);
  displayList.append (subfeature);
  
  tableName = (domain == WORD) ? "WordFeatureDef" : "PhonemeFeatureDef";
  query.prepare ("select displayType from " + tableName + " where name == :n");
  query.bindValue (":n", feat);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return QStringList ();
  }
  
  if (!query.next ())  {
    query.finish ();
    return QStringList ();
  }
  
  int displayIndex = displayType.indexOf (query.value (0).toString ());
  
  if (displayIndex >= 0 && displayIndex < displayList.size ())
    displayList.append (displayList[displayIndex]);
  else displayList.append ("");
  
  return displayList;
}

void CDICDatabase::addNaturalClass (int domain, QString className)  {
  if (!db.isOpen () || className == "") 
    return;
  
  QString tableName = (domain == WORD) ? "NaturalClassWord" : "NaturalClassPhon";
  
  QSqlQuery query (db);
  query.prepare ("select name from " + tableName + " where name == :n");
  query.bindValue (":n", className);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (query.next ())  {
    QMessageBox::warning (NULL, "Error", "A class with that name already exists!");
    query.finish ();
    return;
  }
  
  query.finish ();
  query.prepare ("insert into " + tableName + " values (null, :n)");
  query.bindValue (":n", className);
  
  if (!query.exec ())
    QUERY_ERROR(query)

  query.finish ();
  return;
}
    
void CDICDatabase::deleteNaturalClass (int domain, QStringList classList)  {
  if (!db.open () || !classList.size ()) 
    return;
  
  QString tableName = (domain == WORD) ? "NaturalClassWord" : "NaturalClassPhon";
  
  QSqlQuery query (db);
  
  for (int x = 0; x < classList.size (); x++)  {
    query.prepare ("delete from " + tableName + " where name == :n");
    query.bindValue (":n", classList[x]);
  
    if (!query.exec ())
      QUERY_ERROR(query)
    
    query.finish ();
  }
}

void CDICDatabase::renameNaturalClass (int domain, QString before, QString after)  {
  if (!db.isOpen () || before == "" || after == "")
    return;
  
  QString tableName = (domain == WORD) ? "NaturalClassWord" : "NaturalClassPhon";
  
  QSqlQuery query (db);
  query.prepare ("select name from " + tableName + " where name == :n");
  query.bindValue (":n", after);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (query.next ())  {
    QMessageBox::warning (NULL, "Error", "There is already a natural class with that name.");
    query.finish ();
    return;
  }
  
  query.finish ();
  query.prepare ("update " + tableName + " set name = :new where name == :old");
  query.bindValue (":new", after);
  query.bindValue (":old", before);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
void CDICDatabase::addFeatureToClass (int domain, QString cName, QString feat, QString sub)  {
  if (!db.isOpen () || cName == "" || feat == "") 
    return;
  
  QString classTable = (domain == WORD) ? "NaturalClassWord" : "NaturalClassPhon";
  QString bundleTable = (domain == WORD) ? "FeatureBundleWord" : "FeatureBundlePhon";
  
  QSqlQuery query (db);
  query.prepare ("select bundleID from " + classTable + " where name == :n");
  query.bindValue (":n", cName);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (!query.next ())  {
    query.finish ();
    return;
  }
  
  int classID = query.value (0).toInt ();
  
  query.finish ();
  query.prepare ("insert into " + bundleTable + " values (:id, :feat, :val)");
  query.bindValue (":id", classID);
  query.bindValue (":feat", feat);
  query.bindValue (":val", sub);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
  return;
}

void CDICDatabase::addFeatureToSet (int domain, int id, QString feat, QString sub)  {
  if (!db.isOpen () || feat == "") 
    return;
  
  QString tableName = (domain == WORD) ? "WordFeatureSet" : "PhonemeFeatureSet";
  
  QSqlQuery query (db);
  query.prepare ("insert into " + tableName + " values (:id, :feat, :val)");
  query.bindValue (":id", id);
  query.bindValue (":feat", feat);
  query.bindValue (":val", sub);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}

void CDICDatabase::removeFeatureFromClass (int domain, QString className, QString feat)  {
  if (!db.isOpen () || className == "" || feat == "") 
    return;
  
  QString classTable = (domain == WORD) ? "NaturalClassWord" : "NaturalClassPhon";
  QString bundleTable = (domain == WORD) ? "FeatureBundleWord" : "FeatureBundlePhon";
  
  QSqlQuery query (db);
  query.prepare ("select bundleID from " + classTable + " where name == :n");
  query.bindValue (":n", className);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (!query.next ())  {
    query.finish ();
    return;
  }
  
  int classID = query.value (0).toInt ();
  
  query.finish ();
  query.prepare ("delete from " + bundleTable + 
                " where id == :id and feature == :feat");
  query.bindValue (":id", classID);
  query.bindValue (":feat", feat);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}

void CDICDatabase::removeFeatureFromSet (int domain, int id, QString feat)  {
  if (!db.isOpen () || feat == "") 
    return;
  
  QString tableName = (domain == WORD) ? "WordFeatureSet" : "PhonemeFeatureSet";
  QString idName = (domain == WORD) ? "wordID" : "phonemeID";
  
  QSqlQuery query (db);
  query.prepare ("delete from " + tableName + 
                " where " + idName + " == :id and feature == :feat");
  query.bindValue (":id", id);
  query.bindValue (":feat", feat);
  
  if (!query.exec ())
    QUERY_ERROR (query)
    
  query.finish ();
}
    
void CDICDatabase::clearClass (int domain, QString className)  {
  if (!db.isOpen () || className == "") 
    return;
  
  QString classTable = (domain == WORD) ? "NaturalClassWord" : "NaturalClassPhon";
  QString bundleTable = (domain == WORD) ? "FeatureBundleWord" : "FeatureBundlePhon";
  
  QSqlQuery query (db);
  query.prepare ("select bundleID from " + className + "where name == :n");
  query.bindValue (":n", className);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return;
  }
  
  if (!query.next ())  {
    query.finish ();
    return;
  }
  
  int classID = query.value (0).toInt ();
  
  query.finish ();
  query.prepare ("delete from " + bundleTable + " where id == :id");
  query.bindValue (":id", classID);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}
    
void CDICDatabase::clearSet (int domain, int id)  {
  if (!db.isOpen ()) return;
  
  QString tableName = (domain == WORD) ? "WordFeatureSet" : "PhonemeFeatureSet";
  QString idName = (domain == WORD) ? "wordID" : "phonemeID";
  
  QSqlQuery query (db);
  query.prepare ("delete from " + tableName + " where " + idName + " == :id");
  query.bindValue (":id", id);
  
  if (!query.exec ())
    QUERY_ERROR(query)
    
  query.finish ();
}

QStringList CDICDatabase::getClassList (int domain)  {
  if (!db.isOpen ()) return QStringList ();
  
  QString tableName = (domain == WORD) ? "NaturalClassWord" : "NaturalClassPhon";
  
  QSqlQuery query (db);
  query.prepare ("select name from " + tableName);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return QStringList ();
  }
  
  QStringList classList;
  
  while (query.next ())
    classList.append (query.value (0).toString ());
  
  return classList;
}

QStringList  CDICDatabase::getClassList (int domain, int id)  {
  if (!db.isOpen ()) return QStringList ();
  
  QString viewName = (domain == WORD) ? "WordClassList" : "PhonClassList";
  
  QSqlQuery query (db);
  query.prepare ("select class from " + viewName + " where id == :id");
  query.bindValue (":id", id);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return QStringList ();
  }
  
  QStringList classList;
  
  while (query.next ())
    classList.append (query.value(0).toString ());
  
  query.finish ();
  
  return classList;
}

QString CDICDatabase::applyDiacritic (int diacritic, QString old)  {
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

void CDICDatabase::putInOrder (int *first, int *second, int *third)  {
  // Takes four int pointers, and assigns all non-negative values the numbers
  // one through four, based on the order of the numbers from lowest to highest.

  int *array[4] = {first, second, third};

  for (int x = 0; x < 2; x++)  {
    int smallest = x;

    for (int y = x+1; y < 3; y++)
      if (*array[y] < *array[smallest])
        smallest = y;

    int *temp = array[x];
    array[x] = array[smallest];
    array[smallest] = temp;
  }

  int number = 1;

  for (int x = 0; x < 3; x++)
    if (*array[x] >= 0)
      *array[x] = number++;
}

bool CDICDatabase::readSQLFile (QString filename)  {
  if (!db.isOpen ()) return false;
  
  QFile file (filename);
  QTextStream in (&file);
  file.open (QIODevice::ReadOnly | QIODevice::Text);
      
  QString line = in.readLine ();
      
  while (!in.atEnd ())  {
    QString queryText = "";
        
    while (line.trimmed () == "" || line.startsWith ("--"))
      line = in.readLine ();
        
    while (line.trimmed () != "" && !in.atEnd ())  {
      queryText += line;
      line = in.readLine ();
    };
        
    if (line != "")
      queryText += line;
        
    queryText.chop (1);
        
    QSqlQuery query (db);
        
    if (!query.exec (queryText))  {
      QUERY_ERROR(query);
      query.finish ();
      file.close ();
      return false;
    }
        
    query.finish ();
    queryText = "";
  };
      
  file.close ();
  
//  QMessageBox::information (NULL, "", "End of readSQLFile");
  return true;
}

bool CDICDatabase::loadInventory (QDomElement inv)  {
  if (inv.isNull ()) return false;

  QString alphabet = inv.firstChildElement ("alphabet").text ();
  QStringList alphaStrings = alphabet.split (" ");

  QDomNodeList nodeList = inv.childNodes ();
  int currentID = 0;

  for (int x = 0; x < nodeList.size (); x++)  {
    QDomElement currentElement = nodeList.at(x).toElement ();
    
    if (currentElement.isNull ()) 
      continue;
    if (currentElement.tagName () != "phonemeDef")
      continue;

    QString name = currentElement.attribute ("name");
    QStringList spellings = currentElement.firstChildElement ("orthography").text ().split (' ');
    QString notes = currentElement.firstChildElement ("notes").text ();
    int alpha = alphaStrings.indexOf (name);
      
    QSqlQuery query (db);
    query.prepare ("insert into Phoneme values (:id, :alpha, :name, :notes)");
    query.bindValue (":id", currentID);
    query.bindValue (":alpha", alpha);
    query.bindValue (":name", name);
    query.bindValue (":notes", notes);

    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      return false;
    }
      
    query.finish ();
      
    for (int x = 0; x < spellings.size (); x++)  {
      query.prepare ("insert into PhonemeSpelling values (:id, :spelling)");
      query.bindValue (":id", currentID);
      query.bindValue (":spelling", spellings[x]);

      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        return false;
      }
        
      query.finish ();
    }
    
    currentID++;
  }
  
  return true;
}

bool CDICDatabase::loadSupras (QDomElement supras)  {
  if (supras.isNull ()) return true;
  
  QDomNodeList nodeList = supras.childNodes ();
  int currentID = 0;
  
  for (int x = 0; x < nodeList.size (); x++)  {
    QDomElement currentElement = nodeList.at(x).toElement ();
    
    if (currentElement.isNull ()) 
      continue;
    if (currentElement.tagName () != "supraDef")
      continue;
    
    QString name = currentElement.attribute ("name");
    int domain = currentElement.attribute ("domain", "0").toInt ();
    QStringList applies = currentElement.firstChildElement ("applies").text ().split (' ');
    QString notes = currentElement.firstChildElement ("notes").text ();

    QDomElement spelling = currentElement.firstChildElement ("spelling");
    int spellType = spelling.attribute ("type", "0").toInt ();
    QString spellText = spelling.text ();
    
    switch (spellType)  {
      case 0:
        if (spellText == "Acute Accent")
          spellType = TYPE_ACUTE;
        else if (spellText == "Grave Accent")
          spellType = TYPE_GRAVE;
        else if (spellText == "Circumflex")
          spellType = TYPE_CIRCUMFLEX;
        else if (spellText == "Diaresis/Umlaut")
          spellType = TYPE_DIARESIS;
        else if (spellText == "Macron")
          spellType = TYPE_MACRON;
        spellText = "";
        break;
      case 1:
        spellType = TYPE_BEFORE;
        break;
      case 2:
        spellType = TYPE_AFTER;
        break;
      case 3:
        spellType = TYPE_DOUBLED;
        spellText = "";
        break;
      default:
        spellType = 0;
        spellText = "";
    }
    
    QDomElement rep = currentElement.firstChildElement ("representation");
    int repType = rep.attribute ("type", "0").toInt ();
    QString repText = rep.text ();
    
    switch (repType)  {
      case 0:
        if (repText == "Acute Accent")
          repType = TYPE_ACUTE;
        else if (repText == "Grave Accent")
          repType = TYPE_GRAVE;
        else if (repText == "Circumflex")
          repType = TYPE_CIRCUMFLEX;
        else if (repText == "Diaresis/Umlaut")
          repType = TYPE_DIARESIS;
        else if (repText == "Macron")
          repType = TYPE_MACRON;
        repText = "";
        break;
      case 1:
        repType = TYPE_BEFORE;
        break;
      case 2:
        repType = TYPE_AFTER;
        break;
      case 3:
        repType = TYPE_DOUBLED;
        repText = "";
        break;
      default:
        repType = 0;
        repText = "";
    }
    
    QSqlQuery query (db);
    query.prepare ((QString)"insert into Suprasegmental values " +
                            "(:id, :name, :domain, :rt, :rx, :st, :sx, :notes)");
    query.bindValue (":id", currentID);
    query.bindValue (":name", name);
    query.bindValue (":domain", domain);
    query.bindValue (":rt", repType);
    query.bindValue (":rx", repText);
    query.bindValue (":st", spellType);
    query.bindValue (":sx", spellText);
    query.bindValue (":notes", notes);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      return false;
    }
    
    query.finish ();
    
    QStringList placeholders;
    for (int a = 0; a < applies.size (); a++)
      placeholders.append ("?");
    
    query.prepare ((QString)"insert into SupraApplies " +
                            "select ?, id from Phoneme where name in " +
                            "(" + placeholders.join (", ") + ")");
    query.bindValue (0, currentID);
    for (int a = 0; a < applies.size (); a++)
      query.bindValue (a+1, applies[a]);
  
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      return false;
    }
    
    query.finish ();
    
    currentID++;
  }
  
  return true;
}

bool CDICDatabase::loadWordFeatures (QDomElement words)  {
  if (words.isNull ()) return true;
  
  QStringList types;
  QList<QStringList> subTypes;
  
  QDomNodeList nodeList = words.childNodes ();
  
  int bundleID = 1;
  
  for (int x = 0; x < nodeList.size (); x++)  {
    QDomElement currentElement = nodeList.at(x).toElement ();
    
    if (currentElement.isNull ()) 
      continue;
    if (currentElement.tagName () != "wordDef")
      continue;
    
    QString t = currentElement.attribute ("type", "");
    QString st = currentElement.attribute ("subtype", "");
    
    if (t == "") continue;
    
    if (!types.contains (t))
      types.append (t);
    
    if (st == "") continue;
    
    int index = types.indexOf (t);
    
    while (subTypes.size () <= index)
      subTypes.append (QStringList ());
    
    subTypes[index].append (st);
  }
  
  while (subTypes.size () < types.size ())
    subTypes.append (QStringList ());
  
  QSqlQuery query (db);
  query.prepare ("insert into WordFeatureDef values (:n, null, null, :d)");
  query.bindValue (":n", "Type");
  query.bindValue (":d", displayType[DISPLAY_COLON]);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return false;
  }
  
  query.finish ();
  
  for (int x = 0; x < types.size (); x++)  {
    query.prepare ("insert into WordSubfeature values (:name, :value)");
    query.bindValue (":name", "Type");
    query.bindValue (":value", types[x]);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      continue;
    }
    
    query.finish ();
    query.prepare ("insert into NaturalClassWord values (:id, :name)");
    query.bindValue (":id", bundleID);
    query.bindValue (":name", types[x]);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      continue;
    }
    
    query.finish ();
    query.prepare ("insert into FeatureBundleWord values (:id, :feat, :val)");
    query.bindValue (":id", bundleID);
    query.bindValue (":feat", "Type");
    query.bindValue (":name", types[x]);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      continue;
    }
    
    query.finish ();
    bundleID++;
    
    if (subTypes[x].size () > 0)  {
      query.prepare ("insert into WordFeatureDef values (:n, :pn, :pv, :d)");
      query.bindValue (":n", types[x]);
      query.bindValue (":pn", "Type");
      query.bindValue (":pv", types[x]);
      query.bindValue (":d", displayType[DISPLAY_COLON]);
      
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        continue;
      }
      
      query.finish ();
      
      for (int s = 0; s < subTypes[x].size (); s++)  {
        query.prepare ("insert into WordSubfeature values (:name, :value)");
        query.bindValue (":name", types[x]);
        query.bindValue (":value", subTypes[x][s]);
        
        if (!query.exec ())
          QUERY_ERROR(query)
          
        query.finish ();
      }
    }
  }
  
  return true;
}

bool CDICDatabase::loadSequence (QDomElement sequence, int loc, int wordID, 
                                 int syllNum)  {
  if (sequence.isNull ()) return true;
  
  QString tableName;
  if (loc == ONSET)
    tableName = "Onset";
  else if (loc == PEAK)
    tableName = "Peak";
  else tableName = "Coda";
  
  QDomNodeList nodeList = sequence.childNodes ();
  int indNum = 0;
  
  QSqlQuery query (db);
  
  for (int p = 0; p < nodeList.size (); p++)  {
    QDomElement currentPhoneme = nodeList.at(p).toElement ();
      
    if (currentPhoneme.isNull ())
      continue;
    if (currentPhoneme.tagName () != "phoneme")
      continue;
      
    query.prepare ("select id from Phoneme where name == :name");
    query.bindValue (":name", currentPhoneme.attribute ("name", ""));
      
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      return false;
    }
      
    if (!query.next ())  {
      DATA_ERROR("Could not find phoneme: " + currentPhoneme.attribute ("name", ""))
      continue;
    }
      
    int phonNum = query.value (0).toInt ();
        
    query.finish ();
    query.prepare ("insert into " + tableName + 
                   " values (:word, :syll, :ind, :phon)");
    query.bindValue (":word", wordID);
    query.bindValue (":syll", syllNum);
    query.bindValue (":ind", indNum);
    query.bindValue (":phon", phonNum);
        
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      return false;
    }
        
    QDomNodeList supraList = currentPhoneme.childNodes ();
    for (int s = 0; s < supraList.size (); s++)  {
      QDomElement currentSupra = supraList.at(s).toElement ();
        
      if (currentSupra.isNull ())
        continue;
      if (currentSupra.tagName () != "supra")
        continue;
        
      query.finish ();
      query.prepare ("select id from Suprasegmental where name == :name");
      query.bindValue (":name", currentSupra.text ());
        
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        return false;
      }
        
      if (!query.next ())  {
        DATA_ERROR("Could not find suprasegmental: " + currentSupra.text ())
        continue;
      }
        
      int supraNum = query.value (0).toInt ();
        
      query.finish ();
      query.prepare ("insert into " + tableName +
                     "Supra values (:word, :syll, :ind, :supra)");
      query.bindValue (":word", wordID);
      query.bindValue (":syll", syllNum);
      query.bindValue (":ind", indNum);
      query.bindValue (":supra", supraNum);
        
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        return false;
      }
    }
      
    query.finish ();
      
    indNum++;
  }
  
  return true;
}

bool CDICDatabase::loadWord (QDomElement word, int wordID)  {
  if (word.isNull ()) return true;
  
  QString name = word.attribute ("name");
  QString definition = word.firstChildElement ("definition").text ();
  
  QSqlQuery query (db);
  query.prepare ("insert into Word values (:id, :name, :def)");
  query.bindValue (":id", wordID);
  query.bindValue (":name", name);
  query.bindValue (":def", definition);
  
  if (!query.exec ())  {
    QUERY_ERROR(query)
    query.finish ();
    return false;
  }
  
  query.finish ();
  
  QString type = word.attribute ("type", "");
  QString subtype = word.attribute ("subtype", "");
  
  if (type != "")  {
    query.prepare ("insert into WordFeatureSet values (:word, :feat, :val)");
    query.bindValue (":word", wordID);
    query.bindValue (":feat", "Type");
    query.bindValue (":val", type);
    
    if (!query.exec ())  {
      QUERY_ERROR(query)
      query.finish ();
      return false;
    }
    
    query.finish ();
    
    if (subtype != "")  {
      query.prepare ("insert into WordFeatureSet values (:word, :feat, :val)");
      query.bindValue (":word", wordID);
      query.bindValue (":feat", type);
      query.bindValue (":val", subtype);
      
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        return false;
      }
    }
  }
  
  QDomNodeList syllList = word.firstChildElement ("phonology").childNodes ();
  int syllNum = 0;
  
  for (int x = 0; x < syllList.size (); x++)  {
    QDomElement currentSyllable = syllList.at(x).toElement ();
    
    if (currentSyllable.isNull ())
      continue;
    if (currentSyllable.tagName () != "syllable")
      continue;
    
    QStringList syllSupras;
    syllSupras.append (currentSyllable.attribute ("bSupra", ""));
    syllSupras.append (currentSyllable.attribute ("fSupra", ""));
    syllSupras.append (currentSyllable.attribute ("pSupra", ""));
    
    for (int sup = 0; sup < syllSupras.size (); sup++)  {
      if (syllSupras[sup] == "") continue;
      
      query.prepare ("select id from Suprasegmental where name == :name");
      query.bindValue (":name", syllSupras[sup]);
      
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        return false;
      }
      
      if (!query.next ())  {
        DATA_ERROR("Could not find suprasegmental: " + syllSupras[sup])
        continue;
      }
      
      int supraNum = query.value (0).toInt ();
        
      query.finish ();
      query.prepare ("insert into SyllableSupra values (:word, :syll, :supra)");
      query.bindValue (":word", wordID);
      query.bindValue (":syll", syllNum);
      query.bindValue (":supra", supraNum);
        
      if (!query.exec ())  {
        QUERY_ERROR(query)
        query.finish ();
        return false;
      }
      
      query.finish ();
    }
    
    if (!loadSequence (currentSyllable.firstChildElement ("onset"), ONSET, wordID, syllNum))
      return false;
    
    if (!loadSequence (currentSyllable.firstChildElement ("peak"), PEAK, wordID, syllNum))
      return false;

    if (!loadSequence (currentSyllable.firstChildElement ("coda"), CODA, wordID, syllNum))
      return false;
    
    syllNum++;
  }
  
  return true;
}

bool CDICDatabase::loadWordList (QDomElement words)  {
  if (words.isNull ()) return true;
  
  if (!loadWordFeatures (words))
    return false;
  
  QDomNodeList nodeList = words.childNodes ();
  int wordID = 0;
  
  for (int x = 0; x < nodeList.size (); x++)  {
    QDomElement currentWord = nodeList.at(x).toElement ();
    
    if (currentWord.isNull ())
      continue;
    if (currentWord.tagName () != "wordDef")
      continue;
    
    if (!loadWord (currentWord, wordID))
      return false;
    
    wordID++;
  }
  
  return true;
}