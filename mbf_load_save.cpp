void MainWindow::loadFile(const QString &fileName)
{
  settingsmap m;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  if(!m.load_file(fileName.toStdString()))
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("QMBCFN"),
                         tr("Cannot read file %1:\n")
                         .arg(fileName));
    return;
  }


  if(m.exists("version"))
  {
    double v=m.get_double("version");
    if(v>version)
    {
      QApplication::restoreOverrideCursor();
      int ret=QMessageBox::warning(this, tr("QMBCFN"),
                                   tr("Warning: %1 was saved with a newer QMBCFN version: %2\n This QMBCFN version is %3\n Do you want to continue loading the file?").arg(strippedName(fileName)).arg(v).arg(version),
                                   QMessageBox::Yes,
                                   QMessageBox::No  | QMessageBox::Default | QMessageBox::Escape);
      if(ret==QMessageBox::No)
      {
        return;
      }
      QApplication::setOverrideCursor(Qt::WaitCursor);
    }
  }


  if(!m.exists("nfunctions"))
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("QMBCFN"),
                         tr("%1 does not contain number of functions")
                         .arg(fileName));
    return;
  }

  if(!m.exists("nparameters"))
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("QMBCFN"),
                         tr("%1 does not contain number of parameters")
                         .arg(fileName));
    return;
  }

  if(!m.exists("nconstants"))
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("QMBCFN"),
                         tr("%1 does not contain number of constants")
                         .arg(fileName));
    return;
  }

  int n_functions=m.get_int("nfunctions");
  int n_parameters=m.get_int("nparameters");
  int n_constants=m.get_int("nconstants");

  funcWidget->set_n_functions(n_functions);
  parderWidget->set_n_parameters(n_parameters);
  constsWidget->set_n_constants(n_constants);

  if(m.exists("currentmodel"))
  {
    current_model=static_cast<model>(m.get_int("currentmodel"));
    check_current_model();
    need_to_reset_fitter=true;
  }
  else
  {
    current_model=PARSE;
    check_current_model();
    need_to_reset_fitter=true;
  }

  for(int f=0; f<n_functions; ++f)
  {
    if(m.exists("functionname", f))
    {
      funcWidget->set_function_name(f, m.get_string("functionname", f));
    }
    else
    {
      stringstream sst;
      sst << f+1;
      string st;
      sst >> st;
      funcWidget->set_function_name(f, st);
    }
    funcWidget->set_function(f, m.get_string("function", f));
    if(m.exists("functionactive", f))
    {
      funcWidget->set_function_active(f, m.get_bool("functionactive", f));
    }
  }

  for(int p=0; p<n_parameters; ++p)
  {
    parderWidget->set_parameter(p, m.get_string("parameter", p));
    if(m.exists("startval", p))
    {
      double start_val=m.get_double("startval", p);
      if(start_val!=empty_double)
      {
        mainWidget->set_start_value(p, start_val);
      }
    }

    if(m.exists("prior", p))
    {
      double prior=m.get_double("prior", p);
      if(prior!=empty_double)
      {
        mainWidget->set_prior(p, prior);
      }
    }

    if(m.exists("sigma", p))
    {
      double sigma=m.get_double("sigma", p);
      if(sigma!=empty_double)
      {
        mainWidget->set_sigma(p, sigma);
      }
    }
  }

  for(int c=0; c<n_constants; ++c)
  {
    constsWidget->set_constant_name(c, m.get_string("constant", c));
    if(m.exists("constantvalue", c))
    {
      double val=m.get_double("constantvalue", c);
      if(val!=empty_double)
      {
        constsWidget->set_constant_value(c, val);
      }
    }
  }

  for(int f=0; f<n_functions; ++f)
  {
    for(int p=0; p<n_parameters; ++p)
    {
      parderWidget->set_derivative(f, p, m.get_string("derivative", f, p));
    }
  }

  if(m.exists("bayesian")) fitsetWidget->set_bayes(m.get_bool("bayesian"));

  if(m.exists("bootprior")) fitsetWidget->set_boot_prior(m.get_bool("bootprior"));
  if(m.exists("datafiletype"))
  {
    mainWidget->set_data_file_type(m.get_int("datafiletype"));
  }

  if(m.exists("numdiff")) fitsetWidget->set_num_diff(m.get_bool("numdiff"));
  if(m.exists("secondderivminimization")) fitsetWidget->set_second_deriv_minimization(m.get_bool("secondderivminimization"));
  if(m.exists("secondderivcovariance")) fitsetWidget->set_second_deriv_covariance(m.get_bool("secondderivcovariance"));

  if(m.exists("numdiffstep"))
  {
    double num_diff_step=m.get_double("numdiffstep");
    if(num_diff_step!=empty_double)
    {
      fitsetWidget->set_num_diff_step(num_diff_step);
    }
  }


  if(m.exists("startlambda"))
  {
    double startlambda=m.get_double("startlambda");
    if(startlambda!=empty_double)
    {
      fitsetWidget->set_startlambda(startlambda);
    }
  }

  if(m.exists("lambdafactor"))
  {
    double lambdafactor=m.get_double("lambdafactor");
    if(lambdafactor!=empty_double)
    {
      fitsetWidget->set_lambdafac(lambdafactor);
    }
  }

  if(m.exists("tolerance"))
  {
    double tolerance=m.get_double("tolerance");
    if(tolerance!=empty_double)
    {
      fitsetWidget->set_tolerance(tolerance);
    }
  }

  if(m.exists("svdcut"))
  {
    int svdcut=m.get_int("svdcut");
    if(svdcut!=empty_int)
    {
      fitsetWidget->set_svd(svdcut);
    }
  }

  if(m.exists("svdratio"))
  {
    double ratio=m.get_double("svdratio");
    if(ratio!=empty_double)
    {
      fitsetWidget->set_svd_ratio(ratio);
    }
  }

  if(m.exists("svdvalue"))
  {
    double value=m.get_double("svdvalue");
    if(value!=empty_double)
    {
      fitsetWidget->set_svd_value(value);
    }
  }

  if(m.exists("rescalevalue"))
  {
    double value=m.get_double("rescalevalue");
    if(value!=empty_double)
    {
      fitsetWidget->set_rescale_value(value);
    }
  }

  if(m.exists("inversionmethod"))
  {
    fitsetWidget->set_inversion_method(static_cast<inversion_method>(m.get_int("inversionmethod")));
  }
  else
  {
    if(fitsetWidget->get_svd()>0)
    {
      fitsetWidget->set_inversion_method(simple_cut);
    }
    else
    {
      fitsetWidget->set_inversion_method(LU_inversion);
    }
  }

  if(m.exists("maxsteps"))
  {
    int maxsteps=m.get_int("maxsteps");
    if(maxsteps!=empty_int)
    {
      fitsetWidget->set_steps(maxsteps);
    }
  }

  mainWidget->set_data_file(m.get_string("datafile"));
  mainWidget->set_output_dir(m.get_string("outputdir"));

  if(m.exists("nparametersdof"))
  {
    parderWidget->set_n_parameters_dof(m.get_int("nparametersdof"));
  }

  if(m.exists("chisqrextraterm:enabled"))
  {
    chisqrextraWidget->set_enabled(m.get_bool("chisqrextraterm:enabled"));
    if(m.exists("chisqrextraterm:function")) chisqrextraWidget->set_function(m.get_string("chisqrextraterm:function"));
    if(m.exists("chisqrextraterm:numdiffstep")) chisqrextraWidget->set_num_diff_step(m.get_double("chisqrextraterm:numdiffstep"));

    if(!m.exists("chisqrextraterm:nconstants"))
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::warning(this, tr("QMBCFN"),
                           tr("%1 does not contain number of chi^2 extra term constants")
                           .arg(fileName));
      return;
    }
    chisqrextraWidget->set_n_constants(m.get_int("chisqrextraterm:nconstants"));
    for(int c=0; c<m.get_int("chisqrextraterm:nconstants"); ++c)
    {
      chisqrextraWidget->set_constant_name(c, m.get_string("chisqrextraterm:constant", c));
      if(m.exists("chisqrextraterm:constantvalue", c))
      {
        double val=m.get_double("chisqrextraterm:constantvalue", c);
        if(val!=empty_double)
        {
          chisqrextraWidget->set_constant_value(c, val);
        }
      }
    }
  }


  QApplication::restoreOverrideCursor();
  setCurrentFile(fileName);
}


