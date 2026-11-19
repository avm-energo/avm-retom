#pragma once

#include <QObject>
#include <QTimer>
#include "SignalSetMeta.h"
#include <memory>

// Owns the RetomDriver and lives on its own QThread: connect()/apply() block for seconds
// (1 s port settle, self-test, ~140 arm frames), so the GUI must never call them directly.
// All slots run serialised on the worker thread, so the driver is never used concurrently.
class RetomWorker : public QObject
{
    Q_OBJECT
public:
    explicit RetomWorker(QObject *parent = nullptr);
    ~RetomWorker() override;

public slots:
    void connectDevice(const QString &port, double maxU, double maxI);
    void disconnectDevice();
    void apply(const retom::RetomSignalSet &set);
    void off();
    void setOutContact(int number, bool closed);
    void verifyVoltage();
    void readInputs();
    void setPolling(bool on);

signals:
    void log(const QString &line);
    void stateChanged(bool connected, bool enabled, bool outputActive);
    void inputsRead(quint32 bits);
    void busy(bool on);
    void voltageCheckResult(bool ok, const QString &message);

private:
    template <typename F> void guarded(const QString &what, F &&f);
    void emitState();

    std::unique_ptr<retom::RetomDriver> driver_;
    QTimer *pollTimer_ = nullptr;
    int lastIo_ = -1;
};
