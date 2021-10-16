////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	axisfiltertb.cpp
// {{{
// Project:	DSP Filtering Example Project
//
// Purpose:	A generic filter testbench class.
//
//	This class has been modified from the original filter testbench to
//	support an AXI stream based filter.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
// }}}
// Copyright (C) 2021, Gisselquist Technology, LLC
// {{{
// This file is part of the DSP filtering set of designs.
//
// The DSP filtering designs are free RTL designs: you can redistribute them
// and/or modify any of them under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// The DSP filtering designs are distributed in the hope that they will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTIBILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
// General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with these designs.  (It's in the $(ROOT)/doc directory.  Run make
// with no target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
// }}}
// License:	LGPL, v3, as defined and found on www.gnu.org,
// {{{
//		http://www.gnu.org/licenses/lgpl.html
//
////////////////////////////////////////////////////////////////////////////////
//
// }}}
#include <math.h>
#include "axisfiltertb.h"

// sbits
// {{{
static uint64_t	sbits(uint64_t val, int b) {
	int64_t	s;

	s = (val << (sizeof(val)*8-b));
	s >>= (sizeof(val)*8-b);
	return	s;
}
// }}}

// ubits
// {{{
static uint64_t ubits(uint64_t val, int b) {
	uint64_t	one = 1;

	return	val &= (one<<b)-one;
}
// }}}

// tick
// {{{
template<class VFLTR> void	AXISFILTERTB<VFLTR>::tick(void) {
	bool	ce;
	int64_t	vec[2];

	ce = (TESTB<VFLTR>::m_core->S_AXI_TVALID &&
		TESTB<VFLTR>::m_core->S_AXI_TREADY);
	if (ce)
		vec[0] = sbits(TESTB<VFLTR>::m_core->S_AXI_TDATA, IW());
	else
		vec[0] = 0;

	TESTB<VFLTR>::tick();

	o_ce = (TESTB<VFLTR>::m_core->M_AXI_TVALID &&
		TESTB<VFLTR>::m_core->M_AXI_TREADY);
	if (o_ce)
		vec[1] = sbits(TESTB<VFLTR>::m_core->o_result, OW());
	else
		vec[1] = 0;

	if (result_fp)
		fwrite(vec, sizeof(int64_t), 2, result_fp);
}
// }}}

// reset
// {{{
template<class VFLTR> void	AXISFILTERTB<VFLTR>::reset(void) {
	TESTB<VFLTR>::m_core->i_tap_wr     = 0;
	TESTB<VFLTR>::m_core->i_tap        = 0;
	TESTB<VFLTR>::m_core->S_AXI_TVALID = 0;
	TESTB<VFLTR>::m_core->S_AXI_TDATA  = 0;
	TESTB<VFLTR>::m_core->M_AXI_TREADY = 1;

	TESTB<VFLTR>::reset();

	TESTB<VFLTR>::m_core->i_reset = 0;
}
// }}}

// apply
// {{{
bool	AXISFILTERTB<VFLTR>::apply(int64_t input, int64_t &result) {
	bool	accepted = false, return_value = false;
	int	returned_values = 0;

	// Make sure the CE line is high
	TESTB<VFLTR>::m_core->S_AXI_TVALID = 1;

	// Strip off any excess bits
	TESTB<VFLTR>::m_core->S_AXI_TDATA  = ubits(data[i], IW());

	// Make sure we accept any output(s)
	TESTB<VFLTR>::m_core->M_AXI_TREADY = 1;

	do {
		accepted = TESTB<VFLTR>::m_core->S_AXI_TREADY;

		// Apply the filter
		tick();

		if (TESTB<VFLTR>::m_core->M_AXI_TVALID) {
			returned_values ++;
			result = sbits(TESTB<VFLTR>::m_core->M_AXI_TDATA, OW());
		}
	} while(!accepted);

	// Idle the interface--whether or not further values are active within
	// it.
	TESTB<VFLTR>::m_core->S_AXI_TVALID = 0;
	TESTB<VFLTR>::m_core->S_AXI_TREADY = 1;

	assert(returned_values <= 1);
	return (returned_values > 0);
}

template<class VFLTR> void	AXISFILTERTB<VFLTR>::apply(int nlen, int64_t *data) {
// printf("AXISFILTERTB::apply(%d, ...)\n", nlen);
	TESTB<VFLTR>::m_core->i_reset      = 0;
	TESTB<VFLTR>::m_core->i_tap_wr     = 0;
	TESTB<VFLTR>::m_core->S_AXI_TVALID = 0;
	tick();

	for(int i=0; i<nlen; i++) {
		if (apply(data[i], data[outln]))
			outln++;
	}
	TESTB<VFLTR>::m_core->i_ce     = 0;
}
// }}}

