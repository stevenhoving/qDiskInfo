import QtQuick
import QtQuick.Controls

Popup {
    id: flyout
    width: 200
    height: 100
    modal: false
    focus: true
    //x: Screen.width - width - 20
    //y: Screen.height - height - 50

    Rectangle {
        anchors.fill: parent
        color: "#333"
        radius: 8
        border.color: "#555"
        border.width: 1

        Label {
            text: "Hello, World!"
            color: "white"
            anchors.centerIn: parent
        }
    }

    function toggleVisibility() {
        console.log("Toggling visibility");
        visible = !visible;
    }
}
