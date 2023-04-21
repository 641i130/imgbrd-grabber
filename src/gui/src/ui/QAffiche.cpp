#include "ui/QAffiche.h"
#include <QMouseEvent>
#include <QMovie>
#include <QResizeEvent>


QAffiche::QAffiche(const QVariant &id, int border, QColor color, QWidget *parent)
	: QLabel(parent), m_lastPressed(Qt::NoButton)
{
	m_pressed = false;
	m_id = id;
	m_border = border;
	m_color = std::move(color);
	setText(QString());
}

void QAffiche::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton) {
		emit doubleClicked();
		emit doubleClicked(m_id.toInt());
	}
	QLabel::mouseDoubleClickEvent(e);
}

void QAffiche::mousePressEvent(QMouseEvent *e)
{
	m_lastPressed = e->button();
	m_pressed = e->button() == Qt::LeftButton || e->button() == Qt::MiddleButton || e->button() == Qt::RightButton;
	emit pressed();
	emit pressed(m_id.toInt());
	QLabel::mousePressEvent(e);
}

void QAffiche::mouseReleaseEvent(QMouseEvent *e)
{
	if (m_pressed && e->button() == Qt::LeftButton && hitLabel(e->pos())) {
		emit clicked();
		emit clicked(m_id.toInt());
		emit clicked(m_id.toString());
	} else if (m_pressed && e->button() == Qt::MiddleButton && hitLabel(e->pos())) {
		emit middleClicked();
		emit middleClicked(m_id.toInt());
		emit middleClicked(m_id.toString());
	} else if (m_pressed && e->button() == Qt::RightButton && hitLabel(e->pos())) {
		emit rightClicked();
		emit rightClicked(m_id.toInt());
		emit rightClicked(m_id.toString());
	}
	m_pressed = false;
	emit released();
	emit released(m_id.toInt());
	QLabel::mouseReleaseEvent(e);
}

void QAffiche::enterEvent(QEnterEvent *e)
{
	emit mouseOver();
	emit mouseOver(m_id.toInt());
	emit mouseOver(m_id.toString());
	QLabel::enterEvent(e);
}

void QAffiche::leaveEvent(QEvent *e)
{
	emit mouseOut();
	emit mouseOut(m_id.toInt());
	emit mouseOut(m_id.toString());
	QLabel::leaveEvent(e);
}

void QAffiche::resizeEvent(QResizeEvent *e)
{
	QMovie *mov = movie();
	if (mov != nullptr) {
		const QSize &movieSize = mov->currentPixmap().size();
		const QSize &newSize = e->size();
		if (newSize.width() < movieSize.width() || newSize.height() < movieSize.height()) {
			mov->setScaledSize(movieSize.scaled(newSize, Qt::KeepAspectRatio));
		}
	}
}

bool QAffiche::hitLabel(QPoint p)
{ return rect().contains(p); }
Qt::MouseButton QAffiche::lastPressed()
{ return m_lastPressed; }

void QAffiche::setImage(const QImage &image)
{ this->setPixmap(QPixmap::fromImage(image)); }
void QAffiche::setImage(const QPixmap &image)
{ this->setPixmap(image); }
