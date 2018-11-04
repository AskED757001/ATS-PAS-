#include "ATSIni.h"
#include "ats.h"
#include "dllmain.hpp"
#include "ATS-P.h"
#include "EB.h"

/// このATSプラグインの、コンピュータ上の絶対パス
char g_module_dir[MAX_PATH];

/// DLLのメイン関数
BOOL WINAPI DllMain(
					HINSTANCE hinstDLL,  ///< DLL モジュールのハンドル
					DWORD fdwReason,     ///< 関数を呼び出す理由
					LPVOID lpvReserved   ///< 予約済み
					)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
        {
            char fullpath[MAX_PATH];
            char drive[MAX_PATH],
                    dir[MAX_PATH];
			char *posIni;

            GetModuleFileNameA(hinstDLL, fullpath, MAX_PATH);
            _splitpath_s(fullpath, drive, MAX_PATH, dir, MAX_PATH, 0, 0, 0, 0);

            strcpy(g_module_dir, drive);
            strcat(g_module_dir, dir);

			//パスから.dllの位置を検索
			posIni = strstr(fullpath, ".dll");

			// .dllを.iniに置換
			memmove(posIni, ".ini", 4);

			// INIファイルをロードして結果を取得
			if (!g_ini.load(fullpath))
				g_ini.save(fullpath);
	}

        break;

	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:	
        break;
	}

	return true;
}

/// このプラグインがBVEによって読み込まれた時に呼び出される。
void WINAPI atsLoad()
{
}

/// このプラグインがBVEから解放された時に呼び出される。
void WINAPI atsDispose()
{
}

/// BVEがこのATSプラグインのバージョン値を取得しようとした時に呼び出される。
/// \return ATSプラグインのバージョン値
int WINAPI atsGetPluginVersion()
{
	return ATS_VERSION;
}

/// BVEに列車が読み込まれた時に呼び出される。
/// \param[in] vspec 車両の諸元
void WINAPI atsSetVehicleSpec(ATS_VEHICLESPEC vspec)
{
	emgBrake = vspec.BrakeNotches + 1;
	ATSNotch = vspec.AtsNotch;
}

/// BVEのシナリオが開始された時に呼び出される。
/// \param[in] param ブレーキハンドルの位置
void WINAPI atsInitialize(int param)
{
	speed = 0;
	if (g_ini.DATA.PEnabled != 0)
	PInit();
	P.MaxSpeed = g_ini.DATA.PMaxspeed;
	if (g_ini.DATA.EBEnabled != 0)
		EBInit();
}
/// BVEがシナリオ実行中に毎フレームごとに呼び出される。
/// \param[in] vs 毎処理ごとの車両の状態
/// \param[out] p_panel 運転台へ送る値の配列 (配列の範囲: 0-255)
/// \param[out] p_sound サウンド命令の値の配列 (配列の範囲: 0-255)
/// \return 列車の操縦命令
ATS_HANDLES WINAPI atsElapse(ATS_VEHICLESTATE vs, int *p_panel, int *p_sound)
{
	speed = fabs(vs.Speed);
	deltaT = vs.Time - time;
	double deltaL = vs.Speed * deltaT / 3600;
	time = vs.Time;
	ATS_HANDLES ret;
	if (P.EmrBrake || EB.EBBrake)
		ret.Brake = emgBrake;
	else if (P.SignalBrake && BrakeNotch < emgBrake)
		ret.Brake = emgBrake - 1;
	else
		ret.Brake = BrakeNotch;

	if (g_ini.DATA.DIR != 0 && !pilotlamp)
		ret.Power = 0;
	else
		ret.Power = PowerNotch;

	ret.Reverser = Reverser;
	if (g_ini.DATA.PEnabled != 0 && P.ATSPower)
	{
		PRun(deltaL);
	}

	if (g_ini.DATA.EBEnabled != 0)
	{
		if (BrakeNotch < emgBrake && !EB.EBPower)
			EBStart();

		RunEB(deltaT,speed);
	}
	
	if (g_ini.DATA.EBEnabled != 0)
	{
		p_panel[10] = EB.EBLamp;
		p_sound[3] = EB.EBSound;
	}
	if (g_ini.DATA.PEnabled != 0)
	{
		p_panel[72] = P.ATSPower;
		p_panel[73] = P.Pattern;
		p_panel[74] = P.Warning;
		p_panel[75] = P.SignalBrake;
		p_panel[76] = P.EmrBrake;
		if (speed < g_ini.DATA.BarFullGauge && P.ATSPower)
			p_panel[77] = speed * g_ini.DATA.BarFullNumber / g_ini.DATA.BarFullGauge;
		else if(P.ATSPower)
			p_panel[77] = g_ini.DATA.BarFullNumber;
		else
			p_panel[77] = 0;

		if (P.PatternSpeed < g_ini.DATA.BarFullGauge && P.ATSPower)
			p_panel[78] = P.PatternSpeed * g_ini.DATA.BarFullNumber / g_ini.DATA.BarFullGauge;
		else
			p_panel[78] = g_ini.DATA.BarFullNumber;
		p_panel[79] = P.EmrDrive;
		p_panel[80] = P.EmrDriveCut;
		p_sound[30] = P.PatternStart;
		p_sound[31] = P.WarningSound;
		p_sound[32] = P.PatternEnd;
		p_sound[33] = P.BrakeSound;
		p_sound[34] = P.ATSSound;
		p_sound[35] = P.BrakeBell;
		p_sound[36] = P.CancelSound;
		p_sound[37] = P.EmrdriveSound;
		p_sound[38] = P.ATSChimeL;
		p_sound[39] = P.ATSChimeR;
		p_sound[41] = P.PChimeL;
		p_sound[42] = P.PChimeR;
		p_sound[43] = P.BrakeBellEnd;
	}

	return ret;
}

