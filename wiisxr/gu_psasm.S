/* General Purpose Registers (GPRs) */

#define    r0    0
#define    r1    1
#define    r2    2
#define    r3    3
#define    r4    4
#define    r5    5
#define    r6    6
#define    r7    7
#define    r8    8
#define    r9    9
#define    r10    10
#define    r11    11
#define    r12    12
#define    r13    13
#define    r14    14
#define    r15    15
#define    r16    16
#define    r17    17
#define    r18    18
#define    r19    19
#define    r20    20
#define    r21    21
#define    r22    22
#define    r23    23
#define    r24    24
#define    r25    25
#define    r26    26
#define    r27    27
#define    r28    28
#define    r29    29
#define    r30    30
#define    r31    31

#define    lr    8

/* Floating Point Registers (FPRs) */

#define	fr0		0
#define	fr1		1
#define	fr2		2
#define	fr3		3
#define	fr4		4
#define	fr5		5
#define	fr6		6
#define	fr7		7
#define	fr8		8
#define	fr9		9
#define	fr10	10
#define	fr11	11
#define	fr12	12
#define	fr13	13
#define	fr14	14
#define	fr15	15
#define	fr16	16
#define	fr17	17
#define	fr18	18
#define	fr19	19
#define	fr20	20
#define	fr21	21
#define	fr22	22
#define	fr23	23
#define	fr24	24
#define	fr25	25
#define	fr26	26
#define	fr27	27
#define	fr28	28
#define	fr29	29
#define	fr30	30
#define	fr31	31

#define fr14ShiftR12 14
#define fr15ShiftR16 15
#define fr10Kmagic   10

// FNC_OVERFLOW1 fr8: chk1/chk2, fr12: chkL/chkL, fr11: chkH/chkH, r10: highFlg, r9: lowFlg, r8: allFlg
FNC_OVERFLOW1:
    // chk low
    ps_cmpo0    fr0,fr8,fr12         // if (chk1 < chkL)
    blt        FNC_OVERFLOW_L_Flag

    // chk high
    ps_cmpo0    fr0,fr11,fr8         // if (chkH < chk1)
    blt        FNC_OVERFLOW_H_Flag
    blr

// FNC_OVERFLOW2 fr8: chk1/chk2, fr12: chkL/chkL, fr11: chkH/chkH, r10: highFlg, r9: lowFlg, r8: allFlg
FNC_OVERFLOW2:
    // chk low
    ps_cmpo1    fr0,fr8,fr12         // if (chk2 < chkL)
    blt         FNC_OVERFLOW_L_Flag

    // chk high
    ps_cmpo1    fr0,fr11,fr8         // if (chkH < chk2)
    blt         FNC_OVERFLOW_H_Flag
    blr

FNC_OVERFLOW_L_Flag:
    or          r8,r8,r9             // set low flag
    blr
FNC_OVERFLOW_H_Flag:
    or          r8,r8,r10            // set high flag
    blr

// NC_OVERFLOW r6: chk, r14: LVal, r15: HVal, r10: highFlg, r9: lowFlg, r8: allFlg, r7: retVal
NC_OVERFLOW:
    // chk low
    cmp         0,1,r6,r14               // if (chk < L)
    blt         NC_OVERFLOW_L_Flag

    // chk high
    cmp         0,1,r15,r6               // if (H < chk)
    blt         NC_OVERFLOW_H_Flag
    addi        r7,r6,0
    blr

NC_OVERFLOW_L_Flag:
    or          r8,r8,r9             // set low flag
    addi        r7,r14,0
    blr
NC_OVERFLOW_H_Flag:
    or          r8,r8,r10            // set high flag
    addi        r7,r15,0
    blr

