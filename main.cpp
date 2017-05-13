
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QAction>
#include <QMenuBar>
#include <QMessageBox>

#include "window.h"

int main (int argc, char *argv[])
{
  QApplication app (argc, argv);

  QMainWindow *window = new QMainWindow;
  QMenuBar *tool_bar = new QMenuBar (window);
  Window *graph_area = new Window (window);
  QAction *action;
  
  if (graph_area->parse_command_line (argc, argv))
    {
      QMessageBox::warning (0, "Wrong input arguments!", 
                            "Wrong input arguments!");
      return -1;
    }

      
  action = tool_bar->addAction ("Change &function", graph_area, SLOT (change_func ()));
  action->setShortcut (QString ("Ctrl+F"));

  action = tool_bar->addAction ("Change &method", graph_area, SLOT (change_method ()));
  action->setShortcut (QString ("Ctrl+M"));

  action = tool_bar->addAction ("&Increase dots amount x2", graph_area, SLOT (increase_dots ()));
  action->setShortcut (QString ("Ctrl+I"));

  action = tool_bar->addAction ("&Decrease dots amount x2", graph_area, SLOT (decrease_dots ()));
  action->setShortcut (QString ("Ctrl+D"));

  action = tool_bar->addAction ("E&xit", window, SLOT (close ()));
  action->setShortcut (QString ("Ctrl+X"));

  tool_bar->setMaximumHeight (30);

  window->setMenuBar (tool_bar);
  window->setCentralWidget (graph_area);
  window->setWindowTitle ("Graph");

  window->show ();
  return app.exec ();
}
