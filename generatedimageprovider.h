#ifndef GENERATEDIMAGEPROVIDER_H
#define GENERATEDIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>

class GeneratedImageProvider : public QQuickImageProvider
{
public:
    GeneratedImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    void updateImage(const QImage &image);

private:
    QImage m_image;
    mutable QMutex m_mutex;
};

#endif // GENERATEDIMAGEPROVIDER_H