// fr6: gteSZX, fr5: retVAl
//NATIVE_DIVIDE:
//    psq_l       fr5,106(r3),1,4      // fr5 = gteH   (psxRegs.CP2C.p[26].w.l)
//    lfs		    fr7,MUL2@sdarel(r13)
//    fmuls       fr7, fr7, fr6
//    fcmpu       fr0, fr5, fr7
//    blt         DIVIDE_DO            // if gteH < gteSZX * 2
//    lfs		    fr5,MAXDIV@sdarel(r13)// fr5 = 0x1ffff
//    blr
//
//DIVIDE_DO:
//    lfs		    fr8,MUL16@sdarel(r13)
//    fmuls       fr5, fr5, fr8        // gteH << 16
//    fdivs       fr5, fr5, fr6        // gteH << 16 / gteSZX
//    lfs		    fr8,MAXDIV@sdarel(r13)
//    fcmpu       fr0, fr8, fr5        // retVal > 0x1ffff
//    blt         DIVIDE_OVERFLOW
//    blr
//
//DIVIDE_OVERFLOW:
//    oris        r8, r8, 2            // set overflow flag 1<<17
//    fmr         fr5, fr8
//    blr
NATIVE_DIVIDE:
    lhz         r9, 106(r3)          // r9 = gteH   (psxRegs.CP2C.p[26].w.l)
    rotlwi      r10, r6, 1
    cmp         0, 1, r9, r10
    blt         DIVIDE_DO            // if gteH < gteSZX * 2
    lfs		    fr5,MAXDIV@sdarel(r13)// fr5 = 0x1ffff
    blr

DIVIDE_DO:
    rotlwi      r9, r9, 16           // gteH << 16
    divwu       r9, r9, r6           // gteH << 16 / gteSZX
    lwz         r15, DivOverflow@sdarel(r13)
    cmp         0, 1, r15, r9        // retVal > 0x1ffff
    blt         DIVIDE_OVERFLOW
    xoris r9, r9, 0x8000
    stw r9, FloatBufB@sdarel(r13)
    lfd fr5, FloatBufA@sdarel(r13)
    fsub fr5, fr5, fr10Kmagic        // fr3 = TRY(double)
    frsp fr5, fr5                    // fr3 = TRY(single)
    blr

DIVIDE_OVERFLOW:
    oris        r8, r8, 2            // set overflow flag 1<<17
    lfs		    fr5,MAXDIV@sdarel(r13)// fr5 = 0x1ffff
    blr

