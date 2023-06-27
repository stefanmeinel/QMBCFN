#include "mainwindow.h"

bool _in(const string& sub, const string& s)
{
  if(s.find(sub)!=string::npos)
  {
    return true;
  }
  else
  {
    return false;
  }
}


// *******************************************************************
// constructor
// *******************************************************************
MainWindow::MainWindow(bool load_file, QString file_name)
{
  resize(1200, 640);

  createActions();
  createMenus();
  create_Widgets_Dialogs();

  setCentralWidget(tabWidget);

  _fitter=NULL;
  fitthread = new fit_thread;
  connect(fitthread, SIGNAL(fit_finished()), this, SLOT(fit_finished()));
  bootstrapthread = new bootstrap_thread();
  connect(bootstrapthread, SIGNAL(finished()), this, SLOT(bootstrap_finished()));
  connect(bootstrapthread, SIGNAL(message_signal(QString)), mainWidget, SLOT(printm(QString)));
  connect(bootstrapthread, SIGNAL(step(int)), bootstrapdialog, SLOT(update_bar(int)));

  _gaussian_prior=NULL;
  _chisqr_extra_term=NULL;
  _model=NULL;

  reset();

  if(load_file)
  {
    current_dir=QFileInfo(file_name).absolutePath();
    emit lf(file_name);
  }

#ifdef Q_WS_MACX
  FileOpenFilter *file_open_filter = new FileOpenFilter();
  this -> installEventFilter(file_open_filter);
  connect(file_open_filter, SIGNAL(file_open(QString)), this, SLOT(loadFile(QString)));
#endif

  gsl_set_error_handler_off();
}


// *******************************************************************
// this function creates the actions for menu and toolbar
// *******************************************************************
void MainWindow::createActions()
{
  newAct=new QAction(QIcon(":/images/filenew.png"), tr("&New"), this);
  newAct->setShortcut(tr("Ctrl+N"));
  connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

  openAct=new QAction(QIcon(":/images/fileopen.png"), tr("&Open..."), this);
  openAct->setShortcut(tr("Ctrl+O"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  saveAct=new QAction(QIcon(":/images/filesave.png"), tr("&Save"), this);
  saveAct->setShortcut(tr("Ctrl+S"));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

  saveAsAct=new QAction(QIcon(":/images/filesaveas.png"), tr("Save &As..."), this);
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

  exitAct=new QAction(QIcon(":/images/exit.png"), tr("&Quit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  parse_Act=new QAction(tr("&User-defined Model"), this);
  parse_Act->setCheckable(true);
  parse_Act->setChecked(true);
  connect(parse_Act, SIGNAL(triggered()), this, SLOT(parse_triggered()));
  connect(parse_Act, SIGNAL(toggled(bool)), this, SLOT(parse_toggled()));

  start_to_prior_Act=new QAction(QIcon(":/images/ps.png"), tr("Set &Priors to Start Values"), this);
  connect(start_to_prior_Act, SIGNAL(triggered()), this, SLOT(start_to_prior()));
  result_to_prior_Act=new QAction(QIcon(":/images/pf.png"), tr("Set Priors to &Fit Results"), this);
  connect(result_to_prior_Act, SIGNAL(triggered()), this, SLOT(result_to_prior()));
  result_to_start_Act=new QAction(QIcon(":/images/sf.png"), tr("Set &Start Values to Fit Results"), this);
  connect(result_to_start_Act, SIGNAL(triggered()), this, SLOT(result_to_start()));

  chisqr_start_Act=new QAction(QIcon(":/images/chisqr.png"), tr("Compute chi^2/dof for Start Values"), this);
  connect(chisqr_start_Act, SIGNAL(triggered()), this, SLOT(chisqr_start()));

  reload_data_file_Act=new QAction(QIcon(":/images/reload.png"), tr("Reload &Data File"), this);
  connect(reload_data_file_Act, SIGNAL(triggered()), this, SLOT(reload_data_file()));

  write_results_Act=new QAction(QIcon(":/images/kget.png"), tr("Write Fit Results and &Covariance to Files"), this);
  connect(write_results_Act, SIGNAL(triggered()), this, SLOT(write_results()));

  aboutAct=new QAction(QIcon(":/images/qmbf.png"), tr("&About"), this);
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutWindow()));
}







// *******************************************************************
// this function creates the menu and toolbar
// *******************************************************************
void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAct);
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
  fileMenu->addAction(saveAsAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  modelMenu = menuBar()->addMenu(tr("&Model"));
  modelMenu->addAction(parse_Act);

  toolsMenu = menuBar()->addMenu(tr("&Tools"));
  toolsMenu->addAction(start_to_prior_Act);
  toolsMenu->addAction(result_to_prior_Act);
  toolsMenu->addAction(result_to_start_Act);
  toolsMenu->addSeparator();
  toolsMenu->addAction(chisqr_start_Act);
  toolsMenu->addSeparator();
  toolsMenu->addAction(reload_data_file_Act);
  toolsMenu->addSeparator();
  toolsMenu->addAction(write_results_Act);

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);

  setIconSize(QSize(22,22));

  fileToolBar = addToolBar(tr("File"));
  fileToolBar->addAction(newAct);
  fileToolBar->addAction(openAct);
  fileToolBar->addAction(saveAct);
  fileToolBar->addAction(saveAsAct);

  toolsToolBar = addToolBar(tr("Tools"));
  toolsToolBar->addAction(start_to_prior_Act);
  toolsToolBar->addAction(result_to_prior_Act);
  toolsToolBar->addAction(result_to_start_Act);
  toolsToolBar->addSeparator();
  toolsToolBar->addAction(chisqr_start_Act);
  toolsToolBar->addSeparator();
  toolsToolBar->addAction(reload_data_file_Act);
  toolsToolBar->addSeparator();
  toolsToolBar->addAction(write_results_Act);
}



