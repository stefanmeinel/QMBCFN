#ifndef BOOTSTRAP_THREAD_H
#define BOOTSTRAP_THREAD_H

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <QtWidgets>
#include <QThread>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_gamma.h>

#include "fitter.h"
#include "gaussian_prior.h"

#include "convert_qstring.h"

enum exit_code
{
  ok = 0,
  fit_not_converge = 1,
  output_error = 2,
  aborted = 3
};


class bootstrap_thread : public QThread
{
  Q_OBJECT

  public:

    bootstrap_thread(QObject* parent = 0);
    ~bootstrap_thread();

    void set_data_file_type(int type);

    void set_fitter(fitter* f);
    void set_gaussian_prior(gaussian_prior* gp);

    void set_max_steps(int steps);
    int get_steps_needed();

    void set_n_parameters(int n);
    void set_parameter_names(const vector< string >& names);

    void set_n_parameters_dof(int n);

    void set_boot_prior(bool pr);

    void set_multi_fit_data(const vector< vector< double > >& mfd);
    void set_multi_fit_correlation_matrix(const vector< vector< vector< double > > >& mfcm);

    void set_priors(const vector< double >& prs);
    void set_sigmas(const vector< double >& sgs);

    void set_output_dir(const string& d);
    void set_file_name(const QString& f);

    exit_code get_exit_code();

    bool stop;

  signals:
    void fit_finished(bool);
    void message_signal(QString);
    void step(int);

  protected:
    void run();

  private:

    int data_file_type;

    fitter* _fitter;
    gaussian_prior* _gaussian_prior;

    int steps_needed;
    int max_steps;

    int n_parameters;

    int n_parameters_dof;

    bool boot_prior;

    string dir;
    QString curFile;

    exit_code ec;

    vector< vector< double > > multi_fit_data;
    vector< vector< vector< double > > > multi_fit_correlation_matrix;

    vector< string > parameter_names;

    vector< double > priors;
    vector< double > sigmas;

};

#endif
