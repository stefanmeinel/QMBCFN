#include "bootstrap_thread.h"

bootstrap_thread::bootstrap_thread(QObject* parent)
{
  ec = ok;
  max_steps = 100;
  steps_needed = 100;
  boot_prior = true;
  stop = false;
  n_parameters_dof=0;
}

bootstrap_thread::~bootstrap_thread()
{
}


void bootstrap_thread::set_data_file_type(int type)
{
  data_file_type = type;
}


void bootstrap_thread::set_fitter(fitter* f)
{
  _fitter = f;
}


void bootstrap_thread::set_max_steps(int steps)
{
  max_steps = steps;
}


int bootstrap_thread::get_steps_needed()
{
  return steps_needed;
}


void bootstrap_thread::set_gaussian_prior(gaussian_prior* gp)
{
  _gaussian_prior = gp;
}


void bootstrap_thread::set_n_parameters(int n)
{
  n_parameters = n;
}

void bootstrap_thread::set_parameter_names(const vector< string >& names)
{
  parameter_names = names;
}


void bootstrap_thread::set_n_parameters_dof(int n)
{
  n_parameters_dof = n;
}


void bootstrap_thread::set_boot_prior(bool pr)
{
  boot_prior = pr;
}


exit_code bootstrap_thread::get_exit_code()
{
  return ec;
}


void bootstrap_thread::set_multi_fit_data(const vector< vector< double > >& mfd)
{
  multi_fit_data = mfd;
}


void bootstrap_thread::set_multi_fit_data_covariance_matrix(const vector< vector< vector< double > > >& mfcm)
{
  multi_fit_data_covariance_matrix = mfcm;
}


void bootstrap_thread::set_priors(const vector< double >& prs)
{
  priors = prs;
}


void bootstrap_thread::set_sigmas(const vector< double >& sgs)
{
  sigmas = sgs;
}


void bootstrap_thread::set_file_name(const QString& f)
{
  curFile = f;
}

void bootstrap_thread::set_output_dir(const string& d)
{
  dir = d;
  if(*(dir.end()-1)!='/')
  {
    dir.append("/");
  }
}



QString strippedName(const QString &fullFileName)
{
  return QFileInfo(fullFileName).fileName();
}


void bootstrap_thread::run()
{
  ec = ok;
  string fit_message;
  steps_needed=_fitter->fit(max_steps, fit_message);
  if(stop)
  {
    ec = aborted;
    return;
  }

  emit fit_finished(true);

  msleep(200);

  if(steps_needed==max_steps+1)
  {
    ec = fit_not_converge;
    return;
  }

  if(fit_message!="")
  {
    emit message_signal(fit_message.c_str());
    return;
  }

  gsl_rng_default_seed=0;
  gsl_rng_env_setup();
  const gsl_rng_type* Trng=gsl_rng_mt19937;
  gsl_rng* rng=gsl_rng_alloc(Trng);

  vector< double > boot_results_temp(n_parameters, 0.0);
  vector< vector< double > > boot_results(multi_fit_data.size(), boot_results_temp);

  vector< ofstream* > all_output;
  vector< string > all_filenames;
  all_output.resize(n_parameters);
  all_filenames.resize(n_parameters);

  for(int p=0; p<n_parameters; ++p)
  {
    string filename;
    if(curFile.isEmpty())
    {
      filename=dir+"untitled.mbcfn_multifit_"+parameter_names[p]+".dat";
    }
    else
    {
      filename=dir+strippedName(curFile).toStdString()+"_multifit_"+parameter_names[p]+".dat";
    }
    all_filenames[p]=filename;

    ofstream* output = new ofstream(filename.c_str());

    if(!(*output))
    {
      ec = output_error;
      return;
    }

    output -> precision(15);
    all_output[p]=output;
  }

  for(int boot=0; boot<multi_fit_data.size(); ++boot)
  {
    if(data_file_type==1)
    {
      _fitter->set_data_only(multi_fit_data[boot]);
    }
    else if(data_file_type==2)
    {
      string set_data_message;
      bool status;
      status = _fitter->set_data(multi_fit_data[boot], multi_fit_data_covariance_matrix[boot], set_data_message);
      if(set_data_message!="")
      {
        stringstream message;
        message << "Fit #" << boot+1 << ": " << set_data_message;
        emit message_signal((message.str()).c_str());
      }
      if(status!=true)
      {
        stringstream message;
        message << "Fit #" << boot+1 << ": Failed to invert data covariance matrix";
        return;
      }
    }

    if(boot_prior)
    {
      for(int p=0; p<n_parameters; ++p)
      {
        _gaussian_prior->set_prior(p, priors[p]+gsl_ran_gaussian(rng, sigmas[p]));
      }
    }

    steps_needed=_fitter->fit(max_steps, fit_message);
    if(fit_message!="")
    {
      emit message_signal(fit_message.c_str());
    }

    if(stop)
    {
      gsl_rng_free(rng);
      ec = aborted;
      for(int p=0; p<n_parameters; ++p)
      {
        all_output[p] -> close();
        delete all_output[p];
      }
      return;
    }

/*
    double chisqr=_fitter->get_chi_sqr();
    double dof=_fitter->get_dof()-_fitter->get_cut()-n_parameters_dof;
    if(dof<0.0)
    {
      dof=0.0;
    }
    if(steps_needed==max_steps+1)
    {
      stringstream message;
      message << "Warning: Fit #" << boot+1 << " did not converge after " << max_steps << " iterations. " << "chi^2/dof = " << chisqr/dof;
      emit message_signal((message.str()).c_str());
    }
    else
    {
      stringstream message;
      message << "Fit #" << boot+1 << " converged after " << steps_needed << " iterations. " << "chi^2/dof = " << chisqr/dof;
      emit message_signal((message.str()).c_str());
    }
*/

    if(steps_needed==max_steps+1)
    {
      stringstream message;
      message << "Warning: Fit #" << boot+1 << " did not converge after " << max_steps << " iterations." << endl;
      emit message_signal((message.str()).c_str());
    }


    for(int p=0; p<n_parameters; ++p)
    {
      boot_results[boot][p]=_fitter->get_parameter(p);
      *all_output[p] << boot_results[boot][p] << endl;
    }
    emit step(boot);
  }

  for(int p=0; p<n_parameters; ++p)
  {
    all_output[p] -> close();
    delete all_output[p];
  }

  gsl_rng_free(rng);

  stringstream message;
  message << multi_fit_data.size() << " fits completed.";
  emit message_signal((message.str()).c_str());

}
