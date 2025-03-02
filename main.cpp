#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include "barcodemanager.h"
#include "imageprovider.h"
#include "generatedimageprovider.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Register the BarcodeManager type to QML
    qmlRegisterType<BarcodeManager>("ZXingBarcodeReader", 1, 0, "BarcodeManager");

    // Create our image providers
    ImageProvider *imageProvider = new ImageProvider();
    GeneratedImageProvider *generatedImageProvider = new GeneratedImageProvider();

    // Create our barcode manager
    BarcodeManager *barcodeManager = new BarcodeManager();

    QQmlApplicationEngine engine;

    // Add the image providers
    engine.addImageProvider("camera", imageProvider);
    engine.addImageProvider("generated", generatedImageProvider);

    // Set up connections between barcode manager and image provider
    QObject::connect(barcodeManager, &BarcodeManager::barcodeGenerated, [=]() {
        // When a barcode is generated, update the image provider
        if (generatedImageProvider) {
            QMetaObject::invokeMethod(barcodeManager, [=]() {
                QImage barcodeImage = generatedImageProvider->property("image").value<QImage>();
                if (!barcodeImage.isNull()) {
                    generatedImageProvider->updateImage(barcodeImage);
                }
            });
        }
    });

    // Expose the image providers to QML
    engine.rootContext()->setContextProperty("imageProvider", imageProvider);
    engine.rootContext()->setContextProperty("generatedImageProvider", generatedImageProvider);

    // Expose the barcode manager to QML
    engine.rootContext()->setContextProperty("barcodeManager", barcodeManager);

    engine.loadFromModule("ZXingBarcodeReader", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
