/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of the Qt Build Suite.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "applecodesignutils.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>

#include <QtCore/private/qcore_mac_p.h>
#include <CommonCrypto/CommonCrypto.h>
#include <Security/Security.h>

namespace qbs {
namespace Internal {

// Apple documentation states that the Sec* family of functions are not thread-safe on OS X
// https://developer.apple.com/library/mac/documentation/Security/Reference/certifkeytrustservices/
static QMutex securityMutex;

QByteArray smimeMessageContent(const QByteArray &data)
{
    QCFType<CMSDecoderRef> decoder = NULL;
    if (CMSDecoderCreate(&decoder) != noErr)
        return QByteArray();

    if (CMSDecoderUpdateMessage(decoder, data.constData(), data.size()) != noErr)
        return QByteArray();

    if (CMSDecoderFinalizeMessage(decoder) != noErr)
        return QByteArray();

    QCFType<CFDataRef> content = NULL;
    if (CMSDecoderCopyContent(decoder, &content) != noErr)
        return QByteArray();

    return QByteArray::fromCFData(content);
}

QString certificateCommonName(const QByteArray &data)
{
    QMutexLocker locker(&securityMutex);
    Q_UNUSED(locker);

    QCFType<SecCertificateRef> certificate = SecCertificateCreateWithData(kCFAllocatorDefault,
                                                                          data.toRawCFData());
    if (!certificate)
        return QString();

    QCFType<CFStringRef> commonName = NULL;
    if (SecCertificateCopyCommonName(certificate, &commonName) != errSecSuccess)
        return QString();

    return QString::fromCFString(commonName);
}

static QString oidString(const QByteArray &components)
{
    const auto getComponent = [](const QByteArray &data, int &pos) {
        unsigned long q = 0;
        do {
            q = (q * 128) + (static_cast<unsigned char>(data.at(pos)) & ~0x80);
        } while ((pos < data.size()) && (static_cast<unsigned char>(data.at(pos++)) & 0x80));
        return q;
    };

    if (components.isEmpty())
        return QString();

    int pos = 0;
    unsigned long oid1 = getComponent(components, pos);
    unsigned long q1 = std::min(oid1 / 40, 2ul);
    QString s = QString::asprintf("%lu.%lu", q1, oid1 - (q1 * 40));
    while (pos < components.size())
        s += QString::asprintf(".%lu", getComponent(components, pos));
    return s;
}

static QVariantMap identityProperties(SecIdentityRef identity)
{
    if (!identity)
        return QVariantMap();

    QCFType<SecCertificateRef> certificate = NULL;
    if (SecIdentityCopyCertificate(identity, &certificate) != errSecSuccess)
        return QVariantMap();

    const void *pkeys[] = {
        kSecOIDX509V1SubjectName,
        kSecOIDX509V1ValidityNotBefore,
        kSecOIDX509V1ValidityNotAfter,
        kSecOIDExtendedKeyUsage
    };
    QCFType<CFArrayRef> keys = CFArrayCreate(kCFAllocatorDefault,
                                             pkeys,
                                             sizeof(pkeys) / sizeof(pkeys[0]),
                                             &kCFTypeArrayCallBacks);

    QCFType<CFDictionaryRef> values = SecCertificateCopyValues(certificate, keys, NULL);
    if (!values)
        return QVariantMap();

    CFDictionaryRef extendedKeyUsage = (CFDictionaryRef)CFDictionaryGetValue(values,
                                                                        kSecOIDExtendedKeyUsage);
    CFArrayRef extendedKeyUsageValue = (CFArrayRef)CFDictionaryGetValue(extendedKeyUsage,
                                                                        CFSTR("value"));

    // Also potentially useful, but these are for signing pkgs which aren't used here
    // 1.2.840.113635.100.4.9 - 3rd Party Mac Developer Installer: <name>
    // 1.2.840.113635.100.4.13 - Developer ID Installer: <name>
    for (CFIndex i = 0; i < CFArrayGetCount(extendedKeyUsageValue); ++i) {
        CFDataRef extendedKeyUsageValueN = (CFDataRef)CFArrayGetValueAtIndex(extendedKeyUsageValue,
                                                                             i);
        const QString oid = oidString(QByteArray::fromRawCFData(extendedKeyUsageValueN));
        if (oid != QString::fromCFString(kSecOIDExtendedUseCodeSigning))
            return QVariantMap();
    }

    CFDictionaryRef subjectName = (CFDictionaryRef)CFDictionaryGetValue(values,
                                                                        kSecOIDX509V1SubjectName);
    CFArrayRef subjectNameValue = (CFArrayRef)CFDictionaryGetValue(subjectName,
                                                                   CFSTR("value"));

    const QVariantMap keyNames {
        {QStringLiteral("CN"), QString::fromCFString(kSecOIDCommonName)},
        {QStringLiteral("O"), QString::fromCFString(kSecOIDOrganizationName)},
        {QStringLiteral("OU"), QString::fromCFString(kSecOIDOrganizationalUnitName)}
    };

    // Pull CN, O, OU, etc., properties from the subject name dictionary
    QVariantMap props;
    QVariantMap snprops;
    for (CFIndex i = 0; i < CFArrayGetCount(subjectNameValue); ++i) {
        CFDictionaryRef dict = (CFDictionaryRef)CFArrayGetValueAtIndex(subjectNameValue, i);
        QMapIterator<QString, QVariant> it(keyNames);
        while (it.hasNext()) {
            it.next();
            if (QString::fromCFString((CFStringRef)CFDictionaryGetValue(dict, CFSTR("label"))) ==
                it.value()) {
                const QString value = QString::fromCFString((CFStringRef)CFDictionaryGetValue(dict,
                                                                                CFSTR("value")));
                if (value.isEmpty())
                    return QVariantMap();
                snprops[it.key()] = value;
            }
        }
    }

    if (snprops.isEmpty())
        return QVariantMap();
    props[QStringLiteral("subjectName")] = snprops;

    CFNumberRef number;
    long long numberll;

    number = (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)CFDictionaryGetValue(values,
                                               kSecOIDX509V1ValidityNotBefore),
                                               CFSTR("value"));
    if (number && CFNumberGetValue(number, kCFNumberLongLongType, &numberll)) {
        CFDateRef cfdate = CFDateCreate(kCFAllocatorDefault, numberll);
        const QDateTime qdatetime = QDateTime::fromCFDate(cfdate);
        if (QDateTime::currentDateTimeUtc() < qdatetime)
            return QVariantMap();
        props[QStringLiteral("validBefore")] = qdatetime;
    }