// *******************************************************************
// this function creates widgets and dialogs
// *******************************************************************
void MainWindow::create_Widgets_Dialogs()
{
  mainWidget=new MainWidget(start_bayes, start_n_parameters, max_n_parameters);
  connect(mainWidget, SIGNAL(modified()), this, SLOT(modified()));
  connect(mainWidget, SIGNAL(data_file_modified()), this, SLOT(data_file_modified()));
  connect(mainWidget, SIGNAL(file_type_modified(int)), this, SLOT(file_type_modified(int)));
  connect(mainWidget, SIGNAL(start_fit()), this, SLOT(fit()));

  connect(this, SIGNAL(lf(QString)), this, SLOT(loadFile(const QString&)));

  tabWidget = new QTabWidget();
  tabWidget -> addTab(mainWidget, tr("Main"));

  funcWidget=new FunctionWidget(start_n_functions, max_n_functions);
  connect(funcWidget, SIGNAL(modified()), this, SLOT(modified()));
  connect(funcWidget, SIGNAL(n_functions_modified(int)), this, SLOT(n_functions_modified(int)));
  connect(funcWidget, SIGNAL(function_changed(int, std::string)), this, SLOT(function_changed()));
  connect(funcWidget, SIGNAL(function_active_changed(int, bool)), this, SLOT(function_changed()));
  tabWidget -> addTab(funcWidget, tr("Model Functions"));

  parderWidget=new ParamDerivsWidget(start_num_diff, start_n_functions, max_n_functions, start_n_parameters, max_n_parameters);
  connect(parderWidget, SIGNAL(modified()), this, SLOT(modified()));
  connect(parderWidget, SIGNAL(n_parameters_modified(int)), this, SLOT(n_parameters_modified(int)));
  connect(parderWidget, SIGNAL(n_functions_modified(int)), this, SLOT(n_functions_modified(int)));
  connect(parderWidget, SIGNAL(parameter_changed(int, std::string)), this, SLOT(parameter_changed(int, std::string)));
  connect(parderWidget, SIGNAL(derivative_changed(int, int, std::string)), this, SLOT(derivative_changed()));
  tabWidget -> addTab(parderWidget, tr("Parameters / Derivatives"));

  constsWidget=new ConstantsWidget(start_n_constants, max_n_constants);
  connect(constsWidget, SIGNAL(modified()), this, SLOT(modified()));
  connect(constsWidget, SIGNAL(n_constants_modified(int)), this, SLOT(n_constants_modified()));
  connect(constsWidget, SIGNAL(constant_changed(int, std::string)), this, SLOT(constant_changed()));
  tabWidget -> addTab(constsWidget, tr("Constants"));

  chisqrextraWidget=new ChiSqrExtraTermWidget(start_n_constants, max_n_constants);
  connect(chisqrextraWidget, SIGNAL(modified()), this, SLOT(modified()));
  connect(chisqrextraWidget, SIGNAL(function_modified()), this, SLOT(function_changed()));
  connect(chisqrextraWidget, SIGNAL(enabled_modified()), this, SLOT(chisqrextra_enabled_modified()));
  connect(chisqrextraWidget, SIGNAL(n_constants_modified(int)), this, SLOT(n_constants_modified()));
  connect(chisqrextraWidget, SIGNAL(constant_changed(int, std::string)), this, SLOT(constant_changed()));
  tabWidget -> addTab(chisqrextraWidget, tr("Additional term in chi^2"));

  fitsetWidget=new FitSettingsWidget(start_bayes, start_num_diff, start_second_deriv_minimization, start_second_deriv_covariance, start_num_diff_step, start_startlambda, start_lambdafac, start_tolerance, start_svd, start_svd_ratio, start_svd_value, start_rescale_value, start_steps);
  connect(fitsetWidget, SIGNAL(modified()), this, SLOT(modified()));
  connect(fitsetWidget, SIGNAL(bayesianmodified(int)), this, SLOT(bayesian_modified(int)));
  connect(fitsetWidget, SIGNAL(numdiffmodified(int)), this, SLOT(num_diff_modified(int)));
  connect(fitsetWidget, SIGNAL(svd_modified()), this, SLOT(svd_modified()));
  connect(fitsetWidget, SIGNAL(secondderivminimizationmodified(int)), this, SLOT(second_deriv_minimization_modified(int)));
  connect(fitsetWidget, SIGNAL(secondderivcovariancemodified(int)), this, SLOT(second_deriv_covariance_modified(int)));
  tabWidget -> addTab(fitsetWidget, tr("Fit Settings"));

  fitdialog = new fit_dialog();
  connect(fitdialog, SIGNAL(abort()), this, SLOT(abort_fit()));

  bootstrapdialog = new bootstrap_dialog();
  connect(bootstrapdialog, SIGNAL(abort()), this, SLOT(abort_bootstrap()));

  funcWidget->updateSizes();
  mainWidget->updateSizes();
  constsWidget->updateSizes();
}






void MainWindow::closeEvent(QCloseEvent *event)
{
  if(maybeSave())
  {
    event->accept();
  }
  else
  {
    event->ignore();
  }
}


void MainWindow::newFile()
{
  if(maybeSave())
  {
    reset();
    setCurrentFile("");
  }
}


void MainWindow::open()
{
  if(maybeSave())
  {
    QString fileName=QFileDialog::getOpenFileName(this, tr("Open MBCFN File"),
                                                  current_dir,
                                                  tr("MBCFN files (*.mbcfn);; All files (*)"));
    if(!fileName.isEmpty())
    {
      reset();
      loadFile(fileName);
      current_dir=QFileInfo(fileName).absolutePath();
    }
  }
}



// *******************************************************************
// checks whether the mbf file should be saved
// *******************************************************************
bool MainWindow::maybeSave()
{
  if(something_modified)
  {
    int ret=QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Do you want to save your changes?"),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No, QMessageBox::Cancel
                                 | QMessageBox::Escape);
    if(ret==QMessageBox::Yes)
    {
      return save();
    }
    else if(ret == QMessageBox::Cancel)
    {
      return false;
    }
  }
  return true;
}


bool MainWindow::save()
{
  if(curFile.isEmpty())
  {
    return saveAs();
  }
  else
  {
    return saveFile(curFile);
  }
}


bool MainWindow::saveAs()
{
  QString fileName=QFileDialog::getSaveFileName(this, tr("Save MBCFN File"),
                                                curFile,
                                                tr("MBCFN files (*.mbcfn);; All files (*)"));
  if(fileName.isEmpty())
  {
    return false;
  }
  current_dir=QFileInfo(fileName).absolutePath();
  return saveFile(fileName);
}


// *******************************************************************
// *******************************************************************
#include "mbf_load_save.cpp"
// *******************************************************************
// *******************************************************************



// *******************************************************************
// this function updates the mbf file name
// *******************************************************************
void MainWindow::setCurrentFile(const QString &fileName)
{
  curFile=fileName;

  setWindowModified(false);
  something_modified=false;

  QString shownName;
  if(curFile.isEmpty())
  {
    shownName="untitled.mbcfn";
  }
  else
  {
    shownName=strippedName(curFile);
  }
  setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("QMBCFN")));
}


// *******************************************************************
// this function returns the name of the file, excluding the path
// *******************************************************************
QString MainWindow::strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}



// *******************************************************************
// this function shows an "About" window
// *******************************************************************
void MainWindow::aboutWindow()
{
  stringstream version_st;
  version_st.precision(2);
  version_st.setf(ios::fixed);
  version_st << version;
  QMessageBox::about(this, tr("About QMBCFN"),
                           tr("\nQMBCFN\n"
                              "Version %1 \n\n"
                              "by Stefan Meinel").arg(version_st.str().c_str()));
}





void MainWindow::parse_triggered()
{
  current_model=PARSE;
  need_to_reset_fitter=true;
  check_current_model();
}


void MainWindow::parse_toggled()
{
  parderWidget->set_user_def(parse_Act->isChecked());
  constsWidget->set_user_def(parse_Act->isChecked());
  funcWidget->set_user_def(parse_Act->isChecked());
}





void MainWindow::update_model_widgets()
{
  mainWidget->save_tables();

  funcWidget->reset();
  constsWidget->reset();
  parderWidget->reset();
  mainWidget->reset_tables();

  int n_parameters=_model->get_n_parameters();
  parderWidget->set_n_parameters(n_parameters);
  for(int p=0; p<n_parameters; ++p)
  {
    parderWidget->set_parameter(p, _model->get_parameter_name(p));
  }
  int n_functions=_model->get_n_fit_points();
  funcWidget->set_n_functions(n_functions);
  for(int f=0; f<n_functions; ++f)
  {
    funcWidget->set_function(f, _model->get_function_name(f));
  }
  int n_constants=_model->get_n_constants();
  constsWidget->set_n_constants(n_constants);
  for(int c=0; c<n_constants; ++c)
  {
    constsWidget->set_constant_name(c, _model->get_constant_name(c));
  }

  mainWidget->restore_tables();
}


void MainWindow::modified()
{
  something_modified=true;
  setWindowModified(true);
}

void MainWindow::data_file_modified()
{
  need_to_load_data_file=true;
}