bool MainWindow::saveFile(const QString &fileName)
{
  settingsmap m;

  int n_functions=funcWidget->get_n_functions();
  int n_parameters=parderWidget->get_n_parameters();
  int n_constants=constsWidget->get_n_constants();

  m.set_double("version", version);

  m.set_int("currentmodel", current_model);

  m.set_int("nfunctions", n_functions);
  m.set_int("nparameters", n_parameters);
  m.set_int("nconstants", n_constants);

  m.set_int("nparametersdof", parderWidget->get_n_parameters_dof());

  for(int f=0; f<n_functions; ++f)
  {
    m.set_string("functionname", f, funcWidget->get_function_name(f));
    m.set_string("function", f, funcWidget->get_function(f));
    m.set_bool("functionactive", f, funcWidget->get_function_active(f));
  }

  for(int p=0; p<n_parameters; ++p)
  {
    m.set_string("parameter", p, parderWidget->get_parameter(p));
    m.set_double("startval", p, mainWidget->get_start_value(p));
    m.set_double("prior", p, mainWidget->get_prior(p));
    m.set_double("sigma", p, mainWidget->get_sigma(p));
  }

  for(int c=0; c<n_constants; ++c)
  {
    m.set_string("constant", c, constsWidget->get_constant_name(c));
    m.set_double("constantvalue", c, constsWidget->get_constant_value(c));
  }

  for(int f=0; f<n_functions; ++f)
  {
    for(int p=0; p<n_parameters; ++p)
    {
      m.set_string("derivative", f, p, parderWidget->get_derivative(f, p));
    }
  }

  m.set_bool("bayesian", fitsetWidget->get_bayes());
  m.set_bool("bootprior", fitsetWidget->get_boot_prior());
  m.set_int("datafiletype", mainWidget->get_data_file_type());
  m.set_bool("numdiff", fitsetWidget->get_num_diff());
  m.set_bool("secondderivminimization", fitsetWidget->get_second_deriv_minimization());
  m.set_bool("secondderivcovariance", fitsetWidget->get_second_deriv_covariance());
  m.set_double("numdiffstep", fitsetWidget->get_num_diff_step());

  m.set_double("startlambda", fitsetWidget->get_startlambda());
  m.set_double("lambdafactor", fitsetWidget->get_lambdafac());
  m.set_double("tolerance", fitsetWidget->get_tolerance());

  m.set_int("inversionmethod", fitsetWidget->get_inversion_method());
  m.set_double("svdratio", fitsetWidget->get_svd_ratio());
  m.set_double("svdvalue", fitsetWidget->get_svd_value());
  m.set_int("svdcut", fitsetWidget->get_svd());
  m.set_double("rescalevalue", fitsetWidget->get_rescale_value());

  m.set_int("maxsteps", fitsetWidget->get_steps());
  m.set_string("datafile", mainWidget->get_data_file());
  m.set_string("outputdir", mainWidget->get_output_dir());

  m.set_bool("chisqrextraterm:enabled", chisqrextraWidget->get_enabled());
  m.set_string("chisqrextraterm:function", chisqrextraWidget->get_function());
  m.set_double("chisqrextraterm:numdiffstep", chisqrextraWidget->get_num_diff_step());
  m.set_int("chisqrextraterm:nconstants", chisqrextraWidget->get_n_constants());
  for(int c=0; c<chisqrextraWidget->get_n_constants(); ++c)
  {
    m.set_string("chisqrextraterm:constant", c, chisqrextraWidget->get_constant_name(c));
    m.set_double("chisqrextraterm:constantvalue", c, chisqrextraWidget->get_constant_value(c));
  }

  QApplication::setOverrideCursor(Qt::WaitCursor);
  if(!m.save_file(fileName.toStdString()))
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("QMBCFN"),
                         tr("Cannot write file %1:\n")
                         .arg(fileName));
    return false;
  }
  QApplication::restoreOverrideCursor();
  setCurrentFile(fileName);
  return true;
}
