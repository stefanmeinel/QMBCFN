#include <QApplication>

#ifdef Q_WS_MACX

  #include "qmbcfn.h"

  int main(int argc, char *argv[])
  {
    Q_INIT_RESOURCE(QMBCFN);
    qmbcfn app(argc, argv);
    app.setWindowIcon(QIcon(":/images/qmbf_48x48.png"));
    return app.exec();
  }

#else

  #include "mainwindow.h"

  int main(int argc, char *argv[])
  {
    Q_INIT_RESOURCE(QMBCFN);
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/qmbf_48x48.png"));
    if((app.arguments()).size()==2)
    {
      QString file=(app.arguments()).at(1);
      MainWindow mainwin(true, file, false);
      mainwin.show();
      return app.exec();
    }
    if((app.arguments()).size()==3)
    {
      QString option=(app.arguments()).at(1);
      if(option=="--runfit")
      {
        QString file=(app.arguments()).at(2);
        MainWindow mainwin(true, file, true);
      //  mainwin.show();
      //  return app.exec();
        return 0;
      }
    }
    MainWindow mainwin(false, "", false);
    mainwin.show();
    return app.exec();
  }

#endif
