#ifndef CDICDATABASE_H
#define CDICDATABASE_H

#include <QSqlDatabase>
#include <QList>

#include "const.h"
#include "earleyparser.h"

class QString;
class QSqlQueryModel;
class QSqlTableModel;
class EditableQueryModel;
class QDomElement;

// This is basically an interface for QSqlDatabase, so that other classes do not
// have to deal with SQL and queries, and can just call functions of this class.
class CDICDatabase { 
  public:
    CDICDatabase ();
    CDICDatabase (QString);
    
    // database management
    bool open (QString);
    void close ();
    void clear ();
    void clearWordlist ();
    void transaction ();
    void rollback ();
    void commit ();
    
    QString currentDB ();
    
    bool loadFromXML (QString);
    bool loadFromText (QString, QString);
    bool loadLexique (QString, QStringList);
    bool loadFeatures (int, QString);
    
    bool saveToText (QString, QString);
    bool saveFeatures (int, QString);
    
    // dictionary settings
    bool settingDefined (QString);
    QString getValue (QString);
    void setValue (QString, QString);
    
    // phonemes
    QSqlQueryModel *getPhonemeListModel ();
    QSqlTableModel *getPhonemeDisplayModel ();
    
    void addPhoneme (QString name, QString notes = "");
    void deletePhoneme (QString);
    void movePhonemeUp (int);
    void movePhonemeDown (int);
    void setSpellings (QString, QString);
    void assignNaturalClass (QString, QString);
    QString getPhonemeName (int);
    QString getSpellingText (QString);
    QStringList getAllPhonemeNames ();
    
    // suprasegmentals
    QSqlQueryModel *getSupraListModel ();
    QSqlTableModel *getSupraDisplayModel ();
    
    void addSupra (QString);
    void deleteSupra (QString);
    void setSupraApplies (QString, QStringList);
    QStringList getSupraApplies (QString);
    QList<Suprasegmental> getSupras ();
    
    // phonotactics
    QStringList getSequenceList (int);
    void addSequence (int, QList<QStringList>);
    void removeSequence (int, int);
    
    // words
    QSqlQueryModel *getWordListModel ();
    void searchWordList (QSqlQueryModel*, QString, QString, QString);
    QSqlTableModel *getWordDisplayModel ();
    
    int addWord (QString name, QString definition = "");
    void deleteWord (int);
    void assignNaturalClass (QString, int);
    bool setPhonology (int, QList<Syllable>);
    QString getRepresentation (int);
    QList<Syllable> getPhonology (int);
    int getNumberOfWords ();
    void setDefinition (QString, int);
    QString getDefinition (int);
    
    // for word parsing
    QList< QList<QStringList> > getPhonotacticSequenceList (int);
    QList<Suprasegmental> getDiacriticSupras ();
    QList<Suprasegmental> getBeforeSupras ();
    QList<Suprasegmental> getAfterSupras ();
    QList<Suprasegmental> getDoubledSupras ();
    QMap<QString, QStringList> getPhonemesAndSpellings ();
    QList<int> getAllWordIDs ();
    QString getWordName (int);
    QStringList getPhonemesOfClass (QStringList);
    QList<Rule> getParsingGrammar ();
    
    // features and natural classes (domain-generalized)
    // models for feature dialog
    EditableQueryModel *getFeatureListModel (int, int = UNIVALENT);
    void filterFeatureModel (int, EditableQueryModel*, int);
    EditableQueryModel *getSubfeatureModel (int, QString);
    void updateSubfeatureModel (int, EditableQueryModel*, QString);
    
    // models for bundles dialog
    EditableQueryModel *getNaturalClassModel (int);
    QStringList getBundledFeatures (int, QString);
    QStringList getBundledFeatures (int, int);
    
    // functions for features dialog
    void addFeature (int, QStringList, QStringList = QStringList ());
    void deleteFeature (int, QStringList);
    void addSubfeature (int, QString, QStringList);
    void deleteSubfeature (int, QString, QStringList);
    void renameFeature (int, QString, QString);
    void renameSubfeature (int, QString, QString, QString);
    void setDisplay (int, QString, int);
    void setFeatureParent (int, QString, QString = QString (), QString = QString ());
    QString getParent (int, QString);
    QStringList getDisplayType (int, QString);
    
    // functions for bundles dialog
    void addNaturalClass (int, QString);
    void deleteNaturalClass (int, QStringList);
    void renameNaturalClass (int, QString, QString);
    void addFeatureToClass (int, QString, QString, QString);
    void addFeatureToSet (int, int, QString, QString);
    void removeFeatureFromClass (int, QString, QString);
    void removeFeatureFromSet (int, int, QString);
    void clearClass (int, QString);
    void clearSet (int, int);
    
    // misc tab-specific functions that can still be domain-generalized
    QStringList getClassList (int);
    QStringList getClassList (int, int);
    
    QString applyDiacritic (int, QString);
    
  private:
    void putInOrder (int*, int*, int*);
    
    bool readSQLFile (QString);
    
    bool loadInventory (QDomElement);
    bool loadSupras (QDomElement);

    bool loadWordFeatures (QDomElement);
    bool loadSequence (QDomElement, int, int, int);
    bool loadWord (QDomElement, int);
    bool loadWordList (QDomElement);

    QSqlDatabase db;
};

#endif