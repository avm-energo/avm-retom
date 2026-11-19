#include "MainWindow.h"

#include "RetomWorker.h"
#include "RockerSwitch.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QPalette>
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
#include <QTimer>
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

const char *kGreenBtn = "QPushButton{background:#43a047;color:white;font-weight:bold;border:none;border-radius:4px;"
                        "padding:5px 14px}QPushButton:hover{background:#4caf50}"
                        "QPushButton:disabled{background:#d0d0d0;color:#8a8a8a}";
const char *kRoseBtn = "QPushButton{background:#ec6f9a;color:white;font-weight:bold;border:none;border-radius:4px;"
                       "padding:5px 14px}QPushButton:hover{background:#f080a8}"
                       "QPushButton:disabled{background:#d0d0d0;color:#8a8a8a}";

// Light-red, bright theme with black text.
const char *kRedField = "{background:#ffb3b3;color:#000000}";
const char *kOutBtn = "QPushButton{background:#ff6b6b;color:#000000;font-weight:bold;border:1px solid #d32f2f;"
                      "border-radius:4px;padding:5px 10px}QPushButton:hover{background:#ff8585}"
                      "QPushButton:checked{background:#ff1a1a;border:2px solid #8b0000}"
                      "QPushButton:disabled{background:#d0d0d0;color:#8a8a8a;border-color:#b0b0b0}";
const char *kInCheck = "QCheckBox{background:#ff9a9a;color:#000000;border-radius:3px;padding:3px 6px}"
                       "QCheckBox:disabled{color:#000000}";
const char *kLog = "QPlainTextEdit{background:#ffd0d0;color:#000000}";

