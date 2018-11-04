#pragma once
void PInit(void);
void PCancel(void);
void PRun(double);
void PBeacon(int, int,double, int);
double PPattern(double, int, double);
bool PWarning(double, double, int, int, double, int);
void EBInit(void);
void EBStart(void);
void EBReset(void);
void RunEB(int, double);
void ATSChime(void);
void PChime(int);


int BrakeNotch;
int PowerNotch;
int Reverser;
int time;
int deltaT;
int emgBrake;
double speed;
int ATSNotch;
bool pilotlamp;
double Sndistance;
int TyreSize;

ATSIni g_ini;

class ATSP
{
public:
	double SignalDistance;
	int Signal;
	int NextSignal;
	int SignalSpeed[256] = {};
	bool SignalStop[256] = {};
	double PatternSpeed;
	bool EmrBrake;
	bool SignalBrake;
	bool BrakePattern;
	bool ATSPower;
	bool Pattern15;
	bool EmrDrive;
	bool Pattern;
	bool Warning;
	double LimitDistance;
	double LimitSpeed;
	double LimitPattern;
	int ATSSound;
	int PatternStart;
	int PatternEnd;
	int BrakeSound;
	int BrakeBell;
	int BrakeBellEnd;
	int WarningSound;
	int CancelSound;
	int EmrdriveSound;
	bool EmrDriveCut;
	double AbsoluteDistance;
	bool ChimeButton;
	int ATSChime;//1000
	int ATSChimeL;
	int ATSChimeR;
	int ATSChimeX;
	int PChime;//3000,8000
	int PChimeL;
	int PChimeR;
	int PChimeX;
	int MaxSpeed;
	double PatternSignal;
	bool WarningPattern;
};
ATSP P;
class ATSEB
{
public:
	bool EBPower;
	bool EBLamp;
	int EBTimer;
	bool EBBrake;
	int EBSound;
};
ATSEB EB;