#include "RetomWorker.h"

RetomWorker::RetomWorker(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<retom::RetomSignalSet>();
    pollTimer_ = new QTimer(this);
    pollTimer_->setInterval(1000);
    connect(pollTimer_, &QTimer::timeout, this, &RetomWorker::readInputs);
}

RetomWorker::~RetomWorker()
{
    if (driver_)
    {
        try { driver_->disconnect(); } catch (...) {}
    }
}

template <typename F> void RetomWorker::guarded(const QString &what, F &&f)
{
    emit busy(true);
    try
    {
        f();
    } catch (const std::exception &e)
    {
        emit log(QStringLiteral("ERROR %1: %2").arg(what, QString::fromUtf8(e.what())));
    }
    emitState();
    emit busy(false);
}

void RetomWorker::emitState()
{
    emit stateChanged(driver_ && driver_->isConnected(), driver_ && driver_->isEnabled(),
        driver_ && driver_->isOutputActive());
}

void RetomWorker::connectDevice(const QString &port, double maxU, double maxI)
{
    guarded(QStringLiteral("connect"), [&] {
        retom::RetomConfig cfg;
        cfg.maxU = maxU;
        cfg.maxI = maxI;
        driver_ = std::make_unique<retom::RetomDriver>(
            cfg, std::make_unique<retom::FtdiTransport>(retom::kRetomVid, retom::kRetomPid, port));
        emit log(QStringLiteral("Connecting to %1 (self-test/calibration takes several seconds)...").arg(port));
        try
        {
            driver_->connect(true);
        } catch (...)
        {
            driver_.reset();
            throw;
        }
        emit log(QStringLiteral("Connected and enabled."));
    });
}

void RetomWorker::disconnectDevice()
{
    pollTimer_->stop();
    guarded(QStringLiteral("disconnect"), [&] {
        if (!driver_)
            return;
        // disconnect() does not open discrete outputs, so release all four ourselves first.
        for (int n = 0; n < 4; ++n)
        {
            try { driver_->setOutContact(n, false); } catch (...) {}
        }
        try
        {
            driver_->disconnect();
        } catch (...)
        {
            driver_.reset();
            throw;
        }
        driver_.reset();
        emit log(QStringLiteral("Disconnected (analog output off)."));
    });
}

void RetomWorker::apply(const retom::RetomSignalSet &set)
{
    guarded(QStringLiteral("apply"), [&] {
        if (!driver_)
            throw retom::RetomNotConnected();
        driver_->apply(set);
        emit log(QStringLiteral("Applied: U=%1/%2/%3 V  I=%4/%5/%6 A  f=%7 Hz")
                     .arg(set.u[0]).arg(set.u[1]).arg(set.u[2]).arg(set.i[0]).arg(set.i[1]).arg(set.i[2])
                     .arg(set.freq));
    });
}

void RetomWorker::off()
{
    guarded(QStringLiteral("off"), [&] {
        if (!driver_)
            throw retom::RetomNotConnected();
        driver_->safetyStop();
        // safetyStop() also disable()s; bring the device back to a usable state.
        driver_->enable();
        emit log(QStringLiteral("Output off."));
    });
}

void RetomWorker::setOutContact(int number, bool closed)
{
    guarded(QStringLiteral("outcontact"), [&] {
        if (!driver_)
            throw retom::RetomNotConnected();
        driver_->setOutContact(number, closed);
        emit log(QStringLiteral("Output contact %1 -> %2").arg(number + 1).arg(closed ? "closed" : "open"));
    });
}

void RetomWorker::verifyVoltage()
{
    guarded(QStringLiteral("voltage check"), [&] {
        if (!driver_)
            throw retom::RetomNotConnected();
        emit log(QStringLiteral("Voltage check: applying 10 V / 0.1 A on phase A, settling 5 s..."));
        try
        {
            driver_->verifyVoltageOutput();
        } catch (const retom::RetomVoltageStageNotReady &e)
        {
            emit log(QStringLiteral("Voltage check FAILED: voltage stage not ready."));
            emit voltageCheckResult(false, QString::fromUtf8(e.what()));
            return;
        }
        emit log(QStringLiteral("Voltage check OK (output is off again)."));
        emit voltageCheckResult(true, QStringLiteral("Voltage stage is ready."));
    });
}

void RetomWorker::readInputs()
{
    if (!driver_ || !driver_->isConnected())
        return;
    try
    {
        emit inputsRead(driver_->readInputContacts());
    } catch (const std::exception &e)
    {
        emit log(QStringLiteral("ERROR readInputContacts: %1").arg(QString::fromUtf8(e.what())));
    }
}

void RetomWorker::setPolling(bool on)
{
    if (on)
        pollTimer_->start();
    else
        pollTimer_->stop();
}
