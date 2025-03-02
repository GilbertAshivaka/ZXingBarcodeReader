#include "generatedimageprovider.h"
#include <QMutexLocker>

GeneratedImageProvider::GeneratedImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    // Initialize with a transparent image
    m_image = QImage(300, 100, QImage::Format_ARGB32);
    m_image.fill(Qt::transparent);
}

QImage GeneratedImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);

    QMutexLocker locker(&m_mutex);

    if (size)
        *size = m_image.size();

    if (requestedSize.width() > 0 && requestedSize.height() > 0) {
        return m_image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return m_image;
}

void GeneratedImageProvider::updateImage(const QImage &image)
{
    if (image.isNull())
        return;

    QMutexLocker locker(&m_mutex);
    m_image = image;
}
