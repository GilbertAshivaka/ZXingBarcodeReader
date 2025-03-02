#include "imageprovider.h"
#include <QMutexLocker>
#include <QVideoFrame>
ImageProvider::ImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    // Initialize with an empty black image
    m_currentFrame = QImage(640, 480, QImage::Format_RGB32);
    m_currentFrame.fill(Qt::black);
}
QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id)
    QMutexLocker locker(&m_mutex);
    if (size)
        *size = m_currentFrame.size();
    if (requestedSize.width() > 0 && requestedSize.height() > 0) {
        return m_currentFrame.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return m_currentFrame;
}
void ImageProvider::updateFrame(const QVideoFrame &frame)
{
    if (!frame.isValid())
        return;
    QVideoFrame mappedFrame = frame;
    if (!mappedFrame.map(QVideoFrame::ReadOnly))
        return;

    QImage image;
    // Get a pointer to the frame data and stride
    const uchar *bits = mappedFrame.bits(0);
    int stride = mappedFrame.bytesPerLine(0);

    // Convert based on pixel format
    if (mappedFrame.pixelFormat() == QVideoFrameFormat::Format_ARGB8888 ||
        mappedFrame.pixelFormat() == QVideoFrameFormat::Format_ARGB8888_Premultiplied) {
        image = QImage(bits,
                       mappedFrame.width(),
                       mappedFrame.height(),
                       stride,
                       QImage::Format_ARGB32).copy();
    } else if (mappedFrame.pixelFormat() == QVideoFrameFormat::Format_XRGB8888) {
        image = QImage(bits,
                       mappedFrame.width(),
                       mappedFrame.height(),
                       stride,
                       QImage::Format_RGB32).copy();
    } else {
        // For other formats, convert if possible
        QImage::Format format = QVideoFrameFormat::imageFormatFromPixelFormat(mappedFrame.pixelFormat());
        if (format != QImage::Format_Invalid) {
            image = QImage(bits,
                           mappedFrame.width(),
                           mappedFrame.height(),
                           stride,
                           format).convertToFormat(QImage::Format_ARGB32).copy();
        }
    }
    mappedFrame.unmap();
    if (!image.isNull()) {
        QMutexLocker locker(&m_mutex);
        m_currentFrame = image;
    }
}
QImage ImageProvider::currentFrame() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentFrame;
}
