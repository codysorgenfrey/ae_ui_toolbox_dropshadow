#pragma once

#ifndef DROPSHADOW_H
#define DROPSHADOW_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#define PF_DEEP_COLOR_AWARE 1	// make sure we get 16bpc pixels; 
								// AE_Effect.h checks for this.

#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "UIDropShadow_Strings.h"

/* Versioning information */

#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1


/* Parameter defaults */

#define	DROPSHADOW_BLUR_MIN 0
#define	DROPSHADOW_BLUR_MAX 500
#define	DROPSHADOW_BLUR_DFLT 4
#define DROPSHADOW_OFFSET_MIN -500
#define DROPSHADOW_OFFSET_MAX 500
#define DROPSHADOW_OFFSET_DFLT 4
#define DROPSHADOW_COLOR_R_DFLT 0
#define	DROPSHADOW_COLOR_G_DFLT 0
#define	DROPSHADOW_COLOR_B_DFLT 0
#define DROPSHADOW_OPACITY_DFLT 25

enum {
	DROPSHADOW_INPUT = 0,
	DROPSHADOW_BLUR,
	DROPSHADOW_OFFSETX,
	DROPSHADOW_OFFSETY,
	DROPSHADOW_COLOR,
	DROPSHADOW_OPACITY,
	DROPSHADOW_BLEND,
	DROPSHADOW_NUM_PARAMS
};

enum {
	BLUR_DISK_ID = 1,
	OFFSETX_DISK_ID,
	OFFSETY_DISK_ID,
	COLOR_DISK_ID,
	OPACITY_DISK_ID,
	BLEND_DISK_ID
};

extern "C" {

	DllExport
	PF_Err
	EffectMain(
		PF_Cmd			cmd,
		PF_InData		*in_data,
		PF_OutData		*out_data,
		PF_ParamDef		*params[],
		PF_LayerDef		*output,
		void			*extra);

}

enum {
	BLEND_NORMAL = 1,
	PADDING1,
	BLEND_DARKEN,
	BLEND_MULTIPLY,
	BLEND_COLORBURN,
	PADDING2,
	BLEND_LIGHTEN,
	BLEND_SCREEN,
	BLEND_COLORDODGE,
	PADDING3,
	BLEND_OVERLAY,
	BLEND_SOFTLIGHT,
	BLEND_HARDLIGHT,
	PADDING4,
	BLEND_DIFFERENCE,
	BLEND_EXCLUSION,
	PADDING5,
	BLEND_HUE,
	BLEND_SATURATION,
	BLEND_COLOR,
	BLEND_LUMINOSITY
};

typedef struct PixelInfo{
	PF_Pixel color;
} PixelInfo, *PixelInfoP, **PixelInfoH;

#endif // DROPSHADOW_H
