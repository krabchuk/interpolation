#include <QObject>
#include <QPainter>
#include <stdio.h>
#include <QMessageBox>
#include <QDebug>
#include <math.h>

#include "window.h"

#define DELTA 0.01
#define MIN 1e-15

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

  c[0] /= b[0];
  for (i = 1; i < n - 1; i++)
    {
      b[i] -= a[i - 1] * c[i - 1];
      c[i] /= b[i];
    }
  b[n - 1] -= a[n - 2] * c[n - 2];

  x[0] = rhs[0] / b[0];
  for (i = 1; i < n; i++)
    x[i] = (rhs[i] - a[i - 1] * x[i - 1]) / b[i];

  for (i = n - 2; i >= 0; i--)
    x[i] -= c[i] * x[i + 1];
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

  spline_left[0] = newton_x[2] - newton_x[1];
  spline_centr[0] = newton_x[2] - newton_x[0];
  spline_right[0] = 0;

  rhs[0] = (diff[1] * (newton_x[2] - newton_x[1]) * (2 * newton_x[2] + newton_x[1] - 3 * newton_x[0])
          + diff[2] * (newton_x[1] - newton_x[0]) * (newton_x[1] - newton_x[0]))
          / (newton_x[2] - newton_x[0]);

  for (int i = 1; i < n - 1; i++)
    {
      spline_left[i] = newton_x[i + 1] - newton_x[i];
      spline_centr[i] = 2 * (newton_x[i + 1] - newton_x[i - 1]);
      spline_right[i] = newton_x[i] - newton_x[i - 1];

      rhs[i] = 3 * diff[i - 1] * (newton_x[i + 1] - newton_x[i])
              + 3 * diff[i] * (newton_x[i] - newton_x[i - 1]);
    }

  spline_left[n - 1] = 0;
  spline_centr[n - 1] = newton_x[n - 1] - newton_x[(n - 1) - 2];
  spline_right[n - 1] = newton_x[(n - 1) - 1] - newton_x[(n - 1) - 2];

  rhs[n - 1] = (diff[n - 2] * (newton_x[n - 1] - newton_x[(n - 1) - 1]) * (newton_x[n - 1] - newton_x[(n - 1) - 1])
              + diff[n - 1] * (newton_x[(n - 1) - 1] - newton_x[(n - 1) - 2])
               * (3 * newton_x[n - 1] - newton_x[(n - 1) - 1] - 2 * newton_x[(n - 1) - 2]))
              / (newton_x[n - 1] - newton_x[(n - 1) - 2]);

  solve_matrix (spline_left, spline_centr, spline_right, rhs, answer, n);

  //calc exact coeffs

  for (int i = 0; i < n - 1; i++)
    {
      spline_c1[i] = newton_y[i];

      spline_c2[i] = answer[i];

      spline_c3[i] = (3 * diff[i + 1] - 2 * answer[i] - answer[i + 1]) / (newton_x[i + 1] - newton_x[i]);

      spline_c4[i] = (answer[i] + answer[i + 1] - 2 * diff[i + 1]) / ((newton_x[i + 1] - newton_x[i]) * (newton_x[i + 1] - newton_x[i]));
    }

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


























