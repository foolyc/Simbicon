#include <stdlib.h>
#include <gsl/vector/gsl_vector.h>
#include <gsl/gsl_errno.h>



#define BASE_DOUBLE
#include <gsl/templates_on.h>
#include <gsl/vector/prop_source.c>
#include <gsl/templates_off.h>
#undef BASE_DOUBLE

