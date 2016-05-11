#define _GNU_SOURCE
#include "strm.h"
#include <stdlib.h>

int
sort_cmp(const void* a_p, const void* b_p)
{
  strm_value av = *(strm_value*)a_p;
  strm_value bv = *(strm_value*)b_p;
  double a, b;

  if (strm_num_p(av)) {
    a = strm_value_flt(av);
  }
  else {
    if (strm_num_p(bv)) {
      return 1;
    }
    return 0;
  }
  if (strm_num_p(bv)) {
    b = strm_value_flt(bv);
  }
  else {
    return -1;
  }
  if(a > b)
    return 1;
  else if(a < b)
    return -1;
  return 0;
}

#if defined(__APPLE__) || defined(__FreeBSD__)
#define qsort_arg(p,nmem,size,cmp,arg) qsort_r(p,nmem,size,arg,cmp)
#define cmp_args(a,b,c) (c,a,b)
#else
#define qsort_arg(p,nmem,size,cmp,arg) qsort_r(p,nmem,size,cmp,arg)
#define cmp_args(a,b,c) (a,b,c)
#endif

struct sort_arg {
  strm_stream* strm;
  strm_value func;
};

int
sort_cmpf cmp_args(const void* a_p, const void* b_p, void* arg)
{
  strm_value args[2];
  struct sort_arg* a = arg;
  strm_value val;
  strm_int cmp;

  args[0] = *(strm_value*)a_p;
  args[1] = *(strm_value*)b_p;

  if (strm_funcall(a->strm, a->func, 2, args, &val) == STRM_NG) {
    return 0;
  }
  if (!strm_num_p(val)) {
    return 0;
  }
  cmp = strm_value_int(val);
  if(cmp > 0)
    return 1;
  else if(cmp < 0)
    return -1;
  return 0;
}

static int
ary_sort(strm_stream* strm, int argc, strm_value* args, strm_value* ret)
{
  strm_array ary;

  switch (argc) {
  case 1:
  case 2:
    {
      strm_value* p;
      strm_int len;

      ary = strm_ary_new(strm_ary_ptr(args[0]), strm_ary_len(args[0]));
      p = strm_ary_ptr(ary);
      len = strm_ary_len(ary);
      if (argc == 1) {
        qsort(p, len, sizeof(strm_value), sort_cmp);
      }
      else {
        struct sort_arg arg;

        arg.strm = strm;
        arg.func = args[1];
        qsort_arg(p, len, sizeof(strm_value), sort_cmpf, &arg);
      }
      *ret = strm_ary_value(ary);
      return STRM_OK;
    }
    break;
  default:
    strm_raise(strm, "wrong number of arguments");
    return STRM_NG;
  }
}

static int
num_cmp(strm_stream* strm, int argc, strm_value* args, strm_value* ret)
{
  double a, b;
  strm_int cmp;

  if (argc != 2) {
    strm_raise(strm, "wrong number of arguments");
    return STRM_NG;
  }
  if (!strm_num_p(args[0]) || !strm_num_p(args[1])) {
    strm_raise(strm, "non number comparison");
    return STRM_NG;
  }
  a = strm_value_flt(args[0]);
  b = strm_value_flt(args[1]);
  if(a > b)
    cmp = 1;
  else if(a < b)
    cmp = -1;
  else
    cmp = 0;
  *ret = strm_int_value(cmp);
  return STRM_OK;
}

void
strm_sort_init(strm_state* state)
{
  strm_var_def(strm_array_ns, "sort", strm_cfunc_value(ary_sort));
  strm_var_def(state, "cmp", strm_cfunc_value(num_cmp));
}