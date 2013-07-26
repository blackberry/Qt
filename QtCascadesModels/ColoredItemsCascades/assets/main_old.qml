import bb.cascades 1.0

Page {
    content: Container {
        ListView {
            layout: FlowListLayout { }
            dataModel: myCacsadesDataModel
            listItemComponents: [
                ListItemComponent {
                    Container {
                        background: Color.create (ListItemData.color)
                        layoutProperties: FlowListLayoutProperties {
                            aspectRatio: 15/1
                            fillRatio: 1
                        }
                        Label {
                            text: ListItemData.name
                            horizontalAlignment: HorizontalAlignment.Center
                        }
                    }
                }
            ]
        }
    }
}
