/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#pragma once

#ifndef CIV5_MILITARY_AI_H
#define CIV5_MILITARY_AI_H

#include "CvEnumMap.h"

class CvArmyAI;
class CvCity;

enum DefenseState
{
    NO_DEFENSE_STATE = -1,
    DEFENSE_STATE_ENOUGH,
    DEFENSE_STATE_NEUTRAL,
    DEFENSE_STATE_NEEDED,
    DEFENSE_STATE_CRITICAL,
};

const char* ArmyTypeToString(ArmyType eType);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvMilitaryAIStrategyXMLEntry
//!  \brief		A single entry in the AI strategy XML file
//
//!  Key Attributes:
//!  - Populated from XML\???? (not sure what path/name you want)
//!  - Array of these contained in CvAIStretegies class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvMilitaryAIStrategyXMLEntry: public CvBaseInfo
{
public:
	CvMilitaryAIStrategyXMLEntry();
	virtual ~CvMilitaryAIStrategyXMLEntry();

	virtual bool CacheResults(Database::Results& kResults, CvDatabaseUtility& kUtility);

	// Accessor Functions
	int GetPlayerFlavorValue(int i) const;
	int GetCityFlavorValue(int i) const;

	int GetWeightThreshold() const;
	int GetPersonalityFlavorThresholdMod(int i) const;

	bool IsNoMinorCivs() const;
	bool IsOnlyMinorCivs() const;
	bool RequiresCitySpecializationUpdate() const;

	int GetTechPrereq() const;
	int GetTechObsolete() const;
	int GetMinimumNumTurnsExecuted() const;
	int GetCheckTriggerTurnCount() const;
	int GetFirstTurnExecuted() const;

	AdvisorTypes GetAdvisor() const;
	const char* GetAdvisorCounselText() const;
	int GetAdvisorCounselImportance() const;

private:
	int* m_piPlayerFlavorValue;
	int* m_piCityFlavorValue;

	int m_iWeightThreshold;
	int* m_piPersonalityFlavorThresholdMod;

	bool m_bNoMinorCivs;
	bool m_bOnlyMinorCivs;
	bool m_bUpdateCitySpecializations;

	int m_iTechPrereq;
	int m_iTechObsolete;
	int m_iMinimumNumTurnsExecuted;
	int m_iCheckTriggerTurnCount;
	int m_iFirstTurnExecuted;

	AdvisorTypes m_eAdvisor;
	CvString m_strAdvisorCounselText;
	int m_iAdvisorCounselImportance;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvMilitaryAIStrategyXMLEntries
//!  \brief		Game-wide information about possible AI strategies
//
//! Key Attributes:
//! - Plan is it will be contained in CvGameRules object within CvGame class
//! - Populated from XML\???? (not sure what path/name you want)
//! - Contains an array of CvAIStrategyXMLEntry from the above XML file
//! - One instance for the entire game
//! - Accessed heavily by CvPlayerAIStrategy class (which stores the AI strategy state for 1 player)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvMilitaryAIStrategyXMLEntries
{
public:
	CvMilitaryAIStrategyXMLEntries(void);
	~CvMilitaryAIStrategyXMLEntries(void);

	// Accessor functions
	std::vector<CvMilitaryAIStrategyXMLEntry*>& GetMilitaryAIStrategyEntries();
	int GetNumMilitaryAIStrategies();
	_Ret_maybenull_ CvMilitaryAIStrategyXMLEntry* GetEntry(int index);

	void DeleteArray();

private:
	std::vector<CvMilitaryAIStrategyXMLEntry*> m_paAIStrategyEntries;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  STRUCT:     CvMilitaryTarget
//!  \brief		A possible operation target (and muster city) for evaluation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct CvAttackTarget
{
	CvAttackTarget() : m_armyType(ARMY_TYPE_ANY), m_iMusterPlotIndex(-1), m_iStagingPlotIndex(-1), m_iTargetPlotIndex(-1), m_iPathLength(0), m_iApproachScore(0), m_bPreferred(false) {}
	ArmyType m_armyType;
	int m_iMusterPlotIndex;
	int m_iStagingPlotIndex;
	int m_iTargetPlotIndex;
	int m_iPathLength;
	int m_iApproachScore;
	bool m_bPreferred;

	PlayerTypes GetAttacker() const;
	PlayerTypes GetDefender() const;
	void SetWaypoints(const SPath&);
	CvPlot* GetMusterPlot() const;
	CvPlot* GetStagingPlot() const;
	CvPlot* GetTargetPlot() const;
	int GetPathLength() const;
	bool IsValid() const;
	bool IsPreferred() const;
};

FDataStream& operator<<(FDataStream&, const CvAttackTarget&);
FDataStream& operator>>(FDataStream&, CvAttackTarget&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvMilitaryAI
//!  \brief		Information about the AI military strategies of a single player
//
//!  Key Attributes:
//!  - Plan is it will be contained in CvPlayerState object within CvPlayer class
//!  - One instance for each civ (player or AI)
//!  - Accessed by any class that needs to check a civ's AI strategy state
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct CvStrikeWindow;
class CvMilitaryAI
{
public:
	CvMilitaryAI(void);
	~CvMilitaryAI(void);
	void Init(CvMilitaryAIStrategyXMLEntries* pAIStrategies, CvPlayer* pPlayer, CvDiplomacyAI* pDiplomacyAI);
	void Uninit();
	void Reset();
	template<typename MilitaryAI, typename Visitor>
	static void Serialize(MilitaryAI& militaryAI, Visitor& visitor);
	void Read(FDataStream& kStream);
	void Write(FDataStream& kStream) const;

	CvPlayer* GetPlayer();
	CvMilitaryAIStrategyXMLEntries* GetMilitaryAIStrategies();

	bool IsUsingStrategy(MilitaryAIStrategyTypes eStrategy);
	void SetUsingStrategy(MilitaryAIStrategyTypes eStrategy, bool bValue);
	int GetTurnStrategyAdopted(MilitaryAIStrategyTypes eStrategy);
	void SetTurnStrategyAdopted(MilitaryAIStrategyTypes eStrategy, int iValue);
	void DoTurn();

	DefenseState GetLandDefenseState() const
	{
		return m_eLandDefenseState;
	};
	DefenseState GetNavalDefenseState() const
	{
		return m_eNavalDefenseState;
	};

	// Requests for specific military operations
	bool RequestCityAttack(PlayerTypes eIntendedTarget, int iNumUnitsWillingToBuild, bool bCareful=true, const CvCity* pPreferredCity=NULL);	// AI_AB_TEST: pPreferredCity, the strike's objective
	bool RequestPillageAttack(PlayerTypes eEnemy);
	bool RequestNukeAttack(PlayerTypes eEnemy);
	bool RequestBullyingOperation(PlayerTypes eEnemy);

	// Emergency purchases
	CvUnit* BuyEmergencyUnit(UnitAITypes eUnitType, CvCity* pCity);
	bool BuyEmergencyBuilding(CvCity* pCity);

	// Finding best cities to target
	bool HavePossibleAttackTarget(PlayerTypes eEnemy) const;
	bool HaveStrikeLandAttackTarget(PlayerTypes eEnemy) const;	// AI_AB_TEST: a city of eEnemy is a land attack target (the strike's reach)
	const CvCity* GetStrikeObjectiveCity(PlayerTypes eTarget) const;	// AI_AB_TEST: the target's original capital (the strike's objective)
	bool IsStrikeTargetReachable(PlayerTypes eTarget) const;	// AI_AB_TEST: a land army path to its goal city (the capital once seen); our own turn only
	void RefreshStrikeSnapshot();	// AI_AB_TEST: the strike's stored answers, in UpdateStrikeState
	bool ComputeStrikeTargetTakeable() const;	// AI_AB_TEST: the snapshot's takeable computation (our own turn only)
	bool IsStrikeTargetDone(PlayerTypes eTarget) const;	// AI_AB_TEST: a major whose original capital is out of its hands
	bool IsStrikeTargetValid(PlayerTypes eTarget, bool bNewTarget = true) const;	// a target the strike can still declare on and reach
	bool IsStrikeHoldingReachableTarget() const;	// AI_AB_TEST: the strike holds a target a land army can ever reach
	bool IsStrikeTargetFinished(PlayerTypes eTarget) const;	// AI_AB_TEST: dead, or (after a capture in this war) its original capital out of its hands, or out of reach
	bool HavePreferredAttackTarget(PlayerTypes eEnemy) const;
	bool IsPossibleAttackTarget(const CvCity* pCity, ArmyType eArmyType) const;
	bool IsPreferredAttackTarget(const CvCity* pCity) const;
	bool IsExposedToEnemy(const CvCity* pCity, PlayerTypes eOtherPlayer, ArmyType eArmyType=ARMY_TYPE_ANY) const;
	bool IsPossibleMusterCity(const CvCity* pCity, ArmyType eArmyType) const;

	bool IsPlayerValid(PlayerTypes eOtherPlayer) const;

	size_t UpdateAttackTargets();
	map<int, SPath> GetArmyPathsFromCity(CvCity* pMusterCity, bool bWater);
	int ScoreAttackTarget(const CvAttackTarget& target);

	// Accessors to provide military data to other AI subsystems
	bool ShouldFightBarbarians() const;
	int GetNumberCivsAtWarWith(bool bIncludeMinor = true) const;
	// AI_AB_TEST (candidate, flag TimingBuildup): peace with garrisons only, a build-up window once a target is found, then the attack
	bool IsTimingBuildupPlayer() const;
	bool IsTimingBuildupPeace() const;
	bool IsInTimingBuildupWindow() const;
	bool HasTimingBuildupWindowEnded() const;
	bool IsTimingBuildupArmyReady() const;
	// AI_AB_TEST (diagnostic flag BigArmy): the ready army's size, doubled under BigArmy, and a log of the army
	bool IsTimingBuildupBigArmy() const;
	bool IsTimingBuildupWarBudget() const;	// AI_AB_TEST (flag WarBudget): the army target is what gold pays for
	int GetTimingBuildupFieldUnits() const;	// the field units the ready test counts (ArmyQuality's floor when on)
	// AI_AB_TEST (flag EarlyStrike, War's half): a timing attacker on Economy's EarlyStrike book, and its strike's build-up
	bool IsEarlyStrikeCiv() const;
	bool IsEarlyStrikeBuildup() const;
	bool IsEarlyStrikeTargetUnwalled() const;
	bool GetEarlyStrikeWindow(CvStrikeWindow& kWindow) const;	// Personality's per-civ plan (CvGrandStrategyAI)
	int GetEarlyStrikeMeleeUnits() const;	// land units that can take a city (melee, not siege, not no-capture)
	int GetEarlyStrikeCoreSharePercent() const;	// Personality's core share (50 if none)
	UnitClassTypes GetEarlyStrikeCore() const;	// Economy's plan core (the Huns: Horse Archer), else the profile's pick
	// AI_AB_TEST (flag LateStrike): a late-spike civ's timed strike on Personality's strike plan
	bool IsLateStrikeCiv() const;
	bool IsLateStrikeOpen() const;		// the plan's window has opened: its open turn and the core unit trainable, or its fallback turn
	bool IsLateStrikeBuildup() const;	// the build-up under way is the late strike's
	bool IsStrikeBuildup() const;		// EarlyStrike's or LateStrike's build-up
	int GetLateStrikeFieldFloor() const;	// the plan's core + support
	static const int STRIKE_TAKE_DAMAGE_PERCENT = 90;	// IsStrikeArmyStrongEnough's buffer (the user: "a little buffer"): our damage counted at 90%
	static const int STRIKE_TAKE_LOSS_PERCENT = 110;	// and the city's damage to us at 110% (calibrated: the user's Marrakesh reads GO)
	static const int EARLY_STRIKE_MELEE_MIN = 3;	// "bring two in case one dies" (the strike-plans research): 2 at the city, 1 spare
	int GetEarlyStrikeUpgradeBill(int* piUnits = NULL, bool* pbTechSoon = NULL) const;	// upgrade-and-go (the user's Composite Bowmen)
	bool IsEarlyStrikeHoldingUpgrades() const;	// no upgrades until the strike's army is built (Economy: keep the reserve)
	bool IsTimingBuildupArmyBuilt() const;		// the ready test's unit counts, without EarlyStrike's upgrades
	bool IsBigArmyUpkeepWaived() const;
	// AI_AB_TEST (flag ArmyQuality): the strength floor for the TimingBuildup army (70% of the best trainable melee / ranged land unit)
	int GetArmyQualityFloor(bool bRanged) const;
	bool IsArmyQualityUnit(const CvUnitEntry* pkUnitEntry) const;
	int GetTimingBuildupSiegeTarget() const;
	int GetTimingBuildupFieldTarget() const;
	void LogBigArmy(const char* szWhen) const;
	int GetTimingBuildupPeaceCap() const;
	void StartTimingBuildup(PlayerTypes eTarget);
	// read-only plan for other subsystems (Economy's WarChest and WarEconomy)
	enum TimingBuildupPhase { TIMING_BUILDUP_NONE, TIMING_BUILDUP_PEACE, TIMING_BUILDUP_PLANNED, TIMING_BUILDUP_WINDOW, TIMING_BUILDUP_ATTACK, TIMING_BUILDUP_WAR };
	// AI_AB_TEST (flags EarlyStrike, LateStrike; the user: "dont have the ai judge itself based on the window during a game"): the
	// strike's state, from game state only (no turns), worked out once a turn and saved. Economy's book reads it (patch 0034).
	enum StrikeStateTypes { STRIKE_NONE, STRIKE_PREP, STRIKE_BUILDUP, STRIKE_WAR, STRIKE_OVER };
	StrikeStateTypes GetStrikeState() const { return (StrikeStateTypes)m_iStrikeState; }
	void SetStrikeDeclared(PlayerTypes eTarget = NO_PLAYER);	// CvDiplomacyAI::DeclareWar: a declaration of ours (the lead's review: a city-state's war on us isn't one). Economy 0052: on a city-state it counts once the war still stands at the next strike update
	bool HasStrikeDeclared() const { return m_iStrikeDeclared == 1; }	// we have declared a war: the strike's (BUILDUP -> WAR, its city-state) or any of our own since the game began (Economy's bans end here; public for CvPlayer, Economy 0045) (0052: confirmed only; 2 + a city-state's index is a declaration waiting for confirmation)
	bool IsStrikeArmyStrongEnough(const CvCity* pCity, int* piTurnsNeeded = NULL) const;	// can the plan's (upgraded) army take the city (the game's damage math)
	bool IsStrikeTargetTakeableNow(int* piTurnsNeeded = NULL) const;	// can the field army on hand take the strike target's nearest city
	bool StrikeUnitsTakeCity(const std::vector<const CvUnit*>& vUnits, const CvCity* pCity, int* piTurnsNeeded = NULL) const;	// the strike's math for these units, defenders near the city subtracted
	bool IsStrikeRefusingPeace(PlayerTypes eOther) const;	// ownership gate (2-4): no peace, approach WAR, no retreat aborts toward this player
	bool IsStrikeKeepingUnit(const CvUnit* pUnit) const;	// ownership gate (5): a strike unit no CP system disbands or gifts
	bool IsStrikeOwningOperations() const;	// ownership gate (4): PREP, BUILDUP, or WAR before the target's capital is taken
	bool IsStrikeOperationTarget(PlayerTypes eOther) const;	// a player the strike's army may attack
	int GetStrikeCaptureCount() const;	// cities captured from majors and city-states
	int GetStrikeTargetCaptures() const;	// our cities whose previous owner is the strike's target (the strike's capture measure)
	bool IsStrikeNoRetreat(PlayerTypes eEnemy) const;	// the strike's war on this player, nothing taken yet: no peace, no abort
	bool IsStrikeHomeSafe() const;	// none of our cities in danger of falling: a strike's declaration waits for it
	bool IsStrikeDeclarationBlocked(PlayerTypes eOn) const;	// before the strike (PREP, BUILDUP) no declaration of ours but the strike's on its target, when ready
	bool IsStrikeArmyBuildable() const;	// the core, or the plan's pre-build, can be trained
	bool IsStrikeClassBuildable(UnitClassTypes eClass) const;	// trainable now, or but for resources we own and have in use
	bool IsStrikeAsReadyAsItGets() const;	// a normal army, 3 melee, and nothing more affordable
	PlayerTypes GetStrikeCityStateTarget() const { return (PlayerTypes)m_iStrikeCityState; }	// EarlyStrike: a city-state when no major is in reach, or a re-target (major or city-state)
	TimingBuildupPhase GetTimingBuildupPhase() const;
	PlayerTypes GetTimingBuildupTarget() const;
	int GetTimingBuildupUnitsWanted() const;
	UnitTypes GetTimingBuildupUnitType(bool bSiege) const;

	int GetRecommendedMilitarySize() const
	{
		return m_iRecLandUnits + m_iRecNavalUnits + m_iRecExplorerUnits;
	};

	int GetPowerOfStrongestBuildableUnit(DomainTypes eDomain);
	bool HasAirforce() const
	{
		return m_iNumAirUnits > 0;
	};
	CvUnit* FindUselessShip();
	CvUnit* FindArmyScrapUnit() const;	// AI_AB_TEST (flag ArmyScrap)
	CvUnit* FindUnitToScrap(DomainTypes eDomain, bool bCheckObsolete, int& iReturnedScore, bool bOverSupplyCap, bool bInDeficit);
	bool IsBuildingArmy(ArmyType eType) const;
	UnitTypes GetUnitTypeForArmy(CvCity* pCity) const;
	bool CanCityHelpFillArmy(const CvCity* pCity, const CvArmyAI* pArmy) const;
	bool ShouldCityHelpFillArmies(const CvCity* pCity) const;
	int GetNumEnemyAirUnitsInRange(CvPlot* pCenterPlot, int iRange, bool bCountFighters, bool bCountBombers) const;
	CvPlot *GetBestAirSweepTarget(CvUnit* pFighter) const;

	int GetNumberOfTimesOpsBuildSkippedOver() const
	{
		return m_iNumberOfTimesOpsBuildSkippedOver;
	}
	void BumpNumberOfTimesOpsBuildSkippedOver()
	{
		++m_iNumberOfTimesOpsBuildSkippedOver;
	}
	void ResetNumberOfTimesOpsBuildSkippedOver()
	{
		m_iNumberOfTimesOpsBuildSkippedOver = 0;
	}

	int GetNumberOfTimesSettlerBuildSkippedOver() const
	{
		return m_iNumberOfTimesSettlerBuildSkippedOver;
	}
	void BumpNumberOfTimesSettlerBuildSkippedOver()
	{
		++m_iNumberOfTimesSettlerBuildSkippedOver;
	}
	void ResetNumberOfTimesSettlerBuildSkippedOver()
	{
		m_iNumberOfTimesSettlerBuildSkippedOver = 0;
	}

	// Public logging functions
	void LogDeclarationOfWar(PlayerTypes eOpponent);
	void LogCityCaptured(CvCity* pCity, PlayerTypes eOldOwner);
	void LogCityRazed(CvCity* pCity, PlayerTypes eOldOwner);
	void LogPeace(PlayerTypes eOpponent);
	void LogPeace(TeamTypes eOpponentTeam);
	void LogDeficitScrapUnit(CvUnit* pUnit, bool bGifted);

	WarTypes GetWarType(PlayerTypes ePlayer = NO_PLAYER);
	void UpdateWarType();
	void SetupInstantDefenses(PlayerTypes ePlayer);

	int GetRecommendLandArmySize() const
	{
		return m_iRecLandUnits;
	};
	int GetRecommendNavySize() const
	{
		return m_iRecNavalUnits;
	};
	int GetRecommendedExplorers() const
	{
		return m_iRecExplorerUnits;
	};
	int GetNumLandUnits() const
	{
		return m_iNumLandUnits;
	};
	int GetNumNavalUnits() const
	{
		return m_iNumNavalUnits;
	};
	int GetNumAAUnits() const
	{
		return m_iNumAntiAirUnits;
	};
	int GetNumFreeCarrier() const
	{
		return m_iNumFreeCarriers;
	}

private:

	// Functions to process a turn
	void UpdateBaseData();
	void ScanForBarbarians();
	void UpdateDefenseState();
	void UpdateMilitaryStrategies();
	void UpdateOperations();
	void DoNuke(PlayerTypes ePlayer);
	void CheckLandDefenses(PlayerTypes eEnemy, CvCity* pThreatenedCity);
	void CheckSeaDefenses(PlayerTypes ePlayer, CvCity* pThreatenedCity);
	void DoCityAttacks(PlayerTypes ePlayer);
	void SetRecommendedArmyNavySize();
	void MakeEmergencyPurchases();
	void DisbandObsoleteUnits();

	// Logging functions
	void LogStrategy(MilitaryAIStrategyTypes eStrategy, bool bValue);
	void LogWarStateChange(PlayerTypes ePlayer, WarStateTypes eNewWarState, WarStateTypes eOldWarState);
	void LogMilitaryStatus();
	void LogAvailableForces();
	void LogScrapUnit(CvUnit* pUnit, bool bDeficit, bool bSupply);
	void LogGiftUnit(CvUnit* pUnit, bool bDeficit, bool bSupply);
	CvString GetLogFileName(CvString& playerName, bool bSummary=false) const;

	CvPlayer* m_pPlayer;
	CvDiplomacyAI* m_pDiplomacyAI;
	CvMilitaryAIStrategyXMLEntries* m_pAIStrategies;
	bool* m_pabUsingStrategy;
	int* m_paiTurnStrategyAdopted;
	CvEnumMap<FlavorTypes, int> m_aiTempFlavors;
	int m_aiWarFocus[MAX_MAJOR_CIVS];

	// Internal calculated values - must be serialized
	int m_iNumberOfTimesOpsBuildSkippedOver;
	int m_iNumberOfTimesSettlerBuildSkippedOver;

	vector<CvAttackTarget> m_potentialAttackTargets; //enemy cities we might want to attack
	vector<CvAttackTarget> m_exposedCities; //those of our cities which might be tempting to the enemies

	int m_iNumLandUnits;
	int m_iNumRangedLandUnits;
	int m_iNumMobileLandUnits;
	int m_iNumMeleeLandUnits;
	int m_iNumNavalUnits;
	int m_iNumLandUnitsInArmies;
	int m_iNumNavalUnitsInArmies;
	int m_iRecNavalUnits;
	int m_iNumAirUnits;
	int m_iNumAntiAirUnits;
	int m_iBarbarianCampCount;
	int m_iVisibleBarbarianCount;
	int m_iRecLandUnits;
	int m_iRecExplorerUnits;
	int m_iTimingBuildupStartTurn;		// AI_AB_TEST TimingBuildup: -1 when no window
	PlayerTypes m_eTimingBuildupTarget;
	bool m_bTimingBuildupSaving;
	int m_iTimingBuildupPausedTurns;	// turns this window was paused while units were unaffordable
	int m_iStrikeState;				// AI_AB_TEST StrikeStateTypes (saved)
	int m_iStrikeWarsDeclaredMark;	// our wars declared / wars on us when the strike entered its state, cities captured at its war,
	int m_iStrikeWarsOnUsMark;		// and the land army at the declaration (saved): the exits compare against them
	int m_iStrikeCapturedMark;
	int m_iStrikeCitiesAtWar;		// our city count at the strike's war (saved): a loss to anyone (a city-state, barbarians) releases no-retreat
	int m_iStrikeLostMark;			// our cities lost (AB_COUNT_CITIES_LOST) at the strike's war (saved): no-retreat releases on a loss
	int m_iStrikeArmyAtWar;
	int m_iStrikeCityState;			// EarlyStrike's city-state target (PlayerTypes; the user: with no major in reach), saved
	void UpdateStrikeState();
	PlayerTypes FindStrikeCityState(bool bUnwalledOnly = false, PlayerTypes eExclude = NO_PLAYER) const;
	PlayerTypes FindStrikeRetarget(PlayerTypes eExclude, const CvCity* pCurrent) const;	// a clearly easier city in reach, a major's or a city-state's
	PlayerTypes FindStrikeMajorInReach() const;	// the nearest valid major in reach
	bool IsStrikeCoreLineObsolete(UnitClassTypes eCoreClass) const;	// every unit on the core's strike line (to the plan's upgrade) obsolete (the strike's obsolete exit)
	bool CanBuildStrikeIncomeItem() const;	// the capital can build a Market or a Caravan
	bool IsStrikeCoreUntrainable() const;	// the core (and the pre-build) can't be trained with its tech in: a resource missing
	int m_iStrikeDeclared;			// HasStrikeDeclared, saved
	int m_iStrikeSiege;				// the build-up brings siege (the army too weak for the target, no easier city in reach), saved
	mutable int m_iArmyQualityTurn;		// AI_AB_TEST ArmyQuality: the turn the floor was computed (not saved, recomputed)
	mutable int m_iSiegeTrainableTurn;	// AI_AB_TEST strike: the turn the siege check was made, and its answer (not saved)
	mutable bool m_bSiegeTrainable;
	bool m_bTakeable;					// AI_AB_TEST strike snapshot (RefreshStrikeSnapshot, our own turn only; saved): takeable now,
	bool m_bStrikeTargetFinished;		// the strike's target finished,
	bool m_bStrikeHoldingReachable;		// a held target an army can reach,
	bool m_bStrikeReady;				// and the army ready or as ready as it gets (the declaration block)
	int m_iStrikeNotReadyKey;			// AI_AB_TEST strike: the last logged "not ready" blockers (not saved; logs only)
	mutable int m_iArmyQualityFloor;
	mutable int m_iArmyQualityRangedFloor;
	void DoTimingBuildup();
	int m_iNumFreeCarriers;

	//new unit counters
	int m_iNumArcherLandUnits;
	int m_iNumSiegeLandUnits;
	int m_iNumSkirmisherLandUnits;
	int m_iNumReconLandUnits;
	int m_iNumBomberAirUnits;
	int m_iNumFighterAirUnits;
	int m_iNumMeleeNavalUnits;
	int m_iNumRangedNavalUnits;
	int m_iNumSubmarineNavalUnits;
	int m_iNumCarrierNavalUnits;
//	int m_iNumNukeUnits; CvPlayer has this
	int m_iNumMissileUnits;
	int m_iNumActiveUniqueUnits;

	DefenseState m_eLandDefenseState;
	DefenseState m_eNavalDefenseState;
};

FDataStream& operator>>(FDataStream&, CvMilitaryAI&);
FDataStream& operator<<(FDataStream&, const CvMilitaryAI&);

FDataStream& operator<<(FDataStream&, const DefenseState&);
FDataStream& operator>>(FDataStream&, DefenseState&);

namespace MilitaryAIHelpers
{
int GetWeightThresholdModifier(MilitaryAIStrategyTypes eStrategy, CvPlayer* pPlayer);

// Functions that check triggers to see if a strategy should be adopted/continued
bool IsTestStrategy_EnoughMilitaryUnits(CvPlayer* pPlayer);
bool IsTestStrategy_EmpireDefense(CvPlayer* pPlayer);
bool IsTestStrategy_EmpireDefenseCritical(CvPlayer* pPlayer);
bool IsTestStrategy_EnoughNavalUnits(CvPlayer* pPlayer);
bool IsTestStrategy_NeedNavalUnits(CvPlayer* pPlayer);
bool IsTestStrategy_NeedNavalUnitsCritical(CvPlayer* pPlayer);
bool IsTestStrategy_WarMobilization(MilitaryAIStrategyTypes eStrategy, CvPlayer* pPlayer);
bool IsTestStrategy_AtWar(CvPlayer* pPlayer, bool bMinor = true);
bool IsTestStrategy_MinorCivGeneralDefense();
bool IsTestStrategy_MinorCivThreatElevated(CvPlayer* pPlayer);
bool IsTestStrategy_MinorCivThreatCritical(CvPlayer* pPlayer);
bool IsTestStrategy_EradicateBarbarians(MilitaryAIStrategyTypes eStrategy, CvPlayer* pPlayer, int iBarbarianCampCount, int iVisibleBarbarianCount);
bool IsTestStrategy_EradicateBarbariansCritical(MilitaryAIStrategyTypes eStrategy, CvPlayer* pPlayer, int iBarbarianCampCount, int iVisibleBarbarianCount);
bool IsTestStrategy_WinningWars(CvPlayer* pPlayer);
bool IsTestStrategy_LosingWars(CvPlayer* pPlayer);
bool IsTestStrategy_EnoughRangedUnits(CvPlayer* pPlayer, int iNumRanged, int iNumMelee);
bool IsTestStrategy_NeedRangedUnits(CvPlayer* pPlayer, int iNumRanged, int iNumMelee);
bool IsTestStrategy_NeedRangedDueToEarlySneakAttack(CvPlayer* pPlayer);
bool IsTestStrategy_EnoughMobileUnits(CvPlayer* pPlayer, int iNumMobile, int iNumMelee);
bool IsTestStrategy_NeedMobileUnits(CvPlayer* pPlayer, int iNumMobile, int iNumMelee);
bool IsTestStrategy_EnoughAirUnits(CvPlayer* pPlayer, int iNumAir, int iNumMelee);
bool IsTestStrategy_NeedAirUnits(CvPlayer* pPlayer, int iNumAir, int iNumMelee);
bool IsTestStrategy_NeedANuke(CvPlayer* pPlayer);
bool IsTestStrategy_EnoughAntiAirUnits(CvPlayer* pPlayer, int iNumAA, int iNumMelee);
bool IsTestStrategy_NeedAntiAirUnits(CvPlayer* pPlayer, int iNumAA, int iNumMelee);
bool IsTestStrategy_NeedAirCarriers(CvPlayer* pPlayer);

// MOD_AI_UNIT_PRODUCTION
//VP - new strategies
bool IsTestStrategy_EnoughArcherUnits(CvPlayer* pPlayer, int iNumArcher, int iNumMelee);
bool IsTestStrategy_NeedArcherUnits(CvPlayer* pPlayer, int iNumArcher, int iNumMelee);
bool IsTestStrategy_EnoughSiegeUnits(CvPlayer* pPlayer, int iNumSiege, int iNumMelee);
bool IsTestStrategy_NeedSiegeUnits(CvPlayer* pPlayer, int iNumSiege, int iNumMelee);
bool IsTestStrategy_EnoughSkirmisherUnits(CvPlayer* pPlayer, int iNumSkirmisher, int iNumMelee);
bool IsTestStrategy_NeedSkirmisherUnits(CvPlayer* pPlayer, int iNumSkirmisher, int iNumMelee);
bool IsTestStrategy_EnoughNavalMeleeUnits(CvPlayer* pPlayer, int iNumNavalMelee, int iNumNaval);
bool IsTestStrategy_NeedNavalMeleeUnits(CvPlayer* pPlayer, int iNumNavalMelee, int iNumNaval);
bool IsTestStrategy_EnoughNavalRangedUnits(CvPlayer* pPlayer, int iNumNavalRanged, int iNumNaval);
bool IsTestStrategy_NeedNavalRangedUnits(CvPlayer* pPlayer, int iNumNavalRanged, int iNumNaval);
bool IsTestStrategy_EnoughSubmarineUnits(CvPlayer* pPlayer, int iNumSubmarine, int iNumNaval);
bool IsTestStrategy_NeedSubmarineUnits(CvPlayer* pPlayer, int iNumSubmarine, int iNumNaval);
bool IsTestStrategy_EnoughBomberUnits(CvPlayer* pPlayer, int iNumBomber, int iNumAir);
bool IsTestStrategy_NeedBomberUnits(CvPlayer* pPlayer, int iNumBomber, int iNumAir);
bool IsTestStrategy_EnoughFighterUnits(CvPlayer* pPlayer, int iNumFighter, int iNumAir);
bool IsTestStrategy_NeedFighterUnits(CvPlayer* pPlayer, int iNumFighter, int iNumAir);

MultiunitFormationTypes GetCurrentBestFormationTypeForLandAttack();
MultiunitFormationTypes GetCurrentBestFormationTypeForCombinedAttack();
MultiunitFormationTypes GetCurrentBestFormationTypeForPureNavalAttack();
CvPlot* GetCoastalWaterNearPlot(CvPlot *pTarget, bool bCheckTeam = false);
bool NeedShipInArea(PlayerTypes ePlayer, CvLandmass* pWaterBody);

bool ArmyPathIsGood(const SPath& path, PlayerTypes eAttacker, PlayerTypes eIntendedEnemy, int iThresholdForDiscard);
int EvaluateTargetApproach(const CvAttackTarget& target, PlayerTypes ePlayer, ArmyType eArmyType);
void SetBestTargetApproach(CvAttackTarget& target, PlayerTypes ePlayer);
}

#endif //CIV5_MILITARY_AI_H