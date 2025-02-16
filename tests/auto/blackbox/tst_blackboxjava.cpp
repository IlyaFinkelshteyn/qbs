/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "tst_blackboxjava.h"

#include "../shared.h"
#include <tools/hostosinfo.h>
#include <tools/profile.h>
#include <tools/settings.h>

#include <QJsonDocument>
#include <QTemporaryDir>

using qbs::Internal::HostOsInfo;
using qbs::Profile;
using qbs::Settings;

QMap<QString, QString> TestBlackboxJava::findAndroid(int *status)
{
    QTemporaryDir temp;
    QDir::setCurrent(testDataDir + "/find");
    QbsRunParameters params = QStringList() << "-f" << "find-android.qbs";
    params.buildDirectory = temp.path();
    const int res = runQbs(params);
    if (status)
        *status = res;
    QFile file(temp.path() + "/" + relativeProductBuildDir("find-android") + "/android.json");
    if (!file.open(QIODevice::ReadOnly))
        return QMap<QString, QString> { };
    const auto tools = QJsonDocument::fromJson(file.readAll()).toVariant().toMap();
    return QMap<QString, QString> {
        {"sdk", QDir::fromNativeSeparators(tools["sdk"].toString())},
        {"ndk", QDir::fromNativeSeparators(tools["ndk"].toString())},
    };
}

TestBlackboxJava::TestBlackboxJava() : TestBlackboxBase (SRCDIR "/testdata-java", "blackbox-java")
{
}

void TestBlackboxJava::android()
{
    QFETCH(QString, projectDir);
    QFETCH(QStringList, productNames);
    QFETCH(QList<int>, apkFileCounts);

    int status;
    const auto androidPaths = findAndroid(&status);

    const auto ndkPath = androidPaths["ndk"];
    static const QStringList ndkSamplesDirs = QStringList() << "teapot" << "no-native";
    if (!ndkPath.isEmpty() && !QFileInfo(ndkPath + "/samples").isDir()
            && ndkSamplesDirs.contains(projectDir))
        QSKIP("NDK samples directory not present");

    QDir::setCurrent(testDataDir + "/android/" + projectDir);
    Settings s((QString()));
    Profile p("qbs_autotests-android", &s);
    if (!p.exists() || (status != 0 && !p.value("Android.sdk.ndkDir").isValid()))
        QSKIP("No suitable Android test profile");
    QbsRunParameters params(QStringList("profile:" + p.name())
                            << "Android.ndk.platform:android-21");
    params.useProfile = false;
    QCOMPARE(runQbs(params), 0);
    for (int i = 0; i < productNames.count(); ++i) {
        const QString productName = productNames.at(i);
        QVERIFY(m_qbsStdout.contains("Creating " + productName.toLocal8Bit() + ".apk"));
        const QString apkFilePath = relativeProductBuildDir(productName, p.name())
                + '/' + productName + ".apk";
        QVERIFY2(regularFileExists(apkFilePath), qPrintable(apkFilePath));
        const QString jarFilePath = findExecutable(QStringList("jar"));
        QVERIFY(!jarFilePath.isEmpty());
        QProcess jar;
        jar.start(jarFilePath, QStringList() << "-tf" << apkFilePath);
        QVERIFY2(jar.waitForStarted(), qPrintable(jar.errorString()));
        QVERIFY2(jar.waitForFinished(), qPrintable(jar.errorString()));
        QVERIFY2(jar.exitCode() == 0, qPrintable(jar.readAllStandardError().constData()));
        QCOMPARE(jar.readAllStandardOutput().trimmed().split('\n').count(), apkFileCounts.at(i));
    }
}

