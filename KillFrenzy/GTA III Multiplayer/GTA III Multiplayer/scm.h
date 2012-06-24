#ifndef _SCM_
#define _SCM_

#define MEM_SCM_MainThread			0x008E2CA8
#define FUNC_ProcessSCMCommand		0x00439500
#define MAX_SCRIPT_SIZE				0xFF
#define MAX_VARIABLES				0x10
#define MEM_SCM_STARTMAIN			0x00591076
#define FUNC_STARTMAIN				0x00438790
#define MEM_ProcessScriptCall		0x0048C9F5
#define MEM_ProcessScriptEvents		0x0048CA8E
#define MEM_ForwardScriptCall		0x004393DF

#define SCMCAST_INT			0x01
#define SCMCAST_VAR			0x02
#define SCMCAST_CHAR		0x04
#define SCMCAST_SHORT		0x05
#define SCMCAST_FLOAT		0x06

/*
	Credits to spookie for his Vice City SCM hook, I learned from it
*/
typedef struct scmThread_s	// 0x88 bytes total.
{							// - Credit to CyQ & PatrickW
	void* pNext;			// 0x00
	void* pPrev;			// 0x04
	char strName[8];		// 0x08
	DWORD dwScriptIP;		// 0x10
	DWORD dwReturnStack[6];	// 0x14
	DWORD dwStackPointer;	// 0x2C
	DWORD dwLocalVar[18];	// 0x30
	BYTE bStartNewScript;	// 0x78
	BYTE bJumpFlag;			// 0x79
	BYTE bWorky;			// 0x7A
	BYTE bAwake;			// 0x7B
	DWORD dwWakeTime;		// 0x7C
	WORD wIfParam;			// 0x80
	BYTE bNotFlag;			// 0x82
	BYTE bWastedBustedCheck;// 0x83
	BYTE bWastedBustedFlag;	// 0x84
	BYTE bMissionThread;	// 0x85
	BYTE pad[2];			// 0x86
} scmThread_t;

struct SCRIPT_COMMAND		//	Params
{							//		i = integer
	WORD OpCode;			//		f = float
	char Params[13];		//		v = variable
};							//		s = string

/*	Below are some opcodes i've defined, but it's still missing
	about 95% of them ;) See http://vc-db.webtools4you.net/		*/
