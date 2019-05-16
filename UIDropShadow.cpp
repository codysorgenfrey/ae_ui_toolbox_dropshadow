#include "UIDropShadow.h"

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name),
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

    out_data->out_flags =  PF_OutFlag_I_EXPAND_BUFFER;
	out_data->out_flags2 = PF_OutFlag2_REVEALS_ZERO_ALPHA;
	
	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err err = PF_Err_NONE;
	PF_ParamDef	def; // used in the macros

	//Blur
	PF_ADD_FLOAT_SLIDERX(STR(StrID_Blur_Param_Name),
		DROPSHADOW_BLUR_MIN,
		DROPSHADOW_BLUR_MAX,
		DROPSHADOW_BLUR_MIN,
		DROPSHADOW_BLUR_MAX,
		DROPSHADOW_BLUR_DFLT,
		PF_Precision_HUNDREDTHS,
		0,
		0,
		BLUR_DISK_ID);

	//Offset x
	PF_ADD_FLOAT_SLIDERX(STR(StrID_Offsetx_Param_Name),
		DROPSHADOW_OFFSET_MIN,
		DROPSHADOW_OFFSET_MAX,
		DROPSHADOW_OFFSET_MIN,
		DROPSHADOW_OFFSET_MAX,
		DROPSHADOW_OFFSET_DFLT,
		PF_Precision_HUNDREDTHS,
		0,
		0,
		OFFSETX_DISK_ID);

	//Offset y
	PF_ADD_FLOAT_SLIDERX(STR(StrID_Offsety_Param_Name),
		DROPSHADOW_OFFSET_MIN,
		DROPSHADOW_OFFSET_MAX,
		DROPSHADOW_OFFSET_MIN,
		DROPSHADOW_OFFSET_MAX,
		DROPSHADOW_OFFSET_DFLT,
		PF_Precision_HUNDREDTHS,
		0,
		0,
		OFFSETY_DISK_ID);

	//Color
	PF_ADD_COLOR(STR(StrID_Color_Param_Name),
		DROPSHADOW_COLOR_R_DFLT,
		DROPSHADOW_COLOR_G_DFLT, 
		DROPSHADOW_COLOR_B_DFLT, 
		COLOR_DISK_ID);

	//Opacity
	PF_ADD_PERCENT(STR(StrID_Opacity_Param_Name),
		DROPSHADOW_OPACITY_DFLT,
		OPACITY_DISK_ID);

	//blend mode
	PF_ADD_POPUP(STR(StrID_Blend_Param_Name), 
		BLEND_LUMINOSITY,
		BLEND_NORMAL,
		(A_char*)STR(StrID_Blend_Options),
		BLEND_DISK_ID);
	
	out_data->num_params = DROPSHADOW_NUM_PARAMS;

	return err;
}

static PF_Err
KnockOut (
    void        *refcon,
    A_long        xL,
    A_long        yL,
    PF_Pixel8    *inP,
    PF_Pixel8    *outP)
{
    PF_Err err = PF_Err_NONE;
	PixelInfo *pi = reinterpret_cast<PixelInfo*>(refcon);

	outP = &pi->color;
	if (inP->alpha > 0)
		outP->alpha = 0;
    return err;
}

static PF_Err 
Render (
    PF_InData		*in_data,
    PF_OutData		*out_data,
    PF_ParamDef		*params[],
    PF_LayerDef		*output )
{
    AEGP_SuiteHandler suites(in_data->pica_basicP);
    PF_Err err = PF_Err_NONE;
    //PF_LayerDef *input = &params[DROPSHADOW_INPUT]->u.ld;
	
	//Check out myself for input to get original layer alpha for stackability
	AEGP_RenderOptionsH renderOptionsH;

	AEGP_FrameReceiptH frameRecieptH;
	ERR(suites.RenderSuite4()->AEGP_RenderAndCheckoutFrame(renderOptionsH,
		NULL,
		NULL,
		&frameRecieptH));

    // Make new empty AEGP_WorldH
    AEGP_WorldH newWorld;
    ERR(suites.WorldSuite3()->AEGP_New(NULL, AEGP_WorldType_8, input->width, input->height, &newWorld));
        
    //Copy PF_EffectWorld data into new AEGP_World
    A_u_long rowbytes = 0;
    void *data = 0;
    ERR(suites.WorldSuite3()->AEGP_GetRowBytes(newWorld, &rowbytes));
    ERR(suites.WorldSuite3()->AEGP_GetBaseAddr8(newWorld, (PF_Pixel8**) &data));
        
    auto dst = (char*) data;
    auto src = (char*) input->data;
    for(int y=0; y<input->height; y++){
        memcpy(dst, src, rowbytes);
        src += input->rowbytes;
        dst += rowbytes;
    }
        
    //Blur the AEGP_World
    ERR(suites.WorldSuite3()->AEGP_FastBlur((A_FpLong)params[DROPSHADOW_BLUR]->u.fs_d.value,
                                            PF_MF_Alpha_STRAIGHT,
                                            PF_Quality_HI,
                                            newWorld));
        
    //Convert the AEGP_World data into a PF_EffectWorld
    PF_EffectWorld tmpEffectWorld;
    ERR(suites.WorldSuite3()->AEGP_FillOutPFEffectWorld(newWorld, &tmpEffectWorld));
        
    //Knock out input's alpha in the effectworld and color pixels
	PixelInfo pi;
	AEFX_CLR_STRUCT(pi);
	pi.color = params[DROPSHADOW_COLOR]->u.cd.value;

    ERR(suites.Iterate8Suite1()->iterate(in_data,
                                            0, // progress base
                                            input->height, // progress final
                                            input, // src
                                            NULL, // area - null for all pixels
                                            (void*)&pi, // refcon - your custom data pointer
                                            KnockOut, // pixel function pointer
                                            &tmpEffectWorld));
        
    //Copy the tmpEffectWorld to the output LayerDef
    ERR(PF_COPY(&tmpEffectWorld, output, NULL, NULL));
        
    //Clean up worldH
    ERR(suites.WorldSuite3()->AEGP_Dispose(newWorld));
	
	return err;
}


extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		"UI Drop Shadow", // Name
		"UIToolbox DropShadow", // Match Name
		"UI Toolbox", // Category
		AE_RESERVED_INFO); // Reserved Info

	return result;
}


PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}