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
    
    
    //Composite over layer
    AEFX_CLR_STRUCT(def);
    PF_ADD_LAYER(STR(StrID_Comp_Param_Name),
                 PF_LayerDefault_MYSELF,
                 COMP_DISK_ID);
    
	//Blur
    AEFX_CLR_STRUCT(def);
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
    AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(STR(StrID_Offsetx_Param_Name),
		DROPSHADOW_OFFSET_MIN,
		DROPSHADOW_OFFSET_MAX,
		DROPSHADOW_OFFSET_MIN,
		DROPSHADOW_OFFSET_MAX,
		DROPSHADOW_OFFSETX_DFLT,
		PF_Precision_HUNDREDTHS,
		0,
		0,
		OFFSETX_DISK_ID);

	//Offset y
    AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDERX(STR(StrID_Offsety_Param_Name),
		DROPSHADOW_OFFSET_MIN,
		DROPSHADOW_OFFSET_MAX,
		DROPSHADOW_OFFSET_MIN,
		DROPSHADOW_OFFSET_MAX,
		DROPSHADOW_OFFSETY_DFLT,
		PF_Precision_HUNDREDTHS,
		0,
		0,
		OFFSETY_DISK_ID);

	//Color
    AEFX_CLR_STRUCT(def);
	PF_ADD_COLOR(STR(StrID_Color_Param_Name),
		DROPSHADOW_COLOR_R_DFLT,
		DROPSHADOW_COLOR_G_DFLT, 
		DROPSHADOW_COLOR_B_DFLT, 
		COLOR_DISK_ID);

	//Opacity
    AEFX_CLR_STRUCT(def);
	PF_ADD_PERCENT(STR(StrID_Opacity_Param_Name),
		DROPSHADOW_OPACITY_DFLT,
		OPACITY_DISK_ID);

	//blend mode
    AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP(STR(StrID_Blend_Param_Name), 
		BLEND_LUMINOSITY,
		BLEND_NORMAL,
		(A_char*)STR(StrID_Blend_Options),
		BLEND_DISK_ID);
	
	out_data->num_params = DROPSHADOW_NUM_PARAMS;

	return err;
}

