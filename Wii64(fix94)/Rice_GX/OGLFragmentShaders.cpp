/*
Rice_GX - OGLFragmentShaders.cpp
Copyright (C) 2003 Rice1964
Copyright (C) 2010, 2011, 2012 sepp256 (Port to Wii/Gamecube/PS3)
Wii64 homepage: http://www.emulatemii.com
email address: sepp256@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __GX__
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#endif //__GX__

#include "stdafx.h"

COGLFragmentShaderCombiner::COGLFragmentShaderCombiner(CRender *pRender)
: COGLColorCombiner(pRender)
{
    m_bShaderIsSupported = false;
}
COGLFragmentShaderCombiner::~COGLFragmentShaderCombiner()
{
}

bool COGLFragmentShaderCombiner::Initialize(void)
{
    if( !COGLColorCombiner::Initialize() )
        return false;

    COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);
    if( pcontext->IsExtensionSupported("GL_ARB_fragment_shader") )
    {
        m_bShaderIsSupported = true;
    }

    return true;
}

void COGLFragmentShaderCombiner::InitCombinerCycle12(void)
{
}
void COGLFragmentShaderCombiner::DisableCombiner(void)
{
    COGLColorCombiner::DisableCombiner();
}

void COGLFragmentShaderCombiner::InitCombinerCycleCopy(void)
{
    COGLColorCombiner::InitCombinerCycleCopy();
}

void COGLFragmentShaderCombiner::InitCombinerCycleFill(void)
{
    COGLColorCombiner::InitCombinerCycleFill();
}
void COGLFragmentShaderCombiner::InitCombinerBlenderForSimpleTextureDraw(uint32 tile)
{
    COGLColorCombiner::InitCombinerBlenderForSimpleTextureDraw(tile);
}

#ifdef _DEBUG
void COGLFragmentShaderCombiner::DisplaySimpleMuxString(void)
{
    COGLColorCombiner::DisplaySimpleMuxString();
}
#endif



COGL_FragmentProgramCombiner::COGL_FragmentProgramCombiner(CRender *pRender)
: COGLColorCombiner4(pRender)
{
    delete m_pDecodedMux;
    m_pDecodedMux = new DecodedMuxForPixelShader;
    m_bFragmentProgramIsSupported = false;
}
COGL_FragmentProgramCombiner::~COGL_FragmentProgramCombiner()
{
    int size = m_vCompiledShaders.size();
    for (int i=0; i<size; i++)
    {
        GLuint ID = m_vCompiledShaders[i].programID;
#ifndef __GX__
        glDeleteProgramsARB(1, &ID);
#endif //!__GX__
        m_vCompiledShaders[i].programID = 0;
    }

    m_vCompiledShaders.clear();
}

bool COGL_FragmentProgramCombiner::Initialize(void)
{
    if( !COGLColorCombiner4::Initialize() )
        return false;

    COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);
    if( pcontext->IsExtensionSupported("GL_ARB_fragment_program") )
    {
        m_bFragmentProgramIsSupported = true;
    }

    return true;
}



void COGL_FragmentProgramCombiner::DisableCombiner(void)
{
#ifndef __GX__
	//TODO: Implement in GX
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
#endif //!__GX__
    COGLColorCombiner4::DisableCombiner();
}

void COGL_FragmentProgramCombiner::InitCombinerCycleCopy(void)
{
#ifndef __GX__
	//TODO: Implement in GX
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
#endif //!__GX__
    COGLColorCombiner4::InitCombinerCycleCopy();
}

void COGL_FragmentProgramCombiner::InitCombinerCycleFill(void)
{
#ifndef __GX__
	//TODO: Implement in GX
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
#endif //!__GX__
    COGLColorCombiner4::InitCombinerCycleFill();
}

const char *muxToFP_Maps[][2] = {
//color -- alpha
{"0", "0"}, //MUX_0 = 0,
{"1", "1"}, //MUX_1,
{"comb", "comb.a"}, //MUX_COMBINED,
{"t0", "t0.a"}, //MUX_TEXEL0,
{"t1", "t1.a"}, //MUX_TEXEL1,
{"program.env[2]", "program.env[2].a"}, //MUX_PRIM,
{"fragment.color", "fragment.color.a"}, //MUX_SHADE,
{"program.env[1]", "program.env[1].a"}, //MUX_ENV,
{"comb.a", "comb.a"}, //MUX_COMBALPHA,
{"t0.a", "t0.a"}, //MUX_T0_ALPHA,   
{"t1.a", "t1.a"}, //MUX_T1_ALPHA,   
{"primcolor.a", "primcolor.a"}, //MUX_PRIM_ALPHA,   
{"fragment.color.a", "fragment.color.a"}, //MUX_SHADE_ALPHA,    
{"envcolor.a", "envcolor.a"}, //MUX_ENV_ALPHA,  
{"program.env[3]", "program.env[3]"}, //MUX_LODFRAC,
{"program.env[4]", "program.env[4]"}, //MUX_PRIMLODFRAC,
{"1", "1"}, //MUX_K5,
{"1", "1"}, //MUX_UNK,  // Should not be used
};


const char *oglFPTest = 
"!!ARBfp1.0\n"
"#Declarations\n"
"TEMP t0;\n"
"TEMP t1;\n"
"TEMP comb;\n"
"TEMP comb2;\n"
"\n"
"ATTRIB coord0 = fragment.texcoord[0];\n"
"ATTRIB coord1 = fragment.texcoord[1];\n"
"ATTRIB shade = fragment.color;\n"
"ATTRIB fogfactor = fragment.fogcoord;\n"
"\n"
"OUTPUT out = result.color;\n"
"\n"
"#Instructions\n"
"TEX t0, coord0, texture[0], 2D;\n"
"TEX t1, coord1, texture[1], 2D;\n"
"\n"
"MAD_SAT out, t0, program.env[1],program.env[0];\n"
//"SUB comb.rgb,    t0, 0;\n"
//"MAD_SAT out.rgb, comb, program.env[1], 0;\n"
//"SUB comb.a,      t0, 0;\n"
//"MAD_SAT out.a,   comb, program.env[1], 0;\n"
"END\n";

char oglNewFP[4092];

char* MuxToOC(uint8 val)
{
// For color channel
if( val&MUX_ALPHAREPLICATE )
    return (char*)muxToFP_Maps[val&0x1F][1];
else
    return (char*)muxToFP_Maps[val&0x1F][0];
}

char* MuxToOA(uint8 val)
{
// For alpha channel
return (char*)muxToFP_Maps[val&0x1F][0];
}

void COGL_FragmentProgramCombiner::GenerateProgramStr()
{
    DecodedMuxForPixelShader &mux = *(DecodedMuxForPixelShader*)m_pDecodedMux;

    mux.splitType[0] = mux.splitType[1] = mux.splitType[2] = mux.splitType[3] = CM_FMT_TYPE_NOT_CHECKED;
    m_pDecodedMux->Reformat(false);

    const char *leadstr = "!!ARBfp1.0\n"
        "#Declarations\n"
        "%s\n" //"OPTION ARB_fog_linear;\n"
        "TEMP t0;\n"
        "TEMP t1;\n"
        "TEMP comb;\n"
        "TEMP comb2;\n"
        "\n"
        "#Instructions\n"
        "TEX t0, fragment.texcoord[0], texture[0], 2D;\n"
        "TEX t1, fragment.texcoord[1], texture[1], 2D;\n"
        "\n"
        "# N64 cycle 1, result is in comb\n";

    // New solution
    bool bFog = gRDP.bFogEnableInBlender && gRSP.bFogEnabled;
    oglNewFP[0] = 0;
    char tempstr[500];

    sprintf(oglNewFP, leadstr, bFog?"OPTION ARB_fog_linear;":"\n");
    for( int cycle=0; cycle<2; cycle++ )
    {
        for( int channel=0; channel<2; channel++)
        {
            char* (*func)(uint8) = channel==0?MuxToOC:MuxToOA;
            char *dst = channel==0?(char*)"rgb":(char*)"a";
            N64CombinerType &m = mux.m_n64Combiners[cycle*2+channel];
            switch( mux.splitType[cycle*2+channel] )
            {
            case CM_FMT_TYPE_NOT_USED:
                sprintf(tempstr, "\n");
                break;
            case CM_FMT_TYPE_D:
                sprintf(tempstr, "MOV comb.%s, %s;\n", dst, func(m.d));
                break;
            case CM_FMT_TYPE_A_MOD_C:
                sprintf(tempstr, "MUL comb.%s, %s, %s;\n", dst, func(m.a), func(m.c));
                break;
            case CM_FMT_TYPE_A_ADD_D:
                sprintf(tempstr, "ADD_SAT comb.%s, %s, %s;\n", dst, func(m.a), func(m.d));
                break;
            case CM_FMT_TYPE_A_SUB_B:
                sprintf(tempstr, "SUB comb.%s, %s, %s;\n", dst, func(m.a), func(m.b));
                break;
            case CM_FMT_TYPE_A_MOD_C_ADD_D:
                sprintf(tempstr, "MAD_SAT comb.%s, %s, %s, %s;\n", dst, func(m.a), func(m.c), func(m.d));
                break;
            case CM_FMT_TYPE_A_LERP_B_C:
                sprintf(tempstr, "LRP_SAT comb.%s, %s, %s, %s;\n", dst, func(m.c), func(m.a), func(m.b));
                //sprintf(tempstr, "SUB comb.%s, %s, %s;\nMAD_SAT comb.%s, comb, %s, %s;\n", dst, func(m.a), func(m.b), dst, func(m.c), func(m.b));
                break;
            default:
                sprintf(tempstr, "SUB comb2.%s, %s, %s;\nMAD_SAT comb.%s, comb2, %s, %s;\n", dst, func(m.a), func(m.b), dst, func(m.c), func(m.d));
                break;
            }
            strcat(oglNewFP, tempstr);
        }
    }

    strcat(oglNewFP, "MOV result.color, comb;\n");
    strcat(oglNewFP, "END\n\n");
}

int COGL_FragmentProgramCombiner::ParseDecodedMux()
{
    if( !m_bFragmentProgramIsSupported )
        return COGLColorCombiner4::ParseDecodedMux();

    OGLShaderCombinerSaveType res;

#ifndef __GX__
	//TODO: Implement in GX
    glGenProgramsARB( 1, &res.programID);
    glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, res.programID);
#endif //!__GX__
    GenerateProgramStr();

#ifndef __GX__
	//TODO: Implement in GX
    glProgramStringARB( GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(oglNewFP), oglNewFP);
    //glProgramStringARB(   GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB, strlen(oglFPTest), oglFPTest);

    if (glGetError() != 0)
    {
        GLint position;
#ifdef _DEBUG
        char *str = (char*)glGetString(GL_PROGRAM_ERROR_STRING_ARB);
#endif
        glGetIntegerv( GL_PROGRAM_ERROR_POSITION_ARB, &position);
        if( position >= 0 )
        {
#ifdef _DEBUG
            if( m_lastIndex >= 0 ) COGLColorCombiner4::DisplaySimpleMuxString();
            ErrorMsg("%s\n\n%s\n", str, oglNewFP+position);
#endif
            glDisable(GL_FRAGMENT_PROGRAM_ARB);
            return COGLColorCombiner4::ParseDecodedMux();
        }
    }

    glEnable(GL_FRAGMENT_PROGRAM_ARB);
#endif //!__GX__
    res.dwMux0 = m_pDecodedMux->m_dwMux0;
    res.dwMux1 = m_pDecodedMux->m_dwMux1;
    res.fogIsUsed = gRDP.bFogEnableInBlender && gRSP.bFogEnabled;

    m_vCompiledShaders.push_back(res);
    m_lastIndex = m_vCompiledShaders.size()-1;

    return m_lastIndex;
}

void COGL_FragmentProgramCombiner::GenerateCombinerSetting(int index)
{
    GLuint ID = m_vCompiledShaders[index].programID;
#ifndef __GX__
	//TODO: Implement in GX
    glBindProgramARB( GL_FRAGMENT_PROGRAM_ARB, ID );
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
#endif //!__GX__
}

void COGL_FragmentProgramCombiner::GenerateCombinerSettingConstants(int index)
{
#ifndef __GX__
	//TODO: Implement in GX
    float *pf;
    pf = GetEnvColorfv();
    glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, pf);
    pf = GetPrimitiveColorfv();
    glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, pf);

    float frac = gRDP.LODFrac / 255.0f;
    float tempf[4] = {frac,frac,frac,frac};
    glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 3, tempf);

    float frac2 = gRDP.primLODFrac / 255.0f;
    float tempf2[4] = {frac2,frac2,frac2,frac2};
    glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, tempf2);

    float tempf3[4] = {0,0,0,0};
    glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, tempf3);
    glProgramEnvParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 6, tempf3);
#endif //!__GX__
}

int COGL_FragmentProgramCombiner::FindCompiledMux()
{
#ifdef _DEBUG
    if( debuggerDropCombiners )
    {
        m_vCompiledShaders.clear();
        //m_dwLastMux0 = m_dwLastMux1 = 0;
        debuggerDropCombiners = false;
    }
#endif
    for( uint32 i=0; i<m_vCompiledShaders.size(); i++ )
    {
        if( m_vCompiledShaders[i].dwMux0 == m_pDecodedMux->m_dwMux0 
            && m_vCompiledShaders[i].dwMux1 == m_pDecodedMux->m_dwMux1 
            && m_vCompiledShaders[i].fogIsUsed == (gRDP.bFogEnableInBlender && gRSP.bFogEnabled) )
            return (int)i;
    }

    return -1;
}

//////////////////////////////////////////////////////////////////////////
void COGL_FragmentProgramCombiner::InitCombinerCycle12(void)
{
    if( !m_bFragmentProgramIsSupported )    
    {
        COGLColorCombiner4::InitCombinerCycle12();
        return;
    }

#ifdef _DEBUG
    if( debuggerDropCombiners )
    {
        UpdateCombiner(m_pDecodedMux->m_dwMux0,m_pDecodedMux->m_dwMux1);
        m_vCompiledShaders.clear();
        m_dwLastMux0 = m_dwLastMux1 = 0;
        debuggerDropCombiners = false;
    }
#endif

    m_pOGLRender->EnableMultiTexture();

    bool combinerIsChanged = false;

    if( m_pDecodedMux->m_dwMux0 != m_dwLastMux0 || m_pDecodedMux->m_dwMux1 != m_dwLastMux1 || m_lastIndex < 0 )
    {
        combinerIsChanged = true;
        m_lastIndex = FindCompiledMux();
        if( m_lastIndex < 0 )       // Can not found
        {
            m_lastIndex = ParseDecodedMux();
        }

        m_dwLastMux0 = m_pDecodedMux->m_dwMux0;
        m_dwLastMux1 = m_pDecodedMux->m_dwMux1;
    }


    GenerateCombinerSettingConstants(m_lastIndex);
    if( m_bCycleChanged || combinerIsChanged || gRDP.texturesAreReloaded || gRDP.colorsAreReloaded )
    {
        if( m_bCycleChanged || combinerIsChanged )
        {
            GenerateCombinerSettingConstants(m_lastIndex);
            GenerateCombinerSetting(m_lastIndex);
        }
        else if( gRDP.colorsAreReloaded )
        {
            GenerateCombinerSettingConstants(m_lastIndex);
        }

        m_pOGLRender->SetAllTexelRepeatFlag();

        gRDP.colorsAreReloaded = false;
        gRDP.texturesAreReloaded = false;
    }
    else
    {
        m_pOGLRender->SetAllTexelRepeatFlag();
    }
}

#ifdef _DEBUG
void COGL_FragmentProgramCombiner::DisplaySimpleMuxString(void)
{
    COGLColorCombiner::DisplaySimpleMuxString();
    DecodedMuxForPixelShader &mux = *(DecodedMuxForPixelShader*)m_pDecodedMux;
    mux.Reformat(false);
    GenerateProgramStr();
    //sprintf(oglNewFP, oglFP, 
    //  MuxToOC(mux.aRGB0), MuxToOC(mux.bRGB0), MuxToOC(mux.cRGB0), MuxToOC(mux.dRGB0),
    //  MuxToOA(mux.aA0), MuxToOA(mux.bA0), MuxToOA(mux.cA0), MuxToOA(mux.dA0),
    //  MuxToOC(mux.aRGB1), MuxToOC(mux.bRGB1), MuxToOC(mux.cRGB1), MuxToOC(mux.dRGB1),
    //  MuxToOA(mux.aA1), MuxToOA(mux.bA1), MuxToOA(mux.cA1), MuxToOA(mux.dA1)
    //  );

    TRACE0("OGL Fragment Program:");
    TRACE0(oglNewFP);
}
#endif

