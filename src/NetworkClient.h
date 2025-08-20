#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>
#include <QDebug>
#include <QImage>
#include <QDateTime>
#include <QHash>
#include <opencv2/opencv.hpp>

#include "utils.h"

class NetworkClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString receivedData READ receivedData NOTIFY receivedDataChanged)
    Q_PROPERTY(QImage currentImage READ currentImage NOTIFY currentImageChanged)
    Q_PROPERTY(int imageWidth READ imageWidth NOTIFY currentImageChanged)
    Q_PROPERTY(int imageHeight READ imageHeight NOTIFY currentImageChanged)
    Q_PROPERTY(int currentPipe READ currentPipe NOTIFY frameInfoChanged)
    Q_PROPERTY(int currentFrame READ currentFrame NOTIFY frameInfoChanged)
    Q_PROPERTY(double currentFps READ currentFps NOTIFY frameInfoChanged)
    Q_PROPERTY(QVariantList activePipes READ activePipes NOTIFY activePipesChanged)

public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient();

    bool connected() const { return m_connected; }
    QString statusMessage() const { return m_statusMessage; }
    QString receivedData() const { return m_receivedData; }
    QImage currentImage() const { return m_currentImage; }
    int imageWidth() const { return m_currentImage.width(); }
    int imageHeight() const { return m_currentImage.height(); }
    int currentPipe() const { return m_currentPipe; }
    int currentFrame() const { return m_currentFrame; }
    double currentFps() const { return m_currentFps; }
    QVariantList activePipes() const { return m_activePipesList; }
    
    Q_INVOKABLE QImage getImageForPipe(int pipeId);
    Q_INVOKABLE int getFrameForPipe(int pipeId);
    Q_INVOKABLE double getFpsForPipe(int pipeId);

public slots:
    void connectToServer(const QString &ip, int port);
    void disconnectFromServer();

signals:
    void connectedChanged();
    void statusMessageChanged();
    void receivedDataChanged();
    void currentImageChanged();
    void frameInfoChanged();
    void activePipesChanged();
    void pipeImageChanged(int pipeId);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    void setConnected(bool connected);
    void setStatusMessage(const QString &message);
    void setReceivedData(const QString &data);
    void setCurrentImage(const QImage &image);
    void sendStartMessage();
    void sendStopMessage();
    void processReceivedData();
    void processMessage(const QByteArray &data);
    QImage convertNV12ToRGB(const QByteArray &nv12Data, int width, int height);
    QImage addOverlayToImage(const QImage &image, int pipe, int frame, double fps);

    QTcpSocket *m_socket;
    bool m_connected;
    QString m_statusMessage;
    QString m_receivedData;
    QString m_authMessage;
    QImage m_currentImage;
    
    // Protocol state
    enum ReceiveState {
        WAITING_FOR_HEADER,
        WAITING_FOR_BODY
    };
    
    ReceiveState m_receiveState;
    cmd_header_new_t m_currentHeader;
    quint32 m_width;
    quint32 m_height;
    quint32 m_pipe;
    QByteArray m_receiveBuffer;
    quint32 m_expectedBodyLength;
    
    // Frame tracking for FPS calculation
    int m_currentPipe;
    int m_currentFrame; 
    double m_currentFps;
    QDateTime m_lastFrameTime;
    int m_lastFrameId;
    
    // Multi-pipe support
    struct PipeData {
        QImage image;
        int frameId;
        double fps;
        QDateTime lastFrameTime;
        int lastFrameId;
        quint32 width;
        quint32 height;
    };
    
    QHash<int, PipeData> m_pipeData;
    QVariantList m_activePipesList;
};

#endif // NETWORKCLIENT_H