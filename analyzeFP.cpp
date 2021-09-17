#include "stdafx.h"
#include "analyzeFP.hpp"
#include "pugixml.hpp"
#include "pugixml.cpp"


extern "C" IMAGE_DOS_HEADER __ImageBase;

bool blink;
bool debugMode, initialSidLoad;

int disCount;

ifstream sidDatei;
char DllPathFile[_MAX_PATH];
string pfad;

vector<string> sidName;
vector<string> sidEven;
vector<int> sidMin;
vector<int> sidMax;
vector<string> AircraftIgnore;

using namespace std;
using namespace EuroScopePlugIn;
using namespace pugi;

	// Run on Plugin Initialization
InitialClimbPlugin::InitialClimbPlugin(void) :CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, MY_PLUGIN_NAME, MY_PLUGIN_VERSION, MY_PLUGIN_DEVELOPER, MY_PLUGIN_COPYRIGHT)
{
	string loadingMessage = "Version: ";
	loadingMessage += MY_PLUGIN_VERSION;
	loadingMessage += " loaded.";
	sendMessage(loadingMessage);

	// Register Tag Item "Initial climb"
	RegisterTagItemType("Initial Climb", TAG_ITEM_INITIALCLIMB);
	RegisterTagItemFunction("Add to CFL", TAG_FUNC_ADDTOCFL);

	// Get Path of the Sid.json
	GetModuleFileNameA(HINSTANCE(&__ImageBase), DllPathFile, sizeof(DllPathFile));
	pfad = DllPathFile;
	pfad.resize(pfad.size() - strlen("InitialClimbPlugin.dll"));
	pfad += "initialClimb.xml";

	debugMode = false;
	initialSidLoad = false;
}

// Run on Plugin destruction, Ie. Closing EuroScope or unloading plugin
InitialClimbPlugin::~InitialClimbPlugin()
{
}


/*
	Custom Functions
*/

void InitialClimbPlugin::debugMessage(string type, string message) {
	// Display Debug Message if debugMode = true
	if (debugMode) {
		DisplayUserMessage("InitialClimbPlugin", type.c_str(), message.c_str(), true, true, true, false, false);
	}
}

void InitialClimbPlugin::sendMessage(string type, string message) {
	// Show a message
	DisplayUserMessage("InitialClimbPlugin", type.c_str(), message.c_str(), true, true, true, true, false);
}

void InitialClimbPlugin::sendMessage(string message) {
	DisplayUserMessage("Message", "InitialClimbPlugin", message.c_str(), true, true, true, false, false);
}

//
void InitialClimbPlugin::OnFunctionCall(int FunctionId, const char* ItemString, POINT Pt, RECT Area) {
	CFlightPlan fp = FlightPlanSelectASEL();

	if (FunctionId == TAG_FUNC_ADDTOCFL) 
	{
		string fpType = fp.GetFlightPlanData().GetPlanType();
		if (fpType != "V") {
			string origin = fp.GetFlightPlanData().GetOrigin(); boost::to_upper(origin);
			string depRwy = fp.GetFlightPlanData().GetDepartureRwy(); boost::to_upper(depRwy);
			string sid = fp.GetFlightPlanData().GetSidName(); boost::to_upper(sid);

			string txt = getInitialClimbFromFile(origin, depRwy, sid);
			if (txt.length() > 0) {
				fp.GetControllerAssignedData().SetClearedAltitude(std::stoi(txt) * 100);
			}
		}
	}
	if (FunctionId == TAG_FUNC_ON_OFF) {
		if (find(AircraftIgnore.begin(), AircraftIgnore.end(), fp.GetCallsign()) != AircraftIgnore.end())
			AircraftIgnore.erase(remove(AircraftIgnore.begin(), AircraftIgnore.end(), fp.GetCallsign()), AircraftIgnore.end());
		else
			AircraftIgnore.emplace_back(fp.GetCallsign());

	}
}

