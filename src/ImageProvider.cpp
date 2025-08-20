#include "ImageProvider.h"
#include "NetworkClient.h"
#include "Logger.h"
#include <QColor>

ImageProvider::ImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
    , m_networkClient(nullptr)
{
    LOG_DEBUG("ImageProvider constructor");
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    LOG_DEBUG("=== ImageProvider::requestImage START ===");
    LOG_DEBUG("Requested ID:" << id);
    LOG_DEBUG("Requested size:" << requestedSize);
    
    Q_UNUSED(requestedSize)
    
    // Parse pipe ID from request ID
    // Format expected: "pipe<id>/<timestamp>" e.g., "pipe0/123456789"
    QImage targetImage = m_image;
    
    if (m_networkClient && id.startsWith("pipe")) {
        QString pipeStr = id.split("/").first();
        if (pipeStr.startsWith("pipe")) {
            bool ok;
            int pipeId = pipeStr.mid(4).toInt(&ok);  // Remove "pipe" prefix
            if (ok) {
                LOG_DEBUG("Requesting image for pipe:" << pipeId);
                targetImage = m_networkClient->getImageForPipe(pipeId);
            }
        }
    }
    
    // Return a valid image even if empty
    if (targetImage.isNull()) {
        LOG_DEBUG("Image is null, creating empty 1x1 image");
        QImage emptyImage(1, 1, QImage::Format_RGB888);
        emptyImage.fill(QColor(0, 0, 0, 0)); // Transparent
        
        if (size) {
            *size = emptyImage.size();
            LOG_DEBUG("Size set to:" << *size);
        }
        
        LOG_DEBUG("Returning empty image");
        LOG_DEBUG("=== ImageProvider::requestImage END ===");
        return emptyImage;
    }
    
    if (size) {
        *size = targetImage.size();
        LOG_DEBUG("Size set to:" << *size);
    }
    
    LOG_DEBUG("Returning image - size:" << targetImage.size() << "isNull:" << targetImage.isNull());
    LOG_DEBUG("=== ImageProvider::requestImage END ===");
    return targetImage;
}

void ImageProvider::setImage(const QImage &image)
{
    LOG_DEBUG("=== ImageProvider::setImage START ===");
    LOG_DEBUG("Setting new image - size:" << image.size() << "isNull:" << image.isNull());
    LOG_DEBUG("Previous image size:" << m_image.size());
    m_image = image;
    LOG_DEBUG("Image set successfully - new size:" << m_image.size());
    LOG_DEBUG("=== ImageProvider::setImage END ===");
}

void ImageProvider::setNetworkClient(NetworkClient *client)
{
    m_networkClient = client;
}