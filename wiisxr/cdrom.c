/***************************************************************************
 *   Copyright (C) 2007 Ryan Schultz, PCSX-df Team, PCSX team              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

/*
* Handles all CD-ROM registers and functions.
*/

#include "cdrom.h"
#include "ppf.h"
#include "psxdma.h"
/* logging */
#if 0
#define CDR_LOG SysPrintf
#else
#define CDR_LOG(...)
#endif
#if 0
#define CDR_LOG_I SysPrintf
#else
#define CDR_LOG_I(...)
#endif
#if 0
#define CDR_LOG_IO SysPrintf
#else
#define CDR_LOG_IO(...)
#endif
//#define CDR_LOG_CMD_IRQ

cdrStruct cdr;
static unsigned char *pTransfer;
static s16 read_buf[CD_FRAMESIZE_RAW/2];

/* CD-ROM magic numbers */
#define CdlSync        0  /* nocash documentation : "Uh, actually, returns error code 40h = Invalid Command...?" */
#define CdlNop         1
#define CdlSetloc      2
#define CdlPlay        3
#define CdlForward     4
#define CdlBackward    5
#define CdlReadN       6
#define CdlStandby     7
#define CdlStop        8
#define CdlPause       9
#define CdlInit        10 // 0xa
#define CdlMute        11 // 0xb
#define CdlDemute      12 // 0xc
#define CdlSetfilter   13 // 0xd
#define CdlSetmode     14 // 0xe
#define CdlGetmode     15 // 0xf
#define CdlGetlocL     16 // 0x10
#define CdlGetlocP     17 // 0x11
#define CdlReadT       18 // 0x12
#define CdlGetTN       19 // 0x13
#define CdlGetTD       20 // 0x14
#define CdlSeekL       21 // 0x15
#define CdlSeekP       22 // 0x16
#define CdlSetclock    23 // 0x17
#define CdlGetclock    24 // 0x18
#define CdlTest        25 // 0x19
#define CdlID          26 // 0x1a
#define CdlReadS       27 // 0x1b
#define CdlReset       28 // 0x1c
#define CdlGetQ        29 // 0x1d
#define CdlReadToc     30 // 0x1e

char *CmdName[0x100]= {
    "CdlSync",     "CdlNop",       "CdlSetloc",  "CdlPlay",
    "CdlForward",  "CdlBackward",  "CdlReadN",   "CdlStandby",
    "CdlStop",     "CdlPause",     "CdlInit",    "CdlMute",
    "CdlDemute",   "CdlSetfilter", "CdlSetmode", "CdlGetmode",
    "CdlGetlocL",  "CdlGetlocP",   "CdlReadT",   "CdlGetTN",
    "CdlGetTD",    "CdlSeekL",     "CdlSeekP",   "CdlSetclock",
    "CdlGetclock", "CdlTest",      "CdlID",      "CdlReadS",
    "CdlReset",    NULL,           "CDlReadToc", NULL
};

unsigned char Test04[] = { 0 };
unsigned char Test05[] = { 0 };
unsigned char Test20[] = { 0x98, 0x06, 0x10, 0xC3 };
unsigned char Test22[] = { 0x66, 0x6F, 0x72, 0x20, 0x45, 0x75, 0x72, 0x6F };
unsigned char Test23[] = { 0x43, 0x58, 0x44, 0x32, 0x39 ,0x34, 0x30, 0x51 };

// cdr.Stat:
#define NoIntr		0
#define DataReady	1
#define Complete	2
#define Acknowledge	3
#define DataEnd		4
#define DiskError	5

/* Modes flags */
#define MODE_SPEED       (1<<7) // 0x80
#define MODE_STRSND      (1<<6) // 0x40 ADPCM on/off
#define MODE_SIZE_2340   (1<<5) // 0x20
#define MODE_SIZE_2328   (1<<4) // 0x10
#define MODE_SIZE_2048   (0<<4) // 0x00
#define MODE_SF          (1<<3) // 0x08 channel on/off
#define MODE_REPORT      (1<<2) // 0x04
#define MODE_AUTOPAUSE   (1<<1) // 0x02
#define MODE_CDDA        (1<<0) // 0x01

/* Status flags */
#define STATUS_PLAY      (1<<7) // 0x80
#define STATUS_SEEK      (1<<6) // 0x40
#define STATUS_READ      (1<<5) // 0x20
#define STATUS_SHELLOPEN (1<<4) // 0x10
#define STATUS_UNKNOWN3  (1<<3) // 0x08
#define STATUS_UNKNOWN2  (1<<2) // 0x04
#define STATUS_ROTATING  (1<<1) // 0x02
#define STATUS_ERROR     (1<<0) // 0x01

/* Errors */
#define ERROR_NOTREADY   (1<<7) // 0x80
#define ERROR_INVALIDCMD (1<<6) // 0x40
#define ERROR_INVALIDARG (1<<5) // 0x20

// 1x = 75 sectors per second
// PSXCLK = 1 sec in the ps
// so (PSXCLK / 75) / BIAS = cdr read time (linuzappz)
#define cdReadTime ((PSXCLK / 75) / BIAS)

enum drive_state {
	DRIVESTATE_STANDBY = 0,
	DRIVESTATE_LID_OPEN,
	DRIVESTATE_RESCAN_CD,
	DRIVESTATE_PREPARE_CD,
	DRIVESTATE_STOPPED,
};

// for cdr.Seeked
enum seeked_state {
	SEEK_PENDING = 0,
	SEEK_DONE = 1,
};

static struct CdrStat stat;

static inline unsigned int msf2sec(const u8 *msf) {
	return ((msf[0] * 60 + msf[1]) * 75) + msf[2];
}

// for that weird psemu API..
static inline unsigned int fsm2sec(const u8 *msf) {
	return ((msf[2] * 60 + msf[1]) * 75) + msf[0];
}

static inline void sec2msf(unsigned int s, u8 *msf) {
	msf[0] = s / 75 / 60;
	s = s - msf[0] * 75 * 60;
	msf[1] = s / 75;
	s = s - msf[1] * 75;
	msf[2] = s;
}

// cdrInterrupt
#define CDR_INT(eCycle) { \
	psxRegs.interrupt |= (1 << PSXINT_CDR); \
	psxRegs.intCycle[PSXINT_CDR].cycle = eCycle; \
	psxRegs.intCycle[PSXINT_CDR].sCycle = psxRegs.cycle; \
}

// cdrReadInterrupt
#define CDREAD_INT(eCycle) { \
	psxRegs.interrupt |= (1 << PSXINT_CDREAD); \
	psxRegs.intCycle[PSXINT_CDREAD].cycle = eCycle; \
	psxRegs.intCycle[PSXINT_CDREAD].sCycle = psxRegs.cycle; \
}

// cdrLidSeekInterrupt
#define CDRLID_INT(eCycle) { \
	psxRegs.interrupt |= (1 << PSXINT_CDRLID); \
	psxRegs.intCycle[PSXINT_CDRLID].cycle = eCycle; \
	psxRegs.intCycle[PSXINT_CDRLID].sCycle = psxRegs.cycle; \
}

// cdrPlayInterrupt
#define CDRMISC_INT(eCycle) { \
	psxRegs.interrupt |= (1 << PSXINT_CDRPLAY); \
	psxRegs.intCycle[PSXINT_CDRPLAY].cycle = eCycle; \
	psxRegs.intCycle[PSXINT_CDRPLAY].sCycle = psxRegs.cycle; \
}

#define StopReading() { \
	if (cdr.Reading) { \
		cdr.Reading = 0; \
		psxRegs.interrupt &= ~(1 << PSXINT_CDREAD); \
	} \
	cdr.StatP &= ~(STATUS_READ|STATUS_SEEK);\
}

