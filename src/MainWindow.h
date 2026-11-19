#pragma once

#include <QMainWindow>
#include <QThread>
#include <array>
#include "SignalSetMeta.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;
class QCheckBox;
class RetomWorker;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

signals:
    void requestConnect(const QString &port, double maxU, double maxI);
    void requestDisconnect();
    void requestApply(const retom::RetomSignalSet &set);
    void requestOff();
    void requestVerifyVoltage();
    void requestOutContact(int number, bool closed);
    void requestPolling(bool on);

private:
    void refreshPorts();
    void onApply();
    void onState(bool connected, bool enabled, bool outputActive);
    void onInputs(quint32 bits);
    void onBusy(bool on);
    void appendLog(const QString &line);
    void updateEnabled();

    QThread thread_;
    RetomWorker *worker_ = nullptr;

    QComboBox *portCombo_;
    QPushButton *refreshBtn_, *connectBtn_, *disconnectBtn_;
    QDoubleSpinBox *maxU_, *maxI_;
    std::array<QDoubleSpinBox *, 3> u_, uPhi_, i_, phi_;
    QDoubleSpinBox *freq_;
    QPushButton *symmetricBtn_, *applyBtn_, *offBtn_, *verifyBtn_;
    QSpinBox *outNumber_;
    QPushButton *outCloseBtn_, *outOpenBtn_;
    std::array<QCheckBox *, 8> inBits_;
    QLabel *inRaw_, *stateLabel_;
    QPlainTextEdit *log_;

    bool connected_ = false, busy_ = false, outputActive_ = false;
};
