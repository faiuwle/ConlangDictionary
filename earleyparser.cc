#include <iostream>
using namespace std;

#include <QTextStream>

#include "earleyparser.h"

EarleyParser::EarleyParser ()  {}

void EarleyParser::setRules (QList<Rule> list)  {
  nonterminals.clear ();
  preterminals.clear ();
  terminals.clear ();
  
  for (int x = 0; x < list.size (); x++)  {
    bool hybrid = false;
    
    if (list[x].rhs.size () == 1 && list[x].rhs[0].startsWith ("\"") &&
        list[x].rhs[0].endsWith ("\""))  {
      QString terminal = list[x].rhs[0];
      
      if (!terminals.contains (terminal))
        terminals[terminal] = QStringList ();
      terminals[terminal].append (list[x].lhs);
    
      if (!preterminals.contains (list[x].lhs))
        preterminals.append (list[x].lhs);
    }
    
    else  {
      for (int y = 0; y < list[x].rhs.size (); y++)
        if (list[x].rhs[y].startsWith ("\"") && 
            list[x].rhs[y].endsWith ("\""))
          hybrid = true;
     
      if (!hybrid) nonterminals.append (list[x]);
    }
  }
/*
  for (int x = 0; x < nonterminals.size (); x++)
    printRule (nonterminals[x]);
  
  QTextStream terminal (stdout);
  
  QMap<QString, QStringList>::const_iterator i;
  for (i = terminals.constBegin (); i != terminals.constEnd (); i++)
    terminal << i.key () << ": " << i.value ().join (", ") << endl;*/
}

void EarleyParser::setIgnored (QString i)  {
  ignored = i;
}

TreeNode EarleyParser::parse (QString input)  {
  chart.clear ();
  
  QStringList words;
  
  for (int x = 0; x < input.size (); x++)
    if (!ignored.contains (input[x]) && input[x] != ' ')
      words.append ("\"" + input[x] + "\"");
  
  QTextStream terminal (stdout);
  
  ChartItem start;
  start.lhs = "TOP";
  start.rhs = QStringList ("S");
  start.nextElement = 0;
  start.start = 0;
  start.end = 0;
  
  addChartItem (start, 0);
  
  for (int x = 0; x < words.size () + 1; x++)  {
    if (chart.size () <= x) break;
    
    for (int y = 0; y < chart[x].size (); y++)  {
      if (chart[x][y].rhs.size () > chart[x][y].nextElement)  {
        QString nextElement = chart[x][y].rhs[chart[x][y].nextElement];
        
        if (preterminals.contains (nextElement) && x < words.size ())
          runScanner (x, y, words[x], ((x+1) < words.size () ? words[x+1] : ""));
        else runPredictor (x, y);
      }
      
      else runCompleter (x, y, (x < words.size () ? words[x] : ""));
    }
  }
  
  QList<TreeNode> nodeList;
  
  if (chart.size () <= words.size ())  {
    TreeNode tn;
    tn.label = "";
    tn.payload = "";
    return tn;
  }
  
  for (int x = 0; x < chart[words.size ()].size (); x++)  {
    ChartItem item = chart[words.size ()][x];
    
    if (item.lhs == "TOP" && item.nextElement == item.rhs.size ())
      nodeList.append (getTreeNode (words.size (), x));
  }
  
  if (nodeList.size () == 0)  {
    TreeNode tn;
    tn.label = "";
    tn.payload = "";
    return tn;
  }
  
  QList<int> bestTrees;
  int bestCodaCount = countCodas (nodeList[0]);
  
  for (int x = 1; x < nodeList.size (); x++)  {
    int codas = countCodas (nodeList[x]);
    
    if (codas < bestCodaCount)
      bestCodaCount = codas;
  }
  
  for (int x = 0; x < nodeList.size (); x++)
    if (countCodas (nodeList[x]) == bestCodaCount)
      bestTrees.append (x);
    
  int bestTree = bestTrees[0];
  int bestContextCount = countContexts (nodeList[bestTree]);
  
  for (int x = 0; x < bestTrees.size (); x++)  {
    int contexts = countContexts (nodeList[bestTrees[x]]);
    
    if (contexts > bestContextCount)  {
      bestTree = bestTrees[x];
      bestContextCount = contexts;
    }
  }
  
  return nodeList[bestTree];
}

