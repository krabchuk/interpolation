
#include <QPainter>
#include <stdio.h>
#include <math.h>

#include "window.h"

#define DEFAULT_A 0
#define DEFAULT_B 1
#define DEFAULT_N 4

#define MAX_N 1048576


static
double f_0 (double x)
{
  return x;
}

static
double f_1 (double x)
{
  return exp (x);
}

Window::Window (QWidget *parent)
  : QWidget (parent)
{
  parent_save = parent;

  a = DEFAULT_A;
  b = DEFAULT_B;
  n = DEFAULT_N;

  func_id = 1;
  method_id = 1;
  error = 0;
  m_name.append ("spline");
  newton_x = 0;
  newton_y = 0;
  spline_c1 = 0;
  spline_c2 = 0;
  spline_c3 = 0;
  spline_c4 = 0;
  answer = 0;
  diff = 0;
  rhs = 0;
  spline_centr = 0;
  spline_left = 0;
  spline_right = 0;
  change_func ();
}

Window::~Window ()
{
  destruction ();
}

void Window::allocation (int m)
{
  newton_x = new double [m + 5];
  memset (newton_x, 0, m + 5);

  newton_y = new double [m + 5];
  memset (newton_y, 0, m + 5);

  spline_c1 = new double [m + 5];
  spline_c2 = new double [m + 5];
  spline_c3 = new double [m + 5];
  spline_c4 = new double [m + 5];

  memset (spline_c1, 0, m + 5);
  memset (spline_c2, 0, m + 5);
  memset (spline_c3, 0, m + 5);
  memset (spline_c4, 0, m + 5);

  spline_centr = new double [m + 5];
  spline_left = new double [m + 5];
  spline_right = new double [m + 5];

  memset (spline_centr, 0, m + 5);
  memset (spline_left, 0, m + 5);
  memset (spline_right, 0, m + 5);

  answer = new double [m + 5];
  memset (answer, 0, m + 5);

  diff = new double [m + 5];
  memset (diff, 0, m + 5);

  rhs = new double [m + 5];
  memset (rhs, 0, m + 5);
}

void Window::destruction ()
{
  if (newton_x)
    delete [] newton_x;
  if (newton_y)
    delete [] newton_y;
  if (spline_c1)
    delete [] spline_c1;
  if (spline_c2)
    delete [] spline_c2;
  if (spline_c3)
    delete [] spline_c3;
  if (spline_c4)
    delete [] spline_c4;
  if (spline_centr)
    delete [] spline_centr;
  if (spline_left)
    delete [] spline_left;
  if (spline_right)
    delete [] spline_right;
  if (answer)
    delete [] answer;
  if (diff)
    delete [] diff;
  if (rhs)
    delete [] rhs;
}

void
Window::exit_all ()
{
  parent_save->close ();
  delete this;
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
      || b - a < 1.e-10
      || (argc > 3 && sscanf (argv[3], "%d", &n) != 1)
      || n <= 0)
    return -2;

  newton_x = 0;
  newton_y = 0;
  spline_c1 = 0;
  spline_c2 = 0;
  spline_c3 = 0;
  spline_c4 = 0;
  answer = 0;
  diff = 0;
  rhs = 0;
  spline_centr = 0;
  spline_left = 0;
  spline_right = 0;

  return 0;
}


/// change current function for drawing
void Window::change_func ()
{
  func_id = (func_id + 1) % 2;

  switch (func_id)
    {
      case 0:
        f_name = "f (x) = x";
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
  destruction ();
  allocation (2 * n);
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