#define StopCdda() { \
	if (cdr.Play) { \
		if (!Config.Cdda) CDR_stop(); \
		cdr.StatP &= ~STATUS_PLAY; \
		cdr.Play = FALSE; \
		cdr.FastForward = 0; \
		cdr.FastBackward = 0; \
		/*SPU_registerCallback( SPUirq );*/ \
	} \
}

#define SetResultSize(size) { \
	cdr.ResultP = 0; \
	cdr.ResultC = size; \
	cdr.ResultReady = 1; \
}

static void setIrq(void)
{
	if (cdr.Stat & cdr.Reg2)
		psxHu32ref(0x1070) |= SWAP32((u32)0x4);
}

static void adjustTransferIndex(void)
{
	unsigned int bufSize = 0;

	switch (cdr.Mode & (MODE_SIZE_2340|MODE_SIZE_2328)) {
		case MODE_SIZE_2340: bufSize = 2340; break;
		case MODE_SIZE_2328: bufSize = 12 + 2328; break;
		default:
		case MODE_SIZE_2048: bufSize = 12 + 2048; break;
	}

	if (cdr.transferIndex >= bufSize)
		cdr.transferIndex -= bufSize;
}
// timing used in this function was taken from tests on real hardware
// (yes it's slow, but you probably don't want to modify it)
void cdrLidSeekInterrupt()
{
	switch (cdr.DriveState) {
	default:
	    #ifdef DISP_DEBUG
        PRINT_LOG("cdrLidSeekInterrupt=default ");
        #endif // DISP_DEBUG
	case DRIVESTATE_STANDBY:
	    #ifdef DISP_DEBUG
        PRINT_LOG1("cdrLidSeekInterrupt=DRIVESTATE_STANDBY: %x ", stat.Status);
        #endif // DISP_DEBUG
		cdr.StatP &= ~STATUS_SEEK;

		if (CDR_getStatus(&stat) == -1)
			return;

        #ifdef DISP_DEBUG
        PRINT_LOG1("cdrLidSeekInterrupt=DRIVESTATE_STANDBY2: %x ", stat.Status);
        #endif // DISP_DEBUG
		if (stat.Status & STATUS_SHELLOPEN)
		{
			StopCdda();
			cdr.DriveState = DRIVESTATE_LID_OPEN;
			CDRLID_INT(0x800);
		}
		break;

	case DRIVESTATE_LID_OPEN:
	    #ifdef DISP_DEBUG
        PRINT_LOG1("cdrLidSeekInterrupt=DRIVESTATE_LID_OPEN: %x ", cdr.StatP);
        #endif // DISP_DEBUG
		if (CDR_getStatus(&stat) == -1)
			stat.Status &= ~STATUS_SHELLOPEN;

		// 02, 12, 10
		if (!(cdr.StatP & STATUS_SHELLOPEN)) {
			StopReading();
			cdr.StatP |= STATUS_SHELLOPEN;

			// could generate error irq here, but real hardware
			// only sometimes does that
			// (not done when lots of commands are sent?)

			CDRLID_INT(cdReadTime * 30);
			break;
		}
		else if (cdr.StatP & STATUS_ROTATING) {
			cdr.StatP &= ~STATUS_ROTATING;
		}
		else if (!(stat.Status & STATUS_SHELLOPEN)) {
			// closed now
			CheckCdrom();

			// cdr.StatP STATUS_SHELLOPEN is "sticky"
			// and is only cleared by CdlNop

			cdr.DriveState = DRIVESTATE_RESCAN_CD;
			CDRLID_INT(cdReadTime * 105);
			break;
		}

		// recheck for close
		CDRLID_INT(cdReadTime * 3);
		break;

	case DRIVESTATE_RESCAN_CD:
	    #ifdef DISP_DEBUG
        PRINT_LOG1("cdrLidSeekInterrupt=DRIVESTATE_RESCAN_CD: %x ", cdr.StatP);
        #endif // DISP_DEBUG
		cdr.StatP |= STATUS_ROTATING;
		cdr.DriveState = DRIVESTATE_PREPARE_CD;

		// this is very long on real hardware, over 6 seconds
		// make it a bit faster here...
		CDRLID_INT(cdReadTime * 150);
		break;

	case DRIVESTATE_PREPARE_CD:
	    #ifdef DISP_DEBUG
        PRINT_LOG1("cdrLidSeekInterrupt=DRIVESTATE_PREPARE_CD: %x ", cdr.StatP);
        #endif // DISP_DEBUG
		cdr.StatP |= STATUS_SEEK;

		cdr.DriveState = DRIVESTATE_STANDBY;
		CDRLID_INT(cdReadTime * 26);
		break;
	}
}

static void Find_CurTrack(const u8 *time)
{
	int current, sect;

	current = msf2sec(time);

	for (cdr.CurTrack = 1; cdr.CurTrack < cdr.ResultTN[1]; cdr.CurTrack++) {
		CDR_getTD(cdr.CurTrack + 1, cdr.ResultTD);
		sect = fsm2sec(cdr.ResultTD);
		if (sect - current >= 150)
			break;
	}
}

static void generate_subq(const u8 *time)
{
	unsigned char start[3], next[3];
	unsigned int this_s, start_s, next_s, pregap;
	int relative_s;

	CDR_getTD(cdr.CurTrack, start);
	if (cdr.CurTrack + 1 <= cdr.ResultTN[1]) {
		pregap = 150;
		CDR_getTD(cdr.CurTrack + 1, next);
	}
	else {
		// last track - cd size
		pregap = 0;
		next[0] = cdr.SetSectorEnd[2];
		next[1] = cdr.SetSectorEnd[1];
		next[2] = cdr.SetSectorEnd[0];
	}

	this_s = msf2sec(time);
	start_s = fsm2sec(start);
	next_s = fsm2sec(next);

	cdr.TrackChanged = FALSE;

	if (next_s - this_s < pregap) {
		cdr.TrackChanged = TRUE;
		cdr.CurTrack++;
		start_s = next_s;
	}

	cdr.subq.Index = 1;

	relative_s = this_s - start_s;
	if (relative_s < 0) {
		cdr.subq.Index = 0;
		relative_s = -relative_s;
	}
	sec2msf(relative_s, cdr.subq.Relative);

	cdr.subq.Track = itob(cdr.CurTrack);
	cdr.subq.Relative[0] = itob(cdr.subq.Relative[0]);
	cdr.subq.Relative[1] = itob(cdr.subq.Relative[1]);
	cdr.subq.Relative[2] = itob(cdr.subq.Relative[2]);
	cdr.subq.Absolute[0] = itob(time[0]);
	cdr.subq.Absolute[1] = itob(time[1]);
	cdr.subq.Absolute[2] = itob(time[2]);
}

