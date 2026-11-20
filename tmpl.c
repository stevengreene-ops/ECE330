/** !
 * Battleship implementation
 *
 * @file battleship.c
 *
 * @author Jacob Smith
 */

// headers
#include "tmpl.h"
#include <stdbool.h>
//#include "seg7.c"
extern unsigned char _7SEG_ASCII[]; //INCLUDING 7SEG TEXT 11.13.26SG

// forward declarations
static void poll_switches ( void );
static void poll_potentiometers ( void );
static void store_message ( const char *p_message );

//static fn_state *state_start;
//static fn_state *state_p1_place;
//static fn_state *state_p2_place;
//static fn_state *state_p1_turn;
//static fn_state *state_p2_turn;
//static fn_state *state_result;


// data
// Function prototypes (matches your function definitions)
static void state_start ( void );
static void state_p1_place ( void );
static void state_p2_place ( void );
static void state_p1_turn ( void );
static void state_p2_turn ( void );
static void state_result ( void );

static fn_state _pfn_states[] =
{
	[STATE_START]    = state_start,
	[STATE_P1_PLACE] = state_p1_place,
	[STATE_P2_PLACE] = state_p2_place,
	[STATE_P1_TURN]  = state_p1_turn,
	[STATE_P2_TURN]  = state_p2_turn,
	[STATE_RESULT]   = state_result,
};

struct
{
	unsigned short sw;     // the switches (16 switches -> 16 bits -> short)
	unsigned short ld;     // the led's (16 leds -> 16 bits -> short)
	bool pc10;             // true IF PC10 is down ELSE false
	bool pc11;             // true IF PC11 is down ELSE false
	unsigned short _pa[3]; // raw potentiometer values (3 potentiometers -> 3 values -> array of 3 shorts)
	unsigned int time;     // the time (in ms) since the board started (incremented by timer interrupt)
} 
device_state =
{
	.sw   = 0,
	.ld   = 0,
	.pc10 = false,
	.pc11 = false,
	._pa  = 
	{ 
		0, // <- PA1
		0, // <- PA2
		0, // <- PA3
	},
};

