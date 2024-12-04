#include <pch.h>
#include "ModMenu.h"
#include "scriptmenu.h"
#include <cstdint>


Vector3 playerPos;
bool explosiveBullets = false;

void _PRINT_BOTTOM_FORMAT(const char* _Format, ...)
{
	char buffer[UCHAR_MAX];

	va_list vaBuffer;

	va_start(vaBuffer, _Format);
	_vsnprintf_s(buffer, sizeof(buffer), _Format, vaBuffer);
	va_end(vaBuffer);

	UNSORTED::PRINT_SMALL_B(buffer, 2.0f, true, 0, 0, 0, 0);
}

class Invincible : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
		{
			Actor localActor = UNSORTED::GET_PLAYER_ACTOR(UNSORTED::GET_LOCAL_SLOT());
			if (UNSORTED::IS_ACTOR_VALID(localActor))
			{
				UNSORTED::SET_ACTOR_INVULNERABILITY(localActor, 0);
				UNSORTED::SET_INFINITE_DEADEYE(localActor, 0);
			}
		}
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState())
		{
			Actor localActor = UNSORTED::GET_PLAYER_ACTOR(UNSORTED::GET_LOCAL_SLOT());
			if (UNSORTED::IS_ACTOR_VALID(localActor))
			{
				UNSORTED::SET_ACTOR_INVULNERABILITY(localActor, 1);
				UNSORTED::SET_INFINITE_DEADEYE(localActor, 1);
			}
		}
	}
public:
	Invincible(string caption)
		: MenuItemSwitchable(caption) {
	}
};

class SpawnHorse : public MenuItemDefault
{
	int CreateActor(ActorModel id)
	{
		bool isLoaded = false;
		UNSORTED::STREAMING_REQUEST_ACTOR(id, true, false);
		for (int i = 0; i < 5; i++)
		{
			if (UNSORTED::STREAMING_IS_ACTOR_LOADED(id, 1)) {
				isLoaded = true;
				break;
			}
			ScriptWait(500);
		}
		if (!isLoaded)
			return 0;

		playerPos = UNSORTED::GET_POSITION(UNSORTED::GET_PLAYER_ACTOR(0));
		Vector2 playerPos2(playerPos.x, playerPos.y);
		Vector2 playerRot2(0,0);
		int peda = UNSORTED::CREATE_ACTOR_IN_LAYOUT(UNSORTED::FIND_NAMED_LAYOUT("PlayerLayout"), "", id, playerPos2, playerPos.z,playerRot2,playerRot2.x);
		return peda;
	}

	int m_value;
	virtual void OnSelect()
	{
		ActorModel id = ACTOR_RIDEABLE_ANIMAL_Horse01;
		Actor localPlayerActor = UNSORTED::GET_PLAYER_ACTOR(-1);
		Actor horse;
		float heading = UNSORTED::GET_HEADING(localPlayerActor);
		horse = CreateActor(id);
		UNSORTED::SET_ACTOR_HEADING(horse, heading, 1);
		UNSORTED::SET_ACTOR_RIDEABLE(horse, 1);
		ScriptWait(77); 
		UNSORTED::ACCESSORIZE_HORSE(horse, 3);
		UNSORTED::ACTOR_MOUNT_ACTOR(localPlayerActor, horse);
		HEALTH::SET_ACTOR_HEALTH(horse, HEALTH::GET_ACTOR_MAX_HEALTH(horse));
		UNSORTED::HORSE_SET_INFINITE_FRESHNESS_CHEAT(1);
		//UNSORTED::PRINT_HELP_B("horse spawned with unlimited stamina", 5.0f, true, 1, 0, 0, 0, 0);
		_PRINT_BOTTOM_FORMAT("horse spawned with unlimited stamina");
	}
public:
	SpawnHorse(string caption, int value)
		: MenuItemDefault(caption),
		m_value(value) {
	}
};

