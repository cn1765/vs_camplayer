import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("VS CamPlayer")
    
    property bool buttonClicked: false
    
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#f0f0f0" }
            GradientStop { position: 1.0; color: "#e0e0e0" }
        }
        
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 30
            
            Text {
                id: titleText
                text: "VS CamPlayer"
                font.pointSize: 32  // Changed from pixSize to pointSize
                font.bold: true
                color: "#333333"
                Layout.alignment: Qt.AlignHCenter
            }
            
            Button {
                id: mainButton
                text: buttonClicked ? "已点击!" : "点击我"
                font.pointSize: 18  // Changed from pixSize to pointSize
                Layout.preferredWidth: 200
                Layout.preferredHeight: 60
                Layout.alignment: Qt.AlignHCenter
                
                background: Rectangle {
                    color: mainButton.pressed ? "#0066cc" : "#0080ff"
                    radius: 10
                    border.width: 2
                    border.color: "#0066cc"
                }
                
                contentItem: Text {
                    text: mainButton.text
                    font: mainButton.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    buttonClicked = true
                    displayText.visible = true
                    clickAnimation.start()
                }
            }
            
            Text {
                id: displayText
                text: "按钮被点击了！🎉"
                font.pointSize: 24  // Changed from pixSize to pointSize
                color: "#008000"
                visible: false
                Layout.alignment: Qt.AlignHCenter
                
                NumberAnimation on opacity {
                    id: clickAnimation
                    from: 0.0
                    to: 1.0
                    duration: 500
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}