#include "plot.h"
#include <inttypes.h>
#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QString>
#include <QRectF>
#include <cmath>


/* ******************************************************************************************** *
 * Implementation of Graph
 * ******************************************************************************************** */
Graph::Graph(QObject *parent)
  : QObject(parent), _xRange(0,0), _yRange(0,0), _x(), _y()
{
    // pass...
}

Graph::Graph(const std::vector<double> &x, const std::vector<double> &y, QObject *parent)
  : QObject(parent), _xRange(0,0), _yRange(0,0), _x(), _y()
{
  size_t N = std::min(x.size(), y.size());
  _x.reserve(N); _y.reserve(N);
  if (0 < N) {
    _xRange = Range(x[0], x[0]);
    _yRange = Range(y[0], y[0]);
  }
  for (size_t i=0; i<N; i++) {
    _x[i] = x[i]; _y[i] = y[i];
    _xRange.first = std::min(_xRange.first, x[i]);
    _xRange.second = std::max(_xRange.second, x[i]);
    _yRange.first = std::min(_yRange.first, y[i]);
    _yRange.second = std::max(_yRange.second, y[i]);
  }
}

Graph::Graph(const Graph &other, QObject *parent)
  : QObject(parent), _xRange(other._xRange), _yRange(other._yRange), _x(other._x), _y(other._y)
{
    // pass...
}

void
Graph::appendValue(double x, double y) {
  _x.push_back(x);
  _y.push_back(y);
  _xRange.first = std::min(_xRange.first, x);
  _xRange.second = std::max(_xRange.second, x);
  _yRange.first = std::min(_yRange.first, y);
  _yRange.second = std::max(_yRange.second, y);
  emit changed();
}


/* ******************************************************************************************** *
 * Implementation of Plot
 * ******************************************************************************************** */
Plot::Plot(QObject *parent)
  : QObject(parent), _xRange(0,0), _yRange(0,0), _graphs()
{
  // pass...
}

size_t
Plot::addGraph(const std::vector<double> &x, const std::vector<double> &y) {
  return addGraph(new Graph(x,y));
}

size_t
Plot::addGraph(Graph *graph) {
  if (0 == graph) { return -1; }
  // take ownership
  graph->setParent(this);
  // connect to changed event
  connect(graph, SIGNAL(changed()), this, SLOT(_graphChanged()));
  // Add to list
  size_t idx = _graphs.size();
  _graphs.push_back(graph);
  // update
  _graphChanged();
  // done
  return idx;
}

void
Plot::_graphChanged() {
  // Update plot ranges
  Range xRange(0,0), yRange(0,0);
  if (0 < _graphs.size()) {
    xRange = _graphs.front()->xRange();
    yRange = _graphs.front()->yRange();
  }
  QList<Graph *>::iterator graph = _graphs.begin();
  for (; graph != _graphs.end(); graph++) {
    xRange.first = std::min(xRange.first, (*graph)->xRange().first);
    xRange.second = std::max(xRange.second, (*graph)->xRange().second);
    yRange.first = std::min(yRange.first, (*graph)->yRange().first);
    yRange.second = std::max(yRange.second, (*graph)->yRange().second);
  }
  // Compute ranges for axes
  xRange.second = std::round(xRange.second*11)/10;
  yRange.second = std::round(yRange.second*11)/10;
  // signal update
  emit changed();
}


/* ******************************************************************************************** *
 * Implementation of PlotView
 * ******************************************************************************************** */
QList<QColor> PlotView::_defaultColors = QList<QColor>()
    << QColor(0, 0, 125) << QColor(125, 0, 0) << QColor(0, 125, 0) << QColor(125, 125, 0)
    << QColor(0, 125, 125) << QColor(125, 0, 125) << QColor(205, 79, 18) << QColor(255, 185, 24)
    << QColor(243, 250, 146) << QColor(105, 151, 102) << QColor(69, 47, 96)
    << QColor(224, 26, 53) << QColor(204, 15, 19) << QColor(63, 61, 153) << QColor(153, 61, 113)
    << QColor(61, 153, 86) << QColor(61, 90, 153) << QColor(153, 61, 144) << QColor(61, 121, 153)
    << QColor(132, 61, 153) << QColor(153, 78, 61) << QColor(98, 153, 61) << QColor(61, 151, 153)
    << QColor(101, 61, 153) << QColor(153, 61, 75);

PlotView::PlotView(Plot *plot, QWidget *parent)
  : QWidget(parent), _plot(plot), _pointer(false)
{
  setMinimumSize(640, 480);
  connect(_plot, SIGNAL(changed()), this, SLOT(update()));
}

void
PlotView::paintEvent(QPaintEvent *evt) {
  // First, paint widget background
  QWidget::paintEvent(evt);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setClipRect(evt->rect());
  painter.save();

  painter.fillRect(evt->rect(), Qt::white);

  _drawGraphs(painter);
  _drawAxes(painter);
  _drawPointer(painter);

  painter.restore();
}


