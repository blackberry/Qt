import QtQuick 1.1

Rectangle {
    width: 360
    height: 360
    property bool flag: false

    Text {
        text: qsTr("Hello World")
        anchors.centerIn: parent
    }

    Rectangle {
        id: swicth
        width: 250
        height: 250
        anchors.centerIn: parent

        color: flag? "grey" : "lightgrey"
        border.color: flag? "lightgrey" : "grey"
        border.width: 2


        Text {
            anchors.centerIn: parent
            text: flag? "On" : "Off"
        }

    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            flag ? socket.writeData("Swicth on!") : socket.writeData("Swicth off!")
            flag = !flag
        }
    }

    Component.onCompleted: {
        socket.connectToHost("Mehdis-MacBook-Pro.local", 50001);
    }
}
