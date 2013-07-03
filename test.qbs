import qbs

Application {
    Depends { name: "Qt"; submodules: [ "core", "gui", "svg" ] }
    files: "main.cpp"
    //qtconf: { Binaries: "." }

    someprop: {
        print(Qt.core.moduleLibFilePath);
        print(Qt.gui.moduleLibFilePath);
        print(Qt.svg.moduleLibFilePath);
        print(Qt.core.pluginLibFilePath("qico", "imageformats"));
    }
}