asm_rtps_comn_mac:
    mfspr r18, lr                    // save link register

    psq_l		fr5, 0(r5), 0, 3     // fr5 = y, x
	psq_l		fr4, 4(r5), 0, 3     // fr4 = 0, z
	ps_merge10  fr4, fr4, fr4        // fr4 = z, 0

    psq_l		fr0,0(r3),0,3        // fr0 = m12, m11
	ps_mul		fr0,fr0,fr5          // fr0 = m12 * y, m11 * x

	psq_l		fr1,6(r3),1,3        // fr1 = m13, 1.0
	psq_l		fr2,4(r3),1,3        // fr2 = m21, 1.0
	psq_l		fr3,10(r3),1,3       // fr3 = m22, 1.0
	ps_merge00  fr2,fr3,fr2          // fr2 = m22, m21

	ps_madd		fr0,fr1,fr4,fr0      // fr0 = (m13 * z) + (m12 * y), (1.0 * 0.0) + (m11 * x)
    ps_mul		fr1,fr2,fr5          // fr1 = m22 * y, m21 * x
	ps_sum0		fr0,fr0,fr0,fr0      // fr0 = (m13 * z) + (m12 * y) + (1.0 * 0.0) + (m11 * x), 1.0

	psq_l		fr2,8(r3),1,3        // fr2 = m23, 1.0
	ps_madd		fr1,fr2,fr4,fr1      // fr1 = (m23 * z) + (m22 * y), (1.0 * 0.0) + (m21 * x)

	psq_l		fr3,12(r3),0,3       // fr3 = m32, m31
	psq_l		fr2,18(r3),1,3       // fr2 = m33, 1.0
	ps_sum0		fr1,fr1,fr1,fr1      // fr1 = (m23 * z) + (m22 * y) + (1.0 * 0.0) + (m21 * x), 1.0

    ps_merge00  fr0,fr0,fr1          // fr0 = SUMX, SUMY

	ps_mul		fr1,fr3,fr5          // fr3 = m32 * y, m31 * x
    ps_madd		fr1,fr2,fr4,fr1      // fr1 = (m33 * z) + (m32 * y), (1.0 * 0.0) + (m31 * x)
    ps_sum0		fr1,fr1,fr1,fr1      // fr1 = (m33 * z) + (m32 * y) + (1.0 * 0.0) + (m31 * x) = SUMZ

    // load TRX, TRY, TRZ to float
    lwz r9, 20(r3)                   // gteTRX (((s32 *)regs->CP2C.r)[5])
    xoris r9, r9, 0x8000
    stw r9, FloatBufB@sdarel(r13)
    lfd fr2, FloatBufA@sdarel(r13)
    fsub fr2, fr2, fr10Kmagic        // fr2 = TRX(double)
    frsp fr2, fr2                    // fr2 = TRX(single)

    lwz r9, 24(r3)                   // gteTRY (((s32 *)regs->CP2C.r)[6])
    xoris r9, r9, 0x8000
    stw r9, FloatBufB@sdarel(r13)
    lfd fr3, FloatBufA@sdarel(r13)
    fsub fr3, fr3, fr10Kmagic        // fr3 = TRY(double)
    frsp fr3, fr3                    // fr3 = TRY(single)

    ps_merge00  fr2, fr2, fr3        // fr2 = TRX, TRY
    ps_mul      fr0, fr0, fr14ShiftR12 // fr0 = SUMX >> 12, SUMY >> 12
	ps_add		fr0, fr0, fr2        // fr0 = SUMX + TRX, SUMY + TRY

	lwz         r9, 28(r3)           // gteTRZ (((s32 *)regs->CP2C.r)[7])
    xoris       r9, r9, 0x8000
    stw         r9, FloatBufB@sdarel(r13)
    lfd         fr2, FloatBufA@sdarel(r13)
    fsub        fr2, fr2, fr10Kmagic // fr2 = TRZ(double)
    frsp        fr2, fr2             // fr2 = TRZ(single)
    ps_mul      fr1, fr1, fr14ShiftR12  // fr1 = SUMZ >> 12
	ps_add		fr1, fr1, fr2        // fr1 = SUMZ + TRZ, 0

    // A1, A2, A3 check
    // fr8: chk1/chk2, fr12: chkL/chkL, fr11: chkH/chkH, r10: highFlg, r9: lowFlg, r8: allFlg
    //ps_mr       fr8, fr0
	//lis r10, 0x4000                  // gteFLAG |= 1<<30;
    //lis r9, 0x800                    // gteFLAG |= 1<<27;
    //bl FNC_OVERFLOW1                 // check SUMX + TRX

    //lis r10, 0x2000                  // gteFLAG |= 1<<29;
    //lis r9, 0x400                    // gteFLAG |= 1<<26;
    //bl FNC_OVERFLOW2                 // check SUMY + TRY

    //ps_mr      fr8, fr1              // fr8 = SUMZ + TRZ, 0
    //lis r10, 0x1000                  // gteFLAG |= 1<<28;
    //lis r9, 0x200                    // gteFLAG |= 1<<25;
    //bl FNC_OVERFLOW1                 // check SUMZ + TRZ

    addi       r9,r4,100
    fctiw      fr6, fr0              // SUMX + TRX => int
    stfiwx     fr6,0,r9              // SUMX + TRX => gteMAC1 CP2D.r)[25]

    addi       r9,r9,4
    ps_merge10 fr0,fr0,fr0           // fr0 = SUMY + TRY
    fctiw      fr6, fr0              // SUMY + TRY => int
    stfiwx     fr6,0,r9              // SUMY + TRY => gteMAC2 CP2D.r)[26]

    addi       r9,r9,4
    fctiw      fr6, fr1              // SUMZ + TRZ => int
    stfiwx     fr6,0,r9              // SUMZ + TRZ => gteMAC3 CP2D.r)[27]

    // MAC int check
    lwz r14, IntOverflowL@sdarel(r13)
    lwz r15, IntOverflowH@sdarel(r13)

    lwz r6, 100(r4)                  // gteMAC1 CP2D.r)[25]
    lis r10, 0x4000                  // gteFLAG |= 1<<30;
    lis r9, 0x800                    // gteFLAG |= 1<<27;
    bl NC_OVERFLOW                   // A1 check

    lwz r6, 104(r4)                  // gteMAC2 CP2D.r)[26]
    lis r10, 0x2000                  // gteFLAG |= 1<<29;
    lis r9, 0x400                    // gteFLAG |= 1<<26;
    bl NC_OVERFLOW                   // A2 check

    lwz r6, 108(r4)                  // gteMAC3 CP2D.r)[27]
    lis r10, 0x1000                  // gteFLAG |= 1<<28;
    lis r9, 0x200                    // gteFLAG |= 1<<25;
    bl NC_OVERFLOW                   // A3 check

    // limB1, limB2, limB3 check
    addi r14, 0, -0x8000
    addi r15, 0, 0x7fff

    // limB1 Check: r6: chk, r14: LVal, r15: HVal, r10: highFlg, r9: lowFlg, r8: allFlg, r7: retVal
    lwz r6, 100(r4)                  // gteMAC1 CP2D.r)[25]
    lis r9, 0x100                    // gteFLAG |= 1<<24;
    lis r10, 0x100                   // gteFLAG |= 1<<24;
    bl NC_OVERFLOW                   // limB1 check
    stw r7,36(r4)                    // set gteIR1  (s32*)psxRegs.CP2D.r)[9]

    // limB2 Check: r6: chk, r14: LVal, r15: HVal, r10: highFlg, r9: lowFlg, r8: allFlg, r7: retVal
    lwz r6, 104(r4)                  // gteMAC2 CP2D.r)[26]
    lis r9, 0x80                     // gteFLAG |= 1<<23;
    lis r10, 0x80                    // gteFLAG |= 1<<23;
    bl NC_OVERFLOW                   // limB2 check
    stw r7,40(r4)                    // set gteIR2  (s32*)psxRegs.CP2D.r)[10]

    // limB3 Check: r6: chk, r14: LVal, r15: HVal, r10: highFlg, r9: lowFlg, r8: allFlg, r7: retVal
    lwz r6, 108(r4)                  // gteMAC3 CP2D.r)[27]
    lis r9, 0x40                     // gteFLAG |= 1<<22;
    lis r10, 0x40                    // gteFLAG |= 1<<22;
    bl NC_OVERFLOW                   // limB3 check
    stw r7,44(r4)                    // set gteIR3  (s32*)psxRegs.CP2D.r)[11]
    mtspr lr, r18
    blr