// load
// {{{
template<class VFLTR> void	AXISFILTERTB<VFLTR>::load(int  ntaps, int64_t *data) {
	TESTB<VFLTR>::m_core->i_reset = 0;
	TESTB<VFLTR>::m_core->i_ce    = 0;
	TESTB<VFLTR>::m_core->i_tap_wr= 1;
	TESTB<VFLTR>::m_core->S_AXI_TVALID = 0;
	TESTB<VFLTR>::m_core->M_AXI_TREADY = 1;

	for(int i=0; i<ntaps; i++) {
		// Strip off any excess bits
		TESTB<VFLTR>::m_core->i_tap = ubits(data[i], TW());

		// Apply the filter
		tick();
	}
	TESTB<VFLTR>::m_core->i_tap_wr= 0;

	// clear_cache();
}
// }}}

// test
// {{{
template<class VFLTR> void	AXISFILTERTB<VFLTR>::test(int  nlen, int64_t *data) {
	const	bool	debug = false;
	assert(nlen > 0);

	reset();

	TESTB<VFLTR>::m_core->i_reset  = 0;
	TESTB<VFLTR>::m_core->i_tap_wr = 0;

	apply(nlen, data);
}
// }}}

// operator[]
// {{{
template<class VFLTR> int	AXISFILTERTB<VFLTR>::operator[](const int tap) {
/*
	if ((tap < 0)||(tap >= 2*NTAPS()))
		return 0;
	else if (!m_hk) {
		int	nlen = 2*NTAPS();
		m_hk = new int64_t[nlen];

		// Create an input vector with a single impulse in it
		for(int i=0; i<nlen; i++)
			m_hk[i] = 0;
		m_hk[0] = -(1<<(IW()-1));

		// Apply the filter to the impulse vector
		test(nlen, m_hk);

		// Set our m_hk vector based upon the results
		for(int i=0; i<nlen; i++) {
			int	shift;
			shift = IW()-1;
			m_hk[i] >>= shift;
			m_hk[i] = -m_hk[i];
		}
	}

	return m_hk[tap];
*/
	return 0;
}
// }}}

// testload
// {{{
template<class VFLTR> void	AXISFILTERTB<VFLTR>::testload(int nlen, int64_t *data) {
	// Load the filter's coefficients
	load(nlen, data);
	reset();

/*
	// Verify that the filter's coefficients, as derived, match the ones
	// we just loaded.
	//
	bool	mismatch = false;

	for(int k=0; k<nlen; k++) {
		int	m = (*this)[k];
		if (data[k] != m) {
			printf("Err: Data[%*d] = %8ld != (*this)[%*d] = %8d\n",
				k, (int)ceil(log(nlen+1)/log(10.0)), data[k],
				k, (int)ceil(log(nlen+1)/log(10.0)), m);
			mismatch = true;
		}
	}

	if (mismatch) {
		fflush(stdout);
		assert(!mismatch);
	}
	for(int k=nlen; k<2*DELAY(); k++)
		assert(0 == (*this)[k]);
*/
}
// }}}

// test_overflow
// {{{
template<class VFLTR> bool	AXISFILTERTB<VFLTR>::test_overflow(void) {
	// Insert a specially designed test sequence, designed to overflow
	// the filter.  This would work if operator[] was working.  For now,
	// we'll just comment it out.
/*
	int	nlen = 2*NTAPS();
	int64_t	*input  = new int64_t[nlen],
		*output = new int64_t[nlen];
	int64_t	maxv = (1<<(IW()-1))-1;
	bool	pass = true, tested = false;

	// maxv = 1;

	for(int k=0; k<nlen; k++) {
		// input[v] * (*this)[(NTAPS-1)-v]
		if ((*this)[NTAPS()-1-k] < 0)
			input[k] = -maxv-1;
		else
			input[k] =  maxv;
		output[k]= input[k];
	}

	test(nlen, output);

	for(int k=0; k<nlen; k++) {
		int64_t	acc = 0;
		bool	all = true;
		for(int v = 0; v<NTAPS(); v++) {
			if (k-v >= 0) {
				acc += input[k-v] * (*this)[v];
				if (acc < 0)
					all = false;
			} else
				all = false;
		}

		if (all)
			tested = true;

		pass = (pass)&&(output[k] == acc);
		assert(output[k] == acc);
	}

	delete[] input;
	delete[] output;
	return (pass)&&(tested);
*/
	return false;
}
// }}}

