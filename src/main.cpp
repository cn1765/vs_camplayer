#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "NetworkClient.h"
#include "ImageProvider.h"
#include "Logger.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 注册NetworkClient类型，这样QML才能使用
    qmlRegisterType<NetworkClient>("NetworkClient", 1, 0, "NetworkClient");

    QQmlApplicationEngine engine;
    
    // 创建NetworkClient实例并设置为上下文属性
    NetworkClient networkClient;
    engine.rootContext()->setContextProperty("networkClient", &networkClient);
    
    // 创建图像提供器
    ImageProvider *imageProvider = new ImageProvider();
    imageProvider->setNetworkClient(&networkClient);
    engine.addImageProvider("networkimage", imageProvider);
    
    // 连接NetworkClient的图像更新信号到ImageProvider
    QObject::connect(&networkClient, &NetworkClient::currentImageChanged, 
                     [imageProvider, &networkClient]() {
        LOG_DEBUG("=== main.cpp currentImageChanged slot START ===");
        QImage currentImg = networkClient.currentImage();
        LOG_DEBUG("NetworkClient current image size:" << currentImg.size() << "isNull:" << currentImg.isNull());
        imageProvider->setImage(currentImg);
        LOG_DEBUG("=== main.cpp currentImageChanged slot END ===");
    });
    
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