// ReadTrack=========================
static void ReadTrack(const u8 *time) {
	unsigned char tmp[3];
	struct SubQ *subq;
	u16 crc;

	tmp[0] = itob(time[0]);
	tmp[1] = itob(time[1]);
	tmp[2] = itob(time[2]);

	//if (memcmp(cdr.Prev, tmp, 3) == 0)
	//	return;
	if (cdr.Prev[0] == tmp[0] && cdr.Prev[1] == tmp[1] && cdr.Prev[2] == tmp[2])
    {
        return;
    }

	//CDR_LOG("ReadTrack *** %02x:%02x:%02x\n", tmp[0], tmp[1], tmp[2]);

	cdr.RErr = CDR_readTrack(tmp);
	//memcpy(cdr.Prev, tmp, 3);
	cdr.Prev[0] = tmp[0];
	cdr.Prev[1] = tmp[1];
	cdr.Prev[2] = tmp[2];

	//if (CheckSBI(time))
	//	return;

	/*subq = (struct SubQ *)CDR_getBufferSub();
	if (subq != NULL && cdr.CurTrack == 1) {
		crc = calcCrc((u8 *)subq + 12, 10);
		if (crc == (((u16)subq->CRC[0] << 8) | subq->CRC[1])) {
			cdr.subq.Track = subq->TrackNumber;
			cdr.subq.Index = subq->IndexNumber;
			memcpy(cdr.subq.Relative, subq->TrackRelativeAddress, 3);
			memcpy(cdr.subq.Absolute, subq->AbsoluteAddress, 3);
		}
		else {
			CDR_LOG_I("subq bad crc @%02x:%02x:%02x\n",
				tmp[0], tmp[1], tmp[2]);
		}
	}
	else {
		generate_subq(time);
	}*/

	//CDR_LOG(" -> %02x,%02x %02x:%02x:%02x %02x:%02x:%02x\n",
	//	cdr.subq.Track, cdr.subq.Index,
	//	cdr.subq.Relative[0], cdr.subq.Relative[1], cdr.subq.Relative[2],
	//	cdr.subq.Absolute[0], cdr.subq.Absolute[1], cdr.subq.Absolute[2]);
}

// AddIrqQueue=====================
static void AddIrqQueue(unsigned short irq, unsigned long ecycle) {
	if (cdr.Irq != 0) {
		if (irq == cdr.Irq || irq + 0x100 == cdr.Irq) {
			cdr.IrqRepeated = 1;
			CDR_INT(ecycle);
			return;
		}

		//CDR_LOG_I("cdr: override cmd %02x -> %02x\n", cdr.Irq, irq);
	}

	cdr.Irq = irq;
	cdr.eCycle = ecycle;

	CDR_INT(ecycle);
}

static void cdrPlayInterrupt_Autopause()
{
	u32 abs_lev_max = 0;
	bool abs_lev_chselect;
	u32 i;

	if ((cdr.Mode & MODE_AUTOPAUSE) && cdr.TrackChanged) {
		//CDR_LOG( "CDDA STOP\n" );

		// Magic the Gathering
		// - looping territory cdda

		// ...?
		//cdr.ResultReady = 1;
		//cdr.Stat = DataReady;
		cdr.Stat = DataEnd;
		setIrq();

		StopCdda();
	}
	else if (((cdr.Mode & MODE_REPORT) || cdr.FastForward || cdr.FastBackward)) {
		CDR_readCDDA(cdr.SetSectorPlay[0], cdr.SetSectorPlay[1], cdr.SetSectorPlay[2], (u8 *)read_buf);
		cdr.Result[0] = cdr.StatP;
		cdr.Result[1] = cdr.subq.Track;
		cdr.Result[2] = cdr.subq.Index;

		abs_lev_chselect = cdr.subq.Absolute[1] & 0x01;

		/* 8 is a hack. For accuracy, it should be 588. */
		for (i = 0; i < 8; i++)
		{
			abs_lev_max = MAX_VALUE(abs_lev_max, abs(read_buf[i * 2 + abs_lev_chselect]));
		}
		abs_lev_max = MIN_VALUE(abs_lev_max, 32767);
		abs_lev_max |= abs_lev_chselect << 15;

		if (cdr.subq.Absolute[2] & 0x10) {
			cdr.Result[3] = cdr.subq.Relative[0];
			cdr.Result[4] = cdr.subq.Relative[1] | 0x80;
			cdr.Result[5] = cdr.subq.Relative[2];
		}
		else {
			cdr.Result[3] = cdr.subq.Absolute[0];
			cdr.Result[4] = cdr.subq.Absolute[1];
			cdr.Result[5] = cdr.subq.Absolute[2];
		}

		cdr.Result[6] = abs_lev_max >> 0;
		cdr.Result[7] = abs_lev_max >> 8;

		// Rayman: Logo freeze (resultready + dataready)
		cdr.ResultReady = 1;
		cdr.Stat = DataReady;

		SetResultSize(8);
		setIrq();
	}
}

// also handles seek
void cdrPlayInterrupt()
{
	if (cdr.Seeked == SEEK_PENDING) {
		if (cdr.Stat) {
			CDR_LOG_I("cdrom: seek stat hack\n");
			CDRMISC_INT(0x800);
			return;
		}
		SetResultSize(1);
		cdr.StatP |= STATUS_ROTATING;
		cdr.StatP &= ~STATUS_SEEK;
		cdr.Result[0] = cdr.StatP;
		cdr.Seeked = SEEK_DONE;
		if (cdr.Irq == 0) {
			cdr.Stat = Complete;
			setIrq();
		}

		if (cdr.SetlocPending) {
			//memcpy(cdr.SetSectorPlay, cdr.SetSector, 4);
			*((u32*)cdr.SetSectorPlay) = *((u32*)cdr.SetSector);
			cdr.SetlocPending = 0;
			cdr.m_locationChanged = TRUE;
		}
		Find_CurTrack(cdr.SetSectorPlay);
		ReadTrack(cdr.SetSectorPlay);
		cdr.TrackChanged = FALSE;
	}

	if (!cdr.Play) return;

	//CDR_LOG( "CDDA - %d:%d:%d\n",
		//cdr.SetSectorPlay[0], cdr.SetSectorPlay[1], cdr.SetSectorPlay[2] );

	if (memcmp(cdr.SetSectorPlay, cdr.SetSectorEnd, 3) == 0) {
		StopCdda();
		cdr.TrackChanged = TRUE;
	}

	if (!cdr.Irq && !cdr.Stat && (cdr.Mode & (MODE_AUTOPAUSE|MODE_REPORT)))
		cdrPlayInterrupt_Autopause();

	if (CDR_readCDDA && !cdr.Muted && (cdr.Mode & MODE_REPORT) && !Config.Cdda) {
		cdrAttenuate(read_buf, CD_FRAMESIZE_RAW / 4, 1);
		if (SPU_playCDDAchannel)
			SPU_playCDDAchannel(read_buf, CD_FRAMESIZE_RAW);
	}

	cdr.SetSectorPlay[2]++;
	if (cdr.SetSectorPlay[2] == 75) {
		cdr.SetSectorPlay[2] = 0;
		cdr.SetSectorPlay[1]++;
		if (cdr.SetSectorPlay[1] == 60) {
			cdr.SetSectorPlay[1] = 0;
			cdr.SetSectorPlay[0]++;
		}
	}

	if (cdr.m_locationChanged)
	{
		CDRMISC_INT(cdReadTime * 30);
		cdr.m_locationChanged = FALSE;
	}
	else
	{
		CDRMISC_INT(cdReadTime);
	}

	// update for CdlGetlocP/autopause
	generate_subq(cdr.SetSectorPlay);
}