/// プレイヤーによって力行ノッチ位置が変更された時に呼び出される。
/// \param[in] notch 変更後の力行ノッチ位置
void WINAPI atsSetPower(int notch)
{
	if (g_ini.DATA.EBEnabled != 0)
		EBReset();
	PowerNotch = notch;
}

/// プレイヤーによってブレーキノッチ位置が変更された時に呼び出される。
/// \param[in] notch 変更後のブレーキノッチ位置
void WINAPI atsSetBrake(int notch)
{
	if (g_ini.DATA.EBEnabled != 0)
	{
		if (notch == emgBrake)
			EBStart();
		EBReset();
	}
	BrakeNotch = notch;
}

/// プレイヤーによってレバーサーの位置が変更された時に呼び出される。
/// \param[in] pos 変更後のレバーサーの位置
void WINAPI atsSetReverser(int pos)
{
	if (g_ini.DATA.EBEnabled != 0)
		EBReset();
	Reverser = pos;
}

/// プレイヤーによってATSプラグインで使用するキーが押された時に呼び出される。
/// \param[in] ats_key_code ATSプラグインで使用するキーのインデックス
void WINAPI atsKeyDown(int ats_key_code)
{
	if (ats_key_code == ATS_KEY_S)
	{
		if (g_ini.DATA.PEnabled != 0)
			PCancel();
	}
	if (ats_key_code == ATS_KEY_A1)
	{
		if (g_ini.DATA.PEnabled != 0)
			P.ChimeButton = false;
	}
	if (ats_key_code == ATS_KEY_A2)
	{
		if (g_ini.DATA.EBEnabled != 0)
			EBReset();
	}
}

/// プレイヤーによってATSプラグインで使用するキーが押されていて、それが離された時に呼び出される。
/// \param[in] ats_key_code ATSプラグインで使用するキーのインデックス
void WINAPI atsKeyUp(int ats_key_code)
{
}

/// プレイヤーによって警笛が取り扱われた時に呼び出される。
/// \param[in] ats_horn 警笛の種類
void WINAPI atsHornBlow(int ats_horn)
{
	if (g_ini.DATA.EBEnabled != 0)
		EBReset();
}

/// BVEによって列車のドアが開かれた時に呼び出される。
void WINAPI atsDoorOpen()
{
	pilotlamp = false;
}

/// BVEによって列車のドアが閉じられた時に呼び出される。
void WINAPI atsDoorClose()
{
	pilotlamp = true;
}

/// BVEによって現在の信号現示が変更された時に呼び出される。
/// \param[in] signal 信号現示のインデックス
void WINAPI atsSetSignal(int signal)
{
	if (signal < 256)
		P.Signal = signal;
}

/// BVEによって地上子を通過した際に呼び出される。
/// \param[in] beacon_data 地上子の情報
void WINAPI atsSetBeaconData(ATS_BEACONDATA beacon_data)
{
	if (g_ini.DATA.PEnabled != 0)
		PBeacon(beacon_data.Type, beacon_data.Signal, beacon_data.Distance, beacon_data.Optional);
}