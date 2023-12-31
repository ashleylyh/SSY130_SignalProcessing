#include "lab_ofdm.h"
#include <stdbool.h>
#include "lab_ofdm_process.h"
#include "backend/arm_math.h"
#include "blocks/sources.h"
#include "blocks/sinks.h"
#include "blocks/misc.h"
#include "util.h"
#include "config.h"
#include "arm_math.h"
#include "backend/printfn/printfn.h"

#if defined(SYSMODE_OFDM)

//#define OFDM_DEBUGMODE					//!<- Define to print debugging information to terminal during runtime

#define LAB_OFDM_THRS_SCALE			(1.0f+1.0f/(1<<5)) //!<- Scaling parameter for threshold value
#define LAB_OFDM_DETECTOR_DEC		1					//!<- Decimation rate for OFDM signal detector. Every n'th input sample will be tested
#define LAB_OFDM_N_THRS LAB_OFDM_CYCLIC_PREFIX_SIZE/16*LAB_OFDM_UPSAMPLE_RATE	//!<- Number of samples that must exceed threshold to register as signal detection
#define LAB_OFDM_BPFILT_CENTER 		4000				//!<- Bandpass filter center frequency [Hz]
#define LAB_OFDM_BPFILT_WIDTH 		2000				//!<- Bandpass filter bandwidth [Hz]
#define LAB_OFDM_BPFILT_FS			16000				//!<- Bandpass filter design samplerate [Hz]
#define LAB_OFDM_DETECTOR_WINDOW_BB_SAMPLES	8			//!<- Number of baseband samples to compare over for signal detector
#define LAB_OFDM_TRANSMIT_DELAY		8					//!<- Number of AUDIO_BLOCKSIZE samples to wait after trigger before transmitting
#define LAB_OFDM_DEFAULT_TRIG_OFFSET -80				//!<- Default trigger offset [samples]

/** @brief Bandpass filter coefficients as generated by 'ofdm_bpfilt_gen.m' */
float bpfilt_coeffs[] = {-5.8069723789e-02, -2.6939235156e-17, 2.7495204270e-02, -8.1634045928e-19, 1.0604780398e-01, 1.0367523833e-16, -2.9169910211e-01, 6.4490896283e-17, 3.8034334542e-01, 6.4490896283e-17, -2.9169910211e-01, 1.0367523833e-16, 1.0604780398e-01, -8.1634045928e-19, 2.7495204270e-02, -2.6939235156e-17, -5.8069723789e-02};

float bpfilt_state[NUMEL(bpfilt_coeffs) + AUDIO_BLOCKSIZE - 1];	//!<- Bandpass filter state-keeping array. Used internally by arm_fir_32 function
arm_fir_instance_f32 bpfilt_s;				//!<- Bandpass filter state structure
float tx_data[LAB_OFDM_TX_FRAME_SIZE];		//!<- OFDM output data frame
float inpbuf[AUDIO_BLOCKSIZE * (1 + CEILING(LAB_OFDM_TX_FRAME_SIZE, AUDIO_BLOCKSIZE))];	//!<- Raw input data, make comfortably larger than entire message to allow for shifting what is viewed as the start of message
struct misc_queuedbuf_s queue_s;			//!<- Storage element for queued outgoing OFDM message (which may be longer than AUDIO_BLOCKSIZE)
size_t trig_offset = LAB_OFDM_DEFAULT_TRIG_OFFSET;	//!<- Manual offset tuning for start-of-message
size_t blank_blocks = 0;					//!<- Number of AUDIO_BLOCKSIZE samples to disable received message detection
bool bpfilt_enbl = true;					//!<- Bandpass filter activation state
bool randpilot_enbl = true;					//!<- Random pilot message state
bool equalization_div = false;				//!<- Equalize message by R(k)/H(k) if true, else by R(k) * conj(H(k))
int sendmsg_delay = -1;						//!<- Number of AUDIO_BLOCKSIZE samples to wait before next transmission. Negative values imply do not transmit.