asm_rtps_comn_sxy:
    mfspr r18, lr                    // save link register

    // limD(gteMAC3) r6: chk, r14: LVal, r15: HVal, r10: highFlg, r9: lowFlg, r8: allFlg, r7: retVal
    lwz r6, 108(r4)                  // gteMAC3 CP2D.r)[27]
    lis r9, 0x4                      // gteFLAG |= 1<<18;
    lis r10, 0x4                     // gteFLAG |= 1<<18;
    addi r14, 0, 0
    addis r15, 0, 0x1                // 0x10000
    addic. r15, r15, -1              // r15 = 0x10000 - 1 = 0xffff
    bl NC_OVERFLOW                   // limD(gteMAC3) check

    sth r7, 0(r5)                    // gteSZ3  (regs->CP2D.p[19].w.l)

    // DIVIDE(gteH, gteSZ3) and limE
    //psq_l fr6, 0(r5), 1, 4           // fr6 = gteSZ3  (regs->CP2D.p[19].w.l)
    lhz r6, 0(r5)                    // r6 = getSZX
    bl NATIVE_DIVIDE                 // fr5 = limE(DIVIDE(gteH, gteSZ3))

    ps_merge00 fr5, fr5, fr5         // fr5 = quotient / quotient

    // (s64)gteOFX + ((s64)gteIR1 * quotient)
    // (s64)gteOFY + ((s64)gteIR2 * quotient)
    // Load getOFX�C gteOFY
    lwz r9, 96(r3)                   // gteOFX (((s32 *)regs->CP2C.r)[24])
    xoris r9, r9, 0x8000
    stw r9, FloatBufB@sdarel(r13)
    lfd fr0, FloatBufA@sdarel(r13)
    fsub fr0, fr0, fr10Kmagic        // fr0 = gteOFX
    frsp fr0, fr0                    // double to single

    lwz r9, 100(r3)                  // gteOFY (((s32 *)regs->CP2C.r)[25])
    xoris r9, r9, 0x8000
    stw r9, FloatBufB@sdarel(r13)
    lfd fr1, FloatBufA@sdarel(r13)
    fsub fr1, fr1, fr10Kmagic        // fr1 = gteOFY
    frsp fr1, fr1                    // double to single
    ps_merge00  fr0,fr0,fr1          // fr0 = gteOFX / gteOFY

    // load IR1, IR2
    psq_l		fr1,38(r4),1,3       // gteIR1  (regs->CP2D.p[9].sw.l), fr1 = gteIR1 / 1.0
    psq_l		fr2,42(r4),1,3       // gteIR2  (regs->CP2D.p[10].sw.l), fr2 = gteIR2 / 1.0
    ps_merge00  fr1,fr1,fr2          // fr1 = gteIR1 / gteIR2

    ps_madd     fr0,fr1,fr5,fr0
    ps_mr       fr8, fr0              // fr8 = (s64)gteOFX + ((s64)gteIR1 * quotient) / (s64)gteOFY + ((s64)gteIR2 * quotient)

    addis       r10, 0, 0x1           // gteFLAG |= 1<<16;
    addi        r9, 0, 0x7fff
    addi        r9, r9, 1             // gteFLAG |= 1<<15;
    bl FNC_OVERFLOW1                  // F (s64)gteOFX + ((s64)gteIR1 * quotient)
    bl FNC_OVERFLOW2                  // F (s64)gteOFY + ((s64)gteIR2 * quotient)

    ps_merge00  fr15ShiftR16, fr15ShiftR16, fr15ShiftR16 // fr15 = 1 >> 16 / 1 >> 16
    ps_mul      fr8, fr8, fr15ShiftR16

	// store gteSY2, gteSX2
	ps_merge10  fr8, fr8, fr8
	psq_st		fr8, 0(r16),0,3       // gteSY2gteSX2  (regs->CP2D.p[14].sw)

	// limG1(gteSX2), limG2(gteSY2)
    // r6: chk, r14: LVal, r15: HVal, r10: highFlg, r9: lowFlg, r8: allFlg, r7: retVal
    lha  r6, 2(r16)
    addi r9, 0, 0x4000                // gteFLAG |= 1<<14;
    addi r10, 0, 0x4000               // gteFLAG |= 1<<14;
    addi r14, 0, -0x400
    addi r15, 0, 0x3ff
    bl NC_OVERFLOW                    // limG1(gteSX2)
    sth r7, 2(r16)                     // store gteSX2

    lha r6, 0(r16)
    addi r9, 0, 0x2000                // gteFLAG |= 1<<13;
    addi r10, 0, 0x2000               // gteFLAG |= 1<<13;
    bl NC_OVERFLOW                    // limG2(gteSY2)
    sth r7, 0(r16)                    // store gteSY2
    mtspr lr, r18
    blr

