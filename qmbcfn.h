// note: this is only used for OS X

#ifndef QMBCFN_H
#define QMBCFN_H

#include <QtWidgets>
#include "mainwindow.h"

class qmbcfn : public QApplication
{
    Q_OBJECT
  public:
    qmbcfn(int argc, char* argv[]);
    virtual ~qmbcfn();
  protected:
     bool event(QEvent*);
  private:
     void loadFile(const QString& fileName);
  public:
    MainWindow* mainwin;
};

#endif
