%module(directors="1") smartscompiler
%{

#include "smartsmatcher.h"
#include "smartspattern.h"

%}

%include "std_string.i"
%include "std_vector.i"
%include "std_pair.i"


namespace std {
  %template(vector_bond) vector<SC::SmartsBond>;
}

%include "smartspattern.h"
%include "smartsmatcher.h"

%feature("director") SC::SmartsPattern;
