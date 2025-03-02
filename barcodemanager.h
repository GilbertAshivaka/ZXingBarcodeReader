#ifndef BARCODEMANAGER_H
#define BARCODEMANAGER_H

#include <QObject>
#include <QVideoFrame>
#include <QMutex>
#include <memory>
#include <zxing-cpp/core/src/Barcode.h>
#include <zxing-cpp/core/src/Matrix.h>

namespace ZXing {
class MultiFormatReader;
namespace OneD {
class Code128Writer;
}
}

class BarcodeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString lastResult READ lastResult NOTIFY lastResultChanged)
    Q_PROPERTY(bool scanning READ scanning WRITE setScanning NOTIFY scanningChanged)
    Q_PROPERTY(bool flashEnabled READ flashEnabled WRITE setFlashEnabled NOTIFY flashEnabledChanged)

public:
    explicit BarcodeManager(QObject *parent = nullptr);

    ~BarcodeManager();

    Q_INVOKABLE bool processImage(const QImage &image);
    Q_INVOKABLE bool processFrame(const QVideoFrame &frame);
    Q_INVOKABLE QImage generateCode128(const QString &content, int width = 300, int height = 100);
    Q_INVOKABLE QImage generateBarcode(const QString &content, ZXing::BarcodeFormat format, int width, int height);
    Q_INVOKABLE bool saveBarcode(const QImage &image, const QString &filePath);

    QString lastResult() const;

    //scanning state
    bool scanning() const;
    void setScanning(bool scanning);

    // flash control
    bool flashEnabled() const;
    void setFlashEnabled(bool enabled);

signals:
    void lastResultChanged(const QString &result);
    void scanningChanged(bool scanning);
    void flashEnabledChanged(bool enabled);
    void barcodeDetected(const QString &result);
    void barcodeGenerated();


private:
    //process the image with zxing
    ZXing::Result decodeImage(const QImage &image);

    //member variables
    std::unique_ptr<ZXing::MultiFormatReader> m_reader;
    std::unique_ptr<ZXing::OneD::Code128Writer> m_writer;

    QString m_lastResult;
    bool m_scanning;
    bool m_flashEnabled;
    QMutex m_mutex;
};

#endif // BARCODEMANAGER_H















