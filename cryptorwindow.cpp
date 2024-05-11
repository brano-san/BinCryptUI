#include "cryptorwindow.h"
#include "ui_cryptorwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

CryptorWindow::CryptorWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CryptorWindow)
{
    ui->setupUi(this);

    _files = std::make_unique<crypt::MaskedFileManager>();

    QRegularExpression byteRegular("[0-9A-Fa-f]{2}");
    auto validator = new QRegularExpressionValidator(byteRegular, this);

    QString byteLinesName = "byteText_";
    for (int i = 0; i < 8; ++i)
    {
        this->findChild<QLineEdit*>(byteLinesName + QString::number(i))->setValidator(validator);
    }

    connect(ui->inputPathButton, &QPushButton::clicked,
            this, &CryptorWindow::onInputPathButtonClicked);

    connect(ui->outputPathButton, &QPushButton::clicked,
            this, &CryptorWindow::onOutputPathButtonClicked);

    connect(ui->startButton, &QPushButton::clicked,
            this, &CryptorWindow::onStartButtonClicked);

    connect(ui->strategyNameBox, &QComboBox::currentIndexChanged,
            this, &CryptorWindow::onStrategyIndexChanged);

    connect(ui->onceRadioButton, &QRadioButton::clicked,
            this, &CryptorWindow::onLaunchRadioButtonChanged);
    connect(ui->timerRadioButton, &QRadioButton::clicked,
            this, &CryptorWindow::onLaunchRadioButtonChanged);

    connect(ui->endButton, &QPushButton::clicked,
            this, &CryptorWindow::onEndButtonClicked);

    _cryptor.setFileSaverStrategy(new crypt::WritingToEndSavingStrategy);

    _launchTimer = std::make_unique<QTimer>(new QTimer(this));
    connect(_launchTimer.get(), &QTimer::timeout,
            this, &CryptorWindow::onLaunchTimerTimeout);
}

CryptorWindow::~CryptorWindow()
{
    delete ui;
}

void CryptorWindow::onInputPathButtonClicked()
{
    ui->inputText->setText(getDirectory());
}

void CryptorWindow::onOutputPathButtonClicked()
{
    ui->outputText->setText(getDirectory());
}

void CryptorWindow::onStartButtonClicked()
{
    launchCrypt();
}

void CryptorWindow::onStrategyIndexChanged(int index)
{
    switch (index)
    {
    case 0:
        ui->deletionRequiredBox->setChecked(false);
        ui->deletionRequiredBox->setEnabled(false);
        _cryptor.setFileSaverStrategy(new crypt::WritingToEndSavingStrategy);
        break;
    case 1:
        ui->deletionRequiredBox->setChecked(false);
        ui->deletionRequiredBox->setEnabled(false);
        _cryptor.setFileSaverStrategy(new crypt::OverwritingSavingStrategy);
        break;
    case 2:
        ui->deletionRequiredBox->setEnabled(true);
        _cryptor.setFileSaverStrategy(new crypt::NewNameSavingStrategy);
        break;
    }
}

void CryptorWindow::onLaunchRadioButtonChanged()
{
    ui->timerLabel->setEnabled(!ui->timerLabel->isEnabled());
    ui->askFileTimerBox->setEnabled(!ui->askFileTimerBox->isEnabled());
}

void CryptorWindow::onEndButtonClicked()
{
    _launchTimer->stop();
}

void CryptorWindow::onLaunchTimerTimeout()
{
    _files->findFilesByMask(
        ui->inputText->toPlainText().toStdString(),
        ui->maskText->toPlainText().toStdString());

    if (_files->getFileCount() > 0)
        launchCrypt();
}

QString CryptorWindow::getDirectory()
{
    return QFileDialog::getExistingDirectory(
        this, tr("Open Directory"),
        "",
        QFileDialog::ShowDirsOnly |
        QFileDialog::DontResolveSymlinks) + "/";
}

void CryptorWindow::launchCrypt()
{
    if (isFieldsEmpty())
    {
        showMessage("Предупреждение!", "Для работы программы необходимо заполнить все обязательные поля!");
        return;
    }

    QString byteLinesName = "byteText_";
    for (int i = 0; i < 8; ++i)
    {
        auto byteText = findChild<QLineEdit*>(byteLinesName + QString::number(i))->text().toStdString();
        _keyBytes[i] = static_cast<std::byte>(std::stoi(byteText, 0, 16));
    }

    _files->findFilesByMask(
        ui->inputText->toPlainText().toStdString(),
        ui->maskText->toPlainText().toStdString());

    _cryptor.cryptFile(_files.get(), _keyBytes, ui->outputText->toPlainText().toStdString());

    if (!_launchTimer->isActive())
    {
        showMessage("Информация", "Количество обработанных файлов: " +
            QString::number(_files->getFileCount()));
    }

    if (ui->deletionRequiredBox->isChecked())
        _files->deleteFiles();

    if (ui->timerRadioButton->isChecked() && !_launchTimer->isActive())
        _launchTimer->start(ui->askFileTimerBox->text().toInt() * 1000);
}

bool CryptorWindow::isFieldsEmpty()
{
    QString byteLinesName = "byteText_";
    for (int i = 0; i < 8; ++i)
    {
        if (findChild<QLineEdit*>(byteLinesName + QString::number(i))->text().isEmpty())
            return true;
    }

    if (ui->outputText->toPlainText().isEmpty() ||
        ui->inputText->toPlainText().isEmpty() ||
        ui->maskText->toPlainText().isEmpty())
    {
        return true;
    }
    return false;
}

void CryptorWindow::showMessage(QString title, QString msg)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(msg);
    msgBox.exec();
}
