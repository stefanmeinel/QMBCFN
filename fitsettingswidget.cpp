#include "fitsettingswidget.h"

FitSettingsWidget::FitSettingsWidget(bool bayes, bool num_diff, bool second_deriv_minimization, bool second_deriv_covariance, double num_diff_step, double startlambda, double lambdafac, double tolerance, int svd, double svd_ratio, double svd_value, double rescale_value, int steps, QWidget* parent)
  : QWidget(parent)
{
  start_bayes=bayes;
  start_num_diff=num_diff;
  start_second_deriv_minimization=second_deriv_minimization;
  start_second_deriv_covariance=second_deriv_covariance;
  start_num_diff_step=num_diff_step;
  startlambda0=startlambda;
  lambdafac0=lambdafac;
  tolerance0=tolerance;
  svd0=svd;
  svd_ratio0=svd_ratio;
  svd_value0=svd_value;
  rescale_value0=rescale_value;
  steps0=steps;
  createGadgets();
  reset();
}

void FitSettingsWidget::set_bayes(bool bayes)
{
  bayes_cb->setChecked(bayes);
}

bool FitSettingsWidget::get_bayes()
{
  return bayes_cb->isChecked();
}

void FitSettingsWidget::set_boot_prior(bool boot_prior)
{
  boot_prior_cb->setChecked(boot_prior);
}


bool FitSettingsWidget::get_boot_prior()
{
  return boot_prior_cb->isChecked();
}

void FitSettingsWidget::set_num_diff(bool num_diff)
{
  num_diff_cb->setChecked(num_diff);
}

bool FitSettingsWidget::get_num_diff()
{
  return num_diff_cb->isChecked();
}

void FitSettingsWidget::set_second_deriv_minimization(bool second_deriv_minimization)
{
  second_deriv_minimization_cb->setChecked(second_deriv_minimization);
}

bool FitSettingsWidget::get_second_deriv_minimization()
{
  return second_deriv_minimization_cb->isChecked();
}

void FitSettingsWidget::set_second_deriv_covariance(bool second_deriv_covariance)
{
  second_deriv_covariance_cb->setChecked(second_deriv_covariance);
}

bool FitSettingsWidget::get_second_deriv_covariance()
{
  return second_deriv_covariance_cb->isChecked();
}


void FitSettingsWidget::set_num_diff_step(double num_diff_step)
{
  numdiffEd->setText(double_to_QString(num_diff_step));
}


double FitSettingsWidget::get_num_diff_step()
{
  return QString_to_double(numdiffEd->text());
}




void FitSettingsWidget::set_startlambda(double startlambda)
{
  startlambdaEd->setText(double_to_QString(startlambda));
}

double FitSettingsWidget::get_startlambda()
{
  return QString_to_double(startlambdaEd->text());
}

void FitSettingsWidget::set_lambdafac(double lambdafac)
{
  lambdafacEd->setText(double_to_QString(lambdafac));
}

double FitSettingsWidget::get_lambdafac()
{
  return QString_to_double(lambdafacEd->text());
}

void FitSettingsWidget::set_tolerance(double tolerance)
{
  toleranceEd->setText(double_to_QString(tolerance));
}

double FitSettingsWidget::get_tolerance()
{
  return QString_to_double(toleranceEd->text());
}


void FitSettingsWidget::set_inversion_method(inversion_method method)
{
  int index=0;
  switch(method)
  {
    case LU_inversion:
      index=0;
      break;
    case simple_cut:
      index=1;
      break;
    case ratio_cut:
      index=2;
      break;
    case absolute_cut:
      index=3;
      break;
    case diagonal:
      index=4;
      break;
    case no_inversion:
      index=5;
      break;
    case off_diagonal_rescale:
      index=6;
      break;
    default:
      break;
  }
  invMethodCb->setCurrentIndex(index);
}


