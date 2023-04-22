#pragma once
#ifndef _DLL_H_
#define _DLL_H_

#if BUILDING_DLL
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif



extern "C" {
#endif
	/*
	** This is the public structure for passing data between the caller and
	** the G72x encoder and decoder.
	** The private array is used by the encoder and decoder for internal
	** state information and should not be changed in any way by the caller.
	** When decoding or encoding a stream, the same instance of this struct
	** should be used for every call so that the decoder/encoder keeps the
	** correct state data between calls.
	*/
	typedef struct private_g72x
	{
		long  yl;	/* Locked or steady state step size multiplier. */
		short yu;	/* Unlocked or non-steady state step size multiplier. */
		short dms;	/* Short term energy estimate. */
		short dml;	/* Long term energy estimate. */
		short ap;	/* Linear weighting coefficient of 'yl' and 'yu'. */

		short a[2];	/* Coefficients of pole portion of prediction filter. */
		short b[6];	/* Coefficients of zero portion of prediction filter. */
		short pk[2];	/*
						** Signs of previous two samples of a partially
						** reconstructed signal.
						**/
		short dq[6];	/*
						** Previous 6 samples of the quantized difference
						** signal represented in an internal floating point
						** format.
						**/
		short sr[2];	/*
						** Previous 2 samples of the quantized difference
						** signal represented in an internal floating point
						** format.
						*/
		char td;	/* delayed tone detect, new in 1988 version */

		/*	The following struct members were added for libsndfile. The original
		**	code worked by calling a set of functions on a sample by sample basis
		**	which is slow on architectures like Intel x86. For libsndfile, this
		**	was changed so that the encoding and decoding routines could work on
		**	a block of samples at a time to reduce the function call overhead.
		*/
		//	short		(*encoder) (short, struct private_g72x* state) ;
		//	short		(*decoder) (short, struct private_g72x* state) ;

		//	int		codec_bits ;
		//	int		byte_index, sample_index ;

	} G72x_STATE;


	//extern G72x_STATE enc_state, dec_state ;

	//void adpcm_inial(void);
	void private_init_state(G72x_STATE* state_ptr);

	void adpcm_coder(short* indata, char* outdata, int len, G72x_STATE* state);

	void adpcm_decoder(char* indata, short* outdata, int len, G72x_STATE* state);

	extern G72x_STATE* getG72x_STATE();

	//void getadpcm_decoder(char* indata, short* outdata, int len);

#ifdef __cplusplus
}
#endif










