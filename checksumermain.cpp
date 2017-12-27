#include "checksumermain.h"
#include "ui_checksumermain.h"

static const quint64 TIME_UPDATE_TIMEOUT = 100;

ChecksumerMain::ChecksumerMain(Checksumer *checksumer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChecksumerMain),
    m_updatetimer(this)
{
    ui->setupUi(this);
    m_Checksumer = checksumer;
    m_updatetimer.setTimerType(Qt::PreciseTimer);

    QObject::connect(m_Checksumer, SIGNAL(Checksumer_RangeChangedSignal(int,int)), this, SLOT(processbar_SetRange(int,int)), Qt::AutoConnection);
    QObject::connect(m_Checksumer, SIGNAL(Checksumer_ValueChangedSignal(int)), this, SLOT(processbar_SetValue(int)), Qt::AutoConnection);
    QObject::connect(m_Checksumer, SIGNAL(Checksumer_ChecksumResultSignal(quint64, qint64)), this, SLOT(setChecksumResult(quint64, qint64)), Qt::AutoConnection);
    QObject::connect(&m_updatetimer, SIGNAL(timeout()), this, SLOT(elapsedTimeUpdate()), Qt::AutoConnection);
    QObject::connect(ui->progressBar, SIGNAL(valueChanged(int)), this, SLOT(processbar_ValueChanged(int)), Qt::AutoConnection);
}

ChecksumerMain::~ChecksumerMain()
{
    delete ui;
}

void ChecksumerMain::ActivationChangedProc()
{
    if (true == isActiveWindow()){
        if (Checksumer::CHECKSUMER_COMPLETE == m_Checksumer->m_status){
            ui->checksumDisplay->setFocus(Qt::ActiveWindowFocusReason);
            ui->checksumDisplay->selectAll();
        }
    }
}

void ChecksumerMain::changeEvent(QEvent *event)
{
    if(event->type()==QEvent::ActivationChange)
    {
        QTimer::singleShot(0, this, SLOT(ActivationChangedProc()));
    }

    QDialog::changeEvent(event);
}

void ChecksumerMain::on_openfileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    NULL,
                                                    "Files (*)");

    if (fileName.length() != 0){
        emit m_Checksumer->OpenFileButtonClicked(fileName);

        QFileInfo fileInfo(fileName);
        QString dispFilename = fileInfo.fileName();
        ui->filenameDisplay->setText(dispFilename);
        ui->filenameDisplay->setToolTip(fileName);

        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(0);
        ui->checksumDisplay->clear();
        this->setWindowTitle(QString("CheckSumer"));
    }
    else{
    }
}

void ChecksumerMain::on_checksumButton_clicked()
{
    if (false == Checksumer::m_filepath.isEmpty()){
        ui->progressBar->setRange(0, 100);
        ui->progressBar->setValue(0);
        ui->checksumDisplay->clear();
        this->setWindowTitle(QString("CheckSumer"));
        m_updatetimer.start(TIME_UPDATE_TIMEOUT);
        emit m_Checksumer->ChecksumButtonClicked();
    }
    else{
        QMessageBox::warning(this, "CheckSumer", "Select file first!");
    }
}

void ChecksumerMain::processbar_SetRange(int minimum, int maximum)
{
    ui->openfileButton->setEnabled(false);
    ui->progressBar->setRange(minimum, maximum);
    ui->progressBar->setValue(0);
    ui->checksumDisplay->clear();
    this->setWindowTitle(QString("CheckSumer"));
}

void ChecksumerMain::processbar_SetValue(int progressValue)
{
    ui->progressBar->setValue(progressValue);
}

void ChecksumerMain::setChecksumResult(quint64 checksum, qint64 elapsedtime)
{
    QString checksum_str;
    checksum_str.setNum(checksum % 0x100000000,16);    // we just want low 32bit
    checksum_str = checksum_str.toUpper();
    ui->checksumDisplay->setText(checksum_str);
    m_updatetimer.stop();
    setElapsedTimetoLCDNumber(elapsedtime);

    qreal displayvalue = ui->elapsedtimeLCDNumber->value();
    QString realString = QString::number(displayvalue, 'f', 1);
    realString.prepend(QChar('('));
    realString.append(" Sec)");
    this->setWindowTitle(QString("Complete") + realString + QString(" - CheckSumer"));
    ui->openfileButton->setEnabled(true);
}

void ChecksumerMain::processbar_ValueChanged(int value)
{
    Q_UNUSED(value);
    if (ui->progressBar->text() != QString("100%")){
        QString titlestring = ui->progressBar->text() + " " + ui->filenameDisplay->text() + " - CheckSumer";
        this->setWindowTitle(titlestring);
    }
}

void ChecksumerMain::elapsedTimeUpdate(void)
{
    qint64 ElapsedTime = m_Checksumer->getElapsedTime();

    if (ElapsedTime != -1){
        setElapsedTimetoLCDNumber(ElapsedTime);
    }
}

void ChecksumerMain::setElapsedTimetoLCDNumber(qint64 elapsedtime)
{
    qreal realelapsedtime = (qreal)(elapsedtime)/1000;
    QString realString = QString::number(realelapsedtime, 'f', 1);
    ui->elapsedtimeLCDNumber->display(realString);
}