asm_rtps_comn_dqab:
    mfspr r18, lr                    // save link register

    // (s64)gteDQB + ((s64)gteDQA * quotient)
    // load gteDQA
    psq_l		fr0,110(r3),1,3       // fr0 = gteDQA (regs->CP2C.p[27].sw.l) / 1.0
    lwz         r9, 112(r3)           // gteDQB (((s32 *)regs->CP2C.r)[28])
    xoris       r9, r9, 0x8000
    stw         r9, FloatBufB@sdarel(r13)
    lfd         fr1, FloatBufA@sdarel(r13)
    fsub        fr1, fr1, fr10Kmagic  // fr1 = gteDQB
    frsp        fr1, fr1              // double to single

    fmadds      fr8, fr0, fr5, fr1    // fr0 = ((s64)gteDQA * quotient) + (s64)gteDQB
    addis       r10, 0, 0x1           // gteFLAG |= 1<<16;
    addi        r9, 0, 0x7fff
    addi        r9, r9, 1             // gteFLAG |= 1<<15;
    bl          FNC_OVERFLOW1         // F (s64)gteDQB + ((s64)gteDQA * quotient)

    fctiw       fr1, fr8              // (s64)gteDQB + ((s64)gteDQA * quotient) => int
    addi        r9,  r4, 96
    stfiwx      fr1, 0, r9            // gteMAC0  (((s32 *)regs->CP2D.r)[24])

    // IR0= Lm_H[(s64)gteDQB + ((s64)gteDQA * quotient) >> 12]
    fmuls       fr8, fr8, fr14ShiftR12
    psq_st		fr8, 34(r4), 1, 3     // set gteIR0  (regs->CP2D.p[8].sw.l)

    lha         r6, 34(r4)
    addi r9, 0, 0x1000                // gteFLAG |= 1<<12;
    addi r10, 0, 0x1000               // gteFLAG |= 1<<12;
    addi r14, 0, 0
    addi r15, 0, 0x1000
    bl NC_OVERFLOW                    // Lm_H check
    stw  r7, 32(r4)                   // set gteIR0  (regs->CP2D.p[8].sw.l)
    mtspr lr, r18
    blr

