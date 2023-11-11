#include "sources.h"
#include "config.h"
#include <math.h>
#include <stdint.h>
#include "backend/hw/headphone.h"
#include "backend/printfn/printfn.h"
#include "util.h"
#include "arm_math.h"
#ifdef AUDIO_SOURCE_WAVEFORM
#include "backend/helix/pub/mp3dec.h"
#include "backend/helix/pub/mp3common.h"
#include "blocks/waveforms/mp3_data.h"
#include "backend/polyfifo.h"
#include <stdlib.h>
#endif
#ifdef AUDIO_SOURCE_SYNTH
#include "backend/tsf/tsf_fastmath.h"
#define TSF_IMPLEMENTATION
#define TSF_NO_STDIO
#include "backend/tsf/tsf.h"
#define TML_IMPLEMENTATION
#define TML_NO_STDIO
#include "backend/tsf/tml.h"
#include "backend/tsf/tsf_data.h"
#endif

/** @brief Channel response for unit-testing LMS algorithm */
const float h_sim_int[] = {10, -8, 6, -4, 2, -1, 1};

/** @brief Reference input signal to pass through simualted channel h, used for 
unit-testing LMS filter */
const float test_x[] = {14.8932485, -36.4125825, 49.6098742, -35.4846256, 33.5805224, 
-37.2492035, 20.8040165, -15.8582675, 13.4601244, -23.3828053, 23.2106558, 
-29.2438718, 20.4087478, -7.21076326, 6.33646669, 7.29170102, 2.56950095, 
-9.34445857, 9.55170621, -16.9394193, -0.260314187, 11.902817, -9.38023172, 
7.48980224, 4.40947625, 0.554168547, 2.18526666, 10.4388343, 0.895980308, 
1.28314079, -7.22703933, -0.528340904, 12.9917571, -26.003008, 21.7181535, 
-34.105149, 34.4417038, -16.2093727, 9.7912756, -5.11442585, -23.7706529, 
24.0264104, -41.0469766, 8.4977257, -1.0262878, -11.6924303, 14.0071515, 
-4.39772103, 9.71108348, -15.6537233, 14.3129292, -10.1725884, 14.3665133, 
-4.00377257, 11.6911611, -9.52909365, 4.82707489, 7.2190423, -27.5685094, 
17.9532188, -24.7068563, 13.8368214, -1.46003582, 7.97696244};

/** @brief Result of passing x through h, used tor unit-testing LMS filter */
const float test_y[]={2.02369089, -2.25835397, 2.22944568, 0.337563701, 1.00006082, 
-1.66416447, -0.590034564, -0.278064164, 0.422715691, -1.6702007, 0.471634326, 
-1.2128472, 0.0661900484, 0.652355889, 0.327059967, 1.0826335, 1.00607711, 
-0.650907737, 0.257056157, -0.944377806, -1.32178852, 0.924825933, 
4.98490753e-05, -0.0549189146, 0.911127266, 0.594583697, 0.350201174, 
1.25025123, 0.929789459, 0.239763257, -0.690361103, -0.651553642, 1.19210187, 
-1.61183039, -0.0244619366, -1.94884718, 1.02049801, 0.861716302, 0.00116208348, 
-0.0708372132, -2.48628392, 0.581172323, -2.19243492, -2.31928031, 0.0799337103, 
-0.948480984, 0.411490621, 0.676977806, 0.857732545, -0.691159125, 0.449377623, 
0.10063335, 0.826069998, 0.53615708, 0.897888426, -0.131937868, -0.147201456, 
1.00777341, -2.12365546, -0.504586406, -1.27059445, -0.382584803, 0.648679262, 
0.825727149};

/** @brief The next element from wavetable based waveforms */
uint_fast32_t dist_idx = 0;
uint_fast32_t test_y_idx = 0;
uint_fast32_t test_x_idx = 0;

/** @brief The current angle for sine/cosine sources */
float trig_ang = 0;

/** @brief The current frequency to use for sine/cosine outputs, default to 1kHz */
float trig_freq = 1e3;

#ifdef AUDIO_SOURCE_WAVEFORM
// Large buffer for all thus far decoded mp3 data
int16_t mp3_bigbuf[4096];
POLYFIFO_DECLARE(int16_t);
POLYFIFO_DEFINE(int16_t, mp3_bigfifo, NUMEL(mp3_bigbuf), mp3_bigbuf);

// MP3 decoder variables
const char * mp3_ptr = mp3_data;
size_t mp3_bytes_left = NUMEL(mp3_data);
HMP3Decoder hMP3Decoder;

