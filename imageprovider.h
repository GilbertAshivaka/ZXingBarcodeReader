#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>
#include <QVideoFrame>

class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void updateFrame(const QVideoFrame &frame);
    QImage currentFrame() const;

private:
    QImage m_currentFrame;
    mutable QMutex m_mutex;
};

#endif // IMAGEPROVIDER_H
