/* Replace "dll.h" with the name of your header */
#include "adpcm.h"
#include <windows.h>
#include <iostream>
#include<exception>
#define dllexport extern "C" __declspec(dllexport)

dllexport void HelloWorld()
{
	//MessageBox(0, "Hello World from DLL!\n", "Hi", MB_ICONINFORMATION);
}

/* 获取初始化 G72x_STATE 结构体 */
G72x_STATE* getG72x_STATE() {
	G72x_STATE* g72xchushihua = {0};	
	return g72xchushihua;
}

dllexport void getadpcm_decoder(char* indata, short* outdata, int len, G72x_STATE* state) {
	//G72x_STATE* jietouti = getG72x_STATE();
	//std::cout << "C++ 提示：已声明结构体" << std::endl;
	//private_init_state(state);
	//std::cout << "C++ 提示：结构体初始化" << std::endl;
	//std::cout << "C++ 提示：解码函数开始执行" << std::endl;
	adpcm_decoder(indata,outdata,len, state);
	//std::cout << "C++ 提示：解码函数执行完毕" << std::endl;
}


dllexport void setadpcm_coder(short*  indata, char* outdata, int len, G72x_STATE* state) {
	private_init_state(state);
	adpcm_coder(indata, outdata,len,state);
}



dllexport void test() {
	std::cout << "这是来自C++的输出" << std::endl;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		break;
	}
	case DLL_PROCESS_DETACH:
	{
		break;
	}
	case DLL_THREAD_ATTACH:
	{
		break;
	}
	case DLL_THREAD_DETACH:
	{
		break;
	}
	}

	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}


short g721_encoder(short sl, G72x_STATE* state_ptr);
short g721_decoder(short i, G72x_STATE* state_ptr);

void private_init_state(G72x_STATE* state_ptr);

static int fmult(int an, int srn);
static short quan(short val, short size);
static short quan1(short val, short* table, short size);

int	predictor_zero(G72x_STATE* state_ptr);

int	predictor_pole(G72x_STATE* state_ptr);

int	step_size(G72x_STATE* state_ptr);

int	quantize(int d, int	y, short* table, int size);

int	reconstruct(int sign, int dqln, int y);

void update(int y, int wi, int fi, int dq, int sr, int dqsez, G72x_STATE* state_ptr);


static short qtab_721[7] = { -124, 80, 178, 246, 300, 349, 400 };
/*
 * Maps G.721 code word to reconstructed scale factor normalized log
 * magnitude values.
 */
static short	_dqlntab[16] = { -2048, 4, 135, 213, 273, 323, 373, 425,
				425, 373, 323, 273, 213, 135, 4, -2048 };

/* Maps G.721 code word to log of scale factor multiplier. */
static short	_witab[16] = { -12, 18, 41, 64, 112, 198, 355, 1122,
				1122, 355, 198, 112, 64, 41, 18, -12 };
/*
 * Maps G.721 code words to a set of values whose long and short
 * term averages are computed and then compared to give an indication
 * how stationary (steady state) the signal is.
 */
static short	_fitab[16] = { 0, 0, 0, 0x200, 0x200, 0x200, 0x600, 0xE00,
				0xE00, 0x600, 0x200, 0x200, 0x200, 0, 0, 0 };





//======================================
//?????160?,adpcm_coder????????5.5ms,adpcm_decoder????????5ms
void adpcm_coder(short* indata, char* outdata, int len, G72x_STATE* state)
{
	short code_1;
	short code_2;

	len >>= 1;
	while (len-- > 0) {
		//std::cout << "adpcm_coder进入while" << std::endl;
		code_1 = g721_encoder(*indata++, state);
		//std::cout << "adpcm_coder执行code1" << std::endl;
		code_2 = g721_encoder(*indata++, state);
		*outdata++ = code_1 | (code_2 << 4);
	}
}

void adpcm_decoder(char* indata, short* outdata, int len, G72x_STATE* state)
{
	short output;

	len >>= 1;
	while (len-- > 0) {
		//std::cout << "adpcm_decoder执行code1" << std::endl;
		output = *indata++;
		*outdata++ = g721_decoder(output & 0xf, state);
		*outdata++ = g721_decoder(output >> 4, state);
	}
}

