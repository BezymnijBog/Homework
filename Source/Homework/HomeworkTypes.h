#pragma once

#define ECC_Climbing ECC_GameTraceChannel1
#define ECC_InteractionVolume ECC_GameTraceChannel2
#define ECC_WallRunable ECC_GameTraceChannel3
#define ECC_Bullet ECC_GameTraceChannel4
#define ECC_Melee ECC_GameTraceChannel5

UENUM(BlueprintType)
enum class EEquipableItemType : uint8
{
	None = 0,
	Pistol,
	Rifle,
	Bow,
	Throwable,
	Melee
};

UENUM(BlueprintType)
enum class EAmunitionType : uint8
{
	None = 0,
	Arrows,
	Pistol,
	Rifle,
	ShotgunShells,
	FragGrenade,
	RifleGrenades,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EEquipmentSlots : uint8
{
	None,
	SideArm,
	PrimaryWeapon,
	SecondaryWeapon,
	PrimaryItemSlot,
	MeleeWeapon,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EReticleType : uint8
{
	None,
	Default,
	Sniper,
	Bow,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMeleeAttackTypes : uint8
{
	None,
	PrimaryAttack,
	SecondaryAttack,
	MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ETeams : uint8
{
    Player,
	Enemy
};

UENUM()
enum class EAttributes : uint8
{
	Health,
	Stamina,
	Oxygen
};

const FName FXParamTraceEnd = FName("Trace End");

const FName BoneHead = FName("head");

const FName SocketFPCamera = FName("CameraSocket");
const FName SocketCharacterThrowable = FName("GrenadeSocket");
const FName SocketCharacterWeapon = FName("CharacterWeaponSocket");
const FName SocketWeaponMuzzle = FName("MuzzleSocket");
const FName SocketWeaponForeGrip = FName("ForeGripSocket");

const FName CollisionProfilePawn = FName("Pawn");
const FName CollisionProfilePawnInteractionVolume = FName("PawnInteractionVolume");
const FName CollisionProfileNoCollision = FName("NoCollision");
const FName CollisionProfileNoRagdoll = FName("Ragdoll");

const FName SectionMontageReloadEnd = FName("ReloadEnd");

const FName DebugCategoryLedgeDetection = FName("LedgeDetection");
const FName DebugCategoryCharacterAttributes = FName("CharacterAttributes");
const FName DebugCategoryMeleeWeapon = FName("MeleeWeapon");
const FName DebugCategoryRangeWeapon = FName("RangeWeapon");

const FName BB_CurrentTraget = FName("CurrentTarget");
const FName BB_NextLocation = FName("NextLocation");

const FName ActionInteract = FName("Interact");