#include "fitter.h"

using namespace std;

fitter::fitter(abstract_global_model* fit_model,
               abstract_prior* fit_prior,
               chisqr_extra_term* fit_extra_term,
               QObject * parent)
{
  model=fit_model;
  prior=fit_prior;
  extra_term=fit_extra_term;

  num_diff=false;
  num_diff_step=default_num_diff_step;

  second_deriv_minimization=false;
  second_deriv_covariance=false;

  n_fit_points=model->get_n_fit_points();
  n_parameters=model->get_n_parameters();

  start_lambda=default_start_lambda;
  lambda_fac=default_lambda_fac;
  tolerance=default_tolerance;

  start_vals.resize(n_parameters, 0.0);

  inv_datacov=NULL;

  model->set_parameters(start_vals);

  result_cov=gsl_matrix_calloc(n_parameters, n_parameters);
  result_params=start_vals;

  tmp_params=start_vals;

  inv_method=default_inversion_method;
  svd_cut=default_svd_cut;

  svd_ratio=default_svd_ratio;
  svd_value=default_svd_value;
  off_diagonal_rescale_factor=default_off_diagonal_rescale_factor;

  cut=0;

  stop = false;
}


fitter::~fitter()
{
  if(inv_datacov!=NULL)
  {
    gsl_matrix_free(inv_datacov);
  }
  gsl_matrix_free(result_cov);
}


void fitter::set_num_diff(bool b)
{
  num_diff=b;
}


void fitter::set_num_diff_step(double d)
{
  num_diff_step=d;
}


void fitter::set_second_deriv_minimization(bool b)
{
  second_deriv_minimization=b;
}


void fitter::set_second_deriv_covariance(bool b)
{
  second_deriv_covariance=b;
}


