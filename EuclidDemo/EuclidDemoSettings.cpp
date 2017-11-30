#pragma once

#include "resource.h"
#include <ColorUtil.h>
#include <afx.h>
#include "EuclidDemo.h"
#include "palette.h"
//#include <commctrl.h>

using std::vector;


/* from https://labs.tineye.com/color/ color extractor:
BBGGRR color, fraction, color name
*/

// Jackson Pollock's Shimmer
weighted_palette_entry<COLORREF> g_jp_shimmer_palette[] = {
	{ 0xb2d7e5,  0.682, "Givry (Brown)" },
	{ 0x7cd2e9,  0.154, "Wild Rice (Green)" },
	{ 0x82969c,  0.087, "Lemon Grass (Grey)" },
	{ 0x495658,  0.023, "Millbrook (Grey)" },
	{ 0x5f84be,  0.020, "Twine (Brown)" },
	{ 0xf3fcfc,  0.009, "Floral White (White)" },
	{ 0x3463bc,  0.007, "Smoke Tree (Orange)" },
	{ 0x546c88,  0.007, "Cement (Brown)" },
	{ 0x469bda,  0.007, "Fire Bush (Yellow)" },
	{ 0x659299,  0.006, "Avocado (Green)" }
};

// Autumn Rhythm (#30)
weighted_palette_entry<COLORREF> g_jp_autumnrhythm_palette[] = {
	{ 0xE0E8F5,  1.000, "Background" },
	{ 0x9dbcd3,  0.523, "Soft Amber (Brown)" },
	{ 0x678194,  0.242, "Squirrel (Brown)" },
	{ 0x28343d,  0.138, "Jacko Bean (Brown)" },
	{ 0x425563,  0.097, "Judge Grey (Brown)" }
};

// Burning Landscape
weighted_palette_entry<COLORREF> g_jp_burninglandscape_palette[] = {
	{ 0xFFFFFF,  1.000, "Background" },
	{ 0x8d98ab,  0.409, "Thatch (Brown)" },
	{ 0x3b5aa9,  0.134, "Orange Roughy (Orange)" },
	{ 0x5b4c3c,  0.112, "Cello (Blue)" },
	{ 0x4d9aa3,  0.097, "Husk (Brown)" },
	{ 0x1f4fcd,  0.086, "Trinidad (Brown)" },
	{ 0x2eb4cb,  0.071, "Old Gold (Yellow)" },
	{ 0x508bb9,  0.034, "Tussock (Brown)" },
	{ 0x2587c5,  0.024, "Geebung (Yellow)" },
	{ 0x4d6442,  0.017, "Stromboli (Green)" },
	{ 0x4e8567,  0.016, "Glade Green (Green)" }
};

// Untitled (1942-1944)
weighted_palette_entry<COLORREF> g_jp_untitled1942_palette[] = {
	{ 0xFFFFFF,  1.000, "Background" },
	{ 0x86aec4,  0.405, "Ecru (Brown)" },
	{ 0x1b1823,  0.117, "Nero (Black)" },
	{ 0x121d94,  0.114, "Mandarian Orange (Orange)" },
	{ 0x565e47,  0.093, "Viridian Green (Green)" },
	{ 0x284358,  0.072, "Bracken (Brown)" },
	{ 0x21aad9,  0.061, "Galliano (Yellow)" },
	{ 0x28377c,  0.056, "Lusty (Red)" },
	{ 0x3081b0,  0.052, "Mandalay (Yellow)" },
	{ 0x286361,  0.022, "Costa Del Sol (Green)" },
	{ 0x7d5a38,  0.008, "Matisse (Blue)" }
};

// Mondrian - Broadway Boogie-woogie
weighted_palette_entry<COLORREF> g_mondrian_broadway_palette[] = {
	{ 0xf0f3f2,  0.497, "Background" },
	{ 0x29c6f4,  0.170, "Golden Dream (Yellow)" },
	{ 0x0816d3,  0.084, "Venetian Red (Red)" },
	{ 0xc9eff3,  0.073, "Spring Sun (Green)" },
	{ 0x99e1f1,  0.061, "Vis Vis (Yellow)" },
	{ 0x60c5e4,  0.049, "Golden Sand (Yellow)" },
	{ 0x52332f,  0.024, "Lucky Point (Blue)" },
	{ 0x3d6674,  0.017, "Yellow Metal (Brown)" },
	{ 0x935123,  0.016, "Endeavour (Blue)" },
	{ 0x34373e,  0.009, "Kilamanjaro (Grey)" }
};