void cdrInterrupt() {
	u16 Irq = cdr.Irq;
	int no_busy_error = 0;
	int start_rotating = 0;
	int error = 0;
	int delay;
	unsigned int seekTime = 0;

	// Reschedule IRQ
	if (cdr.Stat) {
		CDR_LOG_I("cdrom: stat hack: %02x %x\n", cdr.Irq, cdr.Stat);
		CDR_INT(0x800);
		return;
	}

	cdr.Ctrl &= ~0x80;

	// default response
	SetResultSize(1);
	cdr.Result[0] = cdr.StatP;
	cdr.Stat = Acknowledge;

	if (cdr.IrqRepeated) {
		cdr.IrqRepeated = 0;
		if (cdr.eCycle > psxRegs.cycle) {
			CDR_INT(cdr.eCycle);
			goto finish;
		}
	}

	cdr.Irq = 0;
	struct SubQ *subq;
	u16 crc;

	switch (Irq) {
		case CdlNop:
			if (cdr.DriveState != DRIVESTATE_LID_OPEN)
				cdr.StatP &= ~STATUS_SHELLOPEN;
			no_busy_error = 1;
			break;

		case CdlSetloc:
			break;

		do_CdlPlay:
		case CdlPlay:
			StopCdda();
			if (cdr.Seeked == SEEK_PENDING) {
				// XXX: wrong, should seek instead..
				cdr.Seeked = SEEK_DONE;
			}

			cdr.FastBackward = 0;
			cdr.FastForward = 0;

			if (cdr.SetlocPending) {
				//memcpy(cdr.SetSectorPlay, cdr.SetSector, 4);
				*((u32*)cdr.SetSectorPlay) = *((u32*)cdr.SetSector);
				cdr.SetlocPending = 0;
				cdr.m_locationChanged = TRUE;
			}

			// BIOS CD Player
			// - Pause player, hit Track 01/02/../xx (Setloc issued!!)

			if (cdr.ParamC == 0 || cdr.Param[0] == 0) {
				//CDR_LOG("PLAY Resume @ %d:%d:%d\n",
				//	cdr.SetSectorPlay[0], cdr.SetSectorPlay[1], cdr.SetSectorPlay[2]);
			}
			else
			{
				int track = btoi( cdr.Param[0] );

				if (track <= cdr.ResultTN[1])
					cdr.CurTrack = track;

				//CDR_LOG("PLAY track %d\n", cdr.CurTrack);

				if (CDR_getTD((u8)cdr.CurTrack, cdr.ResultTD) != -1) {
					cdr.SetSectorPlay[0] = cdr.ResultTD[2];
					cdr.SetSectorPlay[1] = cdr.ResultTD[1];
					cdr.SetSectorPlay[2] = cdr.ResultTD[0];
				}
			}

			/*
			Rayman: detect track changes
			- fixes logo freeze

			Twisted Metal 2: skip PREGAP + starting accurate SubQ
			- plays tracks without retry play

			Wild 9: skip PREGAP + starting accurate SubQ
			- plays tracks without retry play
			*/
			Find_CurTrack(cdr.SetSectorPlay);
			ReadTrack(cdr.SetSectorPlay);
			cdr.TrackChanged = FALSE;

			if (!Config.Cdda)
				CDR_play(cdr.SetSectorPlay);

			// Vib Ribbon: gameplay checks flag
			cdr.StatP &= ~STATUS_SEEK;
			cdr.Result[0] = cdr.StatP;

			cdr.StatP |= STATUS_PLAY;

			// BIOS player - set flag again
			cdr.Play = TRUE;

			CDRMISC_INT( cdReadTime );
			start_rotating = 1;
			break;

		case CdlForward:
			// TODO: error 80 if stopped
			cdr.Stat = Complete;
			// GameShark CD Player: Calls 2x + Play 2x
			if( cdr.FastForward == 0 ) cdr.FastForward = 2;
			else cdr.FastForward++;

			cdr.FastBackward = 0;
			break;

		case CdlBackward:
			cdr.Stat = Complete;

			// GameShark CD Player: Calls 2x + Play 2x
			if( cdr.FastBackward == 0 ) cdr.FastBackward = 2;
			else cdr.FastBackward++;

			cdr.FastForward = 0;
			break;

		case CdlStandby:
			if (cdr.DriveState != DRIVESTATE_STOPPED) {
				error = ERROR_INVALIDARG;
				goto set_error;
			}
			AddIrqQueue(CdlStandby + 0x100, cdReadTime * 125 / 2);
			start_rotating = 1;
			break;

		case CdlStandby + 0x100:
			cdr.Stat = Complete;
			break;

		case CdlStop:
			if (cdr.Play) {
				// grab time for current track
				CDR_getTD((u8)(cdr.CurTrack), cdr.ResultTD);

				cdr.SetSectorPlay[0] = cdr.ResultTD[2];
				cdr.SetSectorPlay[1] = cdr.ResultTD[1];
				cdr.SetSectorPlay[2] = cdr.ResultTD[0];
			}

			StopCdda();
			StopReading();

			delay = 0x800;
			if (cdr.DriveState == DRIVESTATE_STANDBY)
				delay = cdReadTime * 30 / 2;

			cdr.DriveState = DRIVESTATE_STOPPED;
			AddIrqQueue(CdlStop + 0x100, delay);
			break;

		case CdlStop + 0x100:
			cdr.StatP &= ~STATUS_ROTATING;
			cdr.Result[0] = cdr.StatP;
			cdr.Stat = Complete;
			break;

		case CdlPause:
			/*
			Gundam Battle Assault 2: much slower (*)
			- Fixes boot, gameplay

			Hokuto no Ken 2: slower
			- Fixes intro + subtitles

			InuYasha - Feudal Fairy Tale: slower
			- Fixes battles
			*/
			/* Gameblabla - Tightening the timings (as taken from Duckstation).
			 * The timings from Duckstation are based upon hardware tests.
			 * Mednafen's timing don't work for Gundam Battle Assault 2 in PAL/50hz mode,
			 * seems to be timing sensitive as it can depend on the CPU's clock speed.
			 *
			 * We will need to get around this for Bedlam/Rise 2 later...
			 * */
			if (cdr.DriveState != DRIVESTATE_STANDBY)
			{
				delay = 7000;
			}
			else
			{
				delay = (((cdr.Mode & MODE_SPEED) ? 2 : 1) * (1000000));
				CDRMISC_INT((cdr.Mode & MODE_SPEED) ? cdReadTime / 2 : cdReadTime);
			}
			AddIrqQueue(CdlPause + 0x100, delay >> 2);
			//AddIrqQueue(CdlPause + 0x100, cdReadTime * 3);
			cdr.Ctrl |= 0x80;
			break;

		case CdlPause + 0x100:
			cdr.StatP &= ~STATUS_READ;
			cdr.Result[0] = cdr.StatP;
			cdr.Stat = Complete;
			break;

		case CdlInit:
            cdr.Muted = FALSE;
			cdr.Mode = 0x20; /* Needed for This is Football 2, Pooh's Party and possibly others. */
			//AddIrqQueue(CdlReset + 0x100, 4100000);
			AddIrqQueue(CdlInit + 0x100, cdReadTime * 6);
			no_busy_error = 1;
			start_rotating = 1;
			break;

		case CdlInit + 0x100:
			cdr.Stat = Complete;
			break;

		case CdlMute:
			cdr.Muted = TRUE;
			break;

		case CdlDemute:
			cdr.Muted = FALSE;
			break;

		case CdlSetfilter:
			cdr.File = cdr.Param[0];
			cdr.Channel = cdr.Param[1];
			break;

		case CdlSetmode:
			no_busy_error = 1;
			break;

		case CdlGetmode:
			/* Gameblabla : According to mednafen, Result size should be 5 and done this way. */
			SetResultSize(5);
			cdr.Result[1] = cdr.Mode;
			cdr.Result[2] = 0;
			cdr.Result[3] = cdr.File;
			cdr.Result[4] = cdr.Channel;
			no_busy_error = 1;
			break;

		case CdlGetlocL:
			SetResultSize(8);
			//memcpy(cdr.Result, cdr.Transfer, 8);
			*((unsigned long long *)cdr.Result) = *((unsigned long long *)cdr.Transfer);
			break;

		case CdlGetlocP:
			SetResultSize(8);

			subq = (struct SubQ *)CDR_getBufferSub();
            if (subq != NULL && cdr.CurTrack == 1) {
                crc = calcCrc((u8 *)subq + 12, 10);
                if (crc == (((u16)subq->CRC[0] << 8) | subq->CRC[1])) {
                    cdr.subq.Track = subq->TrackNumber;
                    cdr.subq.Index = subq->IndexNumber;
                    memcpy(cdr.subq.Relative, subq->TrackRelativeAddress, 3);
                    memcpy(cdr.subq.Absolute, subq->AbsoluteAddress, 3);
                }
                else {
                    CDR_LOG_I("subq bad crc @%02x:%02x:%02x\n",
                        tmp[0], tmp[1], tmp[2]);
                }
            }
            else {
                generate_subq(cdr.SetSectorPlay);
            }

			//memcpy(&cdr.Result, &cdr.subq, 8);
			*((unsigned long long *)&cdr.Result) = *((unsigned long long *)&cdr.subq);

			//if (!cdr.Play && CheckSBI(cdr.Result+5))
			//	memset(cdr.Result+2, 0, 6);
			if (!cdr.Play && !cdr.Reading)
				cdr.Result[1] = 0; // HACK?
			break;

		case CdlReadT: // SetSession?
			// really long
			AddIrqQueue(CdlReadT + 0x100, cdReadTime * 290 / 4);
			start_rotating = 1;
			break;

		case CdlReadT + 0x100:
			cdr.Stat = Complete;
			break;

		case CdlGetTN:
			SetResultSize(3);
			if (CDR_getTN(cdr.ResultTN) == -1) {
				cdr.Stat = DiskError;
				cdr.Result[0] |= STATUS_ERROR;
			} else {
				cdr.Stat = Acknowledge;
				cdr.Result[1] = itob(cdr.ResultTN[0]);
				cdr.Result[2] = itob(cdr.ResultTN[1]);
			}
			break;

		case CdlGetTD:
			cdr.Track = btoi(cdr.Param[0]);
			SetResultSize(4);
			if (CDR_getTD(cdr.Track, cdr.ResultTD) == -1) {
				cdr.Stat = DiskError;
				cdr.Result[0] |= STATUS_ERROR;
			} else {
				cdr.Stat = Acknowledge;
				cdr.Result[0] = cdr.StatP;
				cdr.Result[1] = itob(cdr.ResultTD[2]);
				cdr.Result[2] = itob(cdr.ResultTD[1]);
				/* According to Nocash's documentation, the function doesn't care about ff.
				 * This can be seen also in Mednafen's implementation. */
				//cdr.Result[3] = itob(cdr.ResultTD[0]);
			}
			break;

		case CdlSeekL:
		case CdlSeekP:
			StopCdda();
			StopReading();
			cdr.StatP |= STATUS_SEEK;

			/*
			Crusaders of Might and Magic = 0.5x-4x
			- fix cutscene speech start

			Eggs of Steel = 2x-?
			- fix new game

			Medievil = ?-4x
			- fix cutscene speech

			Rockman X5 = 0.5-4x
			- fix capcom logo
			*/
			CDRMISC_INT(cdr.Seeked == SEEK_DONE ? 0x800 : cdReadTime * 4);
			cdr.Seeked = SEEK_PENDING;
			start_rotating = 1;
			break;

		case CdlTest:
			switch (cdr.Param[0]) {
				case 0x20: // System Controller ROM Version
					SetResultSize(4);
					memcpy(cdr.Result, Test20, 4);
					break;
				case 0x22:
					SetResultSize(8);
					memcpy(cdr.Result, Test22, 4);
					break;
				case 0x23: case 0x24:
					SetResultSize(8);
					memcpy(cdr.Result, Test23, 4);
					break;
			}
			no_busy_error = 1;
			break;

		case CdlID:
			//AddIrqQueue(CdlID + 0x100, 20480);
			AddIrqQueue(CdlID + 0x100, 0x800);
			break;

		case CdlID + 0x100:
			SetResultSize(8);
			cdr.Result[0] = cdr.StatP;
			cdr.Result[1] = 0;
			cdr.Result[2] = 0;
			cdr.Result[3] = 0;

			// 0x10 - audio | 0x40 - disk missing | 0x80 - unlicensed
			if (CDR_getStatus(&stat) == -1 || stat.Type == 0 || stat.Type == 0xff) {
				cdr.Result[1] = 0xc0;
			}
			else {
				if (stat.Type == 2)
					cdr.Result[1] |= 0x10;
				if (CdromId[0] == '\0')
					cdr.Result[1] |= 0x80;
			}
			cdr.Result[0] |= (cdr.Result[1] >> 4) & 0x08;

			/* This adds the string "PCSX" in Playstation bios boot screen */
			memcpy((char *)&cdr.Result[4], "PCSX", 4);
			cdr.Stat = Complete;
			break;

		case CdlReset:
			// yes, it really sets STATUS_SHELLOPEN
			cdr.StatP |= STATUS_SHELLOPEN;
			cdr.DriveState = DRIVESTATE_RESCAN_CD;
			CDRLID_INT(20480);
			no_busy_error = 1;
			start_rotating = 1;
			break;

		case CdlGetQ:
			no_busy_error = 1;
			break;

		case CdlReadToc:
			AddIrqQueue(CdlReadToc + 0x100, cdReadTime * 180 / 4);
			no_busy_error = 1;
			start_rotating = 1;
			break;

		case CdlReadToc + 0x100:
			cdr.Stat = Complete;
			no_busy_error = 1;
			break;

		case CdlReadN:
		case CdlReadS:
			if (cdr.SetlocPending) {
				seekTime = abs(msf2sec(cdr.SetSectorPlay) - msf2sec(cdr.SetSector)) * (cdReadTime / 200);
                /*
				* Gameblabla :
				* It was originally set to 1000000 for Driver, however it is not high enough for Worms Pinball
				* and was unreliable for that game.
				* I also tested it against Mednafen and Driver's titlescreen music starts 25 frames later, not immediatly.
				*
				* Obviously, this isn't perfect but right now, it should be a bit better.
				* Games to test this against if you change that setting :
				* - Driver (titlescreen music delay and retry mission)
				* - Worms Pinball (Will either not boot or crash in the memory card screen)
				* - Viewpoint (short pauses if the delay in the ingame music is too long)
				*
				* It seems that 3386880 * 5 is too much for Driver's titlescreen and it starts skipping.
				* However, 1000000 is not enough for Worms Pinball to reliably boot.
				*/
				if(seekTime > 3386880 * 2) seekTime = 3386880 * 2;
				//if(seekTime > 1000000) seekTime = 1000000;
				//memcpy(cdr.SetSectorPlay, cdr.SetSector, 4);
				*((u32 *)cdr.SetSectorPlay) = *((u32 *)cdr.SetSector);
				cdr.SetlocPending = 0;
				cdr.m_locationChanged = TRUE;
			}
			Find_CurTrack(cdr.SetSectorPlay);

			if ((cdr.Mode & MODE_CDDA) && cdr.CurTrack > 1)
				// Read* acts as play for cdda tracks in cdda mode
				goto do_CdlPlay;

			cdr.Reading = 1;
			cdr.FirstSector = 1;

			// Fighting Force 2 - update subq time immediately
			// - fixes new game
			ReadTrack(cdr.SetSectorPlay);


			// Crusaders of Might and Magic - update getlocl now
			// - fixes cutscene speech
			{
				u8 *buf = CDR_getBuffer();
				if (buf != NULL)
					//memcpy(cdr.Transfer, buf, 8);
					*((unsigned long long *)cdr.Transfer) = *((unsigned long long *)buf);
			}

			/*
			Duke Nukem: Land of the Babes - seek then delay read for one frame
			- fixes cutscenes
			C-12 - Final Resistance - doesn't like seek
			*/

			/*
				By nicolasnoble from PCSX Redux :
				"It LOOKS like this logic is wrong, therefore disabling it with `&& false` for now.
				For "PoPoLoCrois Monogatari II", the game logic will soft lock and will never issue GetLocP to detect
				the end of its XA streams, as it seems to assume ReadS will not return a status byte with the SEEK
				flag set. I think the reasonning is that since it's invalid to call GetLocP while seeking, the game
				tries to protect itself against errors by preventing from issuing a GetLocP while it knows the
				last status was "seek". But this makes the logic just softlock as it'll never get a notification
				about the fact the drive is done seeking and the read actually started.
				In other words, this state machine here is probably wrong in assuming the response to ReadS/ReadN is
				done right away. It's rather when it's done seeking, and the read has actually started. This probably
				requires a bit more work to make sure seek delays are processed properly.
				Checked with a few games, this seems to work fine."
				Gameblabla additional notes :
				This still needs the "+ seekTime" that PCSX Redux doesn't have for the Driver "retry" mission error.
			*/
			cdr.StatP |= STATUS_READ;
			cdr.StatP &= ~STATUS_SEEK;

			CDREAD_INT(((cdr.Mode & 0x80) ? (cdReadTime) : cdReadTime * 2) + seekTime);

			cdr.Result[0] = cdr.StatP;
			start_rotating = 1;
			break;

		case CdlSync:
		default:
			CDR_LOG_I("Invalid command: %02x\n", Irq);
			error = ERROR_INVALIDCMD;
			// FALLTHROUGH

		set_error:
			SetResultSize(2);
			cdr.Result[0] = cdr.StatP | STATUS_ERROR;
			cdr.Result[1] = error;
			cdr.Stat = DiskError;
			break;
	}

	if (cdr.DriveState == DRIVESTATE_STOPPED && start_rotating) {
		cdr.DriveState = DRIVESTATE_STANDBY;
		cdr.StatP |= STATUS_ROTATING;
	}

	if (!no_busy_error) {
		switch (cdr.DriveState) {
		case DRIVESTATE_LID_OPEN:
		case DRIVESTATE_RESCAN_CD:
		case DRIVESTATE_PREPARE_CD:
			SetResultSize(2);
			cdr.Result[0] = cdr.StatP | STATUS_ERROR;
			cdr.Result[1] = ERROR_NOTREADY;
			cdr.Stat = DiskError;
			break;
		}
	}

finish:
	setIrq();
	cdr.ParamC = 0;

#ifdef CDR_LOG_CMD_IRQ
	{
		int i;
		SysPrintf("CDR IRQ %d cmd %02x stat %02x: ",
			!!(cdr.Stat & cdr.Reg2), Irq, cdr.Stat);
		for (i = 0; i < cdr.ResultC; i++)
			SysPrintf("%02x ", cdr.Result[i]);
		SysPrintf("\n");
	}
#endif
}