// Get FlightPlan, and therefore get the first waypoint of the flightplan (ie. SID). Check if the (RFL/1000) corresponds to the SID Min FL and report output "OK" or "FPL"
void InitialClimbPlugin::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	string callsign = FlightPlan.GetCallsign();

	string origin = FlightPlan.GetFlightPlanData().GetOrigin(); boost::to_upper(origin);
	string destination = FlightPlan.GetFlightPlanData().GetDestination(); boost::to_upper(destination);

	string sid = FlightPlan.GetFlightPlanData().GetSidName(); boost::to_upper(sid);
	string depRwy = FlightPlan.GetFlightPlanData().GetDepartureRwy(); boost::to_upper(depRwy);

	string first_wp = sid.substr(0, sid.find_first_of("0123456789"));
	if (0 != first_wp.length())
		boost::to_upper(first_wp);
	string sid_suffix;
	if (first_wp.length() != sid.length()) {
		sid_suffix = sid.substr(sid.find_first_of("0123456789"), sid.length());
		boost::to_upper(sid_suffix);
	}
	//Get data from the xml function and if has value add it when checking the correct itemCode.
	string txt = getInitialClimbFromFile(origin, depRwy, sid);
	bool hasInitialClimbSet = false;
	const char* initialAlt = "";

	if (txt.length() > 0) 
	{
		initialAlt = txt.c_str();
		hasInitialClimbSet = true;
	}
	else {
		hasInitialClimbSet = false;
	}

	if (ItemCode == TAG_ITEM_INITIALCLIMB)
	{
		string FlightPlanString = FlightPlan.GetFlightPlanData().GetRoute();
		int RFL = FlightPlan.GetFlightPlanData().GetFinalAltitude();

		*pColorCode = TAG_COLOR_RGB_DEFINED;
		string fpType{ FlightPlan.GetFlightPlanData().GetPlanType() };
		if (fpType == "V") {
			*pRGB = TAG_GREEN;
			strcpy_s(sItemString, 16, "VFR");
		}
		else {
			if (hasInitialClimbSet)
				{
				if (FlightPlan.GetControllerAssignedData().GetClearedAltitude() == std::stoi(txt) * 100)
				{
					*pRGB = TAG_GREEN;
					strcpy_s(sItemString, 16, initialAlt);
				}
				else {
					*pRGB = TAG_RED;
					strcpy_s(sItemString, 16, initialAlt);
				}

				}
				else {
					*pRGB = TAG_GREY;
					strcpy_s(sItemString, 16, "-");
				}
		}
	}
}

//Get all data from the xml file
string InitialClimbPlugin::getInitialClimbFromFile(string origin, string depRwy, string sid)
{
	xml_document doc;

	// load the XML file
	doc.load_file(pfad.c_str());

	string xpath = "/initialClimb/apt[@icao='" + origin + "']/runway[@name='" + depRwy + "']/sid[@name='" + sid + "']/alt";
	pugi::xpath_node_set altPugi = doc.select_nodes(xpath.c_str());
	
	std::vector<std::string> result;
	for (auto xpath_node : altPugi) {
		if (xpath_node.attribute() != nullptr)
			result.push_back(xpath_node.attribute().value());
		else
			result.push_back(xpath_node.node().child_value());
	}

	if (result.size() > 0)
	{
		return result[0];
	}
	else {
		return "";
	}
}

//
void InitialClimbPlugin::OnFlightPlanDisconnect(CFlightPlan FlightPlan)
{
	AircraftIgnore.erase(remove(AircraftIgnore.begin(), AircraftIgnore.end(), FlightPlan.GetCallsign()), AircraftIgnore.end());	
}

void InitialClimbPlugin::OnTimer(int Counter) {

	blink = !blink;

	if (blink) {
		if (disCount < 3) {
			disCount++;
		}
		else {
			disCount = 0;
		}
	}

	// Loading proper Sids, when logged in
	if (GetConnectionType() != CONNECTION_TYPE_NO && !initialSidLoad) {
		string callsign{ ControllerMyself().GetCallsign() };
		initialSidLoad = true;
	}
	else if (GetConnectionType() == CONNECTION_TYPE_NO && initialSidLoad) {
		sidName.clear();
		sidEven.clear();
		sidMin.clear();
		sidMax.clear();
		initialSidLoad = false;
		sendMessage("Unloading... ");
	}
}