.globl asm_rtps
// r3: CP2C, r4: CP2D
// r3: (m12, m11, m21, m13, m23, m22, m32, m31, 0, m33, TRX, TRY, TRZ)
// r4: (vy0, vx0, 0, vz0, vy1, vx1, 0, vz1, vy2, vx2, 0, vz2)
asm_rtps:
    mfspr r11, lr                    // save link register

    addi r8, 0, 0
    stw r8, 124(r3)                  // gteFLAG = 0 CP2C.r[31]

	// set fix value
	lfd         fr10Kmagic, kmagic@sdarel(r13)
	lfs		    fr14ShiftR12, UnitShiftR12@sdarel(r13)
	lfs         fr15ShiftR16, UnitShiftR16@sdarel(r13)
	lfs		    fr11,FncOverflowH@sdarel(r13)
    lfs		    fr12,FncOverflowL@sdarel(r13)
	ps_merge00  fr11,fr11,fr11
	ps_merge00  fr12,fr12,fr12
	ps_merge00  fr14ShiftR12,fr14ShiftR12,fr14ShiftR12  // fr14 = 1 >> 12, 1 >> 12

	addi        r5, r4, 0            // set gteVY0
	bl          asm_rtps_comn_mac

	// gteSZ0 = gteSZ1; gteSZ1 = gteSZ2; gteSZ2 = gteSZ3;
    lwz r6,68(r4)                    // gteSZ1  (regs->CP2D.p[17].w.l)
    stw r6,64(r4)
    lwz r6,72(r4)                    // gteSZ2  (regs->CP2D.p[18].w.l)
    stw r6,68(r4)
    lwz r6,76(r4)                    // gteSZ3  (regs->CP2D.p[19].w.l)
    stw r6,72(r4)

    // gteSXY0 = gteSXY1; gteSXY1 = gteSXY2;
    lwz r6,52(r4)                    // gteSXY1 (regs->CP2D.r[13])
    stw r6,48(r4)
    lwz r6,56(r4)                    // gteSXY2 (regs->CP2D.r[14])
    stw r6,52(r4)

    addi        r5, r4, 78           // set gteSZ3  (regs->CP2D.p[19].w.l)
    addi        r16, r4, 56          // set gteSY2gteSX2  (regs->CP2D.p[14].sw)
    bl          asm_rtps_comn_sxy

    bl          asm_rtps_comn_dqab

    stw r8, 124(r3)                   // set gteFLAG , CP2C.r[31]
    mtspr lr, r11
    blr