/*forgot
	{ 0xdadbdf,  0.551, "Porcelain (Grey)" },
	{ 0x1ca4d7,  0.178, "Galliano (Yellow)" },
	{ 0x59b4d2,  0.070, "Tacha (Brown)" },
	{ 0x1123b3,  0.067, "Fire Brick (Red)" },
	{ 0x93c8d9,  0.060, "Tahuna Sands (Brown)" },
	{ 0x764d41,  0.021, "Astronaut (Blue)" },
	{ 0x4a51b2,  0.018, "Chestnut (Brown)" },
	{ 0x652020,  0.015, "Midnight Blue (Blue)" },
	{ 0xb5410c,  0.014, "Cobalt (Blue)" },
	{ 0x404653,  0.007, "Woody Brown (Brown)" }
};*/

// Mondrian - Red Tree
weighted_palette_entry<COLORREF> g_mondrian_redtree_palette[] = {
	{ 0x8c3f3a,  0.350, "Dark Slate Blue (Blue)" },
	{ 0xb37c5d,  0.268, "Chetwode Blue (Blue)" },
	{ 0xab9986,  0.130, "Bali Hai (Blue)" },
	{ 0x462649,  0.107, "Loulou (Violet)" },
	{ 0x514e90,  0.052, "Lotus (Brown)" },
	{ 0x36319d,  0.032, "Milano Red (Red)" },
	{ 0x262ed1,  0.025, "Persian Red (Red)" },
	{ 0x8aa0b1,  0.021, "Del Rio (Brown)" },
	{ 0x76afd3,  0.009, "Putty (Yellow)" },
	{ 0x4276e4,  0.006, "Jaffa (Orange)" }
};

// Van Gogh, garden of St Paul
weighted_palette_entry<COLORREF> g_vangogh_garden_st_paul[] = {
	{ 0x615758, 0.528, "Chicago (Grey)" },
	{ 0x9c704c, 0.157, "Dark Tan (Brown)" },
	{ 0x4977ac, 0.076, "Steel Blue (Blue)" },
	{ 0x4a6653, 0.069, "Mineral Green (Green)" },
	{ 0xd1ab58, 0.056, "Apache (Brown)" },
	{ 0x8d3222, 0.041, "Burnt Umber (Brown)" },
	{ 0x7498aa, 0.029, "Neptune (Green)" },
	{ 0xb53b1d, 0.023, "Rust (Red)" },
	{ 0xded1a5, 0.010, "Sapling (Yellow)" },
	{ 0xcb6c2b, 0.010, "Gold Drop (Orange)" }
};

// Claude Monet - View of San Giorgio Maggiore, Venice by Twilight, 1908
weighted_palette_entry<COLORREF> g_monet_san_giorgio_maggiore[] = {
	{ 0xbf7f33, 0.284, "Geebung (Yellow)" },
	{ 0x866c36, 0.190, "McKenzie (Brown)" },
	{ 0xc85c1c, 0.146, "Aloy Orange (Orange)" },
	{ 0x30120e, 0.123, "Seal Brown (Black)" },
	{ 0x6aa6a8, 0.073, "Tradewind (Green)" },
	{ 0x723216, 0.060, "Peru Tan (Brown)" },
	{ 0x9bb58f, 0.047, "Norway (Green)" },
	{ 0x25508c, 0.036, "Endeavour (Blue)" },
	{ 0x37799f, 0.028, "Lochmara (Blue)" },
	{ 0x3a5052, 0.014, "Atomic (Blue)" }
};

// Monet
weighted_palette_entry<COLORREF> g_monet_detail[] = {
	{ 0xb48b91, 0.193, "Rosy Brown (Brown)" },
	{ 0xf3ae4e, 0.171, "Casablanca (Orange)" },
	{ 0xe1a780, 0.166, "Tumbleweed (Brown)" },
	{ 0x877187, 0.122, "Topaz (Violet)" },
	{ 0xedac6e, 0.121, "Harvest Gold (Yellow)" },
	{ 0xd2958c, 0.071, "Petite Orchid (Pink)" },
	{ 0xf9c33d, 0.054, "Saffron (Yellow)" },
	{ 0xad8670, 0.052, "Mongoose (Brown)" },
	{ 0x9374a0, 0.041, "Ce Soir (Violet)" },
	{ 0x69625f, 0.009, "Storm Dust (Grey)" }
};

//
// default settings
//
void EuclidDemoSettings::resetToDefaults(int which) {


	usePresetPalette(0);
}

