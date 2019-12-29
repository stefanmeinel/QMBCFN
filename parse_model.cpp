#include "parse_model.h"

parse_model::parse_model(const vector< string >& functions,
                         const vector< string >& parameters,
                         const vector< string >& constants,
                         const vector< vector< string > >& derivatives)
{
  errors=0;

  n_fit_points=functions.size();
  n_parameters=parameters.size();
  n_constants=constants.size();

  param_names=parameters;
  const_names=constants;
  func_names=functions;

  for(vector< string >::const_iterator it=functions.begin(); it!=functions.end(); ++it)
  {
    funcs_parsers.push_back(new parser(*it));
  }

  for(vector< vector< string > >::const_iterator it1=derivatives.begin(); it1!=derivatives.end(); ++it1)
  {
    vector< parser* > derivs;
    for(vector< string >::const_iterator it2=(*it1).begin(); it2!=(*it1).end(); ++it2)
    {
      derivs.push_back(new parser(*it2));
    }
    derivs_parsers.push_back(derivs);
  }
}


parse_model::~parse_model()
{
  for(vector< parser* >::iterator it=funcs_parsers.begin(); it!=funcs_parsers.end(); ++it)
  {
    delete *it;
  }
  for(vector< vector< parser* > >::iterator it1=derivs_parsers.begin(); it1!=derivs_parsers.end(); ++it1)
  {
    for(vector< parser* >::iterator it2=(*it1).begin(); it2!=(*it1).end(); ++it2)
    {
      delete *it2;
    }
  }
}


int parse_model::get_n_fit_points()
{
  return n_fit_points;
}


int parse_model::get_n_parameters()
{
  return n_parameters;
}


int parse_model::get_n_constants()
{
  return n_constants;
}


void parse_model::set_parameters(const vector< double >& params)
{
  for(int p=0; p<n_parameters; ++p)
  {
    table[param_names[p]]=params[p];
  }
}


void parse_model::set_constants(const vector< double >& constants)
{
  for(int c=0; c<n_constants; ++c)
  {
    table[const_names[c]]=constants[c];
  }
}



double parse_model::eval_function(int fit_point)
{
  double d=funcs_parsers[fit_point]->parse(table);
  errors=funcs_parsers[fit_point]->get_no_of_errors();
  return d;
}


double parse_model::eval_derivative(int fit_point, int parameter)
{
  double d=derivs_parsers[fit_point][parameter]->parse(table);
  errors=derivs_parsers[fit_point][parameter]->get_no_of_errors();
  return d;
}

string parse_model::get_function_name(int fit_point)
{
  return func_names[fit_point];
}

string parse_model::get_parameter_name(int parameter)
{
  return param_names[parameter];
}

string parse_model::get_constant_name(int constant)
{
  return const_names[constant];
}

int parse_model::no_of_errors()
{
  return errors;
}
