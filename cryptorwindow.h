#ifndef CRYPTORWINDOW_H
#define CRYPTORWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "BinCryptor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CryptorWindow; }
QT_END_NAMESPACE

class CryptorWindow : public QMainWindow
{
    Q_OBJECT

public:
    CryptorWindow(QWidget *parent = nullptr);

    ~CryptorWindow();

private:
    QString getDirectory();

    void launchCrypt();

    bool isFieldsEmpty();

    void showMessage(QString title, QString msg);

    void onInputPathButtonClicked();

    void onOutputPathButtonClicked();

    void onStartButtonClicked();

    void onStrategyIndexChanged(int index);

    void onOnceLaunchClicked();

    void onTimerLaunchClicked();

    void onLaunchRadioButtonChanged();

    void onEndButtonClicked();

    void onLaunchTimerTimeout();

private:
    Ui::CryptorWindow *ui;

    std::unique_ptr<QTimer> _launchTimer;

    crypt::Cryptor _cryptor;

    std::array<std::byte, 8> _keyBytes;

    std::unique_ptr<crypt::MaskedFileManager> _files;
};
#endif // CRYPTORWINDOW_H