short g721_encoder(short sl, G72x_STATE* state_ptr)
{
	short		sezi, se, sez;		/* ACCUM */
	short		d;			/* SUBTA */
	short		sr;			/* ADDB */
	short		y;			/* MIX */
	short		dqsez;			/* ADDC */
	short		dq, i;

	/* linearize input sample to 14-bit PCM */
	sl >>= 2;			/* 14-bit dynamic range */

	sezi = predictor_zero(state_ptr);
	sez = sezi >> 1;
	se = (sezi + predictor_pole(state_ptr)) >> 1;	/* estimated signal */

	d = sl - se;				/* estimation difference */

	/* quantize the prediction difference */
	y = step_size(state_ptr);		/* quantizer step size */
	i = quantize(d, y, qtab_721, 7);	/* i = ADPCM code */

	dq = reconstruct(i & 8, _dqlntab[i], y);	/* quantized est diff */

	sr = (dq < 0) ? se - (dq & 0x3FFF) : se + dq;	/* reconst. signal */

	dqsez = sr + sez - se;			/* pole prediction diff. */

	update(y, _witab[i] << 5, _fitab[i], dq, sr, dqsez, state_ptr);

	return (i);
}

/*
 * g721_decoder()
 *
 * Description:
 *
 * Decodes a 4-bit code of G.721 encoded data of i and
 * returns the resulting linear PCM, A-law or u-law value.
 * return -1 for unknown out_coding value.
 */
short g721_decoder(short i, G72x_STATE* state_ptr)
{
	//std::cout << "进入g721_decoder" << std::endl;
	short		sezi, sei, sez, se;	/* ACCUM */
	short		y;			/* MIX */
	short		sr;			/* ADDB */
	short		dq;
	short		dqsez;

	i &= 0x0f;			/* mask to get proper bits */
	sezi = predictor_zero(state_ptr);
	sez = sezi >> 1;
	sei = sezi + predictor_pole(state_ptr);
	se = sei >> 1;			/* se = estimated signal */

	y = step_size(state_ptr);	/* dynamic quantizer step size */

	dq = reconstruct(i & 0x08, _dqlntab[i], y); /* quantized diff. */

	sr = (dq < 0) ? (se - (dq & 0x3FFF)) : se + dq;	/* reconst. signal */

	dqsez = sr - se + sez;			/* pole prediction diff. */

	update(y, _witab[i] << 5, _fitab[i], dq, sr, dqsez, state_ptr);

	/* sr was 14-bit dynamic range */
	return (sr << 2);
}


void private_init_state(G72x_STATE* state_ptr)
{
	int		cnta;

	state_ptr->yl = 34816;
	state_ptr->yu = 544;
	state_ptr->dms = 0;
	state_ptr->dml = 0;
	state_ptr->ap = 0;
	for (cnta = 0; cnta < 2; cnta++) {
		state_ptr->a[cnta] = 0;
		state_ptr->pk[cnta] = 0;
		state_ptr->sr[cnta] = 32;
	}
	for (cnta = 0; cnta < 6; cnta++) {
		state_ptr->b[cnta] = 0;
		state_ptr->dq[cnta] = 32;
	}
	state_ptr->td = 0;
}	/* private_init_state */

/*
 * quan()
 *
 * quantizes the input val against the table of size short integers.
 * It returns i if table[i - 1] <= val < table[i].
 *
 * Using linear search for simple coding.
 */
short quan(short val, short size)
{
	short		i;
	//short *p;
	if (val <= 0) {
		return 0;
	}
	i = 15;
	while (1) {
		if (val & 0x4000) {
			return i;
		}
		val <<= 1;
		i--;
	}
}

short quan1(short val, short* table, short size)
{
	if (val < -124) {
		return 0;
	}
	else if (val < 80) {
		return 1;
	}
	else if (val < 178) {
		return 2;
	}
	else if (val < 246) {
		return 3;
	}
	else if (val < 300) {
		return 4;
	}
	else if (val < 349) {
		return 5;
	}
	else if (val < 400) {
		return 6;
	}
	return 7;
}


/*
 * fmult()
 *
 * returns the integer product of the 14-bit integer "an" and
 * "floating point" representation (4-bit exponent, 6-bit mantessa) "srn".
 */