void MainWindow::file_type_modified(int newtype)
{
  if(newtype!=0)
  {
    result_to_start_Act->setEnabled(false);
    result_to_prior_Act->setEnabled(false);
    chisqr_start_Act->setEnabled(false);
    write_results_Act->setEnabled(false);
  }
  else
  {
    result_to_start_Act->setEnabled(true);
    result_to_prior_Act->setEnabled(true);
    chisqr_start_Act->setEnabled(true);
    write_results_Act->setEnabled(true);
  }
}

void MainWindow::bayesian_modified(int newState)
{
  need_to_fit=true;
  if(newState==2)
  {
    mainWidget->set_bayesian(true);
    parderWidget->set_n_parameters_dof(0);
  }
  if(newState==0)
  {
    mainWidget->set_bayesian(false);
    parderWidget->set_n_parameters_dof(parderWidget->get_n_parameters());
  }
}


void MainWindow::chisqrextra_enabled_modified()
{
  need_to_reset_fitter=true;
  need_to_fit=true;
}


void MainWindow::num_diff_modified(int newState)
{
  need_to_reset_fitter=true;
  need_to_fit=true;
  if(newState==2)
  {
    parderWidget->set_num_diff(true);
  }
  if(newState==0)
  {
    parderWidget->set_num_diff(false);
  }
}


void MainWindow::second_deriv_minimization_modified(int newState)
{
  need_to_fit=true;
}

void MainWindow::second_deriv_covariance_modified(int newState)
{
  need_to_fit=true;
}

void MainWindow::n_functions_modified(int new_n_functions)
{
  parderWidget->set_n_functions(new_n_functions);
  funcWidget->set_n_functions(new_n_functions);
  need_to_reset_fitter=true;
  need_to_set_fit_data=true;
  need_to_fit=true;
}

void MainWindow::n_parameters_modified(int new_n_parameters)
{
  parderWidget->set_n_parameters(new_n_parameters);
  if(!(fitsetWidget->get_bayes()))
  {
    parderWidget->set_n_parameters_dof(new_n_parameters);
  }
  mainWidget->set_n_parameters(new_n_parameters);
  need_to_reset_fitter=true;
  need_to_fit=true;
}

void MainWindow::n_constants_modified()
{
  need_to_reset_fitter=true;
  need_to_fit=true;
}

void MainWindow::parameter_changed(int p, std::string new_name)
{
  mainWidget->set_parameter(p, new_name);
  need_to_reset_fitter=true;
  need_to_fit=true;
}

void MainWindow::derivative_changed()
{
  need_to_reset_fitter=true;
  need_to_fit=true;
}

void MainWindow::function_changed()
{
  need_to_reset_fitter=true;
  need_to_fit=true;
  need_to_set_fit_data=true;
}

void MainWindow::constant_changed()
{
  need_to_reset_fitter=true;
  need_to_fit=true;
}

void MainWindow::svd_modified()
{
  need_to_set_fit_data=true;
  need_to_fit=true;
}


// *******************************************************************
// makes fitter ready for data assignment and fitting
// *******************************************************************
bool MainWindow::prepare_fit()
{
  if(need_to_reset_fitter)
  {
    if(!reset_fitter())
    {
      return false;
    }
  }

  _gaussian_prior->set_enabled(fitsetWidget->get_bayes());

  _fitter->set_num_diff(fitsetWidget->get_num_diff());
  _fitter->set_second_deriv_minimization(fitsetWidget->get_second_deriv_minimization());
  _fitter->set_second_deriv_covariance(fitsetWidget->get_second_deriv_covariance());


  if( (fitsetWidget->get_num_diff()) || (fitsetWidget->get_second_deriv_minimization()) || (fitsetWidget->get_second_deriv_covariance()) )
  {
    double num_diff_step=fitsetWidget->get_num_diff_step();
    if(num_diff_step==empty_double)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: Numerical differentiation step size missing"));
      return false;
    }
    if(num_diff_step==0)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: Numerical differentiation step size set to 0"));
      return false;
    }
    _fitter->set_num_diff_step(num_diff_step);
  }

  double start_lambda=fitsetWidget->get_startlambda();
  if(start_lambda==empty_double)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: start lambda missing"));
    return false;
  }
  if(start_lambda==0)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: Start lambda set to 0"));
    return false;
  }
  _fitter->set_initial_lambda(start_lambda);

  double lambdafac=fitsetWidget->get_lambdafac();
  if(lambdafac==empty_double)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: lambda factor missing"));
    return false;
  }
  if(lambdafac==0)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: Lambda factor set to 0"));
    return false;
  }
  _fitter->set_lambda_factor(lambdafac);

  double tolerance=fitsetWidget->get_tolerance();
  if(tolerance==empty_double)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: delta(chi^2) tolerance missing"));
    return false;
  }
  if(tolerance==0)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: delta(chi^2) tolerance set to 0"));
    return false;
  }
  _fitter->set_tolerance(tolerance);

  inversion_method inv_method=fitsetWidget->get_inversion_method();
  _fitter->set_inversion_method(inv_method);

  if(inv_method==simple_cut)
  {
    int svd_cut=fitsetWidget->get_svd();
    if(svd_cut==empty_int)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: SVD cut missing"));
      return false;
    }
    _fitter->set_svd_cut(fitsetWidget->get_svd());
  }

  if(inv_method==ratio_cut)
  {
    double svd_cut_ratio=fitsetWidget->get_svd_ratio();
    if(svd_cut_ratio==empty_double)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: SVD EV ratio cut missing"));
      return false;
    }
    _fitter->set_svd_cut_ratio(svd_cut_ratio);
  }

  if(inv_method==absolute_cut)
  {
    double absolute_cut=fitsetWidget->get_svd_value();
    if(absolute_cut==empty_double)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: SVD cut missing"));
      return false;
    }
    _fitter->set_svd_cut_absolute(fitsetWidget->get_svd_value());
  }

  if(inv_method==off_diagonal_rescale)
  {
    double rescale_factor=fitsetWidget->get_rescale_value();
    if(rescale_factor==empty_double)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: rescale value missing"));
      return false;
    }
    _fitter->set_off_diagonal_rescale_factor(fitsetWidget->get_rescale_value());
  }

  vector< double > constants(constsWidget->get_n_constants(), 0.0);
  for(int c=0; c<constsWidget->get_n_constants(); ++c)
  {
    double val=constsWidget->get_constant_value(c);
    if(val==empty_double)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: constant value missing"));
      return false;
    }
    constants[c]=val;
  }
  _model->set_constants(constants);


  for(int p=0; p<parderWidget->get_n_parameters(); ++p)
  {
    double val=mainWidget->get_start_value(p);
    if(val==empty_double)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: start value missing"));
      return false;
    }
    _fitter->set_starting_value(p, val);
  }

  if(fitsetWidget->get_bayes())
  {
    for(int p=0; p<parderWidget->get_n_parameters(); ++p)
    {
      double val=mainWidget->get_prior(p);
      if(val==empty_double)
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: prior value missing"));
        return false;
      }
      _gaussian_prior->set_prior(p, val);
    }
    for(int p=0; p<parderWidget->get_n_parameters(); ++p)
    {
      double val=mainWidget->get_sigma(p);
      if(val==empty_double)
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: prior sigma missing"));
        return false;
      }
      if(val<=0.0)
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: prior sigma not positive"));
        return false;
      }
      _gaussian_prior->set_sigma(p, val);
    }
  }

  _chisqr_extra_term->set_enabled(chisqrextraWidget->get_enabled());
  if(chisqrextraWidget->get_enabled())
  {
    double num_diff_step=chisqrextraWidget->get_num_diff_step();
    if(num_diff_step==empty_double)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: additional chi^2 term: numerical differentiation step size missing"));
      return false;
    }
    if(num_diff_step==0)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: additional chi^2 term: numerical differentiation step size set to 0"));
      return false;
    }
    _chisqr_extra_term->set_num_diff_step(num_diff_step);
    vector< double > chisqr_extra_term_constants(chisqrextraWidget->get_n_constants(), 0.0);
    for(int c=0; c<chisqrextraWidget->get_n_constants(); ++c)
    {
      double val=chisqrextraWidget->get_constant_value(c);
      if(val==empty_double)
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: additional chi^2 term: constant value missing"));
        return false;
      }
      chisqr_extra_term_constants[c]=val;
    }
    _chisqr_extra_term->set_constants(chisqr_extra_term_constants);

    vector< double > startvals(parderWidget->get_n_parameters());
    for(int p=0; p<parderWidget->get_n_parameters(); ++p)
    {
      startvals[p]=mainWidget->get_start_value(p);
    }
    _chisqr_extra_term->set_parameters(startvals);
    _chisqr_extra_term->chi_sqr();

    if(_chisqr_extra_term->no_of_errors()!=0)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: Error while evaluating chi^2 extra term function"));
      return false;
    }
  }


  if(need_to_load_data_file)
  {
    if(!load_data_file())
    {
      return false;
    }
  }
  if(need_to_set_fit_data)
  {
    if(!set_fit_data())
    {
      return false;
    }
  }

  if(!(_fitter->test_model()))
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error in fit model"));
    return false;
  }

  return true;
}






