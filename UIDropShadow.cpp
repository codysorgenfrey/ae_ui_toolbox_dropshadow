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

    out_data->out_flags =  PF_OutFlag_NONE;
	
	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def; // used in the macros

	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Radius_Param_Name),
							DROPSHADOW_RADIUS_MIN,
							DROPSHADOW_RADIUS_MAX,
							DROPSHADOW_RADIUS_MIN,
							DROPSHADOW_RADIUS_MAX,
							DROPSHADOW_RADIUS_DFLT,
							PF_Precision_HUNDREDTHS,
							0,
							0,
							RADIUS_DISK_ID);
	
	out_data->num_params = DROPSHADOW_NUM_PARAMS;

	return err;
}

static PF_Err
CopyAlpha (
    void        *refcon,
    A_long        xL,
    A_long        yL,
    PF_Pixel8    *inP,
    PF_Pixel8    *outP)
{
    PF_Err err = PF_Err_NONE;
    outP->alpha = inP->alpha;
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
    PF_LayerDef *input = &params[DROPSHADOW_INPUT]->u.ld;
    
    if (params[DROPSHADOW_RADIUS]->u.fs_d.value){    // we're doing some blurring...
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
        ERR(suites.WorldSuite3()->AEGP_FastBlur((A_FpLong)params[DROPSHADOW_RADIUS]->u.fs_d.value,
                                                PF_MF_Alpha_STRAIGHT,
                                                PF_Quality_HI,
                                                newWorld));
        
        //Convert the AEGP_World data into a PF_EffectWorld
        PF_EffectWorld tmpEffectWorld;
        ERR(suites.WorldSuite3()->AEGP_FillOutPFEffectWorld(newWorld, &tmpEffectWorld));
        
        //Copy input's alpha into the effectworld
        ERR(suites.Iterate8Suite1()->iterate(in_data,
                                             0, // progress base
                                             input->height, // progress final
                                             input, // src
                                             NULL, // area - null for all pixels
                                             NULL, // refcon - your custom data pointer
                                             CopyAlpha, // pixel function pointer
                                             &tmpEffectWorld));
        
        //Copy the tmpEffectWorld to the output LayerDef
        ERR(PF_COPY(&tmpEffectWorld, output, NULL, NULL));
        
        //Clean up worldH
        ERR(suites.WorldSuite3()->AEGP_Dispose(newWorld));
        
    } else {
        ERR(PF_COPY(input, output, NULL, NULL));
    }

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

