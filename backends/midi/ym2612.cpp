/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
 *
 * YM2612 tone generation code written by Tomoaki Hayasaka.
 * Used under the terms of the GNU General Public License.
 * Adpated to ScummVM by Jamieson Christian.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include <math.h>
#include <string>
#include "common/util.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

////////////////////////////////////////
//
// Miscellaneous
//
////////////////////////////////////////

#define BASE_FREQ 250
#define FIXP_SHIFT  16

static int *sintbl = 0;
static int *powtbl = 0;
static int *frequencyTable = 0;
static int *keycodeTable = 0;
static int *keyscaleTable = 0;
static int *attackOut = 0;

////////////////////////////////////////
//
// Class declarations
//
////////////////////////////////////////

class Operator2612;
class Voice2612;
class MidiChannel_YM2612;
class MidiDriver_YM2612_Internal;

class Operator2612 {
protected:
	Voice2612 *_owner;
	enum State { _s_ready, _s_attacking, _s_decaying, _s_sustaining, _s_releasing };
	State _state;
	int32 _currentLevel;
	int _frequency;
	uint32 _phase;
	int _lastOutput;
	int _feedbackLevel;
	int _detune;
	int _multiple;
	int32 _totalLevel;
	int _keyScale;
	int _velocity;
	int _specifiedTotalLevel;
	int _specifiedAttackRate;
	int _specifiedDecayRate;
	int _specifiedSustainLevel;
	int _specifiedSustainRate;
	int _specifiedReleaseRate;
	int _tickCount;
	int _attackTime;
	int32 _decayRate;
	int32 _sustainLevel;
	int32 _sustainRate;
	int32 _releaseRate;

public:
	Operator2612 (Voice2612 *owner);
	~Operator2612();
	void feedbackLevel(int level);
	void setInstrument(byte const *instrument);
	void velocity(int velo);
	void keyOn();
	void keyOff();
	void frequency(int freq);
	int nextTick(uint16 rate, int phaseShift);
};

class Voice2612 {
protected:
	Operator2612 *_opr[4];
	int _velocity;
	int _control7;
	int _note;
	int _frequencyOffs;
	int _frequency;
	int _algorithm;

public:
	uint16 _rate;

public:
	Voice2612();
	~Voice2612();
	void setControlParameter(int control, int value);
	void setInstrument(byte const *instrument);
	void velocity(int velo);
	void nextTick(int *outbuf, int buflen);
	void noteOn(int n, int onVelo);
	bool noteOff(int note);
	void pitchBend(int value);
	void recalculateFrequency();
};

class MidiChannel_YM2612 : public MidiChannel {
protected:
	Voice2612 *_voice;

public:
	MidiChannel_YM2612();
	virtual ~MidiChannel_YM2612();

	// MidiChannel interface
	MidiDriver *device() { return 0; }
	byte getNumber() { return 0; }
	void release() { }
	void send(uint32 b) { }
	void noteOff(byte note);
	void noteOn(byte note, byte onVelo);
	void programChange(byte program) { }
	void pitchBend(int16 value);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value) { }
	void sysEx_customInstrument(uint32 type, byte *instr);

	void nextTick(int *outbuf, int buflen);
	void rate(uint16 r);
};

class MidiDriver_YM2612 : public MidiDriver {
protected:
	MidiChannel_YM2612 *_channel[16];
	int _volume;
	int _rate;

	bool _isOpen;
	SoundMixer *_mixer;
	typedef void TimerCallback(void *);
	TimerCallback *_timer_proc;
	void *_timer_param;
	int _next_tick;
	int _samples_per_tick;

protected:
	static void createLookupTables();
	void nextTick(int16 *buf1, int buflen);
	int volume(int val = -1) { if (val >= 0) _volume = val; return _volume; }
	void rate(uint16 r);

	void generate_samples(int16 *buf, int len);
	static void premix_proc(void *param, int16 *buf, uint len);

public:
	MidiDriver_YM2612(SoundMixer *mixer);
	virtual ~MidiDriver_YM2612();