bool fitter::set_data(const vector< double >& data, const vector< vector< double > >& data_covariance_matrix, std::string& message)
{
  dat=data;

  gsl_matrix* datacov=gsl_matrix_alloc(n_fit_points, n_fit_points);
  
  if( inv_method==off_diagonal_rescale )
  {
    for(int m1=0; m1<n_fit_points; ++m1)
    {
      for(int m2=0; m2<m1; ++m2)
      {
        gsl_matrix_set(datacov, m1, m2, off_diagonal_rescale_factor*data_covariance_matrix[m1][m2]);
      }
      gsl_matrix_set(datacov, m1, m1, data_covariance_matrix[m1][m1]);
    }
    for(int m1=0; m1<n_fit_points; ++m1)
    {
      for(int m2=m1+1; m2<n_fit_points; ++m2)
      {
        gsl_matrix_set(datacov, m1, m2, gsl_matrix_get(datacov, m2, m1)) ;
      }
    }
  }
  else
  {
    for(int m1=0; m1<n_fit_points; ++m1)
    {
      for(int m2=0; m2<=m1; ++m2)
      {
        gsl_matrix_set(datacov, m1, m2, data_covariance_matrix[m1][m2]);
      }
    }
    for(int m1=0; m1<n_fit_points; ++m1)
    {
      for(int m2=m1+1; m2<n_fit_points; ++m2)
      {
        gsl_matrix_set(datacov, m1, m2, gsl_matrix_get(datacov, m2, m1)) ;
      }
    }
  }

// invert covariance matrix

  cut=0;
  int zero_values=0;

  if(inv_datacov!=NULL)
  {
    gsl_matrix_free(inv_datacov);
  }
  inv_datacov=gsl_matrix_alloc(n_fit_points, n_fit_points);

  bool singular=false;

  bool success=true;

  if(inv_method==no_inversion)
  {
    for(int m1=0; m1<n_fit_points; ++m1)
    {
      for(int m2=0; m2<n_fit_points; ++m2)
      {
        gsl_matrix_set(inv_datacov, m1, m2, gsl_matrix_get(datacov, m1, m2));
      }
    }
    gsl_matrix_free(datacov);
    return true;
  }
  
  if(inv_method==diagonal)
  {
    for(int m1=0; m1<n_fit_points; ++m1)
    {
      for(int m2=0; m2<n_fit_points; ++m2)
      {
        gsl_matrix_set(inv_datacov, m1, m2, 0.0) ;
      }
    }

    for(int m=0; m<n_fit_points; ++m)
    {
      double d=gsl_matrix_get(datacov, m, m);
      if(d<=0)
      {
        message="Error: zero or negative diagonal element in data covariance matrix.";
        gsl_matrix_free(datacov);
        return false;
      }
      gsl_matrix_set(inv_datacov, m, m, 1.0/d);
    }
    gsl_matrix_free(datacov);
    return true;
  }

  if( (inv_method==LU_inversion) || (inv_method==off_diagonal_rescale) )
  {
    gsl_matrix* work_m=gsl_matrix_alloc(n_fit_points, n_fit_points);
    gsl_matrix_memcpy(work_m, datacov);
    gsl_permutation* perm=gsl_permutation_alloc(n_fit_points);
    int signum;
    if(gsl_linalg_LU_decomp(work_m, perm, &signum)!=GSL_SUCCESS)
    {
      singular=true;
      message="Warning: gsl_linalg_LU_decomp of data covariance matrix failed. Using SVD instead of full inverse.";
    }
    else if(gsl_linalg_LU_invert(work_m, perm, inv_datacov)!=GSL_SUCCESS)
    {
      singular=true;
      message="Warning: gsl_linalg_LU_invert of data covariance matrix failed. Using SVD instead of full inverse.";
    }
    gsl_permutation_free(perm);
    gsl_matrix_free(work_m);
  }
  if( ((inv_method!=LU_inversion) && (inv_method!=off_diagonal_rescale)) || singular)
  {
    gsl_matrix* work_m=gsl_matrix_alloc(n_fit_points, n_fit_points);
    gsl_matrix_memcpy(work_m, datacov);

    gsl_matrix* V=gsl_matrix_alloc(n_fit_points, n_fit_points);
    gsl_vector* S=gsl_vector_alloc(n_fit_points);
    gsl_vector* work_v=gsl_vector_alloc(n_fit_points);

    if(gsl_linalg_SV_decomp(work_m, V, S, work_v)!=GSL_SUCCESS)  // now work_m contains the matrix U
    {
      message+="\n\n Warning: gsl_linalg_SV_decomp of data covariance matrix failed.";
      success=false;
    }

    if(inv_method==simple_cut)
    {
      cut=svd_cut;
    }
    else if(inv_method==ratio_cut)
    {
      double largest_ev=gsl_vector_get(S, 0);
      for(int k=1; k<n_fit_points; ++k)
      {
        if(gsl_vector_get(S, k)/largest_ev<svd_ratio)
        {
          cut=n_fit_points-k;
          break;
        }
      }
    }
    else if(inv_method==absolute_cut)
    {
      for(int k=0; k<n_fit_points; ++k)
      {
        if(gsl_vector_get(S, k)<svd_value)
        {
          cut=n_fit_points-k;
          break;
        }
      }
    }

    double inv_datacov_temp;
    for(int i=0; i<n_fit_points; ++i)
    {
      for(int j=0; j<=i; ++j)
      {
        inv_datacov_temp=0.0;
        zero_values=0.0;
        for(int k=0; k<n_fit_points-cut; ++k)
        {
          if(gsl_vector_get(S, k)>0)
          {
            inv_datacov_temp+=gsl_matrix_get(V, i, k)*(1.0/gsl_vector_get(S, k))*gsl_matrix_get(work_m, j, k);
          }
          else
          {
            ++zero_values;
          }
        }
        gsl_matrix_set(inv_datacov, i, j, inv_datacov_temp);
      }
    }

    for(int i=0; i<n_fit_points; ++i)
    {
      for(int j=i+1; j<n_fit_points; ++j)
      {
        gsl_matrix_set(inv_datacov, i, j, gsl_matrix_get(inv_datacov, j, i)) ;
      }
    }

    gsl_vector_free(work_v);
    gsl_matrix_free(V);
    gsl_vector_free(S);
    gsl_matrix_free(work_m);
  }

  cut+=zero_values;

  gsl_matrix_free(datacov);

  return success;
}


void fitter::set_data_only(const vector< double >& data)
{
  dat=data;
}

void fitter::set_starting_value(int parameter, double value)
{
  start_vals[parameter]=value;
}


void fitter::set_initial_lambda(double initial_lambda)
{
  start_lambda=initial_lambda;
}


void fitter::set_lambda_factor(double lambda_factor)
{
  lambda_fac=lambda_factor;
}


void fitter::set_tolerance(double dchisqr_tolerance)
{
  tolerance=dchisqr_tolerance;
}


double fitter::get_parameter(int parameter)
{
  return result_params[parameter];
}


