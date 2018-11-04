void PInit(void)
{
	P.AbsoluteDistance = 0;
	P.ATSPower = false;
	P.BrakeBell = ATS_SOUND_STOP;
	P.ChimeButton = false;
	P.EmrBrake = false;
	P.EmrDrive = false;
	P.SignalDistance = 0;
	P.SignalBrake = false;
	P.Pattern15 = 0;
	P.PatternSpeed = 0;
	P.LimitDistance = 0;
	P.LimitSpeed = 0;
	P.NextSignal = 0;
	P.ATSChime = 0;
	P.ATSChimeX = 0;
	P.PChimeX = 0;
	P.SignalStop[0] = true;
	for (P.PChime = 1; P.PChime < 256; P.PChime++)
	{
		P.SignalStop[P.PChime] = false;
	}
	P.PChime = 0;
	if (g_ini.DATA.WarningPattern != 0)
		P.WarningPattern = true;
	else
		P.WarningPattern = false;
}
void PCancel(void)
{
	if(P.SignalStop[P.Signal] && P.AbsoluteDistance == 0)
	{
		P.EmrDrive = !P.EmrDrive;
		P.CancelSound = ATS_SOUND_PLAY;
		P.EmrdriveSound = ATS_SOUND_PLAY;
	}
	if(P.SignalDistance != 0)
	P.CancelSound = ATS_SOUND_PLAY;
	P.SignalDistance = 0;
	P.Pattern15 = false;
}
void PRun(double deltaL)
{
	P.ATSSound = ATS_SOUND_CONTINUE;
	P.PatternStart = ATS_SOUND_CONTINUE;
	P.PatternEnd = ATS_SOUND_CONTINUE;
	P.BrakeSound = ATS_SOUND_CONTINUE;
	P.WarningSound = ATS_SOUND_CONTINUE;
	P.CancelSound = ATS_SOUND_CONTINUE;
	P.EmrdriveSound = ATS_SOUND_CONTINUE;
	P.ATSChimeL = ATS_SOUND_CONTINUE;
	P.ATSChimeR = ATS_SOUND_CONTINUE;
	P.BrakeBellEnd = ATS_SOUND_CONTINUE;
	P.PChimeL = ATS_SOUND_CONTINUE;
	P.PChimeR = ATS_SOUND_CONTINUE;
	if (P.SignalDistance > 0)
		P.SignalDistance -= deltaL;
	else
		P.SignalDistance = 0;
	if (P.LimitDistance > 100)
		P.LimitDistance -= deltaL;
	if (P.AbsoluteDistance > 0)
	{
		P.AbsoluteDistance -= deltaL;
		P.EmrDriveCut = true;
	}
	else
	{
		P.AbsoluteDistance = 0;
		P.EmrDriveCut = false;
	}

	P.BrakePattern = false;
	P.PatternSpeed = g_ini.DATA.PMaxspeed;
	P.PatternSignal = PPattern(P.SignalDistance - g_ini.DATA.Margin, P.SignalSpeed[P.NextSignal], g_ini.DATA.Pdecelerate);
	P.LimitPattern = PPattern(P.LimitDistance, P.LimitSpeed, g_ini.DATA.Pdecelerate);
	if (P.PatternSpeed > P.PatternSignal && P.PatternSignal >= 0 && (!P.WarningPattern || P.NextSignal == 0))
	{
		P.PatternSpeed = P.PatternSignal;
		P.BrakePattern = true;
	}
	if (P.PatternSpeed > P.LimitPattern && P.LimitPattern >= 0)
	{
		P.PatternSpeed = P.LimitPattern;
		P.BrakePattern = true;
	}
	if (P.PatternSpeed < 10 && P.PatternSpeed >= 0)
	{
		P.PatternSpeed = 10;
	}
	if (P.Pattern15)
	{
		P.PatternSpeed = 15;
	}
	if ((P.PatternSpeed > P.SignalSpeed[P.Signal] || P.PatternSpeed < 0) && P.SignalSpeed[P.Signal] > 0)
	{
		P.PatternSpeed = P.SignalSpeed[P.Signal];
	}
	if (P.SignalStop[P.Signal])
	{
		P.PatternSpeed = 0;
	}
	if (P.EmrDrive)
		P.PatternSpeed = 15;
	if ((P.SignalStop[P.Signal] && !P.EmrDrive) || P.PatternSpeed < speed)
	{
		if (P.SignalStop[P.Signal] && !P.EmrDrive)
		{
			if (!P.EmrBrake)
			{
				P.BrakeSound = ATS_SOUND_PLAY;
				P.BrakeBell = ATS_SOUND_PLAYLOOPING;
			}
			P.EmrBrake = true;
		}
		else if (P.EmrDrive)
		{
			if (!P.EmrBrake)
			{
				P.BrakeSound = ATS_SOUND_PLAY;
				P.BrakeBell = ATS_SOUND_PLAYLOOPING;
			}
			P.EmrBrake = true;
		}
		else if (P.SignalStop[P.NextSignal] && speed < 25)
		{
			if (!P.EmrBrake)
			{
				P.BrakeSound = ATS_SOUND_PLAY;
				P.BrakeBell = ATS_SOUND_PLAYLOOPING;
			}
			P.EmrBrake = true;
		}
		else if(g_ini.DATA.PEnabled == 1)
		{
			if (!P.SignalBrake)
			{
				P.ATSSound = ATS_SOUND_PLAY;
				P.BrakeSound = ATS_SOUND_PLAY;
			}
			P.SignalBrake = true;
		}
		else if (g_ini.DATA.PEnabled == 2)
		{
			if (!P.EmrBrake)
			{
				P.BrakeSound = ATS_SOUND_PLAY;
				P.BrakeBell = ATS_SOUND_PLAYLOOPING;
			}
			P.EmrBrake = true;
		}
	}
	if (P.SignalDistance != 0 && (P.SignalSpeed[P.NextSignal] < P.SignalSpeed[P.Signal] || P.SignalStop[P.NextSignal]) || P.LimitDistance != 0)
	{
		if (!P.Pattern)
			P.PatternStart = ATS_SOUND_PLAY;
		P.Pattern = true;
	}
	else
	{
		if (P.Pattern)
			P.PatternEnd = ATS_SOUND_PLAY;
		P.Pattern = false;
	}
	if(!P.SignalStop[P.Signal] || P.AbsoluteDistance != 0)
	{
		if (P.EmrDrive)
			P.EmrdriveSound = ATS_SOUND_PLAY;
		P.EmrDrive = false;
	}
	if (0 == speed && (!P.SignalStop[P.Signal] || P.EmrDrive))
	{
		if (P.EmrBrake)
		{
			P.BrakeBell = ATS_SOUND_STOP;
			P.ChimeButton = true;
			P.BrakeBellEnd = ATS_SOUND_PLAY;
		}
		if(P.SignalStop[P.NextSignal] && !P.EmrDrive && P.EmrBrake)
			P.Pattern15 = true;
		P.EmrBrake = false;
	}
	if (P.ChimeButton)
		ATSChime();
	if (P.SignalBrake)
		PChime(3000);
	if (P.SignalDistance < 75)
		P.Pattern15 = false;
	if (!P.BrakePattern && P.PatternSpeed > speed && g_ini.DATA.PMaxspeed > speed)
	{
		P.SignalBrake = false;
	}
	else if (P.SignalSpeed[P.NextSignal] > speed && (P.LimitSpeed > speed || P.LimitDistance == 0) && g_ini.DATA.PMaxspeed > speed || 15 > speed)
	{
		P.SignalBrake = false;
	}
	if (P.PatternSpeed > P.PatternSignal && P.PatternSignal >= 0 && P.WarningPattern && P.NextSignal != 0 && time % 1000 < 500)
	{
		P.PatternSpeed = P.PatternSignal;
	}

	if (PWarning(P.SignalDistance, speed, P.SignalSpeed[P.NextSignal], P.SignalSpeed[P.NextSignal], g_ini.DATA.Pdecelerate, g_ini.DATA.Margin) && P.NextSignal < P.Signal
		|| PWarning(P.LimitDistance, speed, P.LimitSpeed, P.LimitSpeed - 5, g_ini.DATA.Pdecelerate, 0)
		|| P.PatternSpeed - 5 < speed)
	{
		if (!P.Warning)
			P.WarningSound = ATS_SOUND_PLAY;
		P.Warning = true;
		PChime(8000);
	}
	else
	{
		if (P.Warning)
			P.WarningSound = ATS_SOUND_PLAY;
		P.Warning = false;
	}
}
double PPattern(double distance, int speed, double decelerate)
{
	double PatternSpeed = -1;

	if (distance > 100)
		PatternSpeed = sqrt((speed * speed) + (7.2 * decelerate * (distance - 100)));
	else if(distance > 0)
		PatternSpeed = speed;

	return PatternSpeed;
}
bool PWarning(double distance, double speed, int Limit, int ReleaseSpeed, double decelerate, int Margin)
{
	bool Warning;
	double Pattern = (speed * speed - Limit * Limit) / 7.2 / decelerate + Margin;

	if ((distance - 100) - speed * 50 / 36 < Pattern && speed > ReleaseSpeed && distance != 0)
		Warning = true;
	else
		Warning = false;
	return Warning;
}
void PBeacon(int Type, int Signal, double distance, int Optional)
{
	/*if (Type == 3 && Optional == 1 && speed != 0)
	{
		if (speed != 0)
		{
			P.NextSignal = Signal;
			P.SignalDistance = distance + 100 - g_ini.DATA.Margin;
		}
	}*/
	if (Type == 27)
	{
		if (Optional == 0)
		{
			P.ATSPower = false;
			PInit();
		}
		else if (Optional == 1)
		{
			if (speed != 0)
			{
				P.NextSignal = Signal;
				P.SignalDistance = distance + 100;
			}
		}
		else if (Optional == 2)
		{
			if (speed != 0)
			{
				P.AbsoluteDistance = distance + 5;
			}
		}
		else if (Optional > 1000 && Optional < 256000)
		{
			P.SignalSpeed[Optional / 1000] = Optional % 1000;
			if (Optional % 1000 == 0)
				P.SignalStop[Optional / 1000] = true;
			else
				P.SignalStop[Optional / 1000] = false;
			P.ATSPower = true;
		}
		else if (Optional == -1)
		{
			P.LimitDistance = 0;
			P.LimitSpeed = 0;
		}
		else if (Optional < 0)
		{
			if (speed != 0)
			{
				P.LimitDistance = Optional / -1000 + 100;
				P.LimitSpeed = -1 * (Optional % 1000);
			}
		}
	}
}
void ATSChime(void)
{
	if(time - P.ATSChime > 500)
	P.ATSChime = time;
	if(time - P.ATSChime < 250)
	{
		if (P.ATSChimeX != 1)
			P.ATSChimeL = ATS_SOUND_PLAY;
		P.ATSChimeX = 1;
	}
	else
	{
		if (P.ATSChimeX != 0)
			P.ATSChimeR = ATS_SOUND_PLAY;
		P.ATSChimeX = 0;
	}
}
void PChime(int Looptime)
{
	if (time - P.PChime > Looptime)
		P.PChime = time;
	if (time - P.PChime < Looptime / 2)
	{
		if (P.PChimeX != 1)
			P.PChimeL = ATS_SOUND_PLAY;
		P.PChimeX = 1;
	}
	else
	{
		if (P.PChimeX != 0)
			P.PChimeR = ATS_SOUND_PLAY;
		P.PChimeX = 0;
	}
}