	int open();
	void close();
	void send(uint32 b);
	void send(byte channel, uint32 b); // Supports higher than channel 15
	uint32 property(int prop, uint32 param) { return 0; }

	void setPitchBendRange(byte channel, uint range) { }
	void sysEx(byte *msg, uint16 length);

	void setTimerCallback(void *timer_param, void (*timer_proc)(void *));
	uint32 getBaseTempo() { return 1000000 / BASE_FREQ; }

	MidiChannel *allocateChannel() { return 0; }
	MidiChannel *getPercussionChannel() { return 0; }
};

////////////////////////////////////////
//
// Operator2612 implementation
//
////////////////////////////////////////

Operator2612::Operator2612 (Voice2612 *owner) :
_owner (owner),
_state (_s_ready),
_currentLevel ((int32)0x7f << 15),
_phase (0),
_lastOutput (0),
_feedbackLevel (0),
_detune (0),
_multiple (1),
_keyScale (0),
_specifiedTotalLevel (127),
_specifiedAttackRate (0),
_specifiedDecayRate (0),
_specifiedSustainRate (0),
_specifiedReleaseRate (15)
{
	velocity(0);
}

Operator2612::~Operator2612()
{ }

void Operator2612::velocity(int velo) {
	_velocity = velo;
	_totalLevel = ((int32)_specifiedTotalLevel << 15) +
	              ((int32)(127-_velocity) << 13);
	_sustainLevel = ((int32)_specifiedSustainLevel << 17);
}

void Operator2612::feedbackLevel(int level) {
	_feedbackLevel = level;
}

void Operator2612::setInstrument(byte const *instrument) {
	_detune = (instrument[8] >> 4) & 7;
	_multiple = instrument[8] & 15;
	_specifiedTotalLevel = instrument[12] & 127;
	_keyScale = (instrument[16] >> 6) & 3;
	_specifiedAttackRate = instrument[16] & 31;
	_specifiedDecayRate = instrument[20] & 31;
	_specifiedSustainRate = instrument[24] & 31;
	_specifiedSustainLevel = (instrument[28] >> 4) & 15;
	_specifiedReleaseRate = instrument[28] & 15;
	_state = _s_ready; // 本物ではどうなのかな?
	velocity(_velocity);
}

void Operator2612::keyOn() {
	_state = _s_attacking;
	_tickCount = 0;
	_phase = 0;			// どうも、実際こうらしい
	_currentLevel = ((int32)0x7f << 15); // これも、実際こうらしい
}

void Operator2612::keyOff() {
	if (_state != _s_ready)
		_state = _s_releasing;
}

void Operator2612::frequency(int freq) {
	double value; // Use for intermediate computations to avoid int64 arithmetic
	int r;

//	_frequency = freq;
	_frequency = freq / _owner->_rate;

	r = _specifiedAttackRate;
	if (r != 0) {
		r = r * 2 + (keyscaleTable[freq/262205] >> (3-_keyScale));
		if (r >= 64)
		r = 63; // するべきなんだろうとは思うんだけど (赤p.207)
	}

	r = 63 - r;
	if (_specifiedTotalLevel >= 128)
		value = 0;
	else {
		value = powtbl[(r&3) << 7];
		value *= 1 << (r >> 2);
		value *= 41; // r == 20 のとき、0-96[db] が 10.01[ms] == 41.00096
		value /= 1 << (15 + 5);
		value *= 127 - _specifiedTotalLevel;
		value /= 127;
	}
	_attackTime = (int32) value; // 1 秒 == (1 << 12)

	r = _specifiedDecayRate;
	if (r != 0) {
		r = r * 2 + (keyscaleTable[freq/262205] >> (3-_keyScale));
		if (r >= 64)
			r = 63;
	}
	value = (double) powtbl[(r&3) << 7] * (0x10 << (r>>2)) / 31;
	_decayRate = (int32) value;

	r = _specifiedSustainRate;
	if (r != 0) {
		r = r * 2 + (keyscaleTable[freq/262205] >> (3-_keyScale));
		if (r >= 64)
			r = 63;
	}
	value = (double) powtbl[(r&3) << 7] * (0x10 << (r>>2)) / 31;
	_sustainRate = (int32) value;

	r = _specifiedReleaseRate;
	if (r != 0) {
		r = r * 2 + 1;		// このタイミングで良いのかわからん
		r = r * 2 + (keyscaleTable[freq/262205] >> (3-_keyScale));
		// KS による補正はあるらしい。赤p.206 では記述されてないけど。
		if (r >= 64)
			r = 63;
	}
	value = (double) powtbl[(r&3) << 7] * (0x10 << (r>>2)) / 31;
	_releaseRate = (int32) value;
}

