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
Window::init_painter (QPainter *painter)
{
  double delta_y = 0, delta_x = (b - a) / n;

  double dot_x1 = 0, dot_y1 = 0;
  double max_y = -100000, min_y = 100000;

  if (method_id > 1)
    {
      double step = (b - a) / width ();
      int i = 0;
      max_y = 0;
      min_y = 0;

      if (method_id == 2)
        {

          for (dot_x1 = a, i = 0; i < width (); i++)
            {
              dot_x1 = a + step * i;
              dot_y1 = fabs (f (dot_x1) - calc_newton (dot_x1));

              if (dot_y1 > max_y)
                max_y = dot_y1;
            }
        }

      if (method_id == 3)
        {

          for (dot_x1 = a, i = 0; i < width (); i++)
            {
              dot_x1 = a + step * i;
              dot_y1 = fabs (f (dot_x1) - calc_spline (dot_x1));

              if (dot_y1 > max_y)
                max_y = dot_y1;
            }
        }

      if (max_y < 0.0001)
        max_y = 0.0001;
//      if (fabs (min_y - max_y) < MIN_FOR_DRAW)
//        {
//          min_y = 1.;
//          max_y = 2.;
//        }
    }
  else
    {
      double delta = delta_x < DELTA ? delta_x : DELTA;

      for (dot_x1 = a; dot_x1 - b < MIN; dot_x1 += delta)
        {
          dot_y1 = f(dot_x1);
          if (dot_y1 < min_y)
            min_y = dot_y1;
          if (dot_y1 > max_y)
            max_y = dot_y1;
        }
    }

  // graph label
  m_name.clear();
  switch (method_id)
    {
    case 0:
      {
        m_name.append("newton; n=");
        break;
      }
    case 1:
      {
        m_name.append("spline; n=");
        break;
      }
    case 2:
      {
        m_name.append("residual newton; n=");
        break;
      }
    case 3:
      {
        m_name.append ("residual spline; n =");
        break;
      }
    }

  m_name.append (QString::number(n));

  switch (method_id)
    {
    case 0:
      {
        painter->setPen ("blue");
        break;
      }
    case 1:
      {
        painter->setPen ("green");
        break;
      }
    case 2:
      {
        painter->setPen ("blue");
        break;
      }
    case 3:
      {
        painter->setPen ("green");
        break;
      }
    }

  painter->drawText (0, 40, m_name);

  painter->setPen ("black");
  painter->drawText (0, 20, f_name);

  if (error)
    {
      painter->setPen ("red");
      painter->drawText (0, 60, error);
    }

//  if (method_id > 1)
//    {
//      painter->translate (0.5 * width (), 0.5 * height ());
//      painter->scale (width () / (b - a), -height () / max_y);
//      painter->translate (-0.5 * (a + b), -0.5 * (min_y + max_y));


//      delta_y = 0.01 * (max_y - min_y);
//      min_y -= delta_y;
//      max_y += delta_y;

//      //axes
//      painter->setPen ("black");
//      painter->drawLine (a, 0, b, 0);
//      painter->drawLine (0, min_y, 0, max_y);
//    }
//  else
//    {

//    }

  painter->translate (0.5 * width (), 0.5 * height ());
  painter->scale (width () / (b - a), -height () / (max_y - min_y));
  painter->translate (-0.5 * (a + b), -0.5 * (min_y + max_y));

  if (method_id > 1)
    {

    }
  else
    {
      delta_y = 0.01 * (max_y - min_y);
      min_y -= delta_y;
      max_y += delta_y;
    }



  //axes
  painter->setPen ("black");
  painter->drawLine (a, 0, b, 0);
  painter->drawLine (0, min_y, 0, max_y);

}