#ifdef HAVE_ARMV7
 #define ssat32_to_16(v) \
  asm("ssat %0,#16,%1" : "=r" (v) : "r" (v))
#else
 #define ssat32_to_16(v) do { \
  if (v < -32768) v = -32768; \
  else if (v > 32767) v = 32767; \
 } while (0)
#endif

void cdrAttenuate(s16 *buf, int samples, int stereo)
{
	int i, l, r;
	int ll = cdr.AttenuatorLeftToLeft;
	int lr = cdr.AttenuatorLeftToRight;
	int rl = cdr.AttenuatorRightToLeft;
	int rr = cdr.AttenuatorRightToRight;

	if (lr == 0 && rl == 0 && 0x78 <= ll && ll <= 0x88 && 0x78 <= rr && rr <= 0x88)
		return;

	if (!stereo && ll == 0x40 && lr == 0x40 && rl == 0x40 && rr == 0x40)
		return;

	if (stereo) {
		for (i = 0; i < samples; i++) {
			l = buf[i * 2];
			r = buf[i * 2 + 1];
			l = (l * ll + r * rl) >> 7;
			r = (r * rr + l * lr) >> 7;
			ssat32_to_16(l);
			ssat32_to_16(r);
			buf[i * 2] = l;
			buf[i * 2 + 1] = r;
		}
	}
	else {
		for (i = 0; i < samples; i++) {
			l = buf[i];
			l = l * (ll + rl) >> 7;
			//r = r * (rr + lr) >> 7;
			ssat32_to_16(l);
			//ssat32_to_16(r);
			buf[i] = l;
		}
	}
}

