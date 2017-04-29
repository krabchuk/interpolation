
#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class Window : public QWidget
{
  Q_OBJECT

private:
  int func_id;
  const char *f_name;
  double a;
  double b;
  int n;
  double (*f) (double);

public:
  Window (QWidget *parent);

  QSize minimumSizeHint () const;
  QSize sizeHint () const;

  int parse_command_line (int argc, char *argv[]);

public slots:
  void change_func ();

protected:
  void paintEvent (QPaintEvent *event);
};

#endif