void TestBlackboxJava::android_data()
{
    QTest::addColumn<QString>("projectDir");
    QTest::addColumn<QStringList>("productNames");
    QTest::addColumn<QList<int>>("apkFileCounts");
    QTest::newRow("teapot") << "teapot" << QStringList("com.sample.teapot") << (QList<int>() << 31);
    QTest::newRow("no native") << "no-native"
            << QStringList("com.example.android.basicmediadecoder") << (QList<int>() << 22);
    QTest::newRow("multiple libs") << "multiple-libs-per-apk" << QStringList("twolibs")
                                   << (QList<int>() << 10);
    QTest::newRow("multiple apks") << "multiple-apks-per-project"
                                   << (QStringList() << "twolibs1" << "twolibs2")
                                   << (QList<int>() << 15 << 9);
}

static QProcessEnvironment processEnvironmentWithCurrentDirectoryInLibraryPath()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(HostOsInfo::libraryPathEnvironmentVariable(),
               (QStringList() << env.value(HostOsInfo::libraryPathEnvironmentVariable()) << ".")
               .join(HostOsInfo::pathListSeparator()));
    return env;
}

void TestBlackboxJava::java()
{
#if defined(Q_OS_WIN32) && !defined(Q_OS_WIN64)
    QSKIP("QTBUG-3845");
#endif

    Settings settings((QString()));
    Profile p(profileName(), &settings);

    int status;
    const auto jdkTools = findJdkTools(&status);
    QCOMPARE(status, 0);

    QDir::setCurrent(testDataDir + "/java");

    status = runQbs();
    if (p.value("java.jdkPath").toString().isEmpty()
            && status != 0 && m_qbsStderr.contains("jdkPath")) {
        QSKIP("java.jdkPath not set and automatic detection failed");
    }

    QCOMPARE(status, 0);

    const QStringList classFiles =
            QStringList() << "Jet" << "Ship" << "Vehicles";
    QStringList classFiles1 = QStringList(classFiles) << "io/qt/qbs/HelloWorld" << "NoPackage";
    for (int i = 0; i < classFiles1.count(); ++i) {
        QString &classFile = classFiles1[i];
        classFile = relativeProductBuildDir("class_collection") + "/classes/"
                + classFile + ".class";
        QVERIFY2(regularFileExists(classFile), qPrintable(classFile));
    }

    foreach (const QString &classFile, classFiles) {
        const QString filePath = relativeProductBuildDir("jar_file") + "/classes/" + classFile
                + ".class";
        QVERIFY2(regularFileExists(filePath), qPrintable(filePath));
    }
    const QString jarFilePath = relativeProductBuildDir("jar_file") + '/' + "jar_file.jar";
    QVERIFY2(regularFileExists(jarFilePath), qPrintable(jarFilePath));

    // Now check whether we correctly predicted the class file output paths.
    QCOMPARE(runQbs(QbsRunParameters("clean")), 0);
    foreach (const QString &classFile, classFiles1) {
        QVERIFY2(!regularFileExists(classFile), qPrintable(classFile));
    }

    // This tests various things: java.manifestClassPath, JNI, etc.
    QDir::setCurrent(relativeBuildDir() + "/install-root");
    QProcess process;
    process.setProcessEnvironment(processEnvironmentWithCurrentDirectoryInLibraryPath());
    process.start(HostOsInfo::appendExecutableSuffix(jdkTools["java"]),
            QStringList() << "-jar" << "jar_file.jar");
    if (process.waitForStarted()) {
        QVERIFY2(process.waitForFinished(), qPrintable(process.errorString()));
        QVERIFY2(process.exitCode() == 0, process.readAllStandardError().constData());
        const QByteArray stdOut = process.readAllStandardOutput();
        QVERIFY2(stdOut.contains("Driving!"), stdOut.constData());
        QVERIFY2(stdOut.contains("Flying!"), stdOut.constData());
        QVERIFY2(stdOut.contains("Flying (this is a space ship)!"), stdOut.constData());
        QVERIFY2(stdOut.contains("Sailing!"), stdOut.constData());
        QVERIFY2(stdOut.contains("Native code performing complex internal combustion process ("),
                 stdOut.constData());
    }

    process.start("unzip", QStringList() << "-p" << "jar_file.jar");
    if (process.waitForStarted()) {
        QVERIFY2(process.waitForFinished(), qPrintable(process.errorString()));
        const QByteArray stdOut = process.readAllStandardOutput();
        QVERIFY2(stdOut.contains("Class-Path: car_jar.jar random_stuff.jar"), stdOut.constData());
        QVERIFY2(stdOut.contains("Main-Class: Vehicles"), stdOut.constData());
    }
}

