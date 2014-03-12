#ifndef EARLEYPARSER_H
#define EARLEYPARSER_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QMap>

typedef struct Rule_s  {
  QString lhs;
  QStringList rhs;
} Rule;

typedef struct BackPointer_s  {
  int position;
  int item;
} BackPointer;

typedef struct ChartItem_s  {
  QString lhs;
  QStringList rhs;
  int nextElement;
  int start;
  int end;
  QList<BackPointer> backPointers;
} ChartItem;

typedef struct TreeNode_s  {
  QString label;
  QList<struct TreeNode_s> children;
  QString payload;
} TreeNode;

class EarleyParser  {
  public:
    EarleyParser ();
    void setRules (QList<Rule>);
    void setIgnored (QString);
    
    TreeNode parse (QString);
    
    QString stringTreeNode (TreeNode);
    
  private:
    QList<Rule> nonterminals;
    QStringList preterminals;
    QMap<QString, QStringList> terminals;
    QString ignored;
    QList< QList<ChartItem> > chart;
    
    TreeNode getTreeNode (int, int);
    int countCodas (TreeNode);
    
    void runPredictor (int, int);
    void runScanner (int, int, QString);
    void runCompleter (int, int);
    
    void printRule (Rule);
    void printChart ();
    QString stringChartItem (ChartItem);
    
    void addChartItem (ChartItem, int);
};

#endif