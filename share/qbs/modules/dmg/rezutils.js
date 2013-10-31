function _numberToHexString(n) {
    var hex = parseInt(n).toString(16);
    return n >= 16 ? hex : "0" + hex;
}

// Converts an array of bytes to a string representing the body of a Rez data section
function byteArrayToRezString(bytes) {
    var str = "";

    for (var i = 0; i < bytes.length; i += 16) {
        str += '    $"';

        // Get the 16 bytes for this line and convert them to an array of 8 hex strings
        var hexChars = bytes.slice(i, i + 16).map(function(e) { return _numberToHexString(e); });

        // Write out each character with a space every 2 bytes
        for (var j = 0; j < hexChars.length; ++j) {
            if (j > 0 && j % 2 == 0)
                str += " ";
            str += hexChars[j];
        }
        str += '"\n';
    }
    return str;
}

// Returns the JavaScript string (UTF-16 encoded) as a big endian byte array
function numberToByteArray(n, byteCount, endianness) {
    if (endianness !== undefined && endianness !== "big" && endianness !== "little") {
        throw("invalid endianness " + endianness + " valid values are big and little");
    }
    var bytes = [];
    switch (byteCount) {
        case 1:
            bytes.push(n & 0xFF);
            break;
        case 2:
            bytes.push((n & 0xFF00) >> 8);
            bytes.push(n & 0xFF);
            break;
        case 4:
            bytes.push((n & 0xFF000000) >> 32);
            bytes.push((n & 0xFF0000) >> 16);
            bytes.push((n & 0xFF00) >> 8);
            bytes.push(n & 0xFF);
            break;
        default:
            throw("cannot convert number to a " + byteCount + "-byte array");
    }
    return endianness === "little" ? bytes.reverse() : bytes;
}