static int fmult(int an, int srn)
{
	short		anmag, anexp, anmant;
	short		wanexp, wanmant;
	short		retval;

	if (an == 0) {
		anmag = 0;
		anexp = -6;
		anmant = 32;
		wanexp = ((srn >> 6) & 0xF) - 19;
	}
	else {
		anmag = (an > 0) ? an : (-an);
		anexp = quan(anmag, 15) - 6;
		anmant = (anexp >= 0) ? anmag >> anexp : anmag << -anexp;
		wanexp = anexp + ((srn >> 6) & 0xF) - 13;
	}
	/*
	** The original was :
	**		wanmant = (anmant * (srn & 0x37) + 0x30) >> 4 ;
	** but could see no valid reason for the + 0x30.
	** Removed it and it improved the SNR of the codec.
	*/

	wanmant = (anmant * (srn & 0x37)) >> 4;

	retval = (wanexp >= 0) ? ((wanmant << wanexp) & 0x7FFF) :
		(wanmant >> -wanexp);

	return (((an ^ srn) < 0) ? -retval : retval);
}


/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 101b6e25-457d-490a-99ae-e2e74a26ea24
*/
/*
 * predictor_zero()
 *
 * computes the estimated signal from 6-zero predictor.
 *
 */
int  predictor_zero(G72x_STATE* state_ptr)
{
	int		i;
	int		sezi;
	short* short_src;
	short* short_src2;


	short_src = state_ptr->b;
	short_src2 = state_ptr->dq;
	i = 6;
	sezi = 0;
	while (i-- > 0) {
		sezi += fmult(*short_src++ >> 2, *short_src2++);
	}
	return (sezi);
}
/*
 * predictor_pole()
 *
 * computes the estimated signal from 2-pole predictor.
 *
 */
int  predictor_pole(G72x_STATE* state_ptr)
{
	return (fmult(state_ptr->a[1] >> 2, state_ptr->sr[1]) +
		fmult(state_ptr->a[0] >> 2, state_ptr->sr[0]));
}
/*
 * step_size()
 *
 * computes the quantization step size of the adaptive quantizer.
 *
 */
int  step_size(G72x_STATE* state_ptr)
{
	int		y;
	int		dif;
	int		al;

	if (state_ptr->ap >= 256) {
		return (state_ptr->yu);
	}
	else {
		y = state_ptr->yl >> 6;
		dif = state_ptr->yu - y;
		al = state_ptr->ap >> 2;
		if (dif > 0) {
			y += (dif * al) >> 6;
		}
		else if (dif < 0) {
			y += (dif * al + 0x3F) >> 6;
		}
		return (y);
	}
}

/*
 * quantize()
 *
 * Given a raw sample, 'd', of the difference signal and a
 * quantization step size scale factor, 'y', this routine returns the
 * ADPCM codeword to which that sample gets quantized.  The step
 * size scale factor division operation is done in the log base 2 domain
 * as a subtraction.
 */
int quantize(
	int		d,	/* Raw difference signal sample */
	int		y,	/* Step size multiplier */
	short* table,	/* quantization table */
	int		size)	/* table size of short integers */
{
	short		dqm;	/* Magnitude of 'd' */
	short		expon;	/* Integer part of base 2 log of 'd' */
	short		mant;	/* Fractional part of base 2 log */
	short		dl;	/* Log of magnitude of 'd' */
	short		dln;	/* Step size scale factor normalized log */
	int		i;

	/*
	 * LOG
	 *
	 * Compute base 2 log of 'd', and store in 'dl'.
	 */
	if (d < 0) {
		dqm = 0 - d;
	}
	else {
		dqm = d;
	}
	expon = quan(dqm >> 1, 15);
	mant = ((dqm << 7) >> expon) & 0x7F;	/* Fractional portion. */
	dl = (expon << 7) + mant;

	/*
	 * SUBTB
	 *
	 * "Divide" by step size multiplier.
	 */
	dln = dl - (y >> 2);

	/*
	 * QUAN
	 *
	 * Obtain codword i for 'd'.
	 */
	 //i = quan(dln, table, size); 
	i = quan1(dln, table, size);
	if (d < 0)			/* take 1's complement of i */
		return ((size << 1) + 1 - i);
	else if (i == 0)		/* take 1's complement of 0 */
		return ((size << 1) + 1); /* new in 1988 */
	else
		return (i);
}
/*
 * reconstruct()
 *
 * Returns reconstructed difference signal 'dq' obtained from
 * codeword 'i' and quantization step size scale factor 'y'.
 * Multiplication is performed in log base 2 domain as addition.
 */