struct PhaseStyle
{
    const char *badge; // solid colour of the phase letter badge
    const char *text;  // letter colour
    const char *tint;  // light background of the phase's spin boxes
};
// Phase A yellow, B green, C red.
const PhaseStyle kPhase[3] = { { "#f2c200", "#000000", "#fff4bd" },
    { "#2e9e44", "#ffffff", "#d3edd6" },
    { "#d32f2f", "#ffffff", "#f8d0d0" } };
} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("AVM RETOM"));
    // Tinted window background so this window is easy to tell apart from its sibling app.
    {
        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor("#f4e0e0"));
        setPalette(pal);
    }
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
    for (QWidget *w : { static_cast<QWidget *>(maxU_), static_cast<QWidget *>(maxI_) })
        w->setStyleSheet(QStringLiteral("QDoubleSpinBox%1").arg(QLatin1String(kRedField)));
    portCombo_->setStyleSheet(QStringLiteral("QComboBox%1 QComboBox QAbstractItemView{background:#ffd0d0;color:#000000}")
                                  .arg(QLatin1String(kRedField)));
    connectBtn_ = new QPushButton(tr("Connect"));
    connectBtn_->setStyleSheet(QLatin1String(kGreenBtn));
    disconnectBtn_ = new QPushButton(tr("Disconnect"));
    disconnectBtn_->setStyleSheet(QLatin1String(kRoseBtn));
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
    auto *anaBox = new QGroupBox;
    auto *anaV = new QVBoxLayout(anaBox);
    auto *head = new QHBoxLayout;
    power_ = new RockerSwitch;
    power_->setToolTip(tr("Analog output ON / OFF. While ON, changing a value re-applies the signal."));
    auto *anaTitle = new QLabel(tr("<b>Analog output</b>"));
    head->addWidget(power_);
    head->addWidget(anaTitle);
    head->addStretch();
    anaV->addLayout(head);
    auto *g = new QGridLayout;
    anaV->addLayout(g);
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
        auto *badge = new QLabel(QString(QChar('A' + k)));
        badge->setAlignment(Qt::AlignCenter);
        badge->setMinimumWidth(28);
        badge->setStyleSheet(QStringLiteral("QLabel{background:%1;color:%2;font-weight:bold;border-radius:4px;padding:3px}")
                                 .arg(QLatin1String(kPhase[k].badge), QLatin1String(kPhase[k].text)));
        g->addWidget(badge, k + 1, 0);
        for (QDoubleSpinBox *sb : { u_[k], uPhi_[k], i_[k], phi_[k] })
            sb->setStyleSheet(QStringLiteral("QDoubleSpinBox{background:%1;color:#000000}")
                                  .arg(QLatin1String(kPhase[k].tint)));
        g->addWidget(u_[k], k + 1, 1);
        g->addWidget(uPhi_[k], k + 1, 2);
        g->addWidget(i_[k], k + 1, 3);
        g->addWidget(phi_[k], k + 1, 4);
    }
    freq_ = spin(1, 1000, 50, tr(" Hz"), 3);
    symmetricBtn_ = new QPushButton(tr("3-phase from A"));
    symmetricBtn_->setToolTip(tr("Copy phase A's U/I to B and C with phases 0/-120/-240° (I in phase with U)"));
    verifyBtn_ = new QPushButton(tr("Check voltage stage"));
    verifyBtn_->setToolTip(tr("Applies 10 V / 0.1 A on phase A for 5 s, reads the Ready flag, then switches the "
                              "output off. Use it when currents are fine but voltages are zero. "
                              "Only available while the output is OFF."));
    freq_->setStyleSheet(QStringLiteral("QDoubleSpinBox%1").arg(QLatin1String(kRedField)));
    g->addWidget(new QLabel(tr("Frequency:")), 4, 0);
    g->addWidget(freq_, 4, 1);
    g->addWidget(symmetricBtn_, 4, 2);
    g->addWidget(verifyBtn_, 4, 3, 1, 2);
    root->addWidget(anaBox);

    // Tab goes down each column (A, B, C), then on to the next column to the right -- not along the
    // rows as the default creation order would. Then frequency and the two buttons.
    {
        QWidget *prev = power_;
        for (const auto &column : { u_, uPhi_, i_, phi_ })
        {
            for (QDoubleSpinBox *sb : column)
            {
                setTabOrder(prev, sb);
                prev = sb;
            }
        }
        setTabOrder(prev, freq_);
        setTabOrder(freq_, symmetricBtn_);
        setTabOrder(symmetricBtn_, verifyBtn_);
    }

    reapplyTimer_ = new QTimer(this);
    reapplyTimer_->setSingleShot(true);
    reapplyTimer_->setInterval(400);
    connect(reapplyTimer_, &QTimer::timeout, this, [this] {
        if (outputActive_ && connected_)
            onApply();
    });

    // --- Discrete ---
    auto *discBox = new QGroupBox(tr("Discrete contacts"));
    auto *dl = new QGridLayout(discBox);
    dl->addWidget(new QLabel(tr("<b>Outputs</b> (click to close/open)")), 0, 0);
    auto *outL = new QHBoxLayout;
    for (int n = 0; n < 4; ++n)
    {
        outBtns_[n] = new QPushButton(tr("Out %1: open").arg(n + 1));
        outBtns_[n]->setCheckable(true);
        outBtns_[n]->setStyleSheet(QLatin1String(kOutBtn));
        outBtns_[n]->setToolTip(tr("Output contact %1 (confirmed on hardware 2026-11-19)").arg(n + 1));
        outL->addWidget(outBtns_[n]);
        // clicked (not toggled): only user clicks send a command, programmatic resets don't.
        connect(outBtns_[n], &QPushButton::clicked, this, [this, n](bool closed) {
            outBtns_[n]->setText(tr("Out %1: %2").arg(n + 1).arg(closed ? tr("CLOSED") : tr("open")));
            emit requestOutContact(n, closed); // driver index is 0-based, the UI numbers from 1
        });
    }
    outL->addStretch();
    dl->addLayout(outL, 0, 1, 1, 3);
    dl->addWidget(new QLabel(tr("<b>Inputs</b> (polled 1/s)")), 1, 0);
    auto *bitsL = new QHBoxLayout;
    for (int b = 0; b < 8; ++b)
    {
        inBits_[b] = new QCheckBox(QString::number(b + 1));
        inBits_[b]->setStyleSheet(QLatin1String(kInCheck));
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
    log_->setStyleSheet(QLatin1String(kLog));
    root->addWidget(log_, 1);
    statusBar()->addWidget(new QLabel(tr("Created by AVM-Energo LLC with Claude, 2026")));
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
    connect(power_, &QAbstractButton::clicked, this, &MainWindow::onPowerClicked);
    for (int k = 0; k < 3; ++k)
        for (QDoubleSpinBox *sb : { u_[k], uPhi_[k], i_[k], phi_[k] })
            connect(sb, &QDoubleSpinBox::valueChanged, this, [this] { reapplyTimer_->start(); });
    connect(freq_, &QDoubleSpinBox::valueChanged, this, [this] { reapplyTimer_->start(); });
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
        // The EMon's FT232R adapter shares VID/PID 0403:6001 with RETOM, so prefer the "Dynamics" device.
        const bool dynamics = info.description().contains(QLatin1String("Dynamics"), Qt::CaseInsensitive)
           ;
        if (isRetom && (best < 0 || (dynamics && !portCombo_->itemText(best).contains(QLatin1String("Dynamics"), Qt::CaseInsensitive))))
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

void MainWindow::onPowerClicked(bool on)
{
    if (on)
        onApply();
    else
        emit requestOff();
}

void MainWindow::onState(bool connected, bool enabled, bool outputActive)
{
    connected_ = connected;
    outputActive_ = outputActive;
    power_->setChecked(outputActive); // setChecked() does not emit clicked(), so this sends no command
    stateLabel_->setText(tr("%1 | enabled: %2 | output: %3")
                             .arg(connected ? tr("connected") : tr("disconnected"),
                                 enabled ? tr("yes") : tr("no"), outputActive ? tr("ON") : tr("off")));
    emit requestPolling(connected);
    if (!connected)
    {
        onInputs(0);
        inRaw_->setText(QStringLiteral("raw=—"));
        for (int n = 0; n < 4; ++n)
        {
            outBtns_[n]->setChecked(false);
            outBtns_[n]->setText(tr("Out %1: open").arg(n + 1));
        }
    }
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
    power_->setEnabled(idle && connected_);
    for (QPushButton *b : outBtns_)
        b->setEnabled(idle && connected_);
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