// *******************************************************************
// start the simple fit
// *******************************************************************
void MainWindow::fit()
{
  if(!prepare_fit())
  {
    return;
  }

  int steps=fitsetWidget->get_steps();
  if(steps==empty_int)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: Maximum number of iterations missing"));
    return;
  }

  if(mainWidget->get_data_file_type()==0)
  {
    stringstream message;
    message << "Fit started" << endl;
    mainWidget->print(message.str());

    _fitter -> stop = false;

    fitthread -> set_max_steps(steps);

    fitdialog -> set_dof(_fitter->get_dof()-_fitter->get_cut()-parderWidget->get_n_parameters_dof());

    fitdialog -> clear();

    fitthread -> start();

    fitdialog -> exec();
  }
  else if( (mainWidget->get_data_file_type()==1) || (mainWidget->get_data_file_type()==2) )
  {
    bootstrapthread -> set_data_file_type(mainWidget->get_data_file_type());

    bootstrapthread -> set_max_steps(steps);

    string dir=mainWidget->get_output_dir();
    if(dir.empty())
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                tr("No output directory selected"));
      return;
    }
    bootstrapthread -> set_output_dir(dir);

    bootstrapdialog -> set_bssamples(multi_fit_data.size());

    stringstream message;
    message << "Multiple fits started" << endl;
    mainWidget->print(message.str());

    _fitter -> stop = false;
    bootstrapthread -> stop = false;

    bootstrapthread -> set_n_parameters(parderWidget->get_n_parameters());

    bootstrapthread -> set_n_parameters_dof(parderWidget->get_n_parameters_dof());

    vector< string > par_names;
    for(int p=0; p<parderWidget -> get_n_parameters(); ++p)
    {
      par_names.push_back(parderWidget -> get_parameter(p));
    }
    bootstrapthread -> set_parameter_names(par_names);

    bootstrapthread -> set_boot_prior( (fitsetWidget -> get_bayes()) && (fitsetWidget -> get_boot_prior()) );

    if(fitsetWidget->get_bayes())
    {
      vector< double > priors;
      vector< double > sigmas;
      for(int p=0; p<parderWidget->get_n_parameters(); ++p)
      {
        priors.push_back(mainWidget->get_prior(p));
        sigmas.push_back(mainWidget->get_sigma(p));
      }
      bootstrapthread -> set_priors(priors);
      bootstrapthread -> set_sigmas(sigmas);
    }

    bootstrapthread -> set_multi_fit_data(multi_fit_data);
    if(mainWidget->get_data_file_type()==2)
    {
      bootstrapthread -> set_multi_fit_data_covariance_matrix(multi_fit_data_covariance_matrix);
    }

    bootstrapthread -> set_file_name(curFile);

    bootstrapthread -> start();

    bootstrapdialog -> update_bar(0);
    bootstrapdialog -> exec();

  }
}





// *******************************************************************
// stops the simple fit thread manually
// *******************************************************************
void MainWindow::abort_fit()
{
  _fitter -> stop = true;
}




// *******************************************************************
// this function is called when the simple fit thread is finished
// *******************************************************************
void MainWindow::fit_finished()
{
  int steps=fitsetWidget->get_steps();
  int steps_needed;
  string fit_message="";

  fitdialog -> close();
  steps_needed=fitthread -> get_steps_needed();
  fit_message=fitthread->get_message();

  if(fit_message!="")
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr(fit_message.c_str()));
  }

  if( (_fitter -> stop) == true)
  {
    mainWidget->clear_results();
    stringstream message;
    message << "Fit aborted"<< endl;
    mainWidget->print(message.str());
  }
  else if(steps_needed==steps+1)
  {
    mainWidget->clear_results();
    stringstream message;
    message << "Fit did not converge after " << steps << " iterations"<< endl;
    mainWidget->print(message.str());
  }
  else
  {
    mainWidget->clear_results();
    double chisqr=_fitter->get_chi_sqr();
    last_chi_sqr=chisqr;
    double dof=_fitter->get_dof()-_fitter->get_cut()-parderWidget->get_n_parameters_dof();
    if(dof<0.0)
    {
      dof=0.0;
    }

    stringstream message;
    message.precision(15);

    message << "Fit converged after " << steps_needed << " iterations." << endl << "chi^2/dof = " << chisqr/dof << "    Q(dof/2,chi^2/2) = " << gsl_sf_gamma_inc_Q(dof/2.0, chisqr/2.0) << endl;

    for(int p=0; p<parderWidget->get_n_parameters(); ++p)
    {
      mainWidget->set_result(p, _fitter->get_parameter(p));
      message << parderWidget->get_parameter(p) << " = " << _fitter->get_parameter(p) << "    sqrt(cov(" << parderWidget->get_parameter(p) << "," << parderWidget->get_parameter(p) << ")) = " << sqrt(_fitter->get_covariance(p, p)) << endl;
      mainWidget->set_sqrt_cov(p, sqrt(_fitter->get_covariance(p, p)));
    }

    if( !(_fitter->covariance_positive_definite()) )
    {
      message << endl << "WARNING: Hessian of chi^2 is not positive definite" << endl;
    }

    mainWidget->print(message.str());
    need_to_fit=false;
  }
}


// *******************************************************************
// stops the bootstrap thread manually
// *******************************************************************
void MainWindow::abort_bootstrap()
{
  _fitter -> stop = true;
  bootstrapthread -> stop = true;
}





void MainWindow::bootstrap_finished()
{
  bootstrapdialog -> close();

  if( (bootstrapthread -> get_exit_code())==aborted )
  {
    stringstream message;
    message << "Multiple fits aborted" << endl;
    mainWidget->print(message.str());
    return;
  }

  if( (bootstrapthread -> get_exit_code())==fit_not_converge )
  {
    return;
  }

  if( (bootstrapthread -> get_exit_code())==output_error )
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Cannot write multifit output files"));
    return;
  }
}



