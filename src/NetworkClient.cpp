#include <QHostAddress>
#include <QDataStream>
#include <QDateTime>
#include <QBuffer>
#include <QImage>

#include "NetworkClient.h"
#include "Logger.h"

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_connected(false)
    , m_statusMessage("Disconnected")
    , m_authMessage("AUTH:my_secret_token")
    , m_receiveState(WAITING_FOR_HEADER)
    , m_expectedBodyLength(0)
    , m_currentPipe(0)
    , m_currentFrame(0)
    , m_currentFps(0.0)
    , m_lastFrameId(-1)
{
    // Connect socket signals - 修正错误信号连接
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    // 使用 errorOccurred 信号替代 error
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkClient::onError);

    memset(&m_currentHeader, 0, sizeof(m_currentHeader));
}

NetworkClient::~NetworkClient()
{
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
}

void NetworkClient::connectToServer(const QString &ip, int port)
{
    if (m_connected) {
        setStatusMessage("Already connected");
        return;
    }
    
    setStatusMessage(QString("Connecting to %1:%2...").arg(ip).arg(port));
    m_socket->connectToHost(QHostAddress(ip), port);
}

void NetworkClient::disconnectFromServer()
{
    if (m_connected) {
        sendStopMessage();
        m_socket->disconnectFromHost();
        setStatusMessage("Disconnecting...");
    }
}

void NetworkClient::onConnected()
{
    setConnected(true);
    setStatusMessage("Connected");
    sendStartMessage();
}

void NetworkClient::onDisconnected()
{
    setConnected(false);
    setStatusMessage("Disconnected");
    m_receiveState = WAITING_FOR_HEADER;
    m_receiveBuffer.clear();
    memset(&m_currentHeader, 0, sizeof(m_currentHeader));
    
    // Clear pipe data
    m_pipeData.clear();
    m_activePipesList.clear();
    emit activePipesChanged();
}

void NetworkClient::onReadyRead()
{
    // Append new data to buffer
    m_receiveBuffer.append(m_socket->readAll());
    processReceivedData();
}

void NetworkClient::onError(QAbstractSocket::SocketError error)
{
    QString errorString;
    switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
        errorString = "Remote host closed connection";
        break;
    case QAbstractSocket::HostNotFoundError:
        errorString = "Host not found";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        errorString = "Connection refused";
        break;
    default:
        errorString = m_socket->errorString();
    }
    
    setStatusMessage(QString("Error: %1").arg(errorString));
    setConnected(false);
}

void NetworkClient::sendStartMessage()
{    
    m_socket->write(reinterpret_cast<const char*>(&start_cmd_header_new_t), sizeof(start_cmd_header_new_t));

    m_socket->write(reinterpret_cast<const char*>(&start_transfer_info), sizeof(start_transfer_info));
    
    setStatusMessage("start cmd sent");
}

void NetworkClient::sendStopMessage()
{   
    m_socket->write(reinterpret_cast<const char*>(&stop_cmd_header_new_t), sizeof(stop_cmd_header_new_t));

    m_socket->write(reinterpret_cast<const char*>(&stop_transfer_info), sizeof(stop_transfer_info));
    
    setStatusMessage("stop cmd sent");
}

void NetworkClient::processReceivedData()
{
    while (true) {
        if (m_receiveState == WAITING_FOR_HEADER) {
            // Check if we have enough data for header
            if (m_receiveBuffer.size() < sizeof(m_currentHeader)) {
                break; // Wait for more data
            }
            // Extract header
            memcpy(&m_currentHeader, m_receiveBuffer.constData(), sizeof(m_currentHeader));
            m_receiveBuffer.remove(0, sizeof(m_currentHeader));

            m_width = m_currentHeader.metadata.pic_i.stride;
            m_height = m_currentHeader.metadata.pic_i.height;
            m_pipe = m_currentHeader.packinfo.r_i.pipe_id;
            
            // Extract frame information
            m_currentPipe = m_currentHeader.packinfo.r_i.pipe_id;
            m_currentFrame = m_currentHeader.packinfo.r_i.frame_id;
            
            // Initialize pipe data if new pipe
            if (!m_pipeData.contains(m_currentPipe)) {
                PipeData pipeData;
                pipeData.frameId = -1;
                pipeData.fps = 0.0;
                pipeData.lastFrameId = -1;
                pipeData.width = 0;
                pipeData.height = 0;
                m_pipeData[m_currentPipe] = pipeData;
                
                // Update active pipes list
                if (!m_activePipesList.contains(m_currentPipe)) {
                    m_activePipesList.append(m_currentPipe);
                    emit activePipesChanged();
                }
            }
            
            // Update pipe-specific data
            PipeData &pipeData = m_pipeData[m_currentPipe];
            pipeData.frameId = m_currentFrame;
            pipeData.width = m_width;
            pipeData.height = m_height;
            LOG_DEBUG("Updated pipe data - pipe:" << m_currentPipe << "frame:" << m_currentFrame << "size:" << m_width << "x" << m_height);
            
            // Calculate FPS for this pipe
            QDateTime currentTime = QDateTime::currentDateTime();
            if (pipeData.lastFrameId >= 0 && pipeData.lastFrameTime.isValid()) {
                qint64 timeDiff = pipeData.lastFrameTime.msecsTo(currentTime);
                if (timeDiff > 0) {
                    pipeData.fps = 1000.0 / timeDiff;
                }
            }
            pipeData.lastFrameTime = currentTime;
            pipeData.lastFrameId = m_currentFrame;
            
            // Update current values for backward compatibility
            m_currentFps = pipeData.fps;
            
            LOG_DEBUG("Emitting frameInfoChanged - pipe:" << m_currentPipe << "frame:" << pipeData.frameId << "fps:" << pipeData.fps);
            emit frameInfoChanged();
            
            setStatusMessage(QString("Received header - pipe: %1, frame: %2, width: %3, height: %4, fps: %5")
                           .arg(m_currentPipe)
                           .arg(m_currentFrame)
                           .arg(m_width)
                           .arg(m_height)
                           .arg(QString::number(m_currentFps, 'f', 1)));

            m_receiveState = WAITING_FOR_BODY;
            m_expectedBodyLength = m_width*m_height*3/2;
            
        } else if (m_receiveState == WAITING_FOR_BODY) {
            if (m_receiveBuffer.size() < static_cast<int>(m_expectedBodyLength)) {
                break;
            }
            
            QByteArray body = m_receiveBuffer.left(m_expectedBodyLength);
            m_receiveBuffer.remove(0, m_expectedBodyLength);
            
            processMessage(body);
            
            m_receiveState = WAITING_FOR_HEADER;
            memset(&m_currentHeader, 0, sizeof(m_currentHeader));
            m_expectedBodyLength = 0;
        }
    }
}

