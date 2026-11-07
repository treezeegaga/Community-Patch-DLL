/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#include "CvGameCoreDLLPCH.h"
#include "ICvDLLUserInterface.h"
#include "CvGameCoreUtils.h"
#include "CvMinorCivAI.h"
#include "CvDiplomacyAI.h"
#include "CvEconomicAI.h"
#include "CvMilitaryAI.h"
#include "CvGrandStrategyAI.h"
#include "CvCitySpecializationAI.h"
#include "cvStopWatch.h"
#include "CvUnitCombat.h"
#include "CvSpanSerialization.h"
#include "SqliteLoggerRegistrations.h"

// must be included after all other headers
#include "LintFree.h"

// set this to 1 in debugger if needed
int gDebugOutput = 0;

CvMilitaryAIStrategyXMLEntry::CvMilitaryAIStrategyXMLEntry(void):
	m_piPlayerFlavorValue(NULL),
	m_piCityFlavorValue(NULL),
	m_iWeightThreshold(0),
	m_piPersonalityFlavorThresholdMod(NULL),
	m_bNoMinorCivs(false),
	m_bOnlyMinorCivs(false),
	m_bUpdateCitySpecializations(false),
	m_iTechPrereq(NO_TECH),
	m_iTechObsolete(NO_TECH),
	m_iMinimumNumTurnsExecuted(0),
	m_iCheckTriggerTurnCount(0),
	m_iFirstTurnExecuted(0),
	m_eAdvisor(NO_ADVISOR_TYPE),
	m_iAdvisorCounselImportance(1)
{
}
//------------------------------------------------------------------------------
CvMilitaryAIStrategyXMLEntry::~CvMilitaryAIStrategyXMLEntry(void)
{
	SAFE_DELETE_ARRAY(m_piPlayerFlavorValue);
	SAFE_DELETE_ARRAY(m_piCityFlavorValue);
	SAFE_DELETE_ARRAY(m_piPersonalityFlavorThresholdMod);
}
//------------------------------------------------------------------------------
bool CvMilitaryAIStrategyXMLEntry::CacheResults(Database::Results& kResults, CvDatabaseUtility& kUtility)
{
	if(!CvBaseInfo::CacheResults(kResults, kUtility))
		return false;

	//Basic Properties
	m_bNoMinorCivs				 = kResults.GetBool("NoMinorCivs");
	m_bOnlyMinorCivs		     = kResults.GetBool("OnlyMinorCivs");
	m_bUpdateCitySpecializations = kResults.GetBool("UpdateCitySpecializations");
	m_iCheckTriggerTurnCount	 = kResults.GetInt("CheckTriggerTurnCount");
	m_iFirstTurnExecuted            = kResults.GetInt("FirstTurnExecuted");
	m_iMinimumNumTurnsExecuted	 = kResults.GetInt("MinimumNumTurnsExecuted");
	m_iWeightThreshold			 = kResults.GetInt("WeightThreshold");

	const char* strAdvisor = kResults.GetText("Advisor");
	m_eAdvisor = NO_ADVISOR_TYPE;
	if(strAdvisor)
	{
		if(strcmp(strAdvisor, "MILITARY") == 0)
		{
			m_eAdvisor = ADVISOR_MILITARY;
		}
		else if(strcmp(strAdvisor, "ECONOMIC") == 0)
		{
			m_eAdvisor = ADVISOR_ECONOMIC;
		}
		else if(strcmp(strAdvisor, "FOREIGN") == 0)
		{
			m_eAdvisor = ADVISOR_FOREIGN;
		}
		else if(strcmp(strAdvisor, "SCIENCE") == 0)
		{
			m_eAdvisor = ADVISOR_SCIENCE;
		}
	}

	m_strAdvisorCounselText = kResults.GetText("AdvisorCounsel");

	//References
	const char* szTextVal = NULL;
	szTextVal = kResults.GetText("TechObsolete");
	m_iTechObsolete = GC.getInfoTypeForString(szTextVal, true);

	szTextVal = kResults.GetText("TechPrereq");
	m_iTechPrereq = GC.getInfoTypeForString(szTextVal, true);

	//Arrays
	const char* szType = GetType();
	kUtility.SetFlavors(m_piPlayerFlavorValue, "AIMilitaryStrategy_Player_Flavors",
	                    "AIMilitaryStrategyType", szType);
	kUtility.SetFlavors(m_piCityFlavorValue, "AIMilitaryStrategy_City_Flavors",
		"AIMilitaryStrategyType", szType);


	kUtility.SetFlavors(m_piPersonalityFlavorThresholdMod,
	                    "AIMilitaryStrategy_PersonalityFlavorThresholdMods",
	                    "AIMilitaryStrategyType", szType);

	return true;
}

/// What player flavors will be added by adopting this Strategy?
int CvMilitaryAIStrategyXMLEntry::GetPlayerFlavorValue(int i) const
{
	PRECONDITION(i < GC.getNumFlavorTypes(), "Index out of bounds");
	PRECONDITION(i > -1, "Index out of bounds");
	return m_piPlayerFlavorValue ? m_piPlayerFlavorValue[i] : -1;
}

/// What city flavors will be added by adopting this Strategy?
int CvMilitaryAIStrategyXMLEntry::GetCityFlavorValue(int i) const
{
	PRECONDITION(i < GC.getNumFlavorTypes(), "Index out of bounds");
	PRECONDITION(i > -1, "Index out of bounds");
	return m_piCityFlavorValue ? m_piCityFlavorValue[i] : -1;
}

/// The amount of weight a Strategy must accumulate to be adopted (if applicable)
int CvMilitaryAIStrategyXMLEntry::GetWeightThreshold() const
{
	return m_iWeightThreshold;
}

/// How do a player's Personality Flavors affect the Threshold for adopting a Strategy? (if applicable)
int CvMilitaryAIStrategyXMLEntry::GetPersonalityFlavorThresholdMod(int i) const
{
	PRECONDITION(i < GC.getNumFlavorTypes(), "Index out of bounds");
	PRECONDITION(i > -1, "Index out of bounds");
	return m_piPersonalityFlavorThresholdMod ? m_piPersonalityFlavorThresholdMod[i] : -1;
}

/// Is this a valid strategy for Minor Civs to use?
bool CvMilitaryAIStrategyXMLEntry::IsNoMinorCivs() const
{
	return m_bNoMinorCivs;
}

/// Are ONLY Minor Civs allowed to use this?
bool CvMilitaryAIStrategyXMLEntry::IsOnlyMinorCivs() const
{
	return m_bOnlyMinorCivs;
}

/// Should we reprioritize city specializations when this strategy goes on or off
bool CvMilitaryAIStrategyXMLEntry::RequiresCitySpecializationUpdate() const
{
	return m_bUpdateCitySpecializations;
}

/// Technology prerequisite an AI Player must have to enable this Strategy
int CvMilitaryAIStrategyXMLEntry::GetTechPrereq() const
{
	return m_iTechPrereq;
}

/// Technology that obsoletes this Strategy for an AI Player
int CvMilitaryAIStrategyXMLEntry::GetTechObsolete() const
{
	return m_iTechObsolete;
}

/// Minimum # of turns this AI Strategy must be executed for - don't want the AI starting then stopping Strategies every turn
int CvMilitaryAIStrategyXMLEntry::GetMinimumNumTurnsExecuted() const
{
	return m_iMinimumNumTurnsExecuted;
}

/// Once this Strategy has been adopted, check the Trigger conditions every X turns to see if it's maintained
int CvMilitaryAIStrategyXMLEntry::GetCheckTriggerTurnCount() const
{
	return m_iCheckTriggerTurnCount;
}

/// First turn to check for adoption
int CvMilitaryAIStrategyXMLEntry::GetFirstTurnExecuted() const
{
	return m_iFirstTurnExecuted;
}

AdvisorTypes CvMilitaryAIStrategyXMLEntry::GetAdvisor() const
{
	return m_eAdvisor;
}

const char* CvMilitaryAIStrategyXMLEntry::GetAdvisorCounselText() const
{
	return m_strAdvisorCounselText.c_str();
}

int CvMilitaryAIStrategyXMLEntry::GetAdvisorCounselImportance() const
{
	return m_iAdvisorCounselImportance;
}

//=====================================
// CvMilitaryAIStrategyXMLEntries
//=====================================
/// Constructor
CvMilitaryAIStrategyXMLEntries::CvMilitaryAIStrategyXMLEntries(void)
{

}

/// Destructor
CvMilitaryAIStrategyXMLEntries::~CvMilitaryAIStrategyXMLEntries(void)
{
	DeleteArray();
}

/// Returns vector of AIStrategy entries
std::vector<CvMilitaryAIStrategyXMLEntry*>& CvMilitaryAIStrategyXMLEntries::GetMilitaryAIStrategyEntries()
{
	return m_paAIStrategyEntries;
}

/// Number of defined AIStrategies
int CvMilitaryAIStrategyXMLEntries::GetNumMilitaryAIStrategies()
{
	return m_paAIStrategyEntries.size();
}

/// Clear AIStrategy entries
void CvMilitaryAIStrategyXMLEntries::DeleteArray()
{
	for(std::vector<CvMilitaryAIStrategyXMLEntry*>::iterator it = m_paAIStrategyEntries.begin(); it != m_paAIStrategyEntries.end(); ++it)
	{
		SAFE_DELETE(*it);
	}

	m_paAIStrategyEntries.clear();
}

/// Get a specific entry
_Ret_maybenull_ CvMilitaryAIStrategyXMLEntry* CvMilitaryAIStrategyXMLEntries::GetEntry(int index)
{
	return (index != NO_MILITARYAISTRATEGY) ? m_paAIStrategyEntries[index] : NULL;
}


//=====================================
// CvMilitaryAI
//=====================================

/// Constructor
CvMilitaryAI::CvMilitaryAI():
	m_pabUsingStrategy(NULL),
	m_paiTurnStrategyAdopted(NULL),
	m_iNumberOfTimesOpsBuildSkippedOver(0),
	m_iNumberOfTimesSettlerBuildSkippedOver(0)
{
}

/// Destructor
CvMilitaryAI::~CvMilitaryAI(void)
{
	Uninit();
}

/// Initialize
void CvMilitaryAI::Init(CvMilitaryAIStrategyXMLEntries* pAIStrategies, CvPlayer* pPlayer, CvDiplomacyAI* pDiplomacyAI)
{
	// Store off the pointer to the AIStrategies active for this game
	m_pAIStrategies = pAIStrategies;
	m_pPlayer = pPlayer;
	m_pDiplomacyAI = pDiplomacyAI;

	// Initialize arrays
	ASSERT(m_pabUsingStrategy==NULL, "about to leak memory, CvMilitaryAI::m_pabUsingStrategy");
	m_pabUsingStrategy = FNEW(bool[m_pAIStrategies->GetNumMilitaryAIStrategies()], c_eCiv5GameplayDLL, 0);

	ASSERT(m_paiTurnStrategyAdopted==NULL, "about to leak memory, CvMilitaryAI::m_paiTurnStrategyAdopted");
	m_paiTurnStrategyAdopted = FNEW(int[m_pAIStrategies->GetNumMilitaryAIStrategies()], c_eCiv5GameplayDLL, 0);

	m_aiTempFlavors.init();

	Reset();
}

/// Deallocate memory created in initialize
void CvMilitaryAI::Uninit()
{
	SAFE_DELETE_ARRAY(m_pabUsingStrategy);
	SAFE_DELETE_ARRAY(m_paiTurnStrategyAdopted);
	m_aiTempFlavors.uninit();
}

/// Reset AIStrategy status array to all false
void CvMilitaryAI::Reset()
{
	m_potentialAttackTargets.clear();
	m_exposedCities.clear();

	m_iNumLandUnits = 0;
	m_iNumRangedLandUnits = 0;
	m_iNumMobileLandUnits = 0;
	m_iNumAirUnits = 0;
	m_iNumAntiAirUnits = 0;
	m_iNumMeleeLandUnits = 0;
	m_iNumNavalUnits = 0;
	m_iNumLandUnitsInArmies = 0;
	m_iNumNavalUnitsInArmies = 0;
	m_iBarbarianCampCount = 0;
	m_iVisibleBarbarianCount = 0;
	m_iRecLandUnits = 0;
	m_iRecNavalUnits = 0;
	m_iRecExplorerUnits = 0;
	m_iTimingBuildupStartTurn = -1;
	m_eTimingBuildupTarget = NO_PLAYER;
	m_bTimingBuildupSaving = false;
	m_iTimingBuildupPausedTurns = 0;
	m_iStrikeState = STRIKE_NONE;
	m_iStrikeWarsDeclaredMark = 0;
	m_iStrikeWarsOnUsMark = 0;
	m_iStrikeCapturedMark = 0;
	m_iStrikeLostMark = 0;
	m_iStrikeCitiesAtWar = 0;
	m_iStrikeArmyAtWar = 0;
	m_iStrikeCityState = NO_PLAYER;
	m_iStrikeSiege = 0;
	m_iStrikeDeclared = 0;
	m_iArmyQualityTurn = -1;
	m_iSiegeTrainableTurn = -1;
	m_bSiegeTrainable = true;
	m_bTakeable = false;
	m_bStrikeTargetFinished = false;
	m_bStrikeHoldingReachable = false;
	m_bStrikeReady = false;
	m_bStrikeUpgradeGo = false;
	m_iStrikeNotReadyKey = 0;
	m_iArmyQualityFloor = 0;
	m_iArmyQualityRangedFloor = 0;
	m_eLandDefenseState = NO_DEFENSE_STATE;
	m_eNavalDefenseState = NO_DEFENSE_STATE;
	m_iNumberOfTimesOpsBuildSkippedOver = 0;
	m_iNumberOfTimesSettlerBuildSkippedOver = 0;

	for (int iI = 0; iI < MAX_MAJOR_CIVS; iI++)
		m_aiWarFocus[iI] = WARTYPE_UNDEFINED;

	m_iNumFreeCarriers = 0;
	//new unit counters
	m_iNumArcherLandUnits = 0;
	m_iNumSiegeLandUnits = 0;
	m_iNumSkirmisherLandUnits = 0;
	m_iNumReconLandUnits = 0;
	m_iNumBomberAirUnits = 0;
	m_iNumFighterAirUnits = 0;
	m_iNumMeleeNavalUnits = 0;
	m_iNumRangedNavalUnits = 0;
	m_iNumSubmarineNavalUnits = 0;
	m_iNumCarrierNavalUnits = 0;
	m_iNumMissileUnits = 0;
	m_iNumActiveUniqueUnits = 0;

	for(int iI = 0; iI < m_pAIStrategies->GetNumMilitaryAIStrategies(); iI++)
	{
		m_pabUsingStrategy[iI] = false;
		m_paiTurnStrategyAdopted[iI] = -1;
	}
}

///
template<typename MilitaryAI, typename Visitor>
void CvMilitaryAI::Serialize(MilitaryAI& militaryAI, Visitor& visitor)
{
	ASSERT(militaryAI.m_pAIStrategies != NULL && militaryAI.m_pAIStrategies->GetNumMilitaryAIStrategies() > 0, "Number of AIStrategies to serialize is expected to greater than 0");
	visitor(militaryAI.m_iNumberOfTimesOpsBuildSkippedOver);
	visitor(militaryAI.m_iNumberOfTimesSettlerBuildSkippedOver);

	visitor(militaryAI.m_iNumLandUnits);
	visitor(militaryAI.m_iNumRangedLandUnits);
	visitor(militaryAI.m_iNumMobileLandUnits);
	visitor(militaryAI.m_iNumMeleeLandUnits);
	visitor(militaryAI.m_iNumNavalUnits);
	visitor(militaryAI.m_iNumLandUnitsInArmies);
	visitor(militaryAI.m_iNumNavalUnitsInArmies);
	visitor(militaryAI.m_iRecNavalUnits);
	visitor(militaryAI.m_iNumAirUnits);
	visitor(militaryAI.m_iNumAntiAirUnits);
	visitor(militaryAI.m_iBarbarianCampCount);
	visitor(militaryAI.m_iVisibleBarbarianCount);
	visitor(militaryAI.m_iRecLandUnits);
	visitor(militaryAI.m_iRecExplorerUnits);
	visitor(militaryAI.m_iTimingBuildupStartTurn);
	visitor(militaryAI.m_eTimingBuildupTarget);
	visitor(militaryAI.m_bTimingBuildupSaving);
	visitor(militaryAI.m_iTimingBuildupPausedTurns);
	visitor(militaryAI.m_iStrikeState);
	visitor(militaryAI.m_iStrikeWarsDeclaredMark);
	visitor(militaryAI.m_iStrikeWarsOnUsMark);
	visitor(militaryAI.m_iStrikeCapturedMark);
	visitor(militaryAI.m_iStrikeLostMark);
	visitor(militaryAI.m_iStrikeCitiesAtWar);
	visitor(militaryAI.m_iStrikeArmyAtWar);
	visitor(militaryAI.m_iStrikeCityState);
	// AI_AB_TEST (the lead's review of 677f7a4: multiplayer sync) the strike's snapshot, computed only in UpdateStrikeState
	visitor(militaryAI.m_bTakeable);
	visitor(militaryAI.m_bStrikeTargetFinished);
	visitor(militaryAI.m_bStrikeHoldingReachable);
	visitor(militaryAI.m_bStrikeReady);
	visitor(militaryAI.m_bStrikeUpgradeGo);
	visitor(militaryAI.m_iStrikeSiege);
	visitor(militaryAI.m_iStrikeDeclared);
	visitor(militaryAI.m_iNumFreeCarriers);
	visitor(militaryAI.m_iNumArcherLandUnits);
	visitor(militaryAI.m_iNumSiegeLandUnits);
	visitor(militaryAI.m_iNumSkirmisherLandUnits);
	visitor(militaryAI.m_iNumReconLandUnits);
	visitor(militaryAI.m_iNumBomberAirUnits);
	visitor(militaryAI.m_iNumFighterAirUnits);
	visitor(militaryAI.m_iNumMeleeNavalUnits);
	visitor(militaryAI.m_iNumRangedNavalUnits);
	visitor(militaryAI.m_iNumSubmarineNavalUnits);
	visitor(militaryAI.m_iNumCarrierNavalUnits);
	visitor(militaryAI.m_iNumMissileUnits);
	visitor(militaryAI.m_iNumActiveUniqueUnits);

	visitor(militaryAI.m_eLandDefenseState);
	visitor(militaryAI.m_eNavalDefenseState);
	visitor(militaryAI.m_potentialAttackTargets);
	visitor(militaryAI.m_exposedCities);

	const int iNumMilitaryAIStrategies = militaryAI.m_pAIStrategies->GetNumMilitaryAIStrategies();
	visitor(MakeConstSpan(militaryAI.m_pabUsingStrategy, iNumMilitaryAIStrategies));
	visitor(MakeConstSpan(militaryAI.m_paiTurnStrategyAdopted, iNumMilitaryAIStrategies));
	visitor(militaryAI.m_aiWarFocus);
}

/// Serialization read
void CvMilitaryAI::Read(FDataStream& kStream)
{
	CvStreamLoadVisitor serialVisitor(kStream);
	Serialize(*this, serialVisitor);
}

/// Serialization write
void CvMilitaryAI::Write(FDataStream& kStream) const
{
	CvStreamSaveVisitor serialVisitor(kStream);
	Serialize(*this, serialVisitor);
}

FDataStream& operator>>(FDataStream& stream, CvMilitaryAI& militaryAI)
{
	militaryAI.Read(stream);
	return stream;
}
FDataStream& operator<<(FDataStream& stream, const CvMilitaryAI& militaryAI)
{
	militaryAI.Write(stream);
	return stream;
}

FDataStream& operator<<(FDataStream& saveTo, const DefenseState& readFrom)
{
	int v = static_cast<int>(readFrom);
	saveTo << v;
	return saveTo;
}

FDataStream& operator>>(FDataStream& loadFrom, DefenseState& writeTo)
{
	int v = 0;
	loadFrom >> v;
	writeTo = static_cast<DefenseState>(v);
	return loadFrom;
}

/// Returns the Player object the Strategies are associated with
CvPlayer* CvMilitaryAI::GetPlayer()
{
	return m_pPlayer;
}

/// Returns AIStrategies object stored in this class
CvMilitaryAIStrategyXMLEntries* CvMilitaryAI::GetMilitaryAIStrategies()
{
	return m_pAIStrategies;
}

/// Returns whether or not a player has adopted this Strategy
bool CvMilitaryAI::IsUsingStrategy(MilitaryAIStrategyTypes eStrategy)
{
	if (eStrategy == NO_MILITARYAISTRATEGY)
		return false;

	return m_pabUsingStrategy[(int) eStrategy];
}

/// Signifies that eStrategy is now being utilized by this AI Player
void CvMilitaryAI::SetUsingStrategy(MilitaryAIStrategyTypes eStrategy, bool bValue)
{
	if(m_pabUsingStrategy[eStrategy] != bValue)
	{
		m_pabUsingStrategy[eStrategy] = bValue;

		if(bValue)
		{
			SetTurnStrategyAdopted(eStrategy, GC.getGame().getGameTurn());
		}
		else
		{
			SetTurnStrategyAdopted(eStrategy, -1);
		}

		LogStrategy(eStrategy, bValue);
	}
}

/// Returns the turn on which a Strategy was adopted (-1 if it hasn't been)
int CvMilitaryAI::GetTurnStrategyAdopted(MilitaryAIStrategyTypes eStrategy)
{
	return m_paiTurnStrategyAdopted[(int) eStrategy];
}

/// Sets the turn number eStrategy was most recently adopted
void CvMilitaryAI::SetTurnStrategyAdopted(MilitaryAIStrategyTypes eStrategy, int iValue)
{
	m_paiTurnStrategyAdopted[(int) eStrategy] = iValue;
}

/// Process through all the military activities for a player's turn
void CvMilitaryAI::DoTurn()
{
	ScanForBarbarians();
	UpdateBaseData();
	UpdateDefenseState();
	UpdateMilitaryStrategies();
	UpdateWarType();

	//do this also for humans because AI relies on the data!
	UpdateAttackTargets();

	if(!m_pPlayer->isHuman(ISHUMAN_AI_UNITS))
	{
		UpdateOperations();
		if (!m_pPlayer->isHuman(ISHUMAN_AI_ECONOMY))
		{
			MakeEmergencyPurchases();
		}
		if (IsTimingBuildupPlayer())
		{
			UpdateStrikeState();
			DoTimingBuildup();
		}
		DisbandObsoleteUnits();
	}

	LogMilitaryStatus();

	if(IsBuildingArmy(ARMY_TYPE_ANY))
		LogAvailableForces();
}

/// Ask to send a nuke at an enemy
bool CvMilitaryAI::RequestNukeAttack(PlayerTypes eEnemy)
{
	if(m_pPlayer->getNumNukeUnits() > 0)
		return m_pPlayer->addAIOperation(AI_OPERATION_NUKE_ATTACK, 0, eEnemy)!=NULL;

	return false;
}


/// harass the enemy
bool CvMilitaryAI::RequestPillageAttack(PlayerTypes eEnemy)
{
	if (!m_pPlayer->getFirstAIOperationOfType(AI_OPERATION_PILLAGE_ENEMY))
		//don't try to build additional units, only do this if we have enough at hand
		return m_pPlayer->addAIOperation(AI_OPERATION_PILLAGE_ENEMY, 0, eEnemy) != NULL;

	return false;
}

/// Send an army to force concessions
bool CvMilitaryAI::RequestBullyingOperation(PlayerTypes eEnemy)
{
	// AI_AB_TEST (the strike's ownership, gate 4; the lead's review): a bullying city attack is an attack too
	if (IsStrikeOwningOperations() && !IsStrikeOperationTarget(eEnemy))
		return false;
	if (GET_PLAYER(eEnemy).isBarbarian())
		return false;

	// If we already have an operation on the way, don't send another one
	if (m_pPlayer->HasAnyOffensiveOperationsAgainstPlayer(eEnemy))
		return false;

	CvCity* pTargetCity = GET_PLAYER(eEnemy).isMajorCiv() ? m_pPlayer->GetClosestCityToUsByPlots(eEnemy) : GET_PLAYER(eEnemy).getCapitalCity();
	if (!pTargetCity)
		return false;

	//don't venture too far
	int iDistanceTurns = m_pPlayer->GetCityDistancePathLength(pTargetCity->plot());
	if (iDistanceTurns > 23)
		return false;

	CvCity* pMusterCity = m_pPlayer->GetClosestCityByPathLength(pTargetCity->plot());
	if (!pMusterCity)
		return false;

	//if the target is very close assume we can embark or don't even need to
	AIOperationTypes opType = pMusterCity->HasSharedAreaWith(pTargetCity,true,false) ? AI_OPERATION_CITY_ATTACK_LAND : AI_OPERATION_CITY_ATTACK_NAVAL;

	//don't try to build additional units, only do this if we have enough at hand
	return m_pPlayer->addAIOperation(opType, 0, eEnemy, pTargetCity, pMusterCity) != NULL;
}

/// Spend money to quickly add a unit to a city
CvUnit* CvMilitaryAI::BuyEmergencyUnit(UnitAITypes eUnitType, CvCity* pCity)
{
	// No units in puppet cities or automated cities except for Venice!
	if (CityStrategyAIHelpers::IsTestCityStrategy_IsPuppetAndAnnexable(pCity) || pCity->isHumanAutomated())
	{
		return NULL;
	}

	// Get best unit with this AI type
	UnitTypes eType = pCity->GetCityStrategyAI()->GetUnitProductionAI()->RecommendUnit(eUnitType, true);
	CvUnit* pUnit = NULL;
	bool bGold = false;

	if(eType != NO_UNIT)
	{
		CvUnitEntry* pUnitInfo = GC.GetGameUnits()->GetEntry(eType);
		if(pUnitInfo && pUnitInfo->GetUnitAIType(eUnitType))
		{
			//make sure we want to spend our gold on this
			int iGoldCost = pCity->GetPurchaseCost(eType);
			int iPriority = /*500*/ GD_INT_GET(AI_GOLD_PRIORITY_UNIT);
			int iTempWeight = 100;
			iTempWeight = pCity->GetCityStrategyAI()->GetUnitProductionAI()->CheckUnitBuildSanity(eType, false, iTempWeight, true);
			if (iTempWeight > 0)
			{
				if (m_pPlayer->GetEconomicAI()->CanWithdrawMoneyForPurchase(PURCHASE_TYPE_UNIT, iGoldCost, iPriority))
					//try with gold first
					pUnit = pCity->PurchaseUnit(eType, YIELD_GOLD);

				if (pUnit)
					bGold = true;
				else
				{
					//try again with Faith (only because this is any emergency)
					pUnit = pCity->PurchaseUnit(eType, YIELD_FAITH);
				}
			}
		}
	}

	// AI unit promotions have already been processed, so we need to do it explicitly here
	if (pUnit && !pUnit->isHuman(ISHUMAN_AI_UNIT_PROMOTIONS))
		pUnit->AI_promote();

	if (pUnit)
	{
		CvString szMsg;
		szMsg.Format("Emergency %s Unit Purchased: %s, ", bGold?"Gold":"Faith", pUnit->getUnitInfo().GetDescription());
		szMsg += pCity->getName();
		m_pPlayer->GetTacticalAI()->LogTacticalMessage(szMsg);
	}
	return pUnit;
}

/// Spend money to quickly add a defensive building to a city
bool CvMilitaryAI::BuyEmergencyBuilding(CvCity* pCity)
{
	std::vector<int> allBuildingCount = m_pPlayer->GetTotalBuildingCount();
	// Loop through adding the available buildings
	for(int iBldgLoop = 0; iBldgLoop < GC.GetGameBuildings()->GetNumBuildings(); iBldgLoop++)
	{
		const BuildingTypes eBldg = static_cast<BuildingTypes>(iBldgLoop);
		CvBuildingEntry* pkBuildingInfo = GC.GetGameBuildings()->GetEntry(eBldg);
		if(pkBuildingInfo)
		{
			// Make sure this building can be built now
			if(pCity->canConstruct(eBldg,allBuildingCount) && pkBuildingInfo->GetDefenseModifier() > 0)
			{
				if(pCity->IsCanPurchase(/*bTestPurchaseCost*/ true, /*bTestTrainable*/ true, NO_UNIT, eBldg, NO_PROJECT, YIELD_GOLD))
				{
					int iGoldCost = pCity->GetPurchaseCost(eBldg);
					int iPriority = /*250*/ GD_INT_GET(AI_GOLD_PRIORITY_DEFENSIVE_BUILDING);
					if(m_pPlayer->GetEconomicAI()->CanWithdrawMoneyForPurchase(PURCHASE_TYPE_BUILDING, iGoldCost, iPriority))
					{
						m_pPlayer->GetTreasury()->LogExpenditure((CvString)pkBuildingInfo->GetText(), iGoldCost, 8);
						m_pPlayer->GetTreasury()->ChangeGold(-iGoldCost);
						int iResult = pCity->CreateBuilding(eBldg);

						DEBUG_VARIABLE(iResult);
						ASSERT(iResult != -1, "Unable to create building");

						CvString szMsg;
						szMsg.Format("Emergency Building Purchased: %s, ", pkBuildingInfo->GetDescription());
						szMsg += pCity->getName();
						m_pPlayer->GetTacticalAI()->LogTacticalMessage(szMsg);
					}
				}
			}
		}
	}
	return false;
}

// check if the army path makes sense
bool MilitaryAIHelpers::ArmyPathIsGood(const SPath & path, PlayerTypes eAttacker, PlayerTypes eIntendedEnemy, int iThresholdForDiscard)
{
	//define short paths as safe (start and dest plot are included in count)
	if (path.vPlots.size() < 4)
		return true;

	bool bWaterPath = path.get(1)->isWater();
	int iThirdPartyPlots = 0;
	int iWrongCityPlots = 0;

	//ignore beginning and end of path!
	for (size_t i = 3; i < path.vPlots.size() - 3; i++)
	{
		CvPlot* pPlot = GC.getMap().plot(path.vPlots[i].x,path.vPlots[i].y);
		int iCityDistance = GC.getGame().GetClosestCityDistanceInPlots(pPlot);

		//if we're far from a city it's ok
		if (iCityDistance > 3 && !pPlot->isOwned())
			continue;

		//else check whose city it is
		CvCity* pCity = GC.getGame().GetClosestCityByPlots(pPlot);
		if (!pCity)
			continue;

		//passing another city of our enemy? then we should attack that one first
		if (pCity->getOwner() == eIntendedEnemy)
		{
			if (path.get(-1)->getOwningCityID() != pCity->GetID())
			{
				if (!bWaterPath || pCity->isCoastal())
					iWrongCityPlots++;
			}
		}
		//maybe we have a better muster plot?
		else if (pCity->getOwner() == eAttacker)
		{
			if (path.get(0)->getOwningCityID() != pCity->GetID())
			{
				if (!bWaterPath || pCity->isCoastal())
					iWrongCityPlots++;
			}
		}
		//passing through a different warzone? not good
		else if (GET_PLAYER(eAttacker).IsAtWarWith(pCity->getOwner()))
			iThirdPartyPlots++;
		//the trade path may lead through third-party territory
		else if (pPlot->isOwned() && pPlot->getOwner()!=eIntendedEnemy && !pPlot->IsFriendlyTerritory(eAttacker))
			iThirdPartyPlots++;
	}

	//the trade paths often lead through third party territory which an army may not be able to pass
	//if we need open borders and there is no easy detour, exclude this path
	return (iThirdPartyPlots < iThresholdForDiscard) && (iWrongCityPlots < iThresholdForDiscard);
}

bool CvMilitaryAI::IsPossibleAttackTarget(const CvCity* pCity, ArmyType eArmyType) const
{
	if (!pCity || pCity->getOwner()==m_pPlayer->GetID())
		return false;

	//cities may be listed multiple times!
	for (size_t i = 0; i < m_potentialAttackTargets.size(); i++)
		if (m_potentialAttackTargets[i].GetTargetPlot() == pCity->plot())
			if (m_potentialAttackTargets[i].m_armyType == eArmyType || eArmyType == ARMY_TYPE_ANY)
				return true;

	return false;
}

bool CvMilitaryAI::IsPossibleMusterCity(const CvCity* pCity, ArmyType eArmyType) const
{
	if (!pCity || pCity->getOwner() != m_pPlayer->GetID())
		return false;

	//cities may be listed multiple times!
	for (size_t i = 0; i < m_potentialAttackTargets.size(); i++)
		if (m_potentialAttackTargets[i].GetMusterPlot() == pCity->plot())
			if (m_potentialAttackTargets[i].m_armyType == eArmyType || eArmyType == ARMY_TYPE_ANY)
				return true;

	return false;
}

bool CvMilitaryAI::IsPreferredAttackTarget(const CvCity* pCity) const
{
	if (!pCity || pCity->getOwner()==m_pPlayer->GetID())
		return false;

	//cities may be listed multiple times!
	for (size_t i = 0; i < m_potentialAttackTargets.size(); i++)
		if (m_potentialAttackTargets[i].GetTargetPlot() == pCity->plot() && m_potentialAttackTargets[i].IsPreferred())
			return true;

	return false;
}

bool CvMilitaryAI::IsExposedToEnemy(const CvCity * pCity, PlayerTypes eOtherPlayer, ArmyType eArmyType) const
{
	//minors don't really explore, so they don't know what's exposed ... just assume all their cities are
	if (m_pPlayer->isMinorCiv())
		return true;

	//cities may be listed multiple times!
	for (size_t i = 0; i < m_exposedCities.size(); i++)
		if (eOtherPlayer==NO_PLAYER || m_exposedCities[i].GetAttacker() == eOtherPlayer)
			if (pCity == NULL || m_exposedCities[i].GetTargetPlot() == pCity->plot())
				if (m_exposedCities[i].m_armyType == eArmyType || eArmyType == ARMY_TYPE_ANY)
					return true;

	return false;
}

bool CvMilitaryAI::HavePreferredAttackTarget(PlayerTypes eEnemy) const
{
	//cities may be listed multiple times!
	for (size_t i = 0; i < m_potentialAttackTargets.size(); i++)
		if (m_potentialAttackTargets[i].GetTargetPlot()->getOwner() == eEnemy && m_potentialAttackTargets[i].IsPreferred())
			return true;

	return false;
}

bool CvMilitaryAI::HavePossibleAttackTarget(PlayerTypes eEnemy) const
{
	//cities may be listed multiple times!
	for (size_t i = 0; i < m_potentialAttackTargets.size(); i++)
		if (m_potentialAttackTargets[i].GetTargetPlot()->getOwner() == eEnemy)
			return true;

	return false;
}

/// AI_AB_TEST (the strike's reach; the lead, bullygreedy24 17A1A659: the Aztecs' strike chose Milan, 9 tiles away, which was
/// never in their attack targets, and its city attack failed every turn for 90 turns): one of this player's cities is a land
/// attack target (an army path CP scores; the strike's army is a land army). RequestCityAttack needs it
bool CvMilitaryAI::HaveStrikeLandAttackTarget(PlayerTypes eEnemy) const
{
	if (eEnemy == NO_PLAYER)
		return false;
	for (size_t i = 0; i < m_potentialAttackTargets.size(); i++)
		if (m_potentialAttackTargets[i].m_armyType == ARMY_TYPE_LAND && m_potentialAttackTargets[i].GetTargetPlot()->getOwner() == eEnemy)
			return true;
	return false;
}

//override the diplo AI method so that it returns a sensible result for barbarians
bool CvMilitaryAI::IsPlayerValid(PlayerTypes eOtherPlayer) const
{
	if (eOtherPlayer == BARBARIAN_PLAYER)
		return true;

	//ignore our vassals
	if (m_pPlayer->GetDiplomacyAI()->IsMaster(eOtherPlayer))
		return false;

	//minors don't do sneak attacks
	if (m_pPlayer->isMinorCiv() && m_pPlayer->IsAtPeaceWith(eOtherPlayer))
		return false;

	return m_pPlayer->GetDiplomacyAI()->IsPlayerValid(eOtherPlayer);
}

size_t CvMilitaryAI::UpdateAttackTargets()
{
	m_potentialAttackTargets.clear();
	m_exposedCities.clear();

	vector<OptionWithScore<CvAttackTarget>> vAttackOptions;
	vector<OptionWithScore<CvAttackTarget>> vDefenseOptions;

	int iCityLoop = 0;
	for (CvCity* pMusterCity = m_pPlayer->firstCity(&iCityLoop); pMusterCity != NULL; pMusterCity = m_pPlayer->nextCity(&iCityLoop))
	{
		map<int, SPath> landpaths = GetArmyPathsFromCity(pMusterCity, false);
		map<int, SPath> waterpaths = GetArmyPathsFromCity(pMusterCity, true);

		//first filter land targets
		for (map<int, SPath>::iterator it = landpaths.begin(); it != landpaths.end(); ++it)
		{
			PlayerTypes eOtherPlayer = it->second.get(-1)->getOwner();
			if (!IsPlayerValid(eOtherPlayer))
				continue;

			CvAttackTarget target;
			target.SetWaypoints(it->second);
			MilitaryAIHelpers::SetBestTargetApproach(target, m_pPlayer->GetID());

			//keep it if it seems we can move an army there
			if (target.m_iApproachScore > 0 && MilitaryAIHelpers::ArmyPathIsGood(it->second, m_pPlayer->GetID(), eOtherPlayer, 3))
				vAttackOptions.push_back(OptionWithScore<CvAttackTarget>(target, ScoreAttackTarget(target)));

			//and now the other way around - this is not 100% accurate because we used the wrong terrain ownership to generate the path!
			it->second.invert();

			CvAttackTarget reverseTarget;
			reverseTarget.SetWaypoints(it->second);
			MilitaryAIHelpers::SetBestTargetApproach(reverseTarget, eOtherPlayer);

			//use a more liberal check here, never know what the enemy might do ... better safe than sorry!
			//AI_AB_TEST: CP checks our approach to their city (target) instead of their approach to ours
			//(reverseTarget), so a city is only counted as exposed when we can easily attack them.
			if ((m_pPlayer->UsesNewAI() ? reverseTarget.m_iApproachScore : target.m_iApproachScore) > 0 && MilitaryAIHelpers::ArmyPathIsGood(it->second, eOtherPlayer, m_pPlayer->GetID(), 6))
			{
				//do not try any advanced scoring, just use a basic approach/distance score
				int iScore = (reverseTarget.m_iApproachScore * 10) / sqrti(it->second.length());
				vDefenseOptions.push_back(OptionWithScore<CvAttackTarget>(reverseTarget, iScore));
			}
		}

		//then water targets
		for (map<int, SPath>::iterator it = waterpaths.begin(); it != waterpaths.end(); ++it)
		{
			PlayerTypes eOtherPlayer = it->second.get(-1)->getOwner();
			if (!IsPlayerValid(eOtherPlayer))
				continue;

			CvAttackTarget target;
			target.SetWaypoints(it->second);
			MilitaryAIHelpers::SetBestTargetApproach(target, m_pPlayer->GetID());

			//keep it if it seems we can move an army there
			if (target.m_iApproachScore > 0 && MilitaryAIHelpers::ArmyPathIsGood(it->second, m_pPlayer->GetID(), eOtherPlayer, 3))
				vAttackOptions.push_back(OptionWithScore<CvAttackTarget>(target, ScoreAttackTarget(target)));

			//and now the other way around - this is not 100% accurate because we used the wrong terrain ownership to generate the path!
			it->second.invert();

			CvAttackTarget reverseTarget;
			reverseTarget.SetWaypoints(it->second);
			MilitaryAIHelpers::SetBestTargetApproach(reverseTarget, eOtherPlayer);

			//use a more liberal check here, never know what the enemy might do ... better safe than sorry!
			//AI_AB_TEST: CP checks our approach to their city (target) instead of their approach to ours
			//(reverseTarget), so a city is only counted as exposed when we can easily attack them.
			if ((m_pPlayer->UsesNewAI() ? reverseTarget.m_iApproachScore : target.m_iApproachScore) > 0 && MilitaryAIHelpers::ArmyPathIsGood(it->second, eOtherPlayer, m_pPlayer->GetID(), 6))
			{
				//do not try any advanced scoring, just use a basic approach/distance score
				int iScore = (reverseTarget.m_iApproachScore * 10) / sqrti(it->second.length());
				vDefenseOptions.push_back(OptionWithScore<CvAttackTarget>(reverseTarget, iScore));
			}
		}
	}

	if (!vAttackOptions.empty())
	{
		//some might be present twice but we sort that out below
		std::stable_sort(vAttackOptions.begin(), vAttackOptions.end());

		int iBestScore = vAttackOptions.front().score;

		//AI_AB_TEST: CP measures "preferred" against the best target among all players' cities, including civs we're at
		//peace with, so a well-defended enemy may never count as preferred (and careful attacks skip it). The new AI
		//compares each target against the best city of the same owner.
		const bool bPerOwnerPreferred = m_pPlayer->UsesNewAI();
		std::map<int, int> bestScoreByOwner;
		if (bPerOwnerPreferred)
		{
			for (size_t i = 0; i < vAttackOptions.size(); i++)
			{
				int iOwner = vAttackOptions[i].option.GetTargetPlot()->getOwner();
				bestScoreByOwner[iOwner] = max(bestScoreByOwner[iOwner], vAttackOptions[i].score);
			}
		}

		for (size_t i = 0; i < vAttackOptions.size(); i++)
		{
			CvAttackTarget target = vAttackOptions[i].option;
			int iScore = vAttackOptions[i].score;
			if (iScore == 0)
				continue;

			//mark the best targets
			int iReferenceScore = bPerOwnerPreferred ? bestScoreByOwner[target.GetTargetPlot()->getOwner()] : iBestScore;
			target.m_bPreferred = (iScore > iReferenceScore / 3);

			//don't target a city twice with the same army type, only keep the best approach
			if (IsPossibleAttackTarget(target.GetTargetPlot()->getPlotCity(),target.m_armyType))
				continue;

			m_potentialAttackTargets.push_back(target);

			if (GC.getLogging() && GC.getAILogging())
			{
				CvCity* pMuster = target.GetMusterPlot()->getPlotCity();
				CvCity* pTarget = target.GetTargetPlot()->getPlotCity();
				CvString msg = CvString::format("%03d, %s, %sattack target: %s, muster: %s, army type: %s, score: %d",
					GC.getGame().getElapsedGameTurns(), m_pPlayer->getCivilizationShortDescription(), target.IsPreferred() ? "preferred " : "",
					pTarget->getNameNoSpace().c_str(), pMuster->getNameNoSpace().c_str(), ArmyTypeToString(target.m_armyType), iScore);
				CvString playerName = GetPlayer()->getCivilizationShortDescription();
				FILogFile* pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);
				if (pLog)
					pLog->Msg(msg.c_str());
			}
		}
	}

	if (!vDefenseOptions.empty())
	{
		std::stable_sort(vDefenseOptions.begin(), vDefenseOptions.end());

		int iBestScore = vDefenseOptions.front().score;
		for (size_t i = 0; i < vDefenseOptions.size(); i++)
		{
			CvAttackTarget target = vDefenseOptions[i].option;
			int iScore = vDefenseOptions[i].score;

			if (iScore > iBestScore / 3)
			{
				CvCity* pMuster = target.GetMusterPlot()->getPlotCity();
				CvCity* pTarget = target.GetTargetPlot()->getPlotCity();

				//store it multiple times for multiple potential enemies
				if (!IsExposedToEnemy(pTarget, pMuster->getOwner()))
				{
					m_exposedCities.push_back(target);

					if (GC.getLogging() && GC.getAILogging())
					{
						CvString msg = CvString::format("%03d, %s, exposed city: %s, muster: %s, army type: %s, score: %d",
							GC.getGame().getElapsedGameTurns(), m_pPlayer->getCivilizationShortDescription(),
							pTarget->getNameNoSpace().c_str(), pMuster->getNameNoSpace().c_str(), ArmyTypeToString(target.m_armyType), iScore);
						CvString playerName = GetPlayer()->getCivilizationShortDescription();
						FILogFile* pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);
						if (pLog)
							pLog->Msg(msg.c_str());
					}
				}
			}
		}
	}

	return m_potentialAttackTargets.size();
}

map<int, SPath> CvMilitaryAI::GetArmyPathsFromCity(CvCity* pMusterCity, bool bWater)
{
	vector<CvPlot*> vPotentialTargets;
	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);

		//skip teammates and allied city states
		if (kLoopPlayer.getTeam() == m_pPlayer->getTeam())
			continue;
		//AI_AB_TEST (the user: no flip flopping): not the strike's held target, even allied (178B8F13: the Huns allied their own
		//target Hong Kong through its quests; it stays the target and must stay an attack target)
		if (kLoopPlayer.GetMinorCivAI()->GetAlly() == m_pPlayer->GetID() && (PlayerTypes)iPlayer != GetStrikeCityStateTarget())
			continue;

		//don't need to filter for coastal cities, pathfinder will sort it out
		int iCity = 0;
		for (CvCity* pDestCity = kLoopPlayer.firstCity(&iCity); pDestCity != NULL; pDestCity = kLoopPlayer.nextCity(&iCity))
			if (pDestCity->plot()->isCity() && pDestCity->plot()->isAdjacentRevealed(m_pPlayer->getTeam())) //idiot proofing
				vPotentialTargets.push_back(pDestCity->plot());
	}

	//re-use trade route distance to get an era-appropriate max distance
	int iMaxNormDist = m_pPlayer->GetTrade()->GetTradeRouteRange(bWater?DOMAIN_SEA:DOMAIN_LAND, pMusterCity);
	SPathFinderUserData data(m_pPlayer->GetID(), bWater?PT_ARMY_WATER:PT_ARMY_LAND, NO_PLAYER, INT_MAX);
	data.iMaxNormalizedDistance = iMaxNormDist;
	data.iFlags |= CvUnit::MOVEFLAG_IGNORE_RIGHT_OF_PASSAGE;
	data.iFlags |= CvUnit::MOVEFLAG_IGNORE_ENEMIES;

	//get all paths, ie map<index of dest's city plot, path to that city>
	return GC.GetStepFinder().GetMultiplePaths(pMusterCity->plot(), vPotentialTargets, data);
}

bool CvMilitaryAI::RequestCityAttack(PlayerTypes eIntendedTarget, int iNumUnitsWillingToBuild, bool bCareful, const CvCity* pPreferredCity, bool* pbSiegeFed)
{
	if (pbSiegeFed)
		*pbSiegeFed = false;
	// AI_AB_TEST (the strike's ownership, gate 4: the army's operations; the lead, bullygreedy23 169CA418): while the strike
	// is on (PREP, BUILDUP, or WAR before the target's capital is taken) its army attacks only the strike's target, or a player already at
	// war with us. CP's own attacks on another player (sneak operations on its conquest target, coop wars, CS quests) don't
	// start; in 169CA418 they gathered the army at Marrakech while the strike was at war with Ur
	if (IsStrikeOwningOperations() && !IsStrikeOperationTarget(eIntendedTarget))
	{
		if (GC.getLogging() && GC.getAILogging() && eIntendedTarget != NO_PLAYER)
			m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: city attack on %s refused (gate 4: the strike's army attacks only its target)",
				GET_PLAYER(eIntendedTarget).getCivilizationShortDescription()));
		return false;
	}

	//AI_AB_TEST (the strike's objective, the target's capital; the user, the trio game): with a preferred city, only that city
	//while it is an attack target (pass 0); otherwise the usual best-scored city of the target (pass 1)
	const CvPlot* pPreferredPlot = pPreferredCity ? pPreferredCity->plot() : NULL;
	bool bPreferredListed = false;
	for (int iPass = pPreferredPlot ? 0 : 1; iPass < 2 && !bPreferredListed; iPass++)
	{
	//note that a given target might be repeated with different muster points / army types
	for (size_t i = 0; i < m_potentialAttackTargets.size(); i++)
	{
		//ignore bad targets!
		if (bCareful && !m_potentialAttackTargets[i].IsPreferred())
			continue;

		CvPlot* pMusterPlot = m_potentialAttackTargets[i].GetMusterPlot();
		CvPlot* pTargetPlot = m_potentialAttackTargets[i].GetTargetPlot();
		// (the lead's review of 227f759) pass 0 is the capital's land entries only; it counts as handled only when its
		// operation starts (returned below) or SiegeFeed skips it (under siege). A failed or sea-only capital falls through to
		// the target's other cities in pass 1
		if (iPass == 0 && (pTargetPlot != pPreferredPlot || m_potentialAttackTargets[i].m_armyType != ARMY_TYPE_LAND))
			continue;
		PlayerTypes eTargetPlayer = pTargetPlot->getOwner();
		if (eTargetPlayer != eIntendedTarget)
			continue;

		//AI_AB_TEST (candidate, flag SiegeFeed): no new attack operation against a city we're already besieging. A test aggressor's
		//new attack on the city under siege recruited the units SiegeClose had brought up and held them in a gathering army beside the
		//walls for 10 turns, while 1-3 free units attacked (0D9905E5, Ribe). The tactical AI's reinforcement brings units in reach.
		if (m_pPlayer->UsesCandidateAI(FIX_SIEGE_FEED) && eTargetPlayer != NO_PLAYER && pTargetPlot->isCity() &&
			GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eTargetPlayer).getTeam()))
		{
			CvTacticalDominanceZone* pSiegeZone = m_pPlayer->GetTacticalAI()->GetTacticalAnalysisMap()->GetZoneByCity(pTargetPlot->getPlotCity(), false);
			// (the strike's target, 18BE1E3E: La Venta "under siege" T119-151 by 3-32 units with no ranged strength in the zone and
			// 0 expected damage; idle melee units aren't a siege) for the strike's target, a siege needs our ranged units there
			const bool bSiegeHasRanged = !IsStrikeNoRetreat(eTargetPlayer) || (pSiegeZone && pSiegeZone->GetFriendlyRangedStrength() > 0);
			if (pSiegeZone && pSiegeZone->GetOverallDominanceFlag() == TACTICAL_DOMINANCE_FRIENDLY && pSiegeZone->GetTotalFriendlyUnitCount() >= 3 && bSiegeHasRanged)
			{
				if (pbSiegeFed)
					*pbSiegeFed = true;
				if (GC.getLogging() && GC.getAILogging())
				{
					CvString strSiegePlayerName = m_pPlayer->getCivilizationShortDescription();
					FILogFile* pLog = LOGFILEMGR.GetLog(GetLogFileName(strSiegePlayerName), FILogFile::kDontTimeStamp);
					pLog->Msg(CvString::format("%d, %s, SiegeFeed: %s is under siege (%d units in the zone), no new army for it\n",
						GC.getGame().getElapsedGameTurns(), m_pPlayer->getCivilizationShortDescription(),
						pTargetPlot->getPlotCity()->getNameNoSpace().c_str(), pSiegeZone->GetTotalFriendlyUnitCount()).c_str());
				}
				if (iPass == 0)
					bPreferredListed = true;	// the capital is under siege already: no attack elsewhere
				continue;
			}
		}

		AIOperationTypes opType = AI_OPERATION_TYPE_UNKNOWN;
		switch (m_potentialAttackTargets[i].m_armyType)
		{
		case ARMY_TYPE_LAND:
			opType = AI_OPERATION_CITY_ATTACK_LAND;
			break;
		case ARMY_TYPE_NAVAL:
			opType = AI_OPERATION_CITY_ATTACK_NAVAL;
			break;
		case ARMY_TYPE_COMBINED:
			opType = AI_OPERATION_CITY_ATTACK_COMBINED;
			break;
		default:
			continue;
		}

		//don't duplicate operations
		CvAIOperation* pCurrentOp = m_pPlayer->getFirstAIOperationOfType(opType, eTargetPlayer, pTargetPlot);
		if (bCareful && pCurrentOp != NULL && pCurrentOp->GetArmy(0))
		{
			//wait until the previous army has at least left our territory, don't commit all our units to one target
			CvPlot* pCOM = pCurrentOp->GetArmy(0)->GetCenterOfMass();
			if (!pCOM || pCOM->getOwner() == m_pPlayer->GetID())
				continue;
		}

		//if we're being careless, just use whatever units we have and do not wait for new ones
		//AI_AB_TEST (candidate, flag TimingBuildup): a timing attacker in its window or attack phase may leave units to be
		//built, as a careful request can: the operation forms and its own build requests fill the slots (a test aggressor
		//made 302 requests that all failed "for lack of units", since every required slot needed a free unit up front)
		const TimingBuildupPhase eTimingPhase = GetTimingBuildupPhase();
		// (and the strike's war on its target: its attack forms with slots still to fill, trial25 171787CD's "lack of units")
		const bool bTimingAttack = eTimingPhase == TIMING_BUILDUP_WINDOW || eTimingPhase == TIMING_BUILDUP_ATTACK || IsStrikeNoRetreat(eTargetPlayer);
		int iMaxMissing = (bCareful || bTimingAttack) ? iNumUnitsWillingToBuild : 0;
		//AI_AB_TEST (candidate, flag SecondWave): at war with a major, the next attack may form with up to 3 required slots to be
		//built. The siege formation has 6 required slots and the attacks after the first started with 3 units and ended NoUnits
		//the same turn (6 test games), so a damaged city never saw a second wave. SiegeArmy's at-war bar and SiegeRoll decide
		//when it marches.
		if (m_pPlayer->UsesCandidateAI(FIX_SECOND_WAVE) && eTargetPlayer != NO_PLAYER && eTargetPlayer < MAX_MAJOR_CIVS &&
			GET_PLAYER(eTargetPlayer).isMajorCiv() && GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eTargetPlayer).getTeam()))
			iMaxMissing = max(iMaxMissing, max(iNumUnitsWillingToBuild, 3));
		if (m_pPlayer->addAIOperation(opType, iMaxMissing, eTargetPlayer, pTargetPlot->getPlotCity(), pMusterPlot->getPlotCity()) != NULL)
			return true;
		else
		{
			if(GC.getLogging() && GC.getAILogging() && pTargetPlot->getPlotCity() && pMusterPlot->getPlotCity())
			{
				CvString playerName = GetPlayer()->getCivilizationShortDescription();
				FILogFile* pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);
				CvString msg = CvString::format( "%d, %s, requested attack on, %s, from, %s, not possible for lack of units\n", 
					GC.getGame().getElapsedGameTurns(), m_pPlayer->getCivilizationShortDescription(), pTargetPlot->getPlotCity()->getName().c_str(), pMusterPlot->getPlotCity()->getName().c_str());
				pLog->Msg( msg.c_str() );
			}
		}
	}
	}

	return false;
}

void MilitaryAIHelpers::SetBestTargetApproach(CvAttackTarget& target, PlayerTypes ePlayer)
{
	//default
	target.m_iApproachScore = 0;
	target.m_armyType = ARMY_TYPE_ANY;

	//compare
	int iLandScore = EvaluateTargetApproach(target,ePlayer,ARMY_TYPE_LAND);
	int iNavalScore = EvaluateTargetApproach(target,ePlayer,ARMY_TYPE_NAVAL);
	int iCombinedScore = EvaluateTargetApproach(target,ePlayer,ARMY_TYPE_COMBINED);

	//we want a minimum score of 30!
	if (iLandScore >= iNavalScore && iLandScore >= iCombinedScore && iLandScore>30)
	{
		target.m_armyType = ARMY_TYPE_LAND;
		target.m_iApproachScore = iLandScore;
	}
	else if (iNavalScore >= iLandScore && iNavalScore >= iCombinedScore && iNavalScore>30)
	{
		target.m_armyType = ARMY_TYPE_NAVAL;
		target.m_iApproachScore = iNavalScore;
	}
	else if (iCombinedScore >= iNavalScore && iCombinedScore >= iLandScore && iCombinedScore>30)
	{
		target.m_armyType = ARMY_TYPE_COMBINED;
		target.m_iApproachScore = iCombinedScore;
	}
}

int CvMilitaryAI::ScoreAttackTarget(const CvAttackTarget& target)
{
	CvCity* pTargetCity = target.GetTargetPlot()->getPlotCity();

	float fDistWeightInterpolated = 1;
	if(target.m_armyType==ARMY_TYPE_LAND)
	{
		// interpolate linearly between a low and a high distance
		float fDistanceLow = 8;
		float fWeightLow = 10;
		float fDistanceHigh = 24;
		float fWeightHigh = 1;

		float fSlope = (fWeightHigh-fWeightLow) / (fDistanceHigh-fDistanceLow);
		fDistWeightInterpolated = (target.GetPathLength()-fDistanceLow) * fSlope + fWeightLow;
		fDistWeightInterpolated = min( fWeightLow, max( fWeightHigh, fDistWeightInterpolated ) );
	}
	else
	{
		// interpolate linearly between a low and a high distance
		float fDistanceLow = 8;
		float fWeightLow = 6;
		float fDistanceHigh = 36;
		float fWeightHigh = 1;

		float fSlope = (fWeightHigh-fWeightLow) / (fDistanceHigh-fDistanceLow);
		fDistWeightInterpolated = (target.GetPathLength()-fDistanceLow) * fSlope + fWeightLow;
		fDistWeightInterpolated = min( fWeightLow, max( fWeightHigh, fDistWeightInterpolated ) );
	}

	//scores for each target are estimated before calling this function
	float fDesirability = 1.f;
	if (pTargetCity->IsOriginalCapital() && (m_pPlayer->GetDiplomacyAI()->IsGoingForWorldConquest() || m_pPlayer->GetDiplomacyAI()->IsCloseToWorldConquest()))
	{
		fDesirability *= /*250*/ GD_INT_GET(AI_MILITARY_CAPTURING_ORIGINAL_CAPITAL);
		fDesirability /= 100;
	}
	if (pTargetCity->getOriginalOwner() == m_pPlayer->GetID())
	{
		fDesirability *= /*150*/ GD_INT_GET(AI_MILITARY_RECAPTURING_OWN_CITY);
		fDesirability /= 100;
	}

	//Going after a City-State? Depends if it has allies
	if (GET_PLAYER(pTargetCity->getOwner()).isMinorCiv())
	{
		//in general prefer to target major players, except rome
		//todo: maybe also factor in other traits? austria, venice, germany, greece? what about statecraft
		if (m_pPlayer->GetPlayerTraits()->GetCityStateCombatModifier() <= 0)
			fDesirability /= 2;

		//unless they are allied to our enemy
		PlayerTypes eAlly = GET_PLAYER(pTargetCity->getOwner()).GetMinorCivAI()->GetAlly();
		if (eAlly != NO_PLAYER)
		{
			if (GET_TEAM(GET_PLAYER(eAlly).getTeam()).isAtWar(GetPlayer()->getTeam()))
			{
				fDesirability *= m_pPlayer->GetPlayerTraits()->GetCityStateCombatModifier() > 0 ? 200 : 100;
				fDesirability /= 250;
			}
		}
		else if (m_pPlayer->IsAtWarAnyMajor() && !m_pPlayer->IsAtWarWith(pTargetCity->getOwner()))
		{
			if (!MOD_BALANCE_VP || m_pPlayer->GetPlayerTraits()->GetCityStateCombatModifier() <= 0)
			{
				//don't target minors at all while at war with an unrelated major, except rome
				return 0;
			}
			else
				fDesirability /= 2; // reduce emphasis for rome
		}
	}

	if (m_pPlayer->isMajorCiv())
	{
		// If we were given a quest to go to war with this player, that should influence our decision. Plus, it probably means he's a total jerk.
		bool bAnyLiberationQuest = false;
		for (int iMinorLoop = MAX_MAJOR_CIVS; iMinorLoop < MAX_CIV_PLAYERS; iMinorLoop++)
		{
			PlayerTypes eMinor = (PlayerTypes) iMinorLoop;
			CvPlayer* pMinor = &GET_PLAYER(eMinor);
			if (!pMinor->isAlive())
				continue;

			CvMinorCivAI* pMinorCivAI = pMinor->GetMinorCivAI();

			if (pMinorCivAI->IsActiveQuestForPlayer(m_pPlayer->GetID(), MINOR_CIV_QUEST_LIBERATION))
			{
				if (m_pPlayer->GetPlayerToLiberate(pTargetCity) == pMinor->GetMinorCivAI()->GetQuestData3(m_pPlayer->GetID(), MINOR_CIV_QUEST_LIBERATION)
					&& pTargetCity->getX() == pMinor->GetMinorCivAI()->GetQuestData1(m_pPlayer->GetID(), MINOR_CIV_QUEST_LIBERATION)
					&& pTargetCity->getY() == pMinor->GetMinorCivAI()->GetQuestData2(m_pPlayer->GetID(), MINOR_CIV_QUEST_LIBERATION))
				{
					fDesirability *= /*200*/ GD_INT_GET(AI_MILITARY_RECAPTURING_CITY_STATE);
					fDesirability /= 100;
					bAnyLiberationQuest = true;
				}
			}

			if (pMinorCivAI->IsActiveQuestForPlayer(GetPlayer()->GetID(), MINOR_CIV_QUEST_WAR))
			{
				if (pMinorCivAI->GetQuestData1(GetPlayer()->GetID(), MINOR_CIV_QUEST_WAR) == pTargetCity->getOwner())
				{
					fDesirability *= /*150*/ GD_INT_GET(AI_MILITARY_RECAPTURING_OWN_CITY);
					fDesirability /= 100;
				}
			}

			if (pMinorCivAI->IsActiveQuestForPlayer(GetPlayer()->GetID(), MINOR_CIV_QUEST_ACQUIRE_CITY))
			{
				int iX = pMinorCivAI->GetQuestData1(GetPlayer()->GetID(), MINOR_CIV_QUEST_ACQUIRE_CITY);
				int iY = pMinorCivAI->GetQuestData2(GetPlayer()->GetID(), MINOR_CIV_QUEST_ACQUIRE_CITY);

				CvPlot* pPlot = GC.getMap().plot(iX, iY);
				if (pPlot != NULL && pPlot->isCity())
				{
					if (pPlot->getOwner() == pTargetCity->getOwner())
					{
						fDesirability *= /*150*/ GD_INT_GET(AI_MILITARY_RECAPTURING_OWN_CITY);
						fDesirability /= 100;
					}
				}
			}
		}
		// Are we trying to liberate this city for a non-quest reason? That deserves a bonus.
		if (!bAnyLiberationQuest && m_pPlayer->GetDiplomacyAI()->IsTryingToLiberate(pTargetCity))
		{
			fDesirability *= /*150*/ GD_INT_GET(AI_MILITARY_RECAPTURING_OWN_CITY);
			fDesirability /= 100;
		}
	}

	//If we get instant yields from conquering specific terrain types, look for cities with those
	for (int iYield = 0; iYield < YIELD_TOURISM; iYield++)
	{
		YieldTypes eYield = (YieldTypes)iYield;
		for (int iTerrain = 0; iTerrain < GC.getNumTerrainInfos(); iTerrain++)
		{
			TerrainTypes eTerrain = (TerrainTypes)iTerrain;
			if (m_pPlayer->GetPlayerTraits()->GetYieldChangeFromTileConquest(eTerrain, eYield) > 0)
			{
				int iNumTerrain = pTargetCity->CountTerrain(eTerrain); // not using CountAllOwnedTerrain to save performance
				if (iNumTerrain > 0)
				{
					fDesirability *= 100 + (3 * iNumTerrain);
					fDesirability /= 100;
				}
			}
		}
	}

	if (MOD_BALANCE_RESOURCE_MONOPOLIES)
	{
		for(int iResourceLoop = 0; iResourceLoop < GC.getNumResourceInfos(); iResourceLoop++)
		{
			ResourceTypes eResource = (ResourceTypes) iResourceLoop;
			if(pTargetCity->IsHasResourceLocal(eResource, true) && (GC.getMap().getNumResources(eResource) > 0))
			{
				//Will this get us closer to a monopoly?
				int iBonus = (((m_pPlayer->getNumResourceTotal(eResource, false) + m_pPlayer->getResourceExport(eResource)) * 100) / GC.getMap().getNumResources(eResource));
				if(iBonus > 0)
				{
					fDesirability *= 125;
					fDesirability /= 100;
					break;
				}
			}
		}
	}

	// Economic value / hardness of target (clamp to non-negative — maintenance can make economic value negative)
	float fEconomicValue =  sqrt( max(0, pTargetCity->getEconomicValue( GetPlayer()->GetID() )) / float(max(1,pTargetCity->GetMaxHitPoints()-pTargetCity->getDamage())) );

	//everything together now
	int iRtnValue = (int)(target.m_iApproachScore * fDistWeightInterpolated * fDesirability * fEconomicValue);

	/*
	if(GC.getLogging() && GC.getAILogging())
	{
		// Open the right file
		CvString playerName = GetPlayer()->getCivilizationShortDescription();
		FILogFile* pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);

		CvString msg = CvString::format( "%03d, %s is evaluating attack on %s from base in %s. Approach is %.2f. Distance is %d (weight %.2f), desirability %.2f, exposure score %.2f, economic value %.2f --> score %d\n", 
			GC.getGame().getElapsedGameTurns(), m_pPlayer->getCivilizationShortDescription(), target.m_pTargetCity->getName().c_str(), target.m_pMusterCity->getName().c_str(), 
			fApproachMultiplier, target.m_iPathLength, fDistWeightInterpolated, fDesirability, fExposureScore, fEconomicValue, iRtnValue );
		pLog->Msg( msg.c_str() );
	}
	*/

	return iRtnValue;
}

// score 100: optimal terrain, score 0 worst terrain 
int MilitaryAIHelpers::EvaluateTargetApproach(const CvAttackTarget& target, PlayerTypes ePlayer, ArmyType eArmyType)
{
	CvPlot* pMusterPlot = target.GetMusterPlot();
	CvPlot* pStagingPlot = target.GetStagingPlot();
	CvPlot* pTargetPlot = target.GetTargetPlot();
	bool bMountainBonus = GET_PLAYER(ePlayer).CanCrossMountain();
	bool bForestJungleBonus = GET_PLAYER(ePlayer).GetPlayerTraits()->IsWoodlandMovementBonus();
	bool bHillBonus = GET_PLAYER(ePlayer).GetPlayerTraits()->IsFasterInHills();
	bool bRiverBonus = GET_PLAYER(ePlayer).GetPlayerTraits()->IsRiverTradeRoad();

	//basic sanity check
	if (eArmyType == ARMY_TYPE_LAND)
	{
		if (pStagingPlot->isWater())
			return 0;
	}
	else
	{
		if (!pStagingPlot->isWater())
			return 0;
	}

	//Expanded to look at three hexes around each city - will give a better understanding of approach.
	int nGoodPlots = 0;
	int nUsablePlots = 0;

	//all else equal, we prefer a pure naval approach over combined
	int iScale = (eArmyType == ARMY_TYPE_COMBINED ? 54 : 45);

	CvCity* pTargetCity = pTargetPlot->getPlotCity();
	if (!pTargetCity)
		return 0;

	TeamTypes eTeam = GET_PLAYER(ePlayer).getTeam();
	for (int i = RING0_PLOTS; i < RING4_PLOTS; i++)
	{
		bool bIsGood = false;
		CvPlot* pLoopPlot = iterateRingPlots(pTargetPlot,i);
		if (pLoopPlot == NULL)
			continue;

		//cannot go here? important, ignore territory checks (typically we are at peace without open borders)
		if (!pLoopPlot->isValidMovePlot(ePlayer,false) || pLoopPlot->isCity())
			continue;

		//ignore plots owned by third parties
		if (pLoopPlot->isOwned() && pLoopPlot->getTeam() != eTeam && pLoopPlot->getTeam() != pTargetPlot->getTeam())
			if (!GET_TEAM(eTeam).IsAllowsOpenBordersToTeam(pLoopPlot->getTeam()))
				continue;

		//ignore plots outside of the zone
		int iStagingDistance = plotDistance(*pStagingPlot, *pLoopPlot);
		int iTargetDistance = plotDistance(*pTargetPlot, *pLoopPlot);
		if (iTargetDistance > 3 || iStagingDistance > 3)
			continue;

		//correct area?
		if (!pTargetCity->HasAccessToArea(pLoopPlot->getArea()))
			continue;

		if (eArmyType==ARMY_TYPE_LAND)
			//siege weapons cannot set up here
			if (pLoopPlot->isWater())
				continue;

		if (eArmyType==ARMY_TYPE_NAVAL)
			//ships cannot go here
			if (!pLoopPlot->isWater())
				continue;

		//for naval invasions we want coastal plots
		//ignore inland plots, combined attacks tend to get higher scores than pure naval anyway)
		if (eArmyType == ARMY_TYPE_COMBINED)
			if (!pLoopPlot->isWater() && !pLoopPlot->isCoastalLand())
				continue;

		//enemy citadels are dangerous, pretend we cannot use those plots
		if (TacticalAIHelpers::GetOtherPlayerImprovementDamage(pLoopPlot, ePlayer, false) >= 25)
			continue;

		//rough terrain makes us slow
		bool bWoodlandException = bForestJungleBonus && (pLoopPlot->getFeatureType() == FEATURE_FOREST || pLoopPlot->getFeatureType() == FEATURE_JUNGLE) && (MOD_BALANCE_ALTERNATE_IROQUOIS_TRAIT || pLoopPlot->getTeam() == GET_PLAYER(ePlayer).getTeam());
		if (bWoodlandException || (bMountainBonus && pLoopPlot->isMountain()) || (bHillBonus && pLoopPlot->isHills()) || (bRiverBonus && pLoopPlot->isRiver()) || !pLoopPlot->isRoughGround())
			bIsGood = true;

		// owned by us and has a road?
		bIsGood |= pLoopPlot->getTeam() == GET_PLAYER(ePlayer).getTeam() && pLoopPlot->getRouteType() != NO_ROUTE && !pLoopPlot->IsRoutePillaged();

		//we want to have plots for our siege units
		//hard coded?
		bIsGood |= iTargetDistance == 2 && pLoopPlot->canSeePlot(pTargetPlot,NO_TEAM,2,NO_DIRECTION);

		if (bIsGood)
			nGoodPlots++;
		else
			nUsablePlots++;
	}

	//we have 17 eligible plots, so max score is 51
	int iScore = nGoodPlots * 3 + nUsablePlots;
	int iResult = (iScore * 100) / iScale;

	if (gDebugOutput)
	{
		CvCity* pMusterCity = pMusterPlot->getPlotCity();

		OutputDebugString(CvString::format("%s attack approach on %s from %s is %d. staging at (%d:%d), good %d, ok %d\n", 
			ArmyTypeToString(eArmyType), pTargetCity->getNameKey(), pMusterCity ? pMusterCity->getNameKey() : "unknown", iResult, pStagingPlot->getX(), pStagingPlot->getY(), nGoodPlots, nUsablePlots).c_str());
	}

	return iResult;
}

// PROVIDE MILITARY DATA TO OTHER SUBSYSTEMS

bool CvMilitaryAI::ShouldFightBarbarians() const
{
	return m_iBarbarianCampCount > 2 || m_iVisibleBarbarianCount > 4;
}

/// How many civs are we fighting?
int CvMilitaryAI::GetNumberCivsAtWarWith(bool bIncludeMinor) const
{
	vector<PlayerTypes> vEnemies = m_pPlayer->GetPlayersAtWarWith();

	int iRtnValue = 0;
	for(size_t i=0; i<vEnemies.size(); i++)
	{
		//ignore barbarians ...
		if(GET_PLAYER(vEnemies[i]).isBarbarian())
			continue;

		if(GET_PLAYER(vEnemies[i]).isMinorCiv() && !bIncludeMinor)
			continue;

		iRtnValue++;
	}

	return iRtnValue;
}

/// Log it if we issue a DOW
void CvMilitaryAI::LogDeclarationOfWar(PlayerTypes eOpponent)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString opponentName;
		CvString playerName;
		CvString strTemp;
		FILogFile* pLog = NULL;

		// Open the right file
		playerName = GetPlayer()->getCivilizationShortDescription();
		opponentName = GET_PLAYER(eOpponent).getCivilizationShortDescription();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += playerName + ", ";

		// Strategy Info
		strTemp.Format("Declared War on: %s", opponentName.GetCString());
		strOutBuf = strBaseString + strTemp;
		pLog->Msg(strOutBuf);
	}
}

/// Log it if we capture a city
void CvMilitaryAI::LogCityCaptured(CvCity* pCity, PlayerTypes eOldOwner)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strPlayerName;
		CvString strOldOwnerName;
		CvString strCityName;
		CvString strTemp;
		FILogFile* pLog = NULL;

		// Open the right file
		strPlayerName = GetPlayer()->getCivilizationShortDescription();
		strOldOwnerName = GET_PLAYER(eOldOwner).getCivilizationShortDescription();
		strCityName = pCity->getName();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(strPlayerName), FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += strPlayerName + ", ";

		// Strategy Info
		strTemp = "Captured: " + strCityName + ", ";
		strTemp += "From: " + strOldOwnerName;
		strOutBuf = strBaseString + strTemp;
		pLog->Msg(strOutBuf);
	}
}

/// Log it if we raze a city
void CvMilitaryAI::LogCityRazed(CvCity* pCity, PlayerTypes eOldOwner)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strPlayerName;
		CvString strOldOwnerName;
		CvString strCityName;
		CvString strTemp;
		FILogFile* pLog = NULL;

		// Open the right file
		strPlayerName = GetPlayer()->getCivilizationShortDescription();
		strOldOwnerName = GET_PLAYER(eOldOwner).getCivilizationShortDescription();
		strCityName = pCity->getName();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(strPlayerName), FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += strPlayerName + ", ";

		// Strategy Info
		strTemp = "Razed: " + strCityName + ", ";
		strTemp += "From: " + strOldOwnerName;
		strOutBuf = strBaseString + strTemp;
		pLog->Msg(strOutBuf);
	}
}

/// Log it if we make peace
void CvMilitaryAI::LogPeace(PlayerTypes eOpponent)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strOpponentName;
		CvString strPlayerName;
		CvString strTemp;
		FILogFile* pLog = NULL;

		// Open the right file
		strPlayerName = GetPlayer()->getCivilizationShortDescription();
		strOpponentName = GET_PLAYER(eOpponent).getCivilizationShortDescription();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(strPlayerName), FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += strPlayerName + ", ";

		// Strategy Info
		strTemp.Format("Made peace with: %s", strOpponentName.GetCString());
		strOutBuf = strBaseString + strTemp;
		pLog->Msg(strOutBuf);
	}
}

/// Log it if we make peace
void CvMilitaryAI::LogPeace(TeamTypes eOpponentTeam)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strOpponentName;
		CvString strPlayerName;
		CvString strTemp;
		FILogFile* pLog = NULL;

		// Open the right file
		strPlayerName = GetPlayer()->getCivilizationShortDescription();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(strPlayerName), FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += strPlayerName + ", ";

		// Collect the names of the players on the team
		for(int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes) iPlayer);
			if(kPlayer.isAlive() && kPlayer.getTeam() == eOpponentTeam)
			{
				if(strOpponentName.GetLength() != 0)
					strOpponentName += ", ";

				strOpponentName += kPlayer.getCivilizationShortDescription();
			}
		}
		// Strategy Info
		strTemp.Format("Made peace with team: %d (%s)", (int)eOpponentTeam, strOpponentName.GetCString());
		strOutBuf = strBaseString + strTemp;
		pLog->Msg(strOutBuf);
	}
}

/// Log that a unit is being scrapped because of the deficit checks
void CvMilitaryAI::LogDeficitScrapUnit(CvUnit* pUnit, bool bGifted)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strTemp;

		// Open the right file
		CvString playerName = m_pPlayer->getCivilizationShortDescription();
		FILogFile* pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);

		strOutBuf.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strOutBuf += playerName + ", ";
		if (bGifted)
			strTemp.Format("Gifting %s, X: %d, Y: %d, ", pUnit->getUnitInfo().GetDescription(), pUnit->getX(), pUnit->getY());
		else
			strTemp.Format("Scrapping %s, X: %d, Y: %d, ", pUnit->getUnitInfo().GetDescription(), pUnit->getX(), pUnit->getY());
		strOutBuf += strTemp;
		strOutBuf += "b/c bankrupt, , ";   //extra space so format is consistent with LogScrapUnit()

		if(pUnit->getDomainType() == DOMAIN_LAND)
		{
			strTemp.Format("Num Land Units: %d, In Armies %d, Rec Size: %d, ", m_iNumLandUnits, m_iNumLandUnitsInArmies, m_iRecLandUnits + m_iRecExplorerUnits);
		}
		else
		{
			strTemp.Format("Num Naval Units: %d, In Armies %d, Rec Size %d, ", m_iNumNavalUnits, m_iNumNavalUnitsInArmies, m_iRecNavalUnits);
		}

		strOutBuf += strTemp;
		if (pLog)
			pLog->Msg(strOutBuf);
	}
}

// PRIVATE METHODS

/// Compute data we need later to make decisions during the turn processing
void CvMilitaryAI::UpdateBaseData()
{

	// Count how many Military Units I have right now
	m_iNumLandUnits = 0;
	m_iNumRangedLandUnits = 0;
	m_iNumMobileLandUnits = 0;
	m_iNumAirUnits = 0;
	m_iNumAntiAirUnits = 0;
	m_iNumMeleeLandUnits = 0;
	m_iNumNavalUnits = 0;
	m_iNumLandUnitsInArmies = 0;
	m_iNumNavalUnitsInArmies = 0;
	m_iRecNavalUnits = 0;
	m_iNumFreeCarriers = 0;
	
	// new counters
	m_iNumArcherLandUnits = 0;
	m_iNumSiegeLandUnits = 0;
	m_iNumSkirmisherLandUnits = 0;
	m_iNumReconLandUnits = 0;
	m_iNumBomberAirUnits = 0;
	m_iNumFighterAirUnits = 0;
	m_iNumMeleeNavalUnits = 0;
	m_iNumRangedNavalUnits = 0;
	m_iNumSubmarineNavalUnits = 0;
	m_iNumCarrierNavalUnits = 0;
	m_iNumMissileUnits = 0;
	m_iNumActiveUniqueUnits = 0;

	int iLoop = 0;
	for (CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
	{
		// Don't count civilians or exploration units
		if (!pLoopUnit->IsCanAttack())
			continue;

/*		if (m_pPlayer->getCivilizationInfo().isCivilizationUnitOverridden(pLoopUnit->getUnitInfo().GetUnitClassType())) //not currently working, hope for the next release
			m_iNumActiveUniqueUnits++;
*/
		if (pLoopUnit->getDomainType() == DOMAIN_LAND)
		{
			m_iNumLandUnits++;

			if (pLoopUnit->getArmyID() != -1)
				m_iNumLandUnitsInArmies++;

			if (pLoopUnit->IsCanAttackRanged()) // still counts all land ranged
			{
				m_iNumRangedLandUnits++;
				if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_ARCHER", true) && !pLoopUnit->getUnitInfo().IsMounted())
					m_iNumArcherLandUnits++;
				else if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_ARCHER", true) && pLoopUnit->getUnitInfo().IsMounted())
					m_iNumSkirmisherLandUnits++;
				else if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_SIEGE", true))
					m_iNumSiegeLandUnits++;
			}
			else if (pLoopUnit->canIntercept())
				m_iNumAntiAirUnits++;
			else if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_MOUNTED", true) || pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_ARMOR", true))
				m_iNumMobileLandUnits++;
			else if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_RECON", true))
				m_iNumReconLandUnits++;
			else
				m_iNumMeleeLandUnits++;
		}
		else if (pLoopUnit->getDomainType() == DOMAIN_SEA)
		{
			m_iNumNavalUnits++;

			if (pLoopUnit->getArmyID() != -1)
				m_iNumNavalUnitsInArmies++;

			if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_NAVALMELEE", true))
				m_iNumMeleeNavalUnits++;
			else if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_NAVALRANGED", true))
				m_iNumRangedNavalUnits++;
			else if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_SUBMARINE", true))
				m_iNumSubmarineNavalUnits++;

			if (pLoopUnit->AI_getUnitAIType() == UNITAI_CARRIER_SEA)
			{
				m_iNumCarrierNavalUnits++;
				//a carrier is considered free if it is not in a strike group or empty
				if (pLoopUnit->getArmyID() == -1 || pLoopUnit->getCargo() == 0)
					m_iNumFreeCarriers++;
			}
		}
		else if (pLoopUnit->getDomainType() == DOMAIN_AIR && !pLoopUnit->isSuicide())
		{
			m_iNumAirUnits++;

			if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_BOMBER", true))
				m_iNumBomberAirUnits++;
			else if (pLoopUnit->getUnitCombatType() == (UnitCombatTypes)GC.getInfoTypeForString("UNITCOMBAT_FIGHTER", true))
				m_iNumFighterAirUnits++;
		}
		else if (pLoopUnit->getDomainType() == DOMAIN_AIR && pLoopUnit->isSuicide()) // missiles&bombs
		{
//			if (pLoopUnit->getUnitInfo().GetDefaultUnitAIType() == UNITAI_ICBM)
//				m_iNumNukeUnits++; //both atomic bomb&nuclear missile, but already counted by CvPlayer
			if (pLoopUnit->getUnitInfo().GetDefaultUnitAIType() == UNITAI_MISSILE_AIR)
				m_iNumMissileUnits++;
		}
	}

	// finally check how many units we want
	SetRecommendedArmyNavySize();
}

///	How many military units should we have given current threats?
void CvMilitaryAI::SetRecommendedArmyNavySize()
{
	// how many units can we afford?
	int iMaxPossibleUnits = m_pPlayer->GetEconomicAI()->GetSoftSupplyCap();

	// offense is simple for minors
	if (m_pPlayer->isMinorCiv())
	{
		m_iRecNavalUnits = 0;
		if (m_pPlayer->getCapitalCity() && m_pPlayer->getCapitalCity()->isCoastal())
			m_iRecNavalUnits = 2;
		m_iRecLandUnits = iMaxPossibleUnits - m_iRecNavalUnits;
		return;
	}

	int iNumCoastalCities = m_pPlayer->GetNumEffectiveCoastalCities();

	int iLandDefenseWeight = /*3*/ GD_INT_GET(AI_STRATEGY_DEFEND_MY_LANDS_BASE_UNITS) * 10;
	int iNavalDefenseWeight = 0;

	// 1 Unit per City & 1 per Settler
	iLandDefenseWeight += (int)(m_pPlayer->getNumCities() * 10 * /*1.0f*/ GD_FLOAT_GET(AI_STRATEGY_DEFEND_MY_LANDS_UNITS_PER_CITY));
	iLandDefenseWeight += m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_SETTLE, true, true) * 10;

	iNavalDefenseWeight += (int)(iNumCoastalCities * 7 * /*1.0f*/ GD_FLOAT_GET(AI_STRATEGY_DEFEND_MY_LANDS_UNITS_PER_CITY));

	int iLoop = 0;
	for(CvCity* pCity = m_pPlayer->firstCity(&iLoop); pCity != NULL; pCity = m_pPlayer->nextCity(&iLoop))
	{
		//additional units if the enemy is likely to attack here
		if (m_pPlayer->GetMilitaryAI()->IsExposedToEnemy(pCity, NO_PLAYER, ARMY_TYPE_COMBINED))
		{
			iLandDefenseWeight += 10;
			iNavalDefenseWeight += 10;
		}
		else
		{
			if (m_pPlayer->GetMilitaryAI()->IsExposedToEnemy(pCity, NO_PLAYER, ARMY_TYPE_LAND))
				iLandDefenseWeight += 10;
			if (m_pPlayer->GetMilitaryAI()->IsExposedToEnemy(pCity, NO_PLAYER, ARMY_TYPE_NAVAL))
				iNavalDefenseWeight += 10;
		}
	}

	int iTotalOffenseWeight = 0;

	//Look at our competitors ...
	for (int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		PlayerTypes eOtherPlayer = (PlayerTypes) iMajorLoop;
		if (eOtherPlayer != m_pPlayer->GetID())
		{
			//one army per target
			if (GetPlayer()->GetMilitaryAI()->HavePreferredAttackTarget(eOtherPlayer))
				iTotalOffenseWeight += /*6*/ GD_INT_GET(BALANCE_BASIC_ATTACK_ARMY_SIZE) * 10;
		}
	}

	// if we are going for conquest we want at least one more task force, more in later eras
	if (m_pPlayer->isMajorCiv() && m_pPlayer->GetDiplomacyAI()->IsGoingForWorldConquest())
		iTotalOffenseWeight += /*6*/ GD_INT_GET(BALANCE_BASIC_ATTACK_ARMY_SIZE) * 10;

	// now how many should be naval units?
	// int iCoastalPercent = (iNumCoastalCities * 100) / max(1, m_pPlayer->getNumCities());
	int iFlavorNaval = m_pPlayer->GetFlavorManager()->GetPersonalityIndividualFlavor((FlavorTypes)GC.getInfoTypeForString("FLAVOR_NAVAL"));
	int iNavalPercent = (iNumCoastalCities * iFlavorNaval * 7) / max(1, m_pPlayer->getNumCities());

	// Modifiers
	int iFlavorDefense = m_pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_DEFENSE"));
	int iDefenseModifier = 100 + iFlavorDefense * 2;

	int iFlavorOffense = m_pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_OFFENSE"));
	int iOffenseModifier = 100 + m_pPlayer->GetDiplomacyAI()->GetBoldness() + iFlavorOffense;

	iLandDefenseWeight = iLandDefenseWeight * iDefenseModifier;
	iNavalDefenseWeight = iNavalDefenseWeight * iDefenseModifier;
	iTotalOffenseWeight = iTotalOffenseWeight * iOffenseModifier;

	int iTotalWeight = iTotalOffenseWeight + iLandDefenseWeight + iNavalDefenseWeight;

	int iNumExplorers = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_EXPLORE, false, true);
	int iNumUnitsToSupply = m_pPlayer->GetNumUnitsToSupply();
	int iNumUnitsSupplied = m_pPlayer->GetNumUnitsSupplied();

	// Limit explorers to 1/4 of total supply
	int iExplorersNeeded = min(iMaxPossibleUnits / 4, m_pPlayer->GetEconomicAI()->GetExplorersNeeded());

	// If we have too many units to be able to supply this many explorers, limit the number of explorers
	if (iNumUnitsToSupply - iNumExplorers + iExplorersNeeded > iNumUnitsSupplied)
		iExplorersNeeded = iNumExplorers + iNumUnitsSupplied - iNumUnitsToSupply;

	m_iRecExplorerUnits = iExplorersNeeded;

	// If we have more explorers than we need, don't build more units above the force limit anyway
	iMaxPossibleUnits -= max(iExplorersNeeded, iNumExplorers);
	
	// 64 bit math for the numerator - the modifier-scaled weights times the unit cap exceed INT_MAX for large late-game empires
	m_iRecLandUnits = static_cast<int>(((long long)iMaxPossibleUnits * (100LL * iLandDefenseWeight + (long long)iTotalOffenseWeight * (100 - iNavalPercent)) / (100.0f * iTotalWeight)) + 0.5f);
	m_iRecNavalUnits = static_cast<int>(((long long)iMaxPossibleUnits * (100LL * iNavalDefenseWeight + (long long)iTotalOffenseWeight * iNavalPercent)) / (100.0f * iTotalWeight) + 0.5f);

	int iNumLandUnits = m_pPlayer->GetNumUnitsWithDomain(DOMAIN_LAND, true) - iNumExplorers;
	int iNumNavalUnits = m_pPlayer->GetNumUnitsWithDomain(DOMAIN_SEA, true);

	if (iNumLandUnits > m_iRecLandUnits)
	{
		int iDiff = iNumLandUnits - m_iRecLandUnits;
		m_iRecLandUnits += iDiff;
		m_iRecNavalUnits -= iDiff;
	}
	else if (iNumNavalUnits > m_iRecNavalUnits)
	{
		int iDiff = iNumNavalUnits - m_iRecNavalUnits;
		m_iRecNavalUnits += iDiff;
		m_iRecLandUnits -= iDiff;
	}

	//AI_AB_TEST (candidate, flag TimingBuildup): at peace a timing attacker wants its garrisons (and its siege train, up
	//to 3); in the build-up window it wants garrisons plus a 6-unit field army, which makes CP's production build units
	if (IsTimingBuildupPlayer() && !m_pPlayer->IsAtWarAnyMajor())
	{
		const int iCities = max(1, m_pPlayer->getNumCities());
		const int iSiege = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true);
		if (IsInTimingBuildupWindow())
			m_iRecLandUnits = max(m_iRecLandUnits, (IsTimingBuildupBigArmy() || IsTimingBuildupWarBudget()) ? GetTimingBuildupSiegeTarget() + GetTimingBuildupFieldTarget() + 2 : iCities + 6);
		else if (IsTimingBuildupPeace())
			m_iRecLandUnits = min(m_iRecLandUnits, GetTimingBuildupPeaceCap() + min(3, iSiege));
	}
}

/// AI_AB_TEST (candidate, flag TimingBuildup, the user's design): a timing attacker doesn't keep a big army all game. It
/// keeps garrisons and an escort at peace (no upkeep for an idle army), builds up fast for 15 turns once it has a target
/// (production, gold saved ahead, purchases), attacks while the army is fresh, and lets the extras go after the peace.
bool CvMilitaryAI::IsTimingBuildupPlayer() const
{
	return m_pPlayer->IsCandidateWarmonger(FIX_TIMING_BUILDUP);
}

static int TimingBuildupWindowTurns()
{
	return max(1, 15 * GC.getGame().getGameSpeedInfo().getTrainPercent() / 100);
}

/// peace phase: no war with a major and no build-up window or attack under way
bool CvMilitaryAI::IsTimingBuildupPeace() const
{
	return IsTimingBuildupPlayer() && !m_pPlayer->IsAtWarAnyMajor() && m_iTimingBuildupStartTurn < 0;
}

bool CvMilitaryAI::IsInTimingBuildupWindow() const
{
	if (m_iTimingBuildupStartTurn < 0)
		return false;
	//a strike's build-up (EarlyStrike, LateStrike) has no length: it lasts until the army is ready, or as ready as it gets, and
	//the attack phase declares; the strike state's exits end it otherwise (the user: no windows in the AI's own judgement)
	if (IsStrikeBuildup())
		return !IsTimingBuildupArmyReady() && !IsStrikeAsReadyAsItGets();
	const int iElapsed = GC.getGame().getGameTurn() - m_iTimingBuildupStartTurn;
	if (iElapsed < TimingBuildupWindowTurns())
		return true;
	//AI_AB_TEST (candidate, flag WindowUntilReady): the window goes on until the army is ready, up to 2 windows, which leaves the
	//attack phase one window before the plan goes back to peace (3 windows) as the fallback. A test window of 10 turns built 2
	//Warriors and an Archer, then its army target and unit weights ended and units were skipped as over the army size; a coop
	//war took that army to war and lost it (0DB01EF0).
	//AI_AB_TEST (diagnostic flag BigArmy): the doubled army gets up to 4 windows to be built (WindowUntilReady or not)
	if (IsTimingBuildupBigArmy())
		return iElapsed < 4 * TimingBuildupWindowTurns() && !IsTimingBuildupArmyReady();
	return m_pPlayer->UsesCandidateAI(FIX_WINDOW_UNTIL_READY) && iElapsed < 2 * TimingBuildupWindowTurns() && !IsTimingBuildupArmyReady();
}

bool CvMilitaryAI::HasTimingBuildupWindowEnded() const
{
	return m_iTimingBuildupStartTurn >= 0 && !IsInTimingBuildupWindow();
}

/// ready to attack: 2 siege units and 4 field units on top of a garrison per city (DeclareTiming's army)
/// AI_AB_TEST (flag ArmyQuality): land combat units (not scouts) at least 70% as strong as the best one of their kind (melee
/// or ranged, siege apart) the civ can train;
/// the rest of the TimingBuildup army's field units count for nothing. 22% of the BigArmy armies at their first declaration
/// were Warriors and Scouts, counted as army and paid for like the rest.
int CvMilitaryAI::GetArmyQualityFloor(bool bRanged) const
{
	const int iTurn = GC.getGame().getGameTurn();
	if (m_iArmyQualityTurn != iTurn)
	{
		m_iArmyQualityTurn = iTurn;
		m_iArmyQualityFloor = 0;
		m_iArmyQualityRangedFloor = 0;
		const CvCity* pCapital = m_pPlayer->getCapitalCity();
		int iBest = 0, iBestRanged = 0;
		for (int iUnit = 0; pCapital && iUnit < GC.getNumUnitInfos(); iUnit++)
		{
			const UnitTypes eUnit = (UnitTypes)iUnit;
			const CvUnitEntry* pkEntry = GC.getUnitInfo(eUnit);
			if (!pkEntry || pkEntry->GetDomainType() != DOMAIN_LAND || pkEntry->GetDefaultUnitAIType() == UNITAI_EXPLORE ||
				pkEntry->GetDefaultUnitAIType() == UNITAI_CITY_BOMBARD)	// siege is counted apart
				continue;
			const bool bIsRanged = pkEntry->GetRangedCombat() > 0;
			const int iStrength = bIsRanged ? pkEntry->GetRangedCombat() : pkEntry->GetCombat();
			if (iStrength <= (bIsRanged ? iBestRanged : iBest) || !pCapital->canTrain(eUnit))
				continue;
			if (bIsRanged) iBestRanged = iStrength; else iBest = iStrength;
		}
		m_iArmyQualityFloor = iBest * 70 / 100;
		m_iArmyQualityRangedFloor = iBestRanged * 70 / 100;
	}
	return bRanged ? m_iArmyQualityRangedFloor : m_iArmyQualityFloor;
}

/// melee units against the best melee unit, ranged ones against the best ranged unit
bool CvMilitaryAI::IsArmyQualityUnit(const CvUnitEntry* pkUnitEntry) const
{
	if (!pkUnitEntry || pkUnitEntry->GetDomainType() != DOMAIN_LAND || pkUnitEntry->GetDefaultUnitAIType() == UNITAI_EXPLORE)
		return false;
	const bool bRanged = pkUnitEntry->GetRangedCombat() > 0;
	return (bRanged ? pkUnitEntry->GetRangedCombat() : pkUnitEntry->GetCombat()) >= GetArmyQualityFloor(bRanged);
}

/// the TimingBuildup army's field units: land military units that aren't siege (ArmyQuality: only those above the floor)
static int TimingBuildupFieldUnits(const CvPlayer* pPlayer, const CvMilitaryAI* pMilitary, int iSiege)
{
	if (!pPlayer->UsesCandidateAI(FIX_ARMY_QUALITY))
		return const_cast<CvPlayer*>(pPlayer)->GetNumUnitsWithDomain(DOMAIN_LAND, true) - iSiege;	// GetNumUnitsWithDomain isn't const
	int iCount = 0, iLoop = 0;
	for (const CvUnit* pLoopUnit = pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = pPlayer->nextUnit(&iLoop))
	{
		if (pLoopUnit->getDomainType() != DOMAIN_LAND || pLoopUnit->AI_getUnitAIType() == UNITAI_CITY_BOMBARD || !pLoopUnit->IsCombatUnit())
			continue;
		if (pMilitary->IsArmyQualityUnit(&pLoopUnit->getUnitInfo()))
			iCount++;
	}
	return iCount;
}

int CvMilitaryAI::GetTimingBuildupFieldUnits() const
{
	return TimingBuildupFieldUnits(m_pPlayer, this, m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true));
}

bool CvMilitaryAI::IsTimingBuildupArmyBuilt() const
{
	const int iSiege = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true);
	//a strike: siege we can't train yet (its tech not in) doesn't hold the army back (Persia, 15A7D14F: BUILDUP for 73 turns
	//against a 2-siege target with 7 land units); checked once a turn
	int iSiegeTarget = GetTimingBuildupSiegeTarget();
	if (iSiegeTarget > 0 && iSiege < iSiegeTarget && IsStrikeBuildup())
	{
		const int iTurn = GC.getGame().getGameTurn();
		if (m_iSiegeTrainableTurn != iTurn)
		{
			m_iSiegeTrainableTurn = iTurn;
			m_bSiegeTrainable = GetTimingBuildupUnitType(true) != NO_UNIT;
		}
		if (!m_bSiegeTrainable)
			iSiegeTarget = 0;
	}
	return iSiege >= iSiegeTarget && TimingBuildupFieldUnits(m_pPlayer, this, iSiege) >= GetTimingBuildupFieldTarget();
}

/// AI_AB_TEST (flag EarlyStrike, upgrade and go; Economy keeps the treasury for it, patch 0029): until the strike an
/// EarlyStrike civ upgrades nothing (CP upgrades whenever it can pay, so the reserve went on early Warrior to Spearman
/// upgrades). The upgrades come once the strike's army is built (or its window is over), and the declaration waits for
/// them. At war, or past turn 90 (Standard), CP upgrades as usual. Read by CvUnit::CanUpgradeRightNow, every AI upgrade path.
bool CvMilitaryAI::IsEarlyStrikeHoldingUpgrades() const
{
	//on the strike state, no turns: EarlyStrike holds through PREP (the book) and BUILDUP; LateStrike through BUILDUP (Economy
	//saves for the upgrades there). The hold lifts once the strike's build-up has its army built (or its window is over).
	const StrikeStateTypes eState = GetStrikeState();
	if (eState == STRIKE_PREP)
		return IsEarlyStrikeCiv();
	if (eState != STRIKE_BUILDUP)
		return false;
	//lift it as soon as the army is otherwise ready to go by any measure the declaration uses (the normal army, Personality's
	//plan army, or as ready as it gets), so the upgrades happen before the declaration and not after it (Greece, 1464B588,
	//declared at T63 with 289 gold and 2 Archers held; its first upgrade came at T99, in the field)
	if (m_iTimingBuildupStartTurn < 0)
		return true;
	//upgrade and go: once the strike's own upgrade tech is in, the hold has done its job (the gold is there) and the upgrades
	//happen now, at home, before the units muster. Aztec (152231C3, ab-8f9b7f9) held 2 Archers with 300 gold from T50 to T71;
	//its sneak attack then mustered them out of our territory, the bill (home units only) fell to 0 and it declared at T77
	//with nothing upgraded
	const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
	TechTypes eUpgradeTech = kPlan.bValid ? kPlan.eUpgradeTech : NO_TECH;
	CvStrikeWindow kWindow;
	if (eUpgradeTech == NO_TECH && GetEarlyStrikeWindow(kWindow))
		eUpgradeTech = kWindow.eUpgradeTech;
	if (eUpgradeTech == NO_TECH || GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech(eUpgradeTech))
		return false;
	// (the lead's review of 88cf09c: CanUpgradeRightNow reaches here from the UI) the snapshot, computed in our own turn
	return !m_bStrikeUpgradeGo;
}

static bool StrikeCityHasWalls(const CvCity* pCity)
{
	static const BuildingClassTypes eWalls = (BuildingClassTypes)GC.getInfoTypeForString("BUILDINGCLASS_WALLS", true);
	return pCity && eWalls != NO_BUILDINGCLASS && pCity->GetCityBuildings()->HasBuildingClass(eWalls);
}

/// AI_AB_TEST (flags EarlyStrike, LateStrike; the user: "dont have the ai judge itself based on the window during a game"; the
/// lead's go, 10:41pm ET). The strike's state from game state only, once a turn, saved, logged on a change:
///   NONE     not a strike civ
///   PREP     EarlyStrike: the military-first book, no target in reach yet. LateStrike: economy first, not economy-ready yet
///   BUILDUP  EarlyStrike: the committed target's nearest city in reach (12 tiles of our capital) and the core trainable.
///            LateStrike: economy-ready (for now the plan's K cities and the opening book done; Personality's condition to
///            come). Economy beelines and saves; War's unit weights start once the core (or pre-build) can be trained
///   WAR      we declared (our wars-declared count rose) during BUILDUP
///   OVER     an exit: the core unit obsolete; a major declared on us first; the target's nearest city walled and the plan
///            brings no siege; the target fields the counter (3+ units with a bonus against the core's combat class near its
///            city); the army built in PREP with no target in reach; at war, the army below half its size at the declaration
///            with no capture; peace
/// AI_AB_TEST (EarlyStrike; the user: "when no major civ is in reach, the early strike may target a city-state"): the nearest
/// city-state whose city is within 12 tiles of our capital that we can declare on, with no ally (or one we're at war with),
/// no protector at peace with us, no recent peace, and not a walled hill city
PlayerTypes CvMilitaryAI::FindStrikeCityState(bool bUnwalledOnly, PlayerTypes eExclude) const
{
	const CvCity* pCapital = m_pPlayer->getCapitalCity();
	if (!pCapital)
		return NO_PLAYER;
	const TeamTypes eOurTeam = m_pPlayer->getTeam();
	PlayerTypes eBest = NO_PLAYER;
	int iBestDistance = 13;
	for (int iMinor = MAX_MAJOR_CIVS; iMinor < MAX_CIV_PLAYERS; iMinor++)
	{
		const PlayerTypes eMinor = (PlayerTypes)iMinor;
		CvPlayer& kMinor = GET_PLAYER(eMinor);
		if (eMinor == eExclude || !kMinor.isAlive() || !kMinor.isMinorCiv() || !GET_TEAM(eOurTeam).isHasMet(kMinor.getTeam()))
			continue;
		if (!GET_TEAM(eOurTeam).isAtWar(kMinor.getTeam()) && !GET_TEAM(eOurTeam).canDeclareWar(kMinor.getTeam(), m_pPlayer->GetID()))
			continue;
		const PlayerTypes eAlly = kMinor.GetMinorCivAI()->GetAlly();
		if (eAlly == m_pPlayer->GetID() || (eAlly != NO_PLAYER && !GET_TEAM(eOurTeam).isAtWar(GET_PLAYER(eAlly).getTeam())))
			continue;
		bool bProtected = false;
		for (int iMajor = 0; iMajor < MAX_MAJOR_CIVS && !bProtected; iMajor++)
		{
			const PlayerTypes eMajor = (PlayerTypes)iMajor;
			if (eMajor != m_pPlayer->GetID() && GET_PLAYER(eMajor).isAlive() && kMinor.GetMinorCivAI()->IsProtectedByMajor(eMajor) &&
				!GET_TEAM(eOurTeam).isAtWar(GET_PLAYER(eMajor).getTeam()))
				bProtected = true;
		}
		if (bProtected)	// no peace-treaty turns (the user's no-turn rule): only what canDeclareWar itself forbids
			continue;
		const CvCity* pCity = m_pPlayer->GetClosestCityToUsByPlots(eMinor);
		if (!pCity || !pCity->plot()->isRevealed(eOurTeam))
			continue;
		// an army can get there (the lead, 17A1A659: Milan, 9 tiles away and no army path)
		if (!HaveStrikeLandAttackTarget(eMinor))
			continue;
		if (StrikeCityHasWalls(pCity) && (bUnwalledOnly || pCity->plot()->isHills()))
			continue;
		const int iDistance = plotDistance(pCapital->getX(), pCapital->getY(), pCity->getX(), pCity->getY());
		if (iDistance < iBestDistance)
		{
			iBestDistance = iDistance;
			eBest = eMinor;
		}
	}
	return eBest;
}

/// AI_AB_TEST (EarlyStrike, LateStrike): one attacker type in the strike's take-the-city math. Per hit, x100: the whole hit,
/// and while the city's garrison lives, the city's part and the garrison's share (CORE_GARRISON_DAMAGE_ABSORPTION)
struct StrikeCityAttacker
{
	int iFullDamage100;		// no garrison (or it's dead)
	int iCityDamage100;		// with the garrison alive: the city's part
	int iGarrisonDamage100;	// and the garrison's share
	int iSelfDamage100;
	bool bMelee;		// a melee hit (every other turn: it takes the city's counter-damage)
	bool bHitsCity;		// the tactical AI makes this attack: ranged and siege always, melee only if it isn't close to suicide
	bool bCaptures;		// can walk in (melee, not no-capture)
	int iDefence100;	// its melee strength x100 (the city's strike picks our best defender)
	int iCount;
};

/// the tactical AI's melee test (CvTacticalAI::ComputeTotalExpectedDamage, CvAIOperation::GetArmyExpectedCityDamage): a melee
/// attack on the city counts only if it does more than it costs, or twice the cost with more than half the hit points left
/// (at full hit points: the army comes up healed)
static bool StrikeMeleeHitsCity(int iTotalDamage, int iSelfDamage, int iMaxHP)
{
	return iTotalDamage > iSelfDamage || (iTotalDamage * 2 > iSelfDamage && iMaxHP - iSelfDamage > iMaxHP / 2);
}

/// the garrison the strike may count: only one we can see (the lead's review of ae8e9ae: fair play under the fog)
static const CvUnit* StrikeVisibleGarrison(const CvCity* pCity, TeamTypes eTeam)
{
	return (pCity && pCity->plot()->isVisible(eTeam)) ? pCity->GetGarrisonedUnit() : NULL;
}

/// AI_AB_TEST (EarlyStrike, LateStrike; the user: "having a turn limit is bad. just calculate if you can take the city or
/// not", "you probably want a little buffer"; the user's ruling, Nov 7: "my jaguars never attacked the city until the last
/// hit. they killed units, pillaged, and soaked hits while 4 archers slowly brought the city down"): can these attackers
/// take the city, the way our tactical AI fights a siege? Turn by turn:
/// - the city's damage comes from ranged and siege units each turn, and from a melee unit's hit only where the tactical AI
///   makes it (StrikeMeleeHitsCity), every other turn;
/// - while the garrison lives it takes its share of each hit; once our hits have killed it, the city takes them whole (the
///   lead's review of ae8e9ae: the Warrior in Marrakesh dies by about turn 3);
/// - the city heals CITY_HIT_POINTS_HEALED_PER_TURN a turn (plus its population in VP), nothing while blockaded, as the
///   tactical AI's gate counts it;
/// - takeable only within iMaxTurns: 13, the tactical AI's own siege gate (CvTacticalAI, "too early for attacking"), CP's
///   rule for when units attack the city, not a horizon on the strike; 3 in an enemy-dominated zone (SiegeDominated).
///   18BE1E3E: the old math said "about 3-4 turns" for La Venta (defence 23), the tactical AI made 0 melee attacks and
///   called it too early 33 turns;
/// - a melee unit to take the last hit (the capturer; SiegeCapturer brings it up);
/// - over the turns it takes, the city's strike (one a turn) and the melee counter-damage cost less than the army's hit
///   points less one unit;
/// - a buffer: our damage at STRIKE_TAKE_DAMAGE_PERCENT, the city's damage to us at STRIKE_TAKE_LOSS_PERCENT.
static bool StrikeArmyTakesCity(const CvCity* pCity, const std::vector<StrikeCityAttacker>& vArmy, int iGarrisonHP, int iMaxTurns, int* piTurnsNeeded)
{
	if (piTurnsNeeded)
		*piTurnsNeeded = -1;
	if (!pCity)
		return false;
	const CvSeeder kNoSeed;
	const int iCityStrike100 = max(100, pCity->getStrengthValue(/*bForRangeStrike*/ true));
	const int iCityMaxHP100 = max(1, pCity->GetMaxHitPoints()) * 100;
	int iCityHeal = /*20 in CP, 8 in VP*/ GD_INT_GET(CITY_HIT_POINTS_HEALED_PER_TURN);
	if (MOD_BALANCE_VP)
		iCityHeal += pCity->getPopulation();	// as the tactical AI's gate
	if (pCity->IsBlockadedWaterAndLand())
		iCityHeal = 0;
	int iFull100 = 0, iCity100 = 0, iGarrison100 = 0, iSelfDamage100 = 0, iArmyHP = 0, iBestDefence100 = 0, iUnits = 0;
	bool bCapturer = false;
	const int iUnitHP = 100;
	for (size_t i = 0; i < vArmy.size(); i++)
	{
		const StrikeCityAttacker& kA = vArmy[i];
		if (kA.iCount <= 0)
			continue;
		iUnits += kA.iCount;
		iArmyHP += kA.iCount * iUnitHP;
		iBestDefence100 = max(iBestDefence100, kA.iDefence100);
		bCapturer = bCapturer || kA.bCaptures;
		if (!kA.bHitsCity)
			continue;	// a melee unit that won't hit the walls kills the field units, pillages and soaks hits
		const int iDiv = kA.bMelee ? 2 : 1;
		iFull100 += kA.iCount * kA.iFullDamage100 / iDiv;
		iCity100 += kA.iCount * kA.iCityDamage100 / iDiv;
		iGarrison100 += kA.iCount * kA.iGarrisonDamage100 / iDiv;
		if (kA.bMelee)
			iSelfDamage100 += kA.iCount * kA.iSelfDamage100 / 2;
	}
	if (iUnits == 0 || !bCapturer)
		return false;
	iFull100 = iFull100 * CvMilitaryAI::STRIKE_TAKE_DAMAGE_PERCENT / 100;
	iCity100 = iCity100 * CvMilitaryAI::STRIKE_TAKE_DAMAGE_PERCENT / 100;
	iGarrison100 = iGarrison100 * CvMilitaryAI::STRIKE_TAKE_DAMAGE_PERCENT / 100;
	iSelfDamage100 = iSelfDamage100 * CvMilitaryAI::STRIKE_TAKE_LOSS_PERCENT / 100;
	int iHP100 = max(1, pCity->GetMaxHitPoints() - pCity->getDamage()) * 100;
	int iGarrisonHP100 = max(0, iGarrisonHP) * 100;
	int iTurns = -1;
	for (int iTurn = 1; iTurn <= iMaxTurns; iTurn++)
	{
		if (iGarrisonHP100 > 0)
		{
			iHP100 -= iCity100;
			iGarrisonHP100 -= iGarrison100;
		}
		else
			iHP100 -= iFull100;
		iHP100 += iCityHeal * 100;	// the heal counts every turn, as in the tactical AI's (damage - heal) x 13
		if (iHP100 <= 0)
		{
			iTurns = iTurn;
			break;
		}
		iHP100 = min(iHP100, iCityMaxHP100);
	}
	if (iTurns < 0)
		return false;	// the city heals faster than we hurt it, or the tactical AI wouldn't start this siege
	if (piTurnsNeeded)
		*piTurnsNeeded = iTurns;
	const int iCityStrikeDamage100 = CvUnitCombat::DoDamageMath(iCityStrike100, max(100, iBestDefence100),
		GD_INT_GET(RANGE_ATTACK_SAME_STRENGTH_MIN_DAMAGE), GD_INT_GET(RANGE_ATTACK_SAME_STRENGTH_POSSIBLE_EXTRA_DAMAGE), false, kNoSeed, 0);
	const int iLoss = iTurns * (iCityStrikeDamage100 * CvMilitaryAI::STRIKE_TAKE_LOSS_PERCENT / 100 + iSelfDamage100) / 100;
	return iLoss < iArmyHP - iUnitHP;
}

/// unit types and counts (the plan's army, as it will be) as attackers: the game's expected damage math (CvUnitCombat::
/// DoDamageMath) against the city's defence, the garrison's share as CvUnit::GetRangeCombatDamage takes it
static void StrikeAttackersFromEntries(const CvCity* pCity, const CvUnit* pGarrison, const std::vector< std::pair<const CvUnitEntry*, int> >& vEntries,
	std::vector<StrikeCityAttacker>& vOut)
{
	const CvSeeder kNoSeed;
	const int iCityDefence100 = max(100, pCity->getStrengthValue());
	const int iGarrisonMaxHP = (MOD_CORE_GARRISON_DAMAGE_ABSORPTION && pGarrison) ? pGarrison->GetMaxHitPoints() : 0;
	const int iCityMaxHP = max(1, pCity->GetMaxHitPoints());
	for (size_t i = 0; i < vEntries.size(); i++)
	{
		const CvUnitEntry* pkUnit = vEntries[i].first;
		if (!pkUnit || vEntries[i].second <= 0)
			continue;
		StrikeCityAttacker kA;
		kA.iCount = vEntries[i].second;
		kA.iDefence100 = pkUnit->GetCombat() * 100;
		kA.iSelfDamage100 = 0;
		// DoDamageMath returns hit points x100 (the game divides by 100 for HP): kept x100 here (the lead's trial24 1664FF61)
		int iTotal100 = 0;
		if (pkUnit->GetRangedCombat() > 0)
		{
			kA.bMelee = false;
			kA.bCaptures = false;
			iTotal100 = CvUnitCombat::DoDamageMath(pkUnit->GetRangedCombat() * 100, iCityDefence100,
				GD_INT_GET(RANGE_ATTACK_SAME_STRENGTH_MIN_DAMAGE), GD_INT_GET(RANGE_ATTACK_SAME_STRENGTH_POSSIBLE_EXTRA_DAMAGE), false, kNoSeed, 0);
			kA.bHitsCity = true;
		}
		else if (pkUnit->GetCombat() > 0)
		{
			kA.bMelee = true;
			kA.bCaptures = true;	// (no-capture is a promotion, on the unit, not its entry)
			iTotal100 = CvUnitCombat::DoDamageMath(pkUnit->GetCombat() * 100, iCityDefence100,
				GD_INT_GET(ATTACK_SAME_STRENGTH_MIN_DAMAGE), GD_INT_GET(ATTACK_SAME_STRENGTH_POSSIBLE_EXTRA_DAMAGE), false, kNoSeed, 0);
			kA.iSelfDamage100 = CvUnitCombat::DoDamageMath(iCityDefence100, pkUnit->GetCombat() * 100,
				GD_INT_GET(ATTACK_SAME_STRENGTH_MIN_DAMAGE), GD_INT_GET(ATTACK_SAME_STRENGTH_POSSIBLE_EXTRA_DAMAGE), false, kNoSeed, 0);
			kA.bHitsCity = StrikeMeleeHitsCity(iTotal100, kA.iSelfDamage100, 100 * 100);
		}
		else
			continue;
		// the garrison's share (CvUnit::GetRangeCombatDamage, getMeleeCombatDamageCity)
		kA.iFullDamage100 = iTotal100;
		kA.iGarrisonDamage100 = iGarrisonMaxHP > 0 ? (iTotal100 * 2 * iGarrisonMaxHP) / (iCityMaxHP + 2 * iGarrisonMaxHP) : 0;
		kA.iCityDamage100 = iTotal100 - kA.iGarrisonDamage100;
		vOut.push_back(kA);
	}
}

/// the strike plan's army (core and support at their counts, as the units they become at the plan's upgrade tech) against the
/// city: the build-up's target decisions (go, a clearly easier city, siege). No plan: no check (true)
bool CvMilitaryAI::IsStrikeArmyStrongEnough(const CvCity* pCity, int* piTurnsNeeded) const
{
	if (piTurnsNeeded)
		*piTurnsNeeded = -1;
	const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
	if (!pCity || !kPlan.bValid)
		return true;
	std::vector< std::pair<const CvUnitEntry*, int> > vArmy;
	const UnitClassTypes aeClass[2] = { kPlan.eCoreClass, kPlan.eSupportClass };
	const int aiCount[2] = { kPlan.iCoreTarget, kPlan.iSupportTarget };
	for (int i = 0; i < 2; i++)
	{
		const UnitTypes eUnit = (aeClass[i] != NO_UNITCLASS && aiCount[i] > 0) ? m_pPlayer->GetSpecificUnitType(aeClass[i]) : NO_UNIT;
		CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
		if (!pkUnit)
			continue;
		if (kPlan.eUpgradeTech != NO_TECH)	// as the unit it becomes at the strike's upgrade
		{
			for (int iClass = 0; iClass < GC.getNumUnitClassInfos(); iClass++)
			{
				if (!pkUnit->GetUpgradeUnitClass(iClass))
					continue;
				const UnitTypes eTo = m_pPlayer->GetSpecificUnitType((UnitClassTypes)iClass);
				CvUnitEntry* pkTo = (eTo != NO_UNIT) ? GC.getUnitInfo(eTo) : NULL;
				if (pkTo && pkTo->GetPrereqAndTech() == (int)kPlan.eUpgradeTech)
				{
					pkUnit = pkTo;
					break;
				}
			}
		}
		vArmy.push_back(std::make_pair((const CvUnitEntry*)pkUnit, aiCount[i]));
	}
	if (vArmy.empty())
		return true;
	std::vector<StrikeCityAttacker> vAttackers;
	const CvUnit* pGarrison = StrikeVisibleGarrison(pCity, m_pPlayer->getTeam());
	StrikeAttackersFromEntries(pCity, pGarrison, vArmy, vAttackers);
	return StrikeArmyTakesCity(pCity, vAttackers, pGarrison ? pGarrison->GetCurrHitPoints() : 0, STRIKE_TACTICAL_SIEGE_TURNS, piTurnsNeeded);
}

/// the field army: our land combat units but the garrisons (the lead's review: a city's garrison doesn't march)
static void GetStrikeFieldUnits(const CvPlayer* pPlayer, std::vector<const CvUnit*>& vUnits)
{
	vUnits.clear();
	int iLoop = 0;
	for (const CvUnit* pLoopUnit = pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = pPlayer->nextUnit(&iLoop))
	{
		if (pLoopUnit->getDomainType() != DOMAIN_LAND || (!pLoopUnit->IsCombatUnit() && !pLoopUnit->IsCanAttackRanged()) ||
			pLoopUnit->AI_getUnitAIType() == UNITAI_EXPLORE || pLoopUnit->isDelayedDeath() || pLoopUnit->IsGarrisoned())
			continue;
		vUnits.push_back(pLoopUnit);
	}
}

/// the user: "only start the attack if the city is takeable": the land combat units we have (siege included, as they are)
/// against the strike's target's nearest city. No target city: false
/// (the lead's review of 677f7a4: multiplayer sync) the stored answer, computed in UpdateStrikeState during our own turn
/// (RefreshStrikeSnapshot) and saved: the trade screen and other UI paths read it, never compute it
bool CvMilitaryAI::IsStrikeTargetTakeableNow(int* piTurnsNeeded) const
{
	if (piTurnsNeeded)
		*piTurnsNeeded = -1;
	return m_bTakeable;
}

/// the computation behind it (our own turn only)
bool CvMilitaryAI::ComputeStrikeTargetTakeable() const
{
	const PlayerTypes eTarget = (m_eTimingBuildupTarget != NO_PLAYER) ? m_eTimingBuildupTarget : GetStrikeCityStateTarget();
	const CvCity* pCity = (eTarget != NO_PLAYER) ? m_pPlayer->GetClosestCityToUsByPlots(eTarget) : NULL;
	// no army path to the target, nothing to take (the strike's reach, 17A1A659: no declaration on a city no army can reach)
	if (!pCity || !HaveStrikeLandAttackTarget(eTarget))
		return false;
	std::vector<const CvUnit*> vUnits;
	GetStrikeFieldUnits(m_pPlayer, vUnits);
	return StrikeUnitsTakeCity(vUnits, pCity);
}

/// AI_AB_TEST (the lead's review of 677f7a4: multiplayer sync) the strike's game-state answers that UI paths reach (the trade
/// screen's IsPossibleToTradeItem -> the declaration block -> readiness / no-retreat; canTrain -> the K-city cap): computed here,
/// in UpdateStrikeState during our own turn, and saved. Nothing outside our turn computes them
void CvMilitaryAI::RefreshStrikeSnapshot()
{
	m_bTakeable = ComputeStrikeTargetTakeable();
	m_bStrikeReady = IsTimingBuildupArmyReady() || IsStrikeAsReadyAsItGets();	// after m_bTakeable, which both read
	// the upgrade hold's "army going" (IsEarlyStrikeHoldingUpgrades; the UI reaches it through CanUpgradeRightNow)
	m_bStrikeUpgradeGo = IsTimingBuildupArmyBuilt() || HasTimingBuildupWindowEnded() || m_pPlayer->GetGrandStrategyAI()->IsStrikeArmyReady() ||
		IsStrikeAsReadyAsItGets();
	const PlayerTypes eHeld = GetStrikeCityStateTarget();
	// (the lead's ruling on the review of cbebe77) the K-city cap lifts while the held target isn't a land attack target now:
	// a civ that can't fight its target can expand (0061's intent; the bans stay)
	m_bStrikeHoldingReachable = eHeld != NO_PLAYER && GET_PLAYER(eHeld).isAlive() && HaveStrikeLandAttackTarget(eHeld);
	const PlayerTypes eWar = (eHeld != NO_PLAYER) ? eHeld : m_eTimingBuildupTarget;
	m_bStrikeTargetFinished = eWar != NO_PLAYER && IsStrikeTargetFinished(eWar);
}

/// these units against the city (the strike's take-the-city math, StrikeArmyTakesCity), less one of ours for each land
/// combat unit of the city's owner within 3 tiles of it: each defender ties up one attacker (melee first). The lead's review:
/// the target's own units weren't counted. Used for the army on hand (IsStrikeTargetTakeableNow) and for an operation's
/// army (CvAIOperation::IsArmyTooWeakForTarget). Each unit's hit is the tactical AI's own simulation
/// (TacticalAIHelpers::GetSimulatedDamageFromAttackOnCity: its promotions, the city's garrison and its share), so the
/// estimate matches the battlefield AI (the user's ruling, Nov 7)
bool CvMilitaryAI::StrikeUnitsTakeCity(const std::vector<const CvUnit*>& vUnits, const CvCity* pCity, int* piTurnsNeeded) const
{
	if (piTurnsNeeded)
		*piTurnsNeeded = -1;
	if (!pCity)
		return false;
	// the garrison only if we can see it (fog: the city's own strength still has it)
	const CvUnit* pGarrison = StrikeVisibleGarrison(pCity, m_pPlayer->getTeam());
	std::vector<StrikeCityAttacker> vMelee, vRanged;
	for (size_t i = 0; i < vUnits.size(); i++)
	{
		const CvUnit* pUnit = vUnits[i];
		if (!pUnit)
			continue;
		int iSelfDamage = 0, iGarrisonDamage = 0;
		const int iCityDamage = TacticalAIHelpers::GetSimulatedDamageFromAttackOnCity(pCity, pUnit, pUnit->plot(), iSelfDamage, iGarrisonDamage,
			/*bIgnoreUnitAdjacencyBoni*/ true, 0, 0, 0, /*bQuickAndDirty*/ true, /*bOverrideGarrison*/ true, pGarrison);
		StrikeCityAttacker kA;
		kA.iCount = 1;
		kA.iDefence100 = pUnit->getUnitInfo().GetCombat() * 100;
		kA.iFullDamage100 = (iCityDamage + iGarrisonDamage) * 100;	// the share is taken off the whole hit
		kA.iCityDamage100 = iCityDamage * 100;
		kA.iGarrisonDamage100 = iGarrisonDamage * 100;
		kA.iSelfDamage100 = iSelfDamage * 100;
		kA.bMelee = !pUnit->IsCanAttackRanged();
		kA.bCaptures = kA.bMelee && !pUnit->isNoCapture();
		kA.bHitsCity = !kA.bMelee || StrikeMeleeHitsCity(iCityDamage + iGarrisonDamage, iSelfDamage, pUnit->GetMaxHitPoints());
		(kA.bMelee ? vMelee : vRanged).push_back(kA);
	}
	int iDefenders = 0, iLoop = 0;
	const PlayerTypes eOwner = pCity->getOwner();
	// the owner's units, and (before our war on it) those of the majors its defensive pacts would bring in (the lead's review of
	// 12fe132: under the user's all-in rule the strike declares into a pact, so the math counts the pact's units at the city)
	std::vector<PlayerTypes> vDefending(1, eOwner);
	const TeamTypes eOurTeam = m_pPlayer->getTeam();
	if (!GET_TEAM(eOurTeam).isAtWar(pCity->getTeam()))
	{
		std::vector<PlayerTypes> vPact = m_pPlayer->GetDiplomacyAI()->GetDefensiveWarAllies(eOwner, /*bIncludeMinors*/ false, /*bReverseMode*/ true, /*bNewWarsOnly*/ true);
		vDefending.insert(vDefending.end(), vPact.begin(), vPact.end());
	}
	else
	{
		// (the lead's reviews of 647fdd3 and 677f7a4) at war: every civ at war with us and not with the city's owner (the pact's
		// partners among them; a third party besieging the same city isn't a defender), city-states too; not barbarians
		for (int iOther = 0; iOther < MAX_CIV_PLAYERS; iOther++)
		{
			const PlayerTypes eOther = (PlayerTypes)iOther;
			if (eOther != eOwner && GET_PLAYER(eOther).isAlive() && GET_TEAM(eOurTeam).isAtWar(GET_PLAYER(eOther).getTeam()) &&
				!GET_TEAM(GET_PLAYER(eOther).getTeam()).isAtWar(pCity->getTeam()))
				vDefending.push_back(eOther);
		}
	}
	for (size_t iD = 0; iD < vDefending.size(); iD++)
	{
		if (vDefending[iD] == NO_PLAYER || vDefending[iD] == m_pPlayer->GetID())
			continue;
		for (const CvUnit* pDefender = GET_PLAYER(vDefending[iD]).firstUnit(&iLoop); pDefender != NULL; pDefender = GET_PLAYER(vDefending[iD]).nextUnit(&iLoop))
		{
			// not the garrison: the city's strength has it (the lead's review)
			// and only what we can see (the lead's review of 647fdd3: fair play)
			if (pDefender->getDomainType() == DOMAIN_LAND && pDefender->IsCombatUnit() && !pDefender->isDelayedDeath() &&
				pDefender->plot() != pCity->plot() && plotDistance(pDefender->getX(), pDefender->getY(), pCity->getX(), pCity->getY()) <= 3 &&
				pDefender->plot()->isVisible(eOurTeam) && !pDefender->isInvisible(eOurTeam, false))
				iDefenders++;
		}
	}
	// one of ours per defender: melee first (the one that does the least to the walls), then ranged (the weakest). Never the
	// last capturer while a ranged unit can take the defender instead (the lead's review of ae8e9ae: 3 melee against 3
	// defenders left no capturer however many ranged units came, and the build-up trained only ranged and siege)
	for (int iTied = 0; iTied < iDefenders; iTied++)
	{
		int iCapturers = 0;
		for (size_t j = 0; j < vMelee.size(); j++)
			iCapturers += vMelee[j].bCaptures ? 1 : 0;
		bool bMeleeOk = false;	// a melee unit can go without taking the last capturer
		for (size_t j = 0; j < vMelee.size() && !bMeleeOk; j++)
			bMeleeOk = !vMelee[j].bCaptures || iCapturers > 1;
		std::vector<StrikeCityAttacker>* pGroup = (bMeleeOk || (!vMelee.empty() && vRanged.empty())) ? &vMelee : (!vRanged.empty() ? &vRanged : NULL);
		if (!pGroup)
			break;
		int iLeast = -1;
		for (size_t j = 0; j < pGroup->size(); j++)
		{
			if (pGroup == &vMelee && bMeleeOk && (*pGroup)[j].bCaptures && iCapturers <= 1)
				continue;	// keep the last capturer
			const int iThis = (*pGroup)[j].bHitsCity ? (*pGroup)[j].iFullDamage100 : 0;
			const int iBest = iLeast >= 0 ? ((*pGroup)[iLeast].bHitsCity ? (*pGroup)[iLeast].iFullDamage100 : 0) : INT_MAX;
			if (iThis < iBest)
				iLeast = (int)j;
		}
		if (iLeast < 0)
			break;
		pGroup->erase(pGroup->begin() + iLeast);
	}
	std::vector<StrikeCityAttacker> vArmy(vMelee);
	vArmy.insert(vArmy.end(), vRanged.begin(), vRanged.end());
	if (vArmy.empty())
		return false;
	// (the lead's review of ae8e9ae) the tactical AI's zone rule: a city in an enemy-dominated zone (land, and water if any) isn't
	// attacked, unless SiegeDominated gives the units at an enemy major's walls a look: 3 or more of them and a kill in 3 turns
	int iMaxTurns = STRIKE_TACTICAL_SIEGE_TURNS;
	CvTacticalAnalysisMap* pMap = m_pPlayer->GetTacticalAI()->GetTacticalAnalysisMap();
	CvTacticalDominanceZone* pLandZone = pMap ? pMap->GetZoneByCity(pCity, false) : NULL;
	CvTacticalDominanceZone* pWaterZone = pMap ? pMap->GetZoneByCity(pCity, true) : NULL;
	// (the lead's review of 6bf8952) as the tactical AI: no land zone, or an enemy-dominated one, falls back to the water zone (a
	// revealed city always has a land zone; before the war the target's units aren't enemy strength, so this bites at war)
	const bool bLandEnemy = !pLandZone || pLandZone->GetOverallDominanceFlag() == TACTICAL_DOMINANCE_ENEMY;
	const bool bWaterEnemy = !pWaterZone || pWaterZone->GetOverallDominanceFlag() == TACTICAL_DOMINANCE_ENEMY;
	if (pMap && bLandEnemy && bWaterEnemy && !pCity->isInDangerOfFalling())
	{
		if (pLandZone && m_pPlayer->UsesCandidateAI(FIX_SIEGE_DOMINATED) && eOwner < MAX_MAJOR_CIVS && GET_PLAYER(eOwner).isMajorCiv() &&
			GET_TEAM(eOurTeam).isAtWar(pCity->getTeam()) && vArmy.size() >= 3)
			iMaxTurns = 3;
		else
			return false;
	}
	return StrikeArmyTakesCity(pCity, vArmy, pGarrison ? pGarrison->GetCurrHitPoints() : 0, iMaxTurns, piTurnsNeeded);
}

/// a strike's declaration waits while one of our cities is in danger of falling (a third party at war with us: the lead,
/// Persia 15A7D14F was declared on by Germany and Ethiopia at T69); not an exit
bool CvMilitaryAI::IsStrikeHomeSafe() const
{
	int iLoop = 0;
	for (const CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop))
	{
		if (pLoopCity->isInDangerOfFalling())
			return false;
	}
	return true;
}

/// a strike civ before its strike (PREP, BUILDUP) declares no war of its own (the lead: CP's own wars ended strikes, Japan
/// 1468D9F4, Denmark 15E8F168) except the strike's: on its target, once the army is ready or as ready as it gets (both
/// require the city takeable and home safe)
bool CvMilitaryAI::IsStrikeDeclarationBlocked(PlayerTypes eOn) const
{
	const StrikeStateTypes eState = GetStrikeState();
	const bool bTarget = eOn != NO_PLAYER && (eOn == m_eTimingBuildupTarget || eOn == GetStrikeCityStateTarget());
	// in the strike's war, until the target's capital is taken, no second war either (169CA418: CP's sneak operation on
	// Morocco declared at T76 while the strike's war on Ur had taken nothing)
	if (eState == STRIKE_WAR)
	{
		const PlayerTypes eWarTarget = (GetStrikeCityStateTarget() != NO_PLAYER) ? GetStrikeCityStateTarget() : m_eTimingBuildupTarget;
		return !bTarget && eWarTarget != NO_PLAYER && IsStrikeNoRetreat(eWarTarget);
	}
	if (eState != STRIKE_PREP && eState != STRIKE_BUILDUP)
		return false;
	// the snapshot's readiness (the lead's review of 68df55d: the trade screen reaches here; readiness fills per-turn caches)
	if (eState == STRIKE_BUILDUP && bTarget && m_iTimingBuildupStartTurn >= 0 && m_bStrikeReady)
		return false;
	return true;
}

/// the strike's ownership (the user's structural fix, the lead's audit in RESULTS): while the strike is at war with this
/// player and has taken nothing from it (IsStrikeNoRetreat), or is in BUILDUP and already at war with its own target (the
/// city-state's declaration lands here first), no peace (CvTeam::makePeace), the approach toward it is WAR
/// (CvDiplomacyAI::SetCivApproach), and no operation against it retreats (CvAIOperation::SetToAbort)
bool CvMilitaryAI::IsStrikeRefusingPeace(PlayerTypes eOther) const
{
	if (eOther == NO_PLAYER || m_pPlayer->isHuman() || !m_pPlayer->isMajorCiv())
		return false;
	if (IsStrikeNoRetreat(eOther))
		return true;
	return GetStrikeState() == STRIKE_BUILDUP && (eOther == m_eTimingBuildupTarget || eOther == GetStrikeCityStateTarget()) &&
		GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eOther).getTeam());
}

/// the strike's ownership (gate 4, operations): PREP, BUILDUP, or WAR before the target's capital is taken
bool CvMilitaryAI::IsStrikeOwningOperations() const
{
	if (m_pPlayer->isHuman() || !m_pPlayer->isMajorCiv())
		return false;
	const StrikeStateTypes eState = GetStrikeState();
	if (eState == STRIKE_PREP || eState == STRIKE_BUILDUP)
		return true;
	if (eState != STRIKE_WAR)
		return false;
	const PlayerTypes eWarTarget = (GetStrikeCityStateTarget() != NO_PLAYER) ? GetStrikeCityStateTarget() : m_eTimingBuildupTarget;
	return eWarTarget != NO_PLAYER && IsStrikeNoRetreat(eWarTarget);
}

/// a player the strike's army may attack: its target (the window's, the pinned one, or CP's committed major target, which is
/// the strike's when no city-state is pinned), or anyone already at war with us (defence)
bool CvMilitaryAI::IsStrikeOperationTarget(PlayerTypes eOther) const
{
	if (eOther == NO_PLAYER)
		return false;
	if (eOther == m_eTimingBuildupTarget || eOther == GetStrikeCityStateTarget() || GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eOther).getTeam()))
		return true;
	return GetStrikeCityStateTarget() == NO_PLAYER && eOther == m_pPlayer->GetDiplomacyAI()->GetCommittedConquestTarget();
}

/// the strike's ownership (5): while the strike is on (PREP, BUILDUP, WAR), its land combat units are not CP's to disband or
/// gift (the peace cap, deficit scrapping, ArmyScrap, the greedy cap, idle-unit scrapping, unit gifts). Real bankruptcy (gold
/// below 0, the game's own rule) is exempt
bool CvMilitaryAI::IsStrikeKeepingUnit(const CvUnit* pUnit) const
{
	if (!pUnit || m_pPlayer->isHuman() || !m_pPlayer->isMajorCiv() || pUnit->getDomainType() != DOMAIN_LAND || !pUnit->IsCombatUnit())
		return false;
	const StrikeStateTypes eState = GetStrikeState();
	if (eState != STRIKE_PREP && eState != STRIKE_BUILDUP && eState != STRIKE_WAR)
		return false;
	return m_pPlayer->GetTreasury()->GetGold() >= 0;
}

/// the strike's capture measure (the lead's review: any capture ended the strike's war, a counter-attack on a third party
/// included): our cities whose previous owner is the strike's target (the pinned city-state or major, else the window's)
int CvMilitaryAI::GetStrikeTargetCaptures() const
{
	const PlayerTypes eTarget = (GetStrikeCityStateTarget() != NO_PLAYER) ? GetStrikeCityStateTarget() : m_eTimingBuildupTarget;
	if (eTarget == NO_PLAYER)
		return 0;
	int iCount = 0, iLoop = 0;
	for (const CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop))
	{
		if (pLoopCity->getPreviousOwner() == eTarget)
			iCount++;
	}
	return iCount;
}

/// AI_AB_TEST (the strike's objective; the user, the trio game: "huns took indonesias first city easily, but then the game ai
/// took over from the strike ai ... maybe we need to have the strike last until the capital is conquered"): the target's
/// original capital, wherever it stands now. NULL: not on the map (razed, or the target never had one)
const CvCity* CvMilitaryAI::GetStrikeObjectiveCity(PlayerTypes eTarget) const
{
	if (eTarget == NO_PLAYER)
		return NULL;
	const CvPlot* pPlot = GC.getMap().plot(GET_PLAYER(eTarget).GetOriginalCapitalX(), GET_PLAYER(eTarget).GetOriginalCapitalY());
	return (pPlot && pPlot->isCity()) ? pPlot->getPlotCity() : NULL;
}

/// AI_AB_TEST (the user: "the goal is the enemy capital. if we take that, we can declare peace and let them have a minor
/// settlement. i know sometimes you get penalties for eliminating civs completely"): the strike is finished with its target.
/// Dead; or, once a city has been taken from it in this war (the lead's review: a capital held before the war doesn't
/// count), its original capital is out of its hands (ours; or a third party's, or gone, so the goal can't be ours), or none of
/// its cities is a land attack target any more (an unreachable capital or remnant; the lead's checklist: a lock needs a
/// release when its goal can't be reached). Its other cities aren't the strike's to take
bool CvMilitaryAI::IsStrikeTargetFinished(PlayerTypes eTarget) const
{
	if (eTarget == NO_PLAYER || !GET_PLAYER(eTarget).isAlive())
		return true;
	if (GetStrikeTargetCaptures() <= m_iStrikeCapturedMark)
		return false;
	// (the lead's review of 88cf09c) the capital out of its hands is the only release besides death (all in)
	return IsStrikeTargetDone(eTarget);
}

/// AI_AB_TEST (the lead's review of 647fdd3: one "done" test everywhere): a major whose original capital is out of its hands
/// (ours, a third party's, or gone). The strike is done with it: never picked, held or declared on again. A city-state's one
/// city is its capital, so a living city-state is never done
bool CvMilitaryAI::IsStrikeTargetDone(PlayerTypes eTarget) const
{
	if (eTarget == NO_PLAYER || !GET_PLAYER(eTarget).isMajorCiv())
		return false;
	const CvCity* pCapital = GetStrikeObjectiveCity(eTarget);
	return !pCapital || pCapital->getOwner() != eTarget;
}

/// AI_AB_TEST (the strike's reach; the lead's reviews of 647fdd3, 677f7a4 and 68df55d): the path test for it. Like
/// MilitaryAIHelpers::ArmyPathIsGood (3 plots at most of a war zone or a third party's land; 3 at most near another city of
/// ours, a better start), but the target's own cities on the way don't count: its capital stands behind its border cities
static bool StrikeReachPathIsGood(const SPath& path, PlayerTypes eAttacker, PlayerTypes eTarget, int iStart)
{
	if (path.vPlots.size() < 4)
		return true;
	int iThirdPartyPlots = 0, iOurOtherCityPlots = 0;
	for (size_t i = 3; i < path.vPlots.size() - 3; i++)
	{
		CvPlot* pPlot = GC.getMap().plot(path.vPlots[i].x, path.vPlots[i].y);
		if (!pPlot || (GC.getGame().GetClosestCityDistanceInPlots(pPlot) > 3 && !pPlot->isOwned()))
			continue;
		CvCity* pCity = GC.getGame().GetClosestCityByPlots(pPlot);
		if (!pCity || pCity->getOwner() == eTarget)
			continue;
		if (pCity->getOwner() == eAttacker)
		{
			if (pCity->GetID() != iStart && pPlot->getOwningCityID() == pCity->GetID())
				iOurOtherCityPlots++;
		}
		else if (GET_PLAYER(eAttacker).IsAtWarWith(pCity->getOwner()))
			iThirdPartyPlots++;
		else if (pPlot->isOwned() && pPlot->getOwner() != eTarget && !pPlot->IsFriendlyTerritory(eAttacker))
			iThirdPartyPlots++;
	}
	return iThirdPartyPlots < 3 && iOurOtherCityPlots < 3;
}

/// AI_AB_TEST (the strike's reach; the lead's reviews of 647fdd3, 677f7a4 and 68df55d): a land army can get to the target's
/// goal city: a major's original capital while it holds it and once we've seen it (until then its nearest revealed city, so
/// an early strike can pick it on the city it sees), a city-state's city. From any city of ours on the goal city's landmass
/// (the nearest first, 3 tries at most): a land army path ignoring units and right of passage, no distance limit, passing
/// StrikeReachPathIsGood. For a new pick only (the lead's review of 88cf09c); no cache, our own turn only
bool CvMilitaryAI::IsStrikeTargetReachable(PlayerTypes eTarget) const
{
	if (eTarget == NO_PLAYER || !GET_PLAYER(eTarget).isAlive())
		return false;
	const TeamTypes eOurTeam = m_pPlayer->getTeam();
	const CvCity* pTheirs = NULL;
	if (GET_PLAYER(eTarget).isMajorCiv())
	{
		const CvCity* pCapital = GetStrikeObjectiveCity(eTarget);
		if (pCapital && pCapital->getOwner() == eTarget && pCapital->plot()->isAdjacentRevealed(eOurTeam))
			pTheirs = pCapital;
		else if (pCapital && pCapital->getOwner() != eTarget)
			return false;	// done (IsStrikeTargetDone): no goal left
	}
	// (the lead's review of 88cf09c) else its nearest city we've revealed (GetClosestCityToUsByPlots ignores the fog)
	const CvCity* pOurCapital = m_pPlayer->getCapitalCity();
	if (!pTheirs && pOurCapital)
	{
		int iBestRevealed = INT_MAX, iTheirLoop = 0;
		for (const CvCity* pTheirCity = GET_PLAYER(eTarget).firstCity(&iTheirLoop); pTheirCity != NULL; pTheirCity = GET_PLAYER(eTarget).nextCity(&iTheirLoop))
		{
			if (!pTheirCity->plot()->isAdjacentRevealed(eOurTeam))
				continue;
			const int iDistance = plotDistance(pOurCapital->getX(), pOurCapital->getY(), pTheirCity->getX(), pTheirCity->getY());
			if (iDistance < iBestRevealed)
			{
				iBestRevealed = iDistance;
				pTheirs = pTheirCity;
			}
		}
	}
	if (!pTheirs)
		return false;
	const int iLandmass = pTheirs->plot()->getLandmass();
	std::vector< std::pair<int, int> > vOurs;	// (distance, city ID) on the goal city's landmass
	int iLoop = 0;
	for (CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop))
		if (pLoopCity->plot()->getLandmass() == iLandmass)
			vOurs.push_back(std::make_pair(plotDistance(pLoopCity->getX(), pLoopCity->getY(), pTheirs->getX(), pTheirs->getY()), pLoopCity->GetID()));
	std::sort(vOurs.begin(), vOurs.end());
	for (size_t i = 0; i < vOurs.size() && i < 3; i++)
	{
		CvCity* pOurs = m_pPlayer->getCity(vOurs[i].second);
		if (!pOurs)
			continue;
		// no distance limit (the lead's review of 88cf09c: the land trade-route range rejected most capitals behind border cities)
		SPathFinderUserData data(m_pPlayer->GetID(), PT_ARMY_LAND, NO_PLAYER, INT_MAX);
		data.iFlags |= CvUnit::MOVEFLAG_IGNORE_RIGHT_OF_PASSAGE;
		data.iFlags |= CvUnit::MOVEFLAG_IGNORE_ENEMIES;
		SPath path = GC.GetStepFinder().GetPath(pOurs->plot(), pTheirs->plot(), data);
		if (!!path && StrikeReachPathIsGood(path, m_pPlayer->GetID(), eTarget, pOurs->GetID()))
			return true;
	}
	return false;
}

/// AI_AB_TEST (the lead's review of 88cf09c): a city of this player on a landmass where we have a city. Coarse and stable: the
/// hold's one release before the war, and the K-city cap's "holding a target"
bool CvMilitaryAI::IsStrikeTargetOnOurLand(PlayerTypes eTarget) const
{
	if (eTarget == NO_PLAYER)
		return false;
	std::vector<int> vOurLand;
	int iLoop = 0;
	for (const CvCity* pOurCity = m_pPlayer->firstCity(&iLoop); pOurCity != NULL; pOurCity = m_pPlayer->nextCity(&iLoop))
		vOurLand.push_back(pOurCity->plot()->getLandmass());
	for (const CvCity* pTheirCity = GET_PLAYER(eTarget).firstCity(&iLoop); pTheirCity != NULL; pTheirCity = GET_PLAYER(eTarget).nextCity(&iLoop))
		if (std::find(vOurLand.begin(), vOurLand.end(), pTheirCity->plot()->getLandmass()) != vOurLand.end())
			return true;
	return false;
}

/// AI_AB_TEST (the lead's review of 12fe132): the strike holds a target an army can ever reach. While it doesn't, Economy's
/// K-city cap lifts so a waiting strike civ can expand (the bans and the declaration block hold)
bool CvMilitaryAI::IsStrikeHoldingReachableTarget() const
{
	const PlayerTypes eHeld = GetStrikeCityStateTarget();
	(void)eHeld;
	return m_bStrikeHoldingReachable;	// the snapshot (canTrain reaches here from the UI)
}

/// cities taken from majors and from city-states (AB_COUNT_CITIES_CAPTURED counts majors only)
int CvMilitaryAI::GetStrikeCaptureCount() const
{
	return GC.getGame().GetABTestCount(m_pPlayer->GetID(), AB_COUNT_CITIES_CAPTURED) + GC.getGame().GetABTestCount(m_pPlayer->GetID(), AB_COUNT_CITY_STATES_CAPTURED);
}

/// no retreat once the strike is committed (the user: "i dont think there should be any retreat once you commit"): at the
/// strike's war against this player (alive, at war with us), until the target's capital is taken (IsStrikeTargetFinished). Read by the peace willingness,
/// the operations' aborts and the ownership gates. No release by losses (the user: "there is no 2nd place in civ ... they are
/// ALL IN. Their is no retreating"): a city lost, a city in danger or the war nearly lost no longer release it
bool CvMilitaryAI::IsStrikeNoRetreat(PlayerTypes eEnemy) const
{
	if (eEnemy == NO_PLAYER || GetStrikeState() != STRIKE_WAR)
		return false;
	if (eEnemy != m_eTimingBuildupTarget && eEnemy != GetStrikeCityStateTarget())
		return false;
	if (!GET_PLAYER(eEnemy).isAlive() || !GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eEnemy).getTeam()))
		return false;
	// the target's capital taken (the user: "the goal is the enemy capital"; IsStrikeTargetFinished): peace is allowed
	if (m_bStrikeTargetFinished)	// the snapshot (UI paths reach here: the trade screen, the peace gates)
		return false;
	return true;
}

/// AI_AB_TEST (the strike's WAR request; 18BE1E3E): a city attack operation (land, naval or combined) against this player,
/// not any operation outside our land (a rapid response aimed at the target's plot isn't the strike's attack)
bool CvMilitaryAI::HasStrikeCityAttackUnderWay(PlayerTypes eEnemy) const
{
	// every operation, not the first of a type (the lead's review of ae8e9ae: a finished first one hid a live second one)
	for (size_t i = 0; i < m_pPlayer->getNumAIOperations(); i++)
	{
		CvAIOperation* pOp = m_pPlayer->getAIOperationByIndex(i);
		if (!pOp || pOp->GetEnemy() != eEnemy)
			continue;
		const AIOperationTypes eType = pOp->GetOperationType();
		if (eType != AI_OPERATION_CITY_ATTACK_LAND && eType != AI_OPERATION_CITY_ATTACK_NAVAL && eType != AI_OPERATION_CITY_ATTACK_COMBINED)
			continue;
		const AIOperationState eState = pOp->GetOperationState();
		if (eState != AI_OPERATION_STATE_SUCCESSFUL_FINISH && eState != AI_OPERATION_STATE_ABORTED)
			return true;
	}
	return false;
}

/// AI_AB_TEST (EarlyStrike, LateStrike; the lead, Nov 6): with the strike's army too weak for its target, a clearly easier city
/// within 12 tiles of our capital: at most 75% of the target's defence and one the plan's army can take. A major's (one we can
/// declare on with no defensive allies to pull in, or are at war with) or a city-state's (no ally or one we're at war with,
/// no protector at peace with us, no recent peace). The weakest such city
PlayerTypes CvMilitaryAI::FindStrikeRetarget(PlayerTypes eExclude, const CvCity* pCurrent) const
{
	const CvCity* pCapital = m_pPlayer->getCapitalCity();
	if (!pCapital || !pCurrent)
		return NO_PLAYER;
	const TeamTypes eOurTeam = m_pPlayer->getTeam();
	const int iCurrentDefence = max(1, pCurrent->getStrengthValue() / 100);
	PlayerTypes eBest = NO_PLAYER;
	int iBestDefence = INT_MAX;
	for (int iLoop = 0; iLoop < MAX_CIV_PLAYERS; iLoop++)
	{
		const PlayerTypes eOther = (PlayerTypes)iLoop;
		CvPlayer& kOther = GET_PLAYER(eOther);
		if (eOther == eExclude || eOther == m_pPlayer->GetID() || !kOther.isAlive() || kOther.getTeam() == eOurTeam ||
			kOther.isBarbarian() || !GET_TEAM(eOurTeam).isHasMet(kOther.getTeam()))
			continue;
		// the per-turn target check's own definition (the lead, bullygreedy23 16DAE4CC: Singapore too strong -> re-target to
		// Sweden, which the BUILDUP check then rejected (IsStrikeTargetValid: a declared friend, allies, reach) -> back to the
		// nearest city-state Singapore, every turn T96-T106). A re-target is only to a target that check keeps
		if (!IsStrikeTargetValid(eOther))
			continue;
		const bool bAtWar = GET_TEAM(eOurTeam).isAtWar(kOther.getTeam());
		if (!bAtWar && !GET_TEAM(eOurTeam).canDeclareWar(kOther.getTeam(), m_pPlayer->GetID()))
			continue;
		if (kOther.isMajorCiv())
		{
			if (!bAtWar && !m_pPlayer->GetDiplomacyAI()->GetDefensiveWarAllies(eOther, /*bIncludeMinors*/ false, /*bReverseMode*/ true, /*bNewWarsOnly*/ true).empty())
				continue;
		}
		else if (kOther.isMinorCiv())
		{
			const PlayerTypes eAlly = kOther.GetMinorCivAI()->GetAlly();
			if (eAlly == m_pPlayer->GetID() || (eAlly != NO_PLAYER && !GET_TEAM(eOurTeam).isAtWar(GET_PLAYER(eAlly).getTeam())))
				continue;
			bool bProtected = false;
			for (int iMajor = 0; iMajor < MAX_MAJOR_CIVS && !bProtected; iMajor++)
			{
				const PlayerTypes eMajor = (PlayerTypes)iMajor;
				if (eMajor != m_pPlayer->GetID() && GET_PLAYER(eMajor).isAlive() && kOther.GetMinorCivAI()->IsProtectedByMajor(eMajor) &&
					!GET_TEAM(eOurTeam).isAtWar(GET_PLAYER(eMajor).getTeam()))
					bProtected = true;
			}
			if (bProtected)	// no peace-treaty turns (the user's no-turn rule): only what canDeclareWar itself forbids
				continue;
		}
		else
			continue;
		const CvCity* pCity = m_pPlayer->GetClosestCityToUsByPlots(eOther);
		if (!pCity || pCity == pCurrent || !pCity->plot()->isRevealed(eOurTeam) ||
			plotDistance(pCapital->getX(), pCapital->getY(), pCity->getX(), pCity->getY()) > 12)
			continue;
		const int iDefence = max(1, pCity->getStrengthValue() / 100);
		if (iDefence * 100 > iCurrentDefence * 75 || !IsStrikeArmyStrongEnough(pCity))
			continue;
		if (iDefence < iBestDefence)
		{
			iBestDefence = iDefence;
			eBest = eOther;
		}
	}
	return eBest;
}

/// the strike's target is still one it can go for (the lead's review: the block allows only the strike's own declaration, so
/// a stale target deadlocked the strike): alive and met, at war or one we can declare on, its nearest city revealed and within
/// 12 tiles of our capital and an attack target; a major with no defensive allies to pull in; a city-state with no ally (or
/// one we're at war with), no protector at peace with us and no recent peace. The attack-target check (a land or sea path
/// CP's military AI scores) only for a new target: it can drop out for a turn, and a target already held stays
bool CvMilitaryAI::IsStrikeTargetValid(PlayerTypes eTarget, bool bNewTarget) const
{
	// (the lead's review of cbebe77) a held target: the hold test, so the diplomacy's window opens on any held target
	if (!bNewTarget)
		return IsStrikeTargetHoldable(eTarget, GetStrikeState() == STRIKE_WAR);
	const CvCity* pCapital = m_pPlayer->getCapitalCity();
	if (eTarget == NO_PLAYER || !pCapital)
		return false;
	CvPlayer& kOther = GET_PLAYER(eTarget);
	const TeamTypes eOurTeam = m_pPlayer->getTeam();
	if (eTarget == m_pPlayer->GetID() || !kOther.isAlive() || kOther.isBarbarian() || kOther.getTeam() == eOurTeam ||
		!GET_TEAM(eOurTeam).isHasMet(kOther.getTeam()))
		return false;
	const bool bAtWar = GET_TEAM(eOurTeam).isAtWar(kOther.getTeam());
	if (!bAtWar && !GET_TEAM(eOurTeam).canDeclareWar(kOther.getTeam(), m_pPlayer->GetID()))
		return false;
	if (kOther.isMajorCiv())
	{
		if (!bAtWar && !m_pPlayer->GetDiplomacyAI()->GetDefensiveWarAllies(eTarget, /*bIncludeMinors*/ false, /*bReverseMode*/ true, /*bNewWarsOnly*/ true).empty())
			return false;
		// nor a declared friend (a backstab)
		if (!bAtWar && m_pPlayer->GetDiplomacyAI()->IsDoFAccepted(eTarget))
			return false;
	}
	else if (kOther.isMinorCiv())
	{
		if (!bAtWar)
		{
			const PlayerTypes eAlly = kOther.GetMinorCivAI()->GetAlly();
			if (eAlly == m_pPlayer->GetID() || (eAlly != NO_PLAYER && !GET_TEAM(eOurTeam).isAtWar(GET_PLAYER(eAlly).getTeam())))
				return false;
			for (int iMajor = 0; iMajor < MAX_MAJOR_CIVS; iMajor++)
			{
				const PlayerTypes eMajor = (PlayerTypes)iMajor;
				if (eMajor != m_pPlayer->GetID() && GET_PLAYER(eMajor).isAlive() && kOther.GetMinorCivAI()->IsProtectedByMajor(eMajor) &&
					!GET_TEAM(eOurTeam).isAtWar(GET_PLAYER(eMajor).getTeam()))
					return false;
			}
			// no peace-treaty turns here (the user's no-turn rule): only what canDeclareWar (above) itself forbids
		}
	}
	else
		return false;
	const CvCity* pCity = m_pPlayer->GetClosestCityToUsByPlots(eTarget);
	if (!pCity || !pCity->plot()->isRevealed(eOurTeam) || plotDistance(pCapital->getX(), pCapital->getY(), pCity->getX(), pCity->getY()) > 12)
		return false;
	// (the user: "if we take that, we can declare peace and let them have a minor settlement ... penalties for eliminating civs
	// completely"; the lead's review of 647fdd3: one done test) a major whose original capital is out of its hands is done
	if (IsStrikeTargetDone(eTarget))
		return false;
	// a new target is an attack target now (the declaration needs it); a held one, one a land army can reach (stable; the
	// lead's review of 647fdd3: the "no army path this turn" fallbacks re-pinned an unreachable target)
	// (the lead's review of 677f7a4) a new target needs both: an attack target now and its goal city reachable
	// (the lead's review of 88cf09c) reach gates only a new pick (a held one returned above)
	return HaveStrikeLandAttackTarget(eTarget) && IsStrikeTargetReachable(eTarget);
}

/// AI_AB_TEST (the user: no flip flopping; the lead's reviews of 88cf09c and cbebe77): a held target stays held while it's
/// alive, not on our team, not our vassal, fightable (at war with us, declarable, or a peace treaty's turns, which pass), and
/// not done (IsStrikeTargetDone: its capital out of its hands). No reach, distance, friendship or pact test once held (a pact
/// is fought: the take-the-city math counts the partners' units). The one release so no wait lasts for good: before the war,
/// no city of it on a landmass where we have a city
bool CvMilitaryAI::IsStrikeTargetHoldable(PlayerTypes eTarget, bool bAtStrikeWar) const
{
	if (eTarget == NO_PLAYER || eTarget == m_pPlayer->GetID())
		return false;
	const CvPlayer& kOther = GET_PLAYER(eTarget);
	const TeamTypes eOurTeam = m_pPlayer->getTeam();
	if (!kOther.isAlive() || kOther.isBarbarian() || kOther.getTeam() == eOurTeam || GET_TEAM(kOther.getTeam()).IsVassal(eOurTeam))
		return false;
	if (!GET_TEAM(eOurTeam).isAtWar(kOther.getTeam()) && !GET_TEAM(eOurTeam).canDeclareWar(kOther.getTeam(), m_pPlayer->GetID()) &&
		!GET_TEAM(eOurTeam).isForcePeace(kOther.getTeam()))
		return false;
	if (IsStrikeTargetDone(eTarget))
		return false;
	return bAtStrikeWar || IsStrikeTargetOnOurLand(eTarget);
}

/// AI_AB_TEST (the lead's check of 578de30): the right test for this target: the hold test for our held target, the new-pick
/// test (with reach) for any other
bool CvMilitaryAI::IsStrikeTargetValidForUs(PlayerTypes eTarget) const
{
	return IsStrikeTargetValid(eTarget, /*bNewTarget*/ eTarget != GetStrikeCityStateTarget());
}

/// the nearest major the strike can go for (IsStrikeTargetValid), by its nearest city's distance to our capital
PlayerTypes CvMilitaryAI::FindStrikeMajorInReach() const
{
	const CvCity* pCapital = m_pPlayer->getCapitalCity();
	if (!pCapital)
		return NO_PLAYER;
	PlayerTypes eBest = NO_PLAYER;
	int iBestDistance = INT_MAX;
	for (int iLoop = 0; iLoop < MAX_MAJOR_CIVS; iLoop++)
	{
		const PlayerTypes eOther = (PlayerTypes)iLoop;
		if (!IsStrikeTargetValid(eOther))
			continue;
		const CvCity* pCity = m_pPlayer->GetClosestCityToUsByPlots(eOther);
		const int iDistance = pCity ? plotDistance(pCapital->getX(), pCapital->getY(), pCity->getX(), pCity->getY()) : INT_MAX;
		if (iDistance < iBestDistance)
		{
			iBestDistance = iDistance;
			eBest = eOther;
		}
	}
	return eBest;
}

/// a unit class whose tech we have and that needs a resource we own none of (imports count). Not the free count: the army
/// spends its Horses or Iron as it is built (the lead's second review: IsStrikeArmyBuildable counts only free resources, so a
/// strike whose core uses them went OVER right before its declaration)
static bool StrikeClassLacksResource(const CvPlayer* pPlayer, UnitClassTypes eClass)
{
	const UnitTypes eUnit = (eClass != NO_UNITCLASS) ? pPlayer->GetSpecificUnitType(eClass) : NO_UNIT;
	CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
	if (!pkUnit)
		return false;
	const TechTypes eTech = (TechTypes)pkUnit->GetPrereqAndTech();
	if (eTech != NO_TECH && !GET_TEAM(pPlayer->getTeam()).GetTeamTechs()->HasTech(eTech))
		return false;	// not yet: its tech isn't in
	for (int iResource = 0; iResource < GC.getNumResourceInfos(); iResource++)
	{
		const ResourceTypes eResource = (ResourceTypes)iResource;
		const int iNeeded = max(pkUnit->GetResourceType() == iResource ? 1 : 0, pkUnit->GetResourceQuantityRequirement(iResource));
		if (iNeeded > 0 && pPlayer->getNumResourceTotal(eResource, /*bIncludeImport*/ true) < iNeeded)
			return true;
	}
	return false;
}

/// a unit class whose tech we have, that needs a resource, and whose resources we own but have in use (the army built from
/// them): canTrainUnit says no, the strike's army is still buildable (the lead's second review: the free count isn't the test)
static bool StrikeClassTrainableButForUse(const CvPlayer* pPlayer, UnitClassTypes eClass)
{
	const UnitTypes eUnit = (eClass != NO_UNITCLASS) ? pPlayer->GetSpecificUnitType(eClass) : NO_UNIT;
	CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
	if (!pkUnit)
		return false;
	const TechTypes eTech = (TechTypes)pkUnit->GetPrereqAndTech();
	if (eTech != NO_TECH && !GET_TEAM(pPlayer->getTeam()).GetTeamTechs()->HasTech(eTech))
		return false;
	bool bNeedsResource = false;
	for (int iResource = 0; iResource < GC.getNumResourceInfos(); iResource++)
	{
		const int iNeeded = max(pkUnit->GetResourceType() == iResource ? 1 : 0, pkUnit->GetResourceQuantityRequirement(iResource));
		if (iNeeded > 0)
		{
			bNeedsResource = true;
			if (pPlayer->getNumResourceTotal((ResourceTypes)iResource, /*bIncludeImport*/ true) < iNeeded)
				return false;
		}
	}
	return bNeedsResource;
}

/// the strike's line for a class (the lead's review of c7eed909: the whole upgrade line, Archer -> Composite Bowman -> Crossbow
/// -> ..., is never obsolete, so the exit never fired): the class, then up its upgrade line to the plan's upgrade, the class
/// whose unit needs the plan's upgrade tech (Assyria: Archer, Composite Bowman at Construction; the rule IsStrikeArmyReady's
/// upgrade bill uses). No upgrade tech, or no class on the line needing it: the class alone
static void GetStrikeLine(const CvPlayer* pPlayer, UnitClassTypes eClass, std::vector<UnitClassTypes>& vLine)
{
	vLine.clear();
	if (eClass == NO_UNITCLASS)
		return;
	vLine.push_back(eClass);
	const TechTypes eUpgradeTech = pPlayer->GetGrandStrategyAI() ? pPlayer->GetGrandStrategyAI()->GetStrikePlan().eUpgradeTech : NO_TECH;
	if (eUpgradeTech == NO_TECH)
		return;
	// breadth first, with each class's parent, to the first class whose unit needs the upgrade tech
	std::vector<UnitClassTypes> vSeen(1, eClass);
	std::vector<int> vParent(1, -1);
	for (size_t i = 0; i < vSeen.size() && vSeen.size() <= 32; i++)
	{
		const UnitTypes eUnit = pPlayer->GetSpecificUnitType(vSeen[i]);
		CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
		if (!pkUnit)
			continue;
		if ((TechTypes)pkUnit->GetPrereqAndTech() == eUpgradeTech)
		{
			vLine.clear();
			for (int j = (int)i; j >= 0; j = vParent[j])
				vLine.insert(vLine.begin(), vSeen[j]);
			return;
		}
		for (int iClass = 0; iClass < GC.getNumUnitClassInfos(); iClass++)
		{
			if (pkUnit->GetUpgradeUnitClass(iClass) && std::find(vSeen.begin(), vSeen.end(), (UnitClassTypes)iClass) == vSeen.end())
			{
				vSeen.push_back((UnitClassTypes)iClass);
				vParent.push_back((int)i);
			}
		}
	}
}

/// every unit on the core's strike line (GetStrikeLine: the core up to the plan's upgrade) is obsolete for our team. A class
/// with no unit for this civ counts as obsolete. NO_UNITCLASS: false (no core, no exit here)
bool CvMilitaryAI::IsStrikeCoreLineObsolete(UnitClassTypes eCoreClass) const
{
	if (eCoreClass == NO_UNITCLASS)
		return false;
	std::vector<UnitClassTypes> vLine;
	GetStrikeLine(m_pPlayer, eCoreClass, vLine);
	for (size_t i = 0; i < vLine.size(); i++)
	{
		const UnitTypes eUnit = m_pPlayer->GetSpecificUnitType(vLine[i]);
		CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
		if (!pkUnit)
			continue;
		const TechTypes eObsolete = (TechTypes)pkUnit->GetObsoleteTech();
		if (eObsolete == NO_TECH || !GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech(eObsolete))
			return false;	// this one isn't obsolete: the line goes on
	}
	return true;
}

/// the capital can build an income item: a Market or a Caravan (Economy 0054's income-first items under the unit hold; 0057
/// made income-first the capital's only, so the lead's review: test the capital only)
bool CvMilitaryAI::CanBuildStrikeIncomeItem() const
{
	const BuildingTypes eMarket = m_pPlayer->GetSpecificBuildingType("BUILDINGCLASS_MARKET", true);
	const UnitTypes eCaravan = m_pPlayer->GetSpecificUnitType("UNITCLASS_CARAVAN", true);
	CvCity* pCapital = m_pPlayer->getCapitalCity();
	if (!pCapital)
		return false;
	return (eMarket != NO_BUILDING && pCapital->canConstruct(eMarket)) || (eCaravan != NO_UNIT && pCapital->canTrain(eCaravan));
}

/// the lead's review: an early civ whose core can't be trained (a resource missing) sat in PREP for good, every declaration
/// blocked. Its tech is in and we own none of a resource it needs; the same for the plan's pre-build, if any
bool CvMilitaryAI::IsStrikeCoreUntrainable() const
{
	if (!m_pPlayer->GetGrandStrategyAI())
		return false;
	const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
	const UnitClassTypes eCoreClass = IsEarlyStrikeCiv() ? GetEarlyStrikeCore() : kPlan.eCoreClass;
	if (eCoreClass == NO_UNITCLASS)
		return false;
	return StrikeClassLacksResource(m_pPlayer, eCoreClass) && (kPlan.ePreBuildClass == NO_UNITCLASS || StrikeClassLacksResource(m_pPlayer, kPlan.ePreBuildClass));
}

/// AI_AB_TEST (Economy 0052): a declaration of ours. On a major it counts at once (CP doesn't undo those). On a city-state it
/// waits for the next strike update, and counts if the war still stands then. An earlier confirmed declaration is kept
void CvMilitaryAI::SetStrikeDeclared(PlayerTypes eTarget)
{
	if (m_iStrikeDeclared == 1)
		return;
	if (eTarget != NO_PLAYER && eTarget >= 0 && eTarget < MAX_PLAYERS && GET_PLAYER(eTarget).isMinorCiv())
		m_iStrikeDeclared = 2 + (int)eTarget;
	else
		m_iStrikeDeclared = 1;
}

void CvMilitaryAI::UpdateStrikeState()
{
	const bool bEarly = IsEarlyStrikeCiv();
	const bool bLate = !bEarly && IsLateStrikeCiv();
	const StrikeStateTypes eOld = GetStrikeState();
	// Economy's request: an exact "we have declared" (its wonder and gift bans hold until then, also after an OVER). Our own
	// declarations on majors (the game's counter), and every declaration of ours through CvDiplomacyAI::DeclareWar
	// (SetStrikeDeclared). Not a city-state war as such: allied city-states declare on us with their major (the lead's review)
	if (m_iStrikeDeclared != 1 && GC.getGame().GetABTestCount(m_pPlayer->GetID(), AB_COUNT_WARS_DECLARED) > 0)
		m_iStrikeDeclared = 1;
	// (Economy 0052; the user: "a civ that never fights stays banned from wonders and gifts all game") a declaration on a
	// city-state counts only if the war still stands now. bullygreedy23 16DAE4CC (ab-0cb5402): the Aztecs "declared" on
	// Singapore every turn T48-T71, CP's city-state peace undid each one (fixed in 5fe900a1), the flag was set anyway, and
	// after the OVER at T106 the bans lifted: four wonders and city-state gold with no war fought all game
	if (m_iStrikeDeclared >= 2)
	{
		const PlayerTypes eDeclared = (PlayerTypes)(m_iStrikeDeclared - 2);
		const bool bStands = eDeclared >= 0 && eDeclared < MAX_PLAYERS && GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eDeclared).getTeam());
		m_iStrikeDeclared = bStands ? 1 : 0;
		if (GC.getLogging() && GC.getAILogging() && eDeclared >= 0 && eDeclared < MAX_PLAYERS)
			m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: declaration on the city-state %s %s",
				GET_PLAYER(eDeclared).getCivilizationShortDescription(), bStands ? "confirmed (the war stands)" : "not counted (no war a turn later)"));
	}
	if (!bEarly && !bLate)
	{
		if (eOld != STRIKE_OVER)
			m_iStrikeState = STRIKE_NONE;
		return;
	}
	if (eOld == STRIKE_OVER)
		return;

	const CvGame& kGame = GC.getGame();
	const int iWarsDeclared = kGame.GetABTestCount(m_pPlayer->GetID(), AB_COUNT_WARS_DECLARED);
	const int iWarsOnUs = kGame.GetABTestCount(m_pPlayer->GetID(), AB_COUNT_WARS_ON_US);
	StrikeStateTypes eNew = eOld;
	if (eOld == STRIKE_NONE)
	{
		eNew = STRIKE_PREP;
		m_iStrikeWarsDeclaredMark = iWarsDeclared;
		m_iStrikeWarsOnUsMark = iWarsOnUs;
	}
	CvString strWhy = (eOld == STRIKE_NONE) ? "a strike civ" : "";

	const CvCity* pCapital = m_pPlayer->getCapitalCity();
	PlayerTypes eTarget = m_pPlayer->GetDiplomacyAI()->GetCommittedConquestTarget();
	const CvCity* pTargetCity = (eTarget != NO_PLAYER) ? m_pPlayer->GetClosestCityToUsByPlots(eTarget) : NULL;
	bool bInReach = pTargetCity && pCapital && plotDistance(pCapital->getX(), pCapital->getY(), pTargetCity->getX(), pTargetCity->getY()) <= 12;
	// EarlyStrike with no committed major in reach: a pinned target (the user: a city-state when no major is in reach). In PREP
	// the same targets, in the same order, as BUILDUP's check below (the lead's trial23 164B7CEA: Persia's city-state target
	// Sydney stopped qualifying, was dropped silently, and PREP ended "no target in reach" at T45 with Denmark, a major in
	// reach, picked by CP the next turn): the pinned target while it stays valid (IsStrikeTargetValid), else the nearest
	// major in reach, else the nearest city-state. Dropped when CP commits to a major in reach. Through BUILDUP and WAR it is
	// kept while alive (BUILDUP's own check re-validates it)
	const PlayerTypes eOldCS = GetStrikeCityStateTarget();
	PlayerTypes eCS = NO_PLAYER;
	// AI_AB_TEST (the user: "no, no flip flopping. aggressive civs choose targets and then eliminate them. completely."): a
	// target once picked is held while it's alive and not on our team. No re-pick, no re-target, no giving way to a major
	// A held target is released only when it can never be fought (the lead's review: a wait must not hold the bans and
	// production for good): not at war with it and no declaration possible for a reason that doesn't pass by itself. A peace
	// treaty's turns pass (isForcePeace): the strike waits. A test builder's role, a master we can't fight, passive modes: dropped
	// (the lead's reviews of 88cf09c and cbebe77) one hold test, IsStrikeTargetHoldable, shared with IsStrikeTargetValid for a
	// held target (the diplomacy's window). No reach test once held
	const bool bOldHoldable = IsStrikeTargetHoldable(eOldCS, eNew == STRIKE_WAR);
	if (bOldHoldable && (eNew == STRIKE_PREP || eNew == STRIKE_BUILDUP || eNew == STRIKE_WAR))
		eCS = eOldCS;
	// the first pick in PREP: CP's committed major in reach is held as soon as it's valid
	else if (bEarly && eNew == STRIKE_PREP && bInReach && IsStrikeTargetValid(eTarget))
		eCS = eTarget;
	else if (bEarly && (eNew == STRIKE_PREP || eNew == STRIKE_BUILDUP || eNew == STRIKE_WAR))
	{
		// (the lead's review of 12fe132) BUILDUP and WAR keep only a holdable target (above): one that can never be fought is
		// released here, not kept because it's alive
		if (eNew != STRIKE_PREP)
		{
		}
		// (also while CP's committed target is the pinned major itself: the pin made it the committed target, and dropping it
		// let CP's own loop reject it the next turn, a flip every turn; the lead's third review)
		else if (!bInReach || (eOldCS != NO_PLAYER && eOldCS == eTarget))
		{
			// a city-state pin gives way to a major that comes in reach (the user: a city-state when no major is in reach; the
			// lead's review: CP's committed target is cleared under a city-state pin, so the "committed major in reach" drop
			// never fired)
			const bool bMajorNow = eOldCS != NO_PLAYER && GET_PLAYER(eOldCS).isMinorCiv() && FindStrikeMajorInReach() != NO_PLAYER;
			// (the strike's reach, 17A1A659) the pin is kept while an army can reach it; unreachable, a reachable target takes
			// over, and with none the pin stays (a turn out of the attack targets doesn't re-target)
			if (!bMajorNow && bOldHoldable && IsStrikeTargetValid(eOldCS, /*bNewTarget*/ true))
				eCS = eOldCS;
			else
			{
				eCS = FindStrikeMajorInReach();
				if (eCS == NO_PLAYER)
					eCS = FindStrikeCityState();
				if (eCS == NO_PLAYER && !bMajorNow && bOldHoldable && IsStrikeTargetValid(eOldCS, /*bNewTarget*/ false))
					eCS = eOldCS;
			}
		}
	}
	// LateStrike: a pinned target (a re-target, or the BUILDUP target check below) holds through BUILDUP and WAR
	else if (bLate && (eNew == STRIKE_BUILDUP || eNew == STRIKE_WAR) && bOldHoldable)
		eCS = eOldCS;
	if (eCS != eOldCS)
	{
		m_iStrikeCityState = eCS;
		// a released target leaves the window too, so WAR sees no target and goes back to BUILDUP for the next one
		if (eOldCS != NO_PLAYER && eCS == NO_PLAYER && m_eTimingBuildupTarget == eOldCS)
			m_eTimingBuildupTarget = NO_PLAYER;
		if (GC.getLogging() && GC.getAILogging())
		{
			if (eCS != NO_PLAYER)
			{
				const CvCity* pCSCity = m_pPlayer->GetClosestCityToUsByPlots(eCS);
				m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: %s target %s (%s, %d tiles from our capital; no committed major in reach)",
					GET_PLAYER(eCS).isMinorCiv() ? "city-state" : "major", GET_PLAYER(eCS).getCivilizationShortDescription(), pCSCity ? pCSCity->getNameNoSpace().c_str() : "no city",
					(pCSCity && pCapital) ? plotDistance(pCapital->getX(), pCapital->getY(), pCSCity->getX(), pCSCity->getY()) : -1));
			}
			else
				m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: target %s dropped (%s)", GET_PLAYER(eOldCS).getCivilizationShortDescription(),
					!GET_PLAYER(eOldCS).isAlive() ? "dead" : !bOldHoldable ? "it can never be fought: our vassal, no declaration possible, its capital out of its hands, or no army path" :
					bInReach ? "a committed major in reach" : "no longer qualifies, no other target in reach"));
		}
	}
	if (eCS != NO_PLAYER)
	{
		eTarget = eCS;
		pTargetCity = m_pPlayer->GetClosestCityToUsByPlots(eCS);
		// (the lead's review of cbebe77) a held target is in reach: it was picked with reach, and the 12 tiles were checked then
		bInReach = pTargetCity && pCapital && (eCS == eOldCS || plotDistance(pCapital->getX(), pCapital->getY(), pTargetCity->getX(), pTargetCity->getY()) <= 12);
	}
	// the strike's target, checked each BUILDUP turn (the lead's review: before the strike only the strike's own declaration is
	// allowed, so a stale target was a deadlock: a defensive pact, CP's committed target changing under an open window, the
	// target killed; a late civ with no major in reach never opened a window). The first valid one (IsStrikeTargetValid) of:
	// the window's target, the current one, CP's committed target, the nearest major in reach, the nearest city-state in reach.
	// A change is pinned (m_iStrikeCityState; the diplomacy takes a major there as its target) and the window follows. None:
	// OVER. Game state only
	if (eNew == STRIKE_BUILDUP)
	{
		PlayerTypes eWindow = (m_iTimingBuildupStartTurn >= 0) ? m_eTimingBuildupTarget : NO_PLAYER;
		// (the lead's review of 647fdd3) CP's window on a target the strike can't take (a finished major's remnant): closed, so
		// the declaration block and the readiness don't measure against it. A held target reopens it (the diplomacy)
		if (eWindow != NO_PLAYER && eWindow != eCS && !IsStrikeTargetValidForUs(eWindow))	// (the lead: not held, so the new-pick test)
		{
			m_eTimingBuildupTarget = NO_PLAYER;
			m_iTimingBuildupStartTurn = -1;
			eWindow = NO_PLAYER;
		}
		const PlayerTypes eCommitted = m_pPlayer->GetDiplomacyAI()->GetCommittedConquestTarget();
		PlayerTypes eKeep = NO_PLAYER;
		const char* szHow = "";
		// (the strike's reach, 17A1A659) a held target first while an army can reach it; then the new ones (all reachable); a held
		// target out of reach only when nothing reachable is left (a turn out of the attack targets doesn't re-target)
		// (the user: no flip flopping) the held target, while alive
		if (eCS != NO_PLAYER)
			eKeep = eCS, szHow = "the held target";
		else if (IsStrikeTargetValid(eWindow, /*bNewTarget*/ true))
			eKeep = eWindow, szHow = "the build-up's target";
		else if (IsStrikeTargetValid(eTarget, /*bNewTarget*/ true))
			eKeep = eTarget, szHow = "the current target";
		else if (IsStrikeTargetValid(eCommitted))
			eKeep = eCommitted, szHow = "the committed target";
		else if ((eKeep = FindStrikeMajorInReach()) != NO_PLAYER)
			szHow = "the nearest major in reach";
		// (the lead's review of cbebe77) a new pick, with reach; no held-target fallbacks here (a held target is eCS, above)
		else if ((eKeep = FindStrikeCityState()) != NO_PLAYER && IsStrikeTargetValid(eKeep, /*bNewTarget*/ true))
			szHow = "the nearest city-state in reach";
		else
			eKeep = NO_PLAYER;
		// no target in reach: wait in BUILDUP and pick one when it comes (178B8F13: the Huns went OVER at T40, Jakarta came in
		// reach at T42; the user: aggressive civs choose targets). Not an exit
		if (eKeep == NO_PLAYER)
		{
			eTarget = NO_PLAYER;
			pTargetCity = NULL;
			bInReach = false;
		}
		else if (eKeep != eTarget || (eWindow != NO_PLAYER && eKeep != eWindow) || GetStrikeCityStateTarget() != eKeep)
		{
			if (GC.getLogging() && GC.getAILogging())
				m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: target %s -> %s (%s; the build-up's target %s; %s)",
					eTarget != NO_PLAYER ? GET_PLAYER(eTarget).getCivilizationShortDescription() : "none", GET_PLAYER(eKeep).getCivilizationShortDescription(), szHow,
					eWindow != NO_PLAYER ? GET_PLAYER(eWindow).getCivilizationShortDescription() : "none", bEarly ? "EarlyStrike" : "LateStrike"));
			m_iStrikeCityState = eKeep;	// held from now on (the user: no flip flopping)
			if (eWindow != NO_PLAYER)
				m_eTimingBuildupTarget = eKeep;
			// a new city to take: its siege and takeable answers start over (the window's own target only pinned: kept)
			if (eWindow == NO_PLAYER || eKeep != eWindow)
			{
				m_iStrikeSiege = 0;
			}
			eCS = eKeep;
			eTarget = eKeep;
			pTargetCity = m_pPlayer->GetClosestCityToUsByPlots(eKeep);
			bInReach = pTargetCity && pCapital && plotDistance(pCapital->getX(), pCapital->getY(), pTargetCity->getX(), pTargetCity->getY()) <= 12;
		}
		// at war with a third party: the target is pinned, so the diplomacy (CvDiplomacyAI::GetNewAIConquestTarget, which looks
		// for no new target while we're at war) still opens its window and declares on it
		if (eKeep != NO_PLAYER && m_pPlayer->IsAtWarAnyMajor() && GetStrikeCityStateTarget() != eKeep)
		{
			m_iStrikeCityState = eKeep;
			eCS = eKeep;
		}
	}
	RefreshStrikeSnapshot();	// the target is settled for this turn: the exits and readiness below read it
	// (the lead's review of ae8e9ae) the siege the build-up brings (GetTimingBuildupSiegeTarget, and its tech first in research)
	// also when the plan's army looks strong enough but the units we have, its counts met, can't take the target: the city's
	// damage comes from ranged and siege units, and with no ranged line and no siege tech the build-up would train only melee
	if (eNew == STRIKE_BUILDUP && m_iStrikeSiege == 0 && !m_bTakeable && m_pPlayer->GetGrandStrategyAI()->IsStrikeArmyReady())
	{
		const PlayerTypes eSiegeFor = (m_eTimingBuildupTarget != NO_PLAYER) ? m_eTimingBuildupTarget : GetStrikeCityStateTarget();
		if (eSiegeFor != NO_PLAYER && HaveStrikeLandAttackTarget(eSiegeFor))
		{
			m_iStrikeSiege = 1;
			if (GC.getLogging() && GC.getAILogging())
				m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: the plan's counts are met and our army can't take %s's city; the build-up brings siege (%s)",
					GET_PLAYER(eSiegeFor).getCivilizationShortDescription(), bEarly ? "EarlyStrike" : "LateStrike"));
		}
	}
	const bool bAtCSWar = eCS != NO_PLAYER && GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eCS).getTeam());
	const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
	const UnitClassTypes eCoreClass = bEarly ? GetEarlyStrikeCore() : kPlan.eCoreClass;
	const UnitTypes eCore = (eCoreClass != NO_UNITCLASS) ? m_pPlayer->GetSpecificUnitType(eCoreClass) : NO_UNIT;
	CvUnitEntry* pkCore = (eCore != NO_UNIT) ? GC.getUnitInfo(eCore) : NULL;

	if (eNew == STRIKE_PREP || eNew == STRIKE_BUILDUP)
	{
		// the core's whole upgrade line, not the core alone (the lead, bullygreedy24 1762140E: Assyria's late strike, core Archer,
		// upgrade Construction, went OVER at T61 because Construction obsoletes the Archer, which is the plan: upgrade and go to
		// Composite Bowmen; the same rule as Economy 0047's list). OVER only once every unit on the line is obsolete
		if (pkCore && IsStrikeCoreLineObsolete(eCoreClass))
			eNew = STRIKE_OVER, strWhy = "every unit on the core's upgrade line is obsolete";
		// the core can't be trained though its tech is in (a resource missing; the lead's review: an early civ sat in PREP for
		// good with every declaration blocked)
		// a BUILDUP that can't recover (the lead's review: with the unit hold on, core, siege and short-of-plan units all wait,
		// so the army never becomes ready): no unit affordable, the army not ready and not as ready as it gets (takeable but the
		// upgrade bill unpaid or the field short counts too; the lead's next review), income negative, and the capital can't
		// build an income item (a Market, a Caravan). The strike ends and CP's economy and disbanding resume
		// (STOP 18FA5DFB) not while the target is takeable and home is safe: a stalled build-up with a takeable target is as
		// ready as it gets (IsStrikeAsReadyAsItGets) and declares
		// (the lead's reviews of ae8e9ae and 6bf8952) no guard of its own: !IsStrikeAsReadyAsItGets already keeps a stalled build-up
		// whose target is takeable, home safe and the formation's required slots filled. Takeable with a slot it can't fill or a
		// city in danger, the strike ends (no indefinite wait with the bans and the K-city cap held)
		else if (eNew == STRIKE_BUILDUP && m_pPlayer->IsStrikeUnitHold() && !IsTimingBuildupArmyReady() && !IsStrikeAsReadyAsItGets() &&
			m_pPlayer->GetTreasury()->CalculateBaseNetGold() <= 0 && !CanBuildStrikeIncomeItem())
			eNew = STRIKE_OVER, strWhy = CvString::format("stalled: no unit affordable, the army not ready, income negative and no income item to build (short: %s)",
				GetStrikeReadinessGaps().c_str());
		else if (IsStrikeCoreUntrainable())
			eNew = STRIKE_OVER, strWhy = "the core can't be trained (its tech is in; a resource missing)";
		// a major declared war on us (the lead: not an exit by itself). The strike's own target during BUILDUP: that is the
		// strike's war (no retreat). Anyone else, or in PREP: the strike goes on; its declaration waits while one of our cities
		// is in danger of falling (IsStrikeHomeSafe)
		else if (iWarsOnUs > m_iStrikeWarsOnUsMark)
		{
			m_iStrikeWarsOnUsMark = iWarsOnUs;
			if (eNew == STRIKE_BUILDUP && eTarget != NO_PLAYER && GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eTarget).getTeam()))
			{
				eNew = STRIKE_WAR;
				strWhy = "the target declared war on us: the strike's war";
				m_iStrikeCapturedMark = GetStrikeTargetCaptures();	// after the pin (the lead's review: iCaptured is from before it)
				m_iStrikeLostMark = kGame.GetABTestCount(m_pPlayer->GetID(), AB_COUNT_CITIES_LOST);
				m_iStrikeCitiesAtWar = m_pPlayer->getNumCities();
				m_iStrikeArmyAtWar = m_pPlayer->GetNumUnitsWithDomain(DOMAIN_LAND, true);
			}
			else if (GC.getLogging() && GC.getAILogging())
				m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: a major declared war on us; the strike goes on (%s, %s)",
					eNew == STRIKE_PREP ? "PREP" : "BUILDUP", bEarly ? "EarlyStrike" : "LateStrike"));
		}
		// a war of our own before the strike's build-up (CP's war approach, not the strike): the strike is pre-empted and the
		// normal plan runs (Japan, 1468D9F4: LateStrike in PREP from T1 declared on Spain at T102, BUILDUP came at T149)
		else if (eNew == STRIKE_PREP && iWarsDeclared > m_iStrikeWarsDeclaredMark)
			eNew = STRIKE_OVER, strWhy = "we declared a war outside the strike, before its build-up";
		// walls are not an exit (the user: "i think i brute forced through marrakeshs walls"): a strength check. Strong enough,
		// go, walls or not; too weak, a clearly easier city in reach (a major's or a city-state's) is the new target, else the
		// build-up brings siege. Never OVER for this
		else if (eNew == STRIKE_BUILDUP && pTargetCity && m_iStrikeSiege == 0 && !IsStrikeArmyStrongEnough(pTargetCity))
		{
			// (the user: no flip flopping) no re-target to an easier city: the build-up brings siege
			const PlayerTypes eRetarget = NO_PLAYER;
			if (eRetarget != NO_PLAYER)
			{
				const CvCity* pNewCity = m_pPlayer->GetClosestCityToUsByPlots(eRetarget);
				if (GC.getLogging() && GC.getAILogging())
					m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: re-target from %s (%s, defence %d, too strong for the plan's army) to %s (%s, defence %d, %d tiles from our capital; %s)",
						eTarget != NO_PLAYER ? GET_PLAYER(eTarget).getCivilizationShortDescription() : "none", pTargetCity->getNameNoSpace().c_str(), pTargetCity->getStrengthValue() / 100,
						GET_PLAYER(eRetarget).getCivilizationShortDescription(), pNewCity ? pNewCity->getNameNoSpace().c_str() : "-", pNewCity ? pNewCity->getStrengthValue() / 100 : -1,
						(pNewCity && pCapital) ? plotDistance(pCapital->getX(), pCapital->getY(), pNewCity->getX(), pNewCity->getY()) : -1,
						bEarly ? "EarlyStrike" : "LateStrike"));
				m_iStrikeCityState = eRetarget;
				m_eTimingBuildupTarget = eRetarget;
			}
			else
			{
				m_iStrikeSiege = 1;
				if (GC.getLogging() && GC.getAILogging())
					m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: the plan's army can't take %s (defence %d; the damage math), no easier city in reach; the build-up brings siege (%s)",
						pTargetCity->getNameNoSpace().c_str(), pTargetCity->getStrengthValue() / 100, bEarly ? "EarlyStrike" : "LateStrike"));
			}
		}
		else if (eNew == STRIKE_BUILDUP && pTargetCity && pkCore && pkCore->GetUnitCombatType() != NO_UNITCOMBAT)
		{
			int iCounters = 0, iLoop = 0;
			for (const CvUnit* pLoopUnit = GET_PLAYER(eTarget).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(eTarget).nextUnit(&iLoop))
			{
				if (pLoopUnit->getDomainType() == DOMAIN_LAND && pLoopUnit->IsCombatUnit() && pLoopUnit->unitCombatModifier((UnitCombatTypes)pkCore->GetUnitCombatType()) > 0 &&
					plotDistance(pLoopUnit->getX(), pLoopUnit->getY(), pTargetCity->getX(), pTargetCity->getY()) <= 8)
					iCounters++;
			}
			// (the user: no flip flopping) the target's counter units are no exit: the target is held
			(void)iCounters;
		}
		// Personality's plan exits (China's from the research): the target city too strong, the plan's counter units seen near
		// it, or the city on a hill
		if (eNew == STRIKE_BUILDUP && pTargetCity && kPlan.bValid)
		{
			int iPlanCounters = 0, iLoop = 0;
			if (kPlan.eExitCounterClass1 != NO_UNITCLASS || kPlan.eExitCounterClass2 != NO_UNITCLASS)
			{
				for (const CvUnit* pLoopUnit = GET_PLAYER(eTarget).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(eTarget).nextUnit(&iLoop))
				{
					const UnitClassTypes eClass = pLoopUnit->getUnitClassType();
					if ((eClass == kPlan.eExitCounterClass1 || eClass == kPlan.eExitCounterClass2) &&
						plotDistance(pLoopUnit->getX(), pLoopUnit->getY(), pTargetCity->getX(), pTargetCity->getY()) <= 8)
						iPlanCounters++;
				}
			}
			// (the user: no flip flopping) the plan's target exits (city strength, counter units, a hill) are no exit: the target is
			// held, and the build-up brings siege and more core (Economy 0053)
			(void)iPlanCounters;
		}
		// (the user: aggressive civs choose targets) no "army built, no target in reach" exit: PREP waits for a target
	}

	// a strike's build-up: why it isn't ready yet, logged when the blockers change (the lead: a long BUILDUP must say what held
	// it); and a city-state target is declared on directly once the army is ready or as ready as it gets
	if (eNew == STRIKE_BUILDUP && m_iTimingBuildupStartTurn >= 0)
	{
		const bool bReady = IsTimingBuildupArmyReady();
		const bool bAsReady = IsStrikeAsReadyAsItGets();
		if (!bReady && !bAsReady && GC.getLogging() && GC.getAILogging())
		{
			const int iSiege = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true);
			const int iField = TimingBuildupFieldUnits(m_pPlayer, this, iSiege);
			int iPlanBill = 0;
			const bool bPlanArmy = m_pPlayer->GetGrandStrategyAI()->IsStrikeArmyReady(&iPlanBill);
			const int iBill = GetEarlyStrikeUpgradeBill();
			const int iMelee = GetEarlyStrikeMeleeUnits();
			const int iAffordable = m_pPlayer->GetEconomicAI()->GetAffordableWarUnits(20);
			const bool bBuilt = IsTimingBuildupArmyBuilt();
			const int iKey = 1 + iSiege + 8 * GetTimingBuildupSiegeTarget() + 64 * iField + 4096 * GetTimingBuildupFieldTarget() + (bPlanArmy ? 1 << 20 : 0) +
				(iBill > 0 ? 1 << 21 : 0) + (iMelee >= EARLY_STRIKE_MELEE_MIN ? 1 << 22 : 0) + (iAffordable > 0 ? 1 << 23 : 0) + (bBuilt ? 1 << 24 : 0);
			if (iKey != m_iStrikeNotReadyKey)
			{
				m_iStrikeNotReadyKey = iKey;
				const CvStrikePlan& kStrikePlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
				int iTurnsToTake = -1;
				const bool bTakeable = IsStrikeArmyStrongEnough(pTargetCity, &iTurnsToTake);
				m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: not ready (target %s defence %d: the plan's army %s%s; normal army %s: field %d/%d, siege %d/%d; plan army %s: core line %d/%d, support line %d/%d, upgrade tech %s, plan bill %d; upgrade bill %d (gold %d); melee %d/%d; affordable %d more)",
					pTargetCity ? pTargetCity->getNameNoSpace().c_str() : "-", pTargetCity ? pTargetCity->getStrengthValue() / 100 : -1,
					bTakeable ? "takes it" : "can't take it", (bTakeable && iTurnsToTake > 0) ? CvString::format(" in about %d turns", iTurnsToTake).c_str() : "",
					bBuilt ? "built" : "short", iField, GetTimingBuildupFieldTarget(), iSiege, GetTimingBuildupSiegeTarget(), bPlanArmy ? "ready" : "short",
					m_pPlayer->GetGrandStrategyAI()->CountStrikeClassLine(kStrikePlan.eCoreClass), kStrikePlan.iCoreTarget,
					m_pPlayer->GetGrandStrategyAI()->CountStrikeClassLine(kStrikePlan.eSupportClass), kStrikePlan.iSupportTarget,
					(kStrikePlan.eUpgradeTech == NO_TECH || GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech(kStrikePlan.eUpgradeTech)) ? "in" : "not in",
					iPlanBill, iBill, m_pPlayer->GetTreasury()->GetGold(), iMelee, (int)EARLY_STRIKE_MELEE_MIN, iAffordable));
			}
		}
		if (eCS != NO_PLAYER && GET_PLAYER(eCS).isMinorCiv() && !bAtCSWar && (bReady || bAsReady) &&
			GET_TEAM(m_pPlayer->getTeam()).canDeclareWar(GET_PLAYER(eCS).getTeam(), m_pPlayer->GetID()))
		{
			if (GC.getLogging() && GC.getAILogging())
				m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: declaring war on the city-state %s (%s)",
					GET_PLAYER(eCS).getCivilizationShortDescription(), bReady ? "the army ready" : "as ready as it gets"));
			const bool bDeclared = m_pPlayer->GetDiplomacyAI()->DeclareWar(eCS);
			if (bDeclared)
				SetStrikeDeclared(eCS);	// (0052: pending until the war still stands at the next update)
			// the outcome (the lead: 22 "declaring" lines and no war in 167DE984)
			if (GC.getLogging() && GC.getAILogging())
				m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: declaration on the city-state %s %s", GET_PLAYER(eCS).getCivilizationShortDescription(),
					bDeclared && GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eCS).getTeam()) ? "made: at war" :
					!GET_TEAM(m_pPlayer->getTeam()).canDeclareWar(GET_PLAYER(eCS).getTeam(), m_pPlayer->GetID()) ? "failed: canDeclareWar false" :
					IsStrikeDeclarationBlocked(eCS) ? "failed: the strike block" : "failed: DeclareWar refused"));
		}
	}

	if (eNew == STRIKE_PREP)
	{
		// Personality's economy-ready (CvGrandStrategyAI::IsStrikeEconomyReady: early, the core trainable, N core units out and
		// K cities; late, the upgrade tech in reach, K cities and half the upgrade bill saved); EarlyStrike also wants a target
		// in reach that its plan can take
		const bool bEconomyReady = m_pPlayer->GetGrandStrategyAI()->IsStrikeEconomyReady();
		const bool bTrigger = bEconomyReady && (bEarly ? (bInReach && IsStrikeArmyBuildable()) : true);
		if (bTrigger)
		{
			eNew = STRIKE_BUILDUP;
			strWhy = bEarly ? CvString::format("economy-ready, target %s in reach, core trainable", pTargetCity->getNameNoSpace().c_str()) : CvString("economy-ready");
			m_iStrikeWarsDeclaredMark = iWarsDeclared;
			m_iStrikeWarsOnUsMark = iWarsOnUs;
		}
	}
	else if (eNew == STRIKE_BUILDUP && ((m_pPlayer->IsAtWarAnyMajor() && iWarsDeclared > m_iStrikeWarsDeclaredMark) || bAtCSWar))
	{
		eNew = STRIKE_WAR;
		strWhy = IsTimingBuildupArmyReady() ? "declared, the army ready" : "declared, as ready as it got";
		m_iStrikeCapturedMark = GetStrikeTargetCaptures();	// after the pin (the lead's review: iCaptured is from before it)
		m_iStrikeLostMark = kGame.GetABTestCount(m_pPlayer->GetID(), AB_COUNT_CITIES_LOST);
		m_iStrikeCitiesAtWar = m_pPlayer->getNumCities();
		m_iStrikeArmyAtWar = m_pPlayer->GetNumUnitsWithDomain(DOMAIN_LAND, true);
		// (HasStrikeDeclared: set by our own declaration, CvDiplomacyAI::DeclareWar; not here, a city-state's war on us also
		// lands here)
	}
	else if (eNew == STRIKE_WAR)
	{
		// the strike's war ends with its purpose (the lead's review: it outlived it, and its ownership of production, gold
		// and research with it): a city taken (the war itself goes on, CP's), or peace with the target or the target gone
		const PlayerTypes eWarTarget = (eCS != NO_PLAYER) ? eCS : m_eTimingBuildupTarget;
		// (the user: "the goal is the enemy capital. if we take that, we can declare peace and let them have a minor settlement")
		// the target finished (IsStrikeTargetFinished): no-retreat ends, so peace is CP's choice, and the strike goes on to its
		// next target, back in BUILDUP (the army is there; the declaration waits for the take-the-city check). The bans hold
		if (eWarTarget == NO_PLAYER || IsStrikeTargetFinished(eWarTarget))
		{
			eNew = STRIKE_BUILDUP;
			// (the lead's review of cbebe77) why: the capital taken by us or by a third party, the target dead, or released
			const CvCity* pGoal = (eWarTarget != NO_PLAYER) ? GetStrikeObjectiveCity(eWarTarget) : NULL;
			strWhy = (eWarTarget == NO_PLAYER) ? "the target released (its capital out of its hands, or it can't be fought): the next target" :
				!GET_PLAYER(eWarTarget).isAlive() ? "the target eliminated: the next target" :
				(pGoal && pGoal->getTeam() == m_pPlayer->getTeam()) ? "the target's capital taken by us: peace allowed, the next target" :
				"the target's capital taken by a third party (or gone): the next target";
			m_iStrikeCityState = NO_PLAYER;
			m_eTimingBuildupTarget = NO_PLAYER;
			// the window reopens for the next target (the declaration block lets only the window's target through, and
			// StartTimingBuildup sets the target only when the window is closed)
			m_iTimingBuildupStartTurn = -1;
			m_iStrikeSiege = 0;
			m_iStrikeWarsDeclaredMark = iWarsDeclared;
			m_iStrikeWarsOnUsMark = iWarsOnUs;
		}
		// peace with a target not finished (a peace forced on us, e.g. by a vassal's master): the target is held, and the strike
		// builds up and declares again when it can
		else if (!GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(eWarTarget).getTeam()))
		{
			eNew = STRIKE_BUILDUP;
			strWhy = "peace with the target, not finished: the target is held, the strike builds up again";
			m_iTimingBuildupStartTurn = -1;	// the window reopens on the held target
			m_iStrikeSiege = 0;
			m_iStrikeWarsDeclaredMark = iWarsDeclared;
			m_iStrikeWarsOnUsMark = iWarsOnUs;
		}
		// (the user: "they are ALL IN. Their is no retreating") no exit on losses: the war holds until the target's capital is taken
		// the strike's ownership (gate 4, the lead's audit): CP picks its own attack targets; until the target's capital is taken, an
		// attack on it is always under way, aimed at its capital first
		else
		{
			const PlayerTypes eStrikeEnemy = (eCS != NO_PLAYER) ? eCS : m_eTimingBuildupTarget;
			// (18BE1E3E, 18DA3905) an attack under way is a city attack operation on the target: a rapid response aimed at the
			// target's plot (an "offensive" operation outside our land) held off the strike's request on 32 and 7 WAR turns
			if (eStrikeEnemy != NO_PLAYER && IsStrikeNoRetreat(eStrikeEnemy) && !HasStrikeCityAttackUnderWay(eStrikeEnemy))
			{
				// at the target's capital while the target holds it (the user, the trio game: after the first city the army
				// scattered); RequestCityAttack falls back to the target's best city when the capital isn't an attack target.
				// Up to 3 units to build: the formation's 3 required melee slots (18BE1E3E: rapid responses held the Hoplites, and
				// an attack with a Catapult, 4 Composite Bowmen and a General was dropped for 3 missing, 2 allowed)
				const CvCity* pObjective = GetStrikeObjectiveCity(eStrikeEnemy);
				bool bSiegeFed = false;
				const bool bRequested = RequestCityAttack(eStrikeEnemy, 3, false, (pObjective && pObjective->getOwner() == eStrikeEnemy) ? pObjective : NULL, &bSiegeFed);
				if (GC.getLogging() && GC.getAILogging())
					m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: no attack under way on %s; city attack requested (%s)",
						GET_PLAYER(eStrikeEnemy).getCivilizationShortDescription(), bRequested ? "started" : bSiegeFed ? "not started: under siege already (SiegeFeed)" :
						HaveStrikeLandAttackTarget(eStrikeEnemy) ? "not possible this turn" : "not possible: no land attack target, no army path"));
			}
		}
	}

	if (eNew != eOld)
	{
		m_iStrikeState = eNew;
		if (GC.getLogging() && GC.getAILogging())
		{
			static const char* const kNames[] = { "NONE", "PREP", "BUILDUP", "WAR", "OVER" };
			m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("Strike: %s -> %s (%s; %s)", kNames[eOld], kNames[eNew], strWhy.c_str(),
				bEarly ? "EarlyStrike" : "LateStrike"));
		}
	}
	RefreshStrikeSnapshot();	// the turn's final state (a target finished or released above)
}

/// a normal army (cities + 4 field units), 3 melee that can take a city, and nothing more affordable (WarBudget's measure):
/// the strike goes with what it has rather than wait on a clock
bool CvMilitaryAI::IsStrikeAsReadyAsItGets() const
{
	//and the user: "only start the attack if the city is takeable"
	if (!IsStrikeTargetTakeableNow() || !IsStrikeHomeSafe())
		return false;
	const int iSiege = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true);
	if (TimingBuildupFieldUnits(m_pPlayer, this, iSiege) >= max(1, m_pPlayer->getNumCities()) + 4 &&
		GetEarlyStrikeMeleeUnits() >= EARLY_STRIKE_MELEE_MIN && m_pPlayer->GetEconomicAI()->GetAffordableWarUnits(20) <= 0)
		return true;
	// (the lead, STOP 18FA5DFB: the Zulus' build-up went OVER "stalled" at T109 with Nottingham takeable; CP declared at T114
	// with the same 18 units and took it at T117) a stalled build-up can't get any readier: with the target takeable, it goes,
	// if the units we have fill the city attack's required slots (the lead's review of ae8e9ae: a stalled civ builds nothing,
	// and the unit hold blocks every combat unit in a city-state war, so a slot missing now stays missing and no attack starts)
	return IsStrikeBuildupStalled() && GetStrikeFormationMissingRequired() == 0;
}

/// the required slots of a land city attack's formation (CvAIOperation::GetFormationType: SiegeFormation's basic force for the
/// small one) that our field units (GetStrikeFieldUnits) can't fill, matched as OperationalAIHelpers::IsUnitSuitableForRecruitment
/// matches a unit to a slot (its unit AI, or one its type allows)
int CvMilitaryAI::GetStrikeFormationMissingRequired() const
{
	MultiunitFormationTypes eFormation = OperationalAIHelpers::GetArmyFormationForOpType(AI_OPERATION_CITY_ATTACK_LAND);
	if (eFormation == MUFORMATION_SMALL_CITY_ATTACK_FORCE && m_pPlayer->UsesCandidateAI(FIX_SIEGE_FORMATION))
		eFormation = MUFORMATION_BASIC_CITY_ATTACK_FORCE;
	CvMultiUnitFormationInfo* pFormation = (eFormation != NO_MUFORMATION) ? GC.getMultiUnitFormationInfo(eFormation) : NULL;
	if (!pFormation)
		return 0;
	std::vector<CvFormationSlotEntry> vOpen;
	for (size_t i = 0; i < pFormation->getNumFormationSlotEntries(); i++)
	{
		if (pFormation->getFormationSlotEntry(i).m_requiredSlot)
			vOpen.push_back(pFormation->getFormationSlotEntry(i));
	}
	std::vector<const CvUnit*> vUnits;
	GetStrikeFieldUnits(m_pPlayer, vUnits);
	for (size_t u = 0; u < vUnits.size() && !vOpen.empty(); u++)
	{
		const CvUnit* pUnit = vUnits[u];
		const CvUnitEntry* pkInfo = GC.getUnitInfo(pUnit->getUnitType());
		for (size_t j = 0; j < vOpen.size(); j++)
		{
			const CvFormationSlotEntry& kSlot = vOpen[j];
			if (kSlot.m_primaryUnitType == NO_UNITAI || pUnit->AI_getUnitAIType() == kSlot.m_primaryUnitType ||
				pUnit->AI_getUnitAIType() == kSlot.m_secondaryUnitType || (pkInfo && (pkInfo->GetUnitAIType(kSlot.m_primaryUnitType) ||
				(kSlot.m_secondaryUnitType != NO_UNITAI && pkInfo->GetUnitAIType(kSlot.m_secondaryUnitType)))))
			{
				vOpen.erase(vOpen.begin() + j);
				break;
			}
		}
	}
	return (int)vOpen.size();
}

/// a build-up that can't grow (Economy 0055's unit hold: no unit affordable; income negative; the capital can't build a Market
/// or a Caravan): the army it has is the army it gets
bool CvMilitaryAI::IsStrikeBuildupStalled() const
{
	// (Economy's review of ae8e9ae) net 0 counts: with no income growth no unit ever becomes affordable
	return IsStrikeBuildup() && m_pPlayer->IsStrikeUnitHold() && m_pPlayer->GetTreasury()->CalculateBaseNetGold() <= 0 && !CanBuildStrikeIncomeItem();
}

/// what keeps the build-up from "as ready as it gets" (the stall exit's log; the lead, STOP 18FA5DFB: the OVER line didn't say)
CvString CvMilitaryAI::GetStrikeReadinessGaps() const
{
	const int iSiege = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true);
	const int iField = TimingBuildupFieldUnits(m_pPlayer, this, iSiege);
	const int iFieldWanted = max(1, m_pPlayer->getNumCities()) + 4;
	const int iMelee = GetEarlyStrikeMeleeUnits();
	const int iAffordable = m_pPlayer->GetEconomicAI()->GetAffordableWarUnits(20);
	CvString str;
	if (!IsStrikeTargetTakeableNow())
		str += "the target not takeable; ";
	if (!IsStrikeHomeSafe())
		str += "a city of ours in danger; ";
	if (iField < iFieldWanted)
		str += CvString::format("field %d/%d; ", iField, iFieldWanted);
	if (iMelee < EARLY_STRIKE_MELEE_MIN)
		str += CvString::format("melee %d/%d; ", iMelee, (int)EARLY_STRIKE_MELEE_MIN);
	if (iAffordable > 0)
		str += CvString::format("%d more affordable; ", iAffordable);
	return str.empty() ? CvString("none") : str;
}

bool CvMilitaryAI::IsTimingBuildupArmyReady() const
{
	//a strike's build-up (the user: "just calculate if you can take the city"; the lead, bullygreedy24 17436CCD: Zulu sat in
	//BUILDUP T107 to the end, "the plan's army takes it in about 2 turns", held by CP's normal field target growing 13 -> 21
	//and a support count at 7/8): the army on hand taking the target decides. No field count, no exact plan counts, no
	//siege. Upgrade and go (the user: "i upgraded 3 archers to composite archers ... upgrade and go"): not while units at
	//home wait for an upgrade whose tech is in; and 3 melee units that can take the city (2 at the city, 1 spare); and home
	//safe
	if (IsStrikeBuildup())
		return GetEarlyStrikeUpgradeBill() == 0 && GetEarlyStrikeMeleeUnits() >= EARLY_STRIKE_MELEE_MIN && IsStrikeTargetTakeableNow() &&
			IsStrikeHomeSafe();
	return IsTimingBuildupArmyBuilt();
}

/// AI_AB_TEST (flag EarlyStrike): the gold to upgrade the land combat units in our territory whose upgrade's tech is known,
/// or is being researched with at most 8 turns (Standard) left; resources for the new unit required. The user's treasury
/// went from 263 at T40 to 31 at T50 (Quick): saved for the Composite Bowmen, spent just before the declaration.
int CvMilitaryAI::GetEarlyStrikeUpgradeBill(int* piUnits, bool* pbTechSoon) const
{
	if (piUnits)
		*piUnits = 0;
	if (pbTechSoon)
		*pbTechSoon = false;
	CvPlayerTechs* pTechs = m_pPlayer->GetPlayerTechs();
	const TechTypes eResearch = pTechs->GetCurrentResearch();
	const bool bResearchSoon = false;	// no turn horizon (the user): units count when their upgrade's tech is known, or is the strike's own
	CvStrikeWindow kWindow;
	TechTypes eStrikeUpgradeTech = GetEarlyStrikeWindow(kWindow) ? kWindow.eUpgradeTech : NO_TECH;
	//LateStrike, or an early strike without a profile spike (Personality's generic Composite Bowman rush, bGenericRush: no
	//window): the strike plan's upgrade
	if (IsLateStrikeCiv() || (eStrikeUpgradeTech == NO_TECH && m_pPlayer->GetGrandStrategyAI()->GetStrikePlan().bValid))
		eStrikeUpgradeTech = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan().eUpgradeTech;
	int iBill = 0, iLoop = 0;
	for (const CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
	{
		if (pLoopUnit->getDomainType() != DOMAIN_LAND || !pLoopUnit->IsCombatUnit() || pLoopUnit->AI_getUnitAIType() == UNITAI_EXPLORE ||
			pLoopUnit->isDelayedDeath() || pLoopUnit->plot()->getOwner() != m_pPlayer->GetID())
			continue;
		const UnitTypes eUpgrade = pLoopUnit->GetUpgradeUnitType();
		CvUnitEntry* pkUpgrade = eUpgrade != NO_UNIT ? GC.getUnitInfo(eUpgrade) : NULL;
		if (!pkUpgrade || !m_pPlayer->HasResourceForNewUnit(eUpgrade, false, true, pLoopUnit->getUnitType()))
			continue;
		const TechTypes eTech = (TechTypes)pkUpgrade->GetPrereqAndTech();
		const bool bKnown = eTech == NO_TECH || GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech(eTech);
		//the strike's own upgrade (Personality's eUpgradeTech, e.g. Construction for Composite Bowmen): saved for from the
		//start of the build-up, and the declaration waits for it (upgrade and go when the tech lands)
		const bool bStrikeUpgrade = eTech != NO_TECH && eTech == eStrikeUpgradeTech;
		if (!bKnown && !(bResearchSoon && eTech == eResearch) && !bStrikeUpgrade)
			continue;
		iBill += pLoopUnit->upgradePrice(eUpgrade);
		if (piUnits)
			(*piUnits)++;
		if (!bKnown && pbTechSoon)
			*pbTechSoon = true;
	}
	return iBill;
}

/// AI_AB_TEST (diagnostic flag BigArmy, the user and the lead): is the attack failing because the army is too small, or
/// because of how it fights at the walls? Under BigArmy the ready army is twice as big (4 siege units, 2 x (cities + 4)
/// others), the window lasts until it is built (up to 4 windows) and the window's build caps are lifted (CvUnitProductionAI).
/// Nothing else changes. Test only: never in the bundle or a style.
bool CvMilitaryAI::IsTimingBuildupBigArmy() const
{
	return m_pPlayer->UsesCandidateAI(FIX_BIG_ARMY) && IsTimingBuildupPlayer(); // the flag first: this is asked often (unit supply)
}

/// BigArmy: gold is made irrelevant from the window's start to the end of the war (the lead): no unit upkeep, no bankruptcy
bool CvMilitaryAI::IsBigArmyUpkeepWaived() const
{
	return IsTimingBuildupBigArmy() && GetTimingBuildupPhase() >= TIMING_BUILDUP_WINDOW;
}

int CvMilitaryAI::GetTimingBuildupSiegeTarget() const
{
	//EarlyStrike: siege only when the strength check found the army too weak for the target (walls alone don't call for it):
	//then CP's 2, or the plan's siege count if larger; otherwise the plan's always-brought siege (Huns' Rams), if any
	if (IsEarlyStrikeBuildup())
	{
		const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
		const int iPlanSiege = (kPlan.bValid && kPlan.eSiegeClass != NO_UNITCLASS) ? kPlan.iSiegeTarget : 0;
		return m_iStrikeSiege > 0 ? max(2, iPlanSiege) : iPlanSiege;
	}
	//LateStrike: the plan's siege (always brought), plus CP's 2 against a walled target unless the plan says siege only if walled
	if (IsLateStrikeBuildup())
	{
		const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
		return (kPlan.bSiegeOnlyIfWalled && m_iStrikeSiege == 0) ? kPlan.iSiegeTarget : max(2, kPlan.iSiegeTarget);
	}
	return IsTimingBuildupBigArmy() ? 4 : 2;
}

/// AI_AB_TEST (candidate, flag EarlyStrike, War's half; Economy's book half is CvPlayer::IsEarlyStrikeBook, patch 0028; the
/// user: "every civ builds to its strengths"). The user's Aztec took Marrakesh at T54 (Quick) with 6 Jaguars and 4 Composite
/// Bowmen and no siege, from a military-first capital. Over our games (also Quick), wars declared at T60-79 took a city in
/// 31% of games against 6-13% later, unwalled cities fell twice as often as walled ones, and siege at the declaration
/// predicted nothing. An EarlyStrike civ is a timing attacker on Economy's EarlyStrike book (a warmonger whose spike, from
/// Personality's profile, comes by turn 30 Standard). Its build-up opens at the normal turn 40 (Standard), with the book's
/// army already built (Personality's strike window ends before that army can exist: the Aztec's T3-23 on Quick).
bool CvMilitaryAI::IsEarlyStrikeCiv() const
{
	//static (who plays the strike), not the book's own state: Economy's book now follows GetStrikeState
	return IsTimingBuildupPlayer() && m_pPlayer->IsCandidateWarmonger(FIX_EARLY_STRIKE) && m_pPlayer->GetEarlyStrikeCoreClass() != NO_UNITCLASS;
}

/// the strike's build-up: one opened by turn 60 (Standard)
/// a strike's build-up: the strike state's BUILDUP with its build-up window open (no turn limit: the state's exits end it)
bool CvMilitaryAI::IsEarlyStrikeBuildup() const
{
	return m_iTimingBuildupStartTurn >= 0 && GetStrikeState() == STRIKE_BUILDUP && IsEarlyStrikeCiv();
}

bool CvMilitaryAI::GetEarlyStrikeWindow(CvStrikeWindow& kWindow) const
{
	return m_pPlayer->GetGrandStrategyAI() && m_pPlayer->GetGrandStrategyAI()->GetStrikeWindow(kWindow);
}

/// the strike's core unit class: Economy's per-civ plan (CvPlayer::GetEarlyStrikePlan, patch 0031: the Huns' Horse Archer,
/// with Battering Rams as the plan's siege) when there is one, else Personality's pick (GetEarlyStrikeCoreClass)
UnitClassTypes CvMilitaryAI::GetEarlyStrikeCore() const
{
	CvEarlyStrikePlan kPlan;
	if (m_pPlayer->GetEarlyStrikePlan(kPlan) && kPlan.eCore != NO_UNITCLASS)
		return kPlan.eCore;
	return m_pPlayer->GetEarlyStrikeCoreClass();
}

/// AI_AB_TEST (candidate, flag LateStrike, War; the user: "every civ builds to its strengths"; the lead: late-strike plans
/// for the Zulu, Mongolia, Assyria, Japan and Songhai). A timing attacker that isn't on EarlyStrike's book, with a strike plan
/// (Personality's CvGrandStrategyAI::GetStrikePlan; for these civs the data default: the window opens 5 turns after the
/// later of the spike's tech and its upgrade tech, declare within 10 turns, fallback 5 turns later). Infrastructure first:
/// no build-up before the plan's window, then the plan's army and upgrade, then the declaration.
bool CvMilitaryAI::IsLateStrikeCiv() const
{
	//the same civs as Economy's book half (CvPlayer::IsLateStrikeCiv, patch 0033), as timing attackers with a core unit
	if (!IsTimingBuildupPlayer() || !m_pPlayer->IsLateStrikeCiv())
		return false;
	//one source (the lead, 10:33pm ET): the strike plan's own bLateStrike (Personality's late-strike rows, else the default)
	const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
	return kPlan.bLateStrike && kPlan.eCoreClass != NO_UNITCLASS;
}

bool CvMilitaryAI::IsLateStrikeOpen() const
{
	return IsLateStrikeCiv() && GetStrikeState() == STRIKE_BUILDUP && IsStrikeArmyBuildable();
}

/// the strike's army can be built: its core unit, or the plan's pre-build (China's Composite Bowmen, to be Chu-Ko-Nu), can be
/// trained now. Until then a BUILDUP does nothing but research and save (Economy) and hold upgrades.
bool CvMilitaryAI::IsStrikeArmyBuildable() const
{
	if (!m_pPlayer->GetGrandStrategyAI())
		return false;
	const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
	const UnitClassTypes eCoreClass = IsEarlyStrikeCiv() ? GetEarlyStrikeCore() : kPlan.eCoreClass;
	if (eCoreClass == NO_UNITCLASS)
		return false;
	if (IsStrikeClassBuildable(eCoreClass))
		return true;
	//a plan with a pre-build (China's Composite Bowmen, upgraded to Chu-Ko-Nu at Machinery) opens on its turn: the pre-build
	//is the window's army, and the declaration waits for the upgrade (the upgrade bill)
	return IsStrikeClassBuildable(kPlan.ePreBuildClass);
}

/// a strike unit class we can train now, or could but for resources we own and have in use (the army built from them; the
/// lead's reviews: canTrainUnit counts free resources only). Also Personality's early economy-ready test. Any class on its
/// strike line counts (GetStrikeLine; the lead's review of c7eed909: Assyria's Archer, obsolete at Construction, read
/// unbuildable though Composite Bowmen are the plan, so the late window never opened)
bool CvMilitaryAI::IsStrikeClassBuildable(UnitClassTypes eClass) const
{
	std::vector<UnitClassTypes> vLine;
	GetStrikeLine(m_pPlayer, eClass, vLine);
	for (size_t i = 0; i < vLine.size(); i++)
	{
		const UnitTypes eUnit = m_pPlayer->GetSpecificUnitType(vLine[i]);
		if (eUnit == NO_UNIT)
			continue;
		if (m_pPlayer->canTrainUnit(eUnit, false, false, /*bIgnoreCost*/ true) || StrikeClassTrainableButForUse(m_pPlayer, vLine[i]))
			return true;
	}
	return false;
}

bool CvMilitaryAI::IsLateStrikeBuildup() const
{
	return m_iTimingBuildupStartTurn >= 0 && GetStrikeState() == STRIKE_BUILDUP && IsLateStrikeCiv();
}

bool CvMilitaryAI::IsStrikeBuildup() const
{
	return IsEarlyStrikeBuildup() || IsLateStrikeBuildup();
}

int CvMilitaryAI::GetLateStrikeFieldFloor() const
{
	if (!IsLateStrikeCiv())
		return 0;
	const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
	return kPlan.iCoreTarget + kPlan.iSupportTarget;
}

int CvMilitaryAI::GetEarlyStrikeCoreSharePercent() const
{
	CvStrikeWindow kWindow;
	return (GetEarlyStrikeWindow(kWindow) && kWindow.iCoreSharePercent > 0) ? kWindow.iCoreSharePercent : 50;
}

/// AI_AB_TEST (flag EarlyStrike, the expert strike): melee land units that can take a city. The research: "bring two in case
/// one dies"; over 57 logged first capture attempts the city fell 5 of 24 times with 2+ such units in reach, 2 of 18 with 1,
/// 1 of 15 with none.
int CvMilitaryAI::GetEarlyStrikeMeleeUnits() const
{
	int iCount = 0, iLoop = 0;
	for (const CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
	{
		if (pLoopUnit->getDomainType() == DOMAIN_LAND && pLoopUnit->IsCombatUnit() && !pLoopUnit->IsCanAttackRanged() && !pLoopUnit->isNoCapture() &&
			pLoopUnit->AI_getUnitAIType() != UNITAI_CITY_BOMBARD && pLoopUnit->AI_getUnitAIType() != UNITAI_EXPLORE && !pLoopUnit->isDelayedDeath())
			iCount++;
	}
	return iCount;
}

/// the target's city nearest our capital has no walls
bool CvMilitaryAI::IsEarlyStrikeTargetUnwalled() const
{
	static const BuildingClassTypes eWalls = (BuildingClassTypes)GC.getInfoTypeForString("BUILDINGCLASS_WALLS", true);
	const CvCity* pCapital = m_pPlayer->getCapitalCity();
	if (m_eTimingBuildupTarget == NO_PLAYER || !pCapital || eWalls == NO_BUILDINGCLASS)
		return false;
	const CvCity* pNearest = NULL;
	int iBest = MAX_INT, iLoop = 0;
	for (const CvCity* pLoopCity = GET_PLAYER(m_eTimingBuildupTarget).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(m_eTimingBuildupTarget).nextCity(&iLoop))
	{
		const int iDistance = plotDistance(pCapital->getX(), pCapital->getY(), pLoopCity->getX(), pLoopCity->getY());
		if (iDistance < iBest)
		{
			iBest = iDistance;
			pNearest = pLoopCity;
		}
	}
	return pNearest && !pNearest->GetCityBuildings()->HasBuildingClass(eWalls);
}

/// AI_AB_TEST (candidate, flag WarBudget, Economy and War; the lead's honest package): BigArmy's doubled army took cities in
/// 5 of 11 games against 3 of 28, but only with its upkeep waived (73 a turn at war). Under WarBudget the field target is the
/// field units the civ has plus the paid units its gold can keep for 20 turns (CvEconomicAI::GetAffordableWarUnits, patch
/// 0025), from the normal army (cities + 4) up to BigArmy's (twice that). Counted from today's army so the target stays put
/// as units are built (each one built is one less affordable); no waiver. BigArmy, when on, takes precedence.
bool CvMilitaryAI::IsTimingBuildupWarBudget() const
{
	return m_pPlayer->UsesCandidateAI(FIX_WAR_BUDGET) && IsTimingBuildupPlayer() && !IsTimingBuildupBigArmy();
}

int CvMilitaryAI::GetTimingBuildupFieldTarget() const
{
	//EarlyStrike: the plan's army (Personality's core + support counts, at least the 3 melee), not cities + 4 or WarBudget's
	//affordable army (12 in the expert block's strikes, against plans of 8): the window asks for what the plan brings
	if (IsEarlyStrikeBuildup())
	{
		const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
		if (kPlan.bValid && kPlan.iCoreTarget + kPlan.iSupportTarget > 0)
			return max((int)EARLY_STRIKE_MELEE_MIN, kPlan.iCoreTarget + kPlan.iSupportTarget);
	}
	const int iNormal = max(1, m_pPlayer->getNumCities()) + 4;
	if (IsTimingBuildupWarBudget())
	{
		const int iSiege = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true);
		const int iAffordable = max(0, m_pPlayer->GetEconomicAI()->GetAffordableWarUnits(20));
		return max(IsLateStrikeBuildup() ? GetLateStrikeFieldFloor() : 0, max(IsEarlyStrikeBuildup() ? 8 : 0, min(2 * iNormal, max(iNormal, TimingBuildupFieldUnits(m_pPlayer, this, iSiege) + iAffordable))));
	}
	//EarlyStrike: at least 8 field units (the user's strike had about 10, from 2 cities; the normal target is cities + 4)
	return max(IsLateStrikeBuildup() ? GetLateStrikeFieldFloor() : 0, max(IsEarlyStrikeBuildup() ? 8 : 0, iNormal * (IsTimingBuildupBigArmy() ? 2 : 1)));
}

/// BigArmy's log: the land army by unit type, with the gold
void CvMilitaryAI::LogBigArmy(const char* szWhen) const
{
	//EarlyStrike civs log it too (the expert block's read: promotions and melee in reach), whatever the army target flags
	if (!(IsTimingBuildupBigArmy() || IsTimingBuildupWarBudget() || IsEarlyStrikeCiv()) || !(GC.getLogging() && GC.getAILogging()))
		return;
	map<CvString, int> mTypes;
	int iLand = 0, iSiege = 0, iLoop = 0;
	int iPromoted = 0, iPromotions = 0, iXPTimes100 = 0;	// the army's experience (the user's human game: promoted Jaguars took a capital at T54)
	for (const CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
	{
		if (pLoopUnit->getDomainType() != DOMAIN_LAND || (!pLoopUnit->IsCombatUnit() && !pLoopUnit->IsCanAttackRanged()))
			continue;
		iLand++;
		if (pLoopUnit->getLevel() > 1)
			iPromoted++;
		iPromotions += max(0, pLoopUnit->getLevel() - 1);
		iXPTimes100 += pLoopUnit->getExperienceTimes100();
		if (pLoopUnit->AI_getUnitAIType() == UNITAI_CITY_BOMBARD)
			iSiege++;
		mTypes[pLoopUnit->getUnitInfo().GetDescription()]++;
	}
	CvString strTypes;
	for (map<CvString, int>::const_iterator it = mTypes.begin(); it != mTypes.end(); ++it)
		strTypes += CvString::format("%s%s %d", strTypes.empty() ? "" : ", ", it->first.c_str(), it->second);
	//WarBudget writes the same line under its own name, with what gold still pays for (20 turns)
	const CvString strAffordable = IsTimingBuildupWarBudget() ? CvString::format(", affordable %d more", m_pPlayer->GetEconomicAI()->GetAffordableWarUnits(20)) : CvString("");
	m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("%s: army %s: %d land (%d siege; target %d siege, %d others), gold %d, %d a turn%s, promoted %d (%d promotions, %d XP): %s",
		IsTimingBuildupBigArmy() ? "BigArmy" : IsTimingBuildupWarBudget() ? "WarBudget" : "EarlyStrike", szWhen, iLand, iSiege, GetTimingBuildupSiegeTarget(), GetTimingBuildupFieldTarget(), m_pPlayer->GetTreasury()->GetGold(),
		m_pPlayer->GetTreasury()->CalculateBaseNetGold(), strAffordable.c_str(), iPromoted, iPromotions, iXPTimes100 / 100, strTypes.c_str()));
}

/// non-siege land combat units at peace: a garrison per city and one escort
int CvMilitaryAI::GetTimingBuildupPeaceCap() const
{
	const int iNormal = max(1, m_pPlayer->getNumCities()) + 1;
	//a strike civ before its strike (PREP, BUILDUP): the plan's army is allowed (core + support, the pre-build for a late
	//plan), so the peace cap doesn't veto the units the book and the plan ask for. Germany (ab-8555f21, 14BC6621): every unit
	//in Munich at T61 was "badtimeorplace" (this cap), and the city fell back to Heroic Epic and the Temple of Artemis
	const StrikeStateTypes eState = GetStrikeState();
	if (eState == STRIKE_PREP || eState == STRIKE_BUILDUP)
	{
		const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
		if (kPlan.bValid)
			return max(iNormal, kPlan.iCoreTarget + kPlan.iSupportTarget + max(0, kPlan.iPreBuildTarget - kPlan.iCoreTarget));
	}
	return iNormal;
}

/// the plan's phase: NONE (not a timing attacker), PEACE (no target), PLANNED (target chosen, at peace, no window yet),
/// WINDOW (building up), ATTACK (built up or window over, not yet at war), WAR (at war with a major)
CvMilitaryAI::TimingBuildupPhase CvMilitaryAI::GetTimingBuildupPhase() const
{
	if (!IsTimingBuildupPlayer())
		return TIMING_BUILDUP_NONE;
	if (m_pPlayer->IsAtWarAnyMajor())
		return TIMING_BUILDUP_WAR;
	if (IsInTimingBuildupWindow())
		return TIMING_BUILDUP_WINDOW;
	if (m_iTimingBuildupStartTurn >= 0)
		return TIMING_BUILDUP_ATTACK;
	if (m_pPlayer->GetDiplomacyAI()->GetCommittedConquestTarget() != NO_PLAYER)
		return TIMING_BUILDUP_PLANNED;
	return TIMING_BUILDUP_PEACE;
}

/// the attack's target: the window's, or the committed one while planned
PlayerTypes CvMilitaryAI::GetTimingBuildupTarget() const
{
	if (!IsTimingBuildupPlayer())
		return NO_PLAYER;
	if (m_iTimingBuildupStartTurn >= 0 && m_eTimingBuildupTarget != NO_PLAYER)
		return m_eTimingBuildupTarget;
	return m_pPlayer->GetDiplomacyAI()->GetCommittedConquestTarget();
}

/// land units still missing for the attack army (2 siege units and cities + 4 others)
int CvMilitaryAI::GetTimingBuildupUnitsWanted() const
{
	if (!IsTimingBuildupPlayer())
		return 0;
	const int iSiege = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true);
	return max(0, GetTimingBuildupSiegeTarget() - iSiege) + max(0, GetTimingBuildupFieldTarget() - TimingBuildupFieldUnits(m_pPlayer, this, iSiege));
}

/// the unit the build-up would build or buy now (at the capital): siege or attack
UnitTypes CvMilitaryAI::GetTimingBuildupUnitType(bool bSiege) const
{
	CvCity* pCapital = m_pPlayer->getCapitalCity();
	if (!IsTimingBuildupPlayer() || !pCapital)
		return NO_UNIT;
	return pCapital->GetCityStrategyAI()->GetUnitProductionAI()->RecommendUnit(bSiege ? UNITAI_CITY_BOMBARD : UNITAI_ATTACK, true);
}

void CvMilitaryAI::StartTimingBuildup(PlayerTypes eTarget)
{
	if (m_iTimingBuildupStartTurn >= 0)
		return;
	m_iTimingBuildupStartTurn = GC.getGame().getGameTurn();
	if (IsEarlyStrikeBuildup() && GC.getLogging() && GC.getAILogging())
	{
		m_eTimingBuildupTarget = eTarget;
		const UnitClassTypes eCore = GetEarlyStrikeCore();
		CvUnitClassInfo* pkCore = eCore != NO_UNITCLASS ? GC.getUnitClassInfo(eCore) : NULL;
		m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("EarlyStrike: build-up opens, core unit class %s, target's nearest city %s",
			pkCore ? pkCore->GetDescription() : "none", IsEarlyStrikeTargetUnwalled() ? "unwalled (no siege)" : "walled"));
	}
	if (IsLateStrikeBuildup() && GC.getLogging() && GC.getAILogging())
	{
		m_eTimingBuildupTarget = eTarget;
		const CvStrikePlan& kPlan = m_pPlayer->GetGrandStrategyAI()->GetStrikePlan();
		CvUnitClassInfo* pkCore = kPlan.eCoreClass != NO_UNITCLASS ? GC.getUnitClassInfo(kPlan.eCoreClass) : NULL;
		CvUnitClassInfo* pkSupport = kPlan.eSupportClass != NO_UNITCLASS ? GC.getUnitClassInfo(kPlan.eSupportClass) : NULL;
		m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("LateStrike: build-up opens (plan open T%d, declare by T%d, fallback T%d), core %s x%d, support %s x%d, siege x%d, target's nearest city %s",
			kPlan.iOpenTurn, kPlan.iDeclareByTurn, kPlan.iFallbackTurn, pkCore ? pkCore->GetDescription() : "none", kPlan.iCoreTarget,
			pkSupport ? pkSupport->GetDescription() : "none", kPlan.iSupportTarget, kPlan.iSiegeTarget, IsEarlyStrikeTargetUnwalled() ? "unwalled" : "walled"));
	}
	m_eTimingBuildupTarget = eTarget;
	m_iTimingBuildupPausedTurns = 0;
	if (GC.getLogging() && GC.getAILogging())
		m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("TimingBuildup: window opens for %d turns against %s",
			TimingBuildupWindowTurns(), eTarget != NO_PLAYER ? GET_PLAYER(eTarget).getCivilizationShortDescription() : "?"));
}

void CvMilitaryAI::DoTimingBuildup()
{
	const int iTurn = GC.getGame().getGameTurn();
	CvTacticalAI* pTactical = m_pPlayer->GetTacticalAI();
	const bool bLog = GC.getLogging() && GC.getAILogging();

	//AI_AB_TEST (diagnostic flag BigArmy): say each turn what the upkeep waiver saved
	if (bLog && IsBigArmyUpkeepWaived())
		pTactical->LogTacticalMessage(CvString::format("BigArmy: upkeep waived %d (phase %d, gold %d, %d a turn)",
			m_pPlayer->GetTreasury()->CalculateUnitCost(-1, true), (int)GetTimingBuildupPhase(), m_pPlayer->GetTreasury()->GetGold(),
			m_pPlayer->GetTreasury()->CalculateBaseNetGold()));

	// at war: the window has done its job; savings go back to the economy
	// a strike's BUILDUP keeps its window through a third party's war (103bd11d: the strike goes on; the lead's review: its
	// declaration needs the open window, so closing it here held the strike until that war ended)
	const bool bStrikeWindowHolds = GetStrikeState() == STRIKE_BUILDUP && m_eTimingBuildupTarget != NO_PLAYER &&
		!GET_TEAM(m_pPlayer->getTeam()).isAtWar(GET_PLAYER(m_eTimingBuildupTarget).getTeam());
	if (m_pPlayer->IsAtWarAnyMajor() && !bStrikeWindowHolds)
	{
		if (m_iTimingBuildupStartTurn >= 0 && bLog)
			pTactical->LogTacticalMessage(CvString::format("TimingBuildup: at war, window closed after %d turns", iTurn - m_iTimingBuildupStartTurn));
		m_iTimingBuildupStartTurn = -1;
		if (m_bTimingBuildupSaving)
			m_pPlayer->GetEconomicAI()->CancelSaveForPurchase(PURCHASE_TYPE_UNIT);
		m_bTimingBuildupSaving = false;
		return;
	}

	//AI_AB_TEST (candidate, flag WindowUntilReady): say when the window runs past its turns for want of an army
	if (bLog && m_pPlayer->UsesCandidateAI(FIX_WINDOW_UNTIL_READY) && m_iTimingBuildupStartTurn >= 0 &&
		iTurn - m_iTimingBuildupStartTurn == TimingBuildupWindowTurns() && !IsTimingBuildupArmyReady())
		pTactical->LogTacticalMessage(CvString::format("TimingBuildup: army not ready (%d siege, %d land), the window goes on",
			m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true), m_pPlayer->GetNumUnitsWithDomain(DOMAIN_LAND, true)));

	// the attack never came: back to peace
	if (m_iTimingBuildupStartTurn >= 0 && !IsStrikeBuildup() && iTurn - m_iTimingBuildupStartTurn > (IsTimingBuildupBigArmy() ? 5 : 3) * TimingBuildupWindowTurns())
	{
		if (bLog)
			pTactical->LogTacticalMessage("TimingBuildup: no war 3 windows after the build-up, back to the peace army");
		m_iTimingBuildupStartTurn = -1;
	}

	// the window pauses while units are unaffordable: CP's upkeep guard refuses every combat unit when the treasury goes
	// bankrupt in under 9 turns (a test window refused 82 units as "too expensive" and ran out with no army); at most 10
	// paused turns a window
	//AI_AB_TEST (diagnostic flag BigArmy): no pause for gold; it may go into debt for the test, so log the army and the gold
	if (IsTimingBuildupWarBudget() && IsInTimingBuildupWindow())	// WarBudget: the army in the window; the gold pause still applies
		LogBigArmy(CvString::format("in the window (turn %d)", iTurn - m_iTimingBuildupStartTurn + 1).c_str());
	if (IsTimingBuildupBigArmy() && IsInTimingBuildupWindow())
		LogBigArmy(CvString::format("in the window (turn %d)", iTurn - m_iTimingBuildupStartTurn + 1).c_str());
	else if (IsInTimingBuildupWindow() && m_iTimingBuildupPausedTurns < 10)
	{
		const int iAverageGoldPerUnit = m_pPlayer->GetTreasury()->GetExpensePerTurnUnitMaintenance() / max(1, m_pPlayer->getNumUnits());
		if (m_pPlayer->getTurnsToBankruptcy(iAverageGoldPerUnit) < 9)
		{
			m_iTimingBuildupStartTurn++;
			m_iTimingBuildupPausedTurns++;
			if (bLog)
				pTactical->LogTacticalMessage(CvString::format("TimingBuildup: window paused, units unaffordable (%d paused turns)", m_iTimingBuildupPausedTurns));
		}
	}

	// Economy's WarChest owns the gold when it's on (savings and window purchases, CvEconomicAI::DoWarChest); looked up by
	// name so this builds before WarChest is merged
	static const int iWarChestFix = CvPlayer::GetCandidateFixByName("WarChest");
	const bool bWarChest = iWarChestFix >= 0 && m_pPlayer->UsesCandidateAI((CandidateFixTypes)iWarChestFix);

	// gold saved ahead: the price of 2 siege units at the capital, from turn 30 (Standard) until the window is spent
	CvCity* pCapital = m_pPlayer->getCapitalCity();
	const int iSpeed = GC.getGame().getGameSpeedInfo().getTrainPercent();
	if (bWarChest)
		m_bTimingBuildupSaving = false; // WarChest's bucket now: don't start or cancel it
	else if (pCapital && (m_iTimingBuildupStartTurn < 0 || IsInTimingBuildupWindow()) && iTurn >= 30 * iSpeed / 100)
	{
		UnitTypes eSiege = pCapital->GetCityStrategyAI()->GetUnitProductionAI()->RecommendUnit(UNITAI_CITY_BOMBARD, true);
		int iGoldTarget = (eSiege != NO_UNIT) ? 2 * pCapital->GetPurchaseCost(eSiege) : 0;
		//EarlyStrike: the upgrade bill, and no siege gold when the strike needs no siege
		if (IsStrikeBuildup())
			iGoldTarget = (GetTimingBuildupSiegeTarget() > 0 ? iGoldTarget : 0) + GetEarlyStrikeUpgradeBill();
		if (iGoldTarget > 0)
		{
			m_pPlayer->GetEconomicAI()->StartSaveForPurchase(PURCHASE_TYPE_UNIT, iGoldTarget, 400);
			m_bTimingBuildupSaving = true;
		}
	}
	else if (m_bTimingBuildupSaving)
	{
		m_pPlayer->GetEconomicAI()->CancelSaveForPurchase(PURCHASE_TYPE_UNIT);
		m_bTimingBuildupSaving = false;
	}

	// in the window: buy one siege or attack unit a turn in the city nearest the target, while the gold allows
	//EarlyStrike: gold goes to the upgrades first; log the wait
	int iUpgradeUnits = 0;
	bool bUpgradeTechSoon = false;
	const int iUpgradeBill = IsStrikeBuildup() && IsInTimingBuildupWindow() ? GetEarlyStrikeUpgradeBill(&iUpgradeUnits, &bUpgradeTechSoon) : 0;
	if (iUpgradeBill > 0 && bLog)
		pTactical->LogTacticalMessage(CvString::format("EarlyStrike: upgrade and go, %d units to upgrade for %d gold (gold %d)%s",
			iUpgradeUnits, iUpgradeBill, m_pPlayer->GetTreasury()->GetGold(), bUpgradeTechSoon ? ", upgrade tech a few turns off" : ""));
	//a strike's build-up keeps the upgrade bill in reserve and buys with the gold above it (Persia, 13E9ECC9: 350 gold idle
	//against a 165 bill while 5 units short); otherwise no purchase while a bill is due
	//the user (Nov 6): a strike buys no core or melee units (WarmongerHurry and NoObsoleteHurry stand), so its path asks for siege only
	const bool bStrikeBuys = IsStrikeBuildup();
	if (!bWarChest && (iUpgradeBill == 0 || bStrikeBuys) && IsInTimingBuildupWindow() && !IsTimingBuildupArmyReady())
	{
		CvCity* pTheirCity = m_eTimingBuildupTarget != NO_PLAYER ? m_pPlayer->GetClosestCityToUsByPlots(m_eTimingBuildupTarget) : NULL;
		CvCity* pBuyCity = pTheirCity ? m_pPlayer->GetClosestCityByPathLength(pTheirCity->plot()) : pCapital;
		if (pBuyCity && !pBuyCity->IsPuppet())
		{
			const int iSiegeWanted = bStrikeBuys ? GetTimingBuildupSiegeTarget() : 2;	// a strike without siege buys none
			const bool bNeedSiege = m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_CITY_BOMBARD, false, true) < iSiegeWanted;
			UnitTypes eType = (bStrikeBuys && !bNeedSiege) ? NO_UNIT :
				pBuyCity->GetCityStrategyAI()->GetUnitProductionAI()->RecommendUnit(bNeedSiege ? UNITAI_CITY_BOMBARD : UNITAI_ATTACK, true);
			if (eType != NO_UNIT && pBuyCity->IsCanPurchase(/*bTestPurchaseCost*/ true, /*bTestTrainable*/ true, eType, NO_BUILDING, NO_PROJECT, YIELD_GOLD))
			{
				const int iCost = pBuyCity->GetPurchaseCost(eType);
				if (m_pPlayer->GetEconomicAI()->CanWithdrawMoneyForPurchase(PURCHASE_TYPE_UNIT, iCost + iUpgradeBill, 400))
				{
					CvUnit* pUnit = pBuyCity->PurchaseUnit(eType, YIELD_GOLD);
					if (pUnit)
					{
						if (!pUnit->isHuman(ISHUMAN_AI_UNIT_PROMOTIONS))
							pUnit->AI_promote();
						if (bLog)
							pTactical->LogTacticalMessage(CvString::format("TimingBuildup: bought %s in %s for %d gold",
								pUnit->getUnitInfo().GetDescription(), pBuyCity->getNameNoSpace().c_str(), iCost));
					}
				}
			}
		}
	}

	// peace: let one extra land unit go each turn once more than 2 over the cap (siege units stay for the next window),
	// not in the 10 turns after a peace treaty. Not a strike civ before its declaration (PREP, BUILDUP): those units are the
	// strike's army (Persia, 13A89116, let an Archer, 2 Warriors and a Composite Bowman go from T57 to T77)
	const bool bStrikeArmy = GetStrikeState() == STRIKE_PREP || GetStrikeState() == STRIKE_BUILDUP;
	if (IsTimingBuildupPeace() && !bStrikeArmy)
	{
		for (int iTeamLoop = 0; iTeamLoop < MAX_MAJOR_CIVS; iTeamLoop++)
		{
			const TeamTypes eLoopTeam = (TeamTypes)iTeamLoop;
			if (eLoopTeam == m_pPlayer->getTeam())
				continue;
			const int iPeaceTurn = GET_TEAM(m_pPlayer->getTeam()).GetTurnMadePeaceTreatyWithTeam(eLoopTeam);
			if (iPeaceTurn >= 0 && iTurn - iPeaceTurn < 10)
				return;
		}

		int iNonSiege = 0;
		CvUnit* pWeakest = NULL;
		int iWeakestScore = INT_MAX;
		int iLoop = 0;
		for (CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
		{
			if (pLoopUnit->getDomainType() != DOMAIN_LAND || !pLoopUnit->IsCombatUnit() || pLoopUnit->isDelayedDeath())
				continue;
			if (pLoopUnit->AI_getUnitAIType() == UNITAI_EXPLORE || pLoopUnit->AI_getUnitAIType() == UNITAI_CITY_BOMBARD)
				continue;
			iNonSiege++;
			if (pLoopUnit->getArmyID() != -1 || !pLoopUnit->canScrap())
				continue;
			int iScore = pLoopUnit->GetPower() * pLoopUnit->getUnitInfo().GetProductionCost() + pLoopUnit->getLevel();
			if (pLoopUnit->plot()->isCity())
				iScore *= 2; // prefer one that isn't a garrison
			if (iScore < iWeakestScore)
			{
				iWeakestScore = iScore;
				pWeakest = pLoopUnit;
			}
		}

		if (pWeakest && iNonSiege > GetTimingBuildupPeaceCap() + 2)
		{
			if (bLog)
				pTactical->LogTacticalMessage(CvString::format("TimingBuildup: peace, %d land units over a cap of %d, letting %s go",
					iNonSiege, GetTimingBuildupPeaceCap(), pWeakest->getUnitInfo().GetDescription()));
			PlayerTypes eMinor = m_pPlayer->GetBestGiftTarget(DOMAIN_LAND);
			if (eMinor != NO_PLAYER)
				GET_PLAYER(eMinor).AddIncomingUnit(m_pPlayer->GetID(), pWeakest);
			else
				pWeakest->scrap();
		}
	}
}


/// Update how we're doing on defensive units
void CvMilitaryAI::UpdateDefenseState()
{
	// AI_AB_TEST: CP tests "below 100%" before "below 3/4", so NEEDED can never happen and a single missing
	// unit means CRITICAL (emergency production). The new AI checks the lower threshold first, like the
	// naval ladder below.
	if (m_pPlayer->UsesNewAI() && m_iNumLandUnits < m_iRecLandUnits * 3 / 4)
	{
		m_eLandDefenseState = DEFENSE_STATE_CRITICAL;
	}
	else if (m_pPlayer->UsesNewAI() && m_iNumLandUnits < m_iRecLandUnits)
	{
		m_eLandDefenseState = DEFENSE_STATE_NEEDED;
	}
	else if(m_iNumLandUnits < m_iRecLandUnits)
	{
		m_eLandDefenseState = DEFENSE_STATE_CRITICAL;
	}
	else if(m_iNumLandUnits < m_iRecLandUnits * 3 / 4)
	{
		m_eLandDefenseState = DEFENSE_STATE_NEEDED;
	}
	else if(m_iNumLandUnits < m_iRecLandUnits * 5 / 4)
	{
		m_eLandDefenseState = DEFENSE_STATE_NEUTRAL;
	}
	else
	{
		m_eLandDefenseState = DEFENSE_STATE_ENOUGH;
	}

	if (m_eLandDefenseState <= DEFENSE_STATE_NEUTRAL)
	{
		int iCityLoop = 0;
		for (CvCity* pLoopCity = m_pPlayer->firstCity(&iCityLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iCityLoop))
		{
			if (pLoopCity->isUnderSiege())
			{
				m_eLandDefenseState = DEFENSE_STATE_CRITICAL;
				break;
			}
		}
	}

	if(m_iNumNavalUnits <= (m_iRecNavalUnits / 2))
	{
		m_eNavalDefenseState = DEFENSE_STATE_CRITICAL;
	}
	else if(m_iNumNavalUnits <= m_iRecNavalUnits)
	{
		m_eNavalDefenseState = DEFENSE_STATE_NEEDED;
	}
	else if(m_iNumNavalUnits <= m_iRecNavalUnits * 5 / 4)
	{
		m_eNavalDefenseState = DEFENSE_STATE_NEUTRAL;
	}
	else
	{
		m_eNavalDefenseState = DEFENSE_STATE_ENOUGH;
	}

	if (m_eNavalDefenseState <= DEFENSE_STATE_NEUTRAL)
	{
		int iCityLoop = 0;
		for (CvCity* pLoopCity = m_pPlayer->firstCity(&iCityLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iCityLoop))
		{
			if (pLoopCity->isCoastal() && pLoopCity->isUnderSiege())
			{
				m_eNavalDefenseState = DEFENSE_STATE_CRITICAL;
				break;
			}
		}
	}
}

/// Count up barbarian camps and units visible to us
void CvMilitaryAI::ScanForBarbarians()
{
	int iLastTurnBarbarianCount = m_iVisibleBarbarianCount;

	m_iBarbarianCampCount = 0;
	m_iVisibleBarbarianCount = 0;

	TeamTypes eTeam = m_pPlayer->getTeam();

	// Look at revealed Barbarian camps and visible units
	for(int iPlotLoop = 0; iPlotLoop < GC.getMap().numPlots(); iPlotLoop++)
	{
		CvPlot* pPlot = GC.getMap().plotByIndexUnchecked(iPlotLoop);
		if(pPlot->isRevealed(eTeam))
		{
			if(pPlot->getRevealedImprovementType(eTeam) == GD_INT_GET(BARBARIAN_CAMP_IMPROVEMENT))
			{
				//count only reasonably close camps
				if (m_pPlayer->GetCityDistancePathLength(pPlot)<17)
					m_iBarbarianCampCount++;
			}
		}

		if(pPlot->isVisible(eTeam))
		{
			if(pPlot->plotCount(NULL, -1, -1, BARBARIAN_PLAYER, BARBARIAN_TEAM))
			{
				m_iVisibleBarbarianCount++;
			}
		}
	}

	//try to smooth the count a bit
	if (m_iVisibleBarbarianCount < iLastTurnBarbarianCount)
		m_iVisibleBarbarianCount = iLastTurnBarbarianCount - 1;
}

/// Start or stop military strategies to get flavors set properly
void CvMilitaryAI::UpdateMilitaryStrategies()
{

	int iStrategiesLoop = 0;

	// Loop through all strategies
	for(iStrategiesLoop = 0; iStrategiesLoop < GetMilitaryAIStrategies()->GetNumMilitaryAIStrategies(); iStrategiesLoop++)
	{
		MilitaryAIStrategyTypes eStrategy = (MilitaryAIStrategyTypes) iStrategiesLoop;
		CvMilitaryAIStrategyXMLEntry* pStrategy = GetMilitaryAIStrategies()->GetEntry(iStrategiesLoop);

		if(pStrategy == NULL)	// Can have holes in the list
			continue;

		// Minor Civs can't run some Strategies
		if(GetPlayer()->isMinorCiv() && pStrategy->IsNoMinorCivs())
			continue;

		// Some strategies ONLY for Minor Civs
		if(!GetPlayer()->isMinorCiv() && pStrategy->IsOnlyMinorCivs())
			continue;

		bool bTestStrategyStart = true;

		// Do we already have this Strategy adopted?
		if(IsUsingStrategy(eStrategy))
			bTestStrategyStart = false;

		else
		{
			// Has the prereq Tech necessary?
			if(pStrategy->GetTechPrereq() != NO_TECH && !GET_TEAM(GetPlayer()->getTeam()).GetTeamTechs()->HasTech((TechTypes) pStrategy->GetTechPrereq()))
			{
				bTestStrategyStart = false;
			}

			// Has the Tech which obsoletes this Strategy?
			if(bTestStrategyStart && pStrategy->GetTechObsolete() != NO_TECH && GET_TEAM(GetPlayer()->getTeam()).GetTeamTechs()->HasTech((TechTypes) pStrategy->GetTechObsolete()))
			{
				bTestStrategyStart = false;
			}

			// Not time to check this yet?
			if(GC.getGame().getGameTurn() < pStrategy->GetFirstTurnExecuted())
			{
				bTestStrategyStart = false;
			}
		}

		bool bTestStrategyEnd = false;

		// Strategy is active, check to see if we should even try to disable it
		if(IsUsingStrategy(eStrategy))
		{
			if(pStrategy->GetCheckTriggerTurnCount() > 0)
			{
				// Is it a turn where we want to check to see if this Strategy is maintained?
				if((GC.getGame().getGameTurn() - GetTurnStrategyAdopted(eStrategy)) % pStrategy->GetCheckTriggerTurnCount() == 0)
					bTestStrategyEnd = true;
			}

			if(bTestStrategyEnd && pStrategy->GetMinimumNumTurnsExecuted() > 0)
			{
				// Has the minimum # of turns passed for this Strategy?
				if(GC.getGame().getGameTurn() < GetTurnStrategyAdopted(eStrategy) + pStrategy->GetMinimumNumTurnsExecuted())
					bTestStrategyEnd = false;
			}
		}

		// Check Strategy Triggers
		// Functionality and existence of specific Strategies is hardcoded here, but data is stored in XML so it's easier to modify

		if(bTestStrategyStart || bTestStrategyEnd)
		{
			bool bStrategyShouldBeActive = false;

			// Has the Tech which obsoletes this Strategy? If so, Strategy should be deactivated regardless of other factors
			if(pStrategy->GetTechObsolete() != NO_TECH && GET_TEAM(GetPlayer()->getTeam()).GetTeamTechs()->HasTech((TechTypes) pStrategy->GetTechObsolete()))
			{
				bStrategyShouldBeActive = false;
			}

			// Strategy isn't obsolete, so test triggers as normal
			else
			{
				CvString strStrategyName = (CvString) pStrategy->GetType();

				// Check all of the Strategy Triggers
				if(strStrategyName == "MILITARYAISTRATEGY_ENOUGH_MILITARY_UNITS")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughMilitaryUnits(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_EMPIRE_DEFENSE")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EmpireDefense(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_EMPIRE_DEFENSE_CRITICAL")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EmpireDefenseCritical(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_ENOUGH_NAVAL_UNITS")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughNavalUnits(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_NAVAL_UNITS")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedNavalUnits(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_NAVAL_UNITS_CRITICAL")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedNavalUnitsCritical(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_WAR_MOBILIZATION")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_WarMobilization(eStrategy, m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_AT_WAR")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_AtWar(m_pPlayer,false);
				else if(strStrategyName == "MILITARYAISTRATEGY_MINOR_CIV_GENERAL_DEFENSE")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_MinorCivGeneralDefense();
				else if(strStrategyName == "MILITARYAISTRATEGY_MINOR_CIV_THREAT_ELEVATED")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_MinorCivThreatElevated(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_MINOR_CIV_THREAT_CRITICAL")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_MinorCivThreatCritical(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_ERADICATE_BARBARIANS")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EradicateBarbarians(eStrategy, m_pPlayer, m_iBarbarianCampCount, m_iVisibleBarbarianCount);
				else if(strStrategyName == "MILITARYAISTRATEGY_ERADICATE_BARBARIANS_CRITICAL")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EradicateBarbariansCritical(eStrategy, m_pPlayer, m_iBarbarianCampCount, m_iVisibleBarbarianCount);
				else if(strStrategyName == "MILITARYAISTRATEGY_WINNING_WARS")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_WinningWars(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_LOSING_WARS")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_LosingWars(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_RANGED" && !MOD_AI_UNIT_PRODUCTION)
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedRangedUnits(m_pPlayer, m_iNumRangedLandUnits, m_iNumMeleeLandUnits);
				else if(strStrategyName == "MILITARYAISTRATEGY_ENOUGH_RANGED" && !MOD_AI_UNIT_PRODUCTION)
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughRangedUnits(m_pPlayer, m_iNumRangedLandUnits, m_iNumMeleeLandUnits);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_RANGED_EARLY")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedRangedDueToEarlySneakAttack(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_MOBILE")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedMobileUnits(m_pPlayer, m_iNumMobileLandUnits, m_iNumMeleeLandUnits);
				else if(strStrategyName == "MILITARYAISTRATEGY_ENOUGH_MOBILE")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughMobileUnits(m_pPlayer, m_iNumMobileLandUnits, m_iNumMeleeLandUnits);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_AIR")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedAirUnits(m_pPlayer, m_iNumAirUnits, m_iNumMeleeLandUnits);
				else if(strStrategyName == "MILITARYAISTRATEGY_ENOUGH_AIR")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughAirUnits(m_pPlayer, m_iNumAirUnits, m_iNumMeleeLandUnits);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_NUKE")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedANuke(m_pPlayer);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_ANTIAIR")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedAntiAirUnits(m_pPlayer, m_iNumAntiAirUnits, m_iNumMeleeLandUnits);
				else if(strStrategyName == "MILITARYAISTRATEGY_ENOUGH_ANTIAIR")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughAntiAirUnits(m_pPlayer, m_iNumAntiAirUnits, m_iNumMeleeLandUnits);
				else if(strStrategyName == "MILITARYAISTRATEGY_NEED_AIR_CARRIER")
					bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedAirCarriers(m_pPlayer);

				// MOD_AI_UNIT_PRODUCTION : New strategies
				else if (MOD_AI_UNIT_PRODUCTION)
				{
					if (strStrategyName == "MILITARYAISTRATEGY_NEED_ARCHER")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedArcherUnits(m_pPlayer, m_iNumArcherLandUnits, m_iNumMeleeLandUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_ENOUGH_ARCHER")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughArcherUnits(m_pPlayer, m_iNumArcherLandUnits, m_iNumMeleeLandUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_NEED_SIEGE")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedSiegeUnits(m_pPlayer, m_iNumSiegeLandUnits, m_iNumMeleeLandUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_ENOUGH_SIEGE")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughSiegeUnits(m_pPlayer, m_iNumSiegeLandUnits, m_iNumMeleeLandUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_NEED_SKIRMISHER")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedSkirmisherUnits(m_pPlayer, m_iNumSkirmisherLandUnits, m_iNumMeleeLandUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_ENOUGH_SKIRMISHER")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughSkirmisherUnits(m_pPlayer, m_iNumSkirmisherLandUnits, m_iNumMeleeLandUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_NEED_NAVAL_MELEE")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedNavalMeleeUnits(m_pPlayer, m_iNumMeleeNavalUnits, m_iNumNavalUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_ENOUGH_NAVAL_MELEE")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughNavalMeleeUnits(m_pPlayer, m_iNumMeleeNavalUnits, m_iNumNavalUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_NEED_NAVAL_RANGED")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedNavalRangedUnits(m_pPlayer, m_iNumRangedNavalUnits, m_iNumNavalUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_ENOUGH_NAVAL_RANGED")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughNavalRangedUnits(m_pPlayer, m_iNumRangedNavalUnits, m_iNumNavalUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_NEED_SUBMARINE")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedSubmarineUnits(m_pPlayer, m_iNumSubmarineNavalUnits, m_iNumNavalUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_ENOUGH_SUBMARINE")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughSubmarineUnits(m_pPlayer, m_iNumSubmarineNavalUnits, m_iNumNavalUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_NEED_BOMBER")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedBomberUnits(m_pPlayer, m_iNumBomberAirUnits, m_iNumAirUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_ENOUGH_BOMBER")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughBomberUnits(m_pPlayer, m_iNumBomberAirUnits, m_iNumAirUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_NEED_FIGHTER")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_NeedFighterUnits(m_pPlayer, m_iNumFighterAirUnits, m_iNumAirUnits);
					else if (strStrategyName == "MILITARYAISTRATEGY_ENOUGH_FIGHTER")
						bStrategyShouldBeActive = MilitaryAIHelpers::IsTestStrategy_EnoughFighterUnits(m_pPlayer, m_iNumFighterAirUnits, m_iNumAirUnits);

				}
				// Never found it?  Assume it is scenario-specific and should be on unless disabled by Lua hook
				else
				{
					bStrategyShouldBeActive = true;
				}

				// Check Lua hook
				ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
				if(pkScriptSystem && bStrategyShouldBeActive)
				{
					CvLuaArgsHandle args;
					args->Push(iStrategiesLoop);
					args->Push(m_pPlayer->GetID());

					// Attempt to execute the game events.
					// Will return false if there are no registered listeners.
					bool bResult = false;
					if(LuaSupport::CallTestAll(pkScriptSystem, "MilitaryStrategyCanActivate", args.get(), bResult))
					{
						// Check the result.
						if(!bResult)
						{
							bStrategyShouldBeActive = false;
						}
					}
				}
			}

			// This variable keeps track of whether or not we should be doing something (i.e. Strategy is active now but should be turned off, OR Strategy is inactive and should be enabled)
			bool bAdoptOrEndStrategy = false;

			// Strategy should be on, and if it's not, turn it on
			if(bStrategyShouldBeActive)
			{
				if(bTestStrategyStart)
					bAdoptOrEndStrategy = true;
				else if(bTestStrategyEnd)
					bAdoptOrEndStrategy = false;
			}
			// Strategy should be off, and if it's not, turn it off
			else
			{
				if(bTestStrategyStart)
					bAdoptOrEndStrategy = false;
				else if(bTestStrategyEnd)
					bAdoptOrEndStrategy = true;
			}

			// Flavor propagation
			if(bAdoptOrEndStrategy)
			{
				int iFlavorLoop = 0;

				// We should adopt this Strategy
				if(bTestStrategyStart)
				{
					SetUsingStrategy(eStrategy, true);

					for(iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
					{
						m_aiTempFlavors[iFlavorLoop] = pStrategy->GetPlayerFlavorValue(iFlavorLoop);
					}

					GetPlayer()->GetFlavorManager()->ChangeActivePersonalityFlavors(m_aiTempFlavors, pStrategy->GetType(), true);

					for(iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
					{
						m_aiTempFlavors[iFlavorLoop] = pStrategy->GetCityFlavorValue(iFlavorLoop);
					}

					GetPlayer()->GetFlavorManager()->ChangeCityFlavors(m_aiTempFlavors, pStrategy->GetType(), true);

					if(pStrategy->RequiresCitySpecializationUpdate())
						GetPlayer()->GetCitySpecializationAI()->SetSpecializationsDirty(SPECIALIZATION_UPDATE_STRATEGY_NOW_ON);
				}
				// End the Strategy
				else if(bTestStrategyEnd)
				{
					SetUsingStrategy(eStrategy, false);

					for(iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
					{
						m_aiTempFlavors[iFlavorLoop] = -pStrategy->GetPlayerFlavorValue(iFlavorLoop);
					}

					GetPlayer()->GetFlavorManager()->ChangeActivePersonalityFlavors(m_aiTempFlavors, pStrategy->GetType(), false);

					for(iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
					{
						m_aiTempFlavors[iFlavorLoop] = -pStrategy->GetCityFlavorValue(iFlavorLoop);
					}

					GetPlayer()->GetFlavorManager()->ChangeCityFlavors(m_aiTempFlavors, pStrategy->GetType(), false);

					if(pStrategy->RequiresCitySpecializationUpdate())
						GetPlayer()->GetCitySpecializationAI()->SetSpecializationsDirty(SPECIALIZATION_UPDATE_STRATEGY_NOW_OFF);
				}
			}
		}
	}
}

void CvMilitaryAI::DoNuke(PlayerTypes ePlayer)
{
	// if we need to nuke the Barbarians, we've already lost this game 10x over
	if (ePlayer == BARBARIAN_PLAYER)
		return;

	bool bLaunchNuke = false;
	StrengthTypes eMilitaryStrength = m_pPlayer->GetDiplomacyAI()->GetRawMilitaryStrengthComparedToUs(ePlayer);
	WarStateTypes eCurrentWarState = m_pPlayer->GetDiplomacyAI()->GetWarState(ePlayer);

	// only evaluate nukes when we have nukes
	if (m_pPlayer->getNumNukeUnits() > 0) 
	{
		// if we will surely lose this war anyway, we might as well nuke them!
		if (eMilitaryStrength == STRENGTH_IMMENSE || eCurrentWarState == WAR_STATE_NEARLY_DEFEATED)
		{
			bLaunchNuke = true;
		}
		else if (GET_PLAYER(ePlayer).isMajorCiv())
		{
			// if one of us has already nuked the other...well, I'll keep the trend going!
			if (m_pPlayer->GetDiplomacyAI()->GetNumTimesNuked(ePlayer) > 0 || GET_PLAYER(ePlayer).GetDiplomacyAI()->GetNumTimesNuked(m_pPlayer->GetID()) > 0)
			{
				bLaunchNuke = true;
			}
			else
			{
				bool bRollForNuke = false;
				CivOpinionTypes eCivOpinion = m_pPlayer->GetDiplomacyAI()->GetCivOpinion(ePlayer);
				if (eMilitaryStrength == STRENGTH_POWERFUL || eCurrentWarState <= WAR_STATE_TROUBLED)
				{
					// roll every turn
					bRollForNuke = true;
				}
				else if (eCivOpinion <= CIV_OPINION_ENEMY)
				{
					bRollForNuke = true;
				}
				else if (m_pPlayer->GetDiplomacyAI()->IsGoingForWorldConquest() || m_pPlayer->GetDiplomacyAI()->IsCloseToWorldConquest())
				{
					bRollForNuke = true;
				}
				else if (GET_PLAYER(ePlayer).GetDiplomacyAI()->IsCloseToAnyVictoryCondition())
				{
					bRollForNuke = true;
				}
				if (bRollForNuke)
				{
					int iFlavorNuke = m_pPlayer->GetFlavorManager()->GetPersonalityFlavorForDiplomacy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_USE_NUKE"));
					int iRoll = GC.getGame().randRangeExclusive(0, 10, m_pPlayer->GetPseudoRandomSeed().mix(GET_PLAYER(ePlayer).GetPseudoRandomSeed()));
					if (iRoll <= iFlavorNuke)
					{
						bLaunchNuke = true;
					}
				}
			}
		}
	}

	if (bLaunchNuke)
		RequestNukeAttack(ePlayer);
}

void CvMilitaryAI::SetupInstantDefenses(PlayerTypes ePlayer)
{
	if(ePlayer == NO_PLAYER)
		return;

	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strTemp;

		// Open the right file
		CvString strPlayerName = GetPlayer()->getCivilizationShortDescription();
		CvString strOpponentName = GET_PLAYER(ePlayer).getCivilizationShortDescription();
		FILogFile *pLog = LOGFILEMGR.GetLog(GetLogFileName(strPlayerName), FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += strPlayerName + ", ";

		// Strategy Info
		strTemp.Format("War Declared! ***** %s ****** Setting up defenses.", strOpponentName.GetCString());
		strOutBuf = strBaseString + strTemp;
		pLog->Msg(strOutBuf);
	}

	//land response
	vector<CvCity*> allCities = m_pPlayer->GetThreatenedCities(false);
	CvCity* pMostThreatenedCity = allCities.empty() ? NULL : allCities.front();
	if (pMostThreatenedCity != NULL && !m_pPlayer->getFirstAIOperationOfType(AI_OPERATION_RAPID_RESPONSE, NO_PLAYER, pMostThreatenedCity->plot()))
		m_pPlayer->addAIOperation(AI_OPERATION_RAPID_RESPONSE, 0, ePlayer, pMostThreatenedCity);

	//naval response
	vector<CvCity*> coastCities = m_pPlayer->GetThreatenedCities(true);
	CvCity* pMostThreatenedCoastalCity = coastCities.empty() ? NULL : coastCities.front();
	if (pMostThreatenedCoastalCity != NULL && !m_pPlayer->getFirstAIOperationOfType(AI_OPERATION_NAVAL_SUPERIORITY, NO_PLAYER, pMostThreatenedCoastalCity->plot()))
		m_pPlayer->addAIOperation(AI_OPERATION_NAVAL_SUPERIORITY, 0, ePlayer, pMostThreatenedCoastalCity);
}

/// Abort or start operations as appropriate given the current threats and war states
void CvMilitaryAI::CheckLandDefenses(PlayerTypes eEnemy, CvCity* pThreatenedCity)
{
	if(eEnemy == NO_PLAYER || !pThreatenedCity)
		return;
	
	WarStateTypes eWarState = GET_PLAYER(eEnemy).isMajorCiv() ? m_pPlayer->GetDiplomacyAI()->GetWarState(eEnemy) : WAR_STATE_OFFENSIVE;

	if (eWarState >= WAR_STATE_STALEMATE)
	{
		// If we're winning, nothing to do
		// AI_AB_TEST: the new AI still defends a threatened city when the war as a whole looks fine
		if (!m_pPlayer->UsesNewAI())
			return;
	}
	else if (eWarState == WAR_STATE_DEFENSIVE)
		// If we are losing, concentrate on defense against this player
		m_pPlayer->StopAllLandOffensiveOperationsAgainstPlayer(eEnemy, AI_ABORT_WAR_STATE_CHANGE);
	else if (eWarState == WAR_STATE_NEARLY_DEFEATED)
		// If we are really losing, let's pull back everywhere.	
		m_pPlayer->StopAllLandOffensiveOperationsAgainstPlayer(NO_PLAYER, AI_ABORT_WAR_STATE_CHANGE);

	//first a quick one if necessary
	bool bHasOperationUnderway = m_pPlayer->getFirstAIOperationOfType(AI_OPERATION_RAPID_RESPONSE, NO_PLAYER, pThreatenedCity->plot()) != NULL;
	CvPlot* pStartPlot = OperationalAIHelpers::FindEnemiesNearHomelandPlot(m_pPlayer->GetID(), eEnemy, DOMAIN_LAND, pThreatenedCity->plot(), 8);
	if (!bHasOperationUnderway && pStartPlot != NULL && pStartPlot->getOwningCity() != NULL)
		m_pPlayer->addAIOperation(AI_OPERATION_RAPID_RESPONSE, 1, eEnemy, pStartPlot->getOwningCity());

	//this may take a bit longer to arrange
	bool bIsEnemyZone = m_pPlayer->GetTacticalAI()->GetTacticalAnalysisMap()->IsInEnemyDominatedZone(pThreatenedCity->plot());
	if (bIsEnemyZone && m_pPlayer->getFirstAIOperationOfType(AI_OPERATION_CITY_DEFENSE, NO_PLAYER, pThreatenedCity->plot())==NULL)
		m_pPlayer->addAIOperation(AI_OPERATION_CITY_DEFENSE, 2, eEnemy, pThreatenedCity);
}

/// Abort or start operations as appropriate given the current threats and war states
void CvMilitaryAI::CheckSeaDefenses(PlayerTypes ePlayer, CvCity* pThreatenedCity)
{
	if(ePlayer == NO_PLAYER || !pThreatenedCity)
		return;

	WarStateTypes eWarState = GET_PLAYER(ePlayer).isMajorCiv() ? m_pPlayer->GetDiplomacyAI()->GetWarState(ePlayer) : WAR_STATE_OFFENSIVE;
	if (eWarState >= WAR_STATE_STALEMATE)
		return;
	else if (eWarState ==  WAR_STATE_DEFENSIVE)
		m_pPlayer->StopAllSeaOffensiveOperationsAgainstPlayer(ePlayer, AI_ABORT_WAR_STATE_CHANGE);
	else if (eWarState == WAR_STATE_NEARLY_DEFEATED)
		m_pPlayer->StopAllSeaOffensiveOperationsAgainstPlayer(NO_PLAYER, AI_ABORT_WAR_STATE_CHANGE);

	if (!pThreatenedCity->isCoastal())
		return;

	CvPlot* pCoastalPlot = MilitaryAIHelpers::GetCoastalWaterNearPlot(pThreatenedCity->plot());
	if(pCoastalPlot != NULL)
	{
		bool bIsEnemyZone = m_pPlayer->GetTacticalAI()->GetTacticalAnalysisMap()->IsInEnemyDominatedZone(pThreatenedCity->plot());
		bool bHasOperationUnderway = m_pPlayer->getFirstAIOperationOfType(AI_OPERATION_NAVAL_SUPERIORITY, NO_PLAYER, pThreatenedCity->plot()) != NULL;
		if (!bHasOperationUnderway || bIsEnemyZone)
			m_pPlayer->addAIOperation(AI_OPERATION_NAVAL_SUPERIORITY, 1, ePlayer, pThreatenedCity);
	}
}

void CvMilitaryAI::DoCityAttacks(PlayerTypes ePlayer)
{
	//Not perfect, as some operations are mixed, but it will keep us from sending everyone to slaughter all at once.
	int iReservesTotal = ((m_iNumLandUnits + m_iNumNavalUnits) - (m_iNumNavalUnitsInArmies + m_iNumLandUnitsInArmies));
	int iNumOffensiveOps = m_pPlayer->GetNumOffensiveOperations(DOMAIN_LAND) + m_pPlayer->GetNumOffensiveOperations(DOMAIN_SEA);
	bool bCanStartAttack = (iReservesTotal >= m_iRecLandUnits || iNumOffensiveOps <= 0);
	//AI_AB_TEST: m_iRecLandUnits is never below our land unit count, so CP's first condition almost never holds and
	//only one attack runs at a time across all enemies. The new AI may run a second one when its spare land units
	//cover a garrison per city plus a small army.
	if (!bCanStartAttack && m_pPlayer->UsesNewAI())
	{
		int iLandReserves = m_iNumLandUnits - m_iNumLandUnitsInArmies;
		bCanStartAttack = iNumOffensiveOps < 2 && iLandReserves >= m_pPlayer->getNumCities() + 4;
	}
	if (bCanStartAttack)
	{
		WarStateTypes eWarState = GET_PLAYER(ePlayer).isMajorCiv() ? m_pPlayer->GetDiplomacyAI()->GetWarState(ePlayer) : WAR_STATE_OFFENSIVE;
		if (eWarState >= WAR_STATE_TROUBLED)
		{
			RequestCityAttack(ePlayer, 2);
			if (GET_PLAYER(ePlayer).isMajorCiv())
				RequestPillageAttack(ePlayer);
		}
	}
}

/// Abort or start operations as appropriate given the current threats and war states
void CvMilitaryAI::UpdateOperations()
{
	//only major players set up operations
	if(!m_pPlayer->isMajorCiv())
		return;

	vector<CvCity*> allCities = m_pPlayer->GetThreatenedCities(false);
	CvCity* pThreatenedCityA = allCities.size()<1 ? NULL : allCities[0];
	CvCity* pThreatenedCityB = allCities.size()<2 ? NULL : allCities[1];
	vector<CvCity*> coastCities = m_pPlayer->GetThreatenedCities(true);
	CvCity* pThreatenedCoastalCityA = coastCities.size()<1 ? NULL : coastCities[0];
	CvCity* pThreatenedCoastalCityB = coastCities.size()<2 ? NULL : coastCities[1];

	// Are any of our strategies inappropriate given the type of war we are fighting
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_PLAYERS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;

		// Is this a player we have relations with?
		if(eLoopPlayer != m_pPlayer->GetID() && IsPlayerValid(eLoopPlayer))
		{
			// If we've made peace, abort all operations
			if(GET_TEAM(m_pPlayer->getTeam()).isForcePeace(GET_PLAYER(eLoopPlayer).getTeam()))
			{
				m_pPlayer->StopAllLandOffensiveOperationsAgainstPlayer(eLoopPlayer,AI_ABORT_WAR_STATE_CHANGE);
				m_pPlayer->StopAllSeaOffensiveOperationsAgainstPlayer(eLoopPlayer,AI_ABORT_WAR_STATE_CHANGE);
				m_pPlayer->StopAllLandDefensiveOperationsAgainstPlayer(eLoopPlayer,AI_ABORT_WAR_STATE_CHANGE);
				m_pPlayer->StopAllSeaDefensiveOperationsAgainstPlayer(eLoopPlayer,AI_ABORT_WAR_STATE_CHANGE);
			}

			// If we cannot declare war on this player, abort all offensive operations related to him
			if(!m_pPlayer->IsAtWarWith(eLoopPlayer) && !GET_TEAM(m_pPlayer->getTeam()).canDeclareWar(GET_PLAYER(eLoopPlayer).getTeam()))
			{
				m_pPlayer->StopAllLandOffensiveOperationsAgainstPlayer(eLoopPlayer,AI_ABORT_TARGET_NOT_VALID);
				m_pPlayer->StopAllSeaOffensiveOperationsAgainstPlayer(eLoopPlayer,AI_ABORT_TARGET_NOT_VALID);
			}

			// if we're at war with this player
			if (m_pPlayer->IsAtWarWith(eLoopPlayer))
			{
				//Defense check.
				if (pThreatenedCoastalCityA == NULL)
					m_pPlayer->StopAllSeaDefensiveOperationsAgainstPlayer(eLoopPlayer, AI_ABORT_WAR_STATE_CHANGE);

				if(pThreatenedCityA == NULL)
					m_pPlayer->StopAllLandDefensiveOperationsAgainstPlayer(eLoopPlayer, AI_ABORT_WAR_STATE_CHANGE);

				CheckLandDefenses(eLoopPlayer,pThreatenedCityA);
				CheckLandDefenses(eLoopPlayer,pThreatenedCityB);

				CheckSeaDefenses(eLoopPlayer, pThreatenedCoastalCityA);
				CheckSeaDefenses(eLoopPlayer, pThreatenedCoastalCityB);

				//finally offense
				DoNuke(eLoopPlayer);
				DoCityAttacks(eLoopPlayer);
			}
		}
	}

	//if we have an idle carrier, try to start a strike group. the carrier may be empty!
	int iLoop = 0;
	for(CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
	{
		if(pLoopUnit->AI_getUnitAIType() == UNITAI_CARRIER_SEA && pLoopUnit->getArmyID() == -1 && !pLoopUnit->shouldHeal(false))
		{
			//the operation will find it's own target and remain active until indefinitely.
			//the airplanes rebase independently, they are not part of the operation.
			m_pPlayer->addAIOperation(AI_OPERATION_CARRIER_GROUP, 0, NO_PLAYER, NULL, NULL);
			break;
		}
	}
}

/// Spend money on units/buildings for military contingencies
//  NOTE: The defensive side of this is done in dominance zone processing in the Tactical AI; this is spending to speed operations
void CvMilitaryAI::MakeEmergencyPurchases()
{

	// Are we winning all the wars we are in?
	static MilitaryAIStrategyTypes eStrategyAtWar = (MilitaryAIStrategyTypes) GC.getInfoTypeForString("MILITARYAISTRATEGY_AT_WAR");
	if(!IsUsingStrategy(eStrategyAtWar) || m_pPlayer->GetDiplomacyAI()->GetStateAllWars() == STATE_ALL_WARS_WINNING)
	{
		// Is there an operation waiting for one more unit?
		for (size_t i=0; i<m_pPlayer->getNumAIOperations(); i++)
		{
			CvAIOperation* pOp = m_pPlayer->getAIOperationByIndex(i);
			// Can we buy a unit to fill that slot?
			if(pOp->BuyFinalUnit())
				break;
		}
	}
}

/// Delete older units no longer needed by military AI
void CvMilitaryAI::DisbandObsoleteUnits()
{
	bool bInDeficit = false;
	bool bConquestGrandStrategy = false;

	if (m_pPlayer->isBarbarian())
		return;

	if (GC.getGame().getGameTurn() <= 25)
		return;

	// delete low-life units that can't heal
	int iUnitLoop = 0;
	for (CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iUnitLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iUnitLoop))
	{
		if (pLoopUnit->IsCannotHeal(/*bConsiderResourceShortage*/ false) && !pLoopUnit->isDelayedDeath() && pLoopUnit->canScrap() && !pLoopUnit->HasPlague())
		{
			if (pLoopUnit->GetCurrHitPoints() < 75 || (GetNumberCivsAtWarWith(m_pPlayer->isMinorCiv()) > 0 && pLoopUnit->GetCurrHitPoints() < 25))
				pLoopUnit->scrap();
		}
	}

	// Don't do this if at war
	if (GetNumberCivsAtWarWith(m_pPlayer->isMinorCiv()) > 0)
	{
		if (m_pPlayer->isMinorCiv())
			return;

		if (m_pPlayer->isMajorCiv() && m_pPlayer->GetDiplomacyAI()->GetStateAllWars() == STATE_ALL_WARS_LOSING)
			return;
	}

	if (m_pPlayer->isMinorCiv())
	{
		if (m_pPlayer->GetMinorCivAI()->GetNumThreateningBarbarians() > 0)
			return;
	}
	else
	{
		// Are we running at a deficit?
		static EconomicAIStrategyTypes eStrategyLosingMoney = (EconomicAIStrategyTypes)GC.getInfoTypeForString("ECONOMICAISTRATEGY_LOSING_MONEY");
		bInDeficit = m_pPlayer->GetEconomicAI()->IsUsingStrategy(eStrategyLosingMoney);
		//AI_AB_TEST (diagnostic flag BigArmy): no deficit scrapping or gifting while the BigArmy plan runs; test only
		if (IsBigArmyUpkeepWaived())
			bInDeficit = false;
		// Are we running anything other than the Conquest Grand Strategy?
		if (m_pPlayer->GetDiplomacyAI()->IsGoingForWorldConquest())
		{
			bConquestGrandStrategy = true;
		}
	}

	// do we have way too many units?
	int iMaxExcessUnits = bConquestGrandStrategy ? 3 : 1;
	bool bOverSupplyCap = (m_pPlayer->GetNumUnitsOutOfSupply(false) > iMaxExcessUnits);

	CvUnit* pScrapUnit = FindUselessShip();

	//AI_AB_TEST (candidate, flag ArmyScrap, War): a timing attacker short of gold drops a unit too weak to count as army
	if (!pScrapUnit && m_pPlayer->UsesCandidateAI(FIX_ARMY_SCRAP))
		pScrapUnit = FindArmyScrapUnit();

	//if we don't have an easy candidate but are forced to act
	if (!pScrapUnit && (bInDeficit||bOverSupplyCap))
	{
		//check our options
		int iLandScore = MAX_INT;
		int iNavalScore = MAX_INT;
		CvUnit* pNavalUnit = FindUnitToScrap(DOMAIN_SEA, true, iNavalScore, bOverSupplyCap, bInDeficit);
		CvUnit* pLandUnit = FindUnitToScrap(DOMAIN_LAND, true, iLandScore, bOverSupplyCap, bInDeficit);

		//keep the more useful one
		pScrapUnit = (iLandScore < iNavalScore) ? pLandUnit : pNavalUnit;
	}

	if (pScrapUnit)
	{
		// Don't do this if we're a minor civ
		if (!m_pPlayer->isMinorCiv())
		{
			PlayerTypes eMinor = m_pPlayer->GetBestGiftTarget(pScrapUnit->getDomainType());
			if (eMinor != NO_PLAYER)
			{
				LogGiftUnit(pScrapUnit, bInDeficit, bOverSupplyCap);
				GET_PLAYER(eMinor).AddIncomingUnit(m_pPlayer->GetID(), pScrapUnit);
				pScrapUnit = NULL;
			}
		}

		if (pScrapUnit)
		{
			LogScrapUnit(pScrapUnit, bInDeficit, bOverSupplyCap);
			if (pScrapUnit->canScrap())
				pScrapUnit->scrap();
			else
				pScrapUnit->kill(true);
		}
	}
}

CvUnit* CvMilitaryAI::FindUselessShip()
{
	vector<CvUnit*> candidates;

	int iUnitLoop = 0;
	for (CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iUnitLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iUnitLoop))
	{
		if (!pLoopUnit->IsCombatUnit())
			continue;

		if (!pLoopUnit->canScrap())
			continue;

		// Is this a ship on a water body without enemies and without exits?
		if (pLoopUnit->getDomainType() == DOMAIN_SEA)
		{
			//check the current area first
			if (MilitaryAIHelpers::NeedShipInArea(m_pPlayer->GetID(), pLoopUnit->plot()->landmass()))
				continue;

			candidates.push_back(pLoopUnit);
		}
	}

	//sort ships by experience: we want to scrap the veterans last ...
	struct PrSortByExperience
	{
		bool operator()(const CvUnit* lhs, const CvUnit* rhs) const { return lhs->getExperienceTimes100() < rhs->getExperienceTimes100(); }
	};
	std::stable_sort( candidates.begin(), candidates.end(), PrSortByExperience() );

	//check other areas we can reach before deciding
	for (size_t i=0; i<candidates.size(); i++)
	{
		CvUnit* pLoopUnit = candidates[i];

		//assume it's useless until proven otherwise
		bool bIsUseless = true;

		//two turns is a good compromise between reliability and performance 
		SPathFinderUserData data(pLoopUnit, CvUnit::MOVEFLAG_IGNORE_STACKING_SELF | CvUnit::MOVEFLAG_IGNORE_ENEMIES | CvUnit::MOVEFLAG_IGNORE_RIGHT_OF_PASSAGE, 2);
		ReachablePlots plots = GC.GetPathFinder().GetPlotsInReach(pLoopUnit->getX(), pLoopUnit->getY(), data);
		set<int> landmasses;

		for (ReachablePlots::iterator it = plots.begin(); it != plots.end(); ++it)
		{
			CvPlot* pPlot = GC.getMap().plotByIndexUnchecked(it->iPlotIndex);
			if (pPlot && pPlot->isWater())
				landmasses.insert(pPlot->getLandmass());
		}
		for (set<int>::iterator it = landmasses.begin(); it != landmasses.end(); ++it)
		{
			CvLandmass* pWaterBody = GC.getMap().getLandmassById(*it);
			if (MilitaryAIHelpers::NeedShipInArea(m_pPlayer->GetID(), pWaterBody))
			{
				bIsUseless = false;
				break;
			}
		}

		if (bIsUseless)
			return pLoopUnit;
	}

	return NULL;
}

/// Score the strength of the units for a domain; best candidate to scrap (with lowest score) is returned. Only supports land and naval units
/// AI_AB_TEST (candidate, flag ArmyScrap, War): without BigArmy's waiver the attacker's gold holds its army at 9 to 10 land
/// units through 40 turns of war (f6d1e84, 850e142), while 22% of the armies were Warriors and Scouts (BigArmy, at the first
/// declaration). In the window or the attack phase (not at war), when the treasury would run dry in under 9 turns (the window's pause test), give
/// away or disband one land unit a turn below ArmyQuality's floor: not siege, not a garrison, not in an army; the weakest.
CvUnit* CvMilitaryAI::FindArmyScrapUnit() const
{
	//not at war (the lead, 6:39pm ET): over the first 5 package games all 7 drops came at war, none in the window, taking
	//bodies out of the fight they were built for; the upkeep it frees is for building the army, before the war
	if (!IsTimingBuildupPlayer() || GetTimingBuildupPhase() < TIMING_BUILDUP_WINDOW || GetTimingBuildupPhase() >= TIMING_BUILDUP_WAR ||
		m_pPlayer->IsAtWarAnyMajor())
		return NULL;
	const int iAverageGoldPerUnit = m_pPlayer->GetTreasury()->GetExpensePerTurnUnitMaintenance() / max(1, m_pPlayer->getNumUnits());
	if (m_pPlayer->getTurnsToBankruptcy(iAverageGoldPerUnit) >= 9)
		return NULL;
	const bool bReconNeeded = m_pPlayer->GetEconomicAI()->GetReconState() == RECON_STATE_NEEDED;
	CvUnit* pBest = NULL;
	int iBestScore = MAX_INT, iLoop = 0;
	for (CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
	{
		if (pLoopUnit->getDomainType() != DOMAIN_LAND || !pLoopUnit->IsCombatUnit() || !pLoopUnit->canScrap() || pLoopUnit->isDelayedDeath())
			continue;
		if (IsStrikeKeepingUnit(pLoopUnit))	// the strike's ownership, gate 5
			continue;
		if (pLoopUnit->AI_getUnitAIType() == UNITAI_CITY_BOMBARD || pLoopUnit->IsGarrisoned() || pLoopUnit->getArmyID() != -1 || pLoopUnit->IsNoMaintenance())
			continue;
		if (pLoopUnit->AI_getUnitAIType() == UNITAI_EXPLORE && bReconNeeded)
			continue;
		if (IsArmyQualityUnit(&pLoopUnit->getUnitInfo()))
			continue;
		//not a unit waiting for its upgrade (the lead: Archers waiting for Composite Bowmen are the strike's army)
		const UnitTypes eUpgrade = pLoopUnit->GetUpgradeUnitType();
		if (eUpgrade != NO_UNIT && m_pPlayer->HasResourceForNewUnit(eUpgrade, false, true, pLoopUnit->getUnitType()))
		{
			const TechTypes eUpgradeTech = (TechTypes)GC.getUnitInfo(eUpgrade)->GetPrereqAndTech();
			if (eUpgradeTech == NO_TECH || GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech(eUpgradeTech) ||
				eUpgradeTech == m_pPlayer->GetPlayerTechs()->GetCurrentResearch())
				continue;
		}
		const int iScore = pLoopUnit->GetPower() * pLoopUnit->getUnitInfo().GetProductionCost() + pLoopUnit->getLevel();
		if (iScore < iBestScore)
		{
			iBestScore = iScore;
			pBest = pLoopUnit;
		}
	}
	if (pBest && GC.getLogging() && GC.getAILogging())
		m_pPlayer->GetTacticalAI()->LogTacticalMessage(CvString::format("ArmyScrap: dropping %s at (%d:%d), below the army floor (%d melee, %d ranged), gold %d",
			pBest->getName().c_str(), pBest->getX(), pBest->getY(), GetArmyQualityFloor(false), GetArmyQualityFloor(true), m_pPlayer->GetTreasury()->GetGold()));
	return pBest;
}

CvUnit* CvMilitaryAI::FindUnitToScrap(DomainTypes eDomain, bool bCheckObsolete, int& iReturnedScore, bool bOverSupplyCap, bool bInDeficit)
{
	int iUnitLoop = 0;
	CvUnit* pBestUnit = NULL;
	int iBestScore = MAX_INT;

	for (CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iUnitLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iUnitLoop))
	{
		//needed later
		CvUnitEntry& pUnitInfo = pLoopUnit->getUnitInfo();

		if(!pLoopUnit->IsCombatUnit())
			continue;

		if (!pLoopUnit->canScrap())
			continue;

		// AI_AB_TEST (the strike's ownership, gate 5; the lead's audit): not a strike unit while the strike is on (real
		// bankruptcy, gold below 0, is exempt in IsStrikeKeepingUnit)
		if (IsStrikeKeepingUnit(pLoopUnit))
			continue;

		// don't delete no maintenance units if we're scrapping due to deficit
		// don't delete no supply units if we're scrapping due to oversupply
		if (bInDeficit && bOverSupplyCap)
		{
			if (pLoopUnit->IsNoMaintenance() && pLoopUnit->isNoSupply())
				continue;
		}
		else if (bInDeficit && pLoopUnit->IsNoMaintenance())
			continue;
		else if (bOverSupplyCap && pLoopUnit->isNoSupply())
			continue;

		//Failsafe to keep AI from deleting advanced start settlers
		//Probably useless because of the combat unit check above
		if (m_pPlayer->GetNumCitiesFounded() < 3)
			if (pUnitInfo.IsFound() || pUnitInfo.IsFoundAbroad())
				continue;

		if(eDomain != NO_DOMAIN && pLoopUnit->getDomainType() != eDomain)
			continue;

		//don't kill our explorers if we need them
		if (pLoopUnit->AI_getUnitAIType() == UNITAI_EXPLORE_SEA && m_pPlayer->GetEconomicAI()->GetNavalReconState() == RECON_STATE_NEEDED)
			continue;

		//don't kill our explorers if we need them
		if (pLoopUnit->AI_getUnitAIType() == UNITAI_EXPLORE && m_pPlayer->GetEconomicAI()->GetReconState() == RECON_STATE_NEEDED)
			continue;

		// Do we need it to fight?
		if (!pLoopUnit->canUseForAIOperation())
			continue;

		bool bIsObsolete = (TechTypes)pUnitInfo.GetObsoleteTech() != NO_TECH && GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech((TechTypes)(pUnitInfo.GetObsoleteTech()));
		if (bIsObsolete && bCheckObsolete)
		{
			UnitTypes eUpgradeUnit = pLoopUnit->GetUpgradeUnitType();
			if (eUpgradeUnit != NO_UNIT)
			{
				// Ok nice looks like we can upgrade
				bIsObsolete = false;

				// But do we have the resources for it?
				if (!m_pPlayer->HasResourceForNewUnit(eUpgradeUnit, false, true, pLoopUnit->getUnitType()))
					bIsObsolete = true;
			}
		}

		bool bResourceDeficit = false;
		for (int iResourceLoop = 0; iResourceLoop < GC.getNumResourceInfos(); iResourceLoop++)
		{
			ResourceTypes eResource = (ResourceTypes)iResourceLoop;
			int iNumResourceInUse = pLoopUnit->getUnitInfo().GetResourceQuantityRequirement(eResource);
			if (iNumResourceInUse > 0 && m_pPlayer->getNumResourceAvailable(eResource) < 0)
			{
				bResourceDeficit = true;
				break;
			}
		}

		// Can I scrap this unit?
		if (!bCheckObsolete || bIsObsolete || bResourceDeficit)
		{
			int iScore = pLoopUnit->GetPower()*pLoopUnit->getUnitInfo().GetProductionCost() + pLoopUnit->getLevel(); //tiebreaker
			if (bResourceDeficit)
				iScore /= 2;

			if (iScore < iBestScore)
			{
				iBestScore = iScore;
				iReturnedScore = iBestScore;
				pBestUnit = pLoopUnit;
			}
		}
	}

	return pBestUnit;
}

bool CvMilitaryAI::IsBuildingArmy(ArmyType eType) const
{
	int iLoop = 0;
	for (CvArmyAI* pLoopArmyAI = m_pPlayer->firstArmyAI(&iLoop); pLoopArmyAI != NULL; pLoopArmyAI = m_pPlayer->nextArmyAI(&iLoop))
	{
		if (pLoopArmyAI->GetType() == eType || eType == ARMY_TYPE_ANY)
			if (!pLoopArmyAI->GetOpenSlots(true).empty())
				return true;
	}

	return false;
}

/// AI_AB_TEST: can a unit trained in this city join this army? Only armies still recruiting take new units, and they
/// only pick up units that can reach the muster point in time, so the city must be close and able to get there.
bool CvMilitaryAI::CanCityHelpFillArmy(const CvCity* pCity, const CvArmyAI* pArmy) const
{
	if (!pCity || !pArmy || pArmy->GetArmyAIState() != ARMYAISTATE_WAITING_FOR_UNITS_TO_REINFORCE)
		return false;

	const CvAIOperation* pOp = m_pPlayer->getAIOperation(pArmy->GetOperationID());
	CvPlot* pMuster = pOp ? pOp->GetMusterPlot() : NULL;
	if (!pMuster)
		return false;

	const int iMaxDistance = 8;
	if (plotDistance(*pCity->plot(), *pMuster) > iMaxDistance)
		return false;

	return pCity->HasAccessToLandmassOrOcean(pMuster->getLandmass());
}

/// AI_AB_TEST: CP lets only the muster city of the first operation needing units build for an army, so an attack
/// waits on one city's production. The new AI lets any nearby city help while the recruiting armies near it have
/// more open slots than units already being trained in other cities.
bool CvMilitaryAI::ShouldCityHelpFillArmies(const CvCity* pCity) const
{
	if (!pCity)
		return false;

	int iOpenSlots = 0;
	int iLoop = 0;
	for (CvArmyAI* pLoopArmyAI = m_pPlayer->firstArmyAI(&iLoop); pLoopArmyAI != NULL; pLoopArmyAI = m_pPlayer->nextArmyAI(&iLoop))
	{
		if (CanCityHelpFillArmy(pCity, pLoopArmyAI))
			iOpenSlots += static_cast<int>(pLoopArmyAI->GetOpenSlots(true).size());
	}
	if (iOpenSlots == 0)
		return false;

	int iBeingTrained = 0;
	for (const CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop))
	{
		if (pLoopCity == pCity || !pLoopCity->isProductionUnit())
			continue;

		CvUnitEntry* pkUnitInfo = GC.getUnitInfo(pLoopCity->getProductionUnit());
		if (pkUnitInfo && (pkUnitInfo->GetCombat() > 0 || pkUnitInfo->GetRangedCombat() > 0))
			iBeingTrained++;
	}

	return iBeingTrained < iOpenSlots;
}

/// Which unit should be built next for our army
UnitTypes CvMilitaryAI::GetUnitTypeForArmy(CvCity* pCity) const
{
	UnitTypes eBestUnit = NO_UNIT;
	size_t leastOpenSlots = INT_MAX;
	bool bNewAI = m_pPlayer->UsesNewAI();

	int iLoop = 0;
	for (CvArmyAI* pLoopArmyAI = m_pPlayer->firstArmyAI(&iLoop); pLoopArmyAI != NULL; pLoopArmyAI = m_pPlayer->nextArmyAI(&iLoop))
	{
		// AI_AB_TEST: only suggest units for armies this city's units can actually join
		if (bNewAI && !CanCityHelpFillArmy(pCity, pLoopArmyAI))
			continue;

		vector<size_t> vOpenSlots = pLoopArmyAI->GetOpenSlots(true);
		for (size_t i = 0; i < vOpenSlots.size(); i++)
		{
			CvFormationSlotEntry slot = pLoopArmyAI->GetSlotInfo(vOpenSlots[i]);
			UnitTypes eType = pCity->GetCityStrategyAI()->GetUnitProductionAI()->RecommendUnit(slot.m_primaryUnitType, true);

			if (eType == NO_UNIT)
				eType = pCity->GetCityStrategyAI()->GetUnitProductionAI()->RecommendUnit(slot.m_secondaryUnitType, true);

			//don't be greedy, try to complete the armies with the least amount of missing units first
			if (eType != NO_UNIT && vOpenSlots.size() < leastOpenSlots)
			{
				eBestUnit = eType;
				leastOpenSlots = vOpenSlots.size();
			}
		}
	}

	return eBestUnit;
}

/// Assess nearby enemy air assets
int CvMilitaryAI::GetNumEnemyAirUnitsInRange(CvPlot* pCenterPlot, int iRange, bool bCountFighters, bool bCountBombers) const
{
	int iRtnValue = 0;

	// Loop through all players' Units (that we're at war with) to see if they can intercept
	const std::vector<PlayerTypes>& vEnemies = m_pPlayer->GetPlayersAtWarWith();

	for (size_t iI = 0; iI < vEnemies.size(); iI++)
	{
		CvPlayerAI& kPlayer = GET_PLAYER(vEnemies[iI]);

		// Loop through their units looking for bombers (this will allow us to find bombers on carriers also
		int iLoopUnit = 0;
		for(CvUnit* pLoopUnit = kPlayer.firstUnit(&iLoopUnit); pLoopUnit != NULL; pLoopUnit = kPlayer.nextUnit(&iLoopUnit))
		{
			if (pLoopUnit->getDomainType() == DOMAIN_AIR)
			{
				// Just to keep fighters closer to high range bombers (stealth bombers)
				int iAcceptableDistance = min(pLoopUnit->GetRange(), 12) + (iRange / 2);
				// distance was checked to a fixed 10 value
				if (plotDistance(pCenterPlot->getX(), pCenterPlot->getY(), pLoopUnit->getX(), pLoopUnit->getY()) <= iAcceptableDistance)
				{
					// Let's not factor in revealed or visible - As a human I can remember past attacks and intuit whether a bomber could be in range of the city, AIs don't have great intuition...
					if (pLoopUnit->IsAirSweepCapable() || pLoopUnit->canAirDefend())
					{
						if (bCountFighters)
							iRtnValue++;
					}
					else
					{
						if (bCountBombers)
							iRtnValue++;
					}
				}
			}
		}
	}

	return iRtnValue;
}

/// See if this fighter has an air sweep target we like
CvPlot *CvMilitaryAI::GetBestAirSweepTarget(CvUnit* pFighter) const
{
	CvPlot *pBestTarget = NULL;
	int iBestCount = 0;

	// Loop through all the players
	for(int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
		if(kPlayer.isAlive() && kPlayer.GetID() != m_pPlayer->GetID())
		{
			if (atWar(kPlayer.getTeam(), m_pPlayer->getTeam()))
			{
				// Loop through each of their cities
				int iLoop = 0;
				CvCity* pLoopCity = NULL;
				for(pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
				{
					CvPlot *pCityPlot = pLoopCity->plot();
					if (pCityPlot->isVisible(m_pPlayer->getTeam()) && plotDistance(pFighter->getX(), pFighter->getY(), pLoopCity->getX(), pLoopCity->getY()) <= pFighter->GetRange())
					{
						if (pFighter->canAirSweepAt(pCityPlot->getX(), pCityPlot->getY()))
						{
							int iCountFighters = 0;

							for (int iUnitLoop = 0; iUnitLoop < pCityPlot->getNumUnits(); iUnitLoop++)
							{
								CvUnit *pUnit = pCityPlot->getUnitByIndex(iUnitLoop);
								{
									if (pUnit->getDomainType() == DOMAIN_AIR)
									{
										if (pUnit->IsAirSweepCapable() || pUnit->canAirDefend())
										{
											iCountFighters+=2;
										}
										else if (!pUnit->isSuicide() && !pUnit->isCargo())
										{
											iCountFighters++;
										}
									}
								}
							}

							if (iCountFighters > iBestCount)
							{
								iBestCount = iCountFighters;
								pBestTarget = pCityPlot;
							}
						}
					}
				}
			}
		}
	}

	return pBestTarget;
}

/// How strong is the best unit we can train for this domain?
int CvMilitaryAI::GetPowerOfStrongestBuildableUnit(DomainTypes eDomain)
{
	int iRtnValue = 0;
	for(int iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		const UnitTypes eUnit = static_cast<UnitTypes>(iI);
		CvUnitEntry* pkUnitEntry = GC.getUnitInfo(eUnit);
		if(pkUnitEntry != NULL && pkUnitEntry->GetDomainType() == eDomain)
		{
			int iThisPower = pkUnitEntry->GetPower();		// Test the power first, it is much less costly than testing canTrain
			if(iThisPower > iRtnValue)
			{
				if(m_pPlayer->canTrainUnit(eUnit, false /*bContinue*/, false /*bTestVisible*/, true /*bIgnoreCost*/))
				{
					iRtnValue = iThisPower;
				}
			}
		}
	}

	return iRtnValue;
}

/// Log that a strategy is being turned on or off
void CvMilitaryAI::LogStrategy(MilitaryAIStrategyTypes eStrategy, bool bValue)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strTemp;
		CvString playerName;
		CvString strLogName;
		FILogFile* pLog = NULL;

		// Open the right file - strategies are logged to the flavor log, not the military one
		playerName = GetPlayer()->getCivilizationShortDescription();
		if(GC.getPlayerAndCityAILogSplit())
		{
			strLogName = "FlavorAILog_" + playerName + ".csv";
		}
		else
		{
			strLogName = "FlavorAILog.csv";
		}
		pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += playerName + ", ";

		// Strategy Info
		CvMilitaryAIStrategyXMLEntry* pEntry = GC.getMilitaryAIStrategyInfo(eStrategy);
		const char* szType = (pEntry != NULL)? pEntry->GetType() : NULL;

		strTemp.Format("%s, %d", szType, bValue);
		strOutBuf = strBaseString + strTemp;
		pLog->Msg(strOutBuf);

		// Also - write to Other Logs, so that we know what the player is doing in there as well

		if(GC.getPlayerAndCityAILogSplit())
		{
			strLogName = "TechAILog_" + playerName + ".csv";
			pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);
			pLog->Msg(strOutBuf);

			CvString cityName;

			CvCity* pLoopCity = NULL;
			int iLoop = 0;

			for(pLoopCity = GetPlayer()->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GetPlayer()->nextCity(&iLoop))
			{
				cityName = pLoopCity->getName();
				strLogName= "CityStrategyAILog_" + playerName + "_" + cityName + ".csv";
				pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);
				pLog->Msg(strOutBuf);
			}
		}
		else
		{
			pLog = LOGFILEMGR.GetLog("CityStrategyAILog.csv", FILogFile::kDontTimeStamp);
			pLog->Msg(strOutBuf);
			pLog = LOGFILEMGR.GetLog("TechAILog.csv", FILogFile::kDontTimeStamp);
			pLog->Msg(strOutBuf);
		}
	}
}

/// Log some military status info each turn
void CvMilitaryAI::LogMilitaryStatus()
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strTemp;
		CvString playerName;
		CvCity* pCity = NULL;
		FILogFile* pLog = NULL;

		// Open the right file
		playerName = GetPlayer()->getCivilizationShortDescription();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName,true), FILogFile::kDontTimeStamp);

		// Very first update (to write header row?)
		if(GC.getGame().getElapsedGameTurns() == 0 && m_pPlayer->GetID() == 0)
		{
			strTemp.Format("Turn, Player, Era, Cities, Settlers, LandUnits, LandArmySize, RecLandOffensive, RecLandDefensive, NavalUnits, NavalArmySize, RecNavyOffensive, MeleeUnits, MobileUnits, ReconUnits, ArcherUnits, SiegeUnits, SkirmisherUnits, AllLandRanged, AntiAirUnits, MeleeNavalUnits, RangedNavalUnits, Submarines, Carriers, TotalAirUnits, BomberUnits, FighterUnits, Nukes, Missiles, RecTotal, TotalMilitaryUnits, SupplyLimit, OutOfSupply, WarWearinessSupplyReduction, NoSupplyUnits, WarCount, MostEndangeredCity, Danger");
			pLog->Msg(strTemp);
		}

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += playerName + ", ";

		// City info
		strTemp.Format("%d, %d, %d, ", m_pPlayer->GetCurrentEra(), m_pPlayer->getNumCities(), m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_SETTLE, true));
		strOutBuf = strBaseString + strTemp;

		// Military size Info
		strTemp.Format("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, ", m_iNumLandUnits, m_iNumLandUnitsInArmies, m_iRecLandUnits, m_iNumNavalUnits, m_iNumNavalUnitsInArmies, m_iRecNavalUnits, m_iNumMeleeLandUnits, m_iNumMobileLandUnits, m_iNumReconLandUnits, m_iNumArcherLandUnits, m_iNumSiegeLandUnits, m_iNumSkirmisherLandUnits, m_iNumRangedLandUnits, m_iNumAntiAirUnits, m_iNumMeleeNavalUnits, m_iNumRangedNavalUnits, m_iNumSubmarineNavalUnits, m_iNumCarrierNavalUnits, m_iNumAirUnits, m_iNumBomberAirUnits, m_iNumFighterAirUnits, m_pPlayer->getNumNukeUnits(), m_iNumMissileUnits);
		strOutBuf += strTemp;

		// Unit supply
		strTemp.Format("%d, %d, %d, %d, %d, %d, %d, ", 
			GetRecommendedMilitarySize(), m_pPlayer->getNumMilitaryUnits(), m_pPlayer->GetNumUnitsSupplied(), m_pPlayer->GetNumUnitsOutOfSupply(), m_pPlayer->GetNumUnitsOutOfSupply(false) - m_pPlayer->GetNumUnitsOutOfSupply(),
			m_pPlayer->getNumUnitsSupplyFree(), GetNumberCivsAtWarWith(false)); //adjusted for better readability
		strOutBuf += strTemp;

		// Most threatened city
		vector<CvCity*> threatCities = m_pPlayer->GetThreatenedCities(false);
		pCity = threatCities.empty() ? NULL : threatCities.front();
		if(pCity != NULL)
		{
			strOutBuf += pCity->getNameNoSpace();
			strTemp.Format(", %d", pCity->getThreatValue());
			strOutBuf += strTemp;
		}

		pLog->Msg(strOutBuf);
	}

	// Mirror MilitarySummary into the SQLite stats database
	if (MOD_SQLITE_LOGGING)
	{
		RegisterMilitarySummaryTable();

		// Duplicate the two lookups from the CSV block above so this path is self-contained.
		CvString playerName = GetPlayer()->getCivilizationShortDescription();

		CvCity* pCity = NULL;
		vector<CvCity*> threatCities = m_pPlayer->GetThreatenedCities(false);
		pCity = threatCities.empty() ? NULL : threatCities.front();

		// Use an empty string for the city name and zero danger when no threatened city exists.
		CvString strEndangeredCity = "";
		int iDanger = 0;
		if (pCity != NULL)
		{
			strEndangeredCity = pCity->getNameNoSpace();
			iDanger = pCity->getThreatValue();
		}

		GET_SQLITE_LOGGER().BeginLogRow("MilitarySummary")
			.bind(playerName.c_str())
			.bind((int)m_pPlayer->GetCurrentEra())
			.bind(m_pPlayer->getNumCities())
			.bind(m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_SETTLE, true))
			.bind(m_iNumLandUnits)
			.bind(m_iNumLandUnitsInArmies)
			.bind(m_iRecLandUnits)
			.bind(m_iNumNavalUnits)
			.bind(m_iNumNavalUnitsInArmies)
			.bind(m_iRecNavalUnits)
			.bind(m_iNumMeleeLandUnits)
			.bind(m_iNumMobileLandUnits)
			.bind(m_iNumReconLandUnits)
			.bind(m_iNumArcherLandUnits)
			.bind(m_iNumSiegeLandUnits)
			.bind(m_iNumSkirmisherLandUnits)
			.bind(m_iNumRangedLandUnits)
			.bind(m_iNumAntiAirUnits)
			.bind(m_iNumMeleeNavalUnits)
			.bind(m_iNumRangedNavalUnits)
			.bind(m_iNumSubmarineNavalUnits)
			.bind(m_iNumCarrierNavalUnits)
			.bind(m_iNumAirUnits)
			.bind(m_iNumBomberAirUnits)
			.bind(m_iNumFighterAirUnits)
			.bind(m_pPlayer->getNumNukeUnits())
			.bind(m_iNumMissileUnits)
			.bind(GetRecommendedMilitarySize())
			.bind(m_pPlayer->getNumMilitaryUnits())
			.bind(m_pPlayer->GetNumUnitsSupplied())
			.bind(m_pPlayer->GetNumUnitsOutOfSupply())
			.bind(m_pPlayer->GetNumUnitsOutOfSupply(false) - m_pPlayer->GetNumUnitsOutOfSupply())
			.bind(m_pPlayer->getNumUnitsSupplyFree())
			.bind(GetNumberCivsAtWarWith(false))
			.bind(strEndangeredCity.c_str())
			.bind(iDanger)
			.execute();
	}
}

/// Log some military status info each turn
void CvMilitaryAI::LogAvailableForces()
{
	if(GC.getLogging() && GC.getAILogging())
	{
		/*
		int iCapitalX = 0;
		int iCapitalY = 0;
		CvCity* pCapital = GetPlayer()->getCapitalCity();
		if(pCapital)
		{
			iCapitalX = pCapital->getX();
			iCapitalY = pCapital->getY();
		}
		*/

		// Open the right file
		CvString playerName = GetPlayer()->getCivilizationShortDescription();
		FILogFile* pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		CvString strBaseString;
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += playerName + ", ";

		if (IsBuildingArmy(ARMY_TYPE_LAND))
		{
			CvString strOutBuf = strBaseString + ArmyTypeToString(ARMY_TYPE_LAND) + " under construction";
			pLog->Msg(strOutBuf);
		}
		if (IsBuildingArmy(ARMY_TYPE_NAVAL))
		{
			CvString strOutBuf = strBaseString + ArmyTypeToString(ARMY_TYPE_NAVAL) + " under construction";
			pLog->Msg(strOutBuf);
		}
		if (IsBuildingArmy(ARMY_TYPE_COMBINED))
		{
			CvString strOutBuf = strBaseString + ArmyTypeToString(ARMY_TYPE_COMBINED) + " under construction";
			pLog->Msg(strOutBuf);
		}

		/*
		// Loop through our units
		CvUnit* pLoopUnit;
		int iLoop;
		for(pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
		{
			// Never want immobile/dead units, explorers, ones that have already moved or automated human units
			if(pLoopUnit->isDelayedDeath() || pLoopUnit->AI_getUnitAIType() == UNITAI_UNKNOWN ||  pLoopUnit->AI_getUnitAIType() == UNITAI_EXPLORE || !pLoopUnit->canMove() || pLoopUnit->isHuman())
			{
				continue;
			}

			// Now down to land and sea units ... in these groups our unit must have a base combat strength ... or be a great general/admiral
			else if(!pLoopUnit->IsCombatUnit() && !pLoopUnit->IsCombatSupportUnit())
			{
				continue;
			}
			// No units finishing up operations
			else if(pLoopUnit->IsRecentlyDeployedFromOperation())
			{
				continue;
			}

			int iArmyID = pLoopUnit->getArmyID();
			int iDist = plotDistance(pLoopUnit->getX(), pLoopUnit->getY(), iCapitalX, iCapitalY);
			strTemp.Format("%s, Damage=%d, ArmyID=%d, X=%d, Y=%d, DistFromCap=%d", pLoopUnit->getName().GetCString(), pLoopUnit->getDamage(), iArmyID, pLoopUnit->getX(), pLoopUnit->getY(), iDist);
			strOutBuf = strBaseString + strTemp;

			pLog->Msg(strOutBuf);
		}
		*/
	}
}

/// Log some military status info each turn
void CvMilitaryAI::LogWarStateChange(PlayerTypes ePlayer, WarStateTypes eNewWarState, WarStateTypes eOldWarState)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strTemp;
		CvString playerName;
		CvString opponentName;
		FILogFile* pLog = NULL;

		// Open the right file
		playerName = GetPlayer()->getCivilizationShortDescription();
		opponentName = GET_PLAYER(ePlayer).getCivilizationShortDescription();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);

		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += playerName + ", vs. " + opponentName + ", New War State: ";

		// New War State
		switch(eNewWarState)
		{
		case WAR_STATE_CALM:
			strTemp.Format("Calm");
			break;
		case WAR_STATE_NEARLY_WON:
			strTemp.Format("Nearly Won");
			break;
		case WAR_STATE_OFFENSIVE:
			strTemp.Format("Offensive");
			break;
		case WAR_STATE_STALEMATE:
			strTemp.Format("Stalemate");
			break;
		case WAR_STATE_DEFENSIVE:
			strTemp.Format("Defensive");
			break;
		case WAR_STATE_NEARLY_DEFEATED:
			strTemp.Format("Nearly Defeated");
			break;
		default:
			strTemp.Format("XXX");
			break;
		}
		strOutBuf = strBaseString + strTemp + ", Old War State: ";

		// Old War State
		switch(eOldWarState)
		{
		case WAR_STATE_CALM:
			strTemp.Format("Calm");
			break;
		case WAR_STATE_NEARLY_WON:
			strTemp.Format("Nearly Won");
			break;
		case WAR_STATE_OFFENSIVE:
			strTemp.Format("Offensive");
			break;
		case WAR_STATE_STALEMATE:
			strTemp.Format("Stalemate");
			break;
		case WAR_STATE_DEFENSIVE:
			strTemp.Format("Defensive");
			break;
		case WAR_STATE_NEARLY_DEFEATED:
			strTemp.Format("Nearly Defeated");
			break;
		default:
			strTemp.Format("XXX");
			break;
		}
		strOutBuf += strTemp;
		pLog->Msg(strOutBuf);
	}
}

/// Log that a unit is being scrapped
void CvMilitaryAI::LogScrapUnit(CvUnit* pUnit, bool bDeficit, bool bSupply)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strTemp;
		CvString playerName;
		FILogFile* pLog = NULL;

		// Open the right file
		playerName = m_pPlayer->getCivilizationShortDescription();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);

		strOutBuf.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strOutBuf += playerName + ", ";

		strTemp.Format("Scrapping %s, X: %d, Y: %d, ", pUnit->getUnitInfo().GetDescription(), pUnit->getX(), pUnit->getY());
		strOutBuf += strTemp;
		if(bDeficit)
		{
			strOutBuf += "IN DEFICIT, ";
		}
		else
		{
			strOutBuf += "Finances ok, ";
		}
		if(bSupply)
		{
			strOutBuf += "OVER SUPPLY, ";
		}
		else
		{
			strOutBuf += "Supply ok, ";
		}
		if(pUnit->getDomainType() == DOMAIN_LAND)
		{
			strTemp.Format("Num Land Units: %d, In Armies %d, Rec Size: %d, ", m_iNumLandUnits, m_iNumLandUnitsInArmies, GetRecommendLandArmySize());
		}
		else
		{
			strTemp.Format("Num Naval Units: %d, In Armies %d, Rec Size: %d ", m_iNumNavalUnits, m_iNumNavalUnitsInArmies, m_iRecNavalUnits);
		}
		strOutBuf += strTemp;
		pLog->Msg(strOutBuf);
	}
}
/// Log that a unit is being gifted
void CvMilitaryAI::LogGiftUnit(CvUnit* pUnit, bool bDeficit, bool bSupply)
{
	if (GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strTemp;
		CvString playerName;
		FILogFile* pLog = NULL;

		// Open the right file
		playerName = m_pPlayer->getCivilizationShortDescription();
		pLog = LOGFILEMGR.GetLog(GetLogFileName(playerName), FILogFile::kDontTimeStamp);

		strOutBuf.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strOutBuf += playerName + ", ";

		strTemp.Format("Gifting unit %s to City-State, X: %d, Y: %d, ", pUnit->getUnitInfo().GetDescription(), pUnit->getX(), pUnit->getY());
		strOutBuf += strTemp;
		if (bDeficit)
		{
			strOutBuf += "IN DEFICIT, ";
		}
		else
		{
			strOutBuf += "Finances ok, ";
		}
		if (bSupply)
		{
			strOutBuf += "OVER SUPPLY, ";
		}
		else
		{
			strOutBuf += "Supply ok, ";
		}
		if (pUnit->getDomainType() == DOMAIN_LAND)
		{
			strTemp.Format("Num Land Units: %d, In Armies %d, Rec Size: %d, ", m_iNumLandUnits, m_iNumLandUnitsInArmies, GetRecommendLandArmySize());
		}
		else
		{
			strTemp.Format("Num Naval Units: %d, In Armies %d, Rec: %d ", m_iNumNavalUnits, m_iNumNavalUnitsInArmies, m_iRecNavalUnits);
		}
		strOutBuf += strTemp;
		pLog->Msg(strOutBuf);
	}
}

/// Build log filename
CvString CvMilitaryAI::GetLogFileName(CvString& playerName, bool bSummary) const
{
	CvString strLogName;

	if (bSummary)
	{
		strLogName = "MilitarySummary.csv";
	}
	else
	{
		if(GC.getPlayerAndCityAILogSplit())
		{
			strLogName = "MilitaryAILog_" + playerName + ".csv";
		}
		else
		{
			strLogName = "MilitaryAILog.csv";
		}
	}

	return strLogName;
}

//Gets the type of war the player is, overall, facing (used to decide production). 1 is land, 2 is sea (thanks, Paul Revere).
WarTypes CvMilitaryAI::GetWarType(PlayerTypes ePlayer)
{
	int iLand = 0;
	int iSea = 0;
	//No player? Let's get majority.
	if (ePlayer == NO_PLAYER)
	{
		for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
		{
			PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
			if (eLoopPlayer != NO_PLAYER && GET_PLAYER(eLoopPlayer).isAlive() && eLoopPlayer != m_pPlayer->GetID() && !GET_PLAYER(eLoopPlayer).isMinorCiv())
			{
				int iWar = GetWarType(eLoopPlayer);
				if (iWar == WARTYPE_LAND)
				{
					iLand++;
				}
				else if (iWar == WARTYPE_SEA)
				{
					iSea++;
				}
			}
		}
		
		return (iLand >= iSea) ? WARTYPE_LAND : WARTYPE_SEA;
	}

	PRECONDITION(ePlayer >= 0 && ePlayer < MAX_MAJOR_CIVS, "GetWarType only valid for major civs");
	return (WarTypes)m_aiWarFocus[ePlayer];
}

void CvMilitaryAI::UpdateWarType()
{
	int iEnemyWater = 0;
	int iEnemyLand = 0;
	int iEnemyWaterCities = 0;
	int iEnemyLandCities = 0;
	int iLoop = 0;

	int iFriendlyLand = 0;
	int iFriendlySea = 0;
	int iFriendlyLandCities = 0;
	int iFriendlySeaCities = 0;

	for (CvUnit* pLoopUnit = m_pPlayer->firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = m_pPlayer->nextUnit(&iLoop))
	{
		if (pLoopUnit->IsCombatUnit())
		{
			if (pLoopUnit->getDomainType() == DOMAIN_SEA)
			{
				if (pLoopUnit->IsCanAttackRanged())
				{
					iFriendlySea += pLoopUnit->GetBaseRangedCombatStrength();
				}
				else
				{
					iFriendlySea += pLoopUnit->GetBaseCombatStrength();
				}
			}
			else if (pLoopUnit->getDomainType() == DOMAIN_LAND)
			{
				if (pLoopUnit->IsCanAttackRanged())
				{
					iFriendlyLand += pLoopUnit->GetBaseRangedCombatStrength();
				}
				else
				{
					iFriendlyLand += pLoopUnit->GetBaseCombatStrength();
				}
			}
		}
	}

	for (CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop))
	{
		for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
		{
			PlayerTypes eLoopPlayer = (PlayerTypes)iPlayerLoop;
			if (eLoopPlayer != NO_PLAYER && GET_PLAYER(eLoopPlayer).isAlive() && eLoopPlayer != m_pPlayer->GetID() && !GET_PLAYER(eLoopPlayer).isMinorCiv())
			{
				if (GET_TEAM(GET_PLAYER(eLoopPlayer).getTeam()).isAtWar(m_pPlayer->getTeam()) || m_pPlayer->GetDiplomacyAI()->GetCivApproach(eLoopPlayer) == CIV_APPROACH_WAR)
				{
					if (pLoopCity->isCoastal())
					{
						iFriendlySeaCities += 50;
					}
					else
					{
						iFriendlyLandCities += 50;
					}
					if (pLoopCity->IsInDanger(eLoopPlayer))
					{
						if (pLoopCity->isCoastal())
						{
							iFriendlySeaCities += 50;
						}
						else
						{
							iFriendlyLandCities += 50;
						}
					}
				}
			}
		}
	}

	//And now theirs...
	for(int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{	
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
		if(eLoopPlayer != NO_PLAYER && GET_PLAYER(eLoopPlayer).isAlive() && eLoopPlayer != m_pPlayer->GetID() && !GET_PLAYER(eLoopPlayer).isMinorCiv())
		{
			if(GET_TEAM(GET_PLAYER(eLoopPlayer).getTeam()).isAtWar(m_pPlayer->getTeam()) || m_pPlayer->GetDiplomacyAI()->GetCivApproach(eLoopPlayer) == CIV_APPROACH_WAR)
			{
				for(CvUnit* pLoopUnit = GET_PLAYER(eLoopPlayer).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(eLoopPlayer).nextUnit(&iLoop))
				{
					if (pLoopUnit->IsCombatUnit())
					{
						if(pLoopUnit->getDomainType() == DOMAIN_SEA)
						{
							if (pLoopUnit->plot()->isVisible(m_pPlayer->getTeam()))
							{
								if (pLoopUnit->IsCanAttackRanged())
								{
									iEnemyWater += pLoopUnit->GetBaseRangedCombatStrength() * 2;
								}
								else
								{
									iEnemyWater += pLoopUnit->GetBaseCombatStrength() * 2;
								}
							}
							else
							{
								if (pLoopUnit->IsCanAttackRanged())
								{
									iEnemyWater += pLoopUnit->GetBaseRangedCombatStrength();
								}
								else
								{
									iEnemyWater += pLoopUnit->GetBaseCombatStrength();
								}
							}				
						}
						else if (pLoopUnit->getDomainType() == DOMAIN_LAND)
						{
							if (pLoopUnit->plot()->isVisible(m_pPlayer->getTeam()))
							{
								if (pLoopUnit->IsCanAttackRanged())
								{
									iEnemyLand += pLoopUnit->GetBaseRangedCombatStrength() * 2;
								}
								else
								{
									iEnemyLand += pLoopUnit->GetBaseCombatStrength() * 2;
								}
							}
							else
							{
								if (pLoopUnit->IsCanAttackRanged())
								{
									iEnemyLand += pLoopUnit->GetBaseRangedCombatStrength();
								}
								else
								{
									iEnemyLand += pLoopUnit->GetBaseCombatStrength();
								}
							}
						}
					}
				}
				CvCity* pLoopCity = NULL;
				int iLoopCity = 0;
				for(pLoopCity = GET_PLAYER(eLoopPlayer).firstCity(&iLoopCity); pLoopCity != NULL; pLoopCity = GET_PLAYER(eLoopPlayer).nextCity(&iLoopCity))
				{
					if (pLoopCity->IsInDanger(m_pPlayer->GetID()))
					{
						if (pLoopCity->isCoastal())
						{
							iFriendlySeaCities += 50;
						}
						else
						{
							iFriendlyLandCities += 50;
						}
					}
					if(pLoopCity->isCoastal())
					{
						iEnemyWaterCities += 50;
					}
					else
					{
						iEnemyLandCities += 50;
					}
				}
			}
			if (iEnemyWater > 0)
			{
				iEnemyWater += iEnemyWaterCities;
			}
			else
			{
				iEnemyWater += (iEnemyWaterCities / 10);
			}
			if (iEnemyLand > 0)
			{
				iEnemyLand += iEnemyLandCities;
			}
			else
			{
				iEnemyLand += (iEnemyLandCities / 10);
			}

			if (iFriendlySea > 0)
			{
				iFriendlySea += iFriendlySeaCities;
			}
			else
			{
				iFriendlySea += (iFriendlySeaCities / 10);
			}
			if (iFriendlyLand > 0)
			{
				iFriendlyLand += iFriendlyLandCities;
			}
			else
			{
				iFriendlyLand += (iFriendlyLandCities / 10);
			}

			if (iEnemyWater > iEnemyLand && iEnemyWater > iFriendlySea && m_aiWarFocus[eLoopPlayer] != WARTYPE_SEA)
			{
				if (GC.getLogging() && GC.getAILogging())
				{
					CvString strTemp;
					CvString strLogString;
					strLogString.Format("War Type versus %s now WATER. Enemy has: %d Water, %d Land, we have %d Water, %d Land", GET_PLAYER(eLoopPlayer).getCivilizationShortDescription(), iEnemyWater, iEnemyLand, iFriendlySea, iFriendlyLand);
					m_pPlayer->GetTacticalAI()->LogTacticalMessage(strLogString);
				}
				m_aiWarFocus[eLoopPlayer] = WARTYPE_SEA;
			}
			else if (iEnemyLand >= iEnemyWater && iEnemyLand > iFriendlyLand && (iEnemyLand != 0) && m_aiWarFocus[eLoopPlayer] != WARTYPE_LAND)
			{
				if (GC.getLogging() && GC.getAILogging())
				{
					CvString strTemp;
					CvString strLogString;
					strLogString.Format("War Type versus %s now LAND. Enemy has: %d Water, %d Land, we have %d Water, %d Land", GET_PLAYER(eLoopPlayer).getCivilizationShortDescription(), iEnemyWater, iEnemyLand, iFriendlySea, iFriendlyLand);
					m_pPlayer->GetTacticalAI()->LogTacticalMessage(strLogString);
				}
				m_aiWarFocus[eLoopPlayer] = WARTYPE_LAND;
			}
			else
			{
				m_aiWarFocus[eLoopPlayer] = WARTYPE_UNDEFINED;
			}		
		}
	}
}

// NON-MEMBER FUNCTIONS
//
// These are functions that do not need access to the internals of the CvMilitaryAI class.
// Keep them as non-member functions to:
// a) simplify the class (improving encapsulation, reducing coupling)
// b) allow their general use by other classes

// Figure out what the WeightThreshold Mod should be by looking at the Flavors for this player & the Strategy
int MilitaryAIHelpers::GetWeightThresholdModifier(MilitaryAIStrategyTypes eStrategy, CvPlayer* pPlayer)
{
	int iWeightThresholdModifier = 0;

	CvMilitaryAIStrategyXMLEntries* pkAIStrategies = pPlayer->GetMilitaryAI()->GetMilitaryAIStrategies();
	CvFlavorManager* pkFlavorManager = pPlayer->GetFlavorManager();
	// Look at all Flavors for the Player & this Strategy
	for(int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
	{
		FlavorTypes eFlavor = (FlavorTypes) iFlavorLoop;
		int iPersonalityFlavor = pkFlavorManager->GetPersonalityIndividualFlavor(eFlavor);
		CvMilitaryAIStrategyXMLEntry* pkEntry = pkAIStrategies->GetEntry(eStrategy);
		ASSERT(pkEntry != NULL);
		if(pkEntry)
		{
			int iStrategyFlavorMod = pkEntry->GetPersonalityFlavorThresholdMod(eFlavor);

			iWeightThresholdModifier += (iPersonalityFlavor * iStrategyFlavorMod);
		}
	}

	return iWeightThresholdModifier;
}

/// "Enough Military Units" Player Strategy: Does this player have too many military units?  If so, adjust flavors
bool MilitaryAIHelpers::IsTestStrategy_EnoughMilitaryUnits(CvPlayer* pPlayer)
{
	// Are we running at a deficit?
	static EconomicAIStrategyTypes eStrategyLosingMoney = (EconomicAIStrategyTypes) GC.getInfoTypeForString("ECONOMICAISTRATEGY_LOSING_MONEY");
	bool bInDeficit = pPlayer->GetEconomicAI()->IsUsingStrategy(eStrategyLosingMoney);

	// Are we running anything other than the Conquest Grand Strategy?
	if (bInDeficit || !pPlayer->GetDiplomacyAI()->IsGoingForWorldConquest())
	{
		if (pPlayer->GetMilitaryAI()->GetLandDefenseState() == DEFENSE_STATE_ENOUGH)
		{
			return true;
		}
	}
	else
	{
		return false;
	}

	return false;
}

/// "Empire Defense" Player Strategy: Adjusts military flavors if the player doesn't have the recommended number of units
bool MilitaryAIHelpers::IsTestStrategy_EmpireDefense(CvPlayer* pPlayer)
{
	return pPlayer->GetMilitaryAI()->GetLandDefenseState() == DEFENSE_STATE_NEEDED;
}

/// "Empire Defense" Player Strategy: If we have less than 1 unit per city (tweaked a bit by threat level), we NEED some units
bool MilitaryAIHelpers::IsTestStrategy_EmpireDefenseCritical(CvPlayer* pPlayer)
{
	return pPlayer->GetMilitaryAI()->GetLandDefenseState() == DEFENSE_STATE_CRITICAL;
}

/// "Enough Naval Units" Strategy: build navies
bool MilitaryAIHelpers::IsTestStrategy_EnoughNavalUnits(CvPlayer* pPlayer)
{
	return pPlayer->GetMilitaryAI()->GetNavalDefenseState() == DEFENSE_STATE_ENOUGH;
}

/// "Need Naval Units" Strategy: build navies
bool MilitaryAIHelpers::IsTestStrategy_NeedNavalUnits(CvPlayer* pPlayer)
{
	return pPlayer->GetMilitaryAI()->GetNavalDefenseState() == DEFENSE_STATE_NEEDED;
}

/// "Need Naval Units Critical" Strategy: build navies NOW
bool MilitaryAIHelpers::IsTestStrategy_NeedNavalUnitsCritical(CvPlayer* pPlayer)
{
	return pPlayer->GetMilitaryAI()->GetNavalDefenseState() == DEFENSE_STATE_CRITICAL;
}

/// "War Mobilization" Player Strategy: Does this player want to mobilize for war?  If so, adjust flavors
bool MilitaryAIHelpers::IsTestStrategy_WarMobilization(MilitaryAIStrategyTypes eStrategy, CvPlayer* pPlayer)
{
	int iCurrentWeight = 0;
	PlayerTypes eOtherPlayer;

	// If we're at war don't bother with this Strategy
	static MilitaryAIStrategyTypes eStrategyAtWar = (MilitaryAIStrategyTypes) GC.getInfoTypeForString("MILITARYAISTRATEGY_AT_WAR");
	if(pPlayer->GetMilitaryAI()->IsUsingStrategy(eStrategyAtWar))
	{
		return false;
	}

	// Are we running the Conquest Grand Strategy?
	if (pPlayer->GetDiplomacyAI()->IsGoingForWorldConquest())
	{
		iCurrentWeight += 25;
	}

	if(pPlayer->IsCramped())
	{
		iCurrentWeight += 5;
	}

	if (pPlayer->GetMilitaryAI()->IsBuildingArmy(ARMY_TYPE_ANY))
	{
		iCurrentWeight += 100;
	}

	CvDiplomacyAI* pkDiplomacyAI = pPlayer->GetDiplomacyAI();
	for(int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		eOtherPlayer = (PlayerTypes) iMajorLoop;

		if(pkDiplomacyAI->GetVictoryBlockLevel(eOtherPlayer) > BLOCK_LEVEL_WEAK)
		{
			iCurrentWeight += 5;
		}
		if(pkDiplomacyAI->GetVictoryDisputeLevel(eOtherPlayer) > DISPUTE_LEVEL_WEAK)
		{
			iCurrentWeight += 5;
		}

		// Mobilize for war is automatic if we are preparing a sneak attack
		if (pkDiplomacyAI->GetCivApproach(eOtherPlayer) == CIV_APPROACH_WAR)
		{
			iCurrentWeight += 100;
		}

		// Add in weight for each civ we're on really bad terms with
		else if(pkDiplomacyAI->GetCivApproach(eOtherPlayer) <= CIV_APPROACH_HOSTILE ||
		        pkDiplomacyAI->GetCivApproach(eOtherPlayer) == CIV_APPROACH_AFRAID ||
				pkDiplomacyAI->GetVisibleApproachTowardsUs(eOtherPlayer) <= CIV_APPROACH_HOSTILE ||
				pkDiplomacyAI->IsCapitalCapturedBy(eOtherPlayer) || pkDiplomacyAI->IsHolyCityCapturedBy(eOtherPlayer) ||
				pkDiplomacyAI->GetNumCitiesCapturedBy(eOtherPlayer) > 0 || pkDiplomacyAI->GetNumWarsDeclaredOnUs(eOtherPlayer) > 2)
		{
			iCurrentWeight += 50;
		}

		// And some if on fairly bad terms
		else if(pkDiplomacyAI->GetCivApproach(eOtherPlayer) <= CIV_APPROACH_GUARDED ||
				pkDiplomacyAI->GetVisibleApproachTowardsUs(eOtherPlayer) == CIV_APPROACH_GUARDED ||
				pkDiplomacyAI->GetNumWarsDeclaredOnUs(eOtherPlayer) > 0)
		{
			iCurrentWeight += 25;
		}
	}

	CvMilitaryAIStrategyXMLEntry* pStrategy = pPlayer->GetMilitaryAI()->GetMilitaryAIStrategies()->GetEntry(eStrategy);
	ASSERT(pStrategy != NULL);
	if(pStrategy)
	{
		int iWeightThreshold = pStrategy->GetWeightThreshold();	// 100
		if(iCurrentWeight >= iWeightThreshold)
		{
			return true;
		}
	}

	return false;
}

/// "At War" Player Strategy: If the player is at war, increase OFFENSE, DEFENSE and MILITARY_TRAINING.  Then look into which operation(s) to run
bool MilitaryAIHelpers::IsTestStrategy_AtWar(CvPlayer* pPlayer, bool bMinor)
{
	return pPlayer->GetMilitaryAI()->GetNumberCivsAtWarWith(bMinor) > 0;
}

/// "Minor Civ GeneralDefense" Player Strategy: Prioritize CITY_DEFENSE and DEFENSE
bool MilitaryAIHelpers::IsTestStrategy_MinorCivGeneralDefense()
{
	return true;
}

/// "Minor Civ Threat Elevated" Player Strategy: If a Minor Civ is in danger, turn CITY_DEFENSE and DEFENSE up
bool MilitaryAIHelpers::IsTestStrategy_MinorCivThreatElevated(CvPlayer* pPlayer)
{
	return pPlayer->GetMinorCivAI()->GetStatus() == MINOR_CIV_STATUS_ELEVATED;
}

/// "Minor Civ Threat Critical" Player Strategy: If a Minor Civ is in danger, turn CITY_DEFENSE and DEFENSE up
bool MilitaryAIHelpers::IsTestStrategy_MinorCivThreatCritical(CvPlayer* pPlayer)
{
	return pPlayer->GetMinorCivAI()->GetStatus() == MINOR_CIV_STATUS_CRITICAL;
}

/// "Eradicate Barbarians" Player Strategy: If there is a large group of barbarians units or camps near our civilization, increase OFFENSE
bool MilitaryAIHelpers::IsTestStrategy_EradicateBarbarians(MilitaryAIStrategyTypes eStrategy, CvPlayer* pPlayer, int iBarbarianCampCount, int iVisibleBarbarianCount)
{
	// If we're at war don't bother with this Strategy (unless it is clear we are already winning)
	static MilitaryAIStrategyTypes eStrategyAtWar = (MilitaryAIStrategyTypes) GC.getInfoTypeForString("MILITARYAISTRATEGY_AT_WAR");
	if(pPlayer->GetMilitaryAI()->IsUsingStrategy(eStrategyAtWar))
	{
		if(pPlayer->GetDiplomacyAI()->GetStateAllWars() != STATE_ALL_WARS_WINNING)
		{
			return false;
		}
	}

	// We ARE getting our guys back
	CvPlayerAI& BarbPlayer = GET_PLAYER(BARBARIAN_PLAYER);
	int iLoop = 0;
	for (CvUnit* pLoopUnit = BarbPlayer.firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = BarbPlayer.nextUnit(&iLoop))
	{
		if (pLoopUnit->GetOriginalOwner() == pPlayer->GetID() && (pLoopUnit->AI_getUnitAIType() == UNITAI_SETTLE || pLoopUnit->AI_getUnitAIType() == UNITAI_WORKER))
		{
			return true;
		}
	}

	CvMilitaryAIStrategyXMLEntry* pStrategy = pPlayer->GetMilitaryAI()->GetMilitaryAIStrategies()->GetEntry(eStrategy);
	ASSERT(pStrategy != NULL);
	if(pStrategy)
	{
		int iStrategyWeight = iBarbarianCampCount * 75 + iVisibleBarbarianCount * 25;   // Two visible camps or 3 roving Barbarians will trigger this
		int iWeightThresholdModifier = GetWeightThresholdModifier(eStrategy, pPlayer);
		int iWeightThreshold = pStrategy->GetWeightThreshold() + iWeightThresholdModifier;

		if(iStrategyWeight >= iWeightThreshold * 2 && iVisibleBarbarianCount > 0) // barbs are a big threat, our sneak attack plan can wait
		{
			return true;
		}
		else if(iStrategyWeight >= iWeightThreshold)
		{
			// Also don't bother if we're building up for a sneak attack
			CvDiplomacyAI* pkDiplomacyAI = pPlayer->GetDiplomacyAI();
			for (int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
			{
				PlayerTypes eOtherPlayer = (PlayerTypes) iMajorLoop;
				if (pkDiplomacyAI->IsWantsSneakAttack(eOtherPlayer))
				{
					return false;
				}
			}

			return true;
		}

		//Let's hunt barbs if we want to expand
		EconomicAIStrategyTypes eStrategyExpandToOtherContinents = (EconomicAIStrategyTypes) GC.getInfoTypeForString("ECONOMICAISTRATEGY_EXPAND_TO_OTHER_CONTINENTS");
		if(pPlayer->GetEconomicAI()->IsUsingStrategy(eStrategyExpandToOtherContinents))
		{
			return true;
		}
	}

	return false;
}

/// "Eradicate Barbarians" Player Strategy: If there is a large group of barbarians units or camps near our civilization, increase OFFENSE
bool MilitaryAIHelpers::IsTestStrategy_EradicateBarbariansCritical(MilitaryAIStrategyTypes eStrategy, CvPlayer* pPlayer, int iBarbarianCampCount, int iVisibleBarbarianCount)
{
	int iStrategyWeight = 0;

	CvMilitaryAIStrategyXMLEntry* pStrategy = pPlayer->GetMilitaryAI()->GetMilitaryAIStrategies()->GetEntry(eStrategy);
	ASSERT(pStrategy != NULL);
	if(pStrategy)
	{
		iStrategyWeight = iBarbarianCampCount * 75 + iVisibleBarbarianCount * 25;
		int iWeightThresholdModifier = GetWeightThresholdModifier(eStrategy, pPlayer);
		int iWeightThreshold = pStrategy->GetWeightThreshold() + iWeightThresholdModifier;

		if(iStrategyWeight >= iWeightThreshold * 4 && iVisibleBarbarianCount > 1) // barbs are a HUGE threat
		{
			return true;
		}
	}

	return false;
}

/// "Winning Wars" Strategy: boost OFFENSE over DEFENSE
bool MilitaryAIHelpers::IsTestStrategy_WinningWars(CvPlayer* pPlayer)
{
	if(pPlayer->GetDiplomacyAI()->GetStateAllWars() == STATE_ALL_WARS_WINNING)
	{
		return true;
	}

	int iSum = 0;
	PlayerTypes eLoopPlayer;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		eLoopPlayer = (PlayerTypes)iPlayerLoop;
		if (eLoopPlayer != pPlayer->GetID() && pPlayer->IsAtWarWith(eLoopPlayer) && pPlayer->GetDiplomacyAI()->IsPlayerValid(eLoopPlayer))
		{
			iSum += pPlayer->GetWarScore(eLoopPlayer);
		}
	}

	return iSum > 25;
}

/// "Losing Wars" Strategy: boost DEFENSE over OFFENSE
bool MilitaryAIHelpers::IsTestStrategy_LosingWars(CvPlayer* pPlayer)
{
	if(pPlayer->GetDiplomacyAI()->GetStateAllWars() == STATE_ALL_WARS_LOSING)
	{
		return true;
	}

	int iSum = 0;
	PlayerTypes eLoopPlayer;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		eLoopPlayer = (PlayerTypes)iPlayerLoop;
		if (eLoopPlayer != pPlayer->GetID() && pPlayer->IsAtWarWith(eLoopPlayer) && pPlayer->GetDiplomacyAI()->IsPlayerValid(eLoopPlayer))
		{
			iSum += pPlayer->GetWarScore(eLoopPlayer);
		}
	}
	return iSum < -10;
}

/// "Enough Ranged" Player Strategy: If a player has too many ranged units
bool MilitaryAIHelpers::IsTestStrategy_EnoughRangedUnits(CvPlayer* pPlayer, int iNumRanged, int iNumMelee)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_RANGED"));
	int iRatio = iNumRanged * 10 / max(1,iNumMelee+iNumRanged);
	return (iRatio >= iFlavorRange);
}

/// "Need Ranged" Player Strategy: If a player has too many melee units
bool MilitaryAIHelpers::IsTestStrategy_NeedRangedUnits(CvPlayer* pPlayer, int iNumRanged, int iNumMelee)
{
	if(pPlayer->GetMilitaryAI()->IsBuildingArmy(ARMY_TYPE_LAND))
	{
		if (pPlayer->IsUnderrepresentedUnitType(UNITAI_RANGED) || pPlayer->IsUnderrepresentedUnitType(UNITAI_CITY_BOMBARD))
		{
			return true;
		}
	}

	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_RANGED"));
	int iRatio = iNumRanged * 10 / max(1,iNumMelee+iNumRanged);
	return (iRatio <= iFlavorRange / 2);
}

/// "Need Ranged Early" Player Strategy: If a player is planning a early sneak attack we need to make sure that ranged are built since this will block this
bool MilitaryAIHelpers::IsTestStrategy_NeedRangedDueToEarlySneakAttack(CvPlayer* pPlayer)
{
	MilitaryAIStrategyTypes eStrategyWarMob = (MilitaryAIStrategyTypes) GC.getInfoTypeForString("MILITARYAISTRATEGY_WAR_MOBILIZATION");
	return pPlayer->GetMilitaryAI()->IsUsingStrategy(eStrategyWarMob);
}

/// "Enough Mobile" Player Strategy: If a player has too many mobile units
bool MilitaryAIHelpers::IsTestStrategy_EnoughMobileUnits(CvPlayer* pPlayer, int iNumMobile, int iNumMelee)
{
	int iFlavorMobile = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_MOBILE"));
	if (!MOD_AI_UNIT_PRODUCTION)
	{
		int iRatio = iNumMobile * 10 / max(1,iNumMelee+iNumMobile);
		return (iRatio >= iFlavorMobile);
	}
	else
	{
		int iRatio = iNumMobile * 10 / max(1, iNumMelee);
		return (iRatio >= iFlavorMobile);
	}
}

/// "Need Mobile" Player Strategy: If a player has too many slow units
bool MilitaryAIHelpers::IsTestStrategy_NeedMobileUnits(CvPlayer* pPlayer, int iNumMobile, int iNumMelee)
{
	int iFlavorMobile = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_MOBILE"));
	if (!MOD_AI_UNIT_PRODUCTION)
	{
		int iRatio = iNumMobile * 10 / max(1, iNumMelee + iNumMobile);
		return (iRatio <= iFlavorMobile / 2);
	}
	else
	{
		int iRatio = iNumMobile * 10 / max(1, iNumMelee);
		return (iRatio <= iFlavorMobile / 2);
	}
}

/// "Enough Air" Player Strategy: If a player has too many air units
bool MilitaryAIHelpers::IsTestStrategy_EnoughAirUnits(CvPlayer* pPlayer, int iNumAir, int iNumMelee)
{
	int iFlavorAir = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_AIR"));
	int iRatio = iNumAir * 10 / max(1,iNumMelee+iNumAir);
	return (iRatio >= iFlavorAir);
}

/// "Need Air" Player Strategy: If a player has too few flying units
bool MilitaryAIHelpers::IsTestStrategy_NeedAirUnits(CvPlayer* pPlayer, int iNumAir, int iNumMelee)
{
	int iFlavorAir = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_AIR"));
	int iRatio = iNumAir * 10 / max(1,iNumMelee+iNumAir);
	return (iRatio <= iFlavorAir / 2);
}

/// "Need A Nuke" Player Strategy: If a player has no nukes but he could
bool MilitaryAIHelpers::IsTestStrategy_NeedANuke(CvPlayer* pPlayer)
{
	if(GC.getGame().isNoNukes())
	{
		return false;
	}

	if (pPlayer->GetDiplomacyAI()->IsNuclearGandhi(true))
	{
		return true;
	}

	int iFlavorNuke = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_NUKE"));
	int iNumNukes = pPlayer->getNumNukeUnits();

	return (iNumNukes < iFlavorNuke / 3);
}

/// "Enough Anti-Air" Player Strategy: If a player has too many AA units
bool MilitaryAIHelpers::IsTestStrategy_EnoughAntiAirUnits(CvPlayer* pPlayer, int iNumAA, int iNumMelee)
{
	bool bAnyAirforce = false;
	PlayerTypes eLoopPlayer;
	for(int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		eLoopPlayer = (PlayerTypes) iPlayerLoop;
		if(eLoopPlayer != pPlayer->GetID() && pPlayer->GetDiplomacyAI()->IsPlayerValid(eLoopPlayer))
		{
			if(GET_PLAYER(eLoopPlayer).GetMilitaryAI()->HasAirforce())
			{
				bAnyAirforce = true;
				break;
			}
		}
	}

	if(bAnyAirforce)
	{
		// The original code simplifies to 4*iNumAA > iNumMelee
		return (iNumAA * /*2*/ GD_INT_GET(AI_CONFIG_MILITARY_MELEE_PER_AA) > iNumMelee);
	}
	else
	{
		return true;
	}
}

/// "Need Anti-Air" Player Strategy: If a player has too many AA units
bool MilitaryAIHelpers::IsTestStrategy_NeedAntiAirUnits(CvPlayer* pPlayer, int iNumAA, int iNumMelee)
{
	bool bAnyAirforce = false;
	PlayerTypes eLoopPlayer;
	for(int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		eLoopPlayer = (PlayerTypes) iPlayerLoop;
		if(eLoopPlayer != pPlayer->GetID() && pPlayer->GetDiplomacyAI()->IsPlayerValid(eLoopPlayer))
		{
			if(GET_PLAYER(eLoopPlayer).GetMilitaryAI()->HasAirforce())
			{
				bAnyAirforce = true;
				break;
			}
		}
	}

	if(bAnyAirforce)
	{
		// The original code simplifies to 4*iNumAA <= iNumMelee
		return (iNumAA * /*2*/ GD_INT_GET(AI_CONFIG_MILITARY_MELEE_PER_AA) <= iNumMelee);
	}
	else
	{
		return false;
	}
}

/// "Need Air Carrier" Player Strategy: If a player needs some aircraft carriers
bool MilitaryAIHelpers::IsTestStrategy_NeedAirCarriers(CvPlayer* pPlayer)
{
	return pPlayer->GetMilitaryAI()->HasAirforce() && pPlayer->GetMilitaryAI()->GetNumFreeCarrier() == 0;
}


// MOD_AI_UNIT_PRODUCTION: New strategies
/// "Enough Archer" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_EnoughArcherUnits(CvPlayer* pPlayer, int iNumArcher, int iNumMelee)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_ARCHER"));
	int iRatio = iNumArcher * 10 / max(1, iNumMelee);
	return (iRatio >= iFlavorRange);
}

/// "Need Archer" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_NeedArcherUnits(CvPlayer* pPlayer, int iNumArcher, int iNumMelee)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_ARCHER"));
	int iRatio = iNumArcher * 10 / max(1, iNumMelee);
	return (iRatio <= iFlavorRange / 2);
}

/// "Enough Siege" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_EnoughSiegeUnits(CvPlayer* pPlayer, int iNumSiege, int iNumMelee)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_SIEGE"));
	int iRatio = iNumSiege * 10 / max(1, iNumMelee);
	return (iRatio >= iFlavorRange);
}

/// "Need Siege" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_NeedSiegeUnits(CvPlayer* pPlayer, int iNumSiege, int iNumMelee)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_SIEGE"));
	int iRatio = iNumSiege * 10 / max(1, iNumMelee);
	return (iRatio <= iFlavorRange / 4);
}

/// "Enough Skirmisher" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_EnoughSkirmisherUnits(CvPlayer* pPlayer, int iNumSkirmisher, int iNumMelee)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_SKIRMISHER"));
	int iRatio = iNumSkirmisher * 10 / max(1, iNumMelee);
	return (iRatio >= iFlavorRange);
}

/// "Need Skirmisher" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_NeedSkirmisherUnits(CvPlayer* pPlayer, int iNumSkirmisher, int iNumMelee)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_SKIRMISHER"));
	int iRatio = iNumSkirmisher * 10 / max(1, iNumMelee);
	return (iRatio <= iFlavorRange / 4);
}

/// "Enough NavalMelee" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_EnoughNavalMeleeUnits(CvPlayer* pPlayer, int iNumNavalMelee, int iNumNaval)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_NAVAL_MELEE"));
	int iRatio = iNumNavalMelee * 10 / max(1, iNumNaval);
	return (iRatio >= (7 * iFlavorRange / 5));
}

/// "Need NavalMelee" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_NeedNavalMeleeUnits(CvPlayer* pPlayer, int iNumNavalMelee, int iNumNaval)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_NAVAL_MELEE"));
	int iRatio = iNumNavalMelee * 10 / max(1, iNumNaval);
	return (iRatio <= (3 * iFlavorRange / 5));
}

/// "Enough NavalRanged" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_EnoughNavalRangedUnits(CvPlayer* pPlayer, int iNumNavalRanged, int iNumNaval)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_NAVAL_RANGED"));
	int iRatio = iNumNavalRanged * 10 / max(1, iNumNaval);
	return (iRatio >= (7 * iFlavorRange / 5));
}

/// "Need NavalRanged" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_NeedNavalRangedUnits(CvPlayer* pPlayer, int iNumNavalRanged, int iNumNaval)
{	
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_NAVAL_RANGED"));
	int iRatio = iNumNavalRanged * 10 / max(1, iNumNaval);
	return (iRatio <= (3 * iFlavorRange / 5));
}

/// "Enough Submarine" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_EnoughSubmarineUnits(CvPlayer* pPlayer, int iNumSubmarine, int iNumNaval)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_SUBMARINE"));
	int iRatio = iNumSubmarine * 10 / max(1, iNumNaval);
	return (iRatio >= (3 * iFlavorRange / 5));
}

/// "Need Submarine" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_NeedSubmarineUnits(CvPlayer* pPlayer, int iNumSubmarine, int iNumNaval)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_SUBMARINE"));
	int iRatio = iNumSubmarine * 10 / max(1, iNumNaval);
	return (iRatio <= (iFlavorRange / 5));
}

/// "Enough Bomber" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_EnoughBomberUnits(CvPlayer* pPlayer, int iNumBomber, int iNumAir)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_BOMBER"));
	int iRatio = iNumBomber * 10 / max(1, iNumAir);
	return (iRatio >= (7 * iFlavorRange / 5));
}

/// "Need Bomber" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_NeedBomberUnits(CvPlayer* pPlayer, int iNumBomber, int iNumAir)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_BOMBER"));
	int iRatio = iNumBomber * 10 / max(1, iNumAir);
	return (iRatio <= (3 * iFlavorRange / 5));
}

/// "Enough Fighter" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_EnoughFighterUnits(CvPlayer* pPlayer, int iNumFighter, int iNumAir)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_FIGHTER"));
	int iRatio = iNumFighter * 10 / max(1, iNumAir);
	return (iRatio >= (7 * iFlavorRange / 5));
}

/// "Need Fighter" Player Strategy
bool MilitaryAIHelpers::IsTestStrategy_NeedFighterUnits(CvPlayer* pPlayer, int iNumFighter, int iNumAir)
{
	int iFlavorRange = pPlayer->GetGrandStrategyAI()->GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_FIGHTER"));
	int iRatio = iNumFighter * 10 / max(1, iNumAir);
	return (iRatio <= (3 * iFlavorRange / 5));
}


// MORE NON-MEMBER FUNCTIONS

bool MilitaryAIHelpers::NeedShipInArea(PlayerTypes ePlayer, CvLandmass* pWaterBody)
{
	if (!pWaterBody)
		return false;

	int iForeignCities = pWaterBody->getNumCities() - pWaterBody->getCitiesPerPlayer(ePlayer);
	int iForeignUnits = pWaterBody->getNumUnits() - pWaterBody->getUnitsPerPlayer(ePlayer);
	bool bTooManyUnits = (pWaterBody->getNumTiles() < pWaterBody->getUnitsPerPlayer(ePlayer) * /*6*/ GD_INT_GET(AI_CONFIG_MILITARY_TILES_PER_SHIP));

	if (!bTooManyUnits)
		if (iForeignCities > 0 || iForeignUnits > 0)
			return true;

	return false;
}

//todo: use the step pathfinder here to get a plot which is on the correct side of the target? need a starting point then ...
CvPlot* MilitaryAIHelpers::GetCoastalWaterNearPlot(CvPlot *pTarget, bool bCheckTeam)
{
	if (!pTarget)
		return NULL;
	TeamTypes eTeam = pTarget->getTeam();

	//change iteration order, don't return the same plot every time
	//don't use the RNG here: too many calls in the log and diplo quirks can lead to desyncs
	int aiShuffle[3][RING2_PLOTS] = { 
		{ 0,5,6,3,2,4,1,14,13,17,16,15,11,8,9,18,12,7,10 },
		{ 0,2,1,5,4,3,6,14,8,15,12,18,16,9,7,11,10,13,17 },
		{ 0,6,3,2,5,1,4,18,15,16,14,12,17,8,7,10,9,13,11 } };
	uint uShuffleType = GC.getGame().urandLimitExclusive(3, pTarget->GetPseudoRandomSeed());

	for(int iI = RING0_PLOTS; iI < RING2_PLOTS; iI++)
	{
		CvPlot* pAdjacentPlot = iterateRingPlots(pTarget->getX(), pTarget->getY(), aiShuffle[uShuffleType][iI]);
		if(pAdjacentPlot != NULL && 
			(!bCheckTeam || pAdjacentPlot->getTeam()==eTeam || pAdjacentPlot->getTeam()==NO_TEAM) && //ownership check
			pAdjacentPlot->isShallowWater() && //coastal
			pAdjacentPlot->getFeatureType()==NO_FEATURE && //no ice
			!pAdjacentPlot->isLake() && //no lake
			pAdjacentPlot->countPassableNeighbors(DOMAIN_SEA)>2) //no bays
		{
			return pAdjacentPlot;
		}
	}

	return NULL;
}

MultiunitFormationTypes MilitaryAIHelpers::GetCurrentBestFormationTypeForCombinedAttack()
{
	static EraTypes eThreshold = (EraTypes) GC.getInfoTypeForString("ERA_MEDIEVAL", true);
	return GC.getGame().getCurrentEra() > eThreshold ? MUFORMATION_LATE_GAME_INVASION_FORCE : MUFORMATION_NAVAL_INVASION;
}

MultiunitFormationTypes MilitaryAIHelpers::GetCurrentBestFormationTypeForPureNavalAttack()
{
	static EraTypes eThreshold = (EraTypes) GC.getInfoTypeForString("ERA_MEDIEVAL", true);
	return GC.getGame().getCurrentEra() > eThreshold ? MUFORMATION_LATE_GAME_PURE_NAVAL_FORCE : MUFORMATION_PURE_NAVAL_CITY_ATTACK;
}

MultiunitFormationTypes MilitaryAIHelpers::GetCurrentBestFormationTypeForLandAttack()
{
	switch (GC.getGame().getCurrentEra())
	{
	case 0: //ancient
		return MUFORMATION_EARLY_RUSH;
	case 1: //classic
	case 2: //medieval
		return MUFORMATION_SMALL_CITY_ATTACK_FORCE;
	case 3: //renaissance
	case 4: //industrial
		return MUFORMATION_BASIC_CITY_ATTACK_FORCE;
	case 5: //modern
	case 6: //atomic
	case 7: //information
		return MUFORMATION_BIGGER_CITY_ATTACK_FORCE;
	default:
		return MUFORMATION_BASIC_CITY_ATTACK_FORCE;
	}
}

const char * ArmyTypeToString(ArmyType eType)
{
	switch(eType)
	{
	case ARMY_TYPE_ANY:
		return "Undefined Army";
		break;
	case ARMY_TYPE_LAND:
		return "Land Army";
		break;
	case ARMY_TYPE_NAVAL:
		return "Naval Army";
		break;
	case ARMY_TYPE_COMBINED:
		return "Combined Army";
		break;
	case ARMY_TYPE_AIR:
		return "Air Army";
		break;
	case ARMY_TYPE_ESCORT_LAND:
		return "Land Escort";
		break;
	case ARMY_TYPE_ESCORT_SEA:
		return "Naval Escort";
		break;
	default:
		return "No Army";
	}
}

FDataStream& operator<<(FDataStream& saveTo, const CvAttackTarget& readFrom)
{
	saveTo << readFrom.m_armyType;
	saveTo << readFrom.m_iMusterPlotIndex;
	saveTo << readFrom.m_iStagingPlotIndex;
	saveTo << readFrom.m_iTargetPlotIndex;
	saveTo << readFrom.m_iPathLength;
	saveTo << readFrom.m_iApproachScore;
	saveTo << readFrom.m_bPreferred;
	return saveTo;
}

FDataStream& operator>>(FDataStream& loadFrom, CvAttackTarget& writeTo)
{
	loadFrom >> writeTo.m_armyType;
	loadFrom >> writeTo.m_iMusterPlotIndex;
	loadFrom >> writeTo.m_iStagingPlotIndex;
	loadFrom >> writeTo.m_iTargetPlotIndex;
	loadFrom >> writeTo.m_iPathLength;
	loadFrom >> writeTo.m_iApproachScore;
	loadFrom >> writeTo.m_bPreferred;
	return loadFrom;
}

void CvAttackTarget::SetWaypoints(const SPath& path)
{
	m_iPathLength = path.length();
	if (path.length() > 4)
	{
		m_iMusterPlotIndex = path.get(0)->GetPlotIndex();
		m_iStagingPlotIndex = path.get(-4)->GetPlotIndex();
		m_iTargetPlotIndex = path.get(-1)->GetPlotIndex();
	}
	else if (path.length() > 1)
	{
		m_iMusterPlotIndex = path.get(0)->GetPlotIndex();
		m_iStagingPlotIndex = path.get(0)->GetPlotIndex();
		m_iTargetPlotIndex = path.get(-1)->GetPlotIndex();
	}
	else
	{
		m_iMusterPlotIndex = -1;
		m_iStagingPlotIndex = -1;
		m_iTargetPlotIndex = -1;
	}
}

CvPlot* CvAttackTarget::GetMusterPlot() const
{
	return GC.getMap().plotByIndex(m_iMusterPlotIndex);
}

CvPlot* CvAttackTarget::GetStagingPlot() const
{
	return GC.getMap().plotByIndex(m_iStagingPlotIndex);
}

CvPlot* CvAttackTarget::GetTargetPlot() const
{
	return GC.getMap().plotByIndex(m_iTargetPlotIndex);
}

PlayerTypes CvAttackTarget::GetAttacker() const
{
	return GC.getMap().plotByIndex(m_iMusterPlotIndex)->getOwner();
}

PlayerTypes CvAttackTarget::GetDefender() const
{
	return GC.getMap().plotByIndex(m_iTargetPlotIndex)->getOwner();
}

int CvAttackTarget::GetPathLength() const
{
	return m_iPathLength;
}

bool CvAttackTarget::IsValid() const
{
	return m_iMusterPlotIndex >= 0 &&
		m_iStagingPlotIndex >= 0 &&
		m_iTargetPlotIndex >= 0;
}

bool CvAttackTarget::IsPreferred() const
{
	return m_bPreferred;
}