static QString dpkgArch(const QString &prefix = QString())
{
    QProcess dpkg;
    dpkg.start("/usr/bin/dpkg", QStringList() << "--print-architecture");
    dpkg.waitForFinished();
    if (dpkg.exitStatus() == QProcess::NormalExit && dpkg.exitCode() == 0)
        return prefix + QString::fromLocal8Bit(dpkg.readAllStandardOutput().trimmed());
    return QString();
}

void TestBlackboxJava::javaDependencyTracking()
{
    Settings settings((QString()));
    Profile p(profileName(), &settings);

    auto getSpecificJdkVersion = [](const QString &jdkVersion) -> QString {
        if (HostOsInfo::isMacosHost()) {
            QProcess java_home;
            java_home.start("/usr/libexec/java_home", QStringList() << "--version" << jdkVersion);
            java_home.waitForFinished();
            if (java_home.exitStatus() == QProcess::NormalExit && java_home.exitCode() == 0)
                return QString::fromLocal8Bit(java_home.readAllStandardOutput().trimmed());
        } else if (HostOsInfo::isWindowsHost()) {
            QSettings settings("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\"
                               + jdkVersion, QSettings::NativeFormat);
            return settings.value("JavaHome").toString();
        } else {
            QString minorVersion = jdkVersion;
            if (minorVersion.startsWith("1."))
                minorVersion.remove(0, 2);

            const QStringList searchPaths = {
                "/usr/lib/jvm/java-" + minorVersion + "-openjdk" + dpkgArch("-"), // Debian
                "/usr/lib/jvm/java-" + minorVersion + "-openjdk", // Arch
                "/usr/lib/jvm/jre-1." + minorVersion + ".0-openjdk", // Fedora
            };
            for (const QString &searchPath : searchPaths) {
                if (QFile::exists(searchPath + "/bin/javac"))
                    return searchPath;
            }
        }

        return QString();
    };

    auto runQbsTest = [&](const QString &jdkPath, const QString &javaVersion,
            const QString &arg) {
        QDir::setCurrent(testDataDir + "/java");
        QbsRunParameters rp;
        rp.arguments.append(arg);
        if (!jdkPath.isEmpty())
            rp.arguments << ("java.jdkPath:" + jdkPath);
        if (!javaVersion.isEmpty())
            rp.arguments << ("java.languageVersion:'" + javaVersion + "'");
        rmDirR(relativeBuildDir());
        QCOMPARE(runQbs(rp), 0);
    };

    static const auto knownJdkVersions = QStringList() << "1.6" << "1.7" << "1.8" << "1.9"
                                                       << QString(); // default JDK;
    QStringList seenJdkVersions;
    for (const auto &jdkVersion : knownJdkVersions) {
        QString specificJdkPath = getSpecificJdkVersion(jdkVersion);
        if (jdkVersion.isEmpty() || !specificJdkPath.isEmpty()) {
            const auto jdkPath = jdkVersion.isEmpty() ? jdkVersion : specificJdkPath;

            if (!jdkVersion.isEmpty())
                seenJdkVersions << jdkVersion;

            if (!seenJdkVersions.isEmpty()) {
                const auto javaVersions = QStringList()
                    << knownJdkVersions.mid(0, knownJdkVersions.indexOf(seenJdkVersions.last()) + 1)
                    << QString(); // also test with no explicitly specified source version

                for (const auto &currentJavaVersion : javaVersions) {
                    runQbsTest(jdkPath, currentJavaVersion, "--check-outputs");
                    runQbsTest(jdkPath, currentJavaVersion, "--dry-run");
                }
            }
        }
    }

    if (seenJdkVersions.isEmpty())
        QSKIP("No JDKs installed");
}

QTEST_MAIN(TestBlackboxJava)
