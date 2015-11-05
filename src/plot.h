#ifndef PLOT_HH
#define PLOT_HH

#include <QWidget>
#include <vector>
#include <list>
#include <QPainter>
#include <QPoint>
#include <QList>
#include <QColor>


typedef QPair<double, double> Range;

/** Trivial container that contains a graph. */
class Graph: public QObject
{
  Q_OBJECT

public:
  explicit Graph(QObject *parent=0);
  Graph(const std::vector<double> &x, const std::vector<double> &y, QObject *parent=0);
  Graph(const Graph &other, QObject *parent=0);

  inline const QVector<double> &x() const { return _x; }
  inline const QVector<double> &y() const { return _y; }

  inline const Range &xRange() const { return _xRange; }
  inline const Range &yRange() const { return _yRange; }

  void appendValue(double x, double y);

signals:
  void changed();

protected:
  Range _xRange;
  Range _yRange;
  QVector<double> _x;
  QVector<double> _y;
};


class Plot : public QObject
{
  Q_OBJECT

public:
  explicit Plot(QObject *parent=0);

  inline const Range &xRange() const { return _xRange; }
  inline const Range &yRange() const { return _yRange; }

public slots:
  size_t addGraph(Graph *graph);
  size_t addGraph(const std::vector<double> &x, const std::vector<double> &y);
  inline const QList<Graph *> &graphs() const { return _graphs; }

signals:
  void changed();

protected slots:
  void _graphChanged();

protected:
  /** Plot range. */
  Range _xRange;
  /** Plot range. */
  Range _yRange;
  /** The graphs. */
  QList<Graph *> _graphs;
};


/** A trivial plot widget. */
class PlotView : public QWidget
{
  Q_OBJECT

public:
  /** Constructor. */
  explicit PlotView(Plot *plot, QWidget *parent = 0);

protected:
  /** Draws the plot. */
  void paintEvent(QPaintEvent *evt);
  /** Draws the graph. */
  void _drawGraphs(QPainter &painter);
  /** Draws the axes. */
  void _drawAxes(QPainter &painter);
  /** Draws the cursor/pointer. */
  void _drawPointer(QPainter &painter);
  /** Mouse click event handler. */
  void mouseReleaseEvent(QMouseEvent *evt);

  Plot *_plot;

  /** If @c true, the cursor will be drawn. */
  bool _pointer;
  /** Cursor position. */
  QPoint _pointer_pos;
  /** Cursor value. */
  QPointF _pointer_val;

protected:
  static QList<QColor> _defaultColors;
};

#endif // PLOT_HH