/** @brief GLRT detector
 * Determines (using matlab-like syntax) 
 * 
 *           ||v1(1:n_window) - v1(n_samples:n_window + n_samples)||^2 
 * v = 1 -  ------------------------------------------------------------		(1)
 *          || [v1(1:n_window), v1(n_samples:n_window + n_samples)] ||^2
 * 
 * where || foo || denotes the euclidean norm of some vector foo.
 * 
 * @param v1 		Pointer to start of first array element
 * @param n_samples	Periodicity of target signal
 * @param n_window 	Number of samples to include in window
 * @return 			Value V in (1) above
 */
static float lab_ofdm_glrt(float * const v1, const size_t n_samples, const size_t n_window){
	//Generate pointer to vector v1 = inpbuf[i, ..., i + n_window - 1] and 
	//v2 = inpbuf[i + n_samples, ..., i + n_samples + n_window - 1]
	float * const v2 = v1 + n_samples;
	
	//Generate vector v3 = v1 - v2
	float v3[n_window];
	arm_sub_f32(v1, v2, v3, NUMEL(v3));
	
	//Generate vector v4 = [v1,v2]
	float v4[2*NUMEL(v3)];
	arm_copy_f32(v1, &v4[0], NUMEL(v3));
	arm_copy_f32(v2, &v4[NUMEL(v3)], NUMEL(v3));
	
	//Calculate ||v3||^2 and ||v4||^2 in the sense of the two-norm.
	//This is identical to dot(vx,vx), i.e. the dot product of a vector with itself
	float norm2_v3, norm2_v4;
	arm_dot_prod_f32(v3, v3, NUMEL(v3), &norm2_v3);
	arm_dot_prod_f32(v4, v4, NUMEL(v4), &norm2_v4);
	
	//Calculate GLRT output as simply ||v3||^2 / ||v4||^2
	//Manually handle ||v4||^2 == 0 case
	float v;
	if(norm2_v4 == 0.0f){
		v = 0.0f;
	}else{
		v = 1 - norm2_v3/norm2_v4;
	}
	
	#ifdef OFDM_DEBUGMODE
	printf("n_samples %d\nn_window %d\n", n_samples, n_window);
	print_vector_f("v1", v1, n_window);
	print_vector_f("v2", v2, n_window);
	print_vector_f("v3", v3, NUMEL(v3));
	print_vector_f("v4", v4, NUMEL(v4));
	print_vector_f("norm2_v3", &norm2_v3, 1);
	print_vector_f("norm2_v4", &norm2_v4, 1);
	print_vector_f("v", &v, 1);
	#endif
	
	return v;
}

/** @brief Structure with all setup parameters for the OFDM detector */
struct lab_ofdm_detect_setup_s {
	float *	inpbuf;		//!<- Pointer to buffer containing raw input signal
	size_t	inpbuflen;	//!<- Number of samples in a single OFDM pilot/message excluding cyclic prefix at the current sample rate
	size_t	n_samples;	//!<- Number of samples in a single OFDM pilot/message excluding cyclic prefix at the current sample rate
	size_t	m_pairs;	//!<- Number of OFDM pilot/message pairs in entire frame
	size_t	m_samples;	//!<- Number of samples in a single OFDM pilot/message including cyclic prefix at the current sample rate
	size_t	n_window;	//!<- Discrimination window length. Must be no larger than length of cyclic prefix - channel impulse response
	float	thr;		//!<- Detection threshold
	size_t	n_thr;		//!<- Number of successive samples that must exceed thr in order to register a signal detection
	size_t	dec_rate;	//!<- Decimation rate for detection. Every dec_rate'th sample will be tested. Use to reduce computational load
};