void NetworkClient::processMessage(const QByteArray &data)
{
    LOG_DEBUG("=== processMessage START ===");
    LOG_DEBUG("Data length:" << data.length());
    LOG_DEBUG("Width:" << m_width << "Height:" << m_height);
    
    QString messageInfo = QString("receive pipe: %1, Length: %2")
                         .arg(m_pipe)
                         .arg(data.length());
    
    if (!data.isEmpty()) {
        LOG_DEBUG("Data is not empty, proceeding with conversion");
        
        // Convert NV12 data to RGB image and display
        if (m_width > 0 && m_height > 0) {
            LOG_DEBUG("Width and height are valid, calling convertNV12ToRGB");
            QImage rgbImage = convertNV12ToRGB(data, m_width, m_height);
            LOG_DEBUG("Conversion result - isNull:" << rgbImage.isNull() 
                     << "size:" << rgbImage.size() << "format:" << rgbImage.format());
            
            if (!rgbImage.isNull()) {
                LOG_DEBUG("Image is valid, storing without overlay");
                PipeData &pipeData = m_pipeData[m_currentPipe];
                
                // Store image in pipe data without overlay
                pipeData.image = rgbImage;
                
                // For backward compatibility, also set as current image if it's the first/latest pipe
                setCurrentImage(rgbImage);
                
                // Emit pipe-specific image changed signal
                emit pipeImageChanged(m_currentPipe);
                
                LOG_DEBUG("Calling setCurrentImage");
                messageInfo += QString("\nImage converted: %1x%2, pipe: %3, frame: %4, fps: %5")
                              .arg(m_width).arg(m_height).arg(m_currentPipe).arg(m_currentFrame).arg(QString::number(pipeData.fps, 'f', 1));
                LOG_DEBUG("setCurrentImage called successfully");
            } else {
                messageInfo += "\nImage conversion failed";
                LOG_DEBUG("Image conversion FAILED");
            }
        } else {
            LOG_DEBUG("Invalid width or height - width:" << m_width << "height:" << m_height);
            messageInfo += QString("\nInvalid dimensions: %1x%2").arg(m_width).arg(m_height);
        }
        
        QString hexPreview;
        for (int i = 0; i < qMin(data.length(), 20); ++i) {
            hexPreview += QString("%1 ").arg(static_cast<unsigned char>(data[i]), 2, 16, QChar('0'));
        }
        messageInfo += QString("\nData preview: %1").arg(hexPreview);
    } else {
        LOG_DEBUG("Data is EMPTY!");
    }
    
    setReceivedData(messageInfo);
    LOG_DEBUG("=== processMessage END ===");
}

void NetworkClient::setConnected(bool connected)
{
    if (m_connected != connected) {
        m_connected = connected;
        emit connectedChanged();
    }
}

void NetworkClient::setStatusMessage(const QString &message)
{
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged();
    }
}

void NetworkClient::setReceivedData(const QString &data)
{
    m_receivedData = data;
    emit receivedDataChanged();
}

void NetworkClient::setCurrentImage(const QImage &image)
{
    LOG_DEBUG("=== setCurrentImage START ===");
    LOG_DEBUG("New image size:" << image.size() << "format:" << image.format());
    LOG_DEBUG("Current image size:" << m_currentImage.size());
    
    if (m_currentImage != image) {
        LOG_DEBUG("Images are different, updating and emitting signal");
        m_currentImage = image;
        emit currentImageChanged();
        LOG_DEBUG("currentImageChanged signal emitted");
    } else {
        LOG_DEBUG("Images are the same, no update needed");
    }
    LOG_DEBUG("=== setCurrentImage END ===");
}

