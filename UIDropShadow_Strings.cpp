#include "UIDropShadow.h"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;



TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"UI Drop Shadow",
	StrID_Description,				"Creates a stackable drop shadow with no shadow underneath the layer.",
    StrID_Comp_Param_Name,          "Composite Over Layer",
	StrID_Blur_Param_Name,			"Blur",
	StrID_Offsetx_Param_Name,		"X",
	StrID_Offsety_Param_Name,		"Y",
	StrID_Color_Param_Name,			"Color",
	StrID_Opacity_Param_Name,		"Opacity",
	StrID_Blend_Param_Name,			"Blending Mode",
	StrID_Blend_Options,			"Normal|(-|Darken|Multiply|Color Burn|(-|Lighten|Screen|Color Dodge|(-|Overlay|Soft Light|Hard Light|(-|Difference|Exclusion|(-|Hue|Saturation|Color|Luminosity"
};


char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
	
