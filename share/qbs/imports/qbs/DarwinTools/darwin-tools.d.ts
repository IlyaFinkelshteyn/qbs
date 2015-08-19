/**
    * Returns the numeric identifier corresponding to an Apple device name
    * (i.e. for use by TARGETED_DEVICE_FAMILY).
    */
export declare function appleDeviceNumber(deviceName: string): number;
/**
    * Returns the TARGETED_DEVICE_FAMILY string given a list of target device names.
    */
export declare function targetedDeviceFamily(deviceNames: string[]): string;
/**
    * Returns the most appropriate Apple platform name given a targetOS list.
    * Possible platform names include macosx, iphoneos, and iphonesimulator.
    */
export declare function applePlatformName(targetOSList: string[]): string;
/**
    * Replace characters unsafe for use in a domain name with a '-' character (RFC 1034).
    */
export declare function rfc1034(inStr: string): string;
/**
    * Returns the localization of the resource at the given path,
    * or undefined if the path does not contain an {xx}.lproj path segment.
    */
export declare function localizationKey(path: string): string;
/**
    * Returns the path of a localized resource at the given path,
    * relative to its containing {xx}.lproj directory, or '.'
    * if the resource is unlocalized (not contained in an lproj directory).
    */
export declare function relativeResourcePath(path: string): string;
/**
    * Recursively perform variable replacements in an environment dictionary.
    *
    * JSON.stringify(expandPlistEnvironmentVariables({a:"$(x)3$$(y)",b:{t:"%$(y) $(k)"}},
    *                                                {x:"X",y:"Y"}, true))
    *    Warning undefined variable  k  in variable expansion
    * => {"a":"X3$Y","b":{"t":"%Y $(k)"}}
    */
export declare function expandPlistEnvironmentVariables(obj: {
    [key: string]: any;
}, env: {
    [key: string]: string;
}, warn: boolean): {
    [key: string]: any;
};
/**
    * Recursively removes any undefined, null, or empty string values from the property list.
    */
export declare function cleanPropertyList(plist: {
    [key: string]: any;
}): void;