static PF_Err
ColorPixels (
          void        *refcon,
          A_long        xL,
          A_long        yL,
          PF_Pixel8    *inP,
          PF_Pixel8    *outP)
{
    PF_Err err = PF_Err_NONE;
    PixelInfo *pi = reinterpret_cast<PixelInfo*>(refcon);
    
    //Set pixel color but not alpha for all pixels
    outP->red = pi->color.red;
    outP->green = pi->color.green;
    outP->blue = pi->color.blue;
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

    //Copy pixel if original image had alpha "on"
	if (inP->alpha > 0)
		outP->alpha = PF_MAX_CHAN8 - inP->alpha; //take the remainder of the alpha for anti-aliasing
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
	PF_LayerDef *sourceInput = &params[DROPSHADOW_INPUT]->u.ld;

	//Check out shape of layer
    PF_ParamDef checkoutShapeParam;
    AEFX_CLR_STRUCT(checkoutShapeParam);
    ERR(PF_CHECKOUT_PARAM(in_data,
                          DROPSHADOW_COMP_LAYER,
                          in_data->current_time,
                          in_data->time_step,
                          in_data->time_scale,
                          &checkoutShapeParam));
	PF_LayerDef *shapeLayer = &checkoutShapeParam.u.ld;
    
    // Make new empty AEGP_WorldH
    AEGP_WorldH newWorld;
    ERR(suites.WorldSuite3()->AEGP_New(NULL, AEGP_WorldType_8, shapeLayer->width, shapeLayer->height, &newWorld));
        
    //Copy PF_EffectWorld data into new AEGP_World
    A_u_long rowbytes = 0;
    void *data = 0;
    ERR(suites.WorldSuite3()->AEGP_GetRowBytes(newWorld, &rowbytes));
    ERR(suites.WorldSuite3()->AEGP_GetBaseAddr8(newWorld, (PF_Pixel8**) &data));
        
    auto dst = (char*) data;
    auto src = (char*) shapeLayer->data;
    for(int y=0; y<shapeLayer->height; y++){
        memcpy(dst, src, rowbytes);
        src += shapeLayer->rowbytes;
        dst += rowbytes;
    }
        
    //Blur the AEGP_World
    ERR(suites.WorldSuite3()->AEGP_FastBlur((A_FpLong)params[DROPSHADOW_BLUR]->u.fs_d.value,
                                            PF_MF_Alpha_STRAIGHT,
                                            PF_Quality_HI,
                                            newWorld));
    
    
        
    //Get pointer to the AEGP_World data in a tmpEffectWorld
    PF_EffectWorld tmpEffectWorld;
    ERR(suites.WorldSuite3()->AEGP_FillOutPFEffectWorld(newWorld, &tmpEffectWorld));
    
    //Color all pixels
    PixelInfo pi;
    AEFX_CLR_STRUCT(pi);
    pi.color = params[DROPSHADOW_COLOR]->u.cd.value;
    
    ERR(suites.Iterate8Suite1()->iterate(in_data,
                                         0, // progress base
                                         tmpEffectWorld.height, // progress final
                                         &tmpEffectWorld, // src
                                         NULL, // area - null for all pixels
                                         (void*)&pi, // refcon - your custom data pointer
                                         ColorPixels, // pixel function pointer
                                         &tmpEffectWorld));

    //Transform the effect world and composite over sourceInput
    ERR(PF_COPY(sourceInput, output, NULL, NULL));
    
    PF_CompositeMode blendMode;
    AEFX_CLR_STRUCT(blendMode);
    blendMode.opacity = static_cast<int>((FIX_2_FLOAT(params[DROPSHADOW_OPACITY]->u.fd.value) * 0.01) * PF_MAX_CHAN8);
    switch (params[DROPSHADOW_BLEND]->u.pd.value) {
        case BLEND_NORMAL: blendMode.xfer = PF_Xfer_IN_FRONT; break;
        case BLEND_DARKEN: blendMode.xfer = PF_Xfer_DARKEN; break;
        case BLEND_MULTIPLY: blendMode.xfer = PF_Xfer_MULTIPLY; break;
        case BLEND_COLORBURN: blendMode.xfer = PF_Xfer_COLOR_BURN; break;
        case BLEND_LIGHTEN: blendMode.xfer = PF_Xfer_LIGHTEN; break;
        case BLEND_SCREEN: blendMode.xfer = PF_Xfer_SCREEN; break;
        case BLEND_COLORDODGE: blendMode.xfer = PF_Xfer_COLOR_DODGE; break;
        case BLEND_OVERLAY: blendMode.xfer = PF_Xfer_OVERLAY; break;
        case BLEND_SOFTLIGHT: blendMode.xfer = PF_Xfer_SOFT_LIGHT; break;
        case BLEND_HARDLIGHT: blendMode.xfer = PF_Xfer_HARD_LIGHT; break;
        case BLEND_DIFFERENCE: blendMode.xfer = PF_Xfer_DIFFERENCE; break;
        case BLEND_EXCLUSION: blendMode.xfer = PF_Xfer_EXCLUSION; break;
        case BLEND_HUE: blendMode.xfer = PF_Xfer_HUE; break;
        case BLEND_SATURATION: blendMode.xfer = PF_Xfer_SATURATION; break;
        case BLEND_COLOR: blendMode.xfer = PF_Xfer_COLOR; break;
        case BLEND_LUMINOSITY: blendMode.xfer = PF_Xfer_LUMINOSITY; break;
        default: blendMode.xfer = PF_Xfer_IN_FRONT; break;
    }
    
    PF_FloatMatrix float_matrix;
    AEFX_CLR_STRUCT(float_matrix.mat);
    float_matrix.mat[0][0] = float_matrix.mat[1][1] = float_matrix.mat[2][2] = 1;
    float_matrix.mat[2][0] = params[DROPSHADOW_OFFSETX]->u.fs_d.value; // This is the x translation
    float_matrix.mat[2][1] = params[DROPSHADOW_OFFSETY]->u.fs_d.value; // This is the y translation
    
    ERR(in_data->utils->transform_world(in_data->effect_ref,
                                        in_data->quality,
                                        PF_MF_Alpha_STRAIGHT,
                                        in_data->field,
                                        &tmpEffectWorld,
                                        &blendMode,
                                        NULL,
                                        &float_matrix,
                                        1,
                                        TRUE,
                                        &output->extent_hint,
                                        output));
	
    //Knock out sourceInput's alpha in output
    ERR(suites.Iterate8Suite1()->iterate(in_data,
                                            0, // progress base
                                            shapeLayer->height, // progress final
                                            shapeLayer, // src
                                            NULL, // area - null for all pixels
                                            NULL, // refcon - your custom data pointer
                                            KnockOut, // pixel function pointer
                                            output));

	//Fill empty pixels in with sourceInput
	ERR(in_data->utils->composite_rect(in_data->effect_ref,
		NULL,
		PF_MAX_CHAN8,
		sourceInput,
		0,
		0,
		in_data->field,
		PF_Xfer_IN_FRONT,
		output));
    
    //Check in sourceInput
    ERR(PF_CHECKIN_PARAM(in_data, &checkoutShapeParam));
    
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