struct lab_ofdm_detect_setup_s lab_ofdm_detect_setup = {
	.inpbuf = 		inpbuf,
	.inpbuflen = 	NUMEL(inpbuf),
	.n_samples = 	LAB_OFDM_BLOCKSIZE * LAB_OFDM_UPSAMPLE_RATE,
	.m_pairs = 		LAB_OFDM_NUM_MESSAGE,
	.m_samples = 	LAB_OFDM_BLOCK_W_CP_SIZE * LAB_OFDM_UPSAMPLE_RATE,
	.n_window = 	LAB_OFDM_DETECTOR_WINDOW_BB_SAMPLES,
	.thr = 			1.0f/(LAB_OFDM_THRS_SCALE*LAB_OFDM_THRS_SCALE*LAB_OFDM_THRS_SCALE*LAB_OFDM_THRS_SCALE),
	.n_thr = 		LAB_OFDM_N_THRS,
	.dec_rate = 	LAB_OFDM_DETECTOR_DEC,
};

/** @brief Detects the starting index of an OFDM signal with structure
 * [CP_DATA_1, DATA_1, ..., CP_DATA_M, DATA_M], i.e. a total of M CP/data pairs
 * where [CP_DATA_n, DATA_n] has a total length of m samples
 * @param s			Pointer to detector parameters
 * @param idx_start	Pointer to store index of detected signal if present
 * @return			True if OFDM signal detected, false otherwise */
static bool lab_ofdm_detect(struct lab_ofdm_detect_setup_s * s, size_t * idx_start){
	bool retval = false;
	
	size_t i;	//Sample index under study
	size_t n_thr_curr = 0;	//Rolling count of current number of successive samples that exceeded the threshold thr
	
	for(i = 0; i <= s->inpbuflen - s->m_samples * s->m_pairs; i+=s->dec_rate){
		#ifdef OFDM_DEBUGMODE
			printf("\n\ni %d of %d\n\n", i, s->inpbuflen - s->m_samples * s->m_pairs);
		#endif
		//Determine GLRT values for pilot and message
		size_t j;
		float v_a[s->m_pairs];
		for(j = 0; j < NUMEL(v_a); j++){
			//printf("testing from index %d of %d\n", i + j*m_samples, inpbuflen);
			v_a[j] = lab_ofdm_glrt(&(s->inpbuf)[i + j*(s->m_samples)], (s->n_samples), (s->n_window));
		}
		
		//Let v = min(v_a), where v is viewed as the net GLRT output
		float v = INFINITY;
		
		for(j = 0; j < NUMEL(v_a); j++){
			#ifdef OFDM_DEBUGMODE
			printf("message %d, measure %f\n", j, v_a[j]);
			#endif
			v = MIN(v, v_a[j]);
		}
		
		#ifdef OFDM_DEBUGMODE
		printf("Net measure %f\n", v);
		#endif
		
		//Update rolling counter of successive signal detections
		if(v >= s->thr){
			n_thr_curr++;
			#ifdef OFDM_DEBUGMODE
			printf("v>=thr, counter %d.\n", n_thr_curr);
			#endif
		}else{
			n_thr_curr = 0;
			#ifdef OFDM_DEBUGMODE
			printf("v<thr, counter %d.\n", n_thr_curr);
			#endif
		}
		
		//Check if sigal detected, if so quit
		if(n_thr_curr >= s->n_thr){
			retval = true;
			*idx_start = i/LAB_OFDM_DETECTOR_DEC - s->n_thr + 1;
			#ifdef OFDM_DEBUGMODE
			printf("Detect at i %d; net index %d.\n", i, *idx_start);
			#endif
			break;
		}
	}
	return retval;
}

static void lab_ofdm_trigstart_fun(typeof(trig_offset) index){
	board_set_led(board_led_blue, true);
	printf("Possible signal detected, attempting decode from input buffer index %d\n", index);
}

static void lab_ofdm_trigend_fun(void){
	board_set_led(board_led_blue, false);
	printf("Input buffer refreshed, checking for input.\n");
}

