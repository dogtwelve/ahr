// Comms.h: interface for the Series60Comms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MULTIPLAYER_H_CONSTANTS_)
#define _MULTIPLAYER_H_CONSTANTS_

// Messages types ...
enum
{
	BT_MESSAGE_ACCEPTED		= 1,	// from server to client : client has been accepted (&connected) with idx
	BT_MESSAGE_STARTGAME,			// from server to client : start the game with the parameters
	BT_MESSAGE_STARTCARINFO,		// from client to server : car selected info
	BT_MESSAGE_READY,				// from client to server : game ready to start
	BT_MESSAGE_START,				// from server to client : game start
	BT_MESSAGE_UPDATECAR,			// all : update of a car
	BT_MESSAGE_UPDATERANK,			// from server to client (2 player only) : the rank of the client
	BT_MESSAGE_END,					// from server to clients : race ended -> go to result page and wait
	BT_MESSAGE_RESTART,				// from server to clients : race restart
	BT_MESSAGE_NEXTRACE,			// from server to clients : next race restart
	BT_MESSAGE_QUIT,				// from server to clients : quit mp
	BT_MESSAGE_NEWRACE,				// from server to clients : new race restart
	BT_MESSAGE_LOCKDATA,			// from server to clients : lock informations for mp game
	BT_MESSAGE_ABORT,				// from server to clients : abort game (menu or game)
	BT_MESSAGE_HASQUIT,				// from server to clients : client has quitted (idx)
	BT_MESSAGE_HORN,				// all : horn + car idx (server should forward)
	BT_MESSAGE_PAUSE,				// all : pause / unpause
	BT_MESSAGE_KEEPALIVE,			// all : to tell we're still alive :)
	BT_MESSAGE_SERVER_DISCONNECT,	// server to clients : server disconnected (for the case where the BT lib doesn't detect deconnection)
	BT_MESSAGE_UPDATECAR_CRUSH,		// from server to clients : since the server is the only one to compute the collisions betoween cars -> send the crush resulting offsets								
	BT_MESSAGES_NUMBER
};

// BT_MESSAGE_UPDATECAR type msg
enum
{
	MP_MSG_UPDATE_CAR_MSG_TYPE_INDEX = 0, // do not modify this value ! should be 0 !

	MP_MSG_UPDATE_CAR_CAR_IDX_INDEX,

	MP_MSG_UPDATE_CAR_ROTARION_INDEX,

	MP_MSG_UPDATE_CAR_POS_X1_INDEX,
	MP_MSG_UPDATE_CAR_POS_X2_INDEX,

#ifndef USE_MP_MSG_LOW_SIZE
	MP_MSG_UPDATE_CAR_POS_X3_INDEX,
#endif // USE_MP_MSG_LOW_SIZE

	MP_MSG_UPDATE_CAR_POS_Y1_INDEX,
	MP_MSG_UPDATE_CAR_POS_Y2_INDEX,

#ifndef USE_MP_MSG_LOW_SIZE
	MP_MSG_UPDATE_CAR_POS_Y3_INDEX,
#endif // USE_MP_MSG_LOW_SIZE

	MP_MSG_UPDATE_CAR_POS_Z1_INDEX,
	MP_MSG_UPDATE_CAR_POS_Z2_INDEX,

#ifndef USE_MP_MSG_LOW_SIZE
	MP_MSG_UPDATE_CAR_POS_Z3_INDEX,
#endif // USE_MP_MSG_LOW_SIZE

	//MP_MSG_UPDATE_SECTION_IDX_INDEX,
	//MP_MSG_UPDATE_SECTION_FACTOR_INDEX,
	MP_MSG_UPDATE_LAP_NUMBER_INDEX, // actually also some flags
	MP_MSG_UPDATE_LAP_BACKWARDS_INDEX,

#ifdef USE_MULTIPLAYERT_REDUCE_CARS_FLIKERING
	MP_MSG_UPDATE_CAR_TIME1_INDEX,
	MP_MSG_UPDATE_CAR_TIME2_INDEX,
#endif // USE_MULTIPLAYERT_REDUCE_CARS_FLIKERING

	// add here : speed / acceleration / time 

	MP_MSG_UPDATE_CAR_SPEED_X1_INDEX,
	MP_MSG_UPDATE_CAR_SPEED_X2_INDEX,

	MP_MSG_UPDATE_CAR_SPEED_Y1_INDEX,
	MP_MSG_UPDATE_CAR_SPEED_Y2_INDEX,

	MP_MSG_UPDATE_CAR_SPEED_Z1_INDEX,
	MP_MSG_UPDATE_CAR_SPEED_Z2_INDEX,

	// TODO - acceleration also ! 

	MP_MSG_SERVER_UPDATE_CRT_TIME1_INDEX,
	MP_MSG_SERVER_UPDATE_CRT_TIME2_INDEX,

	MP_MSG_UPDATE_CAR_LENGTH,
};

// BT_MESSAGE_UPDATECAR_CRUSH
enum
{
	MP_MSG_UPDATE_CRUSH_MSG_TYPE_INDEX = 0, // do not modify this value ! should be 0 !

	MP_MSG_UPDATE_CRUSH_CAR_IDX_INDEX,

	// MP_MSG_UPDATE_CRUSH_ROTARION_INDEX,

	MP_MSG_UPDATE_CRUSH_OFFSET_X1_INDEX,
	MP_MSG_UPDATE_CRUSH_OFFSET_X2_INDEX,

	// MP_MSG_UPDATE_CRUSH_OFFSET_Y1_INDEX,
	// MP_MSG_UPDATE_CRUSH_OFFSET_Y2_INDEX,

	MP_MSG_UPDATE_CRUSH_OFFSET_Z1_INDEX,
	MP_MSG_UPDATE_CRUSH_OFFSET_Z2_INDEX,

	MP_MSG_UPDATE_CRUSH_LENGTH,
};

// messages lengths

#endif // _MULTIPLAYER_H_CONSTANTS_

