#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

class NetworkClient;

class ImageProvider : public QQuickImageProvider
{
public:
    ImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    void setImage(const QImage &image);
    void setNetworkClient(NetworkClient *client);

private:
    QImage m_image;
    NetworkClient *m_networkClient;
};

#endif // IMAGEPROVIDER_H