.globl asm_rtpt
// r3: CP2C, r4: CP2D
// r3: (m12, m11, m21, m13, m23, m22, m32, m31, 0, m33, TRX, TRY, TRZ)
// r4: (vy0, vx0, 0, vz0, vy1, vx1, 0, vz1, vy2, vx2, 0, vz2)
asm_rtpt:
    mfspr r11, lr                    // save link register

    addi r8, 0, 0
    stw r8, 124(r3)                  // gteFLAG = 0 CP2C.r[31]

	// set fix value
	lfd         fr10Kmagic, kmagic@sdarel(r13)
	lfs		    fr14ShiftR12, UnitShiftR12@sdarel(r13)
	lfs         fr15ShiftR16, UnitShiftR16@sdarel(r13)
	lfs		    fr11,FncOverflowH@sdarel(r13)
    lfs		    fr12,FncOverflowL@sdarel(r13)
	ps_merge00  fr11,fr11,fr11
	ps_merge00  fr12,fr12,fr12
	ps_merge00  fr14ShiftR12,fr14ShiftR12,fr14ShiftR12  // fr14 = 1 >> 12, 1 >> 12

	// gteSZ0 = gteSZ3;
	lwz r6,76(r4)                    // gteSZ3  (regs->CP2D.p[19].w.l)
    stw r6,64(r4)                    // gteSZ0  (regs->CP2D.p[16].w.l)

	addi        r5, r4, 0            // set gteVY0  (regs->CP2D.p[0].sw.h)
	bl          asm_rtps_comn_mac

    addi        r5, r4, 70           // set gteSZ1  (regs->CP2D.p[17].w.l)
    addi        r16, r4, 48          // set gteSXY0 (regs->CP2D.r[12])
    bl          asm_rtps_comn_sxy

    addi        r5, r4, 8            // set gteVY1  (regs->CP2D.p[2].sw.h)
	bl          asm_rtps_comn_mac

    addi        r5, r4, 74           // set gteSZ2  (regs->CP2D.p[18].w.l)
    addi        r16, r4, 52          // set gteSXY1 (regs->CP2D.r[13])
    bl          asm_rtps_comn_sxy

    addi        r5, r4, 16           // set gteVY2  (regs->CP2D.p[4].sw.h)
	bl          asm_rtps_comn_mac

    addi        r5, r4, 78           // set gteSZ2  (regs->CP2D.p[19].w.l)
    addi        r16, r4, 56          // set gteSXY2 (regs->CP2D.r[14])
    bl          asm_rtps_comn_sxy

    bl          asm_rtps_comn_dqab

    stw r8, 124(r3)                   // set gteFLAG , CP2C.r[31]
    mtspr lr, r11
    blr


.globl convertIntToFloat
convertIntToFloat:
    lis r6, 0x4330
    lis r7, 0x8000
    xoris r3, r3, 0x8000
    stw r6, 0x78(r1)
    stw r3, 0x7c(r1)
    lfd fr0, 0x78(r1)

    stw r6, 0x88(r1)
    stw r7, 0x8c(r1)
    //lfd fr1, 0x88(r1)
    lfd fr1, kmagic@sdarel(r13)
    fsub fr0, fr0, fr1
    stfs fr0, 0(r4)
    blr

.globl chkInt
chkInt:
    //lwz r6, 0(r3)
    //sthu r6, 0(r3)
    add r3, r3, r4
    blr

floatOverFlow:
    addi r3, 0, 1
    blr

.globl chkFloat
chkFloat:
    mtfsb0 3                      // clears FPSCR bit 3, so no overflow
    mtfsb1 25                     // sets FPSCR bit 25; overflow enabled
    lfs fr8, 0(r3)
    fctiwz fr0, fr8
    mffs fr1
    stfiwx fr1, 0, r3
    blr

.section .sdata
	.balign 32
MUL2:
	.float 2.0
MUL16:
	.float 65536.0
MAXDIV:
    .float 131071.0                  // 0x1ffff
FloatBufA:
	.int 0x43300000
FloatBufB:
	.int 0x80000000
kmagic:
    .int 0x43300000, 0x80000000
UnitShiftL12:
	.float	4096.0                   // 1 << 12 = 4096.0
UnitShiftR12:
	.float	0.000244140625           // 1 >> 12 = 1 / 4096 = 0.000244140625
FncOverflowH:
	.float	2147483647.0
FncOverflowL:
	.float	-2147483648.0
IntOverflowH:
	.int	2147483647
IntOverflowL:
	.int	-2147483648
UnitShiftR16:
	.float	0.00001526               // 1 >> 16 = 1 / 65536 = 0.00001526
debugArea:
    .int 0
TestOverflowH:
	.float	2147483647.0
DivOverflow:
	.int	0x1ffff