// response
// {{{
template<class VFLTR> void	AXISFILTERTB<VFLTR>::response(int nfreq,
		COMPLEX *rvec, double mag, const char *fname) {
	int	nlen = NTAPS();
	int64_t	*data = new int64_t[nlen];
	double	df = 1./nfreq / 2.;
	COMPLEX	hk;
	const bool	debug= false;

	// Nh tap filter
	// Nv length vector
	// Nh+Nv-1 length output
	// But we want our output length to not include any runups
	//
	// Nh runup + Nv + Nh rundown + delay
	mag = mag * ((1<<(IW()-1))-1);

	for(int i=0; i<nfreq; i++) {
		double	dtheta = 2.0 * M_PI * i * df,
			theta=0.;
		hk = 0;

		if (debug) {
			theta = 0.;
			for(int j=0; j<nlen; j++) {
				double	dv = cos(theta);
				hk.real(hk.real() + dv * (*this)[j]);
				theta -= dtheta;
			}
		}

		theta = -(NTAPS()-1) * dtheta;
		for(int j=0; j<nlen; j++) {
			double	dv = mag * cos(theta);

			theta += dtheta;
			data[j] = dv;
		}

		test(nlen, data);
		rvec[i].real(data[nlen-1] / mag);

		// Repeat what should produce the same response, but using
		// a 90 degree phase offset.  Do this for all but the zero
		// frequency
		if (i > 0) {
			if (debug) {
				theta = 0.;
				for(int j=0; j<2*nlen; j++) {
					double	dv = sin(theta);
					hk.imag(hk.imag() + dv * (*this)[j]);
					theta -= dtheta;
				}
			}

			theta = -(NTAPS()-1) * dtheta;
			for(int j=0; j<nlen; j++) {
				double	dv = mag * sin(theta);

				theta += dtheta;
				data[j] = dv;
			}

			test(nlen, data);
			rvec[i].imag(data[nlen-1] / mag);
		}

		if (debug)
			printf("RSP[%4d / %4d] = %10.2f+%10.2fj  // Expect %10.2f+%10.2fj\n",
				i, nfreq, real(rvec[i]), imag(rvec[i]),
				real(hk), imag(hk));
	}

	delete[] data;

	if (fname) {
		FILE* fp;
		fp = fopen(fname,"w");
		fwrite(rvec, sizeof(COMPLEX), nfreq, fp);
		fclose(fp);
	}
}
// }}}

// measure_lowpass
// {{{
template<class VFLTR> void AXISFILTERTB<VFLTR>::measure_lowpass(double &fp, double &fs,
			double &depth, double &ripple) {
	/*
	const	int	NLEN = 16*NTAPS();
	COMPLEX	*data = new COMPLEX[NLEN];
	double	*magv = new double[NLEN];
	double	dc, maxpass, minpass, maxstop;
	int	midcut;
	bool	passband_ripple = false;

	response(NLEN, data, 1.0, "filter_tb.dbl");

	for(int k=0; k<NLEN; k++) {
		magv[k]= norm(data[k]);
	}

	midcut = NLEN-1;
	dc = magv[0];
	for(int k=0; k<NLEN; k++)
		if (magv[k] < 0.25 * dc) {
			midcut = k;
			break;
		}
	maxpass = dc;
	minpass = dc;
	for(int k=midcut; k>= 0; k--)
		if (magv[k] > maxpass)
			maxpass = magv[k];
	for(int k=midcut; k>= 0; k--)
		if ((magv[k] < minpass)&&(magv[k+1] > magv[k])) {
			minpass = magv[k];
			passband_ripple = true;
		}
	if (!passband_ripple)
		minpass = maxpass / sqrt(2.0);
	for(int k=midcut; k>= 0; k--)
		if (magv[k] > minpass) {
			fp = k;
			break;
		}

	maxstop = magv[NLEN-1];
	for(int k=midcut; k<NLEN; k++)
		if ((fabs(magv[k]) > fabs(magv[k-1]))&&(fabs(magv[k])> maxstop))
			maxstop = fabs(magv[k]);
	for(int k=midcut; k<NLEN; k++)
		if (magv[k] <= maxstop) {
			fs = k;
			break;
		}

printf("MAXPASS= %f\n", maxpass);
printf("MINPASS= %f\n", minpass);
printf("FP     = %f\n", fp);
printf("FS     = %f\n", fs);
printf("DC     = %f\n", dc);
printf("--------\n");

	ripple = 2.0 * (maxpass - minpass)/(maxpass + minpass);
	depth  = 10.0*log(maxstop/dc)/log(10.0);
	fs = fs / NLEN / 2.;
	fp = fp / NLEN / 2.;
	delete[]	data;
	*/
}
// }}}
