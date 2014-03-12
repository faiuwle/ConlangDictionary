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
//      terminal.chop (1);
//      terminal.remove (0, 1);
      
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
  
//  QTextStream out (stdout);
  
//  out << "Nonterminals: " << endl;
//  for (int x = 0; x < nonterminals.size (); x++)
//    printRule (nonterminals[x]);
  
//  out << endl << endl << "Preterminals: " << endl;
//  for (int x = 0; x < preterminals.size (); x++)
//    out << preterminals[x] << " ";
  
//  out << endl << endl << "Terminals: " << endl;
//  QMap<QString, QStringList>::const_iterator i = terminals.constBegin ();
//  while (i != terminals.constEnd ())  {
//    out << i.key () << ":";
    
//    QStringList ptList = i.value ();
    
//    for (int x = 0; x < ptList.size (); x++)
//      out << " " << ptList[x];
    
//    out << endl;
    
//    i++;
//  }
}

void EarleyParser::setIgnored (QString i)  {
  ignored = i;
}

TreeNode EarleyParser::parse (QString input)  {
  chart.clear ();
  
  QStringList words;
  
  for (int x = 0; x < input.size (); x++)
    if (!ignored.contains (input[x]))
      words.append ("\"" + input[x] + "\"");
  
  QTextStream terminal (stdout);
//  terminal << words.join (" ") << endl;
  
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
          runScanner (x, y, words[x]);
        else runPredictor (x, y);
      }
      
      else runCompleter (x, y);
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
  
  int bestTree = 0;
  int bestCount = countCodas (nodeList[0]);
  
  for (int x = 1; x < nodeList.size (); x++)  {
    int codas = countCodas (nodeList[x]);
    
    if (codas < bestCount)  {
      bestTree = x;
      bestCount = codas;
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
      
      addChartItem (newItem, item.end);
    }
    
//  cout << "Ran predictor:" << endl;
//  printChart ();
}

void EarleyParser::runScanner (int chartPosition, int itemNum, QString word)  {
  if (chart.size () <= chartPosition) return;
  if (chart[chartPosition].size () <= itemNum) return;
  
  ChartItem item = chart[chartPosition][itemNum];
  
  if (!terminals.contains (word)) return;
  
  if (item.rhs.size () <= item.nextElement)  {
    cout << "Error: Illegal chart item" << endl;
    return;
  }
  
  QString nextElement = item.rhs[item.nextElement];
  
  if (terminals[word].contains (nextElement))  {
    ChartItem newItem;
    newItem.lhs = nextElement;
    newItem.rhs = QStringList (word);
    newItem.nextElement = 1;
    newItem.start = item.end;
    newItem.end = item.end + 1;
    
    addChartItem (newItem, item.end + 1);
  }
  
//  cout << "Ran scanner:" << endl;
//  printChart ();
}

void EarleyParser::runCompleter (int chartPosition, int itemNum)  {
  if (chart.size () <= chartPosition) return;
  if (chart[chartPosition].size () <= itemNum) return;
  
  ChartItem item = chart[chartPosition][itemNum];
  
  if (chart.size () <= item.start)  {
    cout << "Error: chart not big enough for completer" << endl;
    return;
  }
  
  for (int x = 0; x < chart[item.start].size (); x++)  {
    ChartItem checkItem = chart[item.start][x];
    
    if (checkItem.rhs.size () <= checkItem.nextElement)
      continue;
    
    QString nextElement = checkItem.rhs[checkItem.nextElement];
    
    if (nextElement == item.lhs)  {
      ChartItem newItem;
      newItem.lhs = checkItem.lhs;
      newItem.rhs = checkItem.rhs;
      newItem.nextElement = checkItem.nextElement + 1;
      newItem.start = checkItem.start;
      newItem.end = item.end;
      newItem.backPointers = checkItem.backPointers;
      BackPointer newBackPointer;
      newBackPointer.position = chartPosition;
      newBackPointer.item = itemNum;
      newItem.backPointers.append (newBackPointer);
      
      addChartItem (newItem, item.end);
    }
  }
  
//  cout << "Ran completer:" << endl;
//  printChart ();
}

void EarleyParser::printRule (Rule rule)  {
  QTextStream terminal (stdout);
  
  terminal << rule.lhs << " ->";
  
  for (int x = 0; x < rule.rhs.size (); x++)
    terminal << " " << rule.rhs[x];
  
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
        && item.end == chart[position][x].end)  {
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