void EuclidDemoSettings::randomize() {

	resetToDefaults(0);

	// select a color palette
	if(rand()%1) {
		makeRandomPalette(20);
	}
	else {
		usePresetPalette(-1);
	}

	
}

void EuclidDemoSettings::makeRandomPalette(int n) {

	vector<COLORREF> pal;
	pal.resize(n);


}

void EuclidDemoSettings::usePresetPalette(int index) {
	if(index < 0) {
		index = rand() % 10;
	}

	switch(index) {
	case 0:
	default:
		m_palette.create(g_jp_shimmer_palette, sizeof(g_jp_shimmer_palette) / sizeof(g_jp_shimmer_palette[0]));
		break;

	case 1:
		m_palette.create(g_jp_autumnrhythm_palette, sizeof(g_jp_autumnrhythm_palette) / sizeof(g_jp_autumnrhythm_palette[0]));
		break;

	case 2:
		m_palette.create(g_jp_burninglandscape_palette, sizeof(g_jp_burninglandscape_palette) / sizeof(g_jp_burninglandscape_palette[0]));
		break;

	case 3:
		m_palette.create(g_jp_untitled1942_palette, sizeof(g_jp_untitled1942_palette) / sizeof(g_jp_untitled1942_palette[0]));
		break;

	case 4:
		m_palette.create(g_mondrian_broadway_palette, sizeof(g_mondrian_broadway_palette) / sizeof(g_mondrian_broadway_palette[0]));
		break;

	case 5:
		m_palette.create(g_mondrian_redtree_palette, sizeof(g_mondrian_redtree_palette) / sizeof(g_mondrian_redtree_palette[0]));
		break;

	case 6:
	case 7:
		m_palette.create(g_vangogh_garden_st_paul, sizeof(g_vangogh_garden_st_paul) / sizeof(g_vangogh_garden_st_paul[0]));
		break;

	case 8:
		m_palette.create(g_monet_san_giorgio_maggiore, sizeof(g_monet_san_giorgio_maggiore) / sizeof(g_monet_san_giorgio_maggiore[0]));
		break;

	case 9:
		m_palette.create(g_monet_detail, sizeof(g_monet_detail) / sizeof(g_monet_detail[0]));
	}

	COLORREF bg = m_palette.colorAtIndex(0);
	m_palette.removeColor(0);
}

//	Settings Dialog

CSettingsDialog g_settingsDialog;



void CSettingsDialog::initSettingsDialog(HWND hdlg, const EuclidDemoSettings& ssSettings) {

	this->hdlg = hdlg;
	m_settings = ssSettings;

	SetScrollBarRange(hdlg, IDC_SPEED_SLIDER, m_settings.nSimulationSpeed, 1, 25, 1, 5);
	SetDlgItemFormatText(hdlg, IDC_SPEED_STATIC, "Simulation Speed: %d", m_settings.nSimulationSpeed);
	
	SetScrollBarRange(hdlg, IDC_RANDOMPALETTE_SLIDER, 100 * m_settings.m_fRandomPaletteProbability, 0, 100, 1, 10);
	SetDlgItemFormatText(hdlg, IDC_RANDOMPALETTE_STATIC, "Random palette frequency: %1.2f",
		m_settings.m_fRandomPaletteProbability);

	SetScrollBarRange(hdlg, IDC_RANDOMIZE_SLIDER, m_settings.nRandomizeTimer, 5, 600, 5, 5);
	SetDlgItemFormatText(hdlg, IDC_RANDOMIZE_STATIC, "Randomize timer: %d s",
		m_settings.nRandomizeTimer);

	SetScrollBarRange(hdlg, IDC_CAMERASPEED_SLIDER, m_settings.nCameraSpeed, 1, 10);
	SetDlgItemFormatText(hdlg, IDC_CAMERASPEED_STATIC, "Camera speed: %d",
		m_settings.nCameraSpeed);

	CheckDlgButton(hdlg, IDC_VARIATIONS, 1);
	CheckDlgButton(hdlg, IDC_MULTISCREEN_MODE, m_settings.multiScreenMode);
}


