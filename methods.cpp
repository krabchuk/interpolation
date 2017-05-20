#include <QObject>
#include <QPainter>
#include <stdio.h>
#include <QMessageBox>
#include <QDebug>
#include <math.h>

#include "window.h"

#define DELTA 0.01
#define MIN 1e-15
#define DEBUG 0

void
Window::init_newton ()
{
  double step = (b - a) / (n - 1);

  for (int i = 0; i < n; i++)
    {
      newton_x[i] = a + step * i;
      newton_y[i] = f (a + step * i);
    }

  for (int i = 1; i < n; i++)
    {
      for (int j = n - 1; j >= i; j--)
        {
          newton_y[j] = (newton_y[j] - newton_y[j - 1]) / (newton_x[j] - newton_x[j - i]);
        }
    }

}

double
Window::calc_newton (double x)
{
  double answer = 0;

  for (int i = n - 2; i >= 0; i--)
    {
      answer += newton_y[i + 1];
      answer *= (x - newton_x[i]);
    }
  answer += newton_y[0];

  return answer;
}




static inline void
solve_matrix (double *a, double *b, double *c, double *rhs, double *x, int n)
{
  int i;


  for (i = 0; i < n - 1; i++)
    {
      c[i] /= b[i];
      rhs[i] /= b[i];

      b[i + 1] -= c[i] * a[i + 1];
      rhs[i + 1] -= rhs[i] * a[i + 1];
    }
  rhs[n - 1] /= b[n - 1];

  for (i = n - 1; i > 0; i--)
    {
      rhs[i - 1] -= rhs[i] * c[i - 1];
    }

  for (i = 0; i < n; i++)
    {
      x[i] = rhs[i];
    }
}


void
Window::init_spline ()
{
  // calc divided dif
  double step = (b - a) / (n - 1);

  for (int i = 0; i < n; i++)
    {
      newton_x[i] = a + step * i;
      newton_y[i] = f (a + step * i);
    }

  for (int i = n - 1; i > 0; i--)
    {
      diff[i] = (newton_y[i] - newton_y[i - 1]) / (newton_x[i] - newton_x[i - 1]);
    }

  //build matrix and rhs

  spline_left[0] = 0;
  spline_centr[0] = newton_x[2] - newton_x[1];
  spline_right[0] = newton_x[2] - newton_x[0];

  rhs[0] = (diff[1] * (newton_x[2] - newton_x[1]) * (2 * newton_x[2] + newton_x[1] - 3 * newton_x[0])
          + diff[2] * (newton_x[1] - newton_x[0]) * (newton_x[1] - newton_x[0]))
          / (newton_x[2] - newton_x[0]);
  //printf ("(%f * (%f - %f) * (2 * %f + %f - 3 * %f) + %f * (%f - %f * %f - %f))\n", diff[1],newton_x[2],newton_x[1],newton_x[2],newton_x[1],newton_x[0],diff[2],newton_x[1],newton_x[0],newton_x[1],newton_x[0]);

  for (int i = 1; i < n - 1; i++)
    {
      spline_left[i] = newton_x[i + 1] - newton_x[i];
      spline_centr[i] = 2 * (newton_x[i + 1] - newton_x[i - 1]);
      spline_right[i] = newton_x[i] - newton_x[i - 1];

      rhs[i] = 3 * diff[i] * (newton_x[i + 1] - newton_x[i])
              + 3 * diff[i + 1] * (newton_x[i] - newton_x[i - 1]);
    }

  spline_left[n - 1] = newton_x[n - 1] - newton_x[(n - 1) - 2];
  spline_centr[n - 1] = newton_x[(n - 1) - 1] - newton_x[(n - 1) - 2];
  spline_right[n - 1] = 0;

  rhs[n - 1] = (diff[n - 2] * (newton_x[n - 1] - newton_x[(n - 1) - 1]) * (newton_x[n - 1] - newton_x[(n - 1) - 1])
              + diff[n - 1] * (newton_x[(n - 1) - 1] - newton_x[(n - 1) - 2])
               * (3 * newton_x[n - 1] - newton_x[(n - 1) - 1] - 2 * newton_x[(n - 1) - 2]))
              / (newton_x[n - 1] - newton_x[(n - 1) - 2]);

#if DEBUG
  for (int i = 0; i < n - 1; i++)
    {
      printf ("i = %d c1 = %f c2 = %f c3 = %f c4 = %f\n",i, spline_c1[i], spline_c2[i], spline_c3[i], spline_c4[i]);
      printf ("diff = %f\n", diff[i + 1]);
    }
  printf ("\n");
  for (int i = 0; i < n; i++)
    {
      printf ("i = %d left = %f center = %f right = %f\n", i, spline_left[i], spline_centr[i], spline_right[i]);
      printf ("i = %d rhs = %f\n", i, rhs[i]);
      printf ("i = %d newton x = %f newton y = %f\n", i, newton_x[i], newton_y[i]);
    }
#endif

  solve_matrix (spline_left, spline_centr, spline_right, rhs, answer, n);
#if DEBUG
  for (int i = 0; i < n; i++)
    {
      printf ("rhs = %f\n", rhs[i]);
    }
#endif

  //calc exact coeffs

  for (int i = 0; i < n - 1; i++)
    {
      spline_c1[i] = newton_y[i];

      spline_c2[i] = answer[i];

      spline_c3[i] = (3 * diff[i + 1] - 2 * answer[i] - answer[i + 1]) / (newton_x[i + 1] - newton_x[i]);

      spline_c4[i] = (answer[i] + answer[i + 1] - 2 * diff[i + 1]) / ((newton_x[i + 1] - newton_x[i]) * (newton_x[i + 1] - newton_x[i]));
    }

#if DEBUG
  for (int i = 0; i < n - 1; i++)
    {
      printf ("i = %d c1 = %f c2 = %f c3 = %f c4 = %f\n",i, spline_c1[i], spline_c2[i], spline_c3[i], spline_c4[i]);
    }
  for (int i = 0; i < n; i++)
    {
      printf ("answer = %f\n", answer[i]);
    }
#endif
}

double
Window::calc_spline (double x)
{
  double step = (b - a) / (n - 1);
  int i = (int)trunc ((x - a) / step);

  double value = spline_c1[i] + spline_c2[i] * (x - newton_x[i]) + spline_c3[i] * (x - newton_x[i]) * (x - newton_x[i])
                + spline_c4[i] * (x - newton_x[i]) * (x - newton_x[i]) * (x - newton_x[i]);
  return value;
}


