double fitter::get_covariance(int parameter_1, int parameter_2)
{
  return gsl_matrix_get(result_cov, parameter_1, parameter_2);
}

bool fitter::covariance_positive_definite()
{
  gsl_matrix* work_m=gsl_matrix_alloc(n_parameters, n_parameters);
  gsl_matrix_memcpy(work_m, result_cov);
//  gsl_set_error_handler_off();
  int status=gsl_linalg_cholesky_decomp(work_m);
//  gsl_set_error_handler(NULL);
  gsl_matrix_free(work_m);
  return (status!=GSL_EDOM);
}

double fitter::get_chi_sqr()
{
  return chi_sqr(result_params);
}


double fitter::get_chi_sqr_start()
{
  return chi_sqr(start_vals);
}

int fitter::get_cut()
{
  return cut;
}

int fitter::get_dof()
{
  return n_fit_points;
}

double fitter::chi_sqr(const vector< double >& params)
{
  model->set_parameters(params);
  prior->set_parameters(params);
  extra_term->set_parameters(params);

  vector< double > temp_vec(n_fit_points, 0.0);
  for(int m=0; m<n_fit_points; ++m)
  {
    temp_vec[m]=dat[m]-model->eval_function(m);
  }

  double temp_chi_sqr=0.0;
  double temp_chi_sqr2;

  if(inv_method!=diagonal)
  {
    for(int m1=0; m1<n_fit_points; ++m1)
    {
      temp_chi_sqr2=0.0;
      for(int m2=0; m2<m1; ++m2)
      {
        temp_chi_sqr2+= gsl_matrix_get(inv_datacov, m1, m2)*temp_vec[m2];
      }
      temp_chi_sqr+=temp_chi_sqr2*temp_vec[m1];
    }
    temp_chi_sqr*=2.0;
  }

  for(int m1=0; m1<n_fit_points; ++m1)
  {
    temp_chi_sqr+= gsl_matrix_get(inv_datacov, m1, m1)
                  *temp_vec[m1]*temp_vec[m1];
  }


  temp_chi_sqr+=prior->chi_sqr();
  temp_chi_sqr+=extra_term->chi_sqr();

  return temp_chi_sqr;
}


void fitter::beta(const vector< double >& params, gsl_vector* result)
{
  model->set_parameters(params);
  prior->set_parameters(params);
  extra_term->set_parameters(params);

  tmp_params=params;

  vector< double > temp_vec_1(n_fit_points, 0.0);
  for(int m=0; m<n_fit_points; ++m)
  {
    temp_vec_1[m]=dat[m]-model->eval_function(m);
  }

  for(int p=0; p<n_parameters; ++p)
  {
    vector< double > temp_vec_2(n_fit_points, 0.0);
    if(!num_diff)
    {
      for(int m=0; m<n_fit_points; ++m)
      {
        temp_vec_2[m]=model->eval_derivative(m, p);
      }
    }
    else  // use symmetric difference approximation of derivative
    {
      tmp_params[p]=params[p]+num_diff_step;  // forward step
      model->set_parameters(tmp_params);
      for(int m=0; m<n_fit_points; ++m)
      {
        temp_vec_2[m]=model->eval_function(m);
      }
      tmp_params[p]=params[p]-num_diff_step;  // backward step
      model->set_parameters(tmp_params);
      for(int m=0; m<n_fit_points; ++m)
      {
        temp_vec_2[m]-=model->eval_function(m);
      }
      tmp_params[p]=params[p];                // restore tmp_params
      for(int m=0; m<n_fit_points; ++m)
      {
        temp_vec_2[m]/=(2.0*num_diff_step);
      }
    }

    double temp_beta=0.0;
    double temp_beta2;
    
    if(inv_method!=diagonal)
    {
      for(int m1=0; m1<n_fit_points; ++m1)
      {
        temp_beta2=0.0;
        for(int m2=0; m2<n_fit_points; ++m2)
        {
          temp_beta2+= gsl_matrix_get(inv_datacov, m1, m2)*temp_vec_1[m2];
        }
        temp_beta+=temp_beta2*temp_vec_2[m1];
      }
    }
    else
    {
      for(int m=0; m<n_fit_points; ++m)
      {
        temp_beta += gsl_matrix_get(inv_datacov, m, m)*temp_vec_1[m]*temp_vec_2[m];
      }
    }

    temp_beta+=prior->beta(p);
    temp_beta+=extra_term->beta(p);

    gsl_vector_set(result, p, temp_beta);
  }
}