// player data 
player players[PLAYER_QUANTITY]  =
{

	// player A's data
	[PLAYER_A] =
	{

		// player A's maps
		.maps =
		{

			// player A's ship map is empty when the game starts
			[SHIP] =
			{
				.horizontal = 
				{ 
					{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
					{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
					{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
				},
				.vertical =
				{
					{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
					{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
				},
			},

			// player A's shot map is empty when the game starts
			[SHOT] =
			{
				.horizontal = { 0 },
				.vertical   = { 0 },
			}
		}
		
	},

	// Player B's data
	[PLAYER_B] =
	{

		// player B's maps
		.maps =
		{

			// player B's ship map is empty when the game starts
			[SHIP] =
			{
				.horizontal = {
						{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, }, //added STILL for player B
						{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
						{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
				},
				.vertical   = {
						{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
						{ STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, STILL, },
				},
			},

			// player B's shot map is empty when the game starts
			[SHOT] =
			{
				.horizontal = { 0 },
				.vertical   = { 0 },
			}
		},
	},
};

extern void input_poll ( void )
{

	// poll the switches
	poll_switches();

	// poll the potentiometers
	poll_potentiometers();

	// todo: poll any other input devices
	//

	// done
	return;
}

static void poll_switches ( void )
{

	// todo: store the state of the switches in device_state.sw
	if ((GPIOA->IDR & (1 << 0)) !=0) { //IF input data register !=0
		device_state.sw = 0x01; //button pressed
	}
	else
	{
		device_state.sw = 0x00; //otherwise button is not pressed
	}

	// done
	return;
}

static void poll_potentiometers ( void )
{

	// todo: iterate through each potentiometer and store the value in the corresponding index of device_state._pa
	// for loop to iterate through potentiometer

	/** TESTING WHILE LOOP CONVERSION COMPLETION 11/13/26
	for (int i = 0; i<3; i++) {
		while (!(ADC1->SR & ADC_SR_EOC)); //waiting for conversion to complete

		device_state._pa[i] = (unsigned char)(ADC1->DR >> 4);
	}
	**/

	//Using LAB 10 conversion method///////////////////////////////////
	///////////////////////////////////////////////////////////////////
	/**
	  ADC1->SQR3 = 1;
	  HAL_Delay(1);
	  ADC1->CR2 |= 1 << 30;

	  // POT1
	  ADC1->SQR3 = 1;
	  ADC1->CR2 |= 1 << 30;                 // Start conversion
	  while ((ADC1->SR & (1 << 1)) == 0) {} // Wait here until conversion is done
	  device_state._pa = ADC1->DR >> 4;              // Safe to read now

	  // POT2
	  ADC1->SQR3 = 2;
	  ADC1->CR2 |= 1 << 30;                 // Start conversion
	  while ((ADC1->SR & (1 << 1)) == 0) {} // Wait here until conversion is done
	  device_state._pa = ADC1->DR >> 4;            // Safe to read now

	  // POT3
	  ADC1->SQR3 = 3;
	  ADC1->CR2 |= 1 << 30;                 // Start conversion
	  while ((ADC1->SR & (1 << 1)) == 0) {} // Wait here until conversion is done
	  device_state._pa = ADC1->DR >> 4;             // Safe to read now
	  **/
	  /////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	  //TESTING COMBINED VERSION:

	  const uint8_t pot_channels[3] = {1,2,3};

	  for (int i = 0; i < 3; i++) {
		  ADC1->SQR3 = pot_channels[i];
		  ADC1->CR2 |= (1 << 30);

		  while ((ADC1->SR & (1 << 1 )) ==0) {}

		  device_state._pa[i] = (unsigned char)(ADC1->DR >> 4);
	  }

	// done
	return;
}

extern void timer_step ( void )
{

	// todo: call this function from the SysTick handler
	//

	// increment the time
	device_state.time++;

	// done
	return;
}

extern void draw_board ( map _map )
{

	// initialized data
	unsigned char _board[8] = { 0 };
	bool should_dim   = (device_state.time % 5 == 0 );
	bool should_blink = ( (device_state.time / 250) % 2 == 0 );

	// draw the horizontal segments
	for (size_t i = 0; i < 8; i++)
	{

		// set the top segment based on the horizontal[0] map
		switch(_map.horizontal[0][i])
		{
			case STILL:
				_board[i] |= 0;
				break;
			case HIT:
				_board[i] |= SEG7_TOP;
				break;
			case MISS:
				_board[i] |= should_dim ? SEG7_TOP : 0;
				break;
			case CURSOR:
				_board[i] |= should_blink ? SEG7_TOP : 0;
				break;
		}

		// todo: set the middle segment based on the horizontal[1] map
		// inserting segments for middle of board 11/12 SG
		switch(_map.horizontal[1][i])
		{
		case STILL:
			_board[i] |= 0;
			break;
		case HIT:
			_board[i] |= SEG7_MIDDLE;
			break;
		case MISS:
			_board[i] |= should_dim ? SEG7_MIDDLE : 0;
			break;
		case CURSOR:
			_board[i] |= should_blink ? SEG7_MIDDLE : 0;
			break;
		}

		// todo: set the bottom segment based on the horizontal[2] map
		// SET BOTTOM OF BOARD HORIZONTAL 11/12 SG
		switch(_map.horizontal[2][i])
		{
		case STILL:
			_board[i] |= 0;
			break;
		case HIT:
			_board[i] |= SEG7_BOTTOM;
			break;
		case MISS:
			_board[i] |= should_dim ? SEG7_BOTTOM : 0;
			break;
		case CURSOR:
			_board[i] |= should_blink ? SEG7_BOTTOM : 0;
			break;
		}
	}

	// draw the vertical segments
	for (size_t i = 0; i < 8; i++)
	{

		// todo: set the top left segment based on the vertical[0] map
		// TOPLEFT SET 11/12 SG
		switch(_map.vertical[i][0])
		{
			case STILL:
				_board[i] |= 0;
				break;
			case HIT:
				_board[i] |= SEG7_TOP_LEFT;
				break;
			case MISS:
				_board[i] |= should_dim ? SEG7_TOP_LEFT : 0;
				break;
			case CURSOR:
				_board[i] |= should_blink ? SEG7_TOP_LEFT : 0;
				break;
		}

		// todo: set the top right segment based on the vertical[0] map
		// SET TOP RIGHT MAP 11/12 SG
		switch(_map.vertical[i][1])
		{
			case STILL:
				_board[i] |= 0;
				break;
			case HIT:
				_board[i] |= SEG7_TOP_RIGHT;
				break;
			case MISS:
				_board[i] |= should_dim ? SEG7_TOP_RIGHT : 0;
				break;
			case CURSOR:
				_board[i] |= should_blink ? SEG7_TOP_RIGHT : 0;
				break;
		}
		// todo: set the bottom left segment based on the vertical[1] map
		//
		switch(_map.vertical[i][2])
		{
			case STILL:
				_board[i] |= 0;
				break;
			case HIT:
				_board[i] |= SEG7_BOTTOM_LEFT;
				break;
			case MISS:
				_board[i] |= should_dim ? SEG7_BOTTOM_LEFT : 0;
				break;
			case CURSOR:
				_board[i] |= should_blink ? SEG7_BOTTOM_LEFT : 0;
				break;
		}
		// todo: set the bottom right segment based on the vertical[1] map
		// SET BOTTOMRIGHT MAP 11/12 SG
		switch(_map.vertical[i][3])
		{
			case STILL:
				_board[i] |= 0;
				break;
			case HIT:
				_board[i] |= SEG7_BOTTOM_RIGHT;
				break;
			case MISS:
				_board[i] |= should_dim ? SEG7_BOTTOM_RIGHT : 0;
				break;
			case CURSOR:
				_board[i] |= should_blink ? SEG7_BOTTOM_RIGHT : 0;
				break;
		}
	}

	// write the board array to the seven segment display
	for(int i = 0; i < 8; i++)
		GPIOE->ODR = (0XFF00 | ~_board[i]) & ~(1<<((7-i)+8)),
		GPIOE->ODR |= 0xFF00;

	// done
	return;
}

extern void game_logic ( void )
{

	// static data
	static int state = STATE_START;

	// update
	_pfn_states[state]();

	// state transition
	switch (state)
	{
		case STATE_START:
			if ( device_state.pc10 )
				state = STATE_P1_PLACE;
			break;
		
		// player 1 places boats
		case STATE_P1_PLACE:

			// transition
			if ( 0 ) 
				state = STATE_P2_PLACE;
			
			break;
		
		// player 2 places boats
		case STATE_P2_PLACE:

			// transition
			if ( 0 ) 
				state = STATE_P2_PLACE;

			break;
		
		// player 1 takes a shot
		case STATE_P1_TURN:

			// transition
			if ( 0 ) 
				state = STATE_P2_TURN;
			else if ( 0 )
				state = STATE_RESULT;

			break;
		
		// player 2 takes a shot
		case STATE_P2_TURN:

			// transition
			if ( 0 ) 
				state = STATE_P1_TURN;
			else 
				state = STATE_RESULT;

			break;

		// game over
		case STATE_RESULT:
			if ( device_state.pc10 )
				state = STATE_START;
			break;

		// unknown state -> reset
		default: 
			state = STATE_START; 
			break;
	}
	
	// done
	return;
}

void state_start ( void ) 
{

	// todo

	//RESETTING GAME VARIABLES
	static uint32_t last_scroll_time = 0; //tracks systick ms count when text last shifted
	static uint8_t scroll_index = 0; //tracks which char of the battleship string is aligned with first digit

	char message[] = "    BATTLESHIP    "; //padding with spaces
	int msg_len = sizeof(message) - 1; //calculating num of chars in string

	if ((device_state.time - last_scroll_time) >= 300) { //every 300ms elaps:
		last_scroll_time = device_state.time; //updates last scrool time to current time
		scroll_index++; //increments by one to shift text window 1 char to right
		if (scroll_index > (msg_len - 8)) { // pads scroll window shifts chars to right 8 bits
			scroll_index = 0;
		}
	}

	for (int i = 0; i< 8; i++) {

		char c = message[scroll_index + i];
		uint8_t segment_pattern = _7SEG_ASCII[(unsigned char)c]; // takes ascii value of char, pulls to correct bitmask

		Seven_Segment_Digit(i, segment_pattern, 0); //pushes bitmask to digit position i
	}

	if (device_state.pc10) {
		scroll_index = 0;

		for (int p = 0; p< PLAYER_QUANTITY; p++) {

			players[p].boats = 0;

				for (int m = 0; m < MAP_TYPE_QUANTITY; m++) {

					for (int r = 0; r < 3; r++) {

						for (int c = 0; c < 8; c++) {
							players[p].maps[m].horizontal[r][c] = STILL;
						}
					}

			for (int d = 0; d < 8; d++) {
				for (int s = 0; s < 4; s++) {
					players[p].maps[m].vertical[d][s] = STILL;
				}
			}
				}
			}
		}

	// done
	return;
}

static void state_p1_place ( void )
{

	// todo
	//
	static uint32_t last_scroll_time = 0;
	static uint8_t scroll_index = 0;
	char message[] = "  P1 PLACE  ";
	int msg_len = sizeof(message) - 1;

	if ((device_state.time - last_scroll_time) >= 300) {
		last_scroll_time = device_state.time;
		scroll_index++;
		if(scroll_index > (msg_len - 8)) scroll_index = 0;
	}

	for (int i = 0; i < 8; i++) {
		Seven_Segment_Digit(i, _7SEG_ASCII[(unsigned char)message[scroll_index + i]], 0);

		if (device_state.sw == 0x01) {
			device_state.pc10 = 1;
			scroll_index = 0;
		} else {
			device_state.pc10 = 0;
		}

	}
	// done
	return;
}
static void state_p2_place ( void )
{

	// todo
	//
	static uint32_t last_scroll_time = 0;
	static uint8_t scroll_index = 0;
	char message[] = "  P2 PLACE  ";
	int msg_len = sizeof(message) - 1;

	if ((device_state.time - last_scroll_time) >= 300) {
		last_scroll_time = device_state.time;
		scroll_index++;
		if(scroll_index > (msg_len - 8)) scroll_index = 0;
	}

	for (int i = 0; i < 8; i++) {
		Seven_Segment_Digit(i, _7SEG_ASCII[(unsigned char)message[scroll_index + i]], 0);

		if (device_state.sw == 0x01) {
			device_state.pc10 = 1;
			scroll_index = 0;
		} else {
			device_state.pc10 = 0;
		}

	}
	// done
	return;
}

static void state_p1_turn ( void )
{

	// todo
	//
	device_state.pc10 = 0; //reset transition flags on every tic

	//render active board.
	draw_board(players[PLAYER_A].maps[SHOT]);

	//CHECK FOR FIRE BUTTON
	if (device_state.sw == 0x01) {

		for (int row = 0; row < 3; row++) {
			for (int col = 0; col < 8; col++) {
				if(players[PLAYER_A].maps[SHOT].horizontal[row][col] == CURSOR) {
					if (players[PLAYER_B].maps[SHIP].horizontal[row][col] == HIT) {
						players[PLAYER_A].maps[SHOT].horizontal[row][col] = HIT;
						players[PLAYER_B].boats--;
					} else {
						players[PLAYER_A].maps[SHOT].horizontal[row][col] = MISS;
					}
				}
			}
		}

		for (int digit = 0; digit < 8; digit++) {
			for (int seg = 0;seg < 4; seg++) {
				if (players[PLAYER_A].maps[SHOT].vertical[digit][seg] == CURSOR) {
					if (players[PLAYER_B].maps[SHIP].vertical[digit][seg] == HIT) {
						players[PLAYER_A].maps[SHOT].vertical[digit][seg] = HIT;
						players[PLAYER_B].boats--;
					}
					else {
						players[PLAYER_A].maps[SHOT].vertical[digit][seg] = MISS;
					}
				}
			}
		}

		device_state.pc10 = 1;
	// done
	return;
}}

static void state_p2_turn ( void )
{

	// todo
	//
	device_state.pc10 = 0;

	draw_board(players[PLAYER_B].maps[SHOT]);

	if (device_state.sw == 0x01) {

		for (int row = 0; row < 3; row++) {
			for (int col = 0; col < 8; col++) {
				if(players[PLAYER_B].maps[SHOT].horizontal[row][col] == CURSOR) {
					if (players[PLAYER_A].maps[SHIP].horizontal[row][col] == HIT) {
						players[PLAYER_B].maps[SHOT].horizontal[row][col] = HIT;
						players[PLAYER_A].boats--;
					} else {
						players[PLAYER_B].maps[SHOT].horizontal[row][col] = MISS;
					}
				}
			}
		}

		for (int digit = 0; digit < 8; digit++) {
			for (int seg = 0;seg < 4; seg++) {
				if (players[PLAYER_B].maps[SHOT].vertical[digit][seg] == CURSOR) {
					if (players[PLAYER_A].maps[SHIP].vertical[digit][seg] == HIT) {
						players[PLAYER_B].maps[SHOT].vertical[digit][seg] = HIT;
						players[PLAYER_A].boats--;
					}
					else {
						players[PLAYER_B].maps[SHOT].vertical[digit][seg] = MISS;
					}
				}
			}
		}

		device_state.pc10 = 1;
	}

	// done
	return;
}

 void state_result ( void )
{

	// todo
	//
	static uint32_t last_scroll_time = 0;
	static uint8_t scroll_index = 0;

	device_state.pc10 = 0;

	char *message = (players[PLAYER_B].boats <= 0) ? "  P1 WINS  " : "  P2 WINS  ";
	int msg_len = 11;

	if ((device_state.time - last_scroll_time) >= 300) {

		last_scroll_time = device_state.time;
		scroll_index++;
		if (scroll_index > (msg_len - 8)) {

			scroll_index = 0;
		}
	}

	for (int i = 0; i < 8; i++) {

		Seven_Segment_Digit(i, _7SEG_ASCII[(unsigned char)message[scroll_index + i]],0);

	}

	if (device_state.sw == 0x01) {

		for (int p = 0; p < PLAYER_QUANTITY; p++) {

			players[p].boats = 0;

			for (int m = 0; m < MAP_TYPE_QUANTITY; m++) {

				for (int r = 0; r < 3; r++)
				{
					for (int c = 0; c < 8; c++)
					{

						players[p].maps[m].horizontal[r][c] = STILL;
					}
				}

					for (int d = 0; d < 8; d++)
					{
						for (int s = 0; s < 4; s++)
						{
							players[p].maps[m].vertical[d][s] = STILL;
						}
					}
				}

				scroll_index = 0;
				device_state.pc10 = 1; //triggers state_start
			}
		}


	// done
	return; }
