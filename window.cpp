
#include <QPainter>
#include <stdio.h>
#include <math.h>

#include "window.h"

#define DEFAULT_A -10
#define DEFAULT_B 10
#define DEFAULT_N 8

#define MAX_N 1048576


static
double f_0 (double x)
{
  return x + sin (x);
}

static
double f_1 (double x)
{
  return x * x;
}

Window::Window (QWidget *parent)
  : QWidget (parent)
{
  a = DEFAULT_A;
  b = DEFAULT_B;
  n = DEFAULT_N;

  newton_x = new double [2 * MAX_N];
  memset (newton_x, 0, 2 * MAX_N);

  newton_y = new double [2 * MAX_N];
  memset (newton_y, 0, 2 * MAX_N);

  spline_c1 = new double [2 * MAX_N];
  spline_c2 = new double [2 * MAX_N];
  spline_c3 = new double [2 * MAX_N];
  spline_c4 = new double [2 * MAX_N];

  memset (spline_c1, 0, 2 * MAX_N);
  memset (spline_c2, 0, 2 * MAX_N);
  memset (spline_c3, 0, 2 * MAX_N);
  memset (spline_c4, 0, 2 * MAX_N);

  spline_centr = new double [2 * MAX_N];
  spline_left = new double [2 * MAX_N];
  spline_right = new double [2 * MAX_N];

  memset (spline_centr, 0, 2 * MAX_N);
  memset (spline_left, 0, 2 * MAX_N);
  memset (spline_right, 0, 2 * MAX_N);

  answer = new double [2 * MAX_N];
  memset (answer, 0, 2 * MAX_N);

  diff = new double [2 * MAX_N];
  memset (diff, 0, 2 * MAX_N);

  rhs = new double [2 * MAX_N];
  memset (rhs, 0, 2 * MAX_N);

  func_id = 0;
  method_id = 0;
  error = 0;
  m_name = "newton";
  change_func ();
}

Window::~Window ()
{
  delete [] newton_x;
  delete [] newton_y;

  delete [] spline_c1;
  delete [] spline_c2;
  delete [] spline_c3;
  delete [] spline_c4;

  delete [] spline_centr;
  delete [] spline_left;
  delete [] spline_right;

  delete [] diff;

  delete [] rhs;
}

QSize Window::minimumSizeHint () const
{
  return QSize (100, 100);
}

QSize Window::sizeHint () const
{
  return QSize (1000, 1000);
}

int Window::parse_command_line (int argc, char *argv[])
{
  if (argc == 1)
    return 0;

  if (argc == 2)
    return -1;

  if (   sscanf (argv[1], "%lf", &a) != 1
      || sscanf (argv[2], "%lf", &b) != 1
      || b - a < 1.e-6
      || (argc > 3 && sscanf (argv[3], "%d", &n) != 1)
      || n <= 0
      || n > MAX_N)
    return -2;

  return 0;
}

/// change current function for drawing
void Window::change_func ()
{
  func_id = (func_id + 1) % 2;

  switch (func_id)
    {
      case 0:
        f_name = "f (x) = x + sin (x)";
        f = f_0;
        break;
      case 1:
        f_name = "f (x) = x * x";
        f = f_1;
        break;
    }

  update_data ();
  update ();
}

void Window::change_method ()
{
  method_id = (method_id + 1) % 4;

  if ((method_id == 0 || method_id == 2) && n > 64)
    {
      n = 64;
    }
  if ((method_id == 1 || method_id == 3) && n < 3)
    {
      n = 4;
    }

  update_data ();
  update ();
}

void Window::increase_dots ()
{
  if (method_id == 0 || method_id == 2)
    {
      if (n > 30)
        error = "too many dots";
      else
        {
          n *= 2;
          error = 0;
        }
    }
  else
    {
      if (n > MAX_N / 2)
        error = "too many dots";
      else
        {
          n *= 2;
          error = 0;
        }
    }
  update_data ();
  update ();
}