QString EarleyParser::stringTreeNode (TreeNode node)  {
    if (node.payload != "")
    return "(" + node.label + " " + node.payload + ")";
  
  QString text = "(" + node.label;
  
  for (int x = 0; x < node.children.size (); x++)
    text += " " + stringTreeNode (node.children[x]);
  
  text += ")";
  
  return text;
}

TreeNode EarleyParser::getTreeNode (int chartPosition, int itemNum)  {
  TreeNode node;
  
  if (chart.size () <= chartPosition) return node;
  if (chart[chartPosition].size () <= itemNum) return node;
  
  ChartItem item = chart[chartPosition][itemNum];
  
  node.label = item.lhs;
  node.context = (item.context != "");
  
  if (item.backPointers.size () == 0)  {
    QString p = item.rhs[0];
    p.chop (1);
    p.remove (0, 1);
    
    node.payload = p;
    return node;
  }
  
  for (int x = 0; x < item.backPointers.size (); x++)
    node.children.append (getTreeNode (item.backPointers[x].position, 
                                       item.backPointers[x].item));
   
  node.payload = "";
    
  return node;
}

int EarleyParser::countCodas (TreeNode node)  {
  int count = node.label == "Coda" ? 1 : 0;
  
  for (int x = 0; x < node.children.size (); x++)
    count += countCodas (node.children[x]);
  
  return count;
}

int EarleyParser::countContexts (TreeNode node)  {
  int count = node.context ? 1 : 0;
  
  for (int x = 0; x < node.children.size (); x++)
    count += countContexts (node.children[x]);
  
  return count;
}

void EarleyParser::runPredictor (int chartPosition, int itemNum)  {
  if (chart.size () <= chartPosition) return;
  if (chart[chartPosition].size () <= itemNum) return;
  
  ChartItem item = chart[chartPosition][itemNum];

  if (item.rhs.size () <= item.nextElement)  {
    cout << "Error: Illegal chart item" << endl;
    return;
  }
  
  QString nextElement = item.rhs[item.nextElement];
  
  for (int x = 0; x < nonterminals.size (); x++)
    if (nonterminals[x].lhs == nextElement)  {
      ChartItem newItem;
      newItem.lhs = nonterminals[x].lhs;
      newItem.rhs = nonterminals[x].rhs;
      newItem.nextElement = 0;
      newItem.start = item.end;
      newItem.end = item.end;
      newItem.context = nonterminals[x].context;
      
      addChartItem (newItem, item.end);
    }
    
//  cout << "Ran predictor" << endl;
//  printChart ();
}

void EarleyParser::runScanner (int chartPosition, int itemNum, QString word, QString next)  {
  if (chart.size () <= chartPosition) return;
  if (chart[chartPosition].size () <= itemNum) return;
  
  ChartItem item = chart[chartPosition][itemNum];
  
  if (!terminals.contains (word)) return;
  
  if (item.rhs.size () <= item.nextElement)  {
    cout << "Error: Illegal chart item" << endl;
    return;
  }
  
  QString nextElement = item.rhs[item.nextElement];
  
//  QTextStream terminal (stdout);
//  terminal << stringChartItem (item) << endl;
  
  if (terminals[word].contains (nextElement) &&
      (item.context == "" || ("\"" + item.context + "\"") == next))  {
    ChartItem newItem;
    newItem.lhs = nextElement;
    newItem.rhs = QStringList (word);
    newItem.nextElement = 1;
    newItem.start = item.end;
    newItem.end = item.end + 1;
    newItem.context = "";
    
    addChartItem (newItem, item.end + 1);
  }
  
//  cout << "Ran scanner" << endl;
//  printChart ();
}

