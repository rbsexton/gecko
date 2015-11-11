#define WAKE_COUNT 15
#define BOUNCE_COUNT 6 

typedef struct {
	int value;
	int limit;
	bool active;
	} sCounter;

typedef struct {
	int state; // Start at zero.
	bool active;
	} sSequence;

typedef enum { 
	tUIState_run = 0, // Run
	tUIState_set_hours = 1, // M & S are zeroed.
	tUIState_set_mins  = 2, // H Normal, S are zeroed.
	
	tUIState_awake  = 2,
	} tUIButtonState;

typedef enum { // A basic animation
	tNeedleState_run = 0,   // Normalcy
	tNeedleState_bounce_h0 = 1,
	tNeedleState_bounce_l0 = 2,
	tNeedleState_bounce_h1 = 3,
	tNeedleState_bounce_l1 = 4,
	} tUINeedleState;

void ui_init();	
void UpdateInputs(bool pressed);
void UIStateUpdate();
void NeedleUpdate();
	