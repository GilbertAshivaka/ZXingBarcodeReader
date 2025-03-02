import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Dialogs
import ZXingBarcodeReader 1.0

Window {
    id: root
    width: 800
    height: 600
    visible: true
    title: qsTr("ZXing Barcode Reader")

    // State properties
    property bool cameraActive: false
    property bool flashEnabled: false
    property bool isGeneratorMode: false
    property string generatedBarcodePath: ""

    // BarcodeManager is registered from C++
    BarcodeManager {
        id: barcodeManager
        scanning: cameraActive && !isGeneratorMode
        flashEnabled: root.flashEnabled

        onBarcodeDetected: function(result) {
            resultText.text = result;
            resultPopup.open();
            // Pause briefly to show the result
            scanPauseTimer.start();
        }

        onBarcodeGenerated: {
            saveDialog.open();
        }
    }

    // Timer to briefly pause scanning after a detection
    Timer {
        id: scanPauseTimer
        interval: 2000
        onTriggered: {
            if (cameraActive && !resultPopup.visible && !isGeneratorMode) {
                barcodeManager.scanning = true;
            }
        }
    }

    // Camera for live feed
    CaptureSession {
        id: captureSession
        camera: Camera {
            id: camera
            active: cameraActive && !isGeneratorMode

            // Set flash mode based on the flash enabled state
            flashMode: root.flashEnabled ? Camera.FlashOn : Camera.FlashOff
        }

        videoOutput: videoOutput
    }

    // Frame capture for processing
    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        visible: cameraActive && !isGeneratorMode

        // For video frames processing
        Connections {
            target: videoOutput.videoSink
            function onFrameChanged() {
                if (barcodeManager.scanning && videoOutput.videoSink.hasVideoFrame && !isGeneratorMode) {
                    var frame = videoOutput.videoSink.videoFrame();
                    barcodeManager.processFrame(frame);
                }
            }
        }
    }

    // Barcode Generator UI
    Rectangle {
        id: generatorPanel
        anchors.fill: parent
        color: "#f0f0f0"
        visible: isGeneratorMode

        ColumnLayout {
            anchors.centerIn: parent
            width: parent.width * 0.8
            spacing: 20

            Text {
                text: qsTr("Code 128 Barcode Generator")
                font.pixelSize: 24
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            TextField {
                id: barcodeTextField
                placeholderText: qsTr("Enter text to encode")
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                font.pixelSize: 16
                selectByMouse: true
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                Text {
                    text: qsTr("Width:")
                    font.pixelSize: 16
                }

                SpinBox {
                    id: widthSpinBox
                    from: 100
                    to: 800
                    value: 300
                    stepSize: 50
                }

                Text {
                    text: qsTr("Height:")
                    font.pixelSize: 16
                }

                SpinBox {
                    id: heightSpinBox
                    from: 50
                    to: 400
                    value: 100
                    stepSize: 10
                }
            }

            Button {
                text: qsTr("Generate Barcode")
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                enabled: barcodeTextField.text.length > 0

                onClicked: {
                    var generatedImage = barcodeManager.generateCode128(
                        barcodeTextField.text,
                        widthSpinBox.value,
                        heightSpinBox.value
                    );
                    generatedBarcodeImage.source = "";  // Clear current image
                    generatedBarcodeImage.image = generatedImage;
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 400
                Layout.preferredHeight: 200
                border.color: "#cccccc"
                border.width: 1
                color: "white"

                Image {
                    id: generatedBarcodeImage
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                    property var image: null

                    onImageChanged: {
                        if (image) {
                            source = "image://generated/" + Date.now();
                        }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: qsTr("Generated barcode will appear here")
                    visible: generatedBarcodeImage.source == ""
                    color: "#888888"
                }
            }

            Button {
                text: qsTr("Save Barcode")
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                enabled: generatedBarcodeImage.image !== null

                onClicked: {
                    saveDialog.open();
                }
            }
        }
    }

    // Save file dialog
    FileDialog {
        id: saveDialog
        title: "Save Barcode"
        nameFilters: ["PNG files (*.png)", "JPEG files (*.jpg)"]
        fileMode: FileDialog.SaveFile

        onAccepted: {
            if (generatedBarcodeImage.image) {
                barcodeManager.saveBarcode(generatedBarcodeImage.image, selectedFile);
                generatedBarcodePath = selectedFile;
                savedPopup.open();
            }
        }
    }

    // Saved notification popup
    Popup {
        id: savedPopup
        anchors.centerIn: parent
        width: 300
        height: 150
        modal: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Text {
                text: qsTr("Barcode Saved")
                font.pixelSize: 18
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: qsTr("Barcode has been saved to:") + "\n" + generatedBarcodePath
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignHCenter
                onClicked: savedPopup.close()
            }
        }
    }

    // Controls overlay
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 80
        color: "black"
        opacity: 0.7

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            Button {
                text: isGeneratorMode ? qsTr("Scanner Mode") : qsTr("Generator Mode")
                onClicked: {
                    isGeneratorMode = !isGeneratorMode;
                    if (isGeneratorMode) {
                        cameraActive = false;
                        barcodeManager.scanning = false;
                    }
                }
                Layout.preferredWidth: 120
            }

            Button {
                text: cameraActive ? qsTr("Stop Camera") : qsTr("Start Camera")
                enabled: !isGeneratorMode
                onClicked: {
                    cameraActive = !cameraActive;
                    if (!cameraActive) {
                        barcodeManager.scanning = false;
                    }
                }
                Layout.preferredWidth: 120
            }

            Button {
                text: root.flashEnabled ? qsTr("Flash Off") : qsTr("Flash On")
                enabled: cameraActive && !isGeneratorMode
                onClicked: {
                    root.flashEnabled = !root.flashEnabled;
                }
                Layout.preferredWidth: 120
            }

            Text {
                Layout.fillWidth: true
                text: isGeneratorMode ? qsTr("Create Code 128 barcodes") : qsTr("Scan a 1D barcode")
                color: "white"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    // Scanning guide overlay
    Rectangle {
        id: scanGuide
        anchors.centerIn: parent
        width: parent.width * 0.8
        height: 100
        color: "transparent"
        border.color: "red"
        border.width: 2
        visible: cameraActive && barcodeManager.scanning && !isGeneratorMode
    }

    // Result popup
    Popup {
        id: resultPopup
        anchors.centerIn: parent
        width: parent.width * 0.8
        height: 200
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        onOpened: {
            barcodeManager.scanning = false;
        }

        onClosed: {
            if (cameraActive && !isGeneratorMode) {
                barcodeManager.scanning = true;
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Text {
                text: qsTr("Barcode Detected")
                font.pixelSize: 20
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                id: resultText
                text: ""
                font.pixelSize: 18
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                Layout.fillWidth: true
                Layout.fillHeight: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Button {
                text: qsTr("OK")
                Layout.alignment: Qt.AlignHCenter
                onClicked: resultPopup.close()
            }
        }
    }

    // Instructions when camera is not active and not in generator mode
    Rectangle {
        anchors.fill: parent
        color: "black"
        visible: !cameraActive && !isGeneratorMode

        Text {
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 24
            text: qsTr("Press 'Start Camera' to begin scanning barcodes\nor 'Generator Mode' to create barcodes")
            width: parent.width * 0.8
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