// *******************************************************************
// overall reset
// *******************************************************************
void MainWindow::reset()
{
  setCurrentFile("");

  funcWidget->reset();
  constsWidget->reset();
  chisqrextraWidget->reset();
  fitsetWidget->reset();
  parderWidget->reset();
  mainWidget->reset();

  need_to_load_data_file=true;
  need_to_set_fit_data=true;
  need_to_reset_fitter=true;
  need_to_fit=true;
  something_modified=false;
  setWindowModified(false);

  last_chi_sqr=empty_double;

  current_model=PARSE;
  check_current_model();
}



// *******************************************************************
// refresh "checks" in "Model" menu to current model
// *******************************************************************
void MainWindow::check_current_model()
{
  parse_Act->setChecked(false);

  switch(current_model)
  {
    case PARSE:
      parse_Act->setChecked(true);
      break;
    default:
      break;
  }
}






// *******************************************************************
// set priors to start values
// *******************************************************************
void MainWindow::start_to_prior()
{
  for(int p=0; p<parderWidget->get_n_parameters(); ++p)
  {
    double val=mainWidget->get_start_value(p);
    if(val!=empty_double)
    {
      mainWidget->set_prior(p, val);
    }
  }
}






// *******************************************************************
// set start values to fit results
// *******************************************************************
void MainWindow::result_to_start()
{
  for(int p=0; p<parderWidget->get_n_parameters(); ++p)
  {
    double val=mainWidget->get_result(p);
    if(val!=empty_double)
    {
      mainWidget->set_start_value(p, val);
    }
  }
}






// *******************************************************************
// set priors to fit results
// *******************************************************************
void MainWindow::result_to_prior()
{
  for(int p=0; p<parderWidget->get_n_parameters(); ++p)
  {
    double val=mainWidget->get_result(p);
    if(val!=empty_double)
    {
      mainWidget->set_prior(p, val);
    }
  }
}





// *******************************************************************
// display of chi^2
// *******************************************************************
void MainWindow::chisqr_start()
{
  if(!prepare_fit())
  {
    return;
  }
  double chisqr=_fitter->get_chi_sqr_start();
  double dof=_fitter->get_dof()-_fitter->get_cut()-parderWidget->get_n_parameters_dof();
  if(dof<0.0)
  {
    dof=0.0;
  }

  stringstream message;
  message.precision(15);

  message << "Current start values have chi^2/dof = " << chisqr/dof << "    Q(dof/2,chi^2/2) = " << gsl_sf_gamma_inc_Q(dof/2.0, chisqr/2.0) << endl;
  mainWidget->print(message.str());
}


// *******************************************************************
// this function loads the data file; it refreshes the vectors
// "file_data", "file_data_covariance_matrix", "mult_file_data, "multi_file_data_covariance_matrix" (depending on data file type)
// *******************************************************************
bool MainWindow::load_data_file()
{
  need_to_set_fit_data=true;
  need_to_fit=true;

  string filename=mainWidget->get_data_file();

  if(filename.empty())
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: no data file selected"));
    return false;
  }

  ifstream input(filename.c_str());

  if(!input)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: Could not open data file"));
    return false;
  }

  int n_fit_points;
  input >> n_fit_points;

  if(n_fit_points!=funcWidget->get_n_functions())
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: data file has wrong number of fit points"));
    return false;
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);

  map<string, int> function_names_map;
  for(int m=0; m<n_fit_points; ++m)
  {
    string name_st=funcWidget->get_function_name(m);
    if(name_st.length()==0)
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: function name missing"));
      return false;
    }
    if(function_names_map.find(name_st)!=function_names_map.end())
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: function names not unique"));
      return false;
    }
    function_names_map[name_st]=m;
  }

  string name_st;
  map<string, int> file_function_names_map;

  file_data.clear();
  file_data.resize(n_fit_points, 0.0);
  file_data_covariance_matrix.clear();
  file_data_covariance_matrix.resize(n_fit_points, file_data);

  int n_samples;

  if(mainWidget->get_data_file_type()==0)
  {
    for(int m=0; m<n_fit_points; ++m)
    {
      input >> name_st;
      if(file_function_names_map.find(name_st)!=file_function_names_map.end())
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error: data file has repeated function name"));
        return false;
      }
      file_function_names_map[name_st]=m;
      if(function_names_map.find(name_st)==function_names_map.end())
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error: data file has unknown function name"));
        return false;
      }
      input >> file_data[function_names_map[name_st]];
    }
  }
  else if( (mainWidget->get_data_file_type()==1) || (mainWidget->get_data_file_type()==2) )
  {
    input >> n_samples;
    int sample_tmp;

    vector< string > file_function_names_ordered(n_fit_points);

    multi_file_data.clear();
    multi_file_data.resize(n_samples, file_data);

    for(int m=0; m<n_fit_points; ++m)
    {
      input >> sample_tmp >> name_st;
      if(sample_tmp!=1)
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error while reading data file"));
        return false;
      }
      if(file_function_names_map.find(name_st)!=file_function_names_map.end())
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error: data file has repeated function name"));
        return false;
      }
      file_function_names_map[name_st]=m;
      file_function_names_ordered[m]=name_st;
      if(function_names_map.find(name_st)==function_names_map.end())
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error: data file has unknown function name"));
        return false;
      }
      input >> file_data[function_names_map[name_st]];
    }
    multi_file_data[0]=file_data;

    for(int sample=1; sample<n_samples; ++sample)
    {
      for(int m=0; m<n_fit_points; ++m)
      {
        input >> sample_tmp >> name_st;
        if(sample_tmp!=sample+1)
        {
          QApplication::restoreOverrideCursor();
          QMessageBox::warning(this, tr("QMBCFN"),
                                    tr("Error while reading data file"));
          return false;
        }
        if(name_st!=file_function_names_ordered[m])
        {
          QApplication::restoreOverrideCursor();
          QMessageBox::warning(this, tr("QMBCFN"),
                                    tr("Error: incorrect function name in data file"));
          return false;
        }
        input >> file_data[function_names_map[name_st]];
      }
      multi_file_data[sample]=file_data;
    }
  }

  if( (mainWidget->get_data_file_type()==0) || (mainWidget->get_data_file_type()==1) )
  {
    int m1, m2;
    while(!input.eof())
    {
      input >> name_st;
      if(input.eof())
      {
        break;
      }
      if(function_names_map.find(name_st)==function_names_map.end())
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error: data file has unknown function name"));
        return false;
      }
      m1=function_names_map[name_st];
      input >> name_st;
      if(input.eof())
      {
        break;
      }
      if(function_names_map.find(name_st)==function_names_map.end())
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error: data file has unknown function name"));
        return false;
      }
      m2=function_names_map[name_st];
      input >> file_data_covariance_matrix[m1][m2];
      if(input.eof())
      {
        break;
      }
      file_data_covariance_matrix[m2][m1]=file_data_covariance_matrix[m1][m2];
    }
  }
  else if( mainWidget->get_data_file_type()==2 )
  {
    multi_file_data_covariance_matrix.clear();
    multi_file_data_covariance_matrix.resize(n_samples, file_data_covariance_matrix);

    int m1, m2;
    int sample;
    while(!input.eof())
    {
      input >> sample >> name_st;
      if(input.eof())
      {
        break;
      }
      --sample;
      if( (sample<0) || (sample > n_samples-1) )
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error while reading data file"));
        return false;
      }
      if(function_names_map.find(name_st)==function_names_map.end())
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error: data file has unknown function name"));
        return false;
      }
      m1=function_names_map[name_st];
      input >> name_st;
      if(input.eof())
      {
        break;
      }
      if(function_names_map.find(name_st)==function_names_map.end())
      {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(this, tr("QMBCFN"),
                                  tr("Error: data file has unknown function name"));
        return false;
      }
      m2=function_names_map[name_st];
      input >> multi_file_data_covariance_matrix[sample][m1][m2];
      if(input.eof())
      {
        break;
      }
      multi_file_data_covariance_matrix[sample][m2][m1]=multi_file_data_covariance_matrix[sample][m1][m2];
    }
  }


  input.close();
  QApplication::restoreOverrideCursor();

  need_to_load_data_file=false;

  return true;
}


