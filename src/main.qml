import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: qsTr("Network Client")

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 20

        // Left Panel - Controls
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: "#f0f0f0"
            border.color: "#cccccc"
            border.width: 1
            radius: 8

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                Text {
                    text: "Network Client"
                    font.pointSize: 16
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                // IP Address Input
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Text {
                        text: "IP Address:"
                        font.pointSize: 10
                    }

                    TextField {
                        id: ipInput
                        Layout.fillWidth: true
                        text: "10.10.13.56"
                        placeholderText: "Enter IP address"
                        enabled: networkClient && !networkClient.connected
                    }
                }

                // Port Input
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Text {
                        text: "Port:"
                        font.pointSize: 10
                    }

                    TextField {
                        id: portInput
                        Layout.fillWidth: true
                        text: "10086"
                        placeholderText: "Enter port"
                        validator: IntValidator { bottom: 1; top: 65535 }
                        enabled: networkClient && !networkClient.connected
                    }
                }

                // Connect and Disconnect Buttons Row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    
                    Button {
                        id: connectButton
                        Layout.fillWidth: true
                        text: "Connect"
                        enabled: networkClient && !networkClient.connected && ipInput.text.length > 0 && portInput.text.length > 0
                        
                        background: Rectangle {
                            color: connectButton.enabled ? (connectButton.pressed ? "#45a049" : "#4CAF50") : "#cccccc"
                            border.color: "#45a049"
                            border.width: 1
                            radius: 4
                        }

                        onClicked: {
                            networkClient.connectToServer(ipInput.text, parseInt(portInput.text))
                        }
                    }

                    Button {
                        id: disconnectButton
                        Layout.fillWidth: true
                        text: "Disconnect"
                        enabled: networkClient && networkClient.connected
                        
                        background: Rectangle {
                            color: disconnectButton.enabled ? (disconnectButton.pressed ? "#da190b" : "#f44336") : "#cccccc"
                            border.color: "#da190b"
                            border.width: 1
                            radius: 4
                        }

                        onClicked: {
                            networkClient.disconnectFromServer()
                        }
                    }
                }

                // Status Display
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 80
                    color: "white"
                    border.color: "#cccccc"
                    border.width: 1
                    radius: 4

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 8

                        Text {
                            text: "Status: " + (networkClient ? networkClient.statusMessage : "Initializing...")
                            font.pointSize: 9
                            wrapMode: Text.WordWrap
                            width: parent.width
                        }
                    }
                }

                // Connection status indicator
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    color: (networkClient && networkClient.connected) ? "#4CAF50" : "#f44336"
                    radius: 4

                    Text {
                        anchors.centerIn: parent
                        text: (networkClient && networkClient.connected) ? "● CONNECTED" : "● DISCONNECTED"
                        color: "white"
                        font.bold: true
                    }
                }

                // Received Data Display
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "white"
                    border.color: "#cccccc"
                    border.width: 1
                    radius: 4

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 5

                        Text {
                            text: "Received Data:"
                            font.pointSize: 10
                            font.bold: true
                        }

                        ScrollView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            TextArea {
                                text: networkClient ? networkClient.receivedData : ""
                                font.pointSize: 8
                                wrapMode: Text.WordWrap
                                readOnly: true
                                selectByMouse: true
                            }
                        }
                    }
                }
            }
        }

        // Right Panel - Multi Video Display
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ffffff"
            border.color: "#cccccc"
            border.width: 1
            radius: 8

            GridLayout {
                id: videoGrid
                anchors.fill: parent
                anchors.margins: 4
                
                columns: getGridColumns(networkClient ? networkClient.activePipes.length : 0)
                rows: getGridRows(networkClient ? networkClient.activePipes.length : 0)
                columnSpacing: 2
                rowSpacing: 2
                
                function getGridColumns(pipeCount) {
                    if (pipeCount <= 1) return 1;
                    if (pipeCount <= 2) return 1;  // 2 pipes: 1 column, 2 rows
                    if (pipeCount <= 4) return 2;  // 3-4 pipes: 2 columns
                    return 2; // Max 4 pipes supported
                }
                
                function getGridRows(pipeCount) {
                    if (pipeCount <= 1) return 1;
                    if (pipeCount <= 2) return 2;  // 2 pipes: 1 column, 2 rows
                    if (pipeCount <= 3) return 2;  // 3 pipes: 2 rows (2+1)
                    if (pipeCount <= 4) return 2;  // 4 pipes: 2 rows, 2 columns each
                    return 2; // Max 4 pipes supported
                }

                // Video area components
                Repeater {
                    id: videoRepeater
                    model: 4 // Max 4 video areas
                    
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        // For 3 pipes, make the third one span both columns
                        Layout.columnSpan: (networkClient && networkClient.activePipes.length === 3 && index === 2) ? 2 : 1
                        color: "#ffffff"
                        border.color: "#333333"
                        border.width: 1
                        radius: 4
                        
                        property int pipeId: -1
                        property bool isActive: networkClient && index < networkClient.activePipes.length
                        
                        visible: isActive
                        
                        Component.onCompleted: {
                            updatePipeBinding()
                        }
                        
                        function updatePipeBinding() {
                            if (networkClient && index < networkClient.activePipes.length) {
                                pipeId = networkClient.activePipes[index]
                            } else {
                                pipeId = -1
                            }
                        }
                        
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 2
                            spacing: 2
                            
                            // Pipe indicator bar (above image)
                            Rectangle {
                                id: pipeIndicator
                                Layout.fillWidth: true
                                Layout.preferredHeight: 25
                                color: "#f0f0f0"
                                border.color: "#cccccc"
                                border.width: 1
                                radius: 3
                                visible: parent.parent.pipeId >= 0
                                
                                property int currentPipeId: parent.parent.pipeId
                                property int frameValue: networkClient && currentPipeId >= 0 ? networkClient.getFrameForPipe(currentPipeId) : -1
                                property real fpsValue: networkClient && currentPipeId >= 0 ? networkClient.getFpsForPipe(currentPipeId) : 0.0
                                
                                Row {
                                    anchors.centerIn: parent
                                    spacing: 10
                                    
                                    Text {
                                        text: "PIPE " + (pipeIndicator.currentPipeId >= 0 ? pipeIndicator.currentPipeId : "")
                                        color: "#000000"
                                        font.pointSize: 8
                                        font.bold: true
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    
                                    Text {
                                        text: "Frame: " + (pipeIndicator.frameValue >= 0 ? pipeIndicator.frameValue : "N/A")
                                        color: "#000000"
                                        font.pointSize: 7
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                    
                                    Text {
                                        text: "FPS: " + (pipeIndicator.fpsValue > 0 ? pipeIndicator.fpsValue.toFixed(1) : "0.0")
                                        color: "#000000"
                                        font.pointSize: 7
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }
                                
                                // Update properties when frame info changes
                                Connections {
                                    target: networkClient
                                    function onFrameInfoChanged() {
                                        if (pipeIndicator.currentPipeId >= 0) {
                                            pipeIndicator.frameValue = networkClient.getFrameForPipe(pipeIndicator.currentPipeId)
                                            pipeIndicator.fpsValue = networkClient.getFpsForPipe(pipeIndicator.currentPipeId)
                                        }
                                    }
                                    function onPipeImageChanged(changedPipeId) {
                                        if (pipeIndicator.currentPipeId === changedPipeId) {
                                            pipeIndicator.frameValue = networkClient.getFrameForPipe(changedPipeId)
                                            pipeIndicator.fpsValue = networkClient.getFpsForPipe(changedPipeId)
                                        }
                                    }
                                }
                            }
                            
                            // Image area
                            Image {
                                id: pipeImage
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                fillMode: Image.PreserveAspectFit
                                cache: false
                                
                                source: parent.parent.pipeId >= 0 ? "image://networkimage/pipe" + parent.parent.pipeId + "/" + Date.now() : ""
                                
                                // Handle image updates for this specific pipe
                                Connections {
                                    target: networkClient
                                    function onPipeImageChanged(changedPipeId) {
                                        if (pipeImage.parent.parent.pipeId === changedPipeId) {
                                            var newSource = "image://networkimage/pipe" + changedPipeId + "/" + Date.now()
                                            pipeImage.source = ""
                                            pipeImage.source = newSource
                                        }
                                    }
                                    function onActivePipesChanged() {
                                        pipeImage.parent.parent.updatePipeBinding()
                                        pipeImage.parent.parent.isActive = networkClient && index < networkClient.activePipes.length
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}