int
reconstruct(
	int		sign,	/* 0 for non-negative value */
	int		dqln,	/* G.72x codeword */
	int		y)	/* Step size multiplier */
{
	short		dql;	/* Log of 'dq' magnitude */
	short		dex;	/* Integer part of log */
	short		dqt;
	short		dq;	/* Reconstructed difference signal sample */

	dql = dqln + (y >> 2);	/* ADDA */

	if (dql < 0) {
		return ((sign) ? -0x8000 : 0);
	}
	else {		/* ANTILOG */
		dex = (dql >> 7) & 15;
		dqt = 128 + (dql & 127);
		dq = (dqt << 7) >> (14 - dex);
		return ((sign) ? (dq - 0x8000) : dq);
	}
}

/*
 * update()
 *
 * updates the state variables for each output code
 */
void
update(
	int		y,		/* quantizer step size */
	int		wi,		/* scale factor multiplier */
	int		fi,		/* for long/short term energies */
	int		dq,		/* quantized prediction difference */
	int		sr,		/* reconstructed signal */
	int		dqsez,		/* difference from 2-pole predictor */
	G72x_STATE* state_ptr)	/* coder state pointer */
{
	int		cnt;
	short		mag, expon;	/* Adaptive predictor, FLOAT A */
	short		a2p = 0;	/* LIMC */
	short		a1ul;		/* UPA1 */
	short		pks1;		/* UPA2 */
	short		fa1;
	char		tr;		/* tone/transition detector */
	short		ylint, thr2, dqthr;
	short  		ylfrac, thr1;
	short		pk0;
	short* short_src;
	short* short_src2;
	short		short_value;



	pk0 = (dqsez < 0) ? 1 : 0;	/* needed in updating predictor poles */

	mag = dq & 0x7FFF;		/* prediction difference magnitude */
	/* TRANS */
	ylint = state_ptr->yl >> 15;	/* exponent part of yl */
	ylfrac = (state_ptr->yl >> 10) & 0x1F;	/* fractional part of yl */
	thr1 = (32 + ylfrac) << ylint;		/* threshold */
	thr2 = (ylint > 9) ? 31 << 10 : thr1;	/* limit thr2 to 31 << 10 */
	dqthr = (thr2 + (thr2 >> 1)) >> 1;	/* dqthr = 0.75 * thr2 */
	if (state_ptr->td == 0)		/* signal supposed voice */
		tr = 0;
	else if (mag <= dqthr)		/* supposed data, but small mag */
		tr = 0;			/* treated as voice */
	else				/* signal is data (modem) */
		tr = 1;

	/*
	 * Quantizer scale factor adaptation.
	 */

	 /* FUNCTW & FILTD & DELAY */
	 /* update non-steady state step size multiplier */
	short_value = y + ((wi - y) >> 5);

	/* LIMB */
	if (short_value < 544)	/* 544 <= yu <= 5120 */
		short_value = 544;
	else if (short_value > 5120)
		short_value = 5120;
	state_ptr->yu = short_value;
	/* FILTE & DELAY */
	/* update steady state step size multiplier */
	state_ptr->yl += short_value + ((-state_ptr->yl) >> 6);

	/*
	 * Adaptive predictor coefficients.
	 */
	if (tr == 1) {			/* reset a's and b's for modem signal */
		state_ptr->a[0] = 0;
		state_ptr->a[1] = 0;
		state_ptr->b[0] = 0;
		state_ptr->b[1] = 0;
		state_ptr->b[2] = 0;
		state_ptr->b[3] = 0;
		state_ptr->b[4] = 0;
		state_ptr->b[5] = 0;
	}
	else {			/* update a's and b's */
		pks1 = pk0 ^ state_ptr->pk[0];		/* UPA2 */

		/* update predictor pole a[1] */
		a2p = state_ptr->a[1] - (state_ptr->a[1] >> 7);
		if (dqsez != 0) {
			fa1 = (pks1) ? state_ptr->a[0] : -state_ptr->a[0];
			if (fa1 < -8191)	/* a2p = function of fa1 */
				a2p -= 0x100;
			else if (fa1 > 8191)
				a2p += 0xFF;
			else
				a2p += fa1 >> 5;

			if (pk0 ^ state_ptr->pk[1]) {	/* LIMC */
				if (a2p <= -12160)
					a2p = -12288;
				else if (a2p >= 12416)
					a2p = 12288;
				else
					a2p -= 0x80;
			}
			else if (a2p <= -12416)
				a2p = -12288;
			else if (a2p >= 12160)
				a2p = 12288;
			else
				a2p += 0x80;
		}

		/* TRIGB & DELAY */
		state_ptr->a[1] = a2p;

		/* UPA1 */
		/* update predictor pole a[0] */
		short_value = state_ptr->a[0];
		short_value -= short_value >> 8;
		if (dqsez != 0) {
			if (pks1 == 0)
				short_value += 192;
			else
				short_value -= 192;
		};
		/* LIMD */
		a1ul = 15360 - a2p;
		if (short_value < -a1ul)
			short_value = -a1ul;
		else if (short_value > a1ul)
			short_value = a1ul;
		state_ptr->a[0] = short_value;

		/* UPB : update predictor zeros b[6] */
		short_src = state_ptr->b;
		short_src2 = state_ptr->dq;
		cnt = 6;
		while (cnt-- > 0) {
			short_value = *short_src;
			short_value -= short_value >> 8;
			if (dq & 0x7FFF) {			/* XOR */
				if ((dq ^ (*short_src2++)) >= 0)
					short_value += 128;
				else
					short_value -= 128;
			}
			*short_src++ = short_value;
		}
	}
	short_src = &(state_ptr->dq[5]);
	short_src2 = &(state_ptr->dq[4]);
	cnt = 5;
	while (cnt-- > 0) {//for (cnt = 5; cnt > 0; cnt--){
		*short_src-- = *short_src2--;
	}
	//state_ptr->dq[cnt] = state_ptr->dq[cnt-1];
/* FLOAT A : convert dq[0] to 4-bit exp, 6-bit mantissa f.p. */
	if (mag == 0) {
		state_ptr->dq[0] = (dq >= 0) ? 0x20 : 0xFC20;
	}
	else {
		expon = quan(mag, 15);
		state_ptr->dq[0] = (dq >= 0) ?
			(expon << 6) + ((mag << 6) >> expon) :
			(expon << 6) + ((mag << 6) >> expon) - 0x400;
	}

	state_ptr->sr[1] = state_ptr->sr[0];
	/* FLOAT B : convert sr to 4-bit exp., 6-bit mantissa f.p. */
	if (sr == 0) {
		state_ptr->sr[0] = 0x20;
	}
	else if (sr > 0) {
		expon = quan(sr, 15);
		state_ptr->sr[0] = (expon << 6) + ((sr << 6) >> expon);
	}
	else if (sr > -32768) {
		mag = -sr;
		expon = quan(mag, 15);
		state_ptr->sr[0] = (expon << 6) + ((mag << 6) >> expon) - 0x400;
	}
	else
		state_ptr->sr[0] = (short)0xFC20;

	/* DELAY A */
	state_ptr->pk[1] = state_ptr->pk[0];
	state_ptr->pk[0] = pk0;

	/* TONE */
	if (tr == 1)		/* this sample has been treated as data */
		state_ptr->td = 0;	/* next one will be treated as voice */
	else if (a2p < -11776)	/* small sample-to-sample correlation */
		state_ptr->td = 1;	/* signal may be data */
	else				/* signal is voice */
		state_ptr->td = 0;

	/*
	 * Adaptation speed control.
	 */
	state_ptr->dms += (fi - state_ptr->dms) >> 5;		/* FILTA */
	state_ptr->dml += (((fi << 2) - state_ptr->dml) >> 7);	/* FILTB */

	if (tr == 1)
		state_ptr->ap = 256;
	else if (y < 1536)					/* SUBTC */
		state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
	else if (state_ptr->td == 1)
		state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
	else if (abs((state_ptr->dms << 2) - state_ptr->dml) >=
		(state_ptr->dml >> 3))
		state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
	else
		state_ptr->ap += (-state_ptr->ap) >> 4;

	return;
}
/* update */