QImage NetworkClient::convertNV12ToRGB(const QByteArray &nv12Data, int width, int height)
{
    LOG_DEBUG("=== convertNV12ToRGB START ===");
    LOG_DEBUG("Input - width:" << width << "height:" << height);
    LOG_DEBUG("Data size:" << nv12Data.size() << "Expected:" << (width * height * 3 / 2));
    
    if (nv12Data.size() < width * height * 3 / 2) {
        LOG_DEBUG("NV12 data size insufficient - got:" << nv12Data.size() << "needed:" << (width * height * 3 / 2));
        return QImage();
    }
    
    try {
        LOG_DEBUG("Creating OpenCV Mat from NV12 data");
        // Create OpenCV Mat from NV12 data
        cv::Mat nv12Mat(height * 3 / 2, width, CV_8UC1, (void*)nv12Data.constData());
        LOG_DEBUG("NV12 Mat created - size:" << nv12Mat.cols << "x" << nv12Mat.rows << "channels:" << nv12Mat.channels());
        
        cv::Mat rgbMat;
        LOG_DEBUG("Converting NV12 to RGB using OpenCV");
        
        // Convert NV12 to RGB
        cv::cvtColor(nv12Mat, rgbMat, cv::COLOR_YUV2RGB_NV12);
        LOG_DEBUG("RGB Mat created - size:" << rgbMat.cols << "x" << rgbMat.rows << "channels:" << rgbMat.channels());
        
        // Convert OpenCV Mat to QImage
        LOG_DEBUG("Converting OpenCV Mat to QImage");
        QImage qimg(rgbMat.data, rgbMat.cols, rgbMat.rows, rgbMat.step, QImage::Format_RGB888);
        LOG_DEBUG("QImage created - size:" << qimg.size() << "isNull:" << qimg.isNull());
        
        QImage result = qimg.copy(); // Make a deep copy
        LOG_DEBUG("Deep copy made - size:" << result.size() << "isNull:" << result.isNull());
        LOG_DEBUG("=== convertNV12ToRGB END ===");
        return result;
        
    } catch (const cv::Exception& e) {
        LOG_DEBUG("OpenCV error:" << e.what());
        LOG_DEBUG("=== convertNV12ToRGB END (ERROR) ===");
        return QImage();
    }
}

QImage NetworkClient::addOverlayToImage(const QImage &image, int pipe, int frame, double fps)
{
    if (image.isNull()) {
        return image;
    }
    
    try {
        // Convert QImage to OpenCV Mat
        cv::Mat mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::Mat overlayMat;
        cv::cvtColor(mat, overlayMat, cv::COLOR_RGB2BGR);
        
        // Add text overlay
        QString overlayText = QString("pipe:%1 frame:%2 fps:%3")
                             .arg(pipe)
                             .arg(frame)
                             .arg(QString::number(fps, 'f', 1));
        
        // Set text parameters
        cv::Point2f textPos(10, 40);
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = 1.3;
        cv::Scalar textColor(0, 255, 0); // Green color in BGR
        int thickness = 2;

        // Put text on image
        cv::putText(overlayMat, overlayText.toStdString(), textPos, fontFace, fontScale, textColor, thickness);
        
        // Convert back to QImage
        cv::cvtColor(overlayMat, overlayMat, cv::COLOR_BGR2RGB);
        QImage result(overlayMat.data, overlayMat.cols, overlayMat.rows, overlayMat.step, QImage::Format_RGB888);
        
        return result.copy(); // Make a deep copy
        
    } catch (const cv::Exception& e) {
        LOG_DEBUG("OpenCV overlay error:" << e.what());
        return image; // Return original image if overlay fails
    }
}

QImage NetworkClient::getImageForPipe(int pipeId)
{
    if (m_pipeData.contains(pipeId)) {
        return m_pipeData[pipeId].image;
    }
    return QImage();
}

int NetworkClient::getFrameForPipe(int pipeId)
{
    LOG_DEBUG("getFrameForPipe called for pipe:" << pipeId);
    if (m_pipeData.contains(pipeId)) {
        int frameId = m_pipeData[pipeId].frameId;
        LOG_DEBUG("Returning frame ID:" << frameId << "for pipe:" << pipeId);
        return frameId;
    }
    LOG_DEBUG("Pipe" << pipeId << "not found in pipe data");
    return -1;
}

double NetworkClient::getFpsForPipe(int pipeId)
{
    LOG_DEBUG("getFpsForPipe called for pipe:" << pipeId);
    if (m_pipeData.contains(pipeId)) {
        double fps = m_pipeData[pipeId].fps;
        LOG_DEBUG("Returning FPS:" << fps << "for pipe:" << pipeId);
        return fps;
    }
    LOG_DEBUG("Pipe" << pipeId << "not found in pipe data");
    return 0.0;
}