const SCRIPT_COMMAND create_player					= { 0x0053, "ifffv" };		// model_id, x, y, z, index
const SCRIPT_COMMAND create_actor					= { 0x009A, "iifffv" };		// PEDTYPE_*, #MODEL, x, y, z, var_actor
const SCRIPT_COMMAND destroy_actor					= { 0x009B, "i" };			// actor_id
const SCRIPT_COMMAND create_car						= { 0x00A5, "ifffv" };		// (CAR_*|BIKE_*|BOAT_*), x, y, z, var_car
const SCRIPT_COMMAND destroy_car					= { 0x00A6, "i" };			// vehicle_id
const SCRIPT_COMMAND create_object					= { 0x0107, "ifffv" };		// model_id, x, y, z, index
const SCRIPT_COMMAND destroy_object					= { 0x0108, "i" };			// object_id
const SCRIPT_COMMAND create_pickup					= { 0x0213, "iifffv" };		// ?, type, x, y, z, index
const SCRIPT_COMMAND destroy_pickup					= { 0x0215, "i" };			// pickup_id
const SCRIPT_COMMAND destroy_sphere					= { 0x03BD, "i" };			// sphere_id
const SCRIPT_COMMAND create_particle				= { 0x016F, "ifffiiiiii" };	// type, x, y, z, rotation_factor, intensity, size, flag, flag, flag
const SCRIPT_COMMAND destroy_particle				= { 0x0650, "i" };			// particle_id
const SCRIPT_COMMAND create_train_at				= { 0x06D8, "ifffiv" };		// model_id, x, y, z, direction, index
const SCRIPT_COMMAND destroy_train					= { 0x07BD, "i" };			// train_id
const SCRIPT_COMMAND create_marker_above_car		= { 0x0161, "iiiv" };		// vehicle_id, ?, blip_type, index
const SCRIPT_COMMAND create_marker_at				= { 0x0167, "fffiiv" };		// x, y, z, flag, flag, index
const SCRIPT_COMMAND create_checkpoint_at			= { 0x018A, "fffv" };		// x, y, z, index
const SCRIPT_COMMAND create_racing_checkpoint_at	= { 0x06D5, "ifffffffv" };	// type, x, y, z, point_x, point_y, point_z, radius, index
const SCRIPT_COMMAND disable_racing_checkpoint		= { 0x06D6, "i" };			// checkpoint_id
const SCRIPT_COMMAND create_corona_with_radius		= { 0x024F, "fffiiiiiiv" };	// x, y, z, ?, type, lensflares, r, g, b
const SCRIPT_COMMAND create_marker					= { 0x02A8, "fffiv" };		// x, y, z, type, index
const SCRIPT_COMMAND create_entrance_marker			= { 0x0A40, "fffiv" };		// x, y, z, color, index
const SCRIPT_COMMAND destroy_entrance_marker		= { 0x0A41, "i" };			// marker_id
const SCRIPT_COMMAND create_explosion_of_type		= { 0x020C, "fffi" };		// x, y, z, type
const SCRIPT_COMMAND request_model					= { 0x0247, "i" };			// (CAR_*|BIKE_*|BOAT_*|WEAPON_*|OBJECT_*)
const SCRIPT_COMMAND load_requested_models			= { 0x038B, "" };			// -/-
const SCRIPT_COMMAND release_model					= { 0x0249, "i" };			// model_id
const SCRIPT_COMMAND load_special_actor				= { 0x023C, "is" };			// SPECIAL_*, MODEL_*
const SCRIPT_COMMAND destroy_actor_fading			= { 0x034F, "v" };			// var_actor
const SCRIPT_COMMAND kill_actor						= { 0x0321, "v" };			// var_actor
const SCRIPT_COMMAND kill_player					= { 0x0322, "v" };			// player_char
const SCRIPT_COMMAND force_weather					= { 0x01B5, "i" };			// WEATHER_*
const SCRIPT_COMMAND set_weather					= { 0x01B6, "i" };			// WEATHER_*
const SCRIPT_COMMAND release_weather				= { 0x01B7, "" };			// -/-
const SCRIPT_COMMAND set_fade_color					= { 0x0169, "iii" };		// Red(0-255), Green(0-255), Blue(0-255)
const SCRIPT_COMMAND fade							= { 0x016A, "ii" };			// (time in ms), FADE_*
const SCRIPT_COMMAND is_fading						= { 0x016B, "" };			// -/-
const SCRIPT_COMMAND set_actor_immunities			= { 0x02AB, "viiiii" };		// var_actor, Bullet(1/0), Fire(1/0), Explosion(1/0), Damage(1/0), Water(1/0)
const SCRIPT_COMMAND freeze_player					= { 0x01B4, "vi" };			// PLAYER_CHAR, Freeze(1/0)
const SCRIPT_COMMAND set_max_wanted_level			= { 0x01F0, "i" };			// MaxLevel
const SCRIPT_COMMAND set_wasted_busted_check		= { 0x0111, "i" };			// Check(1/0)
const SCRIPT_COMMAND set_current_time				= { 0x00C0, "ii" };			// Hours, Minutes
const SCRIPT_COMMAND set_camera						= { 0x03CB, "fff" };		// x, y, z
//const SCRIPT_COMMAND create_player					= { 0x0053, "ifffv" };		// 0, x, y, z, PLAYER_CHAR
const SCRIPT_COMMAND create_actor_from_player		= { 0x01F5, "vv" };			// PLAYER_CHAR, PLAYER_ACTOR
const SCRIPT_COMMAND set_actor_skin					= { 0x0352, "vs" };			// var_actor, MODEL_*
const SCRIPT_COMMAND refresh_actor_skin				= { 0x0353, "v" };			// var_actor
const SCRIPT_COMMAND select_interior				= { 0x04BB, "i" };			// INTERIOR_*
const SCRIPT_COMMAND player_in_car					= { 0x0443, "v" };			// PLAYER_CHAR
const SCRIPT_COMMAND get_player_car					= { 0x03C1, "vv" };			// PLAYER_CHAR, var_car
const SCRIPT_COMMAND car_stopped					= { 0x01C1, "v" };			// var_car
const SCRIPT_COMMAND get_car_speed					= { 0x02E3, "vv" };			// var_car var_speed
const SCRIPT_COMMAND check_model					= { 0x0248, "i" };			// model_id
//const SCRIPT_COMMAND release_model					= { 0x0249, "i" };		// model_id
const SCRIPT_COMMAND play_sound						= { 0x018C, "ifff" };		// x, y, z, sound_id
const SCRIPT_COMMAND play_music						= { 0x0394, "i" };			// sound_id
const SCRIPT_COMMAND turn_player_around				= { 0x03C8, "" };			// -/-
const SCRIPT_COMMAND enter_car_driver				= { 0x01D4, "ii" };			// actor_id, car_id

/*
	Vice City SCM hook code end
*/

void	SCM_Init ();
bool	SCM_ProcessCommand ( const struct SCRIPT_COMMAND *cmd,  ... );

#endif