void EarleyParser::runCompleter (int chartPosition, int itemNum, QString next)  {
  if (chart.size () <= chartPosition) return;
  if (chart[chartPosition].size () <= itemNum) return;
  
  ChartItem item = chart[chartPosition][itemNum];
  
  if (chart.size () <= item.start)  {
    cout << "Error: chart not big enough for completer" << endl;
    return;
  }
  
//  QTextStream terminal (stdout);
//  terminal << stringChartItem (item) << endl;
  
  for (int x = 0; x < chart[item.start].size (); x++)  {
    ChartItem checkItem = chart[item.start][x];
    
    if (checkItem.rhs.size () <= checkItem.nextElement)
      continue;
    
    QString nextElement = checkItem.rhs[checkItem.nextElement];
    
//    terminal << stringChartItem (checkItem) << ", " << next << endl;
    
    if (nextElement == item.lhs && 
        (checkItem.context == "" || ("\""+ checkItem.context + "\"") == next))  {      
      ChartItem newItem;
      newItem.lhs = checkItem.lhs;
      newItem.rhs = checkItem.rhs;
      newItem.nextElement = checkItem.nextElement + 1;
      newItem.start = checkItem.start;
      newItem.end = item.end;
      newItem.context = checkItem.context;
      newItem.backPointers = checkItem.backPointers;
      BackPointer newBackPointer;
      newBackPointer.position = chartPosition;
      newBackPointer.item = itemNum;
      newItem.backPointers.append (newBackPointer);
      
      addChartItem (newItem, item.end);
    }
  }
  
//  cout << "Ran completer" << endl;
//  printChart ();
}

void EarleyParser::printRule (Rule rule)  {
  QTextStream terminal (stdout);
  
  terminal << rule.lhs << " ->";
  
  for (int x = 0; x < rule.rhs.size (); x++)
    terminal << " " << rule.rhs[x];
  
  if (rule.context != "")
    terminal << " | " << rule.context;
  
  terminal << endl;
}

void EarleyParser::printChart ()  {
  QTextStream out (stdout);
  
  for (int x = 0; x < chart.size (); x++)  {
    out << "chart[" << x << "]" << endl;
    
    for (int y = 0; y < chart[x].size (); y++)
      out << stringChartItem (chart[x][y]) << endl;
    
    out << endl;
  }
}

QString EarleyParser::stringChartItem (ChartItem item)  {
  QString text = item.lhs + " ->";
  
  for (int x = 0; x < item.rhs.size (); x++)  {
    text += " ";
    
    if (item.nextElement == x)
      text += ". ";
    
    text += item.rhs[x];
  }
  
  if (item.nextElement == item.rhs.size ())
    text += " .";
  
  if (item.context != "")
    text += " | " + item.context;
  
  text += " [" + QString::number (item.start) + ", " + 
          QString::number (item.end) + "]";
          
  for (int x = 0; x < item.backPointers.size (); x++)
    text += " [" + QString::number (item.backPointers[x].position) +
            ", " + QString::number (item.backPointers[x].item) + "]";
          
  return text;
}

void EarleyParser::addChartItem (ChartItem item, int position)  {
  while (chart.size () <= position)
    chart.append (QList<ChartItem> ());
  
  for (int x = 0; x < chart[position].size (); x++)
    if (item.lhs == chart[position][x].lhs && item.rhs == chart[position][x].rhs
        && item.nextElement == chart[position][x].nextElement 
        && item.start == chart[position][x].start 
        && item.end == chart[position][x].end 
        && item.context == chart[position][x].context)  {
      bool sameBP = true;
    
      if (item.backPointers.size () != chart[position][x].backPointers.size ())
        continue;
      
      for (int b = 0; b < item.backPointers.size (); b++)
        if (item.backPointers[b].position != 
            chart[position][x].backPointers[b].position ||
            item.backPointers[b].item !=
            chart[position][x].backPointers[b].item)
          sameBP = false;
        
      if (sameBP) return;
    }
    
  chart[position].append (item);
}