// Final output buffer and function
int16_t waveform_buf[AUDIO_BLOCKSIZE];
void blocks_sources_waveform(float * sample_block);
#endif

void blocks_sources_update(void){
	// Fill the decoded MP3 buffer as much as possible
	int16_t decoded_mp3frame[MAX_NSAMP*MAX_NGRAN];

	while(POLYFIFO_NUMEMPTY(mp3_bigfifo) > NUMEL(decoded_mp3frame)){
		// Update mp3_ptr and mp3_bytes_left for the next frame
		int offset;
		do{
			offset = MP3FindSyncWord((unsigned char *) mp3_ptr, mp3_bytes_left);
			if(offset >= 0){
				// Found another sync word, parse data
				mp3_bytes_left -= offset;
				mp3_ptr += offset;
			}else{
				// Did not fint sync word, restart from start of data
				mp3_bytes_left = NUMEL(mp3_data);
				mp3_ptr = mp3_data;
			}
		}while(offset < 0);

		// Parse data stream
		int res = MP3Decode(hMP3Decoder, (unsigned char **)&mp3_ptr, (int *) &mp3_bytes_left, (short *) decoded_mp3frame, 0);
		if(res){
			printf("ERROR: MP3 decode error %d. See 'mp3dec.h'.\nFile is possibly corrupt. Attempting playback despite this.\n", res);
		}else{
			// No error, send data to buffer

			// Determine how many samples were generated
			MP3FrameInfo mp3FrameInfo;
			MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
			size_t n_samps = mp3FrameInfo.outputSamps;

			// Queue them in buffer
			size_t i;
			for(i = 0; i < n_samps; i++){
				//static int foo = 0;
				//foo = (foo+1)%128;
				POLYFIFO_WRITE(mp3_bigfifo, decoded_mp3frame[i]);
				//POLYFIFO_WRITE(mp3_bigfifo, foo);
			}
		}
	}

	// Write to the ourput waveform buffer
	int_fast32_t i;
	if(POLYFIFO_NUMFULL(mp3_bigfifo) < AUDIO_BLOCKSIZE){
		halt_error("ERROR: MP3 buffer underrun.\nHalting execution.");
	}
	for(i = 0; i < AUDIO_BLOCKSIZE; i++){
		POLYFIFO_READ(mp3_bigfifo, &waveform_buf[i]);
	}
	
	//Compute the relative change in angle between each sample for the requested frequency
	volatile float freq;	//Declare volatile to ensure the math operations are not re-ordered into the atomic block
	ATOMIC(freq = trig_freq);
	const float delta_ang = M_TWOPI * freq / AUDIO_SAMPLE_RATE;
 	trig_ang += delta_ang * AUDIO_BLOCKSIZE;
	//Keep trig_ang in range [0, 2*pi] to keep float epsilon small
	//For reasonable frequencies and block-sizes, repeated subtraction is
	//cheaper than using a divide and multiply operation (and has clearer
	//intent).
	while(trig_ang > M_TWOPI){
		trig_ang -= M_TWOPI;
	}
}

#ifdef AUDIO_SOURCE_SYNTH
// Holds the global instance pointer
tsf* g_TinySoundFont;

// Holds global MIDI playback state
float g_tsf_Msec;               	//current playback time
static tml_message* g_MidiMessage;  //next message to be played

int32_t track;						//current audio track to play
uint_fast32_t seed;					//PRNG seed for track selection

bool flag_next_track = false;			// Flag to set to switch to next track

// Generate the next AUDIO_BLOCKSIZE samples
void blocks_sources_synth(float * sample_block);

// Reset the synth state (i.e. for track changes)
void blocks_sources_synth_reset(void);
#endif

