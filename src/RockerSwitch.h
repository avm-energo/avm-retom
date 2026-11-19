#pragma once

#include <QAbstractButton>

// A checkable rocker switch drawn in code (black bezel, I on top, O at the bottom), modelled on a
// panel rocker. Off: unlit grey face. On: lit red face with a glow.
class RockerSwitch : public QAbstractButton
{
    Q_OBJECT
public:
    explicit RockerSwitch(QWidget *parent = nullptr);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
};