inversion_method FitSettingsWidget::get_inversion_method()
{
  int index=invMethodCb->currentIndex();
  inversion_method result=LU_inversion;
  switch(index)
  {
    case 0:
      result=LU_inversion;
      break;
    case 1:
      result=simple_cut;
      break;
    case 2:
      result=ratio_cut;
      break;
    case 3:
      result=absolute_cut;
      break;
    case 4:
      result=diagonal;
      break;
    case 5:
      result=no_inversion;
      break;
    case 6:
      result=off_diagonal_rescale;
      break;
    default:
      break;
  }
  return result;
}

void FitSettingsWidget::set_svd(int svd_cut)
{
  svdEd->setText(int_to_QString(svd_cut));
}

int FitSettingsWidget::get_svd()
{
  return QString_to_int(svdEd->text());
}

void FitSettingsWidget::set_svd_ratio(double ratio)
{
  svdRatioEd->setText(double_to_QString(ratio));
}

double FitSettingsWidget::get_svd_ratio()
{
  return QString_to_double(svdRatioEd->text());
}

void FitSettingsWidget::set_svd_value(double value)
{
  svdValueEd->setText(double_to_QString(value));
}

double FitSettingsWidget::get_svd_value()
{
  return QString_to_double(svdValueEd->text());
}

void FitSettingsWidget::set_rescale_value(double value)
{
  rescaleValueEd->setText(double_to_QString(value));
}

double FitSettingsWidget::get_rescale_value()
{
  return QString_to_double(rescaleValueEd->text());
}

void FitSettingsWidget::set_steps(int steps)
{
  stepsEd->setText(int_to_QString(steps));
}

int FitSettingsWidget::get_steps()
{
  return QString_to_int(stepsEd->text());
}



void FitSettingsWidget::reset()
{
  bayes_cb->setChecked(start_bayes);
  boot_prior_cb->setChecked(start_bayes);
  boot_prior_cb->setEnabled(start_bayes);
  num_diff_cb->setChecked(start_num_diff);
  second_deriv_minimization_cb->setChecked(start_second_deriv_minimization);
  second_deriv_covariance_cb->setChecked(start_second_deriv_covariance);
  numdiffEd->setText(double_to_QString(start_num_diff_step));
  if(start_num_diff)
  {
    numdiffmodified_slot(Qt::Checked);
  }
  else
  {
    numdiffmodified_slot(Qt::Unchecked);
  }

  if(start_second_deriv_minimization)
  {
    secondderivminimizationmodified_slot(Qt::Checked);
  }
  else
  {
    secondderivminimizationmodified_slot(Qt::Unchecked);
  }

  if(start_second_deriv_covariance)
  {
    secondderivcovariancemodified_slot(Qt::Checked);
  }
  else
  {
    secondderivcovariancemodified_slot(Qt::Unchecked);
  }

  startlambdaEd->setText(double_to_QString(startlambda0));
  lambdafacEd->setText(double_to_QString(lambdafac0));
  toleranceEd->setText(double_to_QString(tolerance0));
  invMethodCb->setCurrentIndex(0);
  inv_method_modified_slot(0);
  svdEd->setText(int_to_QString(svd0));
  svdRatioEd->setText(double_to_QString(svd_ratio0));
  svdValueEd->setText(double_to_QString(svd_value0));
  rescaleValueEd->setText(double_to_QString(rescale_value0));
  stepsEd->setText(double_to_QString(steps0));
}

void FitSettingsWidget::modified_slot()
{
  emit modified();
}

void FitSettingsWidget::bayesianmodified_slot(int newState)
{
  boot_prior_cb->setEnabled(get_bayes());
  emit bayesianmodified(newState);
}


void FitSettingsWidget::numdiffmodified_slot(int newState)
{
/*  if(newState == Qt::Checked)
  {
    numdiffEd->setEnabled(true);
    numdiffLb->setEnabled(true);
  }
  else if(newState == Qt::Unchecked)
  {
    numdiffEd->setEnabled(false);
    numdiffLb->setEnabled(false);
  }
*/
  emit numdiffmodified(newState);
}


void FitSettingsWidget::secondderivminimizationmodified_slot(int newState)
{
  emit secondderivminimizationmodified(newState);
}

