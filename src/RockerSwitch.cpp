#include "RockerSwitch.h"

#include <QLinearGradient>
#include <QPainter>

RockerSwitch::RockerSwitch(QWidget *parent) : QAbstractButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::TabFocus);
}

QSize RockerSwitch::sizeHint() const
{
    return { 54, 68 };
}

void RockerSwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled())
        p.setOpacity(0.45);

    const QRectF bezel = QRectF(rect()).adjusted(1, 1, -1, -1);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0x1b, 0x1b, 0x1b));
    p.drawRoundedRect(bezel, 6, 6);

    const bool on = isChecked();
    const QRectF face = bezel.adjusted(6, 6, -6, -6);

    if (on)
    {
        // Glow around the lit face.
        for (int i = 4; i >= 1; --i)
        {
            p.setBrush(QColor(255, 70, 70, 28));
            p.drawRoundedRect(face.adjusted(-i, -i, i, i), 5 + i, 5 + i);
        }
    }

    QLinearGradient grad(face.topLeft(), face.bottomLeft());
    if (on)
    {
        grad.setColorAt(0.0, QColor(0xff, 0x6a, 0x5e));
        grad.setColorAt(1.0, QColor(0xc6, 0x28, 0x28));
    } else
    {
        grad.setColorAt(0.0, QColor(0x5c, 0x5c, 0x5c));
        grad.setColorAt(1.0, QColor(0x3a, 0x3a, 0x3a));
    }
    p.setBrush(grad);
    p.drawRoundedRect(face, 5, 5);

    // The pressed half of the rocker is darker: top half down when ON ("I"), bottom half when OFF ("O").
    p.setBrush(QColor(0, 0, 0, 55));
    const QRectF half = on ? QRectF(face.left(), face.top(), face.width(), face.height() / 2)
                           : QRectF(face.left(), face.center().y(), face.width(), face.height() / 2);
    p.setClipRect(half);
    p.drawRoundedRect(face, 5, 5);
    p.setClipping(false);

    const QColor mark = on ? QColor(255, 255, 255) : QColor(0xc8, 0xc8, 0xc8);
    p.setBrush(Qt::NoBrush);
    QPen pen(mark, 3.0, Qt::SolidLine, Qt::RoundCap);
    p.setPen(pen);
    const qreal cx = face.center().x();
    // "I" in the top half
    p.drawLine(QPointF(cx, face.top() + face.height() * 0.12), QPointF(cx, face.top() + face.height() * 0.38));
    // "O" in the bottom half
    const qreal r = face.width() * 0.2;
    p.drawEllipse(QPointF(cx, face.top() + face.height() * 0.73), r, r);

    if (hasFocus())
    {
        p.setPen(QPen(QColor(0x42, 0xa5, 0xf5), 1.5));
        p.drawRoundedRect(bezel.adjusted(1, 1, -1, -1), 5, 5);
    }
}
