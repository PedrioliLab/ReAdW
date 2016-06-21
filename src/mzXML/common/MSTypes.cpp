// -*- mode: c++ -*-


/*
	File: MSTypes.cpp
	Description: Shared enum types for instrument and scan information
	Date: July 30, 2007

	Copyright (C) 2007 Natalie Tasman, ISB Seattle


	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "MSTypes.h"

using namespace std;

std::string toString(MSManufacturerType manufacturer) {
	string str;
	switch (manufacturer) {
		case THERMO:
			str = "Thermo";
			break;
		case THERMO_SCIENTIFIC:
			str = "Thermo Scientific";
			break;
		case THERMO_FINNIGAN:
			str = "Thermo Finnigan";
			break;
		case WATERS:
			str = "Waters";
			break;
		case ABI_SCIEX:
			str = "ABI / SCIEX";
			break;
		case AGILENT:
			str = "Agilent";
			break;
		case MANUFACTURER_UNDEF:
		default:
			str = "unknown";
			break;
	}
	return str;
}



std::string toString(MSAcquisitionSoftwareType acquisitionSoftware) {
	string str;
	switch (acquisitionSoftware) {
		case XCALIBUR:
			str = "Xcalibur";
			break;
		case MASSLYNX:
			str = "MassLynx";
			break;
		case ANALYST:
			str = "Analyst";
			break;
		case ANALYSTQS:
			str = "AnalystQS";
			break;
		case MASSHUNTER:
			str = "MassHunter";
			break;
		case ACQUISITIONSOFTWARE_UNDEF:
		default:
			str = "unknown";
			break;
	}
	return str;
}


std::string toString(MSAnalyzerType analyzer) {
	string str;
	switch (analyzer) {
		case ITMS:
			str = "ITMS";
			break;
		case TQMS:
			str = "TQMS";
			break;
		case SQMS:
			str = "SQMS";
			break;
		case TOFMS:
			str = "TOFMS";
			break;
		case FTMS:
			str = "FTMS";
			break;
		case SECTOR:
			str = "SECTOR";
			break;
		case QTOF:
			str = "QTOF";
			break;
		case TANDEM_QUAD:
			str = "TANDEM_QUAD";
		case ANALYZER_UNDEF:
		default:
			str = "unknown";
			break;
	}
	return str;
}


MSAnalyzerType MSAnalyzerTypeFromString(const std::string &analyzer) {

	if (analyzer == "TIMS") return ITMS;
	if (analyzer == "TQMS") return TQMS;
	if (analyzer == "SQMS") return SQMS;
	if (analyzer == "TOFMS") return TOFMS;
	if (analyzer == "FTMS") return FTMS;
	if (analyzer == "SECTOR") return SECTOR;
	if (analyzer == "QTOF") return QTOF;
	if (analyzer == "TANDEM_QUAD") return TANDEM_QUAD;

	return ANALYZER_UNDEF;
}


std::string toString(MSDetectorType detector) {
	string str;
	switch (detector) {
		case DETECTOR_UNDEF:
		default:
			str = "unknown";
			break;
	}
	return str;
}


MSDetectorType MSDetectorTypeFromString(const std::string &detector) {
	return DETECTOR_UNDEF;
}

std::string toString(MSIonizationType ionization) {
	string str;
	switch (ionization) {
		case EI:
			str = "EI";
			break;
		case CI:
			str = "CI";
			break;
		case FAB:
			str = "FAB";
			break;
		case ESI:
			str = "ESI";
			break;
		case APCI:
			str = "APCI";
			break;
		case NSI:
			str = "NSI";
			break;
		case TSP:
			str = "TSP";
			break;
		case FD:
			str = "FD";
			break;
		case MALDI:
			str = "MALDI";
			break;
		case GD:
			str = "GD";
			break;
		case MS_CHIP:
			str = "MS_CHIP";
			break;
		case IONIZATION_UNDEF:
		default:
			str = "unknown";
			break;
	}
	return str;
}



MSIonizationType MSIonizationTypeFromString(const std::string &ionization) {
	if (ionization=="EI") return EI;
	if (ionization=="CI") return CI;
	if (ionization=="FAB") return FAB;
	if (ionization=="ESI") return ESI;
	if (ionization=="APCI") return APCI;
	if (ionization=="NSI") return NSI;
	if (ionization=="TSP") return TSP;
	if (ionization=="FD") return FD;
	if (ionization=="MALDI") return MALDI;
	if (ionization=="GD") return GD;
	if (ionization=="MS_CHIP") return MS_CHIP;

	return IONIZATION_UNDEF;
}

std::string toString(MSScanType scanType) {
	string str;
	switch (scanType) {
		case FULL:
			str = "Full";
			break;
		case SIM:
			str = "SIM";
			break;
		case SRM:
			str = "SRM";
			break;
		case CRM:
			str = "CRM";
			break;
		case Z:
			str = "Z";
			break;
		case Q1MS:
			str = "Q1MS";
			break;
		case Q3MS:
			str = "Q3MS";
			break;

		case Q1Scan:
			str = "Q1 Scan";
			break;
		case Q1MI:
			str = "Q1 MI";
			break;
		case Q3Scan:
			str = "Q3 Scan";
			break;
		case Q3MI:
			str = "Q3 MI";
			break;
		case MRM:
			str = "MRM";
			break;
		case PrecursorScan:
			str = "Precursor Scan";
			break;
		case ProductIonScan:
			str = "Product IonS can";
			break;
		case NeutralLossScan:
			str = "Neutral Loss Scan";
			break;
		case TOFMS1:
			str = "TOF MS1";
			break;
		case TOFMS2:
			str = "TOF MS2";
			break;
		case TOFPrecursorIonScan:
			str = "TOF Precursor Ion Scan";
			break;
		case EPI: // Enhanced Product Ion
			str = "EPI";
			break;
		case ER: // Enhanced Resolution
			str = "ER";
			break;
		case MS3:
			str = "MS3";
			break;
		case TDF: // Time Delayed Fragmentation
			str = "TDF";
			break;
		case EMS: // Enhanced MS
			str = "EMS";
			break;
		case EMC: // Enhanced Multi-Charge
			str = "EMC";
			break;


		case HighResolution:
			str = "HighResolution";
			break;
		case MultipleReaction:
			str = "MultipleReaction";
			break;
		case NeutralGain:
			str = "NeutralGain";
			break;
		case NeutralLoss:
			str = "NeutralLoss";
			break;
		case PrecursorIon:
			str = "PrecursorIon";
			break;
		case MS1SurveyScan:
			str = "MS1SurveyScan";
			break;
		case ProductIon:
			str = "ProductIon";
			break;
		case SelectedIon:
			str = "SelectedIon";
			break;
		case TotalIon:
			str = "TotalIon";
			break;

		case CALIBRATION_SCAN:
			str = "calibration";
			break;


		case SCAN_UNDEF:
		default:
			str = "unknown";
			break;
	}
	return str;
}


std::string toString(MSActivationType activation) {
	string str;
	switch (activation) {
		case CID:
			str = "CID";
			break;
		case MPD:
			str = "MPD";
			break;
		case ECD:
			str = "ECD";
			break;
		case PQD:
			str = "PQD";
			break;
		case ETD:
			str = "ETD";
			break;
		case ETD_SA:
			str = "ETD+SA";
			break;
		case HCD:
			str = "HCD";
			break;
		case SA:
			str = "SA";
			break;
		case PTR:
			str = "PTR";
			break;
		case ACTIVATION_UNDEF:
		default:
			str = "unknown";
			break;
	}
	return str;
}



MSActivationType MSActivationTypeFromString(const std::string &activation) {

	if (activation == "CID") return CID;
	if (activation == "MPD") return MPD;
	if (activation == "ECD") return ECD;
	if (activation == "PQD") return PQD;
	if (activation == "ETD") return ETD;
	if (activation == "HCD") return HCD;
	if (activation == "SA") return SA;
	if (activation == "PTR") return PTR;

	return ACTIVATION_UNDEF;
}


std::string toString(ScanCoordinateType scanCoordinateType) {
	string str;
	switch (scanCoordinateType) {

		case MASSLYNX_COORDINATE_FUNCTION:
			str = "function";
			break;
		case MASSLYNX_COORDINATE_PROCESS:
			str = "process";
			break;
		case MASSLYNX_COORDINATE_SCAN:
			str = "scan";
			break;
		case MASSLYNX_COORDINATE_TRANSITION:
			str = "transition";
			break;

		case ABI_COORDINATE_SAMPLE:
			str = "sample";
			break;
		case ABI_COORDINATE_PERIOD:
			str = "period";
			break;
		case ABI_COORDINATE_EXPERIMENT:
			str = "experiment";
			break;
		case ABI_COORDINATE_CYCLE:
			str = "cycle";
			break;

		case MHDAC_COORDINATE_NATIVESCANNUM:
			str = "scan";
			break;

		case SCAN_COORDINATE_UNDEF:
		default:
			str = "unknown";
			break;
	}
	return str;
}