// *******************************************************************
// reloading of data file
// *******************************************************************
void MainWindow::reload_data_file()
{
  need_to_set_fit_data=true;
  need_to_fit=true;
  load_data_file();
}


void MainWindow::write_results()
{
  string dir=mainWidget->get_output_dir();
  if(dir.empty())
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("No output directory selected"));
    return;
  }
  if(*(dir.end()-1)!='/')
  {
    dir.append("/");
  }

  if(need_to_fit)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Fit data first"));
    return;
  }
// write covariance to file
  string cov_filename;
  if(curFile.isEmpty())
  {
    cov_filename=dir+"untitled.mbcfn_covariance.dat";
  }
  else
  {
    cov_filename=dir+strippedName(curFile).toStdString()+"_covariance.dat";
  }
  ofstream cov_output(cov_filename.c_str());
  if(!cov_output)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                                tr("Error: could not write covariance file."));
    return;
  }
  cov_output.precision(14);
  cov_output.setf(ios::left);
  int max_length=1;
  for(int p1=0; p1<parderWidget->get_n_parameters(); ++p1)
  {
    if(parderWidget->get_parameter(p1).length()>max_length)
    {
      max_length=parderWidget->get_parameter(p1).length();
    }
  }
  for(int p1=0; p1<parderWidget->get_n_parameters(); ++p1)
  {
    for(int p2=0; p2<parderWidget->get_n_parameters(); ++p2)
    {
      cov_output << setw(max_length+4) << parderWidget->get_parameter(p1) << setw(max_length+4) << parderWidget->get_parameter(p2) << setw(0) << _fitter->get_covariance(p1, p2) << endl;
    }
  }
  cov_output.close();

// write results to file
  string results_filename;
  if(curFile.isEmpty())
  {
    results_filename=dir+"untitled.mbcfn_results.dat";
  }
  else
  {
    results_filename=dir+strippedName(curFile).toStdString()+"_results.dat";
  }
  ofstream results_output(results_filename.c_str());
  if(!results_output)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                                tr("Error: could not write results file."));
    return;
  }
  results_output.precision(14);
  results_output.setf(ios::left);
  max_length=1;
  for(int p1=0; p1<parderWidget->get_n_parameters(); ++p1)
  {
    if(parderWidget->get_parameter(p1).length()>max_length)
    {
      max_length=parderWidget->get_parameter(p1).length();
    }
  }
  for(int p1=0; p1<parderWidget->get_n_parameters(); ++p1)
  {
    results_output << setw(max_length+4) << parderWidget->get_parameter(p1) << setw(0) << _fitter->get_parameter(p1) << endl;
  }
  results_output.close();
}



// ***************************************************************************
// this function makes the vectors "fit_data" and "fit_data_covariance_matrix"
// and gives them to the fitter
// ***************************************************************************
bool MainWindow::set_fit_data()
{
  need_to_fit=true;

  vector<int> active_functions;

  for(int f=0; f<funcWidget->get_n_functions(); ++f)
  {
    if(funcWidget->get_function_active(f))
    {
      active_functions.push_back(f);
    }
  }

  int fit_n_functions=active_functions.size();

  if(fit_n_functions==0)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Error: no active functions"));
    return false;
  }

  vector< double > fit_data(fit_n_functions, 0.0);
  vector< vector< double > > fit_data_covariance_matrix(fit_n_functions, fit_data);

  if( mainWidget->get_data_file_type()==0 )
  {
    for(int m1=0; m1<fit_n_functions; ++m1)
    {
      fit_data[m1]=file_data[active_functions[m1]];
    }
  }
  else if( (mainWidget->get_data_file_type()==1) || (mainWidget->get_data_file_type()==2) )
  {
    multi_fit_data.clear();
    multi_fit_data.resize(multi_file_data.size(), fit_data);
    for(int sample=0; sample<multi_file_data.size(); ++sample)
    {
      for(int m1=0; m1<fit_n_functions; ++m1)
      {
        multi_fit_data[sample][m1]=multi_file_data[sample][active_functions[m1]];
      }
    }
  }

  if( (mainWidget->get_data_file_type()==0) || (mainWidget->get_data_file_type()==1) )
  {
    for(int m1=0; m1<fit_n_functions; ++m1)
    {
      for(int m2=0; m2<fit_n_functions; ++m2)
      {
        fit_data_covariance_matrix[m1][m2]=file_data_covariance_matrix[active_functions[m1]][active_functions[m2]];
      }
    }
  }
  else if( mainWidget->get_data_file_type()==2 )
  {
    multi_fit_data_covariance_matrix.clear();
    multi_fit_data_covariance_matrix.resize(multi_file_data.size(), fit_data_covariance_matrix);
    for(int sample=0; sample<multi_file_data.size(); ++sample)
    {
      for(int m1=0; m1<fit_n_functions; ++m1)
      {
        for(int m2=0; m2<fit_n_functions; ++m2)
        {
          multi_fit_data_covariance_matrix[sample][m1][m2]=multi_file_data_covariance_matrix[sample][active_functions[m1]][active_functions[m2]];
        }
      }
    }
  }


  string set_data_message;
  bool status=true;
  if(mainWidget->get_data_file_type()==0)
  {
    status = _fitter->set_data(fit_data, fit_data_covariance_matrix, set_data_message);
  }
  else if(mainWidget->get_data_file_type()==1)
  {
    status = _fitter->set_data(multi_fit_data[0], fit_data_covariance_matrix, set_data_message);
  }
  else if(mainWidget->get_data_file_type()==2)
  {
    status = _fitter->set_data(multi_fit_data[0], multi_fit_data_covariance_matrix[0], set_data_message);
  }

  if(set_data_message!="")
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr(set_data_message.c_str()));
  }

  if(status!=true)
  {
    QMessageBox::warning(this, tr("QMBCFN"),
                               tr("Failed to invert data covariance matrix"));
    return false;
  }

  int dof=_fitter->get_dof();
  int cut=_fitter->get_cut();

  stringstream message;

  message << "(Pseudo-)Inversion method for data covariance matrix: ";
  switch(fitsetWidget->get_inversion_method())
  {
    case LU_inversion:
      message << "LU decomposition" << endl;
      break;
    case simple_cut:
      message << "SVD with fixed cut" << endl;
      message << "Fixed SVD cut: " << fitsetWidget->get_svd() << endl;
      break;
    case ratio_cut:
      message << "SVD with EV ratio cut" << endl;
      message << "SVD EV ratio cut: " << fitsetWidget->get_svd_ratio() << endl;
      break;
    case absolute_cut:
      message << "SVD with EV value cut" << endl;
      message << "SVD EV value cut: " << fitsetWidget->get_svd_value() << endl << endl;
      break;
    case diagonal:
      message << "Diagonal only (uncorrelated fit)" << endl;
      break;
    case no_inversion:
      message << "No inversion (data file contains inverse)" << endl;
      break;
    case off_diagonal_rescale:
      message << "Rescale off-diagonal elements" << endl;
      message << "Rescale factor: " << fitsetWidget->get_rescale_value() << endl;
      break;
    default:
      break;
  }

  if(((fitsetWidget->get_inversion_method())!=diagonal)&&((fitsetWidget->get_inversion_method())!=no_inversion))
  {
    message << "Kept " << dof-cut << " out of " << dof << " eigenvalues." << endl;
  }
  else
  {
    message << "Degrees of freedom: " << dof << "." << endl;
  }

  mainWidget->print(message.str());

  need_to_set_fit_data=false;

  return true;
}



