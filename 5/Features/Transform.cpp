/*
 * Copyright (c) 2017 Technical University of Košice (author: Martin Lojka)
 *
 * This file is part of EAR-TUKE.
 *
 * EAR-TUKE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EAR-TUKE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EAR-TUKE. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Transform.h"
#include <math.h>

using namespace Ear;

CFourier::CFourier()
{
	m_iSize = 0;
	m_iPower = 1;
}

CFourier::~CFourier()
{
}

void CFourier::getData(CDataContainer &_pData)
{
	unsigned int i,j,k,w,N,T;
	double re,im,wre,wim,a1,a2,b1,b2;

	//get data
	_pData.clear(); actualize(_pData);
	if(!_pData.size()) return;

	//compute FFT width if needed
	i = 1; j = 0;
	if(m_iSize < _pData.size()) {
		while(i < _pData.size()){i <<= 1; j++;}
		m_iSize = i;
		m_iPower = j - 1;
		m_iFFT = m_iSize / 2; //only half of the fft
	}

	//reserve space for computation
	_pData.reserve(m_iSize);
	_pData.size() = m_iSize;

	//bit reverse
	for(i = 0, j = 0; i < m_iSize - 1; i+=2) {
		if(j > i)	{
			a1 = _pData[j];      		b1 = _pData[j+1];
			_pData[j] = _pData[i];  _pData[j+1] = _pData[i+1];
			_pData[i] = a1;      		_pData[i+1] = b1;
		}

		k = m_iFFT;
		while(j >= k){ j -= k; k /= 2; }
		j += k;
	}

	//FFT butterfly
	for (N = 2; N <= m_iFFT; N += N) {
		re = cos(EAR_2PI / N);	wre = 1.0;
		im = sin(EAR_2PI / N);	wim = 0.0;
		T = N / 2; //max twidlle factor

		for (k = 0; k < T; k++) {
			for (j = k; j < m_iFFT; j += N) {
				i = 2 * j; //real position in the array
				w = i + 2 * T; //real position in the array
				a1 = wre * _pData[w] - wim * _pData[w+1];
				b1 = wre * _pData[w+1] + wim * _pData[w];
				_pData[w] = _pData[i] - a1;
				_pData[w+1] = _pData[i+1] - b1;
				_pData[i] += a1;
				_pData[i+1] += b1;
			}
			a2 = re * wre - im * wim;
			b2 = re * wim + im * wre;
			wre = a2;
			wim = b2;
		}
	}

	//FFT separation and join (last stage of the real FFT)
	N = m_iSize;
	wre = re = cos(EAR_2PI / N);
	wim = im = sin(EAR_2PI / N);

	//zero frequency
	_pData[0] += _pData[1];
	if(_pData[0] < 0) _pData[0] *= -1;

	//do not use first element
	for (i = 2; i < m_iFFT; i += 2) {

		a1 = _pData[i] + _pData[m_iSize - i];
		b1 = (_pData[m_iSize - i] - _pData[i]) * wim - (_pData[i+1] + _pData[m_iSize - i + 1]) * wre;

		a2 = _pData[i+1] - _pData[m_iSize - i + 1];
		b2 = (_pData[m_iSize - i] - _pData[i]) * wre + (_pData[i+1] + _pData[m_iSize - i + 1]) * wim;

		_pData[i] 							= (a1 - b1) / 2.0;
		_pData[m_iSize - i]			= (a1 + b1) / 2.0;

		_pData[i+1] 						= (b2 + a2) / 2.0;
		_pData[m_iSize - i + 1] = (b2 - a2) / 2.0;

		a2 = re * wre - im * wim;
		b2 = re * wim + im * wre;
		wre = a2;
		wim = b2;
	}

	for(i=2, j=1; i<_pData.size(); i+=2, j++)
		_pData[j] = sqrt((_pData[i] * _pData[i]) + (_pData[i+1] * _pData[i+1]));

	_pData.size() /= 2;
	_pData.freq() /= 2;
}

CDct::CDct(unsigned int _iSize) : ADataProcessor()
{
	m_iOutputSize	= _iSize;
	m_iInputSize = 0;
	m_pfCos = NULL;
}


CDct::~CDct()
{
	unsigned int i;

	if(m_pfCos)
	{
		for(i=0;i<m_iOutputSize;i++)
			delete[] m_pfCos[i];

		delete[] m_pfCos;
	}
}

void CDct::getData(CDataContainer &_pData)
{
	unsigned int i,j; float norm; //float x;

	m_Tmp.clear(); actualize(m_Tmp);
	if(!m_Tmp.size()){_pData.size() =  0; return;}

	_pData.reserve(m_iOutputSize); _pData.clear(); _pData.size() = m_iOutputSize;
	norm = sqrt(2.0 / m_Tmp.size());

	if(m_iInputSize != m_Tmp.size()) {initDct(m_Tmp.size());}

	/*
	for(i=0; i<m_iOutputSize; i++)
	{
		_pData[i] = 0; x = (float)(i+1) * EAR_PI / m_Tmp.size();
		for(j=0; j<m_Tmp.size(); j++)
			_pData[i] += m_Tmp[j] * cos(x*(1+j-0.5));

		_pData[i] *= norm;
	}*/


	for(i=0; i<m_iOutputSize; i++)
	{
		_pData[i] = 0;
		for(j=0; j<m_Tmp.size(); j++)
			_pData[i] += m_Tmp[j] * m_pfCos[i][j];

		_pData[i] *= norm;
	}

/*
	for(i=0; i<m_iOutputSize; i++)
	{
		_pData[i] = 0;
		//_pData[i] += 0.5 * m_Tmp[j] * m_pfCos[i][j];

		for(j=0; j<m_iInputSize; j++)
			_pData[i] += m_Tmp[j] * m_pfCos[i][j];

		//_pData[i] /= (float)m_iInputSize;
		_pData[i] *= sqrt(2.0 / m_iInputSize);

	}*/

}

void CDct::initDct(unsigned int _iSize)
{
	unsigned int i,j; float x;

	if(m_pfCos)
	{
		for(i=0;i<m_iOutputSize;i++)
			delete[] m_pfCos[i];

		delete m_pfCos;
	}

	m_iInputSize = _iSize;
	m_pfCos = new float*[m_iOutputSize];

	for(i=0; i<m_iOutputSize; i++)
	{
		x = (float)(i+1) * EAR_PI / _iSize;
		m_pfCos[i] = new float[_iSize];
		for(j=0; j<_iSize; j++)
			m_pfCos[i][j] = cos(x*(1+j-0.5));
	}
}
