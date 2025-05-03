import QtQuick
import QtQuick.Controls

ApplicationWindow {
    visible: false
    id: root

    Flyout {
        id: flyout
        visible: false
    }

    Connections {
        target: trayHandler
        function onTrayClicked() {
            flyout.toggleVisibility();
            flyout.open();
        }
    }
}