void cdrReadInterrupt() {
	u8 *buf;

	if (!cdr.Reading)
		return;

	if (cdr.Irq || cdr.Stat) {
		CDREAD_INT(0x800);
		return;
	}

	if ((psxHu32ref(0x1070) & psxHu32ref(0x1074) & SWAP32((u32)0x4)) && !cdr.ReadRescheduled) {
		// HACK: with BIAS 2, emulated CPU is often slower than real thing,
		// game may be unfinished with prev data read, so reschedule
		// (Brave Fencer Musashi)
		CDREAD_INT(cdReadTime / 2);
		cdr.ReadRescheduled = 1;
		return;
	}

	cdr.OCUP = 1;
	SetResultSize(1);
	cdr.StatP |= STATUS_READ|STATUS_ROTATING;
	cdr.StatP &= ~STATUS_SEEK;
	cdr.Result[0] = cdr.StatP;
	cdr.Seeked = SEEK_DONE;

	ReadTrack(cdr.SetSectorPlay);

	buf = CDR_getBuffer();
	if (buf == NULL)
		cdr.RErr = -1;

	if (cdr.RErr == -1) {
		//CDR_LOG_I("cdrReadInterrupt() Log: err\n");
		memset(cdr.Transfer, 0, DATA_SIZE);
		cdr.Stat = DiskError;
		cdr.Result[0] |= STATUS_ERROR;
		CDREAD_INT((cdr.Mode & 0x80) ? (cdReadTime / 2) : cdReadTime);
		return;
	}

	//memcpy(cdr.Transfer, buf, DATA_SIZE);
	cacheable_kernel_memcpy(cdr.Transfer, buf, DATA_SIZE);
	//CheckPPFCache(cdr.Transfer, cdr.Prev[0], cdr.Prev[1], cdr.Prev[2]);


	//CDR_LOG("cdrReadInterrupt() Log: cdr.Transfer %x:%x:%x\n", cdr.Transfer[0], cdr.Transfer[1], cdr.Transfer[2]);

	if ((!cdr.Muted) && (cdr.Mode & MODE_STRSND) && (!Config.Xa) && (cdr.FirstSector != -1)) { // CD-XA
		// Firemen 2: Multi-XA files - briefings, cutscenes
		if( cdr.FirstSector == 1 && (cdr.Mode & MODE_SF)==0 ) {
			cdr.File = cdr.Transfer[4 + 0];
			cdr.Channel = cdr.Transfer[4 + 1];
		}

		/* Gameblabla
		 * Skips playing on channel 255.
		 * Fixes missing audio in Blue's Clues : Blue's Big Musical. (Should also fix Taxi 2)
		 * TODO : Check if this is the proper behaviour.
		 * */
		if((cdr.Transfer[4 + 2] & 0x4) &&
			 (cdr.Transfer[4 + 1] == cdr.Channel) &&
			 (cdr.Transfer[4 + 0] == cdr.File) && cdr.Channel != 255) {
			int ret = xa_decode_sector(&cdr.Xa, cdr.Transfer+4, cdr.FirstSector);
			if (!ret) {
				cdrAttenuate(cdr.Xa.pcm, cdr.Xa.nsamples, cdr.Xa.stereo);
				/*
				 * Gameblabla -
				 * This is a hack for Megaman X4, Castlevania etc...
				 * that regressed from the new m_locationChanged and CDROM timings changes.
				 * It is mostly noticeable in Castevania however and the stuttering can be very jarring.
				 *
				 * According to PCSX redux authors, we shouldn't cause a location change if
				 * the sector difference is too small.
				 * I attempted to go with that approach but came empty handed.
				 * So for now, let's just set cdr.m_locationChanged to false when playing back any ADPCM samples.
				 * This does not regress Crash Team Racing's intro at least.
				*/
				cdr.m_locationChanged = FALSE;
				SPU_playADPCMchannel(&cdr.Xa);
				cdr.FirstSector = 0;
			}
			else cdr.FirstSector = -1;
		}
	}

	cdr.SetSectorPlay[2]++;
	if (cdr.SetSectorPlay[2] == 75) {
		cdr.SetSectorPlay[2] = 0;
		cdr.SetSectorPlay[1]++;
		if (cdr.SetSectorPlay[1] == 60) {
			cdr.SetSectorPlay[1] = 0;
			cdr.SetSectorPlay[0]++;
		}
	}

	cdr.Readed = 0;
	cdr.ReadRescheduled = 0;

	uint32_t delay = (cdr.Mode & MODE_SPEED) ? (cdReadTime / 2) : cdReadTime;
	if (cdr.m_locationChanged) {
		CDREAD_INT(delay * 15);
		cdr.m_locationChanged = FALSE;
	} else {
		CDREAD_INT(delay);
	}

	/*
	Croc 2: $40 - only FORM1 (*)
	Judge Dredd: $C8 - only FORM1 (*)
	Sim Theme Park - no adpcm at all (zero)
	*/

	if (!(cdr.Mode & MODE_STRSND) || !(cdr.Transfer[4+2] & 0x4)) {
		cdr.Stat = DataReady;
		setIrq();
	}

	// update for CdlGetlocP
	//ReadTrack(cdr.SetSectorPlay);
}

