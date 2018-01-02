/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          http://www.mrpt.org/                          |
   |                                                                        |
   | Copyright (c) 2005-2017, Individual contributors, see AUTHORS file     |
   | See: http://www.mrpt.org/Authors - All rights reserved.                |
   | Released under BSD License. See details in http://www.mrpt.org/License |
   +------------------------------------------------------------------------+ */

#include "rawlog-edit-declarations.h"

#include <mrpt/topography.h>
#include <mrpt/obs/CObservationGPS.h>
#include <mrpt/obs/CObservationGasSensors.h>
#include <mrpt/img/color_maps.h>
#include <mrpt/math/interp_fit.h>

using namespace mrpt;
using namespace mrpt::obs;
using namespace mrpt::system;
using namespace mrpt::rawlogtools;
using namespace mrpt::topography;
using namespace std;

// STL data must have global scope:
struct TGPSGASDataPoint
{
	double lon, lat, alt;  // degrees, degrees, meters
	uint8_t fix;  // 1: standalone, 2: DGPS, 4: RTK fix, 5: RTK float, ...
	float color;
};

struct TDataPerGPSGAS
{
	map<TTimeStamp, TGPSGASDataPoint> path;
};

// ======================================================================
//		op_export_gps_gas_kml
// ======================================================================
DECLARE_OP_FUNCTION(op_export_gps_gas_kml)
{
	// A class to do this operation:
	class CRawlogProcessor_ExportGPSGAS_KML
		: public CRawlogProcessorOnEachObservation
	{
	   protected:
		string m_inFile;

		map<string, TDataPerGPSGAS> m_gps_paths;  // sensorLabel -> data

		mrpt::obs::CObservationGPS* obs;
		mrpt::obs::CObservationGasSensors* obsGas;
		float gasConcentration;
		float maxGasValue, minGasValue;
		bool hasGAS, hasGPS;

	   public:
		CRawlogProcessor_ExportGPSGAS_KML(
			CFileGZInputStream& in_rawlog, TCLAP::CmdLine& cmdline,
			bool verbose)
			: CRawlogProcessorOnEachObservation(in_rawlog, cmdline, verbose)
		{
			getArgValue<string>(cmdline, "input", m_inFile);

			maxGasValue = -1.0;
			minGasValue = 10000;
			obs = nullptr;
			obsGas = nullptr;
			hasGAS = false;
			hasGPS = false;
		}

		// return false on any error.
		bool processOneObservation(CObservation::Ptr& o)
		{
			if (IS_CLASS(o, CObservationGPS))
			{
				obs = dynamic_cast<CObservationGPS*>(o.get());
				if (!obs->has_GGA_datum)
				{
					obs = nullptr;
					return true;  // Nothing to do...
				}
				hasGPS = true;
			}
			else if (IS_CLASS(o, CObservationGasSensors))
			{
				obsGas = dynamic_cast<CObservationGasSensors*>(o.get());
				if (obsGas->m_readings.size() < 1)
				{
					cout << "Empty Gas Sensor" << endl;
					obsGas = nullptr;
					return true;  // Nothing to do...
				}

				if (obsGas->m_readings[0].readingsVoltage.size() < 1)
				{
					cout << "Empty Gas Obs" << endl;
					obsGas = nullptr;
					return true;  // Nothing to do...
				}
				// cout << "new observations has: "<<
				// obsGas->m_readings[0].readingsVoltage.size() << " elements."
				// << endl;
				gasConcentration = obsGas->m_readings[0].readingsVoltage[0];
				hasGAS = true;
			}
			else
				return true;

			// Insert the new entries:
			if (hasGAS && hasGPS)
			{
				TDataPerGPSGAS& D = m_gps_paths[obs->sensorLabel];
				TGPSGASDataPoint& d = D.path[o->timestamp];
				const mrpt::obs::gnss::Message_NMEA_GGA& gga =
					obs->getMsgByClass<mrpt::obs::gnss::Message_NMEA_GGA>();
				d.lon = gga.fields.longitude_degrees;
				d.lat = gga.fields.latitude_degrees;
				d.alt = gga.fields.altitude_meters;
				d.fix = gga.fields.fix_quality;
				d.color = gasConcentration;

				if (d.color > maxGasValue) maxGasValue = d.color;
				if (d.color < minGasValue) minGasValue = d.color;

				hasGAS = false;
				hasGPS = false;
			}

			return true;  // All ok
		}

		void generate_KML()
		{
			const bool save_altitude = false;

			const string outfilname =
				mrpt::system::fileNameChangeExtension(m_inFile, "kml");
			VERBOSE_COUT << "Writing KML file: " << outfilname << endl;

			CFileOutputStream f(outfilname);

			// Header:
			f.printf(
				"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n"
				"<!-- File automatically generated by rawlog-edit \n"
				"      Part of the MRPT initiative - http://www.mrpt.org/ \n"
				"      Generated on %s from file '%s'  -->\n"
				"  <Document>\n"
				"    <name>GPS-GAS Paths</name>\n"
				"    <description>GPS-GAS paths from dataset "
				"'%s'</description>\n",
				mrpt::system::dateTimeLocalToString(mrpt::system::now())
					.c_str(),
				m_inFile.c_str(), m_inFile.c_str());

			// For each GPS sensor label:
			for (map<string, TDataPerGPSGAS>::const_iterator it =
					 m_gps_paths.begin();
				 it != m_gps_paths.end(); ++it)
			{
				const string& label = it->first;  // GPS Label
				const TDataPerGPSGAS& D = it->second;

				f.printf(
					"    <Folder>\n"
					"      <name>%s all points</name>\n"
					"      <description>%s: All received points (for all "
					"quality levels)</description>\n",
					label.c_str(), label.c_str());

				// For each GPS point
				int n = 0;
				float scale = 1;  // scale to dispay in KML
				int alpha = 120;  //[0 255]
				for (map<TTimeStamp, TGPSGASDataPoint>::const_iterator
						 itP = D.path.begin();
					 itP != D.path.end(); ++itP, n++)
				{
					const TGPSGASDataPoint& d = itP->second;

					// Decode gas concentration [0,1] to color RGBA
					float r, g, b;
					jet2rgb(
						(d.color - minGasValue) / (maxGasValue - minGasValue),
						r, g, b);

					f.printf(
						"        <Placemark>\n"
						"          <description> %s point: %i </description>\n"
						"          <Style>\n"
						"            <IconStyle>\n"
						"              <color> %02X%02X%02X%02X </color>\n"
						"              <scale> %.2f </scale>\n"
						"              "
						"<Icon><href>http://maps.google.com/mapfiles/kml/"
						"shapes/shaded_dot.png</href></Icon>\n"
						"            </IconStyle>\n"
						"          </Style>\n"
						"          <Point>\n"
						"            <coordinates>",
						label.c_str(), n, alpha, (int)floor(b * 255),
						(int)floor(g * 255), (int)floor(r * 255), scale);

					if (save_altitude)
						f.printf(" %.15f,%.15f,%.3f ", d.lon, d.lat, d.alt);
					else
						f.printf(" %.15f,%.15f ", d.lon, d.lat);

					f.printf(
						"</coordinates>\n"
						"          </Point>\n"
						"        </Placemark>\n");
				}

				// end part:
				f.printf("    </Folder>\n");

			}  // end for each sensor label

			f.printf(
				"  </Document>\n"
				"</kml>\n");
		}  // end generate_KML

	};  // end CRawlogProcessor_ExportGPS_KML

	// Process
	// ---------------------------------
	CRawlogProcessor_ExportGPSGAS_KML proc(in_rawlog, cmdline, verbose);
	proc.doProcessRawlog();

	// Now that the entire rawlog is parsed, do the actual output:
	proc.generate_KML();

	// Dump statistics:
	// ---------------------------------
	VERBOSE_COUT << "Time to process file (sec)        : " << proc.m_timToParse
				 << "\n";
}