int Operator2612::nextTick(uint16 rate, int phaseShift) {
	// sampling ひとつ分進める
	switch (_state) {
	case _s_ready:
		return 0;
		break;
	case _s_attacking:
		++_tickCount;
		if (_attackTime <= 0) {
			_currentLevel = 0;
			_state = _s_decaying;
		} else {
			int i = (int) (((double)_tickCount * (1 << (12+10))) / ((double)rate * _attackTime));
			if (i >= 1024) {
				_currentLevel = 0;
				_state = _s_decaying;
			} else {
				_currentLevel = attackOut[i] << (31 - 8 - 16);
			}
		}
		break;
	case _s_decaying:
		_currentLevel += _decayRate / rate;
		if (_currentLevel >= _sustainLevel) {
			_currentLevel = _sustainLevel;
			_state = _s_sustaining;
		}
		break;
	case _s_sustaining:
		_currentLevel += _sustainRate / rate;
		if (_currentLevel >= ((int32)0x7f << 15)) {
			_currentLevel = ((int32)0x7f << 15);
			_state = _s_ready;
		}
		break;
	case _s_releasing:
		_currentLevel += _releaseRate / rate;
		if (_currentLevel >= ((int32)0x7f << 15)) {
			_currentLevel = ((int32)0x7f << 15);
			_state = _s_ready;
		}
		break;
	default:
		// ここには来ないはず
		break;
	};

	int32 level = _currentLevel + _totalLevel;
	int32 output = 0;
	if (level < ((int32)0x7f << 15)) {
		_phase &= 0x3ffff;
		phaseShift >>= 2;		// 正しい変調量は?  3 じゃ小さすぎで 2 じゃ大きいような。
		if (_feedbackLevel)
			phaseShift += (_lastOutput << (_feedbackLevel - 1)) / 1024;
		output = sintbl[((_phase >> 7) + phaseShift) & 0x7ff];
		output >>= (level >> 18);	// 正しい減衰量は?
		// Here is the original code, which requires 64-bit ints
//		output *= powtbl[511 - ((level>>25)&511)];
//		output >>= 16;
//		output >>= 1;
		// And here's our 32-bit trick for doing it. (Props to Fingolfin!)
//		int powVal = powtbl[511 - ((level>>9)&511)];
//		int outputHI = output / 256;
//		int powHI = powVal / 256;
//		output = (outputHI * powHI) / 2 + (outputHI * (powVal % 256) + powHI * (output % 256)) / 512;
		// And here's the even faster code.
		output = ((output >> 4) * (powtbl[511-((level>>9)&511)] >> 3)) / 1024;

		if (_multiple > 0)
//			_phase += (_frequency * _multiple) / rate;
			_phase += _frequency * _multiple; // / rate; already included
		else
//			_phase += _frequency / (rate << 1);
			_phase += _frequency / 2;
	}

	_lastOutput = output;
	return output;
}

////////////////////////////////////////
//
// Voice2612 implementation
//
////////////////////////////////////////