void
PlotView::_drawAxes(QPainter &painter) {
  int32_t height = this->size().height();
  int32_t width = this->size().width();

  QPen pen(Qt::black);
  pen.setWidth(1); pen.setCosmetic(true);
  painter.setPen(pen);

  // Draw axes ticks:
  double dF = (_plot->xRange().second - _plot->xRange().first)/8;
  double F  = _plot->xRange().first;

  QFont font; font.setPointSize(10);
  QFontMetrics fm(font);
  painter.setFont(font);

  pen.setStyle(Qt::DashLine);
  painter.setPen(pen);
  for (size_t i=0; i<9; i++, F+=dF) {
    int x  = (i*width)/8;
    for (size_t j=0; j<9; j++) {
      int    y  = (j*height)/8;
      painter.drawPoint(x, y);
    }

    if ((0!=i)&&(8!=i)) {
      QString label = QString::number(F);
      QRectF bb = fm.boundingRect(label);
      painter.drawText(x-bb.width()/2, 3+fm.ascent(), label);
      painter.drawText(x-bb.width()/2, height-3-fm.descent(), label);
    }
  }

  double dv = (_plot->yRange().second-_plot->yRange().first)/8, v  = _plot->yRange().second-dv;
  for (size_t i=1; i<8; i++, v-=dv) {
    int y = (i*height)/8;
    QString label = tr("%1").arg(v);
    QRectF bb = fm.boundingRect(label);
    float shift = bb.height()/2 - fm.ascent();
    painter.drawText(3, y-shift, label);
    painter.drawText(width-bb.width()-3, y-shift, label);
  }
}


void
PlotView::_drawGraphs(QPainter &painter) {
  painter.save();

  size_t idx = 0;
  QPen pen(_defaultColors.at(idx));
  idx = (idx+1) % _defaultColors.size();
  pen.setWidth(2);
  pen.setStyle(Qt::SolidLine);
  painter.setPen(pen);

  double ppx = width()/(_plot->xRange().second-_plot->xRange().first);
  double ppy = height()/(_plot->yRange().second-_plot->yRange().first);
  QList<Graph *>::const_iterator graph = _plot->graphs().begin();
  for (; graph != _plot->graphs().end(); graph++) {
    size_t N = (*graph)->x().size();
    for (size_t j=1; j<N; j++) {
      int x1 = ppx*((*graph)->x()[j-1]-_plot->xRange().first);
      int y1 = height()-ppy*((*graph)->y()[j-1]-_plot->yRange().first);
      int x2 = ppx*((*graph)->x()[j]-_plot->xRange().first);
      int y2 = height()-ppy*((*graph)->y()[j]-_plot->yRange().first);
      painter.drawLine(x1,y1, x2,y2);
    }
    pen.setColor(_defaultColors.at(idx));
    painter.setPen(pen);
    idx = (idx+1) % _defaultColors.size();
  }

  painter.restore();
}

void
PlotView::_drawPointer(QPainter &painter) {
  if (! _pointer) { return; }

  painter.save();

  QPen pen(Qt::black);
  pen.setWidth(2);
  pen.setStyle(Qt::SolidLine);
  painter.setPen(pen);

  painter.drawLine(_pointer_pos.x()-5,_pointer_pos.y()-5,
                   _pointer_pos.x()+5,_pointer_pos.y()-5);
  painter.drawLine(_pointer_pos.x()+5,_pointer_pos.y()-5,
                   _pointer_pos.x(), _pointer_pos.y());
  painter.drawLine(_pointer_pos.x(), _pointer_pos.y(),
                   _pointer_pos.x()-5,_pointer_pos.y()-5);

  QString label;
  if (_pointer_val.x()<10e3) {
    label = tr("%1 dBm @%2 Hz").arg(QString::number(_pointer_val.y(), 'f', 1)).arg(QString::number(_pointer_val.x(), 'f', 1));
  } else {
    label = tr("%1 dBm @%2 kHz").arg(QString::number(_pointer_val.y(), 'f', 1)).arg(QString::number(_pointer_val.x()/1e3, 'f', 2));
  }
  painter.drawText(_pointer_pos.x()+10, _pointer_pos.y()-10, label);
  painter.restore();
}

void
PlotView::mouseReleaseEvent(QMouseEvent *evt) {
  QWidget::mouseReleaseEvent(evt);
  double dx = (_plot->xRange().second-_plot->xRange().first)/width();
  double dy = (_plot->xRange().second-_plot->xRange().first)/height();
  // Update pointer position
  _pointer_pos = QPoint(evt->pos());
  _pointer_val = QPointF(_pointer_pos.x()*dx + _plot->xRange().first,
                         (height()-_pointer_pos.y())*dy + _plot->yRange().first);
  _pointer = true;
  update();
}