void FitSettingsWidget::secondderivcovariancemodified_slot(int newState)
{
  emit secondderivcovariancemodified(newState);
}


void FitSettingsWidget::svd_modified_slot()
{
  emit svd_modified();
}

void FitSettingsWidget::inv_method_modified_slot(int newindex)
{
  svdEd->setEnabled(false);
  svdLb->setEnabled(false);
  svdRatioEd->setEnabled(false);
  svdRatioLb->setEnabled(false);
  svdValueEd->setEnabled(false);
  svdValueLb->setEnabled(false);
  rescaleValueEd->setEnabled(false);
  rescaleValueLb->setEnabled(false);
  if(newindex==1)
  {
    svdEd->setEnabled(true);
    svdLb->setEnabled(true);
  }
  if(newindex==2)
  {
    svdRatioEd->setEnabled(true);
    svdRatioLb->setEnabled(true);
  }
  if(newindex==3)
  {
    svdValueEd->setEnabled(true);
    svdValueLb->setEnabled(true);
  }
  if(newindex==6)
  {
    rescaleValueEd->setEnabled(true);
    rescaleValueLb->setEnabled(true);
  }
  emit svd_modified();
}


void FitSettingsWidget::createGadgets()
{
  bayes_cb=new QCheckBox(tr("Activate Bayesian constraints"), this);
  connect(bayes_cb, SIGNAL(stateChanged(int)), this, SLOT(modified_slot()));
  connect(bayes_cb, SIGNAL(stateChanged(int)), this, SLOT(bayesianmodified_slot(int)));

  boot_prior_cb=new QCheckBox(tr("Use Gaussian random priors for multiple fits"), this);
  connect(boot_prior_cb, SIGNAL(stateChanged(int)), this, SLOT(modified_slot()));

  num_diff_cb=new QCheckBox(tr("Use numerical differentiation for first derivatives   "), this);
  connect(num_diff_cb, SIGNAL(stateChanged(int)), this, SLOT(modified_slot()));
  connect(num_diff_cb, SIGNAL(stateChanged(int)), this, SLOT(numdiffmodified_slot(int)));

  second_deriv_covariance_cb=new QCheckBox(tr("Use second derivatives for parameter covariance matrix"), this);
  connect(second_deriv_covariance_cb, SIGNAL(stateChanged(int)), this, SLOT(modified_slot()));
  connect(second_deriv_covariance_cb, SIGNAL(stateChanged(int)), this, SLOT(secondderivcovariancemodified_slot(int)));

  second_deriv_minimization_cb=new QCheckBox(tr("Use second derivatives for minimization"), this);
  connect(second_deriv_minimization_cb, SIGNAL(stateChanged(int)), this, SLOT(modified_slot()));
  connect(second_deriv_minimization_cb, SIGNAL(stateChanged(int)), this, SLOT(secondderivminimizationmodified_slot(int)));

  numdiffEd=new QLineEdit();
  numdiffEd->setValidator(new QDoubleValidator(1E-99, 1E99, 15, numdiffEd));
  numdiffLb=new QLabel("Numerical differentiation step size:");
  connect(numdiffEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));

  startlambdaEd=new QLineEdit();
  startlambdaEd->setValidator(new QDoubleValidator(1E-99, 1E99, 15, startlambdaEd));
  startlambdaLb=new QLabel("Start lambda:");
  connect(startlambdaEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));

  lambdafacEd=new QLineEdit();
  lambdafacEd->setValidator(new QDoubleValidator(1E-99, 1E99, 15, lambdafacEd));
  lambdafacLb=new QLabel("Lambda factor:");
  connect(lambdafacEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));

  toleranceEd=new QLineEdit();
  toleranceEd->setValidator(new QDoubleValidator(1E-99, 1E99, 15, toleranceEd));
  toleranceLb=new QLabel("Delta(chi^2) tolerance:");
  connect(toleranceEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));

  invMethodLb=new QLabel("(Pseudo-)Inversion method for data covariance matrix:");
  invMethodCb=new QComboBox;
  invMethodCb->addItem("LU decomposition");
  invMethodCb->addItem("SVD with fixed cut");
  invMethodCb->addItem("SVD with EV ratio cut");
  invMethodCb->addItem("SVD with EV value cut");
  invMethodCb->addItem("Diagonal only (uncorrelated fit)");
  invMethodCb->addItem("No inversion (data file contains inverse)");
  invMethodCb->addItem("Rescale off-diagonal elements");
  connect(invMethodCb, SIGNAL(currentIndexChanged(int)), this, SLOT(modified_slot()));
  connect(invMethodCb, SIGNAL(currentIndexChanged(int)), this, SLOT(inv_method_modified_slot(int)));

  svdEd=new QLineEdit();
  svdEd->setValidator(new QIntValidator(0, 2000000000, svdEd));
  svdLb=new QLabel("Fixed SVD cut:");
  connect(svdEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));
  connect(svdEd, SIGNAL(textChanged(const QString&)), this, SLOT(svd_modified_slot()));

  svdRatioEd=new QLineEdit();
  svdRatioEd->setValidator(new QDoubleValidator(1E-99, 1E99, 15, svdRatioEd));
  svdRatioLb=new QLabel("SVD EV ratio cut:");
  connect(svdRatioEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));
  connect(svdRatioEd, SIGNAL(textChanged(const QString&)), this, SLOT(svd_modified_slot()));

  svdValueEd=new QLineEdit();
  svdValueEd->setValidator(new QDoubleValidator(1E-99, 1E99, 15, svdValueEd));
  svdValueLb=new QLabel("SVD EV value cut:");
  connect(svdValueEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));
  connect(svdValueEd, SIGNAL(textChanged(const QString&)), this, SLOT(svd_modified_slot()));

  rescaleValueEd=new QLineEdit();
  rescaleValueEd->setValidator(new QDoubleValidator(1E-99, 1E99, 15, rescaleValueEd));
  rescaleValueLb=new QLabel("Off-diagonal rescale factor:");
  connect(rescaleValueEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));
  connect(rescaleValueEd, SIGNAL(textChanged(const QString&)), this, SLOT(svd_modified_slot()));

  stepsEd=new QLineEdit();
  stepsEd->setValidator(new QIntValidator(1, 2000000000, stepsEd));
  stepsLb=new QLabel("Maximum number of iterations:");
  connect(stepsEd, SIGNAL(textChanged(const QString&)), this, SLOT(modified_slot()));

  layout=new QGridLayout;

  layout->addWidget(bayes_cb, 0, 0);

  layout->addWidget(boot_prior_cb, 1, 0);

  layout->addWidget(num_diff_cb, 2, 0);
  hlayout3=new QHBoxLayout();
  hlayout3->addWidget(second_deriv_covariance_cb);
  hlayout3->addWidget(second_deriv_minimization_cb);
  layout->addLayout(hlayout3, 2, 1);

  layout->addWidget(numdiffLb, 3, 0);
  layout->addWidget(numdiffEd, 3, 1);

  layout->addWidget(startlambdaLb, 4, 0);
  layout->addWidget(startlambdaEd, 4, 1);

  layout->addWidget(lambdafacLb, 5, 0);
  layout->addWidget(lambdafacEd, 5, 1);

  layout->addWidget(toleranceLb, 6, 0);
  layout->addWidget(toleranceEd, 6, 1);

  layout->addWidget(invMethodLb, 7, 0);
  layout->addWidget(invMethodCb, 7, 1);

  layout->addWidget(svdLb, 8, 0);
  layout->addWidget(svdEd, 8, 1);

  layout->addWidget(svdRatioLb, 9, 0);
  layout->addWidget(svdRatioEd, 9, 1);

  layout->addWidget(svdValueLb, 10, 0);
  layout->addWidget(svdValueEd, 10, 1);

  layout->addWidget(rescaleValueLb, 11, 0);
  layout->addWidget(rescaleValueEd, 11, 1);

  layout->addWidget(stepsLb, 12, 0);
  layout->addWidget(stepsEd, 12, 1);

  setLayout(layout);
}