Voice2612::Voice2612() {
	_control7 = 127;
	_note = 40;
	_frequency = 440;
	_frequencyOffs = 0x2000;
	_algorithm = 7;
	int i;
	for (i = 0; i < ARRAYSIZE(_opr); ++i)
		_opr[i] = new Operator2612 (this);
	velocity(0);
}

Voice2612::~Voice2612() {
	int i;
	for (i = 0; i < ARRAYSIZE(_opr); ++i)
		delete _opr[i];
}

void Voice2612::velocity(int velo) {
	_velocity = velo;
#if 0
	int v = (velo * _control7) >> 7; // これだと精度良くないですね
#else
	int v = velo + (_control7 - 127) * 4;
#endif
	bool iscarrier[8][4] = {
		{ false, false, false,  true, }, //0
		{ false, false, false,  true, }, //1
		{ false, false, false,  true, }, //2
		{ false, false, false,  true, }, //3
		{ false,  true, false,  true, }, //4
		{ false,  true,  true,  true, }, //5
		{ false,  true,  true,  true, }, //6
		{  true,  true,  true,  true, }, //7
	};
	int opr;
	for (opr = 0; opr < 4; opr++)
		if (iscarrier[_algorithm][opr])
			_opr[opr]->velocity(v);
		else
			_opr[opr]->velocity(127);
}

void Voice2612::setControlParameter(int control, int value) {
	switch (control) {
	case 7:
		_control7 = value;
		velocity(_velocity);
		break;
	case 123:
		// All notes off
		noteOff(_note);
	};
}

void Voice2612::setInstrument(byte const *instrument) {
	if (instrument == NULL)
		return;

	_algorithm = instrument[32] & 7;
	_opr[0]->feedbackLevel((instrument[32] >> 3) & 7);
	_opr[1]->feedbackLevel(0);
	_opr[2]->feedbackLevel(0);
	_opr[3]->feedbackLevel(0);
	_opr[0]->setInstrument(instrument + 0);
	_opr[1]->setInstrument(instrument + 2);
	_opr[2]->setInstrument(instrument + 1);
	_opr[3]->setInstrument(instrument + 3);
}

void Voice2612::nextTick(int *outbuf, int buflen) {
	if (_velocity == 0)
		return;

	int i;
	int d1, d2, d3, d4;
	switch (_algorithm) {
	case 0:
		for (i = 0; i < buflen; ++i) {
			d1 = _opr[0]->nextTick(_rate, 0);
			d2 = _opr[1]->nextTick(_rate, d1);
			d3 = _opr[2]->nextTick(_rate, d2);
			d4 = _opr[3]->nextTick(_rate, d3);
			outbuf[i] += d4;
		}
		break;
	case 1:
		for (i = 0; i < buflen; ++i) {
			d1 = _opr[0]->nextTick(_rate, 0);
			d2 = _opr[1]->nextTick(_rate, 0);
			d3 = _opr[2]->nextTick(_rate, d1+d2);
			d4 = _opr[3]->nextTick(_rate, d3);
			outbuf[i] += d4;
		}
		break;
	case 2:
		for (i = 0; i < buflen; ++i) {
			d1 = _opr[0]->nextTick(_rate, 0);
			d2 = _opr[1]->nextTick(_rate, 0);
			d3 = _opr[2]->nextTick(_rate, d2);
			d4 = _opr[3]->nextTick(_rate, d1+d3);
			outbuf[i] += d4;
		}
		break;
	case 3:
		for (i = 0; i < buflen; ++i) {
			d1 = _opr[0]->nextTick(_rate, 0);
			d2 = _opr[1]->nextTick(_rate, d1);
			d3 = _opr[2]->nextTick(_rate, 0);
			d4 = _opr[3]->nextTick(_rate, d2+d3);
			outbuf[i] += d4;
		}
		break;
	case 4:
		for (i = 0; i < buflen; ++i) {
			d1 = _opr[0]->nextTick(_rate, 0);
			d2 = _opr[1]->nextTick(_rate, d1);
			d3 = _opr[2]->nextTick(_rate, 0);
			d4 = _opr[3]->nextTick(_rate, d3);
			outbuf[i] += d2 + d4;
		}
		break;
	case 5:
		for (i = 0; i < buflen; ++i) {
			d1 = _opr[0]->nextTick(_rate, 0);
			d2 = _opr[1]->nextTick(_rate, d1);
			d3 = _opr[2]->nextTick(_rate, d1);
			d4 = _opr[3]->nextTick(_rate, d1);
			outbuf[i] += d2 + d3 + d4;
		}
		break;
	case 6:
		for (i = 0; i < buflen; ++i) {
			d1 = _opr[0]->nextTick(_rate, 0);
			d2 = _opr[1]->nextTick(_rate, d1);
			d3 = _opr[2]->nextTick(_rate, 0);
			d4 = _opr[3]->nextTick(_rate, 0);
			outbuf[i] += d2 + d3 + d4;
		}
		break;
	case 7:
		for (i = 0; i < buflen; ++i) {
			d1 = _opr[0]->nextTick(_rate, 0);
			d2 = _opr[1]->nextTick(_rate, 0);
			d3 = _opr[2]->nextTick(_rate, 0);
			d4 = _opr[3]->nextTick(_rate, 0);
			outbuf[i] += d1 + d2 + d3 + d4;
		}
		break;
	};
}

