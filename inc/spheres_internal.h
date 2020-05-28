#ifdef SKIP_FACTORY_SPHERES_IN_HEXFILE

 	const void *wavetable_list[] = {0};

#else

	#include "spheres/hp_909hits_01.h"
	#include "spheres/hp_Distorted_FM.h"
	#include "spheres/hp_Morphing_Cello.h"
	#include "spheres/hp_TalkativeFM.h"
	#include "spheres/computed_formants.h"
	#include "spheres/hp_wavetable_formants_applespeech_1.h"
	#include "spheres/wavetable_SWN_D.h"
	#include "spheres/wavetable_SWN_wf_rm_hs.h"
	#include "spheres/wavetable_Sine_Seq_JQ.h"
	#include "spheres/wavetable_pailo_sine_square.h"
	#include "spheres/wavetable_pailo_smoothrough.h"
	#include "spheres/wavetable_ring_mod_JQ.h"

	const void *wavetable_list[] = {
		(void *)wavetable_SWN_D,
		(void *)wavetable_pailo_sine_square, 
		(void *)computed_formants,
		(void *)hp_wavetable_formants_applespeech_1,
		(void *)hp_Morphing_Cello,
		(void *)hp_TalkativeFM,
		(void *)hp_Distorted_FM,
		(void *)hp_909hits_01,
		(void *)wavetable_SWN_wf_rm_hs, 
		(void *)wavetable_pailo_smoothrough, 
		(void *)wavetable_ring_mod_JQ,
		(void *)wavetable_Sine_Seq_JQ, 
	};

#endif