class GetWeapons : public MenuItemDefault
{
	virtual void OnSelect()
	{
		Actor localActor = UNSORTED::GET_PLAYER_ACTOR(UNSORTED::GET_LOCAL_SLOT());
		for (int i = 0; i <= 20; i++)
		{
			WeaponModel weapon = static_cast<WeaponModel>(i);
			UNSORTED::GIVE_WEAPON_TO_ACTOR(UNSORTED::GET_PLAYER_ACTOR(UNSORTED::GET_LOCAL_SLOT()),weapon,1,1,1);
			UNSORTED::_SET_ACTOR_INFINITE_AMMO(localActor, weapon, 1);
		}
		//UNSORTED::PRINT_HELP_B("You got all weapons with unlimited ammo", 5.0f, true, 2, 1, 0, 0, 0);
		_PRINT_BOTTOM_FORMAT("You got all weapons with unlimited ammo");
	}
public:
	GetWeapons(string caption)
		: MenuItemDefault(caption) {
	}
};

class InfiniteAmmo : public MenuItemSwitchable
{
	virtual void OnFrame()
	{
		if (GetState())
		{
			Player localActor = UNSORTED::GET_PLAYER_ACTOR(UNSORTED::GET_LOCAL_SLOT());
			for (int i = 0; i <= 20; i++)
			{
				WeaponModel weapon = static_cast<WeaponModel>(i);
				UNSORTED::_SET_ACTOR_INFINITE_AMMO(localActor, weapon, 1);
			}
			if (UNSORTED::IS_ACTOR_SHOOTING(localActor))
			{
				UNSORTED::_ADD_AMMO_OF_TYPE(localActor, WEAPON::GET_AMMO_ENUM(UNSORTED::GET_WEAPON_IN_HAND(localActor)), 100, 1, 0);
				UNSORTED::ACTOR_SET_WEAPON_AMMO(localActor, UNSORTED::GET_WEAPON_IN_HAND(localActor), 55);
			}
		}
		else
		{
			Player localActor = UNSORTED::GET_PLAYER_ACTOR(UNSORTED::GET_LOCAL_SLOT());
			for (int i = 0; i <= 20; i++)
			{
				WeaponModel weapon = static_cast<WeaponModel>(i);
				UNSORTED::_SET_ACTOR_INFINITE_AMMO(localActor, weapon, 0);
			}
		}
	}
public:
	InfiniteAmmo(string caption)
		: MenuItemSwitchable(caption) {
	}
};

class ExplosiveBullets : public MenuItemSwitchable
{
	virtual void OnSelect()
	{
		bool newState = !GetState();
		if (!newState)
		{
			explosiveBullets = false;
		}
		else
		{
			explosiveBullets = true;
		}
		SetState(newState);
	}
	virtual void OnFrame()
	{
		if (GetState()) 
		{
			Vector3 PlayerRetPos;
			Vector3 damage = Vector3(1.0f, 1.0f, 1.0f);
			Actor localActor = UNSORTED::GET_PLAYER_ACTOR(UNSORTED::GET_LOCAL_SLOT());
			bool isPlayerAiming = UNSORTED::IS_PLAYER_WEAPON_ZOOMED(localActor);
			bool isPlayerFiring = UNSORTED::IS_ACTOR_SHOOTING(localActor);
			if (isPlayerAiming)
			{
				TARGETING::GET_RETICLE_TARGET_POINT(localActor, &PlayerRetPos);
				if (isPlayerFiring && explosiveBullets)
				{
					UNSORTED::_CREATE_EXPLOSION(&PlayerRetPos, "ExplosionMedium", true, &damage, true);
				}
			}
		}
	}

public:
	ExplosiveBullets(string caption)
		: MenuItemSwitchable(caption) {
	}
};

class TeleportToWaypoint : public MenuItemDefault
{
	void WaitForWorldToLoad(bool usePrint)
	{
		while (!UNSORTED::STREAMING_IS_WORLD_LOADED())
		{
			UNSORTED::SET_PLAYER_CONTROL(0, 0, 0, 0);
			if (usePrint)
			{
				//UNSORTED::PRINT_HELP_B("Waiting for world to load", 1.0f, true, 1, 0, 0, 0, 0);
				_PRINT_BOTTOM_FORMAT("Waiting for world to load");
			}
			ScriptWait(0);
		}
		UNSORTED::SET_PLAYER_CONTROL(0, 1, 0, 0);
	}