void Voice2612::noteOn(int n, int onVelo) {
	_note = n;
	velocity(onVelo);
	recalculateFrequency();
	int i;
	for (i = 0; i < ARRAYSIZE(_opr); i++)
		_opr[i]->keyOn();
}

bool Voice2612::noteOff(int note) {
	if (_note != note)
		return false;
	int i;
	for (i = 0; i < ARRAYSIZE(_opr); i++)
		_opr[i]->keyOff();
	return true;
}

void Voice2612::pitchBend(int value) {
	_frequencyOffs = value;
	recalculateFrequency();
}

void Voice2612::recalculateFrequency() {
	// MIDI とも違うし....
	// どういう仕様なんだろうか?
	// と思ったら、なんと、これ (↓) が正解らしい。
	int32 basefreq = frequencyTable[_note];
	int cfreq = frequencyTable[_note - (_note % 12)];
	int oct = _note / 12;
	int fnum = (int) (((double)basefreq * (1 << 13)) / cfreq); // OPL の fnum と同じようなもの。
	fnum += _frequencyOffs - 0x2000;
	if (fnum < 0x2000) {
		fnum += 0x2000;
		oct--;
	}
	if (fnum >= 0x4000) {
		fnum -= 0x2000;
		oct++;
	}

	// _frequency は最終的にバイアス 256*1024 倍
	_frequency = (int) ((frequencyTable[oct*12] * (double)fnum) / 8);

	int i;
	for (i = 0; i < ARRAYSIZE(_opr); i++)
		_opr[i]->frequency(_frequency);
}

////////////////////////////////////////
//
// MidiChannel_YM2612
//
////////////////////////////////////////

MidiChannel_YM2612::MidiChannel_YM2612() {
	_voice = new Voice2612();
}

MidiChannel_YM2612::~MidiChannel_YM2612() {
	delete _voice;
}

void MidiChannel_YM2612::noteOn(byte note, byte onVelo) {
	_voice->noteOn(note, onVelo);
}

void MidiChannel_YM2612::noteOff(byte note) {
	_voice->noteOff(note);
}

void MidiChannel_YM2612::controlChange(byte control, byte value) {
  // いいのかこれで?
	_voice->setControlParameter(control, value);
}

void MidiChannel_YM2612::sysEx_customInstrument(uint32 type, byte *fmInst) {
	if (type == 'EUP ')
		_voice->setInstrument(fmInst);
}

void MidiChannel_YM2612::pitchBend(int16 value) {
  // いいのかこれで?
	_voice->pitchBend(value);
}

