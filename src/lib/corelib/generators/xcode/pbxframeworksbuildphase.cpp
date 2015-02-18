#include "pbxframeworksbuildphase.h"

PBXFrameworksBuildPhase::PBXFrameworksBuildPhase(PBXTarget *parent) :
    PBXBuildPhase(parent)
{
}

QString PBXFrameworksBuildPhase::name() const
{
    return QLatin1String("Frameworks");
}

QString PBXFrameworksBuildPhase::isa() const
{
    return QLatin1String("PBXFrameworksBuildPhase");
}

PBXObjectMap PBXFrameworksBuildPhase::toMap() const
{
    PBXObjectMap self = PBXBuildPhase::toMap();
    return self;
}

QString PBXFrameworksBuildPhase::comment() const
{
    return name();
}
