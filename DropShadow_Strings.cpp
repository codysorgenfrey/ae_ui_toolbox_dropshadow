#include "DropShadow.h"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;



TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"UI Drop Shadow",
	StrID_Description,				"Creates a stackable drop shadow with no shadow underneath the layer.",
	StrID_Radius_Param_Name,		"Radius"
};


char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
	