    number = (CFNumberRef)CFDictionaryGetValue((CFDictionaryRef)CFDictionaryGetValue(values,
                                               kSecOIDX509V1ValidityNotAfter),
                                               CFSTR("value"));
    if (number && CFNumberGetValue(number, kCFNumberLongLongType, &numberll)) {
        CFDateRef cfdate = CFDateCreate(kCFAllocatorDefault, numberll);
        const QDateTime qdatetime = QDateTime::fromCFDate(cfdate);
        if (QDateTime::currentDateTimeUtc() > qdatetime)
            return QVariantMap();
        props[QStringLiteral("validAfter")] = qdatetime;
    }

    QCFType<CFDataRef> certificateData = SecCertificateCopyData(certificate);
    props[QStringLiteral("SHA1")] = QCryptographicHash::hash(
                QByteArray::fromRawCFData(certificateData),
                QCryptographicHash::Sha1).toHex().toUpper();

    return props;
}

QVariantMap identitiesProperties()
{
    QMutexLocker locker(&securityMutex);
    Q_UNUSED(locker);

    const void *keys[] = {kSecClass, kSecMatchLimit, kSecAttrCanSign};
    const void *values[] = {kSecClassIdentity, kSecMatchLimitAll, kCFBooleanTrue};
    QCFType<CFDictionaryRef> query = CFDictionaryCreate(kCFAllocatorDefault,
                                                        keys,
                                                        values,
                                                        sizeof(keys) / sizeof(keys[0]),
                                                        &kCFTypeDictionaryKeyCallBacks,
                                                        &kCFTypeDictionaryValueCallBacks);
    QCFType<CFTypeRef> result = NULL;
    if (SecItemCopyMatching(query, &result) != errSecSuccess)
        return QVariantMap();

    QVariantMap items;
    const auto tryAppend = [&](QVariantMap props) {
        if (!props.isEmpty())
            items.insert(props[QStringLiteral("SHA1")].toString(), props);
    };

    if (CFGetTypeID(result) == SecIdentityGetTypeID()) {
        tryAppend(identityProperties((SecIdentityRef)result.operator const void *()));
    } else if (CFGetTypeID(result) == CFArrayGetTypeID()) {
        for (CFIndex i = 0; i < CFArrayGetCount((CFArrayRef)result.operator const void *()); ++i) {
            tryAppend(identityProperties((SecIdentityRef)CFArrayGetValueAtIndex(
                                                                    result.as<CFArrayRef>(), i)));
        }
    }

    return items;
}

} // namespace Internal
} // namespace qbs
