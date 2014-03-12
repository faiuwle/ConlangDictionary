#ifndef EDITABLEQUERYMODEL_H
#define EDITABLEQUERYMODEL_H

#include <QSqlQueryModel>

// A QSqlQueryModel that is "editable" in the sense that it appears that way to
// the view classes.  Instead of actually editing the model, the two signals are
// emitted, which the containing class can field and use to update the database
// in an appropriate way.  (Could be made more effective by setting some SQL
// to be executed when changes are made, using :new and :old, or something like
// that, but then db manipulations would not be exclusive to CDICDatabase, and
// the parent GUI class couldn't manage its db updates/model refreshes as easily.)
class EditableQueryModel : public QSqlQueryModel  {
  Q_OBJECT
  
  public:
    EditableQueryModel (QObject* = 0);
    
    bool setData (const QModelIndex&, const QVariant&, int = Qt::EditRole);
    Qt::ItemFlags flags (const QModelIndex&) const;
    
  signals:
    void itemChanged (QVariant, QVariant);
    void indexEdited (QModelIndex, QVariant);
};

#endif