import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects 

Window {
    id: root
    width: 1300
    height: 900
    visible: true
    title: "Nebula"
    color: "#050505"

    Image {
        id: bgSource
        anchors.fill: parent
        source: Backend.coverArt
        visible: false
    }
    FastBlur {
        anchors.fill: parent
        source: bgSource
        radius: 100
        cached: true
    }
    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: Backend.coverArt == "" ? 0.9 : 0.45
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 25

        // SIDEBAR
        Rectangle {
            Layout.preferredWidth: 260
            Layout.fillHeight: true
            Layout.bottomMargin: 110
            color: Qt.rgba(0, 0, 0, 0.3) 
            radius: 20
            border.color: Qt.rgba(1, 1, 1, 0.08)
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 25
                spacing: 20

                Text {
                    text: "Nebula"
                    font.pixelSize: 32
                    font.weight: Font.Light
                    font.letterSpacing: 1
                    color: "white"
                    opacity: 0.9
                    layer.enabled: true
                    layer.effect: DropShadow { color: Qt.rgba(1,1,1,0.5); radius: 8 }
                }

                Text { text: "BIBLIOTECA"; font.pixelSize: 11; font.bold: true; color: "#888"; font.letterSpacing: 2 }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: Backend.playlistNames
                    clip: true
                    spacing: 8
                    delegate: Rectangle {
                        id: playlistItem
                        width: parent.width
                        height: 45
                        radius: 12
                        color: modelData === Backend.currentPlaylistName ? Qt.rgba(1,1,1,0.15) : "transparent"
                        Behavior on color { ColorAnimation { duration: 150 } }
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15; anchors.rightMargin: 15
                            Text {
                                text: modelData
                                color: modelData === Backend.currentPlaylistName ? "white" : "#aaa"
                                font.pixelSize: 15
                                font.weight: modelData === Backend.currentPlaylistName ? Font.Bold : Font.Normal
                                Layout.fillWidth: true
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: Backend.switchPlaylist(modelData)
                            cursorShape: Qt.PointingHandCursor
                            hoverEnabled: true
                            onEntered: if(modelData !== Backend.currentPlaylistName) playlistItem.color = Qt.rgba(1,1,1,0.05)
                            onExited: if(modelData !== Backend.currentPlaylistName) playlistItem.color = "transparent"
                        }
                    }
                }
                Button {
                    text: "+ Nueva Playlist"
                    Layout.fillWidth: true
                    background: Rectangle { color: Qt.rgba(1,1,1,0.08); radius: 10 }
                    contentItem: Text { text: parent.text; color: "white"; horizontalAlignment: Text.AlignHCenter }
                    onClicked: playlistDialog.open()
                }
            }
        }

        // PANEL CENTRAL
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.bottomMargin: 110
            color: Qt.rgba(0, 0, 0, 0.2) 
            radius: 20
            border.color: Qt.rgba(1, 1, 1, 0.05)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 35
                spacing: 25

                RowLayout {
                    spacing: 25
                    Rectangle {
                        width: 140; height: 140
                        radius: 12
                        color: Qt.rgba(1,1,1,0.05)
                        layer.enabled: true
                        layer.effect: DropShadow { color: "black"; radius: 15; opacity: 0.3 }
                        Image {
                            anchors.fill: parent
                            source: Backend.coverArt
                            fillMode: Image.PreserveAspectCrop
                            visible: Backend.coverArt !== ""
                            layer.enabled: true
                            layer.effect: OpacityMask { maskSource: Rectangle { width: 140; height: 140; radius: 12 } }
                        }
                        Text { anchors.centerIn: parent; text: "💿"; font.pixelSize: 50; visible: Backend.coverArt === "" }
                    }
                    ColumnLayout {
                        spacing: 5
                        Text { text: "PLAYLIST"; color: "#aaa"; font.pixelSize: 12; font.bold: true; font.letterSpacing: 1 }
                        Text { text: Backend.currentPlaylistName; color: "white"; font.pixelSize: 56; font.weight: Font.Bold }
                        Row {
                            spacing: 10
                            Button {
                                text: "Reproducir"
                                background: Rectangle { color: "white"; radius: 20 }
                                contentItem: Text { text: parent.text; color: "black"; font.bold: true; padding: 12; horizontalAlignment: Text.AlignHCenter }
                                onClicked: Backend.playIndex(0)
                            }
                            Button {
                                text: "Añadir Canciones"
                                background: Rectangle { color: Qt.rgba(1,1,1,0.2); radius: 20 }
                                contentItem: Text { text: parent.text; color: "white"; font.bold: true; padding: 12; horizontalAlignment: Text.AlignHCenter }
                                onClicked: fileDialog.open()
                            }
                        }
                    }
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: Backend.currentTrackList
                    spacing: 2
                    delegate: Rectangle {
                        id: trackRow
                        width: parent.width
                        height: 56
                        color: "transparent"
                        radius: 8
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 15; anchors.rightMargin: 15
                            Item { Layout.preferredWidth: 30; Layout.fillHeight: true; Text { text: (index + 1); color: "#888"; font.pixelSize: 14; anchors.centerIn: parent } }
                            Text { text: modelData; color: "white"; font.pixelSize: 16; font.weight: Font.Medium; Layout.fillWidth: true; elide: Text.ElideRight }
                            Text { text: "MP3"; color: "#666"; font.pixelSize: 12 }
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: trackRow.color = Qt.rgba(1,1,1,0.1)
                            onExited: trackRow.color = "transparent"
                            onClicked: Backend.playIndex(index)
                        }
                    }
                }
            }
        }
    }

    // PLAYER FLOTANTE
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 30
        width: Math.min(parent.width - 60, 950)
        height: 85
        radius: 42.5
        color: Qt.rgba(0.12, 0.12, 0.12, 0.85)
        border.color: Qt.rgba(1, 1, 1, 0.15)
        border.width: 1
        layer.enabled: true
        layer.effect: DropShadow { transparentBorder: true; color: "#80000000"; radius: 25; verticalOffset: 12 }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 20
            anchors.rightMargin: 30
            spacing: 20

            Item {
                Layout.preferredWidth: 55; Layout.preferredHeight: 55
                Image {
                    anchors.fill: parent; source: Backend.coverArt; fillMode: Image.PreserveAspectCrop; visible: Backend.coverArt !== ""
                    layer.enabled: true; layer.effect: OpacityMask { maskSource: Rectangle { width: 55; height: 55; radius: 27.5 } }
                }
                Rectangle { anchors.fill: parent; color: "#222"; radius: 27.5; visible: Backend.coverArt === ""; Text { text: "🎵"; anchors.centerIn: parent } }
            }

            Column {
                Layout.preferredWidth: 200
                Text { text: Backend.title; color: "white"; font.bold: true; font.pixelSize: 14; width: parent.width; elide: Text.ElideRight }
                Text { text: Backend.artist; color: "#aaa"; font.pixelSize: 12; width: parent.width; elide: Text.ElideRight }
            }

            RowLayout {
                Layout.fillWidth: true; Layout.alignment: Qt.AlignHCenter; spacing: 20
                RoundButton { flat: true; onClicked: Backend.prev(); contentItem: Text { text: "⏮"; color: "white"; font.pixelSize: 24; anchors.centerIn: parent } }
                Rectangle {
                    width: 50; height: 50; radius: 25; color: "white"
                    scale: playArea.pressed ? 0.9 : 1.0
                    Behavior on scale { NumberAnimation { duration: 100 } }
                    Text { anchors.centerIn: parent; anchors.horizontalCenterOffset: Backend.isPlaying ? 0 : 2; text: Backend.isPlaying ? "⏸" : "▶"; color: "black"; font.pixelSize: 22 }
                    MouseArea { id: playArea; anchors.fill: parent; onClicked: Backend.playPause(); cursorShape: Qt.PointingHandCursor }
                }
                RoundButton { flat: true; onClicked: Backend.next(); contentItem: Text { text: "⏭"; color: "white"; font.pixelSize: 24; anchors.centerIn: parent } }
            }

            // --- VISUALIZER + SLIDER ---
            ColumnLayout {
                Layout.preferredWidth: 280
                spacing: 2
                
                // VISUALIZER ALEATORIO (BARRA DE BARRAS)
                Row {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBottom
                    height: 20
                    spacing: 2
                    visible: Backend.isPlaying // Solo visible si suena
                    
                    Repeater {
                        model: 40 // Número de barritas
                        Rectangle {
                            width: (parent.width / 40) - 2
                            height: 2
                            color: Qt.rgba(1,1,1,0.4)
                            radius: 2
                            anchors.bottom: parent.bottom
                            
                            // Animación Random
                            SequentialAnimation on height {
                                loops: Animation.Infinite
                                running: Backend.isPlaying
                                NumberAnimation { 
                                    to: Math.random() * 20 + 2 // Altura aleatoria entre 2 y 22
                                    duration: Math.random() * 200 + 100 // Duración aleatoria
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Text { text: formatTime(Backend.position); color: "#888"; font.pixelSize: 11 }
                    Slider {
                        id: seekSlider
                        Layout.fillWidth: true
                        from: 0; to: Backend.duration
                        value: pressed ? value : Backend.position
                        onPressedChanged: if(!pressed) Backend.setPosition(value)
                        
                        background: Rectangle {
                            x: seekSlider.leftPadding
                            y: seekSlider.topPadding + seekSlider.availableHeight / 2 - height / 2
                            width: seekSlider.availableWidth; height: 4
                            radius: 2; color: "#333"
                            Rectangle { width: seekSlider.visualPosition * parent.width; height: 4; color: "white"; radius: 2 }
                        }
                        handle: Rectangle {
                            x: seekSlider.leftPadding + seekSlider.visualPosition * (seekSlider.availableWidth - width)
                            y: seekSlider.topPadding + seekSlider.availableHeight / 2 - height / 2
                            width: 12; height: 12; radius: 6; color: "white"
                        }
                    }
                    Text { text: formatTime(Backend.duration); color: "#888"; font.pixelSize: 11 }
                }
            }
        }
    }

    FileDialog { id: fileDialog; nameFilters: ["Música (*.mp3 *.m4a *.mp4 *.flac)"]; fileMode: FileDialog.OpenFiles; onAccepted: { for(var i=0; i<selectedFiles.length; i++) Backend.addToCurrentPlaylist(selectedFiles[i]) } }
    Dialog { id: playlistDialog; title: "Nueva Playlist"; standardButtons: Dialog.Ok | Dialog.Cancel; anchors.centerIn: parent; ColumnLayout { TextField { id: playlistNameField; placeholderText: "Nombre..."; color: "black" } } onAccepted: { Backend.createPlaylist(playlistNameField.text); Backend.switchPlaylist(playlistNameField.text); playlistNameField.text = "" } }
    function formatTime(ms) { var m = Math.floor(ms / 60000); var s = Math.floor((ms % 60000) / 1000); return m + ":" + (s < 10 ? "0" + s : s); }
}