// *******************************************************************
// this function recreates "_fitter" with a new model and prior
// *******************************************************************
bool MainWindow::reset_fitter()
{
  need_to_load_data_file=true;
  need_to_set_fit_data=true;
  need_to_reset_fitter=true;
  need_to_fit=true;

  if(false)
  {
    ;
  }
  else // use PARSE
  {
    vector< string > functions;
    vector< string > parameters(parderWidget->get_n_parameters());
    vector< string > constants(constsWidget->get_n_constants());
    vector< string > der_temp_vec(parderWidget->get_n_parameters());
    vector< vector< string > > derivatives;

    for(int f=0; f<funcWidget->get_n_functions(); ++f)
    {
      if(funcWidget->get_function_active(f))
      {
        string function=funcWidget->get_function(f);
        if(function.empty())
        {
          QMessageBox::warning(this, tr("QMBCFN"),
                                     tr("Error: function missing"));
          return false;
        }
        if( _in("{", function) || _in("&", function) || _in("&", function) || _in("!", function) || _in("}", function)
         || _in("%", function) || _in("?", function) || _in("[", function) || _in(":", function)
         || _in(">", function) || _in("#", function) || _in("~", function) || _in("$", function)
         || _in("]", function) || _in("<", function) )
        {
          stringstream sst;
          sst << "Error: forbidden character in function " << f+1 << endl;
          QMessageBox::warning(this, tr("QMBCFN"),
                                     tr(sst.str().c_str()));
          return false;
        }
        functions.push_back(function);
      }
    }

    for(int p=0; p<parderWidget->get_n_parameters(); ++p)
    {
      if((parderWidget->get_parameter(p)).empty())
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: parameter name missing"));
        return false;
      }
      if(std::find(parameters.begin(), parameters.end(), parderWidget->get_parameter(p))!=parameters.end())
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: parameter name repeated"));
        return false;
      }
      parameters[p]=parderWidget->get_parameter(p);
      if( _in("{", parameters[p]) || _in("&", parameters[p]) || _in("&", parameters[p]) || _in("!", parameters[p]) || _in("}", parameters[p])
       || _in("%", parameters[p]) || _in("?", parameters[p]) || _in("[", parameters[p]) || _in(";", parameters[p]) || _in("|", parameters[p]) || _in("`", parameters[p]) || _in(":", parameters[p])
       || _in(">", parameters[p]) || _in("#", parameters[p]) || _in("~", parameters[p]) || _in("$", parameters[p])
       || _in("]", parameters[p]) || _in("<", parameters[p]) ||
                     _in("arcsin", parameters[p]) ||  _in("sinh", parameters[p]) ||
                     _in("sin", parameters[p]) ||  _in("arccos", parameters[p]) ||
                     _in("cosh", parameters[p]) ||  _in("cos", parameters[p]) ||
                     _in("arctan", parameters[p]) || _in("arccot", parameters[p]) || _in("arctanp", parameters[p]) || _in("arccotp", parameters[p]) ||  _in("tanh", parameters[p]) ||
                     _in("tan", parameters[p]) ||  _in("exp", parameters[p]) ||
                     _in("log", parameters[p]) ||  _in("sqrt", parameters[p]) ||
                     _in("sqr", parameters[p]) ||  _in("alt", parameters[p]) )
      {
        stringstream sst;
        sst << "Error: forbidden character or substring in parameter " << p+1 << endl;
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr(sst.str().c_str()));
        return false;
      }
    }

    for(int c=0; c<constsWidget->get_n_constants(); ++c)
    {
      if((constsWidget->get_constant_name(c)).empty())
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: constant name missing"));
        return false;
      }
      if(std::find(constants.begin(), constants.end(), constsWidget->get_constant_name(c))!=constants.end())
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: constant name repeated"));
        return false;
      }
      constants[c]=constsWidget->get_constant_name(c);
      if( _in("{", constants[c]) || _in("&", constants[c]) || _in("&", constants[c]) || _in("!", constants[c]) || _in("}", constants[c])
       || _in("%", constants[c]) || _in("?", constants[c]) || _in("[", constants[c]) || _in(";", constants[c]) || _in("|", constants[c]) || _in("`", constants[c]) || _in(":", constants[c])
       || _in(">", constants[c]) || _in("#", constants[c]) || _in("~", constants[c]) || _in("$", constants[c])
       || _in("]", constants[c]) || _in("<", constants[c]) ||
                     _in("arcsin", constants[c]) ||  _in("sinh", constants[c]) ||
                     _in("sin", constants[c]) ||  _in("arccos", constants[c]) ||
                     _in("cosh", constants[c]) ||  _in("cos", constants[c]) ||
                     _in("arctan", constants[c]) || _in("arccot", constants[c]) || _in("arctanp", constants[c]) || _in("arccotp", constants[c]) ||   _in("tanh", constants[c]) ||
                     _in("tan", constants[c]) ||  _in("exp", constants[c]) ||
                     _in("log", constants[c]) ||  _in("sqrt", constants[c]) ||
                     _in("sqr", constants[c]) ||  _in("alt", constants[c]) )
      {
        stringstream sst;
        sst << "Error: forbidden character or substring in constant " << c+1 << endl;
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr(sst.str().c_str()));
        return false;
      }
    }

    if(!(fitsetWidget->get_num_diff()))
    {
      for(int f=0; f<funcWidget->get_n_functions(); ++f)
      {
        if(funcWidget->get_function_active(f))
        {
          for(int p=0; p<parderWidget->get_n_parameters(); ++p)
          {
            der_temp_vec[p]=parderWidget->get_derivative(f, p);
            if(der_temp_vec[p].empty())
            {
              stringstream sst;
              sst << "Error: derivative" << endl;
              sst << "of function " << f+1 << endl;
              sst << "by parameter " << p+1 << endl;
              sst << "missing" << endl;

              QMessageBox::warning(this, tr("QMBCFN"),
                                         tr(sst.str().c_str()));
              return false;
            }
            if( _in("{", der_temp_vec[p]) || _in("&", der_temp_vec[p]) || _in("&", der_temp_vec[p]) || _in("!", der_temp_vec[p]) || _in("}", der_temp_vec[p])
             || _in("%", der_temp_vec[p]) || _in("?", der_temp_vec[p]) || _in("[", der_temp_vec[p]) || _in(":", der_temp_vec[p])
             || _in(">", der_temp_vec[p]) || _in("#", der_temp_vec[p]) || _in("~", der_temp_vec[p]) || _in("$", der_temp_vec[p])
             || _in("]", der_temp_vec[p]) || _in("<", der_temp_vec[p]) )
            {
              stringstream sst;
              sst << "Error: forbidden character in derivative" << endl;
              sst << "of function " << f+1 << endl;
              sst << "by parameter " << p+1 << endl;

              QMessageBox::warning(this, tr("QMBCFN"),
                                         tr(sst.str().c_str()));
              return false;
            }
          }
          derivatives.push_back(der_temp_vec);
        }
      }
    }
    else
    {
      derivatives.resize(functions.size());
    }

    if(functions.size()==0)
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: no active function"));
      return false;
    }

    delete _model;
    _model=new parse_model(functions, parameters, constants, derivatives);

  }

  delete _gaussian_prior;
  _gaussian_prior=new gaussian_prior(_model->get_n_parameters());


  string chisqr_extra_term_function="";
  vector< string > chisqr_extra_term_constant_names;
  double chisqr_extra_term_num_diff_step=1e-08;
  if(chisqrextraWidget->get_enabled())
  {
    chisqr_extra_term_function=chisqrextraWidget->get_function();
    if(chisqr_extra_term_function.empty())
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: chi^2 extra term function missing"));
      return false;
    }
    if( _in("{", chisqr_extra_term_function) || _in("&", chisqr_extra_term_function) || _in("&", chisqr_extra_term_function) || _in("!", chisqr_extra_term_function) || _in("}", chisqr_extra_term_function)
      || _in("%", chisqr_extra_term_function) || _in("?", chisqr_extra_term_function) || _in("[", chisqr_extra_term_function) || _in(":", chisqr_extra_term_function)
      || _in(">", chisqr_extra_term_function) || _in("#", chisqr_extra_term_function) || _in("~", chisqr_extra_term_function) || _in("$", chisqr_extra_term_function)
      || _in("]", chisqr_extra_term_function) || _in("<", chisqr_extra_term_function) )
    {
      QMessageBox::warning(this, tr("QMBCFN"),
                                 tr("Error: forbidden character in chi^2 extra term function"));
      return false;
    }
    chisqr_extra_term_num_diff_step=chisqrextraWidget->get_num_diff_step();
    chisqr_extra_term_constant_names.resize(chisqrextraWidget->get_n_constants(), "");
    for(int c=0; c<chisqrextraWidget->get_n_constants(); ++c)
    {
      if((chisqrextraWidget->get_constant_name(c)).empty())
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: chi^2 extra term: constant name missing"));
        return false;
      }
      if(std::find(chisqr_extra_term_constant_names.begin(), chisqr_extra_term_constant_names.end(), chisqrextraWidget->get_constant_name(c))!=chisqr_extra_term_constant_names.end())
      {
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Error: chi^2 extra term: constant name repeated"));
        return false;
      }
      chisqr_extra_term_constant_names[c]=chisqrextraWidget->get_constant_name(c);
      if( _in("{", chisqr_extra_term_constant_names[c]) || _in("&", chisqr_extra_term_constant_names[c]) || _in("&", chisqr_extra_term_constant_names[c]) || _in("!", chisqr_extra_term_constant_names[c]) || _in("}", chisqr_extra_term_constant_names[c])
       || _in("%", chisqr_extra_term_constant_names[c]) || _in("?", chisqr_extra_term_constant_names[c]) || _in("[", chisqr_extra_term_constant_names[c]) || _in(";", chisqr_extra_term_constant_names[c]) || _in("|", chisqr_extra_term_constant_names[c]) || _in("`", chisqr_extra_term_constant_names[c]) || _in(":", chisqr_extra_term_constant_names[c])
       || _in(">", chisqr_extra_term_constant_names[c]) || _in("#", chisqr_extra_term_constant_names[c]) || _in("~", chisqr_extra_term_constant_names[c]) || _in("$", chisqr_extra_term_constant_names[c])
       || _in("]", chisqr_extra_term_constant_names[c]) || _in("<", chisqr_extra_term_constant_names[c]) ||
                     _in("arcsin", chisqr_extra_term_constant_names[c]) ||  _in("sinh", chisqr_extra_term_constant_names[c]) ||
                     _in("sin", chisqr_extra_term_constant_names[c]) ||  _in("arccos", chisqr_extra_term_constant_names[c]) ||
                     _in("cosh", chisqr_extra_term_constant_names[c]) ||  _in("cos", chisqr_extra_term_constant_names[c]) ||
                     _in("arctan", chisqr_extra_term_constant_names[c]) || _in("arccot", chisqr_extra_term_constant_names[c]) || _in("arctanp", chisqr_extra_term_constant_names[c]) || _in("arccotp", chisqr_extra_term_constant_names[c]) ||  _in("tanh", chisqr_extra_term_constant_names[c]) ||
                     _in("tan", chisqr_extra_term_constant_names[c]) ||  _in("exp", chisqr_extra_term_constant_names[c]) ||
                     _in("log", chisqr_extra_term_constant_names[c]) ||  _in("sqrt", chisqr_extra_term_constant_names[c]) ||
                     _in("sqr", chisqr_extra_term_constant_names[c]) ||  _in("alt", chisqr_extra_term_constant_names[c]) )
      {
        stringstream sst;
        sst << "Error: forbidden character or substring in chi^2 extra term constant " << c+1 << endl;
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr(sst.str().c_str()));
        return false;
      }
    }
    for(int p=0; p<_model->get_n_parameters(); ++p)
    {
      if(std::find(chisqr_extra_term_constant_names.begin(), chisqr_extra_term_constant_names.end(), _model->get_parameter_name(p))!=chisqr_extra_term_constant_names.end())
      {
        stringstream sst;
        sst << "Error: constant name \"" << _model->get_parameter_name(p) << "\" in chi^2 extra term function is equal to a fit parameter name " << std::endl << std::endl;
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr(sst.str().c_str()));
        return false;
      }
    }
    for(int c=0; c<_model->get_n_constants(); ++c)
    {
      if(std::find(chisqr_extra_term_constant_names.begin(), chisqr_extra_term_constant_names.end(), _model->get_constant_name(c))!=chisqr_extra_term_constant_names.end())
      {
        stringstream sst;
        sst << "Error: constant name \"" << _model->get_constant_name(c) << "\" in chi^2 extra term function is equal to a fit constant name " << std::endl << std::endl;
        QMessageBox::warning(this, tr("QMBCFN"),
                                   tr(sst.str().c_str()));
        return false;
      }
    }
  }
  delete _chisqr_extra_term;
  _chisqr_extra_term = new chisqr_extra_term(_model->get_n_parameters(), chisqr_extra_term_constant_names.size(), chisqr_extra_term_function);
  _chisqr_extra_term->set_enabled(chisqrextraWidget->get_enabled());
  _chisqr_extra_term->set_num_diff_step(chisqr_extra_term_num_diff_step);
  for(int p=0; p<_model->get_n_parameters(); ++p)
  {
    _chisqr_extra_term->set_parameter_name(p, _model->get_parameter_name(p));
  }
  _chisqr_extra_term->init_parameter_list();
  for(int c=0; c<chisqr_extra_term_constant_names.size(); ++c)
  {
    _chisqr_extra_term->set_constant_name(c, chisqr_extra_term_constant_names[c]);
  }


  delete _fitter;
  _fitter=new fitter(_model, _gaussian_prior, _chisqr_extra_term);
  connect(_fitter, SIGNAL(step_done(int, double, double)), fitdialog, SLOT(step_done(int, double, double)));

  fitthread -> set_fitter(_fitter);
  bootstrapthread -> set_fitter(_fitter);
  bootstrapthread -> set_gaussian_prior(_gaussian_prior);

  need_to_reset_fitter=false;

  return true;
}