void fitter::alpha(const vector< double >& params, double lambda, bool secondderivs, gsl_matrix* result)
{
  model->set_parameters(params);
  prior->set_parameters(params);
  extra_term->set_parameters(params);

  // compute and store first-order derivatives
  tmp_params=params;
  double* all_derivatives=new double[n_parameters*n_fit_points];
  for(int p=0; p<n_parameters; ++p)
  {
    if(!num_diff)
    {
      for(int m=0; m<n_fit_points; ++m)
      {
        all_derivatives[p*n_fit_points+m]=model->eval_derivative(m, p);
      }
    }
    else  // use symmetric difference approximation of derivative
    {
      tmp_params[p]=params[p]+num_diff_step;  // forward step
      model->set_parameters(tmp_params);
      for(int m=0; m<n_fit_points; ++m)
      {
        all_derivatives[p*n_fit_points+m]=model->eval_function(m);
      }
      tmp_params[p]=params[p]-num_diff_step;  // backward step
      model->set_parameters(tmp_params);
      for(int m=0; m<n_fit_points; ++m)
      {
        all_derivatives[p*n_fit_points+m]-=model->eval_function(m);
      }
      tmp_params[p]=params[p];                // restore tmp_params
      for(int m=0; m<n_fit_points; ++m)
      {
        all_derivatives[p*n_fit_points+m]/=(2.0*num_diff_step);
      }
    }
  }

  // store inv_datacov in built-in array for better performance
  double* inv_datacov_vector=new double[n_fit_points*n_fit_points];
  for(int m1=0; m1<n_fit_points; ++m1)
  {
    for(int m2=0; m2<n_fit_points; ++m2)
    {
      inv_datacov_vector[m1*n_fit_points+m2]=gsl_matrix_get(inv_datacov, m1, m2);
    }
  }

  double alpha_temp, alpha_temp2;
  int index1, index2, index3;

  // compute diagonal and lower off-diagonal entries of the part of alpha without second derivatives
  for(int p1=0; p1<n_parameters; ++p1)
  {
    index1=p1*n_fit_points;
    for(int p2=0; p2<=p1; ++p2)
    {
      index2=p2*n_fit_points;
      alpha_temp=0.0;
      if(inv_method!=diagonal)
      {
        for(int m1=0; m1<n_fit_points; ++m1)
        {
          index3=m1*n_fit_points;
          alpha_temp2=0.0;
          for(int m2=0; m2<n_fit_points; ++m2)
          {
            alpha_temp2+= inv_datacov_vector[index3+m2]*all_derivatives[index2+m2];
          }
          alpha_temp+=alpha_temp2*all_derivatives[index1+m1];
        }
      }
      else
      {
        for(int m=0; m<n_fit_points; ++m)
        {
          alpha_temp+= inv_datacov_vector[m*n_fit_points+m]*all_derivatives[index2+m]*all_derivatives[index1+m];
        }
      }
      alpha_temp+=prior->alpha(p1, p2);
      alpha_temp+=extra_term->alpha(p1, p2);
      gsl_matrix_set(result, p1, p2, alpha_temp);
    }
  }
  delete[] all_derivatives;
  // compute diagonal and lower off-diagonal entries of the part of alpha with second derivatives
  if(secondderivs)
  {
    double sqrt_num_diff_step=sqrt(num_diff_step);
    model->set_parameters(params);
    double* temp_vec_1=new double[n_fit_points];
    for(int m=0; m<n_fit_points; ++m)
    {
      temp_vec_1[m]=dat[m]-model->eval_function(m);
    }

    tmp_params=params;
    double* all_second_derivatives=new double[n_fit_points];
    for(int p1=0; p1<n_parameters; ++p1)
    {
      for(int p2=0; p2<=p1; ++p2)
      {

        if(num_diff)   // compute second derivatives completely numerically, using step size = sqrt(num_diff_step)
        {
          if(p1!=p2)
          {
            tmp_params[p1]=params[p1]+sqrt_num_diff_step;  // forward step
            tmp_params[p2]=params[p2]+sqrt_num_diff_step;  // forward step
            model->set_parameters(tmp_params);
            for(int m=0; m<n_fit_points; ++m)
            {
              all_second_derivatives[m]=model->eval_function(m);
            }

            tmp_params[p1]=params[p1]+sqrt_num_diff_step;  // forward step
            tmp_params[p2]=params[p2]-sqrt_num_diff_step;  // backward step
            model->set_parameters(tmp_params);
            for(int m=0; m<n_fit_points; ++m)
            {
              all_second_derivatives[m]-=model->eval_function(m);
            }

            tmp_params[p1]=params[p1]-sqrt_num_diff_step;  // backward step
            tmp_params[p2]=params[p2]+sqrt_num_diff_step;  // forward step
            model->set_parameters(tmp_params);
            for(int m=0; m<n_fit_points; ++m)
            {
              all_second_derivatives[m]-=model->eval_function(m);
            }

            tmp_params[p1]=params[p1]-sqrt_num_diff_step;  // backward step
            tmp_params[p2]=params[p2]-sqrt_num_diff_step;  // backward step
             model->set_parameters(tmp_params);
            for(int m=0; m<n_fit_points; ++m)
            {
              all_second_derivatives[m]+=model->eval_function(m);
            }

            tmp_params[p1]=params[p1];                // restore tmp_params
            tmp_params[p2]=params[p2];                // restore tmp_params
          }
          else
          {
            tmp_params[p1]=params[p1]+2.0*sqrt_num_diff_step;  // forward step
            model->set_parameters(tmp_params);
            for(int m=0; m<n_fit_points; ++m)
            {
              all_second_derivatives[m]=model->eval_function(m);
            }

            tmp_params[p1]=params[p1]-2.0*sqrt_num_diff_step;  // backward step
            model->set_parameters(tmp_params);
            for(int m=0; m<n_fit_points; ++m)
            {
              all_second_derivatives[m]+=model->eval_function(m);
            }

            tmp_params[p1]=params[p1];                // middle
            model->set_parameters(tmp_params);
            for(int m=0; m<n_fit_points; ++m)
            {
              all_second_derivatives[m]-=2.0*(model->eval_function(m));
            }
          }
          // divide by (2*sqrt_num_diff_step)^2 = 4*num_diff_step
          for(int m=0; m<n_fit_points; ++m)
          {
            all_second_derivatives[m]/=(4.0*num_diff_step);
          }
        }
        else  // compute second derivatives by first-order numerical derivative w.r.t. p2 of symbolic first-order derivatives w.r.t. p1
        {
          tmp_params[p2]=params[p2]+num_diff_step;  // forward step
          model->set_parameters(tmp_params);
          for(int m=0; m<n_fit_points; ++m)
          {
            all_second_derivatives[m]=model->eval_derivative(m, p1);
          }
          tmp_params[p2]=params[p2]-num_diff_step;  // backward step
          model->set_parameters(tmp_params);
          for(int m=0; m<n_fit_points; ++m)
          {
            all_second_derivatives[m]-=model->eval_derivative(m, p1);
          }
          tmp_params[p2]=params[p2];                // restore tmp_params
          for(int m=0; m<n_fit_points; ++m)
          {
            all_second_derivatives[m]/=(2.0*num_diff_step);
          }
        }

        alpha_temp=gsl_matrix_get(result, p1, p2);
        if(inv_method!=diagonal)
        {
          for(int m1=0; m1<n_fit_points; ++m1)
          {
            index3=m1*n_fit_points;
            alpha_temp2=0.0;
            for(int m2=0; m2<n_fit_points; ++m2)
            {
              alpha_temp2+=inv_datacov_vector[index3+m2]*temp_vec_1[m2];
            }
            alpha_temp-=alpha_temp2*all_second_derivatives[m1];   // minus sign
          }
          gsl_matrix_set(result, p1, p2, alpha_temp);
        }
        else
        {
          for(int m=0; m<n_fit_points; ++m)
          {
            alpha_temp-=inv_datacov_vector[m*n_fit_points+m]*temp_vec_1[m]*all_second_derivatives[m];   // minus sign
          }
          gsl_matrix_set(result, p1, p2, alpha_temp);
        }

      }
    }

    delete[] all_second_derivatives;
    delete[] temp_vec_1;
  }

  delete[] inv_datacov_vector;

  // insert upper off-diagonal entries
  for(int p1=0; p1<n_parameters; ++p1)
  {
    for(int p2=p1+1; p2<n_parameters; ++p2)
    {
      gsl_matrix_set(result, p1, p2, gsl_matrix_get(result, p2, p1)) ;
    }
  }

  // multiply diagonal terms by (1+lambda)
  for(int p=0; p<n_parameters; ++p)
  {
    gsl_matrix_set(result, p, p, (1.0+lambda)*gsl_matrix_get(result, p, p));
  }
}