void CSettingsDialog::readDialogSettings() {

	m_settings.nSimulationSpeed = SendDlgItemMessage(hdlg, IDC_SPEED_SLIDER, TBM_GETPOS, 0, 0);
	
	int variations = (IsDlgButtonChecked(hdlg, IDC_VARIATIONS) == BST_CHECKED);
	m_settings.nRandomizeTimer = SendDlgItemMessage(hdlg, IDC_RANDOMIZE_SLIDER, TBM_GETPOS, 0, 0);
	m_settings.m_fRandomPaletteProbability = 0.01f * SendDlgItemMessage(hdlg, IDC_RANDOMPALETTE_SLIDER, TBM_GETPOS, 0, 0);
	m_settings.multiScreenMode = (IsDlgButtonChecked(hdlg, IDC_MULTISCREEN_MODE) == BST_CHECKED);
	m_settings.nCameraSpeed = SendDlgItemMessage(hdlg, IDC_CAMERASPEED_SLIDER, TBM_GETPOS, 0, 0);
}

// about dialog
BOOL CALLBACK aboutDialogProc(HWND hdlg, UINT msg, WPARAM wpm, LPARAM lpm) {
	switch(msg) {

	case WM_COMMAND:
		switch(LOWORD(wpm)) {
		case IDOK:
		case IDCANCEL:
			if(::IsWindow(mainWindow)) {
				::DestroyWindow(mainWindow);
			}
			::EndDialog(hdlg, LOWORD(wpm));
			break;
		}
	}
	return FALSE;
}


BOOL screenSaverConfigureDialog(HWND hdlg, UINT msg, WPARAM wpm, LPARAM lpm) {
	return g_settingsDialog.handleMessage(hdlg, msg, wpm, lpm);
}

BOOL CSettingsDialog::handleMessage(HWND hdlg, UINT msg, WPARAM wpm, LPARAM lpm) {

	DWORD ival;
	HICON hAppIcon;
	
	switch(msg) {
	case WM_INITDIALOG:
		InitCommonControls();
		readRegistry(m_settings);
		initSettingsDialog(hdlg, m_settings);
		hAppIcon = ::LoadIcon(mainInstance, MAKEINTRESOURCE(ID_APP));
		::SendMessage(hdlg, WM_SETICON, ICON_SMALL, (LPARAM) hAppIcon);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wpm)) {
		case IDOK:
			readDialogSettings();
			writeRegistry(m_settings);
			// Fall through

		case IDCANCEL:
			if(::IsWindow(mainWindow)) {
				::DestroyWindow(mainWindow);
				mainWindow = NULL;
			}
			EndDialog(hdlg, LOWORD(wpm));
			break;

			//case DEFAULTS0:
			//	setDefaults(0);
			//	initSettingsDialog(hdlg);
			//	break;
			//case DEFAULTS1:
			//	setDefaults(1);
			//	initSettingsDialog(hdlg);
			//	break;
			//case DEFAULTS2:
			//	setDefaults(2);
			//	initSettingsDialog(hdlg);
			//	break;

		case IDC_PREVIEW:
			startWindowedSaver(hdlg);
			break;

		case IDC_ABOUT:
			return ::DialogBox(mainInstance, MAKEINTRESOURCE(IDD_ABOUTBOX),
				hdlg, (DLGPROC)aboutDialogProc);
			break;

		default:
			TRACE("WM_COMMAND: %u\n", LOWORD(wpm));
		}	// case WM_COMMAND
		return TRUE;

	case BM_CLICK:
		TRACE("BM_CLICK\n");
		break;

	case WM_HSCROLL:
		switch(::GetDlgCtrlID(HWND(lpm))) {
		case IDC_SPEED_SLIDER:
			ival = SendDlgItemMessage(hdlg, IDC_SPEED_SLIDER, TBM_GETPOS, 0, 0);
			SetDlgItemFormatText(hdlg, IDC_SPEED_STATIC, "Simulation Speed: %d", ival);
			m_settings.nSimulationSpeed = ival;
			break;

		case IDC_RANDOMPALETTE_SLIDER:
			m_settings.m_fRandomPaletteProbability
				= 0.01f * SendDlgItemMessage(hdlg, IDC_RANDOMPALETTE_SLIDER, TBM_GETPOS, 0, 0);
			SetDlgItemFormatText(hdlg, IDC_RANDOMPALETTE_STATIC, "Random palette frequency: %1.2f",
				m_settings.m_fRandomPaletteProbability);
			break;

		case IDC_RANDOMIZE_SLIDER:
			m_settings.nRandomizeTimer = SendDlgItemMessage(hdlg, IDC_RANDOMIZE_SLIDER, TBM_GETPOS, 0, 0);
			SetDlgItemFormatText(hdlg, IDC_RANDOMIZE_STATIC, "Randomize timer: %d s",
				m_settings.nRandomizeTimer);
			break;

		case IDC_CAMERASPEED_SLIDER:
			m_settings.nCameraSpeed = SendDlgItemMessage(hdlg, IDC_CAMERASPEED_SLIDER, TBM_GETPOS, 0, 0);
			SetDlgItemFormatText(hdlg, IDC_CAMERASPEED_STATIC, "Camera speed: %d",
				m_settings.nCameraSpeed);
			break;

			//case FRAMERATELIMIT:
			//	updateFrameRateLimitSlider(hdlg, FRAMERATELIMIT, FRAMERATELIMITTEXT);
			//	break;

		default:
			TRACE("SLIDER: %04x\n", ::GetDlgCtrlID(HWND(lpm)));
			return TRUE;
		}

	case 32:
	case 132:
	case 512:
	case 6: 
	case 28:
	case 309:
	case 307:
	case 312:
	case 160:
	case 127:
	case 674:
	case 134:
		// get these a lot...
		break;

	default:
		TRACE("Msg: %u 0x%04x\n", msg, msg);
	}

	return FALSE;
}