void blocks_sources_init(void){
	#ifdef AUDIO_SOURCE_WAVEFORM
	// Initialize MP3 decoder
	hMP3Decoder = MP3InitDecoder();

	// Verify MP3 data
	size_t offset = MP3FindSyncWord((unsigned char *) mp3_ptr, NUMEL(mp3_data));
	MP3FrameInfo frame_info;
	int info_res = MP3GetNextFrameInfo(hMP3Decoder, &frame_info, (unsigned char *) &mp3_data[offset]);
	if(info_res == ERR_MP3_INVALID_FRAMEHEADER){
		halt_error("Error: invalid MP3 header in selected data!\nHalting program execution.");
	}

	MP3DecInfo *mp3DecInfo = (MP3DecInfo *)hMP3Decoder;
	if(mp3DecInfo->nChans != 1){
		halt_error("Error: only mono MP3 data supported!\nHalting program execution.");
	}
	if(mp3DecInfo->samprate != AUDIO_SAMPLE_RATE){
		halt_error("Error: MP3 sample rate must be " xstr(AUDIO_SAMPLE_RATE) "\nHalting program execution.");
	}
	// Perform an update to fill the initially empty buffer
	blocks_sources_update();
	#endif
	#ifdef AUDIO_SOURCE_SYNTH

	BUILD_BUG_ON(!ISPOW2(TSF_RENDER_EFFECTSAMPLEBLOCK));	//Require TSF sample blocks to be power of two to ensure an even multiple of AUDIO_BLOCKSIZE
	BUILD_BUG_ON(AUDIO_BLOCKSIZE < TSF_RENDER_EFFECTSAMPLEBLOCK);	//Require TSF sample blocks to be less than AUDIO_BLOCKSIZE
	// Load soundfont for synth
	//g_TinySoundFont = tsf_load_memory(tsf_soundfont, sizeof(tsf_soundfont));

	//This is a minimal SoundFont with a single loopin saw-wave sample/instrument/preset (484 bytes)
	const static unsigned char MinimalSoundFont[] =
	{
		#define TEN0 0,0,0,0,0,0,0,0,0,0
		'R','I','F','F',220,1,0,0,'s','f','b','k',
		'L','I','S','T',88,1,0,0,'p','d','t','a',
		'p','h','d','r',76,TEN0,TEN0,TEN0,TEN0,0,0,0,0,TEN0,0,0,0,0,0,0,0,255,0,255,0,1,TEN0,0,0,0,
		'p','b','a','g',8,0,0,0,0,0,0,0,1,0,0,0,'p','m','o','d',10,TEN0,0,0,0,'p','g','e','n',8,0,0,0,41,0,0,0,0,0,0,0,
		'i','n','s','t',44,TEN0,TEN0,0,0,0,0,0,0,0,0,TEN0,0,0,0,0,0,0,0,1,0,
		'i','b','a','g',8,0,0,0,0,0,0,0,2,0,0,0,'i','m','o','d',10,TEN0,0,0,0,
		'i','g','e','n',12,0,0,0,54,0,1,0,53,0,0,0,0,0,0,0,
		's','h','d','r',92,TEN0,TEN0,0,0,0,0,0,0,0,50,0,0,0,0,0,0,0,49,0,0,0,34,86,0,0,60,0,0,0,1,TEN0,TEN0,TEN0,TEN0,0,0,0,0,0,0,0,
		'L','I','S','T',112,0,0,0,'s','d','t','a','s','m','p','l',100,0,0,0,86,0,119,3,31,7,147,10,43,14,169,17,58,21,189,24,73,28,204,31,73,35,249,38,46,42,71,46,250,48,150,53,242,55,126,60,151,63,108,66,126,72,207,
			70,86,83,100,72,74,100,163,39,241,163,59,175,59,179,9,179,134,187,6,186,2,194,5,194,15,200,6,202,96,206,159,209,35,213,213,216,45,220,221,223,76,227,221,230,91,234,242,237,105,241,8,245,118,248,32,252
	};
	g_TinySoundFont = tsf_load_memory(MinimalSoundFont, sizeof(MinimalSoundFont));

	//TODO: place converted soundfont in nonvolatile memory!
	
	//Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
	tsf_channel_set_bank_preset(g_TinySoundFont, 9, 128, 0);
	// Set the SoundFont rendering output mode to monophonic audio at full volume
	tsf_set_output(g_TinySoundFont, TSF_MONO, AUDIO_SAMPLE_RATE, -10.0f);
	seed = util_get_seed();
	
	blocks_sources_synth_reset();
	
	#endif
}

#ifdef AUDIO_SOURCE_SYNTH
void blocks_sources_synth_reset(void){
	tsf_reset(g_TinySoundFont);
	size_t track = util_rand_range(0, NUMEL(tsf_track_len)-1, &seed);
	g_MidiMessage = tml_load_memory(tsf_tracks[track], tsf_track_len[track]);
	g_tsf_Msec = 0.0f;
}
#endif

int blocks_sources_get_h_sim_len(void){
	return NUMEL(h_sim_int);
}

void blocks_sources_get_h_sim(float * h_sim){
	int i;
	for(i = 0; i < NUMEL(h_sim_int); i++){
		h_sim[i] = h_sim_int[i];
	}
}

void blocks_sources_zeros(float * sample_block){
	arm_fill_f32(0.0f, sample_block, AUDIO_BLOCKSIZE);
}

void blocks_sources_ones(float * sample_block){
	arm_fill_f32(1.0f, sample_block, AUDIO_BLOCKSIZE);
}