/*
cdrRead0:
	bit 0,1 - mode
	bit 2 - unknown
	bit 3 - unknown
	bit 4 - unknown
	bit 5 - 1 result ready
	bit 6 - 1 dma ready
	bit 7 - 1 command being processed
*/

unsigned char cdrRead0(void) {
	if (cdr.ResultReady)
		cdr.Ctrl |= 0x20;
	else
		cdr.Ctrl &= ~0x20;

	if (cdr.OCUP)
		cdr.Ctrl |= 0x40;
//  else
//		cdr.Ctrl &= ~0x40;

	// What means the 0x10 and the 0x08 bits? I only saw it used by the bios
	cdr.Ctrl |= 0x18;

	//CDR_LOG_IO("cdr r0: %02x\n", cdr.Ctrl);

	return psxHu8(0x1800) = cdr.Ctrl;
}

void cdrWrite0(unsigned char rt) {
	//CDR_LOG_IO("cdr w0: %02x\n", rt);

	cdr.Ctrl = (rt & 3) | (cdr.Ctrl & ~3);

	if (rt == 0) {
		cdr.ParamP = 0;
		cdr.ParamC = 0;
		cdr.ResultReady = 0;
	}
}

unsigned char cdrRead1(void) {
	if ((cdr.ResultP & 0xf) < cdr.ResultC)
		psxHu8(0x1801) = cdr.Result[cdr.ResultP & 0xf];
	else
		psxHu8(0x1801) = 0;
	cdr.ResultP++;
	if (cdr.ResultP == cdr.ResultC)
		cdr.ResultReady = 0;

	//CDR_LOG_IO("cdr r1: %02x\n", psxHu8(0x1801));

	return psxHu8(0x1801);
}

void cdrWrite1(unsigned char rt) {
	u8 set_loc[3];
	int i;

	//CDR_LOG_IO("cdr w1: %02x\n", rt);

	switch (cdr.Ctrl & 3) {
	case 0:
		break;
	case 3:
		cdr.AttenuatorRightToRightT = rt;
		return;
	default:
		return;
	}

	cdr.Cmd = rt;
	cdr.OCUP = 0;

#ifdef CDR_LOG_CMD_IRQ
	SysPrintf("CD1 write: %x (%s)", rt, CmdName[rt]);
	if (cdr.ParamC) {
		SysPrintf(" Param[%d] = {", cdr.ParamC);
		for (i = 0; i < cdr.ParamC; i++)
			SysPrintf(" %x,", cdr.Param[i]);
		SysPrintf("}\n");
	} else {
		SysPrintf("\n");
	}
#endif

	cdr.ResultReady = 0;
	cdr.Ctrl |= 0x80;
	// cdr.Stat = NoIntr;
	AddIrqQueue(cdr.Cmd, 0x800);

// cdrWrite1 switch =====================================
	switch (cdr.Cmd) {
	case CdlSetloc:
		//CDR_LOG("CDROM setloc command (%02X, %02X, %02X)\n", cdr.Param[0], cdr.Param[1], cdr.Param[2]);

		// MM must be BCD, SS must be BCD and <0x60, FF must be BCD and <0x75
		if (((cdr.Param[0] & 0x0F) > 0x09) || (cdr.Param[0] > 0x99) || ((cdr.Param[1] & 0x0F) > 0x09) || (cdr.Param[1] >= 0x60) || ((cdr.Param[2] & 0x0F) > 0x09) || (cdr.Param[2] >= 0x75))
		{
			//CDR_LOG("Invalid/out of range seek to %02X:%02X:%02X\n", cdr.Param[0], cdr.Param[1], cdr.Param[2]);
		}
		else
		{
			for (i = 0; i < 3; i++)
			{
				set_loc[i] = btoi(cdr.Param[i]);
			}

			i = msf2sec(cdr.SetSectorPlay);
			i = abs(i - msf2sec(set_loc));
			if (i > 16)
				cdr.Seeked = SEEK_PENDING;

			memcpy(cdr.SetSector, set_loc, 3);
			cdr.SetSector[3] = 0;
			cdr.SetlocPending = 1;
		}
		break;

	case CdlReadN:
	case CdlReadS:
	case CdlPause:
		StopCdda();
		StopReading();
		break;

	case CdlReset:
	case CdlInit:
		cdr.Seeked = SEEK_DONE;
		StopCdda();
		StopReading();
		break;

    	case CdlSetmode:
		//CDR_LOG("cdrWrite1() Log: Setmode %x\n", cdr.Param[0]);

        	cdr.Mode = cdr.Param[0];

		// Squaresoft on PlayStation 1998 Collector's CD Vol. 1
		// - fixes choppy movie sound
		if( cdr.Play && (cdr.Mode & MODE_CDDA) == 0 )
			StopCdda();
        	break;
	}
}

unsigned char cdrRead2(void) {
	unsigned char ret;

	if (cdr.Readed == 0) {
		ret = 0;
	} else {
		ret = *pTransfer++;
	}

	//CDR_LOG_IO("cdr r2: %02x\n", ret);
	return ret;
}

void cdrWrite2(unsigned char rt) {
	//CDR_LOG_IO("cdr w2: %02x\n", rt);

	switch (cdr.Ctrl & 3) {
	case 0:
		if (cdr.ParamC < 8) // FIXME: size and wrapping
			cdr.Param[cdr.ParamC++] = rt;
		return;
	case 1:
		cdr.Reg2 = rt;
		setIrq();
		return;
	case 2:
		cdr.AttenuatorLeftToLeftT = rt;
		return;
	case 3:
		cdr.AttenuatorRightToLeftT = rt;
		return;
	}
}