//
//	Registry settings
//

// Initialize all user-defined stuff
void readRegistry(EuclidDemoSettings &ssSettings) {
	LONG result;
	HKEY hKey;
	DWORD valtype, valsize, val, dValSize;
	float dVal;

	result = RegOpenKeyEx(HKEY_CURRENT_USER, registryPath, 0, KEY_READ, &hKey);
	if(result != ERROR_SUCCESS)
		return;

	valsize = sizeof(val);
	dValSize = sizeof(dVal);
	int ival;
	DWORD ivalSize = sizeof(ival);

	result = RegQueryValueEx(hKey, "SimulationSpeed", 0, &valtype, (LPBYTE)&ival, &ivalSize);
	if(result == ERROR_SUCCESS)
		ssSettings.nSimulationSpeed = ival;

	result = RegQueryValueEx(hKey, "RandomizeTimer", 0, &valtype, (LPBYTE)&ival, &ivalSize);
	if(result == ERROR_SUCCESS)
		ssSettings.nRandomizeTimer = ival;

	result = RegQueryValueEx(hKey, "CameraSpeed", 0, &valtype, (LPBYTE)&ival, &ivalSize);
	if(result == ERROR_SUCCESS)
		ssSettings.nCameraSpeed = ival;

	result = RegQueryValueEx(hKey, "MultiScreenMode", 0, &valtype, (LPBYTE)&ival, &ivalSize);
	if(result == ERROR_SUCCESS)
		ssSettings.multiScreenMode = ival;

	getRegistryFloat(hKey, "RandomPaletteFrequency", &ssSettings.m_fRandomPaletteProbability, 0.0f, 1.0f);

	result = RegQueryValueEx(hKey, "FrameRateLimit", 0, &valtype, (LPBYTE)&val, &valsize);
	if(result == ERROR_SUCCESS)
		dFrameRateLimit = val;

	RegCloseKey(hKey);
}


// Save all user-defined stuff
void writeRegistry(const EuclidDemoSettings& ssSettings) {
	LONG result;
	HKEY hKey;
	DWORD val, disp;

	result = RegCreateKeyEx(HKEY_CURRENT_USER, registryPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &disp);
	if(result != ERROR_SUCCESS)
		return;

	RegSetValueEx(hKey, "SimulationSpeed", 0, REG_DWORD, (CONST BYTE*)&ssSettings.nSimulationSpeed, sizeof(ssSettings.nSimulationSpeed));
	RegSetValueEx(hKey, "RandomizeTimer", 0, REG_DWORD, (CONST BYTE*)&ssSettings.nRandomizeTimer, sizeof(ssSettings.nRandomizeTimer));
	RegSetValueEx(hKey, "CameraSpeed", 0, REG_DWORD, (CONST BYTE*)&ssSettings.nCameraSpeed, sizeof(ssSettings.nCameraSpeed));
	RegSetValueEx(hKey, "MultiScreenMode", 0, REG_DWORD, (CONST BYTE*)&ssSettings.multiScreenMode, sizeof(ssSettings.multiScreenMode));

	setRegistryFloat(hKey, "RandomPaletteFrequency", ssSettings.m_fRandomPaletteProbability);

	val = dFrameRateLimit;
	RegSetValueEx(hKey, "FrameRateLimit", 0, REG_DWORD, (CONST BYTE*)&val, sizeof(val));

	RegCloseKey(hKey);
}