	void TeleportActorToPosition(Actor actor, Vector3 targetPosition)
	{
		float actorHeading = UNSORTED::GET_HEADING(actor);
		if (UNSORTED::IS_ACTOR_RIDING(actor))
			UNSORTED::TELEPORT_ACTOR(UNSORTED::GET_MOUNT(actor), &targetPosition, 1, 1, 1);
		else if (UNSORTED::IS_ACTOR_DRIVING_VEHICLE(actor))
			UNSORTED::TELEPORT_ACTOR(UNSORTED::GET_VEHICLE(actor), &targetPosition, 1, 1, 1);
		else
			UNSORTED::TELEPORT_ACTOR_WITH_HEADING(actor, &targetPosition, actorHeading, 1, 1, 1);
	}

	void TeleportToWaypointPosition()
	{
		Vector3 waypointPosition, savedWaypointPosition, initialPosition;
		Actor playerActor = UNSORTED::GET_PLAYER_ACTOR(UNSORTED::GET_LOCAL_SLOT());
		initialPosition = UNSORTED::GET_POSITION(playerActor);
		UNSORTED::GET_USER_DEFINED_WAYPOINT(&waypointPosition);
		waypointPosition.y = 360.0f; // Hack to address specific issues
		savedWaypointPosition.x = waypointPosition.x;
		savedWaypointPosition.z = waypointPosition.z;

		if (waypointPosition.x != 0.0f && waypointPosition.y != 0.0f && waypointPosition.z != 0.0f) // Ensure waypoint is valid
		{
			waypointPosition.y = 360.0f;
			UNSORTED::TELEPORT_ACTOR(playerActor, &waypointPosition, 1, 1, 1);
			WaitForWorldToLoad(true);
			Time teleportStartTime = CORE::GET_CURRENT_GAME_TIME();

			while (!UNSORTED::IS_ACTOR_ON_GROUND(playerActor))
			{
				if (CORE::GET_CURRENT_GAME_TIME() > (teleportStartTime + 12.0f))
				{
					TeleportActorToPosition(playerActor, initialPosition);
					return;
				}
				waypointPosition.y -= 5;
				savedWaypointPosition.y = waypointPosition.y;
				UNSORTED::TELEPORT_ACTOR(playerActor, &savedWaypointPosition, 1, 1, 1);
				ScriptWait(0);
			}
		}
	}

	int menuValue;
	virtual void OnSelect()
	{
		Vector3 userWaypoint;
		UNSORTED::GET_USER_DEFINED_WAYPOINT(&userWaypoint);
		userWaypoint.y = 360.0f;
		if (userWaypoint.x != 0.0f && userWaypoint.y != 0.0f && userWaypoint.z != 0.0f)
		{
			TeleportToWaypointPosition();
		}
	}
public:
	TeleportToWaypoint(string caption, int value)
		: MenuItemDefault(caption),
		menuValue(value) {
	}
};

MenuBase* CreateMainMenu(MenuController* controller)
{
	auto menu = new MenuBase(new MenuItemTitle("RDR1 BASIC TRAINER"));
	controller->RegisterMenu(menu);

	//menu->AddItem(new demo("abcd", 1000000));
	menu->AddItem(new Invincible("Invincible"));
	menu->AddItem(new SpawnHorse("Spawn horse", 1000000));
	menu->AddItem(new GetWeapons("Get weapons"));
	menu->AddItem(new InfiniteAmmo("Infinite ammo"));
	menu->AddItem(new ExplosiveBullets("Explosive bullets"));
	menu->AddItem(new TeleportToWaypoint("Teleport to waypoint", 1000000));
	return menu;
}


void ModMenu::Update() 
{
	auto menuController = new MenuController();
	auto mainMenu = CreateMainMenu(menuController);

	while (true) 
	{
		if (!menuController->HasActiveMenu() && MenuInput::MenuSwitchPressed())
		{
			MenuInput::MenuInputBeep();
			menuController->PushMenu(mainMenu);
		}
		menuController->Update();

		ScriptWait(0);
	}
}


