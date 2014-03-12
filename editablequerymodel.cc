#include "editablequerymodel.h"

EditableQueryModel::EditableQueryModel (QObject *parent)  
 : QSqlQueryModel (parent)
{}

bool EditableQueryModel::setData (const QModelIndex &index, const QVariant &data, int role)  {
  emit itemChanged (index.data (), data);
  emit indexEdited (index, data);
  
  return true;
}

Qt::ItemFlags EditableQueryModel::flags (const QModelIndex &index) const  {
  return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}