unsigned char cdrRead3(void) {
	if (cdr.Ctrl & 0x1)
		psxHu8(0x1803) = cdr.Stat | 0xE0;
	else
		psxHu8(0x1803) = cdr.Reg2 | 0xE0;
	return psxHu8(0x1803);
}

void cdrWrite3(unsigned char rt) {
	//CDR_LOG_IO("cdr w3: %02x\n", rt);

	switch (cdr.Ctrl & 3) {
	case 0:
		break; // transfer
	case 1:
		cdr.Stat &= ~rt;

		if (rt & 0x40)
			cdr.ParamC = 0;
		return;
	case 2:
		cdr.AttenuatorLeftToRightT = rt;
		return;
	case 3:
		if (rt & 0x20) {
			//memcpy(&cdr.AttenuatorLeftToLeft, &cdr.AttenuatorLeftToLeftT, 4);
			*((u32*)&cdr.AttenuatorLeftToLeft) = *((u32*)&cdr.AttenuatorLeftToLeftT);
			//CDR_LOG_I("CD-XA Volume: %02x %02x | %02x %02x\n",
			//	cdr.AttenuatorLeftToLeft, cdr.AttenuatorLeftToRight,
			//	cdr.AttenuatorRightToLeft, cdr.AttenuatorRightToRight);
		}
		return;
	}

	if ((rt & 0x80) && cdr.Readed == 0) {
		cdr.Readed = 1;
		pTransfer = cdr.Transfer;

		switch (cdr.Mode & 0x30) {
			case MODE_SIZE_2328:
			case 0x00:
				pTransfer += 12;
				break;

			case MODE_SIZE_2340:
				pTransfer += 0;
				break;

			default:
				break;
		}
	}
}

void psxDma3(u32 madr, u32 bcr, u32 chcr) {
	u32 cdsize;
	int size;
	u8 *ptr;

	//CDR_LOG("psxDma3() Log: *** DMA 3 *** %x addr = %x size = %x\n", chcr, madr, bcr);

	switch (chcr) {
		case 0x11000000:
		case 0x11400100:
			if (cdr.Readed == 0) {
				//CDR_LOG("psxDma3() Log: *** DMA 3 *** NOT READY\n");
				break;
			}

			cdsize = (bcr & 0xffff) << 2;

			// Ape Escape: bcr = 0001 / 0000
			// - fix boot
			if( cdsize == 0 )
			{
				switch (cdr.Mode & (MODE_SIZE_2340|MODE_SIZE_2328)) {
					case MODE_SIZE_2340: cdsize = 2340; break;
					case MODE_SIZE_2328: cdsize = 2328; break;
					default:
					case MODE_SIZE_2048: cdsize = 2048; break;
				}
			}


			ptr = (u8 *)PSXM(madr);
			if (ptr == NULL) {
				//CDR_LOG("psxDma3() Log: *** DMA 3 *** NULL Pointer!\n");
				break;
			}

			/*
			GS CDX: Enhancement CD crash
			- Setloc 0:0:0
			- CdlPlay
			- Spams DMA3 and gets buffer overrun
			*/
			size = CD_FRAMESIZE_RAW - (pTransfer - cdr.Transfer);
			if (size > cdsize)
				size = cdsize;
			if (size > 0)
			{
				//memcpy(ptr, pTransfer, size);
				cacheable_kernel_memcpy(ptr, pTransfer, size);
			}

			psxCpu->Clear(madr, cdsize >> 2);
			pTransfer += cdsize;

			if( chcr == 0x11400100 ) {
				HW_DMA3_MADR = SWAPu32(madr + cdsize);
				CDRDMA_INT( cdsize >> 5 );
			}
			else if( chcr == 0x11000000 ) {
				// CDRDMA_INT( (cdsize/4) * 1 );
				// halted
				psxRegs.cycle += (((cdsize >> 2) * 24 ) >> 1);
				CDRDMA_INT(16);
			}
			return;

		default:
			//CDR_LOG("psxDma3() Log: Unknown cddma %x\n", chcr);
			break;
	}

	HW_DMA3_CHCR &= SWAP32(~0x01000000);
	DMA_INTERRUPT(3);
}

void cdrDmaInterrupt()
{
	if (HW_DMA3_CHCR & SWAP32(0x01000000))
	{
		HW_DMA3_CHCR &= SWAP32(~0x01000000);
		DMA_INTERRUPT(3);
	}
}

static void getCdInfo(void)
{
	u8 tmp;

	CDR_getTN(cdr.ResultTN);
	CDR_getTD(0, cdr.SetSectorEnd);
	tmp = cdr.SetSectorEnd[0];
	cdr.SetSectorEnd[0] = cdr.SetSectorEnd[2];
	cdr.SetSectorEnd[2] = tmp;
}

void cdrReset() {
	memset(&cdr, 0, sizeof(cdr));
	cdr.CurTrack = 1;
	cdr.File = 1;
	cdr.Channel = 1;
	cdr.transferIndex = 0;
	cdr.Reg2 = 0x1f;
	cdr.Stat = NoIntr;
	cdr.DriveState = DRIVESTATE_STANDBY;
	cdr.StatP = STATUS_ROTATING;
	pTransfer = cdr.Transfer;
	cdr.SetlocPending = 0;
	cdr.m_locationChanged = FALSE;

	// BIOS player - default values
	cdr.AttenuatorLeftToLeft = 0x80;
	cdr.AttenuatorLeftToRight = 0x00;
	cdr.AttenuatorRightToLeft = 0x00;
	cdr.AttenuatorRightToRight = 0x80;

	getCdInfo();
}

int cdrFreeze(gzFile f, int Mode) {
	u32 tmp;
	u8 tmpp[3];

	if (Mode == 0 && !Config.Cdda)
		CDR_stop();

	cdr.freeze_ver = 0x63647202;
	gzfreeze(&cdr, sizeof(cdr));

	if (Mode == 1) {
		cdr.ParamP = cdr.ParamC;
		tmp = pTransfer - cdr.Transfer;
	}

	gzfreeze(&tmp, sizeof(tmp));

	if (Mode == 0) {
		getCdInfo();

		pTransfer = cdr.Transfer + tmp;

		// read right sub data
		tmpp[0] = btoi(cdr.Prev[0]);
		tmpp[1] = btoi(cdr.Prev[1]);
		tmpp[2] = btoi(cdr.Prev[2]);
		cdr.Prev[0]++;
		ReadTrack(tmpp);

		if (cdr.Play) {
			if (cdr.freeze_ver < 0x63647202)
				memcpy(cdr.SetSectorPlay, cdr.SetSector, 3);

			Find_CurTrack(cdr.SetSectorPlay);
			if (!Config.Cdda)
				CDR_play(cdr.SetSectorPlay);
		}

		if ((cdr.freeze_ver & 0xffffff00) != 0x63647200) {
			// old versions did not latch Reg2, have to fixup..
			if (cdr.Reg2 == 0) {
				SysPrintf("cdrom: fixing up old savestate\n");
				cdr.Reg2 = 7;
			}
			// also did not save Attenuator..
			if ((cdr.AttenuatorLeftToLeft | cdr.AttenuatorLeftToRight
			     | cdr.AttenuatorRightToLeft | cdr.AttenuatorRightToRight) == 0)
			{
				cdr.AttenuatorLeftToLeft = cdr.AttenuatorRightToRight = 0x80;
			}
		}
	}

	return 0;
}

void LidInterrupt() {
	getCdInfo();
	StopCdda();

    cdr.StatP |= STATUS_SHELLOPEN;
    cdr.DriveState = DRIVESTATE_RESCAN_CD;

	cdrLidSeekInterrupt();
}
