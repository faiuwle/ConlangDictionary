#include <QApplication>

#include <QFile>

#include <QTextStream>
#include <QMessageBox>

#include "mainwindow.h"

int main (int argc, char *argv[])  {
  QApplication app (argc, argv);
  QFont font ("DejaVu Sans", 8);
  QString path = ".";
  QString filename = argc > 1 ? argv[1] : "";

  QFile settingsFile ("cd.config");
  QTextStream in (&settingsFile);
  
  if (settingsFile.open (QIODevice::ReadOnly | QIODevice::Text))  {
    while (!in.atEnd ())  {
      QString line = in.readLine ();
    
      if (line.startsWith ("Font"))  {
        line.remove (0, 5);
        font.fromString (line);
      }
    
      else if (line.startsWith ("Path"))  {
        line.remove (0, 5);
        path = line;
      }
    }
  }
  
  app.setFont (font);
  MainWindow mainWin;
  mainWin.setPath (path);
  if (filename != "")
    mainWin.openDictionary (filename);
  mainWin.show ();
  return app.exec ();
}