#define PRINT_HELPMSG() 															\
		printf("Usage guide;\n"														\
			"Press the following keys to change the system behavior\n"				\
			"\t'q' - Shift start of OFDM message back significantly.\n"				\
			"\t'w' - Shift start of OFDM message back slightly.\n"					\
			"\t'e' - Shift start of OFDM message ahead slightly.\n"					\
			"\t'r' - Shift start of OFDM message ahead significantly.\n"			\
			"\t'a' - Decrease detection threshold\n"								\
			"\t's' - Increase detection threshold\n"								\
			"\t'd' - Toggle random/constant pilot message\n"						\
			"\t'f' - Toggle bandpass filter enabled/disabled\n"						\
			"\t'g' - Toggle between phase correct equalization R_eq = R * conj(H)\n"\
			"and phase/magnitude correct equalization R_eq = R/H\n"					\
			"\t' ' (space) - Trigger message transmission\n"						\
			);

void lab_ofdm_init(void){
	BUILD_BUG_ON(LAB_OFDM_CENTER_FREQUENCY != LAB_OFDM_BPFILT_CENTER);
	BUILD_BUG_ON(AUDIO_SAMPLE_RATE/LAB_OFDM_UPSAMPLE_RATE != LAB_OFDM_BPFILT_WIDTH);
	BUILD_BUG_ON(LAB_OFDM_BPFILT_FS != AUDIO_SAMPLE_RATE);
	PRINT_HELPMSG();
	arm_fir_init_f32(&bpfilt_s, NUMEL(bpfilt_coeffs), bpfilt_coeffs, bpfilt_state, AUDIO_BLOCKSIZE);
	lab_ofdm_process_init();
	misc_queuedbuf_init(&queue_s, tx_data, NUMEL(tx_data));
	arm_fill_f32(0.0f, tx_data, NUMEL(tx_data));
	
#ifdef OFDM_DEBUGMODE
	printf("Testing GLRT detector. Will halt after test!\n");
	//Test GLRT detector
	float foo[30];
	size_t i;
	for(i = 0; i < NUMEL(foo); i++){
		foo[i] = i;
	}
	size_t bar;
	lab_ofdm_detect_setup.inpbuf = foo;
	lab_ofdm_detect_setup.inpbuflen = NUMEL(foo);
	lab_ofdm_detect_setup.n_samples = 8;
	lab_ofdm_detect_setup.m_pairs = 2;
	lab_ofdm_detect_setup.m_samples = 10;
	lab_ofdm_detect_setup.n_window = 2;
	lab_ofdm_detect_setup.thr = 0.5;
	lab_ofdm_detect_setup.n_thr = 3;
	lab_ofdm_detect_setup.dec_rate = LAB_OFDM_DETECTOR_DEC;
	lab_ofdm_detect(&lab_ofdm_detect_setup, &bar);
	for(;;);
#endif
}