void Window::decrease_dots ()
{
  if (method_id == 0 || method_id == 2)
    {
      if (n < 3)
        error = "too less dots";
      else
        {
          n /= 2;
          error = 0;
        }
    }
  else
    {
      if (n < 5)
        error = "too less dots";
      else
        {
          n /= 2;
          error = 0;
        }
    }
  update_data ();
  update ();
}

void
Window::update_data ()
{
  switch (method_id)
    {
    case 0:
      {
        init_newton ();
        break;
      }
    case 1:
      {
        init_spline ();
        break;
      }
    case 2:
      {
        init_newton ();
        break;
      }
    case 3:
      {
        init_spline ();
        break;
      }
    }
}

/// render graph
void Window::paintEvent (QPaintEvent * /* event */)
{  
  QPainter painter (this);

  // save current Coordinate System
  painter.save ();

  // init
  init_painter (&painter);

  if (method_id < 2)
    {
      // draw exact function

      painter.setPen ("orange");

      double step = (b - a) / width ();
      double dot_x1 = 0, dot_y1 = 0, dot_x2 = 0, dot_y2 = 0;

      dot_x1 = a;
      dot_y1 = f (a);

      for (int i = 1; i < width(); i++)
        {
          dot_x2 = a + i * step;
          dot_y2 = f (dot_x2);

          painter.drawLine (QPointF(dot_x1, dot_y1), QPointF(dot_x2, dot_y2));

          dot_x1 = dot_x2;
          dot_y1 = dot_y2;
        }

    }

  switch (method_id)
    {
    case 0:
      {
        painter.setPen ("blue");

        double dot_x1 = 0, dot_y1 = 0, dot_x2 = 0, dot_y2 = 0;
        double step = (b - a) / width ();

        dot_x1 = a;
        dot_y1 = calc_newton (a);

        for (int i = 1; i < width(); i++)
          {
            dot_x2 = a + i * step;
            dot_y2 = calc_newton (dot_x2);

            painter.drawLine (QPointF(dot_x1, dot_y1), QPointF(dot_x2, dot_y2));

            dot_x1 = dot_x2;
            dot_y1 = dot_y2;
          }

        painter.drawLine (QPointF(dot_x1, dot_y1), QPointF(b, f (b)));

        break;
      }
    case 1:
      {
        painter.setPen ("green");

        double dot_x1 = 0, dot_y1 = 0, dot_x2 = 0, dot_y2 = 0;
        double step = (b - a) / width ();

        dot_x1 = a;
        dot_y1 = calc_spline (a);

        for (int i = 1; i < width(); i++)
          {
            dot_x2 = a + i * step;
            dot_y2 = calc_spline (dot_x2);

            painter.drawLine (QPointF(dot_x1, dot_y1), QPointF(dot_x2, dot_y2));

            dot_x1 = dot_x2;
            dot_y1 = dot_y2;
          }

        painter.drawLine (QPointF(dot_x1, dot_y1), QPointF(b, f (b)));

        break;
      }
    case 2:
      {
        painter.setPen ("blue");

        double dot_x1 = 0, dot_y1 = 0;
        double step = (b - a) / width ();

        for (int i = 0; i < width(); i++)
          {
            dot_x1 = a + i * step;
            dot_y1 = fabs (f (dot_x1) - calc_newton (dot_x1));

            painter.drawLine (QPointF(dot_x1, 0), QPointF(dot_x1, dot_y1));

          }

        break;
      }
    case 3:
      {
        painter.setPen ("green");

        double dot_x1 = 0, dot_y1 = 0;
        double step = (b - a) / width ();

        for (int i = 0; i < width(); i++)
          {
            dot_x1 = a + i * step;
            dot_y1 = fabs (f (dot_x1) - calc_spline (dot_x1));

            painter.drawLine (QPointF(dot_x1, 0), QPointF(dot_x1, dot_y1));

          }

        break;
      }
    }


  painter.restore ();
}
