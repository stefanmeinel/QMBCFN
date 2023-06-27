#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>

#include <gsl/gsl_sf_gamma.h>

#ifdef Q_WS_MACX
  #include "file_open_filter.h"
#endif

#include "parser.h"

#include "mainwidget.h"
#include "functionwidget.h"
#include "paramderivswidget.h"
#include "constantswidget.h"
#include "chisqrextratermwidget.h"
#include "fitsettingswidget.h"

#include "chisqr_extra_term.h"
#include "fitter.h"
#include "fit_thread.h"
#include "fit_dialog.h"
#include "bootstrap_thread.h"
#include "bootstrap_dialog.h"
#include "abstract_prior.h"
#include "gaussian_prior.h"
#include "abstract_global_model.h"

#include "parse_model.h"

#include "parser.h"

#include "settingsmap.h"

using namespace std;

class QAction;
class QMenu;

const double version=1.86;

const int start_n_functions=1;
const int max_n_functions=1000;

const int start_n_parameters=1;
const int max_n_parameters=500;

const int start_n_constants=0;
const int max_n_constants=500;

const bool start_bayes=false;
const bool start_num_diff=true;
const bool start_second_deriv_minimization=false;
const bool start_second_deriv_covariance=true;
const double start_startlambda=0.001;
const double start_lambdafac=10.0;
const double start_tolerance=0.001;
const double start_num_diff_step=1e-08;
const int start_svd=0;
const double start_svd_ratio=0.000001;
const double start_svd_value=0.000000000001;
const double start_rescale_value=0.99;

const int start_steps=100;

class MainWindow : public QMainWindow
{
  Q_OBJECT

  public:
    MainWindow(bool load_file, QString file_name);

  signals:
    void lf(QString);

  protected:

    void closeEvent(QCloseEvent*);

#ifdef Q_WS_MACX
  public slots:

    void loadFile(const QString& fileName);
#endif

  private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();

    void parse_triggered();
    void parse_toggled();

    void update_model_widgets();

    void start_to_prior();
    void result_to_start();
    void result_to_prior();
    void chisqr_start();
    void reload_data_file();
    void write_results();

    void bayesian_modified(int);
    void chisqrextra_enabled_modified();
    void num_diff_modified(int);
    void second_deriv_minimization_modified(int);
    void second_deriv_covariance_modified(int);
    void svd_modified();
    void n_functions_modified(int);
    void n_parameters_modified(int);
    void n_constants_modified();
    void parameter_changed(int, std::string);
    void derivative_changed();
    void function_changed();
    void constant_changed();

    void aboutWindow();

    void modified();
    void data_file_modified();
    void file_type_modified(int newstate);

#ifndef Q_WS_MACX
    void loadFile(const QString& fileName);
#endif

    void fit();
    void fit_finished();
    void abort_fit();
    void abort_bootstrap();
    void bootstrap_finished();

  private:

    void createActions();
    void createMenus();
    void create_Widgets_Dialogs();

    void check_current_model();

    bool prepare_fit();

    bool load_data_file();

    bool set_fit_data();

    bool reset_fitter();

    void reset();

    bool maybeSave();
    bool saveFile(const QString& fileName);
    void setCurrentFile(const QString& fileName);

    QString strippedName(const QString& fullFileName);

    QTabWidget* tabWidget;

    QMenu* fileMenu;
    QMenu* modelMenu;

    QMenu* toolsMenu;
    QMenu* helpMenu;

    QToolBar* fileToolBar;
    QToolBar* toolsToolBar;

    QString current_dir;
    QString curFile;

    QAction* newAct;
    QAction* openAct;
    QAction* saveAct;
    QAction* saveAsAct;
    QAction* exitAct;

    QAction* parse_Act;

    QAction* start_to_prior_Act;
    QAction* result_to_start_Act;
    QAction* result_to_prior_Act;
    QAction* chisqr_start_Act;
    QAction* reload_data_file_Act;
    QAction* write_results_Act;

    QAction* aboutAct;

    MainWidget* mainWidget;

    FunctionWidget* funcWidget;
    ParamDerivsWidget* parderWidget;
    ConstantsWidget* constsWidget;
    ChiSqrExtraTermWidget* chisqrextraWidget;
    FitSettingsWidget* fitsetWidget;

    enum model  // keep existing entries for backward compatibility! UPDATE mbf_load_save!
    {
       PARSE=0,
    }; // UPDATE mbf_load_save!

    model current_model;

    bool something_modified;
    bool need_to_fit;
    bool need_to_load_data_file;
    bool need_to_set_fit_data;
    bool need_to_reset_fitter;

    fitter* _fitter;
    fit_thread* fitthread;
    fit_dialog* fitdialog;
    bootstrap_thread* bootstrapthread;
    bootstrap_dialog* bootstrapdialog;
    gaussian_prior* _gaussian_prior;
    chisqr_extra_term* _chisqr_extra_term;
    abstract_global_model* _model;

    double last_chi_sqr;

    vector< double > file_data;
    vector< vector< double > > multi_file_data;
    vector< vector< double > > file_data_covariance_matrix;
    vector< vector< vector< double > > > multi_file_data_covariance_matrix;

    vector< vector< double > > multi_fit_data;
    vector< vector< vector< double > > > multi_fit_data_covariance_matrix;

};

#endif