void lab_ofdm(void){
	//Add microphone samples to input buffer
	
	float inpfilt[AUDIO_BLOCKSIZE];
	float inp[AUDIO_BLOCKSIZE];
	blocks_sources_microphone(inp);
	
	if(bpfilt_enbl){
		arm_fir_f32(&bpfilt_s, inp, inpfilt, NUMEL(inpfilt));
	}else{
		arm_copy_f32(inp, inpfilt, NUMEL(inpfilt));
	}
	
	misc_inpbuf_add(inpbuf, NUMEL(inpbuf), inpfilt, NUMEL(inpfilt));
	
	//Act on any stored keypress
	char key;
	if(board_get_usart_char(&key)){ // Check if a key is pressed
		typeof(trig_offset) shift_offset = 0;
		float thr_mod = 0.0f;
		switch(key){
		default:
			printf("Invalid key pressed.\n");
			PRINT_HELPMSG();
			break;
		case 'q':
			shift_offset = -10*LAB_OFDM_DETECTOR_DEC;
			break;
		case 'w':
			shift_offset = -1*LAB_OFDM_DETECTOR_DEC;
			break;
		case 'e':
			shift_offset = 1*LAB_OFDM_DETECTOR_DEC;
			break;
		case 'r':
			shift_offset = 10*LAB_OFDM_DETECTOR_DEC;
			break;
		case 'a':
			thr_mod = 1.0f/LAB_OFDM_THRS_SCALE;
			break;
		case 's':
			thr_mod = LAB_OFDM_THRS_SCALE;
			break;
		case 'd':
			randpilot_enbl = !randpilot_enbl;
			printf("Randomized pilot %s\n", randpilot_enbl ? "enabled" : "disabled");
			lab_ofdm_process_set_randpilot(randpilot_enbl);
			break;
		case 'f':
			bpfilt_enbl = !bpfilt_enbl;
			printf("Bandpass filter %s\n", bpfilt_enbl ? "enabled" : "disabled");
			break;
		case 'g':
			equalization_div = !equalization_div;
			printf("Equalizing message by %s\n", equalization_div ? "R(k)/H(k) (we've got CPU power to waste)" : "R(k)*conj(H(k)) (lean and mean multiplication machine!)");
			break;
		case ' ':
			sendmsg_delay = LAB_OFDM_TRANSMIT_DELAY	;
			printf("Quiet! Transmitting message soon.\n\n--------------------------------------------------------------------------------\n\n");
			break;
		}
		if(shift_offset != 0){
			trig_offset += shift_offset;
			printf("Sample offset shifted %+d samples to %d.\n",shift_offset, trig_offset);
		}
		if(thr_mod != 0.0f){
			lab_ofdm_detect_setup.thr *= thr_mod;
			printf("Detection threshold scaled by %f to %f.\n", thr_mod, lab_ofdm_detect_setup.thr);
		}
	}

	if(blank_blocks){
		blank_blocks--;
		if(!blank_blocks){
			lab_ofdm_trigend_fun();
		}
	}else{
		size_t idx_start;
		bool sig_detect = lab_ofdm_detect(&lab_ofdm_detect_setup, &idx_start);
		if(sig_detect){
			//Disable any further detection until entire input buffer is emptied and fresh data is available
			blank_blocks = NUMEL(inpbuf)/AUDIO_BLOCKSIZE;
			typeof(trig_offset) net_offset = idx_start + trig_offset;
			if(net_offset < 0){
				net_offset = 0;
				printf("Warning! Requested offset shifts beyond the first stored sample.\nSupplying the first LAB_OFDM_TX_FRAME_SIZE stored samples.\n");
			}else if(net_offset + LAB_OFDM_TX_FRAME_SIZE > NUMEL(inpbuf)){
				net_offset = NUMEL(inpbuf) - LAB_OFDM_TX_FRAME_SIZE;
				printf("Warning! Requested offset shifts beyond the last stored sample.\nSupplying the last LAB_OFDM_TX_FRAME_SIZE stored samples.\n");
			}
			lab_ofdm_trigstart_fun(net_offset);
			lab_ofdm_process_rx(&inpbuf[net_offset], equalization_div); // Process data
		}
	}
	
	//Generate output data and send as needed
	if((sendmsg_delay--) == 0){
		lab_ofdm_process_tx(tx_data); // Create OFDM frame to send
		misc_queuedbuf_init(&queue_s, tx_data, NUMEL(tx_data)); // Add to queue
	}
	
	//Send all-zeros to unused output
	float zeroout[AUDIO_BLOCKSIZE];
	arm_fill_f32(0.0f, zeroout, NUMEL(zeroout));
	blocks_sinks_leftout(zeroout);
	
	//Send data as needed to used output
	float out[AUDIO_BLOCKSIZE];
	misc_queuedbuf_process(&queue_s, out, NUMEL(out), 0.0f);
	blocks_sinks_rightout(out);
}

#endif
