import qbs

Depends {
    // 'dependency' is an internal property on Depends referring to the product of the dependency

    // Instead of dependencyTypes, additionalFileTags concats the file tags declared here to each
    // output artifact of the dependency
    additionalFileTags: {
        switch (linkType) {
        case "weak":
            return dependency.type.contains("dynamiclibrary") ? ["dynamiclibrary_weak"] : [];
        case "whole-archive":
            return dependency.type.contains("staticlibrary") ? ["staticlibrary_whole_archive"] : [];
        case "none":
            return ["no_link"];
        }
    }

    // Do NOT expose these on artifacts in prepare scripts; only module properties
    property string linkType: "strong"

    bundle.embed: !(!dependency.bundle.isBundle && product.type.contains("staticlibrary"))
    bundle.embedSubPath: {
        if (product.type.contains("dynamiclibrary") || product.type.contains("staticlibrary"))
            return product.bundle.frameworksFolderPath;
        if (product.type.contains("loadablemodule"))
            return product.bundle.pluginsFolderPath;
        return product.bundle.unlocalizedResourcesFolderPath;
    }
}
