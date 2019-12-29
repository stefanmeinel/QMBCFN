#ifndef PARSE_MODEL_H
#define PARSE_MODEL_H

#include "abstract_global_model.h"

#include <vector>
#include <map>
#include <string>
#include "parser.h"

using namespace std;

class parse_model : public abstract_global_model
{
  public:

    parse_model(const vector< string >& functions,
                const vector< string >& parameters,
                const vector< string >& constants,
                const vector< vector< string > >& derivatives);

// derived functions

    ~parse_model();

    int get_n_fit_points();
    int get_n_parameters();
    int get_n_constants();

    void set_parameters(const vector< double >& parameters);
    void set_constants(const vector< double >& constants);

    double eval_function(int fit_point);
    double eval_derivative(int fit_point, int parameter);

    int no_of_errors();

    string get_function_name(int n_fit_point);
    string get_parameter_name(int parameter);
    string get_constant_name(int constant);

  private:

    int n_fit_points;
    int n_parameters;
    int n_constants;

    vector< string > param_names;
    vector< string > const_names;
    vector< string > func_names;

    vector< parser* > funcs_parsers;
    vector< vector< parser* > > derivs_parsers;

    map< string, double > table;

    int errors;
};

#endif