int fitter::fit(int max_steps, std::string& message)
{
  message="";
  int steps=0;
  double lambda=start_lambda;

  vector< double > old_params=start_vals;
  vector< double > new_params(n_parameters);

  gsl_permutation* perm=gsl_permutation_alloc(n_parameters);
  gsl_vector* delta_params=gsl_vector_alloc(n_parameters);
  int signum;

  double old_chi_sqr=chi_sqr(old_params);
  double new_chi_sqr;

  double tmp;
  bool out_of_range=false;

  for(;;)
  {
    steps+=1;
    if( (steps>max_steps) || stop)
    {
      break;
    }

    gsl_vector* beta_vector=gsl_vector_alloc(n_parameters);
    beta(old_params, beta_vector);

    gsl_matrix* alpha_matrix=gsl_matrix_alloc(n_parameters, n_parameters);
    alpha(old_params, lambda, second_deriv_minimization, alpha_matrix);

    if(gsl_linalg_LU_decomp(alpha_matrix, perm, &signum)!=GSL_SUCCESS)
    {
      message="Warning: gsl_linalg_LU_decomp failed. Iteration aborted";
      gsl_vector_free(beta_vector);
      gsl_matrix_free(alpha_matrix);
      break;
    }

    if(gsl_linalg_LU_solve(alpha_matrix, perm, beta_vector, delta_params)!=GSL_SUCCESS)
    {
      message="Warning: gsl_linalg_LU_solve failed. Iteration aborted";
      gsl_vector_free(beta_vector);
      gsl_matrix_free(alpha_matrix);
      break;
    }

    gsl_vector_free(beta_vector);
    gsl_matrix_free(alpha_matrix);

    for(int p=0; p<n_parameters; ++p)
    {
      tmp=old_params[p]+gsl_vector_get(delta_params, p);

      if( fabs(tmp)<numeric_limits<double>::max() )
      {
        new_params[p]=tmp;
      }
      else
      {
        new_params[p]=old_params[p];
        out_of_range=true;
      }
    }

    if(out_of_range)
    {
      message="Warning: numerical exception in proposed parameter update. Iteration aborted";
      new_params=old_params;
      break;
    }

    new_chi_sqr=chi_sqr(new_params);

    if( !(fabs(new_chi_sqr)<numeric_limits<double>::max()) )
    {
      message="Warning: numerical exception in proposed parameter update. Iteration aborted";
      new_params=old_params;
      break;
    }

    emit step_done(steps, new_chi_sqr, lambda);

    if(new_chi_sqr <= old_chi_sqr)
    {
      if(old_chi_sqr-new_chi_sqr<tolerance)
      {
        break;
      }
      else
      {
        old_params=new_params;
        old_chi_sqr=new_chi_sqr;
        lambda/=lambda_fac;
      }
    }
    else
    {
      lambda*=lambda_fac;
    }
  }

  result_params=new_params;

  gsl_matrix* alpha_matrix=gsl_matrix_alloc(n_parameters, n_parameters);
  alpha(result_params, 0.0, second_deriv_covariance, alpha_matrix);
  gsl_linalg_LU_decomp(alpha_matrix, perm, &signum);
  gsl_linalg_LU_invert(alpha_matrix, perm, result_cov);
  gsl_matrix_free(alpha_matrix);

  gsl_permutation_free(perm);
  gsl_vector_free(delta_params);

  return steps;
}


bool fitter::test_model()
{
  model->set_parameters(start_vals);

  for(int m=0; m<n_fit_points; ++m)
  {
    model->eval_function(m);
    if(model->no_of_errors()>0)
    {
      return false;
    }
    if(!num_diff)
    {
      for(int p=0; p<n_parameters; ++p)
      {
        model->eval_derivative(m, p);
        if(model->no_of_errors()>0)
        {
          return false;
        }
      }
    }
  }
  return true;
}


void fitter::set_inversion_method(inversion_method method)
{
  inv_method=method;
}

void fitter::set_svd_cut(int scut)
{
  svd_cut=scut;
}

void fitter::set_svd_cut_ratio(double ratio)
{
  svd_ratio=ratio;
}

void fitter::set_svd_cut_absolute(double value)
{
  svd_value=value;
}


void fitter::set_off_diagonal_rescale_factor(double value)
{
  off_diagonal_rescale_factor=value;
}

