/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          http://www.mrpt.org/                          |
   |                                                                        |
   | Copyright (c) 2005-2017, Individual contributors, see AUTHORS file     |
   | See: http://www.mrpt.org/Authors - All rights reserved.                |
   | Released under BSD License. See details in http://www.mrpt.org/License |
   +------------------------------------------------------------------------+ */

#include "obs-precomp.h"  // Precompiled headers

#include <mrpt/serialization/CArchive.h>
#include <mrpt/config/CLoadableOptions.h>
#include <mrpt/config/CConfigFileBase.h>
#include <mrpt/maps/metric_map_types.h>

using namespace mrpt::obs;
using namespace mrpt::maps;

IMPLEMENTS_SERIALIZABLE(TMapGenericParams, CSerializable, mrpt::maps)

void TMapGenericParams::loadFromConfigFile(
	const mrpt::config::CConfigFileBase& source, const std::string& sct)
{
	MRPT_LOAD_CONFIG_VAR(enableSaveAs3DObject, bool, source, sct);
	MRPT_LOAD_CONFIG_VAR(enableObservationLikelihood, bool, source, sct);
	MRPT_LOAD_CONFIG_VAR(enableObservationInsertion, bool, source, sct);
}
void TMapGenericParams::dumpToTextStream(std::ostream& out) const
{
	// Common:
	LOADABLEOPTS_DUMP_VAR(enableSaveAs3DObject, bool);
	LOADABLEOPTS_DUMP_VAR(enableObservationLikelihood, bool);
	LOADABLEOPTS_DUMP_VAR(enableObservationInsertion, bool);
}

uint8_t TMapGenericParams::serializeGetVersion() const { return 0; }
void TMapGenericParams::serializeTo(mrpt::serialization::CArchive& out) const
{
	out << enableSaveAs3DObject << enableObservationLikelihood
		<< enableObservationInsertion;
}
void TMapGenericParams::serializeFrom(
	mrpt::serialization::CArchive& in, uint8_t version)
{
	switch (version)
	{
		case 0:
		{
			in >> enableSaveAs3DObject >> enableObservationLikelihood >>
				enableObservationInsertion;
		}
		break;
		default:
			MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version)
	};
}
