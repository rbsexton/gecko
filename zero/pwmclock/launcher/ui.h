#define WAKE_COUNT 15
#define BOUNCE_COUNT 6 

typedef enum { 
	tUIState_init = 0,   // Normalcy
	tUIState_waking = 1, // First press, waiting 2s for next state
	tUIState_awake  = 2,
	} tUIButtonState;

typedef enum { // A basic animation
	tNeedleState_run = 0,   // Normalcy
	tNeedleState_bounce_h0 = 1,
	tNeedleState_bounce_l0 = 2,
	tNeedleState_bounce_h1 = 3,
	tNeedleState_bounce_l1 = 4,
	} tUINeedleState;
	
	
	