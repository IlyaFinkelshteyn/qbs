import qbs

AppleDiskImage {
    name: "hellodmg"
    targetName: "hellodmg-1.0-" + qbs.architecture

    dmg.volumeName: "Hello DMG"
    dmg.volumeIcon: "hello.icns"
    dmg.internetEnable: true

    dmg.createDSStore: false
    //dmg.backgroundImage: "hello.tif"
    dmg.licenses: [
        {
            "Locale": "en",
            "Filename": "eula.txt"
        },
        {
            "Locale": "de",
            "Filename": "eula-de.txt"
        },
        {
            "Locale": "fr",
            "Filename": "eula-fr.txt"
        },
        {
            "Locale": "ja",
            "Filename": "eula-ja.txt"
        },
        {
            "Locale": "ko",
            "Filename": "eula-ko.rtf"
        },
        {
            "Locale": "zh_CN",
            "Filename": "eula-zh_CN.odt"
        },
        {
            "Locale": "zh_TW",
            "Filename": "eula-zh_TW.docx"
        }
    ]

    dmg.files: ["eula.txt"]
    dmg.directories: ["Hello.app"]

    dmg.extraStagingCommands: [ [ "touch", "Pandora's Box" ] ]
}
