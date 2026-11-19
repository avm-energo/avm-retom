#include "MainWindow.h"

#include "RetomWorker.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QSpinBox>
#include <QStatusBar>
#include <QVBoxLayout>

namespace
{
QDoubleSpinBox *spin(double min, double max, double val, const QString &suffix, int decimals = 3)
{
    auto *s = new QDoubleSpinBox;
    s->setRange(min, max);
    s->setDecimals(decimals);
    s->setValue(val);
    s->setSuffix(suffix);
    s->setKeyboardTracking(false);
    return s;
}
} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("AVM RETOM"));
    auto *central = new QWidget;
    auto *root = new QVBoxLayout(central);
    setCentralWidget(central);

    // --- Connection ---
    auto *connBox = new QGroupBox(tr("Connection"));
    auto *connL = new QHBoxLayout(connBox);
    portCombo_ = new QComboBox;
    portCombo_->setEditable(true);
    portCombo_->setMinimumWidth(260);
    refreshBtn_ = new QPushButton(tr("Refresh"));
    maxU_ = spin(1, 300, 120, tr(" V"), 1);
    maxI_ = spin(0.1, 100, 20, tr(" A"), 1);
    connectBtn_ = new QPushButton(tr("Connect"));
    disconnectBtn_ = new QPushButton(tr("Disconnect"));
    stateLabel_ = new QLabel;
    connL->addWidget(new QLabel(tr("Port:")));
    connL->addWidget(portCombo_, 1);
    connL->addWidget(refreshBtn_);
    connL->addWidget(new QLabel(tr("Max U:")));
    connL->addWidget(maxU_);
    connL->addWidget(new QLabel(tr("Max I:")));
    connL->addWidget(maxI_);
    connL->addWidget(connectBtn_);
    connL->addWidget(disconnectBtn_);
    root->addWidget(connBox);

    // --- Analog output ---
    auto *anaBox = new QGroupBox(tr("Analog output"));
    auto *g = new QGridLayout(anaBox);
    const char *hdr[] = { "", "U, V (RMS)", "U phase, °", "I, A (RMS)", "I phase, °" };
    for (int c = 0; c < 5; ++c)
        g->addWidget(new QLabel(tr(hdr[c])), 0, c);
    const double defPhase[3] = { 0, -120, -240 };
    for (int k = 0; k < 3; ++k)
    {
        u_[k] = spin(-300, 300, 0, QString(), 3);
        uPhi_[k] = spin(-360, 360, defPhase[k], QString(), 2);
        i_[k] = spin(0, 100, 0, QString(), 3);
        phi_[k] = spin(-360, 360, defPhase[k], QString(), 2);
        g->addWidget(new QLabel(QStringLiteral("<b>%1</b>").arg(QChar('A' + k))), k + 1, 0);
        g->addWidget(u_[k], k + 1, 1);
        g->addWidget(uPhi_[k], k + 1, 2);
        g->addWidget(i_[k], k + 1, 3);
        g->addWidget(phi_[k], k + 1, 4);
    }
    freq_ = spin(1, 1000, 50, tr(" Hz"), 3);
    symmetricBtn_ = new QPushButton(tr("3-phase from A"));
    symmetricBtn_->setToolTip(tr("Copy phase A's U/I to B and C with phases 0/-120/-240° (I in phase with U)"));
    applyBtn_ = new QPushButton(tr("Apply"));
    offBtn_ = new QPushButton(tr("Output OFF"));
    verifyBtn_ = new QPushButton(tr("Check voltage stage"));
    verifyBtn_->setToolTip(tr("Applies 10 V / 0.1 A on phase A for 5 s, reads the Ready flag, then switches the "
                              "output off. Use it when currents are fine but voltages are zero. "
                              "Only available while no signal is applied."));
    g->addWidget(verifyBtn_, 5, 0, 1, 2);
    g->addWidget(new QLabel(tr("Frequency:")), 4, 0);
    g->addWidget(freq_, 4, 1);
    g->addWidget(symmetricBtn_, 4, 2);
    g->addWidget(applyBtn_, 4, 3);
    g->addWidget(offBtn_, 4, 4);
    root->addWidget(anaBox);

    // --- Discrete ---
    auto *discBox = new QGroupBox(tr("Discrete contacts"));
    auto *dl = new QGridLayout(discBox);
    outNumber_ = new QSpinBox;
    outNumber_->setRange(0, 31);
    outNumber_->setToolTip(tr("libavm-retom only confirmed contact 0 against hardware; the number is currently "
                              "ignored by the library's frame encoder."));
    outCloseBtn_ = new QPushButton(tr("Close"));
    outOpenBtn_ = new QPushButton(tr("Open"));
    dl->addWidget(new QLabel(tr("<b>Outputs</b> — contact #")), 0, 0);
    dl->addWidget(outNumber_, 0, 1);
    dl->addWidget(outCloseBtn_, 0, 2);
    dl->addWidget(outOpenBtn_, 0, 3);
    dl->addWidget(new QLabel(tr("<b>Inputs</b> (polled 1/s)")), 1, 0);
    auto *bitsL = new QHBoxLayout;
    for (int b = 0; b < 8; ++b)
    {
        inBits_[b] = new QCheckBox(QString::number(b));
        inBits_[b]->setAttribute(Qt::WA_TransparentForMouseEvents);
        inBits_[b]->setFocusPolicy(Qt::NoFocus);
        bitsL->addWidget(inBits_[b]);
    }
    inRaw_ = new QLabel(QStringLiteral("raw=—"));
    bitsL->addWidget(inRaw_);
    bitsL->addStretch();
    dl->addLayout(bitsL, 1, 1, 1, 3);
    root->addWidget(discBox);

    log_ = new QPlainTextEdit;
    log_->setReadOnly(true);
    log_->setMaximumBlockCount(2000);
    root->addWidget(log_, 1);
    statusBar()->addPermanentWidget(stateLabel_);

    // --- Worker thread ---
    worker_ = new RetomWorker;
    worker_->moveToThread(&thread_);
    connect(&thread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(this, &MainWindow::requestConnect, worker_, &RetomWorker::connectDevice);
    connect(this, &MainWindow::requestDisconnect, worker_, &RetomWorker::disconnectDevice);
    connect(this, &MainWindow::requestApply, worker_, &RetomWorker::apply);
    connect(this, &MainWindow::requestOff, worker_, &RetomWorker::off);
    connect(this, &MainWindow::requestOutContact, worker_, &RetomWorker::setOutContact);
    connect(this, &MainWindow::requestPolling, worker_, &RetomWorker::setPolling);
    connect(worker_, &RetomWorker::log, this, &MainWindow::appendLog);
    connect(worker_, &RetomWorker::stateChanged, this, &MainWindow::onState);
    connect(worker_, &RetomWorker::inputsRead, this, &MainWindow::onInputs);
    connect(worker_, &RetomWorker::busy, this, &MainWindow::onBusy);
    connect(this, &MainWindow::requestVerifyVoltage, worker_, &RetomWorker::verifyVoltage);
    connect(worker_, &RetomWorker::voltageCheckResult, this, [this](bool ok, const QString &msg) {
        if (ok)
            QMessageBox::information(this, tr("Voltage stage"), msg);
        else
            QMessageBox::critical(this, tr("Voltage stage NOT ready"), msg);
    });
    connect(verifyBtn_, &QPushButton::clicked, this, [this] { emit requestVerifyVoltage(); });
    thread_.start();

    connect(refreshBtn_, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(connectBtn_, &QPushButton::clicked, this,
        [this] { emit requestConnect(portCombo_->currentData().toString().isEmpty()
                                         ? portCombo_->currentText().trimmed()
                                         : portCombo_->currentData().toString(),
                     maxU_->value(), maxI_->value()); });
    connect(disconnectBtn_, &QPushButton::clicked, this, [this] { emit requestDisconnect(); });
    connect(applyBtn_, &QPushButton::clicked, this, &MainWindow::onApply);
    connect(offBtn_, &QPushButton::clicked, this, [this] { emit requestOff(); });
    connect(symmetricBtn_, &QPushButton::clicked, this, [this] {
        const double ph[3] = { 0, -120, -240 };
        for (int k = 0; k < 3; ++k)
        {
            u_[k]->setValue(u_[0]->value());
            i_[k]->setValue(i_[0]->value());
            uPhi_[k]->setValue(ph[k]);
            phi_[k]->setValue(ph[k]);
        }
    });
    connect(outCloseBtn_, &QPushButton::clicked, this, [this] { emit requestOutContact(outNumber_->value(), true); });
    connect(outOpenBtn_, &QPushButton::clicked, this, [this] { emit requestOutContact(outNumber_->value(), false); });

    refreshPorts();
    onState(false, false, false);
    resize(820, 640);
}

MainWindow::~MainWindow()
{
    // Disconnect (analog output off) before the worker goes away; blocks until done.
    QMetaObject::invokeMethod(worker_, &RetomWorker::disconnectDevice, Qt::BlockingQueuedConnection);
    thread_.quit();
    thread_.wait();
}

void MainWindow::refreshPorts()
{
    const QString prev = portCombo_->currentText();
    portCombo_->clear();
    int best = -1;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
    {
        const bool isRetom = info.vendorIdentifier() == retom::kRetomVid && info.productIdentifier() == retom::kRetomPid;
        portCombo_->addItem(QStringLiteral("%1 — %2%3").arg(info.systemLocation(), info.description(),
                                isRetom ? QStringLiteral(" [FTDI 0403:6001]") : QString()),
            info.systemLocation());
        if (isRetom && best < 0)
            best = portCombo_->count() - 1;
    }
    if (best >= 0)
        portCombo_->setCurrentIndex(best);
    else if (portCombo_->count() == 0)
        portCombo_->setEditText(prev.isEmpty() ? QStringLiteral("/dev/ttyUSB0") : prev);
}

void MainWindow::onApply()
{
    retom::RetomSignalSet s;
    for (int k = 0; k < 3; ++k)
    {
        s.u[k] = u_[k]->value();
        s.uPhi[k] = uPhi_[k]->value();
        s.i[k] = i_[k]->value();
        s.phi[k] = phi_[k]->value();
    }
    s.freq = freq_->value();
    emit requestApply(s);
}

void MainWindow::onState(bool connected, bool enabled, bool outputActive)
{
    connected_ = connected;
    outputActive_ = outputActive;
    stateLabel_->setText(tr("%1 | enabled: %2 | output: %3")
                             .arg(connected ? tr("connected") : tr("disconnected"),
                                 enabled ? tr("yes") : tr("no"), outputActive ? tr("ON") : tr("off")));
    emit requestPolling(connected);
    if (!connected)
        onInputs(0), inRaw_->setText(QStringLiteral("raw=—"));
    updateEnabled();
}

void MainWindow::onInputs(quint32 bits)
{
    for (int b = 0; b < 8; ++b)
        inBits_[b]->setChecked((bits >> b) & 1);
    inRaw_->setText(QStringLiteral("raw=0x%1").arg(bits, 8, 16, QLatin1Char('0')));
}

void MainWindow::onBusy(bool on)
{
    busy_ = on;
    updateEnabled();
}

void MainWindow::updateEnabled()
{
    const bool idle = !busy_;
    connectBtn_->setEnabled(idle && !connected_);
    disconnectBtn_->setEnabled(idle && connected_);
    verifyBtn_->setEnabled(idle && connected_ && !outputActive_);
    for (QWidget *w : { static_cast<QWidget *>(applyBtn_), static_cast<QWidget *>(offBtn_),
             static_cast<QWidget *>(outCloseBtn_), static_cast<QWidget *>(outOpenBtn_) })
        w->setEnabled(idle && connected_);
    portCombo_->setEnabled(!connected_);
    refreshBtn_->setEnabled(!connected_);
    maxU_->setEnabled(!connected_);
    maxI_->setEnabled(!connected_);
    setCursor(busy_ ? Qt::BusyCursor : Qt::ArrowCursor);
}

void MainWindow::appendLog(const QString &line)
{
    log_->appendPlainText(QStringLiteral("[%1] %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss"), line));
}
