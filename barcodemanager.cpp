#include "barcodemanager.h"

#include "zxing-cpp/core/src/MultiFormatReader.h"
#include "zxing-cpp/core/src/MultiFormatWriter.h"
#include "zxing-cpp/core/src/DecodeHints.h"
#include "zxing-cpp/core/src/BarcodeFormat.h"
#include <zxing-cpp/core/src/BitMatrix.h>
#include <zxing-cpp/core/src/BinaryBitmap.h>
#include "zxing-cpp/core/src/oned/ODCode128Reader.h"

#include "zxing-cpp/core/src/BinaryBitmap.h"
#include "zxing-cpp/core/src/GlobalHistogramBinarizer.h"
#include "zxing-cpp/core/src/HybridBinarizer.h"

#include <QDebug>
#include <QPainter>
#include <QDir>

BarcodeManager::BarcodeManager(QObject *parent)
    : QObject{parent}
    , m_scanning(false)
    , m_flashEnabled(false)
{
    //configure ZXing reader with the necessary Formats

    ZXing::DecodeHints hints;
    hints.setTryHarder(true);
    hints.setTryRotate(true);

    hints.setFormats(ZXing::BarcodeFormat::Code128 |
                     ZXing::BarcodeFormat::EAN8 |
                     ZXing::BarcodeFormat::EAN13 |
                     ZXing::BarcodeFormat::UPCA |
                     ZXing::BarcodeFormat::UPCE |
                     ZXing::BarcodeFormat::Code39 |
                     ZXing::BarcodeFormat::ITF |
                     ZXing::BarcodeFormat::Codabar
                     );

    m_reader = std::make_unique<ZXing::MultiFormatReader>(hints);
    m_writer = std::make_unique<ZXing::OneD::Code128Writer>();

}

BarcodeManager::~BarcodeManager()
{

}

bool BarcodeManager::processImage(const QImage &image)
{
    if (!m_scanning || image.isNull())
        return false;

    QMutexLocker locker(&m_mutex);

    ZXing::Result result = decodeImage(image);

    if (result.isValid()){
        QString text = QString::fromStdString(result.text());

        //only notify when we have a new result
        if (m_lastResult != text){
            m_lastResult = text;
            emit lastResultChanged(m_lastResult);
            emit barcodeDetected(m_lastResult);

            qDebug() << "Barcode Detected: " << m_lastResult;

            return true;
        }
    }

    return false;
}

bool BarcodeManager::processFrame(const QVideoFrame &frame)
{
    if (!m_scanning || !frame.isValid())
        return false;

    //map the video frame to read its data
    QVideoFrame mappedFrame = frame;

    if (!mappedFrame.map(QVideoFrame::ReadOnly)){
        qWarning() << "Could not map video frame for reading.";
        return false;
    }

    //connect to QImage format
    QImage image;

    // Get a pointer to the frame data
    const uchar *bits = mappedFrame.bits(0);
    int stride = mappedFrame.bytesPerLine(0);

    if (mappedFrame.pixelFormat() == QVideoFrameFormat::Format_ARGB8888 || mappedFrame.pixelFormat()== QVideoFrameFormat::Format_ARGB8888_Premultiplied){
        image = QImage(
            bits,
            mappedFrame.width(),
            mappedFrame.height(),
            stride,
            QImage::Format_ARGB32
            );
    }else if(mappedFrame.pixelFormat() == QVideoFrameFormat::Format_XRGB8888){
        image = QImage(
            bits,
            mappedFrame.width(),
            mappedFrame.height(),
            stride,
            QImage::Format_RGB32
            );
    }else {
        // for other formats convert to ARGB32
        QImage::Format format = QVideoFrameFormat::imageFormatFromPixelFormat(mappedFrame.pixelFormat());
        if (format != QImage::Format_Invalid){
            image = QImage(
                        bits,
                        mappedFrame.width(),
                        mappedFrame.height(),
                        stride,
                        format).convertToFormat(QImage::Format_ARGB32);
        }
    }

    //Unmap the frame
    mappedFrame.unmap();

    if (image.isNull()){
        qDebug() << "Failed to create image from video frame.";
        return false;
    }

    return processImage(image);
}

QImage BarcodeManager::generateCode128(const QString &content, int width, int height)
{
    return generateBarcode(content, ZXing::BarcodeFormat::Code128, width, height);
}

QImage BarcodeManager::generateBarcode(const QString &content, ZXing::BarcodeFormat format, int width, int height)
{
    if (content.isEmpty()){
        qWarning() << "Cannot generate barcode with empty content";
        return QImage();
    }

    try{
        //use multiformat writer, instead of specific writer classes

        ZXing::MultiFormatWriter writer(format);

        ZXing::BitMatrix matrix = writer.encode(content.toStdString(), width, height);
        auto bitmap = ZXing::ToMatrix<uint8_t>(matrix);

        //the barcode image
        QImage barcodeImage = QImage(
            bitmap.data(),
            bitmap.width(),
            bitmap.height(),
            bitmap.width(),
            QImage::Format_Grayscale8
            );

        QImage resultImage = barcodeImage.copy();

        if (height > 0){
            QImage labeledImage(resultImage.width(), resultImage.height() + 30, QImage::Format_ARGB32);

            labeledImage.fill(Qt::white);

            QPainter painter(&labeledImage);
            painter.drawImage(0, 0, resultImage);

            QFont font = painter.font();
            font.setPointSize(10);
            painter.setFont(font);

            painter.drawText(QRect(0, resultImage.height(), resultImage.width(), 30), Qt::AlignCenter, content);

            emit barcodeGenerated();
            return labeledImage;
        }

        return resultImage;
    }catch(std::exception &e){
        qWarning() << "Failed to generate image from content " << e.what();
        return QImage();
    }
}

bool BarcodeManager::saveBarcode(const QImage &image, const QString &filePath)
{
    if (image.isNull()){
        return false;
    }

    //ensure that the directory exists

    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();

    if(!dir.exists()){
        dir.mkpath(".");
    }

    return image.save(filePath);
}

QString BarcodeManager::lastResult() const
{
    return m_lastResult;
}

bool BarcodeManager::scanning() const
{
    return m_scanning;
}

void BarcodeManager::setScanning(bool scanning)
{
    if(m_scanning != scanning){
        m_scanning = scanning;
        emit scanningChanged(m_scanning);
    }
}

bool BarcodeManager::flashEnabled() const
{
    return m_flashEnabled;
}

void BarcodeManager::setFlashEnabled(bool enabled)
{
    if (m_flashEnabled != enabled){
        m_flashEnabled = enabled;
        emit flashEnabledChanged(m_flashEnabled);
    }
}


ZXing::Result BarcodeManager::decodeImage(const QImage &image)
{
    // Convert to ZXing imageView
    ZXing::ImageView imageView(
        image.bits(),
        image.width(),
        image.height(),
        ZXing::ImageFormat::ARGB,
        image.bytesPerLine()
        );

    // Create a BinaryBitmap using one of ZXing's binarizers
    // GlobalHistogramBinarizer is one option, HybridBinarizer is another
    auto binarizer = std::make_shared<ZXing::GlobalHistogramBinarizer>(imageView);
    auto binImage = std::make_shared<ZXing::BinaryBitmap>(binarizer);

    // Now pass the BinaryBitmap to the reader
    return m_reader->read(*binImage);
}