void MidiChannel_YM2612::nextTick(int *outbuf, int buflen) {
	_voice->nextTick(outbuf, buflen);
}

void MidiChannel_YM2612::rate(uint16 r) {
	_voice->_rate = r;
}

////////////////////////////////////////
//
// MidiDriver_YM2612
//
////////////////////////////////////////

MidiDriver_YM2612::MidiDriver_YM2612(SoundMixer *mixer) :
_mixer(mixer)
{
	_isOpen = false;
	_timer_proc = 0;
	_timer_param = 0;
	_next_tick = 0;
	_samples_per_tick = (_mixer->getOutputRate() << FIXP_SHIFT) / BASE_FREQ;

	createLookupTables();
	_volume = 256;
	int i;
	for (i = 0; i < ARRAYSIZE(_channel); i++)
		_channel[i] = new MidiChannel_YM2612;
	rate(_mixer->getOutputRate());
}

MidiDriver_YM2612::~MidiDriver_YM2612() {
	int i;
	for (i = 0; i < ARRAYSIZE(_channel); i++)
		delete _channel[i];
	delete sintbl;
	delete powtbl;
	delete frequencyTable;
	delete keycodeTable;
	delete keyscaleTable;
	delete attackOut;
	sintbl = powtbl = frequencyTable = keycodeTable = keyscaleTable = attackOut = 0;
}

int MidiDriver_YM2612::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_mixer->setupPremix(premix_proc, this);
	_isOpen = true;
	return 0;
}

void MidiDriver_YM2612::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	// Detach the premix callback handler
	_mixer->setupPremix(0, 0);
}

void MidiDriver_YM2612::setTimerCallback(void *timer_param, void (*timer_proc)(void *)) {
	_timer_proc = (TimerCallback *) timer_proc;
	_timer_param = timer_param;
}

void MidiDriver_YM2612::send(uint32 b) {
	send(b & 0xF, b & 0xFFFFFFF0);
}

void MidiDriver_YM2612::send(byte chan, uint32 b) {
	//byte param3 = (byte) ((b >> 24) & 0xFF);
	byte param2 = (byte) ((b >> 16) & 0xFF);
	byte param1 = (byte) ((b >>  8) & 0xFF);
	byte cmd    = (byte) (b & 0xF0);
	if (chan > ARRAYSIZE(_channel))
		return;

	switch (cmd) {
	case 0x80:// Note Off
		_channel[chan]->noteOff(param1);
		break;
	case 0x90: // Note On
		_channel[chan]->noteOn(param1, param2);
		break;
	case 0xA0: // Aftertouch
		break; // Not supported.
	case 0xB0: // Control Change
		_channel[chan]->controlChange(param1, param2);
		break;
	case 0xC0: // Program Change
		_channel[chan]->programChange(param1);
		break;
	case 0xD0: // Channel Pressure
		break; // Not supported.
	case 0xE0: // Pitch Bend
		_channel[chan]->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	case 0xF0: // SysEx
		// We should never get here! SysEx information has to be
		// sent via high-level semantic methods.
		warning("MidiDriver_YM2612: Receiving SysEx command on a send() call");
		break;

	default:
		warning("MidiDriver_YM2612: Unknown send() command 0x%02X", cmd);
	}
}

void MidiDriver_YM2612::sysEx(byte *msg, uint16 length) {
	if (msg[0] != 0x7C || msg[1] >= ARRAYSIZE(_channel))
		return;
	_channel[msg[1]]->sysEx_customInstrument('EUP ', &msg[2]);
}

void MidiDriver_YM2612::premix_proc(void *param, int16 *buf, uint len) {
	((MidiDriver_YM2612 *) param)->generate_samples(buf, len);
}

