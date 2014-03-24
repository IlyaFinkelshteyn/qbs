import qbs 1.0

UnixDMD {
    condition: qbs.targetOS.contains('linux') && qbs.toolchain.contains('dmd')
    rpaths: ['$ORIGIN']
}

