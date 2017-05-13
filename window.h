
#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>


class Window : public QWidget
{
  Q_OBJECT

private:
  int func_id;
  int method_id;
  QString f_name;
  QString m_name;
  double a;
  double b;
  int n;
  double (*f) (double);
  double *newton_y;
  double *newton_x;
  const char *error;
  double *spline_left;
  double *spline_centr;
  double *spline_right;
  double *spline_c1;
  double *spline_c2;
  double *spline_c3;
  double *spline_c4;
  double *rhs;
  double *answer;
  double *diff;

public:
  Window (QWidget *parent);
  ~Window ();
  QSize minimumSizeHint () const;
  QSize sizeHint () const;

  int parse_command_line (int argc, char *argv[]);

public slots:
  void change_func ();
  void change_method ();
  void increase_dots ();
  void decrease_dots ();

 public:

  void update_data ();
  void init_painter (QPainter *painter);

  void init_newton ();
  double calc_newton (double x);

  void init_spline ();
  double calc_spline (double x);


protected:
  void paintEvent (QPaintEvent *event);
};

#endif