void MidiDriver_YM2612::generate_samples(int16 *data, int len) {
	int step;

	do {
		step = len;
		if (step > (_next_tick >> FIXP_SHIFT))
			step = (_next_tick >> FIXP_SHIFT);
		nextTick(data, step);

		_next_tick -= step << FIXP_SHIFT;
		if (!(_next_tick >> FIXP_SHIFT)) {
			if (_timer_proc)
				(*_timer_proc)(_timer_param);
			_next_tick += _samples_per_tick;
		}
		data += step * 2; // Stereo means * 2
		len -= step;
	} while (len);
}

void MidiDriver_YM2612::nextTick(int16 *buf1, int buflen) {
	int *buf0 = new int [buflen];
	memset(buf0, 0, sizeof(buf0[0]) * buflen);

	int i;
	for (i = 0; i < ARRAYSIZE(_channel); i++)
		_channel[i]->nextTick(buf0, buflen);

	for (i = 0; i < buflen; ++i)
		buf1[i*2+1] = buf1[i*2] = ((buf0[i] * volume()) >> 10) & 0xffff;
	delete [] buf0;
}

void MidiDriver_YM2612::rate(uint16 r)
{
	_rate = r;
	int i;
	for (i = 0; i < ARRAYSIZE(_channel); i++)
		_channel[i]->rate(r);
}

#define M_PI 3.14159265358979323846
void MidiDriver_YM2612::createLookupTables() {
	{
		int i;
		sintbl = new int [2048];
		for (i = 0; i < 2048; i++)
			sintbl[i] = (int)(0xffff * sin(i/2048.0*2.0*M_PI));
	}

	{
		int i;
		powtbl = new int [1025];
		for (i = 0; i <= 1024; i++)
			powtbl[i] = (int)(0x10000 * pow(2.0, (i-512)/512.0));
	}

	{
		int i;
		int block;

		static int fnum[] = {
			0x026a, 0x028f, 0x02b6, 0x02df,
			0x030b, 0x0339, 0x036a, 0x039e,
			0x03d5, 0x0410, 0x044e, 0x048f,
		};

		// (int)(880.0 * 256.0 * pow(2.0, (note-0x51)/12.0)); // バイアス 256 倍
		// 0x45 が 440Hz (a4)、0x51 が 880Hz (a5) らしい
		frequencyTable = new int [120];
		for (block = -1; block < 9; block++) {
			for (i = 0; i < 12; i++) {
				double freq = fnum[i] * (166400.0 / 3) * pow(2.0, block-21);
				frequencyTable[(block+1)*12+i] = (int)(256.0 * freq);
			}
		}

		keycodeTable = new int [120];
		// detune 量の計算や KS による rate 変換に使うんじゃないかな
		for (block = -1; block < 9; block++) {
			for (i = 0; i < 12; i++) {
				// see p.204
				int  f8 = (fnum[i] >>  7) & 1;
				int  f9 = (fnum[i] >>  8) & 1;
				int f10 = (fnum[i] >>  9) & 1;
				int f11 = (fnum[i] >> 10) & 1;
				int  n4 = f11;
				int  n3 = f11&(f10|f9|f8) | (~f11&f10&f9&f8);
				int note = n4*2 + n3;
				// see p.207
				keycodeTable[(block+1)*12+i] = block*4 + note;
			}
		}
	}

	{
		int freq;
		keyscaleTable = new int [8192];
		keyscaleTable[0] = 0;
		for (freq = 1; freq < 8192; freq++) {
			keyscaleTable[freq] = (int)(log((double)freq) / 9.03 * 32.0) - 1;
			// 8368[Hz] (o9c) で 32くらい。9.03 =:= ln 8368
		}
	}

	{
		int i;
		attackOut = new int [1024];
		for (i = 0; i < 1024; i++)
			attackOut[i] = (int)(((0x7fff+0x03a5)*30.0) / (30.0+i)) - 0x03a5;
	}
}

////////////////////////////////////////
//
// MidiDriver_YM2612 factory
//
////////////////////////////////////////

MidiDriver *MidiDriver_YM2612_create(SoundMixer *mixer) {
	return new MidiDriver_YM2612(mixer);
}