void blocks_sources_trig_setfreq(float frequency){
	ATOMIC(trig_freq = frequency);
}

void blocks_sources_sin(float * sample_block){
	int_fast32_t i;
	float ang = trig_ang;
	volatile float freq;	//Declare volatile to ensure the math operations are not re-ordered into the atomic block
	ATOMIC(freq = trig_freq);
	const float delta_ang = M_TWOPI * freq / AUDIO_SAMPLE_RATE;
	for(i = 0; i < AUDIO_BLOCKSIZE; i++){
		sample_block[i] = arm_sin_f32(ang);
		ang += delta_ang;
	}
}

void blocks_sources_cos(float * sample_block){
	int_fast32_t i;
	float ang = trig_ang;
	volatile float freq;	//Declare volatile to ensure the math operations are not re-ordered into the atomic block
	ATOMIC(freq = trig_freq);
	const float delta_ang = M_TWOPI * freq / AUDIO_SAMPLE_RATE;
	for(i = 0; i < AUDIO_BLOCKSIZE; i++){
		sample_block[i] = arm_cos_f32(ang);
		ang += delta_ang;
	}
}

void blocks_sources_music(float * sample_block){
	#ifdef AUDIO_SOURCE_WAVEFORM
	blocks_sources_waveform(sample_block);
	#endif
	#ifdef AUDIO_SOURCE_SYNTH
	blocks_sources_synth(sample_block);
	#endif
}

#ifdef AUDIO_SOURCE_WAVEFORM
void blocks_sources_waveform(float * sample_block){
	int_fast32_t i;
	for(i = 0; i < AUDIO_BLOCKSIZE; i++){
		sample_block[i] = waveform_buf[i] * (1.0f/INT16_MAX);
	}
}
#endif

#ifdef AUDIO_SOURCE_SYNTH
void blocks_sources_synth(float * sample_block){
	// If current track empty, randomly select another
	bool flag_next_track_shdw;
	ATOMIC({
		flag_next_track_shdw = flag_next_track;
		flag_next_track = false;
	});
	if(g_MidiMessage == NULL || flag_next_track_shdw){
		blocks_sources_synth_reset();
	}

	
	// Generate the next AUDIO_BLOCKSIZE samples
	int subsample_block = TSF_RENDER_EFFECTSAMPLEBLOCK;	// TSF generates samples in subblocks
	int sample_count = AUDIO_BLOCKSIZE;					// Total number of samples to generate
	for( ; sample_count; sample_count -= subsample_block, sample_block += subsample_block){

		//We progress the MIDI playback and then process subsample_block samples at once
		//Loop through all MIDI messages which need to be played up until the current playback time
		for (g_tsf_Msec += subsample_block * (1000.0 / AUDIO_SAMPLE_RATE);
				g_MidiMessage && g_tsf_Msec >= g_MidiMessage->time;
				g_MidiMessage = g_MidiMessage->next)
		{
			switch (g_MidiMessage->type)
			{
				case TML_PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
					tsf_channel_set_presetnumber(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->program, (g_MidiMessage->channel == 9));
					break;
				case TML_NOTE_ON: //play a note
					tsf_channel_note_on(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
					break;
				case TML_NOTE_OFF: //stop a note
					tsf_channel_note_off(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key);
					break;
				case TML_PITCH_BEND: //pitch wheel modification
					tsf_channel_set_pitchwheel(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
					break;
				case TML_CONTROL_CHANGE: //MIDI controller messages
					tsf_channel_midi_control(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
					break;
			}
		}
		// Render the block of audio samples in float format
		tsf_render_float(g_TinySoundFont, sample_block, subsample_block, 0);
	}
}
#endif

void blocks_sources_test_x(float * sample_block){
	int_fast32_t i;
	for (i = 0; i < AUDIO_BLOCKSIZE; i++) {
		sample_block[i] = test_x[test_x_idx++];
		if (test_x_idx >= NUMEL(test_x)) {
			test_x_idx = 0;
		}
	}
}

void blocks_sources_test_y(float * sample_block){
	int_fast32_t i;
	for (i = 0; i < AUDIO_BLOCKSIZE; i++) {
		sample_block[i] = test_y[test_y_idx++];
		if (test_y_idx >= NUMEL(test_y)) {
			test_y_idx = 0;
		}
	}
}

void blocks_sources_microphone(float * sample_block){
	arm_copy_f32(processed_micdata, sample_block, AUDIO_BLOCKSIZE);
}

#ifdef AUDIO_SOURCE_SYNTH
/** @brief Switches to the next audio track */
void blocks_sources_nexttrack(void){
	ATOMIC(flag_next_track = true);
}
#endif