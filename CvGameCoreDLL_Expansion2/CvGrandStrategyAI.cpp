/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#include "CvGameCoreDLLPCH.h"
#include "CvGrandStrategyAI.h"
#include "CvEconomicAI.h"
#include "CvCitySpecializationAI.h"
#include "CvDiplomacyAI.h"
#include "CvMinorCivAI.h"
#include "ICvDLLUserInterface.h"
#include "CvSpanSerialization.h"

// must be included after all other headers
#include "LintFree.h"

//------------------------------------------------------------------------------
CvAIGrandStrategyXMLEntry::CvAIGrandStrategyXMLEntry(void):
	m_piFlavorValue(NULL),
	m_piSpecializationBoost(NULL),
	m_piFlavorModValue(NULL)
{
}
//------------------------------------------------------------------------------
CvAIGrandStrategyXMLEntry::~CvAIGrandStrategyXMLEntry(void)
{
	SAFE_DELETE_ARRAY(m_piFlavorValue);
	SAFE_DELETE_ARRAY(m_piSpecializationBoost);
	SAFE_DELETE_ARRAY(m_piFlavorModValue);
}
//------------------------------------------------------------------------------
bool CvAIGrandStrategyXMLEntry::CacheResults(Database::Results& kResults, CvDatabaseUtility& kUtility)
{
	if(!CvBaseInfo::CacheResults(kResults, kUtility))
		return false;

	//Arrays
	const char* szType = GetType();
	kUtility.SetFlavors(m_piFlavorValue, "AIGrandStrategy_Flavors", "AIGrandStrategyType", szType);
	kUtility.SetYields(m_piSpecializationBoost, "AIGrandStrategy_Yields", "AIGrandStrategyType", szType);
	kUtility.SetFlavors(m_piFlavorModValue, "AIGrandStrategy_FlavorMods", "AIGrandStrategyType", szType);

	return true;
}

/// What Flavors will be added by adopting this Grand Strategy?
int CvAIGrandStrategyXMLEntry::GetFlavorValue(int i) const
{
	PRECONDITION(i < GC.getNumFlavorTypes(), "Index out of bounds");
	PRECONDITION(i > -1, "Index out of bounds");
	return m_piFlavorValue ? m_piFlavorValue[i] : -1;
}

/// What Flavors will be added by adopting this Grand Strategy?
int CvAIGrandStrategyXMLEntry::GetSpecializationBoost(YieldTypes eYield) const
{
	PRECONDITION(eYield < NUM_YIELD_TYPES, "Index out of bounds");
	PRECONDITION(eYield > -1, "Index out of bounds");
	return m_piSpecializationBoost ? m_piSpecializationBoost[(int)eYield] : 0;
}

/// What Flavors will be added by adopting this Grand Strategy?
int CvAIGrandStrategyXMLEntry::GetFlavorModValue(int i) const
{
	PRECONDITION(i < GC.getNumFlavorTypes(), "Index out of bounds");
	PRECONDITION(i > -1, "Index out of bounds");
	return m_piFlavorModValue ? m_piFlavorModValue[i] : 0;
}



//=====================================
// CvAIGrandStrategyXMLEntries
//=====================================
/// Constructor
CvAIGrandStrategyXMLEntries::CvAIGrandStrategyXMLEntries(void)
{

}

/// Destructor
CvAIGrandStrategyXMLEntries::~CvAIGrandStrategyXMLEntries(void)
{
	DeleteArray();
}

/// Returns vector of AIStrategy entries
std::vector<CvAIGrandStrategyXMLEntry*>& CvAIGrandStrategyXMLEntries::GetAIGrandStrategyEntries()
{
	return m_paAIGrandStrategyEntries;
}

/// Number of defined AIStrategies
int CvAIGrandStrategyXMLEntries::GetNumAIGrandStrategies()
{
	return m_paAIGrandStrategyEntries.size();
}

/// Clear AIStrategy entries
void CvAIGrandStrategyXMLEntries::DeleteArray()
{
	for(std::vector<CvAIGrandStrategyXMLEntry*>::iterator it = m_paAIGrandStrategyEntries.begin(); it != m_paAIGrandStrategyEntries.end(); ++it)
	{
		SAFE_DELETE(*it);
	}

	m_paAIGrandStrategyEntries.clear();
}

/// Get a specific entry
CvAIGrandStrategyXMLEntry* CvAIGrandStrategyXMLEntries::GetEntry(int index)
{
	return (index!=NO_AIGRANDSTRATEGY) ? m_paAIGrandStrategyEntries[index] : NULL;
}



//=====================================
// CvGrandStrategyAI
//=====================================
/// Constructor
CvGrandStrategyAI::CvGrandStrategyAI():
	m_paiGrandStrategyPriority(NULL),
	m_iCachedMetaPosition(0),
	m_iCachedMetaTurn(-1),
	m_iCachedGreedyKind(STRATEGY_GREEDY_TALL),
	m_iCachedStrikeStart(-1),
	m_iCachedStrikeEnd(-1),
	m_iCachedSpikeTech(NO_TECH),
	m_iCachedStrikeDeclareBy(-1),
	m_iCachedCoreUnitClass(NO_UNITCLASS),
	m_iCachedCoreSharePercent(0),
	m_bCachedMilitaryFirst(false),
	m_iCachedUpgradeTech(NO_TECH),
	m_iCachedUpgradeTurn(-1),
	m_iCachedStrikePlanTurn(-1),
	m_iCachedOutOfRoom(0),
	m_iCachedOutOfRoomTurn(-1),
	m_iCachedWarDefenceCap(0),
	m_iCachedWarDefenceTurn(-1)
{
}

/// Destructor
CvGrandStrategyAI::~CvGrandStrategyAI(void)
{
}

/// Initialize
void CvGrandStrategyAI::Init(CvAIGrandStrategyXMLEntries* pAIGrandStrategies, CvPlayer* pPlayer)
{
	// Store off the pointer to the AIStrategies active for this game
	m_pAIGrandStrategies = pAIGrandStrategies;

	m_pPlayer = pPlayer;

	// Initialize AIGrandStrategy status array
	ASSERT(m_paiGrandStrategyPriority==NULL, "about to leak memory, CvGrandStrategyAI::m_paiGrandStrategyPriority");
	m_paiGrandStrategyPriority = FNEW(int[m_pAIGrandStrategies->GetNumAIGrandStrategies()], c_eCiv5GameplayDLL, 0);

	Reset();
}

/// Deallocate memory created in initialize
void CvGrandStrategyAI::Uninit()
{
	SAFE_DELETE_ARRAY(m_paiGrandStrategyPriority);
}

/// Reset AIStrategy status array to all false
void CvGrandStrategyAI::Reset()
{
	int iI = 0;

	m_iNumTurnsSinceActiveSet = 0;

	m_eActiveGrandStrategy = NO_AIGRANDSTRATEGY;

	for(iI = 0; iI < m_pAIGrandStrategies->GetNumAIGrandStrategies(); iI++)
	{
		m_paiGrandStrategyPriority[iI] = -1;
	}

	for(iI = 0; iI < MAX_MAJOR_CIVS; iI++)
	{
		m_eGuessOtherPlayerActiveGrandStrategy[iI] = NO_AIGRANDSTRATEGY;
		m_eGuessOtherPlayerActiveGrandStrategyConfidence[iI] = GUESS_CONFIDENCE_UNSURE;
	}

	//defaults
	m_iFlavorGold = 7;
	m_iFlavorScience = 7;
	m_iFlavorCulture = 7;
	m_iFlavorProduction = 7;
	m_iFlavorFaith = 7;
	m_iFlavorHappiness = 7;
	m_iFlavorGrowth = 7;
	m_iFlavorDiplomacy = 7;

	m_iCachedMetaPosition = 0;
	m_iCachedMetaTurn = -1;
	m_iCachedGreedyKind = STRATEGY_GREEDY_TALL;
	m_iCachedStrikeStart = -1;
	m_iCachedStrikeEnd = -1;
	m_iCachedSpikeTech = NO_TECH;
	m_iCachedStrikeDeclareBy = -1;
	m_iCachedCoreUnitClass = NO_UNITCLASS;
	m_iCachedCoreSharePercent = 0;
	m_bCachedMilitaryFirst = false;
	m_iCachedUpgradeTech = NO_TECH;
	m_iCachedUpgradeTurn = -1;
	m_iCachedStrikePlanTurn = -1;
	m_iCachedOutOfRoom = 0;
	m_iCachedOutOfRoomTurn = -1;
	m_iCachedWarDefenceCap = 0;
	m_iCachedWarDefenceTurn = -1;
}

///
template<typename GrandStrategyAI, typename Visitor>
void CvGrandStrategyAI::Serialize(GrandStrategyAI& grandStrategyAI, Visitor& visitor)
{
	visitor(grandStrategyAI.m_iNumTurnsSinceActiveSet);
	visitor(grandStrategyAI.m_eActiveGrandStrategy);

	ASSERT(grandStrategyAI.m_pAIGrandStrategies != NULL && grandStrategyAI.m_pAIGrandStrategies->GetNumAIGrandStrategies() > 0, "Number of AIGrandStrategies to serialize is expected to greater than 0");
	visitor(MakeConstSpan(grandStrategyAI.m_paiGrandStrategyPriority, grandStrategyAI.m_pAIGrandStrategies->GetNumAIGrandStrategies()));

	visitor(grandStrategyAI.m_eGuessOtherPlayerActiveGrandStrategy);
	visitor(grandStrategyAI.m_eGuessOtherPlayerActiveGrandStrategyConfidence);
}

/// Serialization read
void CvGrandStrategyAI::Read(FDataStream& kStream)
{
	CvStreamLoadVisitor serialVisitor(kStream);
	CvGrandStrategyAI::Serialize(*this, serialVisitor);

	// AI_AB_TEST (Personality): the meta position and war defence cap are worked out again from the loaded game
	m_iCachedMetaTurn = -1;
	m_iCachedOutOfRoomTurn = -1;
	m_iCachedStrikePlanTurn = -1;
	m_iCachedWarDefenceTurn = -1;
}

/// Serialization write
void CvGrandStrategyAI::Write(FDataStream& kStream) const
{
	CvStreamSaveVisitor serialVisitor(kStream);
	CvGrandStrategyAI::Serialize(*this, serialVisitor);
}

FDataStream& operator>>(FDataStream& stream, CvGrandStrategyAI& grandStrategyAI)
{
	grandStrategyAI.Read(stream);
	return stream;
}
FDataStream& operator<<(FDataStream& stream, const CvGrandStrategyAI& grandStrategyAI)
{
	grandStrategyAI.Write(stream);
	return stream;
}

/// Returns the Player object the Strategies are associated with
CvPlayer* CvGrandStrategyAI::GetPlayer()
{
	return m_pPlayer;
}

/// Returns AIGrandStrategies object stored in this class
CvAIGrandStrategyXMLEntries* CvGrandStrategyAI::GetAIGrandStrategies()
{
	return m_pAIGrandStrategies;
}

/// Runs every turn to determine what the player's Active Grand Strategy is and to change Priority Levels as necessary
void CvGrandStrategyAI::DoTurn()
{
	DoGuessOtherPlayersActiveGrandStrategy();

	// AI_AB_TEST (Personality): the strategy profile, logged only
	LogStrategyProfile();
	// AI_AB_TEST (Personality, the styles and MetaDial): refuse city-state unit gifts at the unit cap
	DoStyleCityStateUnitGifts();
	// AI_AB_TEST (Personality, GreedyFewUnits and MetaDial's greedy side): back down to the cap in peace
	DoGreedyDisbandToCap();

	int iGrandStrategiesLoop = 0;
	AIGrandStrategyTypes eGrandStrategy;
	CvAIGrandStrategyXMLEntry* pGrandStrategy = NULL;
	CvString strGrandStrategyName;

	//Only run this on turns we need it.
	if ((GetActiveGrandStrategy() != NO_AIGRANDSTRATEGY && GetNumTurnsSinceActiveSet() == 0) || (GetActiveGrandStrategy() == NO_AIGRANDSTRATEGY))
	{
		// Loop through all GrandStrategies to set their Priorities
		for (iGrandStrategiesLoop = 0; iGrandStrategiesLoop < GetAIGrandStrategies()->GetNumAIGrandStrategies(); iGrandStrategiesLoop++)
		{
			eGrandStrategy = (AIGrandStrategyTypes)iGrandStrategiesLoop;
			pGrandStrategy = GetAIGrandStrategies()->GetEntry(iGrandStrategiesLoop);
			strGrandStrategyName = (CvString)pGrandStrategy->GetType();

			// Base Priority looks at Personality Flavors (0 - 10) and multiplies * the Flavors attached to a Grand Strategy (0-10),
			// so expect a number between 0 and 100 back from this
			int iPriority = GetBaseGrandStrategyPriority(eGrandStrategy);

			int iTempPriority = 0;
			if (strGrandStrategyName == "AIGRANDSTRATEGY_CONQUEST")
			{
				iTempPriority += GetConquestPriority();
			}
			else if (strGrandStrategyName == "AIGRANDSTRATEGY_CULTURE")
			{
				iTempPriority += GetCulturePriority();
			}
			else if (strGrandStrategyName == "AIGRANDSTRATEGY_UNITED_NATIONS")
			{
				iTempPriority += GetUnitedNationsPriority();
			}
			else if (strGrandStrategyName == "AIGRANDSTRATEGY_SPACESHIP")
			{
				iTempPriority += GetSpaceshipPriority();
			}

			//reduce the potency of these until the mid game.
			int MaxTurn = GC.getGame().getEstimateEndTurn();
			if (MaxTurn > 0)
			{
				iTempPriority *= (GC.getGame().getGameTurn() * 2);
				iTempPriority /= MaxTurn;
			}

			iPriority += iTempPriority;

			// Give a boost to the current strategy so that small fluctuation doesn't cause a big change
			if (GetActiveGrandStrategy() == eGrandStrategy && GetActiveGrandStrategy() != NO_AIGRANDSTRATEGY)
			{
				iPriority += /*50*/ GD_INT_GET(AI_GRAND_STRATEGY_CURRENT_STRATEGY_WEIGHT);
			}

			SetGrandStrategyPriority(eGrandStrategy, max(1, iPriority));
		}
		// Now look at what we think the other players in the game are up to - we might have an opportunity to capitalize somewhere
		vector<int> viNumGrandStrategiesAdopted;
		int iNumPlayers = 0;

		// Init vector
		for (iGrandStrategiesLoop = 0; iGrandStrategiesLoop < GetAIGrandStrategies()->GetNumAIGrandStrategies(); iGrandStrategiesLoop++)
		{
			iNumPlayers = 0;

			// Tally up how many players we think are pusuing each Grand Strategy
			for (int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
			{
				PlayerTypes ePlayer = (PlayerTypes)iMajorLoop;
				if (ePlayer == NO_PLAYER || ePlayer == GetPlayer()->GetID())
					continue;

				if (GET_PLAYER(ePlayer).isAlive())
				{
					if (GET_TEAM(GetPlayer()->getTeam()).isHasMet(GET_PLAYER(ePlayer).getTeam()))
					{
						if (GetGuessOtherPlayerActiveGrandStrategy(ePlayer) == (AIGrandStrategyTypes)iGrandStrategiesLoop)
						{
							if (OtherPlayerDoingBetterThanUs(ePlayer, (AIGrandStrategyTypes)iGrandStrategiesLoop))
							{
								iNumPlayers++;
							}
						}
					}
				}
			}

			viNumGrandStrategiesAdopted.push_back(iNumPlayers);
		}

		vector<int> viGrandStrategyChangeForLogging;

		// Now modify our preferences based on how many people are going for stuff
		for (iGrandStrategiesLoop = 0; iGrandStrategiesLoop < GetAIGrandStrategies()->GetNumAIGrandStrategies(); iGrandStrategiesLoop++)
		{
			eGrandStrategy = (AIGrandStrategyTypes)iGrandStrategiesLoop;

			//Get 1/3.
			int iFraction = (GetGrandStrategyPriority(eGrandStrategy) * /*33*/ GD_INT_GET(AI_GRAND_STRATEGY_OTHER_PLAYERS_GS_MULTIPLIER)) / 100;
			int iNumPursuing = viNumGrandStrategiesAdopted[eGrandStrategy];
			int iChange = 0;
			if (iNumPursuing > 0)
			{
				//Modified to only reduce based on known and if we're doing better/worse than them.
				//If 3+ are doing better than us at this GS, we need to move on.
				iChange = min(iFraction * iNumPursuing, GetGrandStrategyPriority(eGrandStrategy));
			}

			ChangeGrandStrategyPriority(eGrandStrategy, -iChange);
			viGrandStrategyChangeForLogging.push_back(-iChange);
		}
		// Now see which Grand Strategy should be active, based on who has the highest Priority right now
		// Grand Strategy must be run for at least 10 turns
		int iBestPriority = -1;
		int iPriority = 0;

		AIGrandStrategyTypes eBestGrandStrategy = NO_AIGRANDSTRATEGY;

		for (iGrandStrategiesLoop = 0; iGrandStrategiesLoop < GetAIGrandStrategies()->GetNumAIGrandStrategies(); iGrandStrategiesLoop++)
		{
			eGrandStrategy = (AIGrandStrategyTypes)iGrandStrategiesLoop;

			iPriority = GetGrandStrategyPriority(eGrandStrategy);

			if (iPriority > iBestPriority)
			{
				iBestPriority = iPriority;
				eBestGrandStrategy = eGrandStrategy;
			}
		}

		if (eBestGrandStrategy != GetActiveGrandStrategy())
		{
			SetActiveGrandStrategy(eBestGrandStrategy);
			m_pPlayer->GetCitySpecializationAI()->SetSpecializationsDirty(SPECIALIZATION_UPDATE_NEW_GRAND_STRATEGY);
		}
		ChangeNumTurnsSinceActiveSet(1);

		LogGrandStrategies(viGrandStrategyChangeForLogging);
	}
	else
	{
		ChangeNumTurnsSinceActiveSet(1);
		if (GetNumTurnsSinceActiveSet() >= /*10*/ GD_INT_GET(AI_GRAND_STRATEGY_NUM_TURNS_STRATEGY_MUST_BE_ACTIVE))
		{
			//Reset to zero.
			SetNumTurnsSinceActiveSet(0);
		}
	}

	// Unused
	if (!m_pPlayer->isHuman())
	{
		m_iFlavorGold = GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_GOLD"));
		m_iFlavorScience = GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_SCIENCE"));
		m_iFlavorCulture = GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_CULTURE"));
		m_iFlavorProduction = GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_PRODUCTION"));
		m_iFlavorFaith = GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_RELIGION"));
		m_iFlavorHappiness = GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_HAPPINESS"));
		m_iFlavorGrowth = GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_GROWTH"));
		m_iFlavorDiplomacy = GetPersonalityAndGrandStrategy((FlavorTypes)GC.getInfoTypeForString("FLAVOR_DIPLOMACY"));
	}

}

/// Returns Priority for Conquest Grand Strategy
int CvGrandStrategyAI::GetConquestPriority()
{
	int iPriority = 0;

	// If Conquest Victory isn't even available then don't bother with anything
	VictoryTypes eVictory = (VictoryTypes)GC.getInfoTypeForString("VICTORY_DOMINATION", true);
	if (eVictory == NO_VICTORY || !GC.getGame().isVictoryValid(eVictory))
	{
		if (!GC.getGame().areNoVictoriesValid())
		{
			return -100;
		}
	}
	
	// Game options have disabled war as an option to win, so pick something else
	if (!GC.getGame().CanPlayerAttemptDominationVictory(GetPlayer()->GetID(), NO_PLAYER, GC.getGame().areNoVictoriesValid()))
		return -100;

	// If we're close to winning, let's go to the finish line!
	if (GetPlayer()->GetDiplomacyAI()->IsCloseToWorldConquest())
	{
		iPriority += 9000;
	}

	int iGeneralWarlikeness = GetPlayer()->GetDiplomacyAI()->GetMajorCivApproachBias(CIV_APPROACH_WAR);
	int iGeneralHostility = GetPlayer()->GetDiplomacyAI()->GetMajorCivApproachBias(CIV_APPROACH_HOSTILE);
	int iGeneralDeceptiveness = GetPlayer()->GetDiplomacyAI()->GetMajorCivApproachBias(CIV_APPROACH_DECEPTIVE);
	int iGeneralFriendliness = GetPlayer()->GetDiplomacyAI()->GetMajorCivApproachBias(CIV_APPROACH_FRIENDLY);

	int iGeneralApproachModifier = max(max(iGeneralDeceptiveness, iGeneralHostility), iGeneralWarlikeness) - iGeneralFriendliness;
	// Boldness gives the base weight for Conquest (no flavors added earlier)
	int iEra = m_pPlayer->GetCurrentEra();
	if(iEra <= 0)
	{
		iEra = 1;
	}
	iPriority += (GetPlayer()->GetDiplomacyAI()->GetBoldness() + iGeneralApproachModifier + GetPlayer()->GetDiplomacyAI()->GetMeanness()) * (10 - iEra); // make a little less likely as time goes on

	CvTeam& pTeam = GET_TEAM(GetPlayer()->getTeam());

	// How many turns must have passed before we test for having met nobody?
	if (GC.getGame().getElapsedGameTurns() >= /*20*/ GD_INT_GET(AI_GS_CONQUEST_NOBODY_MET_FIRST_TURN))
	{
		// If we haven't met any Major Civs yet, then we probably shouldn't be planning on conquering the world
		bool bHasMetMajor = false;

		for (int iTeamLoop = 0; iTeamLoop < MAX_CIV_TEAMS; iTeamLoop++)
		{
			if (pTeam.GetID() != iTeamLoop && !GET_TEAM((TeamTypes)iTeamLoop).isMinorCiv())
			{
				if (pTeam.isHasMet((TeamTypes)iTeamLoop))
				{
					bHasMetMajor = true;
					break;
				}
			}
		}
		if (!bHasMetMajor)
		{
			iPriority += /*-100*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_NOBODY_MET_WEIGHT);
		}
	}

	int iNumConquests = GetPlayer()->GetDiplomacyAI()->GetPlayerNumMajorsConquered(GetPlayer()->GetID());
	if (iNumConquests == 1)
	{
		iPriority += 50;
	}
	else if (iNumConquests > 1)
	{
		iPriority += (iNumConquests * 125);
	}

	// How many turns must have passed before we test for us having a weak military?
	if (GC.getGame().getElapsedGameTurns() >= /*60*/ GD_INT_GET(AI_GS_CONQUEST_MILITARY_STRENGTH_FIRST_TURN))
	{
		// Compare our military strength to the rest of the world
		int iWorldMilitaryStrength = GC.getGame().GetWorldMilitaryStrengthAverage(GetPlayer()->GetID(), true, true);

		//Reduce world average if we're rocking multiple capitals.
		if (iNumConquests > 0)
		{
			iWorldMilitaryStrength *= 100;
			iWorldMilitaryStrength /= (100 + (iNumConquests * 10));
		}

		if (iWorldMilitaryStrength > 0)
		{
			int iMilitaryRatio = (GetPlayer()->GetMilitaryMight() - iWorldMilitaryStrength) * /*100*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_POWER_RATIO_MULTIPLIER) / iWorldMilitaryStrength;

			// Make the likelihood of BECOMING a warmonger lower than dropping the bad behavior
			if (iMilitaryRatio > 0)
				iMilitaryRatio /= 2;

			iPriority += iMilitaryRatio;	// This will add between -100 and 100 depending on this player's MilitaryStrength relative the world average. The number will typically be near 0 though, as it's fairly hard to get away from the world average
		}
	}

	// If we're at war, then boost the weight a bit
	if (pTeam.getAtWarCount(/*bIgnoreMinors*/ false) > 0)
	{
		iPriority += /*10*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_AT_WAR_WEIGHT);
	}

	// If our neighbors are cramping our style, consider less... scrupulous means of obtaining more land
	int iNumPlayersMet = 1;	// Include 1 for me!
	int iTotalLandMe = 0;
	int iTotalLandPlayersMet = 0;

	bool bDesperate = GetPlayer()->GetPlayerPolicies()->GetLateGamePolicyTree() != NO_POLICY_BRANCH_TYPE && !GetPlayer()->GetDiplomacyAI()->IsCloseToWorldConquest() && !GetPlayer()->GetDiplomacyAI()->IsCloseToCultureVictory() && !GetPlayer()->GetDiplomacyAI()->IsCloseToDiploVictory() && !GetPlayer()->GetDiplomacyAI()->IsCloseToSpaceshipVictory();
	int iTotalNumDangerPlayers = 0;

	// Count the number of Majors we know
	for (int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		PlayerTypes ePlayer = (PlayerTypes) iMajorLoop;

		if (GET_PLAYER(ePlayer).isAlive() && iMajorLoop != GetPlayer()->GetID())
		{
			if (pTeam.isHasMet(GET_PLAYER(ePlayer).getTeam()))
			{
				iNumPlayersMet++;
				if (GetPlayer()->GetPlayerPolicies()->GetLateGamePolicyTree() != NO_POLICY_BRANCH_TYPE)
				{
					if (GET_PLAYER(ePlayer).GetDiplomacyAI()->IsCloseToCultureVictory() || GET_PLAYER(ePlayer).GetDiplomacyAI()->IsCloseToDiploVictory() || GET_PLAYER(ePlayer).GetDiplomacyAI()->IsCloseToSpaceshipVictory())
					{
						//Close to nothing, and someone else is? Eek!
						if (bDesperate)
							iTotalNumDangerPlayers += 25;
						//someone else is close, but we are too? Let's keep an eye on domination as an option.
						else
							iTotalNumDangerPlayers += 5;
					}
					//They have an ideology, and we don't? Uh-oh.
					else if (bDesperate)
						iTotalNumDangerPlayers += 5;
				}
			}
		}
	}

	if (iNumPlayersMet > 0)
	{
		// Check every plot for ownership
		for (int iPlotLoop = 0; iPlotLoop < GC.getMap().numPlots(); iPlotLoop++)
		{
			if (GC.getMap().plotByIndexUnchecked(iPlotLoop)->isOwned())
			{
				PlayerTypes ePlayer = GC.getMap().plotByIndexUnchecked(iPlotLoop)->getOwner();

				if (ePlayer == GetPlayer()->GetID())
				{
					iTotalLandPlayersMet++;
					iTotalLandMe++;
				}
				else if (!GET_PLAYER(ePlayer).isMinorCiv() && pTeam.isHasMet(GET_PLAYER(ePlayer).getTeam()))
				{
					iTotalLandPlayersMet++;
				}
			}
		}

		iTotalLandPlayersMet /= iNumPlayersMet;

		if (iTotalLandMe > 0)
		{
			if (iTotalLandPlayersMet / iTotalLandMe > 1)
			{
				iPriority += GetPlayer()->IsCramped() ? /*100*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_CRAMPED_WEIGHT) * 5 : /*20*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_CRAMPED_WEIGHT);
			}
		}
		if (iTotalNumDangerPlayers > 0)
		{
			iPriority += iTotalNumDangerPlayers * iEra;
		}
	}
	// if we do not have nukes and we know someone else who does...
	if (GetPlayer()->getNumNukeUnits() == 0)
	{
		for (int iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
		{
			PlayerTypes ePlayer = (PlayerTypes)iMajorLoop;

			if (GET_PLAYER(ePlayer).isAlive() && iMajorLoop != GetPlayer()->GetID())
			{
				if (pTeam.isHasMet(GET_PLAYER(ePlayer).getTeam()))
				{
					if (GET_PLAYER(ePlayer).getNumNukeUnits() > 0)
					{
						iPriority -= 50;
						break;
					}
				}
			}
		}
	}

	int iPriorityBonus = 0;
	//Add priority value based on flavors of policies we've acquired.
	for (int iPolicyLoop = 0; iPolicyLoop < GC.getNumPolicyInfos(); iPolicyLoop++)
	{
		const PolicyTypes ePolicy = static_cast<PolicyTypes>(iPolicyLoop);
		CvPolicyEntry* pkPolicyInfo = GC.getPolicyInfo(ePolicy);
		if (pkPolicyInfo)
		{
			if (GetPlayer()->GetPlayerPolicies()->HasPolicy(ePolicy) && !GetPlayer()->GetPlayerPolicies()->IsPolicyBlocked(ePolicy))
			{
				for (int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
				{
					if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_OFFENSE")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_MOBILE")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_MILITARY_TRAINING")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_NAVAL")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_NAVAL_RECON")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_RANGED")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_AIR")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}

					//AI_UNIT_PRODUCTION
					else if (MOD_AI_UNIT_PRODUCTION)
					{
						if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_ARCHER")
						{
							iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_SIEGE")
						{
							iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_SKIRMISHER")
						{
							iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_NAVAL_MELEE")
						{
							iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_NAVAL_RANGED")
						{
							iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_SUBMARINE")
						{
							iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_FIGHTER")
						{
							iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_BOMBER")
						{
							iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
						}
					}
				}
			}
		}
	}
	//Look for Buildings and grab flavors.
	int iLoop = 0;
	for (CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop))
	{
		//Add in our base production value.
		iPriorityBonus += (pLoopCity->getProduction() / 10);

		int iNumBuildingInfos = GC.getNumBuildingInfos();
		for (int iI = 0; iI < iNumBuildingInfos; iI++)
		{
			const BuildingTypes eBuildingLoop = static_cast<BuildingTypes>(iI);
			if (eBuildingLoop == NO_BUILDING)
				continue;
			if (pLoopCity->GetCityBuildings()->GetNumBuilding(eBuildingLoop) > 0)
			{
				CvBuildingEntry* pkLoopBuilding = GC.getBuildingInfo(eBuildingLoop);
				if (pkLoopBuilding)
				{
					for (int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
					{
						if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_OFFENSE")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop) * 2;
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_MILITARY_TRAINING")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_NAVAL")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
					}
				}
			}
		}
	}
	/////////
	//RELIGION CHECKS
	////////////
	ReligionTypes eReligion = m_pPlayer->GetReligions()->GetStateReligion();
	if (eReligion != NO_RELIGION)
	{
		const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eReligion, m_pPlayer->GetID());
		if (pReligion)
		{
			CvCity* pHolyCity = pReligion->GetHolyCity();
			CvBeliefXMLEntries* pkBeliefs = GC.GetGameBeliefs();
			const int iNumBeliefs = pkBeliefs->GetNumBeliefs();
			for (int iI = 0; iI < iNumBeliefs; iI++)
			{
				const BeliefTypes eBelief(static_cast<BeliefTypes>(iI));
				CvBeliefEntry* pEntry = pkBeliefs->GetEntry(eBelief);
				if (pEntry && pReligion->m_Beliefs.HasBelief(eBelief) && pReligion->m_Beliefs.IsBeliefValid(eBelief, eReligion, m_pPlayer->GetID(), pHolyCity))
				{
					if (pEntry->GetCombatModifierEnemyCities() > 0)
					{
						iPriorityBonus += pEntry->GetCombatModifierEnemyCities();
					}
					if (pEntry->GetCombatBonusTheirLands() > 0)
					{
						iPriorityBonus += pEntry->GetCombatBonusTheirLands();
					}
					if (pEntry->GetCombatBonusVersusOtherReligionTheirLands() > 0)
					{
						iPriorityBonus += pEntry->GetCombatBonusVersusOtherReligionTheirLands();
					}
					if (pEntry->GetFaithFromKills() > 0)
					{
						iPriorityBonus += pEntry->GetFaithFromKills();
					}
					for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
					{
						const YieldTypes eYield = static_cast<YieldTypes>(iI);
						if (eYield != NO_YIELD)
						{
							if (pEntry->GetYieldFromConquest(eYield) > 0)
							{
								iPriorityBonus += pEntry->GetYieldFromConquest(eYield);
							}
							if (pEntry->GetYieldFromKills(eYield) > 0)
							{
								iPriorityBonus += pEntry->GetYieldFromKills(eYield);
							}
						}
					}
				}
			}
		}
	}
	iPriorityBonus /= 8;

	if (m_pPlayer->GetPlayerTraits()->IsWarmonger())
	{
		iPriorityBonus += 100;
	}
	if (m_pPlayer->GetPlayerTraits()->IsSmaller())
	{
		iPriorityBonus -= 100;
	}
	if (m_pPlayer->GetPlayerTraits()->IsTourism())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsDiplomat())
	{
		iPriorityBonus += -25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsNerd())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsReligious())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsExpansionist())
	{
		iPriorityBonus += 50;
	}

	iPriority += iPriorityBonus;

	return iPriority;
}

/// Returns Priority for Culture Grand Strategy
int CvGrandStrategyAI::GetCulturePriority()
{
	int iPriority = 0;

	// If Culture Victory isn't even available then don't bother with anything
	VictoryTypes eVictory = (VictoryTypes) GC.getInfoTypeForString("VICTORY_CULTURAL", true);
	if(eVictory == NO_VICTORY || !GC.getGame().isVictoryValid(eVictory))
	{
		return -100;
	}

	// If we're close to winning, let's go to the finish line!
	if (GetPlayer()->GetDiplomacyAI()->IsCloseToCultureVictory())
	{
		iPriority += 8000;
	}

	// Before tourism kicks in, add weight based on flavor
	int iFlavorCulture =  m_pPlayer->GetFlavorManager()->GetPersonalityIndividualFlavor((FlavorTypes)GC.getInfoTypeForString("FLAVOR_CULTURE"));

	int iEra = m_pPlayer->GetCurrentEra();
	if(iEra <= 0)
	{
		iEra = 1;
	}
	iPriority += ((iEra * iFlavorCulture * 150) / 100);

	// Loop through Players to see how we are doing on Tourism and Culture
	PlayerTypes eLoopPlayer;
	int iOurCulture = m_pPlayer->GetTotalJONSCulturePerTurnTimes100();
	int iOurTourism = m_pPlayer->GetCulture()->GetTourism();
	int iNumCivsBehindCulture = 0;
	int iNumCivsAheadCulture = 0;
	int iNumCivsBehindTourism = 0;
	int iNumCivsAheadTourism = 0;
	int iNumCivsAlive = 0;

	for(int iPlayerLoop = 0; iPlayerLoop < MAX_CIV_PLAYERS; iPlayerLoop++)
	{
		eLoopPlayer = (PlayerTypes) iPlayerLoop;
		CvPlayer &kPlayer = GET_PLAYER(eLoopPlayer);

		if (kPlayer.isAlive() && !kPlayer.isMinorCiv() && !kPlayer.isBarbarian() && iPlayerLoop != m_pPlayer->GetID())
		{
			if (iOurCulture > kPlayer.GetTotalJONSCulturePerTurnTimes100())
			{
				iNumCivsAheadCulture++;
			}
			else
			{
				iNumCivsBehindCulture++;
			}
			if (iOurTourism > kPlayer.GetCulture()->GetTourism())
			{
				iNumCivsAheadTourism++;
			}
			else
			{
				iNumCivsBehindTourism++;
			}
			iNumCivsAlive++;
		}
	}

	if (iNumCivsAlive > 0 && iNumCivsAheadCulture > iNumCivsBehindCulture)
	{
		iPriority += /*50*/ GD_INT_GET(AI_GS_CULTURE_AHEAD_WEIGHT) * (iNumCivsAheadCulture - iNumCivsBehindCulture) / iNumCivsAlive;
	}
	if (iNumCivsAlive > 0 && iNumCivsAheadTourism > iNumCivsBehindTourism)
	{
		iPriority += /*100*/ GD_INT_GET(AI_GS_CULTURE_TOURISM_AHEAD_WEIGHT) * (iNumCivsAheadTourism - iNumCivsBehindTourism) / iNumCivsAlive;
	}
	if (iNumCivsAlive > 0 && iNumCivsAheadTourism < iNumCivsBehindTourism)
	{
		iPriority -= /*100*/ GD_INT_GET(AI_GS_CULTURE_TOURISM_AHEAD_WEIGHT) * (iNumCivsBehindTourism - iNumCivsAheadTourism) / iNumCivsAlive;
	}

	// for every civ we are Influential over increase this
	int iNumInfluential = m_pPlayer->GetCulture()->GetNumCivsInfluentialOn();
	iPriority += iNumInfluential * /*50*/ GD_INT_GET(AI_GS_CULTURE_INFLUENTIAL_CIV_MOD);

	int iPriorityBonus = 0;

	//Add in our base culture value.
	iPriorityBonus += (m_pPlayer->getJONSCultureTimes100() / 3000);
	iPriorityBonus += (m_pPlayer->GetCulture()->GetTourism() / 130);

	iPriorityBonus += (m_pPlayer->GetNumEffectiveCities() * -10);

	//Add priority value based on flavors of policies we've acquired.
	for(int iPolicyLoop = 0; iPolicyLoop < GC.getNumPolicyInfos(); iPolicyLoop++)
	{
		const PolicyTypes ePolicy = static_cast<PolicyTypes>(iPolicyLoop);
		CvPolicyEntry* pkPolicyInfo = GC.getPolicyInfo(ePolicy);
		if(pkPolicyInfo)
		{
			if(GetPlayer()->GetPlayerPolicies()->HasPolicy(ePolicy) && !GetPlayer()->GetPlayerPolicies()->IsPolicyBlocked(ePolicy))
			{
				for(int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
				{
					if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_GREAT_PEOPLE")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_CULTURE")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_WONDER")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_HAPPINESS")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
				}
			}
		}
	}
	//Look for Buildings and grab flavors.
	int iLoop = 0;
	for (CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop)) 
	{
		int iNumBuildingInfos = GC.getNumBuildingInfos();
		for(int iI = 0; iI < iNumBuildingInfos; iI++)
		{
			const BuildingTypes eBuildingLoop = static_cast<BuildingTypes>(iI);
			if(eBuildingLoop == NO_BUILDING)
				continue;
			if(pLoopCity->GetCityBuildings()->GetNumBuilding(eBuildingLoop) > 0)
			{
				CvBuildingEntry* pkLoopBuilding = GC.getBuildingInfo(eBuildingLoop);
				if(pkLoopBuilding)
				{
					for(int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
					{
						if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_CULTURE")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_GREAT_PEOPLE")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_WONDER")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_HAPPINESS")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
					}
				}
			}
		}
	}
	/////////
	//RELIGION CHECKS
	////////////
	ReligionTypes eReligion = m_pPlayer->GetReligions()->GetStateReligion();
	if (eReligion != NO_RELIGION)
	{
		const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eReligion, m_pPlayer->GetID());
		if (pReligion)
		{
			CvCity* pHolyCity = pReligion->GetHolyCity();
			CvBeliefXMLEntries* pkBeliefs = GC.GetGameBeliefs();
			const int iNumBeliefs = pkBeliefs->GetNumBeliefs();
			for(int iI = 0; iI < iNumBeliefs; iI++)
			{
				const BeliefTypes eBelief(static_cast<BeliefTypes>(iI));
				CvBeliefEntry* pEntry = pkBeliefs->GetEntry(eBelief);
				if (pEntry && pReligion->m_Beliefs.HasBelief(eBelief) && pReligion->m_Beliefs.IsBeliefValid(eBelief, eReligion, m_pPlayer->GetID(), pHolyCity))
				{
					if(pEntry->FaithPurchaseAllGreatPeople())
					{
						iPriorityBonus += 100;
					}
					int iNumBuildingInfos = GC.getNumBuildingInfos();
					for(int iI = 0; iI < iNumBuildingInfos; iI++)
					{
						const BuildingTypes eBuildingLoop = static_cast<BuildingTypes>(iI);
						if(eBuildingLoop == NO_BUILDING)
							continue;

						CvBuildingEntry* pkLoopBuilding = GC.getBuildingInfo(eBuildingLoop);
						if(pkLoopBuilding)
						{
							if(pEntry->GetBuildingClassYieldChange(pkLoopBuilding->GetBuildingClassType(), YIELD_TOURISM))
							{
								iPriorityBonus += 100;
								break;
							}
						}
					}
					if(pEntry->GetFaithBuildingTourism() > 0)
					{
						iPriorityBonus += 100;
					}
					for(int iI = 0; iI < NUM_YIELD_TYPES; iI++)
					{
						const YieldTypes eYield = static_cast<YieldTypes>(iI);
						if(eYield != NO_YIELD)
						{
							if(pEntry->GetYieldFromPolicyUnlock(eYield) > 0)
							{
								iPriorityBonus += pEntry->GetYieldFromPolicyUnlock(eYield);
							}
						}
					}
				}
			}
		}
	}
	iPriorityBonus /= 8;

	if (m_pPlayer->GetPlayerTraits()->IsWarmonger())
	{
		iPriorityBonus -= 50;
	}
	if (m_pPlayer->GetPlayerTraits()->IsSmaller())
	{
		iPriorityBonus += 50;
	}
	if (m_pPlayer->GetPlayerTraits()->IsTourism())
	{
		iPriorityBonus += 100;
	}
	if (m_pPlayer->GetPlayerTraits()->IsDiplomat())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsNerd())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsReligious())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsExpansionist())
	{
		iPriorityBonus -= 50;
	}

	iPriority += iPriorityBonus;

	return iPriority;
}

/// Returns Priority for United Nations Grand Strategy
int CvGrandStrategyAI::GetUnitedNationsPriority()
{
	int iPriority = 0;
	PlayerTypes ePlayer = m_pPlayer->GetID();

	// If UN Victory isn't even available then don't bother with anything
	VictoryTypes eVictory = (VictoryTypes) GC.getInfoTypeForString("VICTORY_DIPLOMATIC", true);
	if(eVictory == NO_VICTORY || !GC.getGame().isVictoryValid(eVictory))
	{
		return -100;
	}

	// If we're close to winning, let's go to the finish line!
	if (GetPlayer()->GetDiplomacyAI()->IsCloseToDiploVictory())
	{
		iPriority += 7000;
	}

	int iPriorityBonus = 0;
	//Add in our base gold value.
	iPriorityBonus += (m_pPlayer->GetTreasury()->CalculateBaseNetGold() / 25);

	if (MOD_BALANCE_VP)
	{
		ResourceTypes ePaper = (ResourceTypes)GC.getInfoTypeForString("RESOURCE_PAPER", true);
		if (ePaper != NO_RESOURCE && m_pPlayer->getResourceFromCSAlliances(ePaper) > 0)
		{
			iPriorityBonus += m_pPlayer->getResourceFromCSAlliances(ePaper) / 5;
		}
	}

	//Add priority value based on flavors of policies we've acquired.
	for(int iPolicyLoop = 0; iPolicyLoop < GC.getNumPolicyInfos(); iPolicyLoop++)
	{
		const PolicyTypes ePolicy = static_cast<PolicyTypes>(iPolicyLoop);
		CvPolicyEntry* pkPolicyInfo = GC.getPolicyInfo(ePolicy);
		if(pkPolicyInfo)
		{
			if(GetPlayer()->GetPlayerPolicies()->HasPolicy(ePolicy) && !GetPlayer()->GetPlayerPolicies()->IsPolicyBlocked(ePolicy))
			{
				for(int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
				{
					if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_GOLD")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_DIPLOMACY")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop) * 3;
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_PRODUCTION")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_RELIGION")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
				}
			}
		}
	}
	//Look for Buildings and grab flavors.
	int iLoop = 0;
	for (CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop)) 
	{
		int iNumBuildingInfos = GC.getNumBuildingInfos();
		for(int iI = 0; iI < iNumBuildingInfos; iI++)
		{
			const BuildingTypes eBuildingLoop = static_cast<BuildingTypes>(iI);
			if(eBuildingLoop == NO_BUILDING)
				continue;
			if(pLoopCity->GetCityBuildings()->GetNumBuilding(eBuildingLoop) > 0)
			{
				CvBuildingEntry* pkLoopBuilding = GC.getBuildingInfo(eBuildingLoop);
				if(pkLoopBuilding)
				{
					for(int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
					{
						if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_GOLD")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_DIPLOMACY")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop) * 2;
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_PRODUCTION")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_RELIGION")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
					}
				}
			}
		}
	}
	/////////
	//RELIGION CHECKS
	////////////
	ReligionTypes eReligion = m_pPlayer->GetReligions()->GetStateReligion();
	if (eReligion != NO_RELIGION)
	{
		const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eReligion, m_pPlayer->GetID());
		if (pReligion)
		{
			CvCity* pHolyCity = pReligion->GetHolyCity();
			CvBeliefXMLEntries* pkBeliefs = GC.GetGameBeliefs();
			const int iNumBeliefs = pkBeliefs->GetNumBeliefs();
			for (int iI = 0; iI < iNumBeliefs; iI++)
			{
				const BeliefTypes eBelief(static_cast<BeliefTypes>(iI));
				CvBeliefEntry* pEntry = pkBeliefs->GetEntry(eBelief);
				if (pEntry && pReligion->m_Beliefs.HasBelief(eBelief) && pReligion->m_Beliefs.IsBeliefValid(eBelief, eReligion, m_pPlayer->GetID(), pHolyCity))
				{
					if(pEntry->GetExtraVotes())
					{
						iPriorityBonus += pEntry->GetExtraVotes();
					}
					if(pEntry->GetCityStateInfluenceModifier() > 0)
					{
						iPriorityBonus += pEntry->GetCityStateInfluenceModifier();
					}
					if(pEntry->GetCityStateFriendshipModifier() > 0)
					{
						iPriorityBonus += pEntry->GetCityStateFriendshipModifier();
					}
					if(pEntry->GetCityStateMinimumInfluence() > 0)
					{
						iPriorityBonus += pEntry->GetCityStateMinimumInfluence();
					}
					if(pEntry->GetMissionaryInfluenceCS() > 0)
					{
						iPriorityBonus += pEntry->GetMissionaryInfluenceCS();
					}
					for(int iI = 0; iI < NUM_YIELD_TYPES; iI++)
					{
						const YieldTypes eYield = static_cast<YieldTypes>(iI);
						if(eYield != NO_YIELD)
						{
							if(pEntry->GetYieldFromHost(eYield) > 0)
							{
								iPriorityBonus += pEntry->GetYieldFromHost(eYield);
							}
							if(pEntry->GetYieldFromProposal(eYield) > 0)
							{
								iPriorityBonus += pEntry->GetYieldFromProposal(eYield);
							}
						}
					}
				}
			}
		}
	}
	iPriorityBonus /= 10;

	if (m_pPlayer->GetPlayerTraits()->IsWarmonger())
	{
		iPriorityBonus -= 100;
	}
	if (m_pPlayer->GetPlayerTraits()->IsSmaller())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsTourism())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsDiplomat())
	{
		iPriorityBonus += 100;
	}
	if (m_pPlayer->GetPlayerTraits()->IsNerd())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsReligious())
	{
		iPriorityBonus += 50;
	}
	if (m_pPlayer->GetPlayerTraits()->IsExpansionist())
	{
		iPriorityBonus += 50;
	}

	iPriority += iPriorityBonus;

	int iVotesNeededToWin = GC.getGame().GetVotesNeededForDiploVictory();

	int iVotesControlled = 0;
	int iVotesControlledDelta = 0;
	int iUnalliedCityStates = 0;
	if (GC.getGame().GetGameLeagues()->GetNumActiveLeagues() == 0)
	{
		// Before leagues kick in, add weight based on flavor
		int iFlavorDiplo =  m_pPlayer->GetFlavorManager()->GetPersonalityIndividualFlavor((FlavorTypes)GC.getInfoTypeForString("FLAVOR_DIPLOMACY"));
		iPriority += (10 - m_pPlayer->GetCurrentEra()) * iFlavorDiplo * 125 / 100;
	}
	else
	{
		CvLeague* pLeague = GC.getGame().GetGameLeagues()->GetActiveLeague();
		ASSERT(pLeague != NULL);
		if (pLeague != NULL)
		{
			// Votes we control
			iVotesControlled += pLeague->CalculateStartingVotesForMember(ePlayer);

			// Votes other players control
			int iHighestOtherPlayerVotes = 0;
			for (int iPlayerLoop = 0; iPlayerLoop < MAX_CIV_PLAYERS; iPlayerLoop++)
			{
				PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;

				if(eLoopPlayer != ePlayer && GET_PLAYER(eLoopPlayer).isAlive())
				{
					if (GET_PLAYER(eLoopPlayer).isMinorCiv())
					{
						if (GET_PLAYER(eLoopPlayer).GetMinorCivAI()->GetAlly() == NO_PLAYER)
						{
							iUnalliedCityStates++;
						}
					}
					else
					{
						int iOtherPlayerVotes = pLeague->CalculateStartingVotesForMember(eLoopPlayer);
						if (iOtherPlayerVotes > iHighestOtherPlayerVotes)
						{
							iHighestOtherPlayerVotes = iOtherPlayerVotes;
						}
					}
				}
			}

			// How we compare
			iVotesControlledDelta = iVotesControlled - iHighestOtherPlayerVotes;
		}
	}

	// Are we close to winning?
	if (iVotesControlled >= iVotesNeededToWin)
	{
		iPriority *= 15;
	}
	else if (iVotesControlled >= ((iVotesNeededToWin * 3) / 4))
	{
		iPriority *= 10;
	}
	else if (iVotesControlled >= ((iVotesNeededToWin * 2) / 4))
	{
		iPriority *= 5;
	}
	// We have the most votes
	if (iVotesControlledDelta > 0)
	{
		iPriority += MAX(100, iVotesControlledDelta * 20);
	}
	// We are equal or behind in votes
	else
	{
		// Could we make up the difference with currently unallied city-states?
		int iPotentialCityStateVotes = iUnalliedCityStates * 2;
		int iPotentialVotesDelta = iPotentialCityStateVotes + iVotesControlledDelta;
		if (iPotentialVotesDelta > 0)
		{
			iPriority += MAX(20, iPotentialVotesDelta * 5);
		}
		else if (iPotentialVotesDelta < 0)
		{
			iPriority += MIN(-100, iPotentialVotesDelta * -50);
		}
	}

	// factor in some traits that could be useful (or harmful)
	iPriority += (m_pPlayer->GetPlayerTraits()->GetCityStateFriendshipModifier() * 2);
	iPriority += (m_pPlayer->GetPlayerTraits()->GetCityStateBonusModifier() * 2);
	iPriority -= (m_pPlayer->GetPlayerTraits()->GetCityStateCombatModifier() * 2);
	iPriority -= (m_pPlayer->GetCityStateCombatModifier());
	iPriority += (m_pPlayer->GetPlayerTraits()->GetAllianceCSDefense() / 2);
	iPriority += (m_pPlayer->GetPlayerTraits()->GetAllianceCSStrength() / 2);
	for(int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		const YieldTypes eYield = static_cast<YieldTypes>(iI);
		if(eYield != NO_YIELD)
		{
			iPriority += m_pPlayer->GetPlayerTraits()->GetYieldFromCSAlly(eYield);
			iPriority += m_pPlayer->GetPlayerTraits()->GetYieldFromCSFriend(eYield);
		}
	}

	return iPriority;
}

/// Returns Priority for Spaceship Grand Strategy
int CvGrandStrategyAI::GetSpaceshipPriority()
{
	int iPriority = 0;

	// If SS Victory isn't even available then don't bother with anything
	VictoryTypes eVictory = (VictoryTypes) GC.getInfoTypeForString("VICTORY_SPACE_RACE", true);
	if(eVictory == NO_VICTORY || !GC.getGame().isVictoryValid(eVictory))
	{
		return -100;
	}

	// If we're close to winning, let's go to the finish line!
	if (GetPlayer()->GetDiplomacyAI()->IsCloseToSpaceshipVictory())
	{
		iPriority += 10000;
	}

	int iFlavorScience =  m_pPlayer->GetFlavorManager()->GetPersonalityIndividualFlavor((FlavorTypes)GC.getInfoTypeForString("FLAVOR_SCIENCE"));

	// the later the game the greater the chance
	iPriority += ((m_pPlayer->GetCurrentEra() * m_pPlayer->GetCurrentEra()) * max(1, iFlavorScience) * 300) / 100;

	int iNumCivsAheadScience = 0;
	int iNumCivsBehindScience = 0;
	int iNumCivsAlive = 0;

	for (int iPlayerLoop = 0; iPlayerLoop < MAX_CIV_PLAYERS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;
		CvPlayer &kPlayer = GET_PLAYER(eLoopPlayer);

		if (kPlayer.isAlive() && !kPlayer.isMinorCiv() && !kPlayer.isBarbarian() && iPlayerLoop != m_pPlayer->GetID())
		{
			if (GET_TEAM(GetPlayer()->getTeam()).GetTeamTechs()->GetNumTechsKnown() > GET_TEAM(kPlayer.getTeam()).GetTeamTechs()->GetNumTechsKnown())
			{
				iNumCivsAheadScience++;
			}
			else
			{
				iNumCivsBehindScience++;
			}
			iNumCivsAlive++;
		}
	}
	if (iNumCivsAlive > 0 && iNumCivsAheadScience > iNumCivsBehindScience)
	{
		iPriority += /*50*/ GD_INT_GET(AI_GS_CULTURE_AHEAD_WEIGHT) * (iNumCivsAheadScience - iNumCivsBehindScience) / iNumCivsAlive;
	}

	//Are we in first overall? Bump it!
	if (iNumCivsAheadScience == iNumCivsAlive)
	{
		iPriority += /*50*/ GD_INT_GET(AI_GS_CULTURE_AHEAD_WEIGHT) * iNumCivsAlive;
	}
	// if I already built the Apollo Program I am very likely to follow through
	ProjectTypes eApolloProgram = (ProjectTypes) GC.getInfoTypeForString("PROJECT_APOLLO_PROGRAM", true);
	if(eApolloProgram != NO_PROJECT)
	{
		if(GET_TEAM(m_pPlayer->getTeam()).getProjectCount(eApolloProgram) > 0)
		{
			iPriority += /*150*/ GD_INT_GET(AI_GS_SS_HAS_APOLLO_PROGRAM);
		}
	}

	int iPriorityBonus = 0;
	//Add in our base science value.
	iPriorityBonus += (m_pPlayer->GetScience() / 25);

	//Add priority value based on flavors of policies we've acquired.
	for(int iPolicyLoop = 0; iPolicyLoop < GC.getNumPolicyInfos(); iPolicyLoop++)
	{
		const PolicyTypes ePolicy = static_cast<PolicyTypes>(iPolicyLoop);
		CvPolicyEntry* pkPolicyInfo = GC.getPolicyInfo(ePolicy);
		if(pkPolicyInfo)
		{
			if(GetPlayer()->GetPlayerPolicies()->HasPolicy(ePolicy) && !GetPlayer()->GetPlayerPolicies()->IsPolicyBlocked(ePolicy))
			{
				for(int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
				{
					if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_SPACESHIP")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_SCIENCE")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_GROWTH")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
					else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_TILE_IMPROVEMENT")
					{
						iPriorityBonus += pkPolicyInfo->GetFlavorValue(iFlavorLoop);
					}
				}
			}
		}
	}
	//Look for Buildings and grab flavors.
	int iLoop = 0;
	for (CvCity* pLoopCity = m_pPlayer->firstCity(&iLoop); pLoopCity != NULL; pLoopCity = m_pPlayer->nextCity(&iLoop)) 
	{
		int iNumBuildingInfos = GC.getNumBuildingInfos();
		for(int iI = 0; iI < iNumBuildingInfos; iI++)
		{
			const BuildingTypes eBuildingLoop = static_cast<BuildingTypes>(iI);
			if(eBuildingLoop == NO_BUILDING)
				continue;
			if(pLoopCity->GetCityBuildings()->GetNumBuilding(eBuildingLoop) > 0)
			{
				CvBuildingEntry* pkLoopBuilding = GC.getBuildingInfo(eBuildingLoop);
				if(pkLoopBuilding)
				{
					for(int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
					{
						if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_SPACESHIP")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_SCIENCE")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if(GC.getFlavorTypes((FlavorTypes) iFlavorLoop) == "FLAVOR_GROWTH")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
						else if (GC.getFlavorTypes((FlavorTypes)iFlavorLoop) == "FLAVOR_TILE_IMPROVEMENT")
						{
							iPriorityBonus += pkLoopBuilding->GetFlavorValue(iFlavorLoop);
						}
					}
				}
			}
		}
	}
	/////////
	//RELIGION CHECKS
	////////////
	ReligionTypes eReligion = m_pPlayer->GetReligions()->GetStateReligion();
	if (eReligion != NO_RELIGION)
	{
		const CvReligion* pReligion = GC.getGame().GetGameReligions()->GetReligion(eReligion, m_pPlayer->GetID());
		if (pReligion)
		{
			CvCity* pHolyCity = pReligion->GetHolyCity();
			CvBeliefXMLEntries* pkBeliefs = GC.GetGameBeliefs();
			const int iNumBeliefs = pkBeliefs->GetNumBeliefs();
			for(int iI = 0; iI < iNumBeliefs; iI++)
			{
				const BeliefTypes eBelief(static_cast<BeliefTypes>(iI));
				CvBeliefEntry* pEntry = pkBeliefs->GetEntry(eBelief);
				if (pEntry && pReligion->m_Beliefs.HasBelief(eBelief) && pReligion->m_Beliefs.IsBeliefValid(eBelief, eReligion, m_pPlayer->GetID(), pHolyCity))
				{
					if(pEntry->GetSciencePerOtherReligionFollower() > 0)
					{
						iPriorityBonus += pEntry->GetSciencePerOtherReligionFollower() * 10;
					}
					
					for(int iI = 0; iI < NUM_YIELD_TYPES; iI++)
					{
						const YieldTypes eYield = static_cast<YieldTypes>(iI);
						if(eYield != NO_YIELD)
						{
							if(pEntry->GetYieldFromEraUnlock(eYield) > 0)
							{
								iPriorityBonus += pEntry->GetYieldFromEraUnlock(eYield);
							}
							if(pEntry->GetYieldPerScience(eYield) > 0)
							{
								iPriorityBonus += pEntry->GetYieldPerScience(eYield);
							}
						}
					}
				}
			}
		}
	}
	iPriorityBonus /= 10;

	if (m_pPlayer->GetPlayerTraits()->IsWarmonger())
	{
		iPriorityBonus -= 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsSmaller())
	{
		iPriorityBonus += 50;
	}
	if (m_pPlayer->GetPlayerTraits()->IsTourism())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsDiplomat())
	{
		iPriorityBonus += 25;
	}
	if (m_pPlayer->GetPlayerTraits()->IsNerd())
	{
		iPriorityBonus += 100;
	}
	if (m_pPlayer->GetPlayerTraits()->IsReligious())
	{
		iPriorityBonus += 50;
	}
	if (m_pPlayer->GetPlayerTraits()->IsExpansionist())
	{
		iPriorityBonus -= 50;
	}

	iPriority += iPriorityBonus;

	return iPriority;
}

/// Get the base Priority for a Grand Strategy; these are elements common to ALL Grand Strategies
int CvGrandStrategyAI::GetBaseGrandStrategyPriority(AIGrandStrategyTypes eGrandStrategy)
{
	CvAIGrandStrategyXMLEntry* pGrandStrategy = GetAIGrandStrategies()->GetEntry(eGrandStrategy);

	int iPriority = 0;

	// Personality effect on Priority
	for(int iFlavorLoop = 0; iFlavorLoop < GC.getNumFlavorTypes(); iFlavorLoop++)
	{
		if(pGrandStrategy->GetFlavorValue(iFlavorLoop) != 0)
		{
			iPriority += (pGrandStrategy->GetFlavorValue(iFlavorLoop) * GetPlayer()->GetFlavorManager()->GetPersonalityIndividualFlavor((FlavorTypes) iFlavorLoop));
		}
	}

	return iPriority;
}

/// Get the base Priority for a Grand Strategy; these are elements common to ALL Grand Strategies
int CvGrandStrategyAI::GetPersonalityAndGrandStrategy(FlavorTypes eFlavorType, bool bBoostGSMainFlavor)
{
	if (m_eActiveGrandStrategy != NO_AIGRANDSTRATEGY)
	{
		CvAIGrandStrategyXMLEntry* pGrandStrategy = GetAIGrandStrategies()->GetEntry(m_eActiveGrandStrategy);
		int iModdedFlavor = pGrandStrategy->GetFlavorModValue(eFlavorType) + m_pPlayer->GetFlavorManager()->GetPersonalityIndividualFlavor(eFlavorType);
		iModdedFlavor = max(0,iModdedFlavor);

		if (bBoostGSMainFlavor && (pGrandStrategy->GetFlavorValue(eFlavorType) > 0))
		{
			iModdedFlavor = min(10, ((pGrandStrategy->GetFlavorValue(eFlavorType) + iModdedFlavor + 1) / 2));
		}

		return iModdedFlavor;
	}
	return m_pPlayer->GetFlavorManager()->GetPersonalityIndividualFlavor(eFlavorType);
}

//	--------------------------------------------------------------------------------
// AI_AB_TEST (Personality): the strategy profile (pack repo tools/ab/RESULTS.md, "The strategy profile: spec"). Step 1:
// worked out and logged, nothing reads it yet. A pure function of game data fixed at game start (traits, leader
// personality, CP's victory pursuit pick, which is saved with the game), so it needs no saved state of its own.

static int GetStrategyProfileFlavor(CvPlayer* pPlayer, const char* szFlavor)
{
	FlavorTypes eFlavor = (FlavorTypes)GC.getInfoTypeForString(szFlavor, true);
	return (eFlavor == NO_FLAVOR) ? 5 : pPlayer->GetFlavorManager()->GetPersonalityIndividualFlavor(eFlavor);
}

static StrategyArchetypeTypes GetStrategyArchetypeForPursuit(VictoryPursuitTypes ePursuit)
{
	switch (ePursuit)
	{
	case VICTORY_PURSUIT_DOMINATION:
		return STRATEGY_ARCHETYPE_CONQUEROR;
	case VICTORY_PURSUIT_SCIENCE:
		return STRATEGY_ARCHETYPE_SCIENTIST;
	case VICTORY_PURSUIT_CULTURE:
		return STRATEGY_ARCHETYPE_TOURISM;
	case VICTORY_PURSUIT_DIPLOMACY:
		return STRATEGY_ARCHETYPE_DIPLOMAT;
	default:
		return NO_STRATEGY_ARCHETYPE;
	}
}

static StrategyVictoryWeightTypes GetStrategyVictoryForPursuit(VictoryPursuitTypes ePursuit)
{
	switch (ePursuit)
	{
	case VICTORY_PURSUIT_SCIENCE:
		return STRATEGY_VICTORY_SCIENCE;
	case VICTORY_PURSUIT_CULTURE:
		return STRATEGY_VICTORY_CULTURE;
	case VICTORY_PURSUIT_DIPLOMACY:
		return STRATEGY_VICTORY_DIPLOMACY;
	default:
		return STRATEGY_VICTORY_DOMINATION;
	}
}

const char* CvGrandStrategyAI::GetStrategyArchetypeName(StrategyArchetypeTypes eArchetype)
{
	switch (eArchetype)
	{
	case STRATEGY_ARCHETYPE_CONQUEROR:
		return "Conqueror";
	case STRATEGY_ARCHETYPE_OPPORTUNIST:
		return "Opportunist";
	case STRATEGY_ARCHETYPE_EXPANSIONIST:
		return "Expansionist";
	case STRATEGY_ARCHETYPE_ZEALOT:
		return "Zealot";
	case STRATEGY_ARCHETYPE_DIPLOMAT:
		return "Diplomat";
	case STRATEGY_ARCHETYPE_TOURISM:
		return "Tourism";
	case STRATEGY_ARCHETYPE_SCIENTIST:
		return "Scientist";
	case STRATEGY_ARCHETYPE_RAIDER:
		return "Raider";
	default:
		return "None";
	}
}

CvStrategyProfile CvGrandStrategyAI::GetStrategyProfile() const
{
	// per archetype: home meta position and its range; victory weights (domination, science, culture, diplomacy)
	static const int aiHome[NUM_STRATEGY_ARCHETYPES] = { 70, 15, 0, 0, -20, -60, -60, 60 };
	static const int aiMin[NUM_STRATEGY_ARCHETYPES] = { 40, -10, -20, -30, -50, -90, -90, 40 };
	static const int aiMax[NUM_STRATEGY_ARCHETYPES] = { 100, 40, 30, 30, 10, -30, -30, 80 };
	static const int aaiWeights[NUM_STRATEGY_ARCHETYPES][NUM_STRATEGY_VICTORY_WEIGHTS] = {
		{ 70, 10, 10, 10 },	// Conqueror
		{ 40, 20, 20, 20 },	// Opportunist
		{ 25, 25, 25, 25 },	// Expansionist
		{ 15, 15, 50, 20 },	// Zealot
		{ 10, 15, 15, 60 },	// Diplomat
		{ 5, 15, 70, 10 },	// Tourism
		{ 5, 70, 10, 15 },	// Scientist
		{ 40, 20, 20, 20 },	// Raider: wars pay for its economy, but it doesn't conquer its way to a win
	};

	CvStrategyProfile kProfile;
	memset(&kProfile, 0, sizeof(kProfile));	// VC9 warns (as an error) that the victory weights may be read uninitialized
	kProfile.eArchetype = NO_STRATEGY_ARCHETYPE;
	kProfile.iHomeMeta = 0;
	for (int i = 0; i < NUM_STRATEGY_VICTORY_WEIGHTS; i++)
		kProfile.aiVictoryWeight[i] = 25;
	kProfile.eRisk = STRATEGY_RISK_MEDIUM;
	kProfile.eExpansion = STRATEGY_EXPANSION_MEDIUM;
	kProfile.eGreedyKind = STRATEGY_GREEDY_TALL;
	kProfile.eSpikeUnit = NO_UNIT;
	kProfile.eSpikeTech = NO_TECH;
	kProfile.iSpikeTurn = -1;
	kProfile.bSpikeTerrain = false;
	kProfile.bMilitaryFirst = false;
	kProfile.iStrikeStart = -1;
	kProfile.iStrikeEnd = -1;
	kProfile.iStrikeDeclareBy = -1;
	kProfile.iCoreSharePercent = 0;
	kProfile.eUpgradeTech = NO_TECH;
	kProfile.iUpgradeTurn = -1;
	if (!m_pPlayer || !m_pPlayer->isMajorCiv())
		return kProfile;

	CvPlayerTraits* pTraits = m_pPlayer->GetPlayerTraits();
	CvDiplomacyAI* pDiplo = m_pPlayer->GetDiplomacyAI();
	const VictoryPursuitTypes ePrimary = pDiplo->GetPrimaryVictoryPursuit();
	const VictoryPursuitTypes eSecondary = pDiplo->GetSecondaryVictoryPursuit();
	const int iOffense = GetStrategyProfileFlavor(m_pPlayer, "FLAVOR_OFFENSE");
	const int iExpansion = GetStrategyProfileFlavor(m_pPlayer, "FLAVOR_EXPANSION");
	const int iBoldness = pDiplo->GetBoldness();

	// 1. the unique ability (CP's trait tags), in this order; the primary pursuit breaks a tie between two tags
	vector<StrategyArchetypeTypes> veTagged;
	if (pTraits->IsWarmonger())
		veTagged.push_back(STRATEGY_ARCHETYPE_CONQUEROR);
	if (pTraits->IsReligious())
		veTagged.push_back(STRATEGY_ARCHETYPE_ZEALOT);
	if (pTraits->IsTourism())
		veTagged.push_back(STRATEGY_ARCHETYPE_TOURISM);
	if (pTraits->IsNerd() || pTraits->IsSmaller())
		veTagged.push_back(STRATEGY_ARCHETYPE_SCIENTIST);
	if (pTraits->IsDiplomat())
		veTagged.push_back(STRATEGY_ARCHETYPE_DIPLOMAT);
	if (pTraits->IsExpansionist())
		veTagged.push_back(STRATEGY_ARCHETYPE_EXPANSIONIST);
	const StrategyArchetypeTypes ePursuitArchetype = GetStrategyArchetypeForPursuit(ePrimary);

	// 0. bonuses for taking cities, or for taking city-states without a war, come before CP's broad tags (the user,
	// 2026-11-04: use each civ's conquest bonus). CP's tags overlap (Assyria's tech from captured cities makes it both a
	// warmonger and a science civ; Venice's "no annexing" makes it a smaller civ), and CP scores the victory pursuit that
	// breaks the tie from flavours, so a conquest civ could come out a Scientist. Trait fields, not a civ list, so mods
	// and other civs work too.
	bool bConquestBonus = pTraits->IsTechFromCityConquer() || pTraits->GetPlunderModifier() > 0 ||
		pTraits->GetCityStateCombatModifier() > 0 || pTraits->GetProductionBonusModifierConquest() > 0 || pTraits->GetCityConquestGWAM() > 0 ||
		pTraits->IsKeepConqueredBuildings() || pTraits->GetGoldenAgeFromVictory() > 0;
	for (int iYield = 0; iYield < NUM_YIELD_TYPES && !bConquestBonus; iYield++)
	{
		if (pTraits->GetYieldFromConquest((YieldTypes)iYield) > 0)
			bConquestBonus = true;
		for (int iTerrain = 0; iTerrain < GC.getNumTerrainInfos() && !bConquestBonus; iTerrain++)
		{
			if (pTraits->GetYieldChangeFromTileConquest((TerrainTypes)iTerrain, (YieldTypes)iYield) > 0)
				bConquestBonus = true;
		}
	}
	// raiders (the user, 2026-11-04: "raider is a great flavour archetype"): rewarded for pillaging, plundering or razing,
	// not for holding cities. Faster razing (the Huns), plundering trade routes without a war, or a free promotion that
	// rewards pillage or attacking cities (free pillage moves, strength or healing from pillage, plunder from city
	// attacks: Denmark's Viking promotion). A conquest bonus wins over this: Songhai's plunder gold comes from taking
	// cities, so it stays a Conqueror (the user).
	bool bRaider = pTraits->GetRazeSpeedModifier() > 0 || pTraits->IsCanPlunderWithoutWar();
	for (int iPromotion = 0; iPromotion < GC.getNumPromotionInfos() && !bRaider; iPromotion++)
	{
		CvPromotionEntry* pkPromotion = GC.getPromotionInfo((PromotionTypes)iPromotion);
		if (!pkPromotion || !(pkPromotion->IsFreePillageMoves() || pkPromotion->IsHealOnPillage() ||
			pkPromotion->GetPillageBonusStrengthPercent() > 0 || pkPromotion->GetCityAttackPlunderModifier() > 0))
			continue;
		for (int iCombat = 0; iCombat < GC.getNumUnitCombatClassInfos() && !bRaider; iCombat++)
			bRaider = pTraits->HasFreePromotionUnitCombat(iPromotion, iCombat);
		for (int iClass = 0; iClass < GC.getNumUnitClassInfos() && !bRaider; iClass++)
			bRaider = pTraits->HasFreePromotionUnitClass(iPromotion, iClass);
	}

	// city-states without a war: diplomatic marriage (Austria), annexing city-states, or a unique unit that can buy one
	// (Venice's Merchant of Venice)
	bool bCityStateTakeover = pTraits->IsDiplomaticMarriage() || pTraits->IsAbleToAnnexCityStates();
	for (int iClass = 0; iClass < GC.getNumUnitClassInfos() && !bCityStateTakeover; iClass++)
	{
		const UnitTypes eUnit = (UnitTypes)m_pPlayer->getCivilizationInfo().getCivilizationUnits(iClass);
		CvUnitEntry* pkUnitInfo = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
		if (pkUnitInfo && pkUnitInfo->IsCanBuyCityState())
			bCityStateTakeover = true;
	}

	if (bConquestBonus)
		kProfile.eArchetype = STRATEGY_ARCHETYPE_CONQUEROR;
	else if (bRaider)
		kProfile.eArchetype = STRATEGY_ARCHETYPE_RAIDER;
	else if (bCityStateTakeover)
		kProfile.eArchetype = STRATEGY_ARCHETYPE_DIPLOMAT;
	else if (!veTagged.empty())
	{
		kProfile.eArchetype = veTagged[0];
		for (size_t i = 0; i < veTagged.size(); i++)
		{
			if (veTagged[i] == ePursuitArchetype)
				kProfile.eArchetype = veTagged[i];
		}
	}
	// 2. no tag: the primary victory pursuit picked for this game
	else if (ePrimary == VICTORY_PURSUIT_DOMINATION)
		kProfile.eArchetype = (iOffense >= 7) ? STRATEGY_ARCHETYPE_CONQUEROR : STRATEGY_ARCHETYPE_OPPORTUNIST;
	else if (ePursuitArchetype != NO_STRATEGY_ARCHETYPE)
		kProfile.eArchetype = ePursuitArchetype;
	else
		kProfile.eArchetype = (iExpansion >= 7) ? STRATEGY_ARCHETYPE_EXPANSIONIST : STRATEGY_ARCHETYPE_OPPORTUNIST;

	const int eArch = (int)kProfile.eArchetype;

	// 3. home meta position: the archetype's, moved by boldness and offence, kept inside its range
	kProfile.iHomeMeta = range(aiHome[eArch] + 5 * (iBoldness - 5) + 5 * (iOffense - 5), aiMin[eArch], aiMax[eArch]);
	// a civ rewarded for taking cities plays the conquest plan (MetaDial: from +50) whatever its boldness
	if (bConquestBonus)
		kProfile.iHomeMeta = max(50, kProfile.iHomeMeta);

	// victory weights: 40% the archetype's, 40% the primary pursuit, 20% the secondary (or the primary if none)
	for (int i = 0; i < NUM_STRATEGY_VICTORY_WEIGHTS; i++)
		kProfile.aiVictoryWeight[i] = aaiWeights[eArch][i] * 40 / 100;
	int iRest = 100;
	for (int i = 0; i < NUM_STRATEGY_VICTORY_WEIGHTS; i++)
		iRest -= kProfile.aiVictoryWeight[i];
	if (ePrimary != NO_VICTORY_PURSUIT)
	{
		const int iSecondaryShare = (eSecondary != NO_VICTORY_PURSUIT) ? iRest / 3 : 0;
		kProfile.aiVictoryWeight[GetStrategyVictoryForPursuit(ePrimary)] += iRest - iSecondaryShare;
		if (eSecondary != NO_VICTORY_PURSUIT)
			kProfile.aiVictoryWeight[GetStrategyVictoryForPursuit(eSecondary)] += iSecondaryShare;
	}
	else
	{
		// no pursuit: the rest follows the archetype too (rounding goes to its first weight)
		for (int i = 0; i < NUM_STRATEGY_VICTORY_WEIGHTS; i++)
			kProfile.aiVictoryWeight[i] += aaiWeights[eArch][i] * 60 / 100;
		int iSum = 0;
		for (int i = 0; i < NUM_STRATEGY_VICTORY_WEIGHTS; i++)
			iSum += kProfile.aiVictoryWeight[i];
		kProfile.aiVictoryWeight[0] += 100 - iSum;
	}

	// risk appetite from boldness
	kProfile.eRisk = (iBoldness >= 7) ? STRATEGY_RISK_HIGH : (iBoldness <= 3) ? STRATEGY_RISK_LOW : STRATEGY_RISK_MEDIUM;

	// expansion style: the archetype's, one step wider or narrower with the expansion flavour (conquest stays)
	switch (kProfile.eArchetype)
	{
	case STRATEGY_ARCHETYPE_CONQUEROR:
		kProfile.eExpansion = STRATEGY_EXPANSION_CONQUEST;
		break;
	case STRATEGY_ARCHETYPE_EXPANSIONIST:
		kProfile.eExpansion = STRATEGY_EXPANSION_WIDE;
		break;
	case STRATEGY_ARCHETYPE_TOURISM:
	case STRATEGY_ARCHETYPE_SCIENTIST:
		kProfile.eExpansion = STRATEGY_EXPANSION_TALL;
		break;
	default:
		kProfile.eExpansion = STRATEGY_EXPANSION_MEDIUM;
		break;
	}
	if (kProfile.eExpansion != STRATEGY_EXPANSION_CONQUEST)
	{
		if (iExpansion >= 8 && kProfile.eExpansion != STRATEGY_EXPANSION_WIDE)
			kProfile.eExpansion = (StrategyExpansionTypes)(kProfile.eExpansion + 1);
		else if (iExpansion <= 3 && kProfile.eExpansion != STRATEGY_EXPANSION_TALL)
			kProfile.eExpansion = (StrategyExpansionTypes)(kProfile.eExpansion - 1);
	}

	// GreedyWide/GreedyTall (the user, 2026-11-04): which kind of greedy the leader plays. The expansionist tag is wide,
	// the small-empire and science tags tall; otherwise the expansion flavour against the larger of growth and science,
	// and a tie by the expansion style above (wide or conquest is wide)
	if (pTraits->IsExpansionist())
		kProfile.eGreedyKind = STRATEGY_GREEDY_WIDE;
	else if (pTraits->IsSmaller() || pTraits->IsNerd())
		kProfile.eGreedyKind = STRATEGY_GREEDY_TALL;
	else
	{
		const int iGrowthOrScience = max(GetStrategyProfileFlavor(m_pPlayer, "FLAVOR_GROWTH"), GetStrategyProfileFlavor(m_pPlayer, "FLAVOR_SCIENCE"));
		if (iExpansion != iGrowthOrScience)
			kProfile.eGreedyKind = (iExpansion > iGrowthOrScience) ? STRATEGY_GREEDY_WIDE : STRATEGY_GREEDY_TALL;
		else
			kProfile.eGreedyKind = (kProfile.eExpansion >= STRATEGY_EXPANSION_WIDE) ? STRATEGY_GREEDY_WIDE : STRATEGY_GREEDY_TALL;
	}

	GetPowerSpike(kProfile);
	return kProfile;
}

/// AI_AB_TEST (Personality; the user, 2026-11-06, after taking Marrakesh at T54 with the Aztec's Jaguars: "every civ needs
/// to build to its strengths"). The military power spike: the civ's earliest unique land combat unit (game data, so mods
/// and other civs work too; scouts don't count), the tech that unlocks it, and the turn that tech usually comes (Standard
/// turns by its column in the tech tree, roughly the new AI's pace in logged games: warmongers' Mathematics, column 3, at
/// about T48, their Physics at T138; scaled to the game's speed). The opening that serves it: military first and a small
/// empire when the spike comes by turn 30 (Standard), otherwise expansion first. The strike window, when to start the
/// war: opens once the army can be built (T60 Standard at the earliest, and 20 turns after the spike), declare within 15
/// turns, closes 30 turns after opening (sooner when the unit goes obsolete).
void CvGrandStrategyAI::GetPowerSpike(CvStrategyProfile& kProfile) const
{
	static const int aiTurnByColumn[] = { 0, 15, 30, 47, 75, 105, 135, 160, 185, 210, 235, 260, 285, 310, 335, 360, 385, 410, 435, 460 };
	static const int iNumColumns = (int)(sizeof(aiTurnByColumn) / sizeof(aiTurnByColumn[0]));
	const int iSpeed = max(1, GC.getGame().getGameSpeedInfo().getTrainPercent());
	const CvCivilizationInfo& kCiv = m_pPlayer->getCivilizationInfo();

	int iBestTurn = MAX_INT;
	for (int iClass = 0; iClass < GC.getNumUnitClassInfos(); iClass++)
	{
		if (!kCiv.isCivilizationUnitOverridden(iClass))
			continue;
		const UnitTypes eUnit = (UnitTypes)kCiv.getCivilizationUnits(iClass);
		CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
		if (!pkUnit || pkUnit->GetDomainType() != DOMAIN_LAND || (pkUnit->GetCombat() <= 0 && pkUnit->GetRangedCombat() <= 0) ||
			pkUnit->GetDefaultUnitAIType() == UNITAI_EXPLORE)
			continue;
		const TechTypes eTech = (TechTypes)pkUnit->GetPrereqAndTech();
		CvTechEntry* pkTech = (eTech != NO_TECH) ? GC.getTechInfo(eTech) : NULL;
		const int iColumn = pkTech ? max(0, pkTech->GetGridX()) : 0;
		const int iTurn = (iColumn < iNumColumns) ? aiTurnByColumn[iColumn] : aiTurnByColumn[iNumColumns - 1] + 20 * (iColumn - iNumColumns + 1);
		if (iTurn >= iBestTurn)
			continue;
		iBestTurn = iTurn;
		kProfile.eSpikeUnit = eUnit;
		kProfile.eSpikeTech = pkTech ? eTech : NO_TECH;
		kProfile.iSpikeTurn = iTurn * iSpeed / 100;

		// fights better in forest or jungle (the Jaguar, the Mohawk Warrior)
		kProfile.bSpikeTerrain = false;
		for (int iPromotion = 0; iPromotion < GC.getNumPromotionInfos() && !kProfile.bSpikeTerrain; iPromotion++)
		{
			CvPromotionEntry* pkPromotion = pkUnit->GetFreePromotions(iPromotion) ? GC.getPromotionInfo((PromotionTypes)iPromotion) : NULL;
			if (!pkPromotion)
				continue;
			for (int iFeature = 0; iFeature < GC.getNumFeatureInfos() && !kProfile.bSpikeTerrain; iFeature++)
				kProfile.bSpikeTerrain = pkPromotion->GetFeatureAttackPercent(iFeature) > 0 || pkPromotion->GetFeatureDefensePercent(iFeature) > 0;
		}

		// the strike window: when to start the war (the strike itself, declaration to capture, can run past it).
		// Calibrated to the user's Aztec game (Quick, Jaguars from the start): military 29 at T30, 45 at T40, 67 at T50,
		// gold spent on units, war declared between T41 and T50, Marrakesh taken at T54. In Standard turns: the army
		// can't be ready before about T60 (first cities, little production), nor sooner than 20 turns after the spike;
		// the window opens then, declare within 15 turns, and it closes 30 turns after opening. An obsolete unit can't be
		// built but the ones out still fight: close 20 turns after the obsoleting tech usually comes, never before the
		// declare-by turn + 5. The target's own walls and counters are War's to check per target (bSiegeOnlyIfWalled).
		const int iOpen = max(60, iTurn + 20);
		const int iDeclareBy = iOpen + 15;
		int iEnd = iOpen + 30;
		const TechTypes eObsolete = (TechTypes)pkUnit->GetObsoleteTech();
		CvTechEntry* pkObsolete = (eObsolete != NO_TECH) ? GC.getTechInfo(eObsolete) : NULL;
		if (pkObsolete && pkObsolete->GetGridX() >= 0 && pkObsolete->GetGridX() < iNumColumns)
			iEnd = min(iEnd, max(iDeclareBy + 5, aiTurnByColumn[pkObsolete->GetGridX()] + 20));
		kProfile.iStrikeStart = iOpen * iSpeed / 100;
		kProfile.iStrikeEnd = iEnd * iSpeed / 100;
		kProfile.iStrikeDeclareBy = iDeclareBy * iSpeed / 100;
		// half the attack army is the spike unit; a third when it's siege (the rest still has to fight)
		kProfile.iCoreSharePercent = (pkUnit->GetDefaultUnitAIType() == UNITAI_CITY_BOMBARD) ? 33 : 50;
	}
	kProfile.bMilitaryFirst = kProfile.iSpikeTurn >= 0 && iBestTurn <= 30;
	if (kProfile.iSpikeTurn < 0)
		return;

	// The upgrade the strike waits for (the user, 2026-11-06: "i beelined to construction", "i upgraded 3 archers to
	// composite archers. that was part of the timing attack. upgrade and go"). The army is the land combat units the civ
	// can have by the time the window opens; the upgrade is the earliest tech after the spike, by the declare-by turn,
	// that unlocks an upgrade of one of them. Ranged first on a tie: ranged units take cities without siege.
	const int iOpenStd = max(60, iBestTurn + 20);
	const int iDeclareByStd = iOpenStd + 15;
	int iBestUpgradeTurn = MAX_INT;
	bool bBestRanged = false;
	for (int iClass = 0; iClass < GC.getNumUnitClassInfos(); iClass++)
	{
		const UnitTypes eUnit = (UnitTypes)kCiv.getCivilizationUnits(iClass);
		CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
		if (!pkUnit || pkUnit->GetDomainType() != DOMAIN_LAND || (pkUnit->GetCombat() <= 0 && pkUnit->GetRangedCombat() <= 0) ||
			pkUnit->GetDefaultUnitAIType() == UNITAI_EXPLORE)
			continue;
		const TechTypes eUnitTech = (TechTypes)pkUnit->GetPrereqAndTech();
		CvTechEntry* pkUnitTech = (eUnitTech != NO_TECH) ? GC.getTechInfo(eUnitTech) : NULL;
		const int iUnitColumn = pkUnitTech ? max(0, pkUnitTech->GetGridX()) : 0;
		if (iUnitColumn >= iNumColumns || aiTurnByColumn[iUnitColumn] > iOpenStd)
			continue;	// not in the army by the time the window opens
		for (int iUpgradeClass = 0; iUpgradeClass < GC.getNumUnitClassInfos(); iUpgradeClass++)
		{
			if (!pkUnit->GetUpgradeUnitClass(iUpgradeClass))
				continue;
			const UnitTypes eUpgrade = (UnitTypes)kCiv.getCivilizationUnits(iUpgradeClass);
			CvUnitEntry* pkUpgrade = (eUpgrade != NO_UNIT) ? GC.getUnitInfo(eUpgrade) : NULL;
			const TechTypes eUpgradeTech = pkUpgrade ? (TechTypes)pkUpgrade->GetPrereqAndTech() : NO_TECH;
			CvTechEntry* pkUpgradeTech = (eUpgradeTech != NO_TECH) ? GC.getTechInfo(eUpgradeTech) : NULL;
			if (!pkUpgradeTech || pkUpgradeTech->GetGridX() < 0 || pkUpgradeTech->GetGridX() >= iNumColumns)
				continue;
			const int iUpgradeTurn = aiTurnByColumn[pkUpgradeTech->GetGridX()];
			if (iUpgradeTurn <= iBestTurn || iUpgradeTurn > iDeclareByStd)
				continue;
			const bool bRanged = pkUpgrade->GetRangedCombat() > 0;
			if (iUpgradeTurn < iBestUpgradeTurn || (iUpgradeTurn == iBestUpgradeTurn && bRanged && !bBestRanged))
			{
				iBestUpgradeTurn = iUpgradeTurn;
				bBestRanged = bRanged;
				kProfile.eUpgradeTech = eUpgradeTech;
				kProfile.iUpgradeTurn = iUpgradeTurn * iSpeed / 100;
			}
		}
	}
}

/// AI_AB_TEST (Personality, MetaDial): the meta position the dial reads, -100 greedy to +100 aggressive. The home
/// position for now (MetaAdapt will move it). Worked out once per turn: the profile needs string lookups, and the
/// dial is read in hot paths (every building and unit choice, every upgrade check).
int CvGrandStrategyAI::GetMetaPosition() const
{
	const int iTurn = GC.getGame().getGameTurn();
	if (m_iCachedMetaTurn != iTurn)
	{
		const CvStrategyProfile kProfile = GetStrategyProfile();
		m_iCachedMetaPosition = kProfile.iHomeMeta;
		m_iCachedGreedyKind = kProfile.eGreedyKind;
		m_iCachedStrikeStart = kProfile.iStrikeStart;
		m_iCachedStrikeEnd = kProfile.iStrikeEnd;
		m_iCachedSpikeTech = kProfile.eSpikeTech;
		m_iCachedStrikeDeclareBy = kProfile.iStrikeDeclareBy;
		m_iCachedCoreUnitClass = (kProfile.eSpikeUnit != NO_UNIT) ? GC.getUnitInfo(kProfile.eSpikeUnit)->GetUnitClassType() : NO_UNITCLASS;
		m_iCachedCoreSharePercent = kProfile.iCoreSharePercent;
		m_bCachedMilitaryFirst = kProfile.bMilitaryFirst;
		m_iCachedUpgradeTech = kProfile.eUpgradeTech;
		m_iCachedUpgradeTurn = kProfile.iUpgradeTurn;
		m_iCachedMetaTurn = iTurn;
	}
	return m_iCachedMetaPosition;
}

/// AI_AB_TEST (Personality, GreedyWide/GreedyTall): which kind of greedy this leader plays (the profile's, once per turn)
StrategyGreedyKindTypes CvGrandStrategyAI::GetGreedyKind() const
{
	GetMetaPosition();	// fills the cache
	return (StrategyGreedyKindTypes)m_iCachedGreedyKind;
}

/// AI_AB_TEST (Personality, power spike; for War's TimingBuildup): the strike window in game turns (elapsed, this game's
/// speed). False when the civ has no unique land combat unit.
bool CvGrandStrategyAI::GetStrikeWindow(int& iStartTurn, int& iEndTurn) const
{
	GetMetaPosition();	// fills the cache
	iStartTurn = m_iCachedStrikeStart;
	iEndTurn = m_iCachedStrikeEnd;
	return iStartTurn >= 0;
}

/// The whole strike window, for War's StrikeWindow/EarlyStrike (the fields War asked for, 2026-11-06). False when the civ
/// has no unique land combat unit (then the fields say so: no tech, no class, -1 turns).
bool CvGrandStrategyAI::GetStrikeWindow(CvStrikeWindow& kWindow) const
{
	GetMetaPosition();	// fills the cache
	kWindow.eSpikeTech = (TechTypes)m_iCachedSpikeTech;
	kWindow.eCoreUnitClass = (UnitClassTypes)m_iCachedCoreUnitClass;
	kWindow.iCoreSharePercent = m_iCachedCoreSharePercent;
	kWindow.iOpenTurn = m_iCachedStrikeStart;
	kWindow.iDeclareByTurn = m_iCachedStrikeDeclareBy;
	kWindow.iCloseTurn = m_iCachedStrikeEnd;
	// an early spike (military first: by T30 Standard, before catapults) takes unwalled cities without siege
	kWindow.bMilitaryFirst = m_bCachedMilitaryFirst;
	kWindow.bSiegeOnlyIfWalled = m_bCachedMilitaryFirst;
	kWindow.eUpgradeTech = (TechTypes)m_iCachedUpgradeTech;
	kWindow.iUpgradeTurn = m_iCachedUpgradeTurn;
	return kWindow.iOpenTurn >= 0;
}

/// AI_AB_TEST (Personality, EarlyStrike): the civ's strike plan, worked out once per turn (the profile needs string lookups)
const CvStrikePlan& CvGrandStrategyAI::GetStrikePlan() const
{
	const int iTurn = GC.getGame().getGameTurn();
	if (m_iCachedStrikePlanTurn != iTurn)
	{
		MakeStrikePlan(GetStrategyProfile(), m_kCachedStrikePlan);
		m_iCachedStrikePlanTurn = iTurn;
	}
	return m_kCachedStrikePlan;
}

/// The strike plan (the user, 2026-11-06: "use the experts ideas right away"; the lead's civfanatics research,
/// STRIKE_PLANS_RESEARCH.md in the pack repo, BNW Deity/Immortal threads at Standard pace). Expert rows for the five most
/// common early aggressors; a data default for every other civ with a unique land combat unit. Turns are Standard here,
/// scaled to the game speed at the end. The research's general rules: Archer to Composite Bowman "upgrade and go" at
/// Construction with 70-80 gold saved per Archer; at least 2 melee units to take a city; 2-3 cities before the strike.
void CvGrandStrategyAI::MakeStrikePlan(const CvStrategyProfile& kProfile, CvStrikePlan& kPlan) const
{
	kPlan.bValid = false;
	kPlan.bExpert = false;
	kPlan.eCoreClass = NO_UNITCLASS;
	kPlan.eSupportClass = NO_UNITCLASS;
	kPlan.eSiegeClass = NO_UNITCLASS;
	kPlan.iCoreTarget = 0;
	kPlan.iSupportTarget = 0;
	kPlan.iSiegeTarget = 0;
	kPlan.eUpgradeTech = NO_TECH;
	kPlan.iUpgradeGoldPerUnit = 75;
	kPlan.iCoreBeforeSettler = 0;
	kPlan.iMaxCitiesBeforeStrike = 0;
	kPlan.iOpenTurn = -1;
	kPlan.iDeclareByTurn = -1;
	kPlan.iFallbackTurn = -1;
	kPlan.eFirstPolicyBranch = NO_POLICY_BRANCH_TYPE;
	kPlan.bSiegeOnlyIfWalled = true;
	kPlan.bBarbarianHunt = false;
	kPlan.bLateStrike = false;
	kPlan.ePreBuildClass = NO_UNITCLASS;
	kPlan.iPreBuildTarget = 0;
	kPlan.iEconomyGoldTarget = 0;
	kPlan.iExitCityStrength = 0;
	kPlan.eExitCounterClass1 = NO_UNITCLASS;
	kPlan.eExitCounterClass2 = NO_UNITCLASS;
	kPlan.bExitRoughTarget = false;
	kPlan.iNumBuilds = 0;
	kPlan.iNumTechs = 0;
	kPlan.bListInferred = false;
	for (int iItem = 0; iItem < STRIKE_LIST_MAX; iItem++)
	{
		kPlan.aBuilds[iItem].bUnit = false;
		kPlan.aBuilds[iItem].iClass = -1;
		kPlan.aeTechs[iItem] = NO_TECH;
	}
	if (!m_pPlayer || !m_pPlayer->isMajorCiv() || kProfile.eSpikeUnit == NO_UNIT)
		return;

	struct ExpertRow
	{
		const char* szCiv;
		const char* szCore; int iCore;
		const char* szSupport; int iSupport;
		const char* szSiege; int iSiege;
		const char* szUpgradeTech;
		int iN, iK, iOpen, iDeclareBy, iFallback;	// Standard turns
		const char* szPolicy;
		const char* szPreBuild; int iPreBuild;		// late strike: built beforehand, upgrades into the core
		int iUpgradeGold;
		bool bLate, bBarbarians;
	};
	// Quick = Standard x0.67. Huns: army ready ~T30, capitals T40-50 (S4, S11); no upgrades at the strike.
	// Aztec: 2-3 Jaguars first, ~6 Jaguars + 3-4 Composite Bowmen; the user declared T41-50 Quick. Inca: no source shows
	// an Inca rush, so (the user, 2026-11-06) "they should just use catapults": Slingers and 2 melee with 2 Catapults
	// (Mathematics) whatever the walls. Greece: Composite Bowmen drop the walls, Hoplites take the
	// city, T50-65. Celts: 4-6 Pictish Warriors + 2-3 Archers or Composite Bowmen. Policy per the lead (the research:
	// Honor where kills pay and for the Huns, Liberty for a plain Composite Bowman rush).
	static const ExpertRow kRows[] = {
		{ "CIVILIZATION_HUNS",   "UNITCLASS_CHARIOT_ARCHER", 5, "UNITCLASS_WARRIOR",  1, "UNITCLASS_SPEARMAN", 2, NULL,                 2, 2, 30, 50, 55, "POLICY_BRANCH_HONOR", NULL, 0, 75, false, true },
		{ "CIVILIZATION_AZTEC",  "UNITCLASS_WARRIOR",        6, "UNITCLASS_ARCHER",   4, NULL,                 0, "TECH_CONSTRUCTION",  3, 2, 52, 70, 75, "POLICY_BRANCH_HONOR", NULL, 0, 75, false, true },
		{ "CIVILIZATION_INCA",   "UNITCLASS_ARCHER",         5, "UNITCLASS_SPEARMAN", 2, "UNITCLASS_CATAPULT", 2, "TECH_CONSTRUCTION",  2, 3, 60, 70, 75, "POLICY_BRANCH_LIBERTY", NULL, 0, 75, false, true },
		{ "CIVILIZATION_GREECE", "UNITCLASS_SPEARMAN",       3, "UNITCLASS_ARCHER",   5, NULL,                 0, "TECH_CONSTRUCTION",  2, 3, 50, 65, 75, "POLICY_BRANCH_LIBERTY", NULL, 0, 75, false, true },
		{ "CIVILIZATION_CELTS",  "UNITCLASS_SPEARMAN",       5, "UNITCLASS_ARCHER",   3, NULL,                 0, "TECH_CONSTRUCTION",  2, 3, 52, 67, 75, "POLICY_BRANCH_HONOR", NULL, 0, 75, false, true },
		// LateStrike (off; one flag with War's window and Economy's book): economy first, pre-build, save the gold, upgrade the turn the tech lands and strike.
		// Zulu (glory7, Deity Challenge #1): a ranged-led wave at T90-100 with Spearmen upgraded to Impi at Civil Service.
		// Mongolia (Deity Challenge #8): Chariot Archers kept alive and upgraded to Keshiks at Chivalry, T92-105, 8+ Keshiks
		// and 2 Horsemen to capture. Assyria (imMapex): Composite Bowmen with 1-2 Siege Towers and 1-2 melee, T80-110.
		// Japan (low confidence): Swordsmen upgraded to Samurai at Steel, T95-105. Songhai (low confidence): Horsemen
		// clearing camps for triple gold, upgraded to Mandekalu at Chivalry.
		{ "CIVILIZATION_ZULU",     "UNITCLASS_PIKEMAN",        5, "UNITCLASS_ARCHER",   8, NULL,                 0, "TECH_CIVIL_SERVICE", 0, 4, 90, 100, 120, "POLICY_BRANCH_LIBERTY",   "UNITCLASS_SPEARMAN",       5,  90, true, true },
		{ "CIVILIZATION_MONGOL",   "UNITCLASS_KNIGHT",         8, "UNITCLASS_HORSEMAN", 2, NULL,                 0, "TECH_CHIVALRY",      0, 3, 95, 105, 120, "POLICY_BRANCH_HONOR",     "UNITCLASS_CHARIOT_ARCHER", 8, 140, true, true },
		{ "CIVILIZATION_ASSYRIA",  "UNITCLASS_ARCHER",         5, "UNITCLASS_SPEARMAN", 2, "UNITCLASS_CATAPULT", 2, "TECH_CONSTRUCTION",  0, 3, 80,  95, 110, "POLICY_BRANCH_TRADITION", NULL,                       0,  75, true, false },
		{ "CIVILIZATION_JAPAN",    "UNITCLASS_LONGSWORDSMAN",  7, "UNITCLASS_ARCHER",   4, NULL,                 0, "TECH_STEEL",         0, 3, 100, 105, 120, "POLICY_BRANCH_TRADITION", "UNITCLASS_SWORDSMAN",      7, 100, true, true },
		// China (the user, 2026-11-05: "cho ko nu rush?"; the lead's research, STRIKE_PLANS_RESEARCH.md "China (Chu-Ko-Nu
		// rush)", thin BNW sources): 7-8 Chu-Ko-Nus (two attacks a turn; Composite Bowmen pre-built, upgraded when Machinery
		// lands, about 100 gold each, an estimate) and 2-3 melee to take the city, a Great General if there is one (Art of
		// War); no siege. Liberty or Tradition into Honor is more common than Honor first: Tradition here.
		{ "CIVILIZATION_CHINA",    "UNITCLASS_CROSSBOWMAN",    8, "UNITCLASS_SWORDSMAN", 3, NULL,                0, "TECH_MACHINERY",     0, 3, 85,  95, 115, "POLICY_BRANCH_TRADITION", "UNITCLASS_COMPOSITE_BOWMAN", 8, 100, true, true },
		{ "CIVILIZATION_SONGHAI",  "UNITCLASS_KNIGHT",         6, "UNITCLASS_ARCHER",   3, NULL,                 0, "TECH_CHIVALRY",      0, 2, 85,  95, 115, "POLICY_BRANCH_HONOR",     "UNITCLASS_HORSEMAN",       6, 100, true, true },
	};
	const int iSpeed = max(1, GC.getGame().getGameSpeedInfo().getTrainPercent());
	const char* szCiv = m_pPlayer->getCivilizationTypeKey();
	for (int i = 0; i < (int)(sizeof(kRows) / sizeof(kRows[0])); i++)
	{
		const ExpertRow& kRow = kRows[i];
		if (!szCiv || strcmp(szCiv, kRow.szCiv) != 0)
			continue;
		if (kRow.bLate && !m_pPlayer->UsesCandidateAI(FIX_LATE_STRIKE))
			continue;
		kPlan.bValid = true;
		kPlan.bExpert = true;
		kPlan.eCoreClass = (UnitClassTypes)GC.getInfoTypeForString(kRow.szCore, true);
		kPlan.iCoreTarget = kRow.iCore;
		kPlan.eSupportClass = (UnitClassTypes)GC.getInfoTypeForString(kRow.szSupport, true);
		kPlan.iSupportTarget = kRow.iSupport;
		kPlan.eSiegeClass = kRow.szSiege ? (UnitClassTypes)GC.getInfoTypeForString(kRow.szSiege, true) : NO_UNITCLASS;
		kPlan.iSiegeTarget = kRow.iSiege;
		kPlan.eUpgradeTech = kRow.szUpgradeTech ? (TechTypes)GC.getInfoTypeForString(kRow.szUpgradeTech, true) : NO_TECH;
		kPlan.iCoreBeforeSettler = kRow.iN;
		kPlan.iMaxCitiesBeforeStrike = kRow.iK;
		kPlan.iOpenTurn = kRow.iOpen * iSpeed / 100;
		kPlan.iDeclareByTurn = kRow.iDeclareBy * iSpeed / 100;
		kPlan.iFallbackTurn = kRow.iFallback * iSpeed / 100;
		kPlan.eFirstPolicyBranch = (PolicyBranchTypes)GC.getInfoTypeForString(kRow.szPolicy, true);
		kPlan.bSiegeOnlyIfWalled = true;
		kPlan.bBarbarianHunt = kRow.bBarbarians;
		kPlan.iUpgradeGoldPerUnit = kRow.iUpgradeGold;
		kPlan.bLateStrike = kRow.bLate;
		kPlan.ePreBuildClass = kRow.szPreBuild ? (UnitClassTypes)GC.getInfoTypeForString(kRow.szPreBuild, true) : NO_UNITCLASS;
		kPlan.iPreBuildTarget = kRow.iPreBuild;
		// late strike: half the upgrade bill in the treasury before the build-up (economy-ready)
		kPlan.iEconomyGoldTarget = kRow.bLate ? kRow.iPreBuild * kRow.iUpgradeGold / 2 : 0;
		// China's exits (the lead's research, "China (Chu-Ko-Nu rush)"): a target city at about 25-30+ defence, Knights or
		// gunpowder units near it, rough ground. Chu-Ko-Nus lost is War's army-lost exit for every plan.
		if (strcmp(kRow.szCiv, "CIVILIZATION_CHINA") == 0)
		{
			kPlan.iExitCityStrength = 25;
			kPlan.eExitCounterClass1 = (UnitClassTypes)GC.getInfoTypeForString("UNITCLASS_KNIGHT", true);
			kPlan.eExitCounterClass2 = (UnitClassTypes)GC.getInfoTypeForString("UNITCLASS_MUSKETMAN", true);
			kPlan.bExitRoughTarget = true;
		}
		SetStrikeLists(kPlan, kRow.szCiv);
		return;
	}

	// The data default: the spike unit is the core (a siege unique unit is the siege, with Archers as the core); support
	// is ranged with a melee core, melee with a ranged core. The window opens when the later of the spike and the upgrade
	// tech lands (every researched plan pre-builds and strikes then), declare within 10 turns.
	CvUnitEntry* pkSpike = GC.getUnitInfo(kProfile.eSpikeUnit);
	if (!pkSpike)
		return;
	const UnitClassTypes eArcher = (UnitClassTypes)GC.getInfoTypeForString("UNITCLASS_ARCHER", true);
	const UnitClassTypes eSpearman = (UnitClassTypes)GC.getInfoTypeForString("UNITCLASS_SPEARMAN", true);
	kPlan.bValid = true;
	if (pkSpike->GetDefaultUnitAIType() == UNITAI_CITY_BOMBARD)
	{
		kPlan.eSiegeClass = (UnitClassTypes)pkSpike->GetUnitClassType();
		kPlan.iSiegeTarget = 2;
		kPlan.eCoreClass = eArcher;
		kPlan.eSupportClass = eSpearman;
	}
	else
	{
		kPlan.eCoreClass = (UnitClassTypes)pkSpike->GetUnitClassType();
		kPlan.eSupportClass = (pkSpike->GetRangedCombat() > 0) ? eSpearman : eArcher;
	}
	kPlan.iCoreTarget = 5;
	kPlan.iSupportTarget = 3;
	kPlan.eUpgradeTech = kProfile.eUpgradeTech;
	kPlan.iCoreBeforeSettler = 2;
	kPlan.iMaxCitiesBeforeStrike = 3;
	const int iSpikeStd = kProfile.iSpikeTurn * 100 / iSpeed;
	const int iUpgradeStd = (kProfile.iUpgradeTurn >= 0) ? kProfile.iUpgradeTurn * 100 / iSpeed : -1;
	const int iOpenStd = max(iSpikeStd, iUpgradeStd) + 5;
	kPlan.iOpenTurn = iOpenStd * iSpeed / 100;
	kPlan.iDeclareByTurn = (iOpenStd + 10) * iSpeed / 100;
	kPlan.iFallbackTurn = (iOpenStd + 15) * iSpeed / 100;
	kPlan.eFirstPolicyBranch = kProfile.bMilitaryFirst ? (PolicyBranchTypes)GC.getInfoTypeForString("POLICY_BRANCH_LIBERTY", true) : NO_POLICY_BRANCH_TYPE;
	kPlan.bSiegeOnlyIfWalled = true;
	kPlan.bBarbarianHunt = kProfile.bMilitaryFirst;
	kPlan.bLateStrike = !kProfile.bMilitaryFirst;

	// The data default's lists [I], in the expert style: N core units, a worker and a settler, then support and core in
	// turn with a second settler when K allows; research the core's tech, a luxury, then the upgrade tech
	kPlan.bListInferred = true;
	const UnitClassTypes eWorker = (UnitClassTypes)GC.getInfoTypeForString("UNITCLASS_WORKER", true);
	const UnitClassTypes eSettler = (UnitClassTypes)GC.getInfoTypeForString("UNITCLASS_SETTLER", true);
	const BuildingClassTypes eMonument = (BuildingClassTypes)GC.getInfoTypeForString("BUILDINGCLASS_MONUMENT", true);
	if (kPlan.bLateStrike)
	{
		// economy first: monument, worker, settlers; the pre-build comes from the plan's targets, not this list
		AddStrikeBuild(kPlan, false, eMonument);
		AddStrikeBuild(kPlan, true, eWorker);
		AddStrikeBuild(kPlan, true, eSettler);
		AddStrikeBuild(kPlan, true, eSettler);
	}
	else
	{
		for (int iItem = 0; iItem < kPlan.iCoreBeforeSettler; iItem++)
			AddStrikeBuild(kPlan, true, kPlan.eCoreClass);
		AddStrikeBuild(kPlan, true, eWorker);
		AddStrikeBuild(kPlan, true, eSettler);
		AddStrikeBuild(kPlan, true, kPlan.eSupportClass);
		AddStrikeBuild(kPlan, true, kPlan.eSupportClass);
		AddStrikeBuild(kPlan, true, kPlan.eCoreClass);
		if (kPlan.iMaxCitiesBeforeStrike >= 3)
			AddStrikeBuild(kPlan, true, eSettler);
		AddStrikeBuild(kPlan, true, kPlan.eSupportClass);
		AddStrikeBuild(kPlan, true, kPlan.eCoreClass);
		AddStrikeBuild(kPlan, true, kPlan.eCoreClass);
	}
	const UnitTypes eCoreUnit = (kPlan.eCoreClass != NO_UNITCLASS) ? m_pPlayer->GetSpecificUnitType(kPlan.eCoreClass) : NO_UNIT;
	const TechTypes eCoreTech = (eCoreUnit != NO_UNIT) ? (TechTypes)GC.getUnitInfo(eCoreUnit)->GetPrereqAndTech() : NO_TECH;
	if (eCoreTech != NO_TECH && kPlan.iNumTechs < STRIKE_LIST_MAX)
		kPlan.aeTechs[kPlan.iNumTechs++] = eCoreTech;
	if (kPlan.iNumTechs < STRIKE_LIST_MAX)
		kPlan.aeTechs[kPlan.iNumTechs++] = STRIKE_TECH_LUXURY;
	if (kPlan.eUpgradeTech != NO_TECH && kPlan.eUpgradeTech != eCoreTech && kPlan.iNumTechs < STRIKE_LIST_MAX)
		kPlan.aeTechs[kPlan.iNumTechs++] = kPlan.eUpgradeTech;
}

/// AI_AB_TEST (Personality, the expert lists): append one build to the plan's list (NO class: skipped)
void CvGrandStrategyAI::AddStrikeBuild(CvStrikePlan& kPlan, bool bUnit, int iClass)
{
	if (iClass < 0 || kPlan.iNumBuilds >= STRIKE_LIST_MAX)
		return;
	kPlan.aBuilds[kPlan.iNumBuilds].bUnit = bUnit;
	kPlan.aBuilds[kPlan.iNumBuilds].iClass = iClass;
	kPlan.iNumBuilds++;
}

/// AI_AB_TEST (Personality; the user, 2026-11-06: "this is why we looked up expert build orders. literally just follow the
/// list"; the lead: the capital's first ~10-12 builds and the research to the upgrade tech, straight from
/// STRIKE_PLANS_RESEARCH.md). Source per row; [I] where the research gives no full list and this is the closest
/// expert-style list. Tokens: UNITCLASS_x / BUILDINGCLASS_x for builds, TECH_x or LUXURY (a luxury tech) for research.
void CvGrandStrategyAI::SetStrikeLists(CvStrikePlan& kPlan, const char* szCiv) const
{
	struct ListRow
	{
		const char* szCiv;
		bool bInferred;
		const char* szBuilds;
		const char* szTechs;
	};
	static const ListRow kLists[] = {
		// Huns (S4 Cromagnus): Scout, Monument, then Horse Archers continuously, a Warrior or Ram as needed; 1-2 cities
		// ([I]: the one settler's place). Research: The Wheel, a luxury, Bronze Working, Horseback Riding, then Writing/Math
		{ "CIVILIZATION_HUNS", true,
			"UNITCLASS_SCOUT BUILDINGCLASS_MONUMENT UNITCLASS_CHARIOT_ARCHER UNITCLASS_CHARIOT_ARCHER UNITCLASS_SETTLER UNITCLASS_CHARIOT_ARCHER UNITCLASS_SPEARMAN UNITCLASS_CHARIOT_ARCHER UNITCLASS_SPEARMAN UNITCLASS_WARRIOR UNITCLASS_CHARIOT_ARCHER",
			"TECH_THE_WHEEL LUXURY TECH_BRONZE_WORKING TECH_HORSEBACK_RIDING TECH_WRITING TECH_MATHEMATICS" },
		// Aztec (the user's game, which the sources fit): Jaguar x3, Worker, Settler, Archer x4, Jaguar x2.
		// Research: a luxury, Archery, Construction
		{ "CIVILIZATION_AZTEC", false,
			"UNITCLASS_WARRIOR UNITCLASS_WARRIOR UNITCLASS_WARRIOR UNITCLASS_WORKER UNITCLASS_SETTLER UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_WARRIOR UNITCLASS_WARRIOR",
			"LUXURY TECH_ARCHERY TECH_CONSTRUCTION" },
		// Inca ([I] + S19: Scout, Monument or Scout, Granary, Settler, Slingers from build 4-5; the user's Catapults).
		// Research: Pottery/Mining, a luxury, Archery, Mathematics (Catapults), Construction
		{ "CIVILIZATION_INCA", true,
			"UNITCLASS_SCOUT BUILDINGCLASS_MONUMENT UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_SETTLER UNITCLASS_WORKER UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_SPEARMAN UNITCLASS_SETTLER UNITCLASS_ARCHER UNITCLASS_CATAPULT UNITCLASS_CATAPULT UNITCLASS_SPEARMAN",
			"TECH_POTTERY TECH_MINING LUXURY TECH_ARCHERY TECH_MATHEMATICS TECH_CONSTRUCTION" },
		// Greece ([I] + S21: Monuments first, then units, no Granaries; 2-3 cities). Research: Bronze Working, Archery,
		// Horseback Riding, Construction
		{ "CIVILIZATION_GREECE", true,
			"BUILDINGCLASS_MONUMENT UNITCLASS_SPEARMAN UNITCLASS_SETTLER UNITCLASS_ARCHER UNITCLASS_SPEARMAN UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_SETTLER UNITCLASS_SPEARMAN UNITCLASS_ARCHER UNITCLASS_ARCHER",
			"TECH_BRONZE_WORKING TECH_ARCHERY LUXURY TECH_HORSEBACK_RIDING TECH_CONSTRUCTION" },
		// Celts (S7/S22 + [I]: Scout, Monument, Pictish Warrior x2 for barbarians, Settler, then Pictish Warriors and
		// Archers). Research: Bronze Working early, Archery, Construction
		{ "CIVILIZATION_CELTS", true,
			"UNITCLASS_SCOUT BUILDINGCLASS_MONUMENT UNITCLASS_SPEARMAN UNITCLASS_SPEARMAN UNITCLASS_SETTLER UNITCLASS_ARCHER UNITCLASS_SPEARMAN UNITCLASS_ARCHER UNITCLASS_SETTLER UNITCLASS_SPEARMAN UNITCLASS_ARCHER UNITCLASS_SPEARMAN",
			"TECH_BRONZE_WORKING LUXURY TECH_ARCHERY TECH_CONSTRUCTION" },
		// Zulu (S24 glory7): Scout, Monument, Scout, Granary, Shrine, Archer x3, Worker, Pyramids, Archer, Settler; then
		// Ikanda (Barracks), Spearmen. Research: Pottery, Archery, Mining, Animal Husbandry, Masonry, luxury, Construction,
		// Bronze Working, Horseback Riding, Writing, Currency, Machinery, Civil Service
		{ "CIVILIZATION_ZULU", false,
			"UNITCLASS_SCOUT BUILDINGCLASS_MONUMENT UNITCLASS_SCOUT BUILDINGCLASS_GRANARY BUILDINGCLASS_SHRINE UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_WORKER BUILDINGCLASS_PYRAMID UNITCLASS_ARCHER UNITCLASS_SETTLER BUILDINGCLASS_BARRACKS UNITCLASS_SPEARMAN",
			"TECH_POTTERY TECH_ARCHERY TECH_MINING TECH_ANIMAL_HUSBANDRY TECH_MASONRY LUXURY TECH_CONSTRUCTION TECH_BRONZE_WORKING TECH_HORSEBACK_RIDING TECH_WRITING TECH_CURRENCY TECH_MACHINERY TECH_CIVIL_SERVICE" },
		// Mongolia (S26 fils): Scout, Scout, Granary, Shrine, Caravan, Chariot Archer, Settler, Chariot Archer, Settler.
		// Research (hicks53081): Pottery, Writing, Animal Husbandry, Mining, Archery, Trapping, The Wheel, Bronze Working,
		// Philosophy, Civil Service, Currency, Chivalry
		{ "CIVILIZATION_MONGOL", false,
			"UNITCLASS_SCOUT UNITCLASS_SCOUT BUILDINGCLASS_GRANARY BUILDINGCLASS_SHRINE UNITCLASS_CARAVAN UNITCLASS_CHARIOT_ARCHER UNITCLASS_SETTLER UNITCLASS_CHARIOT_ARCHER UNITCLASS_SETTLER UNITCLASS_CHARIOT_ARCHER UNITCLASS_CHARIOT_ARCHER",
			"TECH_POTTERY TECH_WRITING TECH_ANIMAL_HUSBANDRY TECH_MINING TECH_ARCHERY TECH_TRAPPING TECH_THE_WHEEL TECH_BRONZE_WORKING TECH_PHILOSOPHY TECH_CIVIL_SERVICE TECH_CURRENCY TECH_CHIVALRY" },
		// Assyria (S28 imMapex): Scout, Scout, Temple of Artemis, Shrine, Worker, Archer, Settler, Granary, Composite Bowman
		// x2, Siege Tower, Composite Bowman, Royal Library (Library), Caravan, National College. Research: Archery, Mining,
		// luxury, Construction, luxury, Mathematics, Writing, Philosophy
		{ "CIVILIZATION_ASSYRIA", false,
			"UNITCLASS_SCOUT UNITCLASS_SCOUT BUILDINGCLASS_TEMPLE_ARTEMIS BUILDINGCLASS_SHRINE UNITCLASS_WORKER UNITCLASS_ARCHER UNITCLASS_SETTLER BUILDINGCLASS_GRANARY UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_CATAPULT UNITCLASS_ARCHER BUILDINGCLASS_LIBRARY UNITCLASS_CARAVAN BUILDINGCLASS_NATIONAL_COLLEGE",
			"TECH_ARCHERY TECH_MINING LUXURY TECH_CONSTRUCTION LUXURY TECH_MATHEMATICS TECH_WRITING TECH_PHILOSOPHY" },
		// Japan ([I] + S3: Tradition, 3 cities, National College around T70, pre-built Swordsmen). Research: a luxury,
		// Bronze Working, Iron Working, Writing, Mathematics, Construction, Civil Service, Machinery, Steel
		{ "CIVILIZATION_JAPAN", true,
			"UNITCLASS_SCOUT BUILDINGCLASS_MONUMENT UNITCLASS_SETTLER UNITCLASS_WORKER UNITCLASS_SETTLER BUILDINGCLASS_LIBRARY UNITCLASS_SWORDSMAN UNITCLASS_SWORDSMAN BUILDINGCLASS_NATIONAL_COLLEGE UNITCLASS_SWORDSMAN UNITCLASS_SWORDSMAN UNITCLASS_ARCHER",
			"LUXURY TECH_BRONZE_WORKING TECH_IRON_WORKING TECH_WRITING TECH_MATHEMATICS TECH_CONSTRUCTION TECH_CIVIL_SERVICE TECH_MACHINERY TECH_STEEL" },
		// Songhai (S33 Redaxe: Scout, Warrior, Warrior, Scout, Warrior, Settler; then [I] Horsemen for the camps).
		// Research (S27): Animal Husbandry, Horseback Riding, Writing, Philosophy, Civil Service, Currency, Chivalry
		{ "CIVILIZATION_SONGHAI", true,
			"UNITCLASS_SCOUT UNITCLASS_WARRIOR UNITCLASS_WARRIOR UNITCLASS_SCOUT UNITCLASS_WARRIOR UNITCLASS_SETTLER UNITCLASS_HORSEMAN UNITCLASS_WORKER UNITCLASS_HORSEMAN UNITCLASS_SETTLER UNITCLASS_HORSEMAN UNITCLASS_HORSEMAN",
			"TECH_ANIMAL_HUSBANDRY TECH_HORSEBACK_RIDING TECH_WRITING TECH_PHILOSOPHY TECH_CIVIL_SERVICE TECH_CURRENCY TECH_CHIVALRY" },
		// China ([I] + S36: Bimblecrumbs built 5 Archers then 2 more; Paper Maker (Library); 2-4 cities). Research
		// (S36 Bimblecrumbs, Gustavus Rex): Archery, a luxury, Writing, Philosophy, Construction, Machinery
		{ "CIVILIZATION_CHINA", true,
			"UNITCLASS_SCOUT BUILDINGCLASS_MONUMENT UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_SETTLER UNITCLASS_WORKER UNITCLASS_ARCHER BUILDINGCLASS_LIBRARY UNITCLASS_ARCHER UNITCLASS_SETTLER UNITCLASS_ARCHER UNITCLASS_ARCHER UNITCLASS_SPEARMAN",
			"TECH_ARCHERY LUXURY TECH_WRITING TECH_PHILOSOPHY TECH_CONSTRUCTION TECH_MACHINERY" },
	};
	for (int iRow = 0; iRow < (int)(sizeof(kLists) / sizeof(kLists[0])); iRow++)
	{
		if (strcmp(kLists[iRow].szCiv, szCiv) != 0)
			continue;
		kPlan.bListInferred = kLists[iRow].bInferred;
		const char* aszLists[2] = { kLists[iRow].szBuilds, kLists[iRow].szTechs };
		for (int iList = 0; iList < 2; iList++)
		{
			const std::string strList(aszLists[iList]);
			size_t iPos = 0;
			while (iPos < strList.size())
			{
				size_t iEnd = strList.find(' ', iPos);
				if (iEnd == std::string::npos)
					iEnd = strList.size();
				const std::string strToken = strList.substr(iPos, iEnd - iPos);
				iPos = iEnd + 1;
				if (strToken.empty())
					continue;
				if (iList == 0)
				{
					const bool bUnit = strToken.compare(0, 10, "UNITCLASS_") == 0;
					AddStrikeBuild(kPlan, bUnit, GC.getInfoTypeForString(strToken.c_str(), true));
				}
				else if (kPlan.iNumTechs < STRIKE_LIST_MAX)
				{
					const TechTypes eTech = (strToken == "LUXURY") ? STRIKE_TECH_LUXURY : (TechTypes)GC.getInfoTypeForString(strToken.c_str(), true);
					if (eTech != NO_TECH)
						kPlan.aeTechs[kPlan.iNumTechs++] = eTech;
				}
			}
		}
		return;
	}
}

/// AI_AB_TEST (Personality, strike plan; War's strike state): units of a class plus those of the classes it upgrades into
/// (one step), so Archers upgraded to Composite Bowmen still count as the plan's support
int CvGrandStrategyAI::CountStrikeClassLine(UnitClassTypes eClass) const
{
	if (!m_pPlayer || eClass == NO_UNITCLASS)
		return 0;
	int iCount = m_pPlayer->getUnitClassCount(eClass);
	const UnitTypes eUnit = m_pPlayer->GetSpecificUnitType(eClass);
	CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
	if (pkUnit)
	{
		for (int iClass = 0; iClass < GC.getNumUnitClassInfos(); iClass++)
		{
			if (iClass != (int)eClass && pkUnit->GetUpgradeUnitClass(iClass))
				iCount += m_pPlayer->getUnitClassCount((UnitClassTypes)iClass);
		}
	}
	return iCount;
}

/// Economy-ready, PREP to BUILDUP (the lead, 10:41pm ET; game state only, no turn).
/// - Early strike: the core unit can be trained, N core units are out (the opening's barbarian hunters), and the plan's K
///   cities are founded (or there's no room for more).
/// - Late strike: the core's tech or the upgrade tech is known or can be researched now (its prerequisites in), the plan's
///   K cities are founded (or no room), and the treasury holds half the upgrade bill.
bool CvGrandStrategyAI::IsStrikeEconomyReady() const
{
	const CvStrikePlan& kPlan = GetStrikePlan();
	if (!m_pPlayer || !kPlan.bValid || kPlan.eCoreClass == NO_UNITCLASS)
		return false;
	const bool bCitiesDone = m_pPlayer->getNumCities() >= kPlan.iMaxCitiesBeforeStrike || m_pPlayer->IsBoxedInForSettling();
	if (!kPlan.bLateStrike)
	{
		const UnitTypes eCore = m_pPlayer->GetSpecificUnitType(kPlan.eCoreClass);
		return eCore != NO_UNIT && m_pPlayer->canTrainUnit(eCore, false, false, /*bIgnoreCost*/ true) &&
			CountStrikeClassLine(kPlan.eCoreClass) >= kPlan.iCoreBeforeSettler && bCitiesDone;
	}
	const TechTypes eKeyTech = (kPlan.eUpgradeTech != NO_TECH) ? kPlan.eUpgradeTech : NO_TECH;
	bool bTechInReach = true;
	if (eKeyTech != NO_TECH)
		bTechInReach = GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech(eKeyTech) || m_pPlayer->GetPlayerTechs()->CanResearch(eKeyTech);
	return bTechInReach && bCitiesDone && m_pPlayer->GetTreasury()->GetGold() >= kPlan.iEconomyGoldTarget;
}

/// Army-ready, BUILDUP to WAR (game state only): the upgrade tech known (or none); core units at the target (pre-built units
/// that upgrade into the core count); support and always-brought siege at their targets (upgraded units count); and the
/// treasury covers the upgrades still to do (units of the pre-build, core and support lines whose upgrade the plan's tech
/// unlocks, iUpgradeGoldPerUnit each). War adds its own check: 2+ melee that can take a city, a target in reach.
bool CvGrandStrategyAI::IsStrikeArmyReady(int* piUpgradeBill) const
{
	const CvStrikePlan& kPlan = GetStrikePlan();
	if (piUpgradeBill)
		*piUpgradeBill = 0;
	if (!m_pPlayer || !kPlan.bValid || kPlan.eCoreClass == NO_UNITCLASS)
		return false;
	if (kPlan.eUpgradeTech != NO_TECH && !GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech(kPlan.eUpgradeTech))
		return false;
	const int iPreBuild = (kPlan.ePreBuildClass != NO_UNITCLASS) ? m_pPlayer->getUnitClassCount(kPlan.ePreBuildClass) : 0;
	if (CountStrikeClassLine(kPlan.eCoreClass) + iPreBuild < kPlan.iCoreTarget)
		return false;
	if (CountStrikeClassLine(kPlan.eSupportClass) < kPlan.iSupportTarget)
		return false;
	if (kPlan.eSiegeClass != NO_UNITCLASS && kPlan.iSiegeTarget > 0 && !kPlan.bSiegeOnlyIfWalled &&
		CountStrikeClassLine(kPlan.eSiegeClass) < kPlan.iSiegeTarget)
		return false;
	// the upgrade bill: units still in a class the plan's tech upgrades (pre-build, core or support line)
	int iBill = 0;
	if (kPlan.eUpgradeTech != NO_TECH)
	{
		const UnitClassTypes aeLines[3] = { kPlan.ePreBuildClass, kPlan.eCoreClass, kPlan.eSupportClass };
		for (int i = 0; i < 3; i++)
		{
			const UnitTypes eUnit = (aeLines[i] != NO_UNITCLASS) ? m_pPlayer->GetSpecificUnitType(aeLines[i]) : NO_UNIT;
			CvUnitEntry* pkUnit = (eUnit != NO_UNIT) ? GC.getUnitInfo(eUnit) : NULL;
			if (!pkUnit)
				continue;
			bool bUpgradedByTech = false;
			for (int iClass = 0; iClass < GC.getNumUnitClassInfos() && !bUpgradedByTech; iClass++)
			{
				if (!pkUnit->GetUpgradeUnitClass(iClass))
					continue;
				const UnitTypes eTo = m_pPlayer->GetSpecificUnitType((UnitClassTypes)iClass);
				bUpgradedByTech = eTo != NO_UNIT && GC.getUnitInfo(eTo)->GetPrereqAndTech() == (int)kPlan.eUpgradeTech;
			}
			if (bUpgradedByTech)
				iBill += m_pPlayer->getUnitClassCount(aeLines[i]) * kPlan.iUpgradeGoldPerUnit;
		}
	}
	if (piUpgradeBill)
		*piUpgradeBill = iBill;
	return m_pPlayer->GetTreasury()->GetGold() >= iBill;
}

/// In the strike window now: within its turns, and the spike's tech is known (the window follows the tech when research
/// runs late)
bool CvGrandStrategyAI::IsInStrikeWindow() const
{
	int iStart = -1, iEnd = -1;
	if (!GetStrikeWindow(iStart, iEnd))
		return false;
	const int iTurn = GC.getGame().getElapsedGameTurns();
	if (iTurn < iStart || iTurn > iEnd)
		return false;
	const TechTypes eTech = (TechTypes)m_iCachedSpikeTech;
	return eTech == NO_TECH || GET_TEAM(m_pPlayer->getTeam()).GetTeamTechs()->HasTech(eTech);
}

/// AI_AB_TEST (Personality, GreedyWide): out of room to settle (CvPlayer::IsBoxedInForSettling), once per turn: it looks
/// for a settle site, and building choices ask every time
bool CvGrandStrategyAI::IsGreedyWideOutOfRoom() const
{
	const int iTurn = GC.getGame().getGameTurn();
	if (m_iCachedOutOfRoomTurn != iTurn)
	{
		m_iCachedOutOfRoom = (m_pPlayer && m_pPlayer->getNumCities() > 0 && m_pPlayer->IsBoxedInForSettling()) ? 1 : 0;
		m_iCachedOutOfRoomTurn = iTurn;
	}
	return m_iCachedOutOfRoom != 0;
}

/// AI_AB_TEST (Personality, GreedyWarArmy): could these enemy land units take pCity, or nearly? A defender's version of the
/// siege sketch in CvDiplomacyAI.cpp (CanTakeNearestCity, flag CanWin), from units we can see: their ranged units fire,
/// the city heals and shoots the most damaging one, which drops out once it dies. "Close to capable" (the user,
/// 2026-11-04): their damage counts a third extra, and one melee unit is enough to walk in.
static bool CouldEnemyTakeCity(const vector<CvUnit*>& vpUnits, CvCity* pCity)
{
	if (!pCity || vpUnits.empty())
		return false;
	const int iGarrisonMaxHP = (pCity->HasGarrison() && pCity->GetGarrisonedUnit()) ? pCity->GetGarrisonedUnit()->GetMaxHitPoints() : 0;
	vector<int> viDamage, viHP, viHitTaken;
	int iMelee = 0;
	for (size_t i = 0; i < vpUnits.size(); i++)
	{
		CvUnit* pUnit = vpUnits[i];
		if (pUnit->IsCanAttackRanged())
		{
			int iGarrisonDamage = 0;
			viDamage.push_back(pUnit->GetRangeCombatDamage(NULL, pCity, iGarrisonMaxHP, iGarrisonDamage, /*bIncludeRand*/ false, 0, 0, NULL, NULL, true, true) * 4 / 3);
			viHP.push_back(pUnit->GetCurrHitPoints());
			viHitTaken.push_back(pCity->rangeCombatDamage(pUnit, /*bIncludeRand*/ false, NULL, /*bQuickAndDirty*/ true) * 3 / 2);
		}
		else
			iMelee++;
	}
	if (iMelee < 1)
		return false;

	const int iMaxHP = pCity->GetMaxHitPoints();
	const int iHeal = /*20*/ GD_INT_GET(CITY_HIT_POINTS_HEALED_PER_TURN);
	int iCityHP = iMaxHP - pCity->getDamage();
	for (int iTurn = 0; iTurn < 20; iTurn++)
	{
		int iFire = 0;
		int iTarget = -1;
		for (size_t i = 0; i < viDamage.size(); i++)
		{
			if (viHP[i] <= 0)
				continue;
			iFire += viDamage[i];
			if (iTarget < 0 || viDamage[i] > viDamage[iTarget])
				iTarget = (int)i;
		}
		if (iFire <= 0)
			return false;
		iCityHP -= iFire;
		if (iCityHP <= 0)
			return true;
		iCityHP = min(iMaxHP, iCityHP + iHeal);
		viHP[iTarget] -= viHitTaken[iTarget];
	}
	return false;
}

/// AI_AB_TEST (Personality, GreedyWarArmy and MetaDial's greedy side): a greedy civ at war judges how much threat it is
/// under (the user, 2026-11-04). An enemy on the other side of the world, or one whose army couldn't take a city, changes
/// nothing: the civ keeps its peace plan. A credible threat is:
/// - one of our cities under siege, or
/// - an enemy force we can see that could take one of our cities, or nearly (the user: "a large army is one that is
///   capable or close to capable of taking one of your cities"): one enemy's land combat units in sight within 8 plots
///   of the city, run through CouldEnemyTakeCity. Only units a human could see.
/// Returns 0 without a credible threat, else the land units the civ needs to defend: the larger of 2 per city and 3/4
/// of the enemy land combat units in sight within 5 plots of our cities. Enemies: major civs at war with us. Worked out
/// once per turn.
int CvGrandStrategyAI::GetWarDefenceLandUnitCap() const
{
	if (!m_pPlayer || !m_pPlayer->isMajorCiv())
		return 0;
	const int iTurn = GC.getGame().getGameTurn();
	if (m_iCachedWarDefenceTurn == iTurn)
		return m_iCachedWarDefenceCap;

	const TeamTypes eTeam = m_pPlayer->getTeam();
	bool bThreat = false;
	int iCityLoop = 0;
	for (CvCity* pCity = m_pPlayer->firstCity(&iCityLoop); pCity != NULL && !bThreat; pCity = m_pPlayer->nextCity(&iCityLoop))
	{
		if (pCity->isUnderSiege())
			bThreat = true;
	}

	int iEnemiesNear = 0;
	for (int iPlayerLoop = 0; iPlayerLoop < MAX_MAJOR_CIVS; iPlayerLoop++)
	{
		CvPlayer& kEnemy = GET_PLAYER((PlayerTypes)iPlayerLoop);
		if (!kEnemy.isAlive() || kEnemy.getTeam() == eTeam || !GET_TEAM(eTeam).isAtWar(kEnemy.getTeam()))
			continue;

		// this enemy's land combat units we can see
		vector<CvUnit*> vpSeen;
		int iUnitLoop = 0;
		for (CvUnit* pUnit = kEnemy.firstUnit(&iUnitLoop); pUnit != NULL; pUnit = kEnemy.nextUnit(&iUnitLoop))
		{
			if (pUnit->IsCombatUnit() && pUnit->getDomainType() == DOMAIN_LAND && pUnit->AI_getUnitAIType() != UNITAI_EXPLORE &&
				pUnit->plot() && pUnit->plot()->isVisible(eTeam))
				vpSeen.push_back(pUnit);
		}
		if (vpSeen.empty())
			continue;

		vector<bool> vbNear(vpSeen.size(), false);
		for (CvCity* pCity = m_pPlayer->firstCity(&iCityLoop); pCity != NULL; pCity = m_pPlayer->nextCity(&iCityLoop))
		{
			vector<CvUnit*> vpForce;
			for (size_t i = 0; i < vpSeen.size(); i++)
			{
				const int iDistance = plotDistance(vpSeen[i]->getX(), vpSeen[i]->getY(), pCity->getX(), pCity->getY());
				if (iDistance <= 8)
					vpForce.push_back(vpSeen[i]);
				if (iDistance <= 5)
					vbNear[i] = true;
			}
			if (!bThreat && CouldEnemyTakeCity(vpForce, pCity))
				bThreat = true;
		}
		for (size_t i = 0; i < vbNear.size(); i++)
			iEnemiesNear += vbNear[i] ? 1 : 0;
	}

	m_iCachedWarDefenceCap = bThreat ? max(2 * max(1, m_pPlayer->getNumCities()), (iEnemiesNear * 3 + 3) / 4) : 0;
	m_iCachedWarDefenceTurn = iTurn;
	return m_iCachedWarDefenceCap;
}

/// AI_AB_TEST (Personality, the styles and MetaDial): militaristic city-states give each friend a land unit every ~19
/// turns, and a city-state can gift one on first contact. In bully pair 0B09BFE1 these took greedy civs well past their
/// cap at peace (Siam: 3 land units built, 8 owned). A human can switch the gifts off in the city-state screen; a civ
/// with a unit cap does that while it is at its cap and switches them back on below it. The caps are the production
/// caps (CvUnitProductionAI::CheckUnitBuildSanity):
/// - GreedyFewUnits: 1 land unit per city in peace, 2 at war (with GreedyWarArmy: the peace cap, or the war defence
///   cap when a war is a credible threat);
/// - BalancedArmy: 3 per city in peace, none at war;
/// - AggressiveArmy: cities + 1 until the build-up window (CvPlayer::IsInWarBuildup), none inside it;
/// - MetaDial (when no style army flag is on): the peace ceiling up to meta position 0 (above it, the aggressive side
///   keeps its gifts); at war on the greedy side (-50 or below) the larger of that and the war defence cap, none above.
/// GreedyAlarm (a neighbour's army near our cities) lifts the cap (GreedyAlarmScale: by a few units). Unit gifts are the only thing the switch stops
/// (CvMinorCivAI::DoSpawnUnit, IsUnitSpawningAllowed).
void CvGrandStrategyAI::DoStyleCityStateUnitGifts()
{
	if (!m_pPlayer || !m_pPlayer->isMajorCiv() || m_pPlayer->isHuman() || !m_pPlayer->isAlive())
		return;

	const int iCities = max(1, m_pPlayer->getNumCities());
	const bool bWar = m_pPlayer->IsAtWarAnyMajor();
	int iCap = -1;	// -1: no cap
	if (m_pPlayer->UsesCandidateAI(FIX_GREEDY_FEW_UNITS))
		iCap = !bWar ? iCities : (m_pPlayer->UsesCandidateAI(FIX_GREEDY_WAR_ARMY) ? max(iCities, GetWarDefenceLandUnitCap()) : iCities * 2);
	else if (m_pPlayer->UsesCandidateAI(FIX_AGGRESSIVE_ARMY))
		iCap = m_pPlayer->IsInWarBuildup() ? -1 : iCities + 1;	// small army until the build-up window
	else if (m_pPlayer->UsesCandidateAI(FIX_BALANCED_ARMY))
		iCap = bWar ? -1 : iCities * 3;
	else if (m_pPlayer->UsesMetaDial())
	{
		const int iMeta = m_pPlayer->GetMetaDialPosition();
		if (!bWar)
		{
			iCap = (iMeta <= 0) ? iCities * m_pPlayer->GetMetaDialLandUnitsPerCityTimes10(true) / 10 : -1;
			if (iMeta >= 50 && !m_pPlayer->IsInWarBuildup())
				iCap = iCities + 1;
		}
		else
			iCap = (iMeta <= -50) ? max(iCities * m_pPlayer->GetMetaDialLandUnitsPerCityTimes10(true) / 10, GetWarDefenceLandUnitCap()) : -1;
	}
	else
		return;	// no style: the switch stays as it is (off)

	bool bRefuse = false;
	if (iCap >= 0)
	{
		const int iLand = m_pPlayer->getNumMilitaryLandUnits() - m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_EXPLORE, false, true);
		bRefuse = iLand >= iCap && !m_pPlayer->IsGreedyAlarmLiftingCap(iLand, iCap);	// GreedyAlarmScale: lifted by some
	}

	for (int iMinorLoop = MAX_MAJOR_CIVS; iMinorLoop < MAX_CIV_PLAYERS; iMinorLoop++)
	{
		CvPlayer& kMinor = GET_PLAYER((PlayerTypes)iMinorLoop);
		if (!kMinor.isEverAlive() || !kMinor.isMinorCiv())
			continue;
		if (kMinor.GetMinorCivAI()->IsUnitSpawningDisabled(m_pPlayer->GetID()) != bRefuse)
			kMinor.GetMinorCivAI()->SetUnitSpawningDisabled(m_pPlayer->GetID(), bRefuse);
	}
}

/// AI_AB_TEST (Personality, GreedyFewUnits and MetaDial's greedy side): in bully pair 0B58EE0F greedy civs kept the units
/// they built for a war after it ended (Assyria 11 land units on 5 cities in peace), and settler escorts built 2 over the
/// cap stayed after their operation. A greedy human disbands what it no longer needs to save upkeep. So in peace, once
/// 10 turns have passed since its last peace treaty and GreedyAlarm is quiet, a greedy civ above its peace cap lets one
/// land unit go each turn: the weakest that isn't a scout, isn't in an army, and preferably isn't in a city. Like CP's
/// own scrapping (CvMilitaryAI::DisbandObsoleteUnits), it gifts the unit to a city-state if one would take it.
void CvGrandStrategyAI::DoGreedyDisbandToCap()
{
	if (!m_pPlayer || !m_pPlayer->isMajorCiv() || m_pPlayer->isHuman() || !m_pPlayer->isAlive())
		return;
	const bool bGreedyFlag = m_pPlayer->UsesCandidateAI(FIX_GREEDY_FEW_UNITS);
	if (!bGreedyFlag && !m_pPlayer->IsMetaDialGreedy())
		return;
	if (m_pPlayer->IsAtWarAnyMajor() || CityStrategyAIHelpers::GetGreedyAlarmThreat(*m_pPlayer) != NO_PLAYER)
		return;

	// not right after a war: it may start again
	const int iTurn = GC.getGame().getGameTurn();
	for (int iTeamLoop = 0; iTeamLoop < MAX_MAJOR_CIVS; iTeamLoop++)
	{
		const TeamTypes eLoopTeam = (TeamTypes)iTeamLoop;
		if (eLoopTeam == m_pPlayer->getTeam())
			continue;
		const int iPeaceTurn = GET_TEAM(m_pPlayer->getTeam()).GetTurnMadePeaceTreatyWithTeam(eLoopTeam);
		if (iPeaceTurn >= 0 && iTurn - iPeaceTurn < 10)
			return;
	}

	const int iCities = max(1, m_pPlayer->getNumCities());
	const int iCap = bGreedyFlag ? iCities : iCities * m_pPlayer->GetMetaDialLandUnitsPerCityTimes10(true) / 10;
	const int iLand = m_pPlayer->getNumMilitaryLandUnits() - m_pPlayer->GetNumUnitsWithUnitAI(UNITAI_EXPLORE, false, true);
	if (iLand <= iCap)
		return;

	CvUnit* pBest = NULL;
	int iBestScore = INT_MAX;
	int iUnitLoop = 0;
	for (CvUnit* pUnit = m_pPlayer->firstUnit(&iUnitLoop); pUnit != NULL; pUnit = m_pPlayer->nextUnit(&iUnitLoop))
	{
		if (!pUnit->IsCombatUnit() || pUnit->getDomainType() != DOMAIN_LAND || pUnit->isDelayedDeath() || !pUnit->canScrap())
			continue;
		if (pUnit->AI_getUnitAIType() == UNITAI_EXPLORE || pUnit->getUnitInfo().GetDefaultUnitAIType() == UNITAI_EXPLORE)
			continue;
		if (pUnit->getArmyID() != -1)
			continue;
		// the weakest first, units outside cities before garrisons
		const int iScore = pUnit->GetPower() + ((pUnit->plot() && pUnit->plot()->isCity()) ? 100000 : 0);
		if (iScore < iBestScore)
		{
			iBestScore = iScore;
			pBest = pUnit;
		}
	}
	if (!pBest)
		return;

	if (GC.getLogging() && GC.getAILogging())
		m_pPlayer->GetHomelandAI()->LogHomelandMessage(CvString::format("GREEDY: %d land units over a cap of %d in peace, letting %s go",
			iLand, iCap, pBest->getUnitInfo().GetDescription()));

	const PlayerTypes eMinor = m_pPlayer->GetBestGiftTarget(DOMAIN_LAND);
	if (eMinor != NO_PLAYER)
		GET_PLAYER(eMinor).AddIncomingUnit(m_pPlayer->GetID(), pBest);
	else
		pBest->scrap();
}

/// AI_AB_TEST (Personality): the profile in StrategyProfile_Log.csv (logged games only), at turns 1, 50, 100, 150...
void CvGrandStrategyAI::LogStrategyProfile()
{
	if (!GC.getLogging() || !GC.getAILogging() || !m_pPlayer || !m_pPlayer->isMajorCiv() || m_pPlayer->isHuman())
		return;
	const int iTurn = GC.getGame().getElapsedGameTurns();
	if (iTurn != 1 && (iTurn <= 0 || iTurn % 50 != 0))
		return;

	static const char* aszRisk[3] = { "low", "medium", "high" };
	static const char* aszExpansion[4] = { "tall", "medium", "wide", "conquest" };
	const CvStrategyProfile kProfile = GetStrategyProfile();
	CvPlayerTraits* pTraits = m_pPlayer->GetPlayerTraits();
	CvString strTags;
	strTags.Format("%s%s%s%s%s%s%s", pTraits->IsWarmonger() ? "W" : "", pTraits->IsReligious() ? "R" : "", pTraits->IsTourism() ? "T" : "",
		pTraits->IsNerd() ? "N" : "", pTraits->IsSmaller() ? "S" : "", pTraits->IsDiplomat() ? "D" : "", pTraits->IsExpansionist() ? "E" : "");

	FILogFile* pLog = LOGFILEMGR.GetLog("StrategyProfile_Log.csv", FILogFile::kDontTimeStamp);
	if (!pLog)
		return;
	CvString strOut;
	strOut.Format("%03d, %s, %s, tags %s, pursuits %d/%d, %s, home meta %d, weights dom %d sci %d cul %d dip %d, risk %s, expansion %s, greedy %s, spike %s at %s T%d%s, opening %s, strike window T%d-T%d declare by T%d, core share %d%%, upgrade %s T%d",
		iTurn, m_pPlayer->getCivilizationShortDescription(), m_pPlayer->getLeaderTypeKey(), strTags.c_str(),
		(int)m_pPlayer->GetDiplomacyAI()->GetPrimaryVictoryPursuit(), (int)m_pPlayer->GetDiplomacyAI()->GetSecondaryVictoryPursuit(),
		GetStrategyArchetypeName(kProfile.eArchetype), kProfile.iHomeMeta,
		kProfile.aiVictoryWeight[STRATEGY_VICTORY_DOMINATION], kProfile.aiVictoryWeight[STRATEGY_VICTORY_SCIENCE],
		kProfile.aiVictoryWeight[STRATEGY_VICTORY_CULTURE], kProfile.aiVictoryWeight[STRATEGY_VICTORY_DIPLOMACY],
		aszRisk[kProfile.eRisk], aszExpansion[kProfile.eExpansion], kProfile.eGreedyKind == STRATEGY_GREEDY_WIDE ? "wide" : "tall",
		(kProfile.eSpikeUnit != NO_UNIT) ? GC.getUnitInfo(kProfile.eSpikeUnit)->GetType() : "none",
		(kProfile.eSpikeTech != NO_TECH) ? GC.getTechInfo(kProfile.eSpikeTech)->GetType() : "start", kProfile.iSpikeTurn,
		kProfile.bSpikeTerrain ? " (forest/jungle)" : "", kProfile.bMilitaryFirst ? "military first" : "expansion first",
		kProfile.iStrikeStart, kProfile.iStrikeEnd, kProfile.iStrikeDeclareBy, kProfile.iCoreSharePercent,
		(kProfile.eUpgradeTech != NO_TECH) ? GC.getTechInfo(kProfile.eUpgradeTech)->GetType() : "none", kProfile.iUpgradeTurn);
	pLog->Msg(strOut);

	const CvStrikePlan& kPlan = GetStrikePlan();
	if (kPlan.bValid)
	{
		CvString strPlan;
		strPlan.Format("%03d, %s, strike plan %s: core %s x%d, support %s x%d, siege %s x%d, upgrade %s (%d gold each), N %d, K %d, open T%d, declare by T%d, fallback T%d, policy %s%s%s",
			iTurn, m_pPlayer->getCivilizationShortDescription(), kPlan.bExpert ? "expert" : "default",
			(kPlan.eCoreClass != NO_UNITCLASS) ? GC.getUnitClassInfo(kPlan.eCoreClass)->GetType() : "none", kPlan.iCoreTarget,
			(kPlan.eSupportClass != NO_UNITCLASS) ? GC.getUnitClassInfo(kPlan.eSupportClass)->GetType() : "none", kPlan.iSupportTarget,
			(kPlan.eSiegeClass != NO_UNITCLASS) ? GC.getUnitClassInfo(kPlan.eSiegeClass)->GetType() : "none", kPlan.iSiegeTarget,
			(kPlan.eUpgradeTech != NO_TECH) ? GC.getTechInfo(kPlan.eUpgradeTech)->GetType() : "none", kPlan.iUpgradeGoldPerUnit,
			kPlan.iCoreBeforeSettler, kPlan.iMaxCitiesBeforeStrike, kPlan.iOpenTurn, kPlan.iDeclareByTurn, kPlan.iFallbackTurn,
			(kPlan.eFirstPolicyBranch != NO_POLICY_BRANCH_TYPE) ? GC.getPolicyBranchInfo(kPlan.eFirstPolicyBranch)->GetType() : "CP's choice",
			kPlan.bBarbarianHunt ? ", barbarian hunt first" : "",
			kPlan.bLateStrike ? (kPlan.ePreBuildClass != NO_UNITCLASS ? CvString::format(", late strike, pre-build %s x%d", GC.getUnitClassInfo(kPlan.ePreBuildClass)->GetType(), kPlan.iPreBuildTarget).c_str() : ", late strike") : "");
		pLog->Msg(strPlan);

		// the expert lists, item by item
		CvString strBuilds, strTechs;
		for (int iItem = 0; iItem < kPlan.iNumBuilds; iItem++)
		{
			const CvStrikeBuildItem& kItem = kPlan.aBuilds[iItem];
			const char* szType = kItem.bUnit ? (GC.getUnitClassInfo((UnitClassTypes)kItem.iClass) ? GC.getUnitClassInfo((UnitClassTypes)kItem.iClass)->GetType() : "?") :
				(GC.getBuildingClassInfo((BuildingClassTypes)kItem.iClass) ? GC.getBuildingClassInfo((BuildingClassTypes)kItem.iClass)->GetType() : "?");
			strBuilds += CvString::format("%s%s", iItem > 0 ? " " : "", szType);
		}
		for (int iItem = 0; iItem < kPlan.iNumTechs; iItem++)
		{
			const TechTypes eTech = kPlan.aeTechs[iItem];
			const char* szTech = (eTech == STRIKE_TECH_LUXURY) ? "LUXURY" : (eTech != NO_TECH && GC.getTechInfo(eTech)) ? GC.getTechInfo(eTech)->GetType() : "?";
			strTechs += CvString::format("%s%s", iItem > 0 ? " " : "", szTech);
		}
		CvString strLists;
		strLists.Format("%03d, %s, strike lists%s: builds [%s], research [%s]", iTurn, m_pPlayer->getCivilizationShortDescription(),
			kPlan.bListInferred ? " (partly inferred)" : "", strBuilds.c_str(), strTechs.c_str());
		pLog->Msg(strLists);
	}
}

/// Returns the Active Grand Strategy for this Player: how am I trying to win right now?
AIGrandStrategyTypes CvGrandStrategyAI::GetActiveGrandStrategy() const
{
	return m_eActiveGrandStrategy;
}

/// Sets the Active Grand Strategy for this Player: how am I trying to win right now?
void CvGrandStrategyAI::SetActiveGrandStrategy(AIGrandStrategyTypes eGrandStrategy)
{
	if(eGrandStrategy != NO_AIGRANDSTRATEGY)
	{
		m_eActiveGrandStrategy = eGrandStrategy;

		SetNumTurnsSinceActiveSet(0);
	}
}

/// The number of turns since the Active Strategy was last set
int CvGrandStrategyAI::GetNumTurnsSinceActiveSet() const
{
	return m_iNumTurnsSinceActiveSet;
}

/// Set the number of turns since the Active Strategy was last set
void CvGrandStrategyAI::SetNumTurnsSinceActiveSet(int iValue)
{
	m_iNumTurnsSinceActiveSet = iValue;
	ASSERT(m_iNumTurnsSinceActiveSet >= 0);
}

/// Change the number of turns since the Active Strategy was last set
void CvGrandStrategyAI::ChangeNumTurnsSinceActiveSet(int iChange)
{
	if(iChange != 0)
	{
		m_iNumTurnsSinceActiveSet += iChange;
	}

	ASSERT(m_iNumTurnsSinceActiveSet >= 0);
}

/// Returns the Priority Level the player has for a particular Grand Strategy
int CvGrandStrategyAI::GetGrandStrategyPriority(AIGrandStrategyTypes eGrandStrategy) const
{
	PRECONDITION(eGrandStrategy != NO_AIGRANDSTRATEGY);
	return m_paiGrandStrategyPriority[eGrandStrategy];
}

/// Sets the Priority Level the player has for a particular Grand Strategy
void CvGrandStrategyAI::SetGrandStrategyPriority(AIGrandStrategyTypes eGrandStrategy, int iValue)
{
	PRECONDITION(eGrandStrategy != NO_AIGRANDSTRATEGY);
	m_paiGrandStrategyPriority[eGrandStrategy] = iValue;
}

/// Changes the Priority Level the player has for a particular Grand Strategy
void CvGrandStrategyAI::ChangeGrandStrategyPriority(AIGrandStrategyTypes eGrandStrategy, int iChange)
{
	PRECONDITION(eGrandStrategy != NO_AIGRANDSTRATEGY);

	if(iChange != 0)
	{
		m_paiGrandStrategyPriority[eGrandStrategy] += iChange;
	}
}



// **********
// Stuff relating to guessing what other Players are up to
// **********



/// Runs every turn to try and figure out what other known Players' Grand Strategies are
void CvGrandStrategyAI::DoGuessOtherPlayersActiveGrandStrategy()
{
	CvWeightedVector<int> vGrandStrategyPriorities;
	vector<int>  vGrandStrategyPrioritiesForLogging;

	GuessConfidenceTypes eGuessConfidence = GUESS_CONFIDENCE_UNSURE;

	int iGrandStrategiesLoop = 0;
	AIGrandStrategyTypes eGrandStrategy = NO_AIGRANDSTRATEGY;
	CvAIGrandStrategyXMLEntry* pGrandStrategy = 0;
	CvString strGrandStrategyName;

	CvTeam& pTeam = GET_TEAM(GetPlayer()->getTeam());

	int iMajorLoop = 0;
	PlayerTypes eMajor = NO_PLAYER;

	int iPriority = 0;

	// Establish world Military strength average
	int iWorldMilitaryAverage = GC.getGame().GetWorldMilitaryStrengthAverage(GetPlayer()->GetID(), true, true);

	// Establish world culture and tourism averages
	int iNumPlayersAlive = 0;
	int iWorldCultureAverage = 0;
	int iWorldTourismAverage = 0;
	for(iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		eMajor = (PlayerTypes) iMajorLoop;

		if(GET_PLAYER(eMajor).isAlive())
		{
			iWorldCultureAverage += (int)(GET_PLAYER(eMajor).GetJONSCultureEverGeneratedTimes100() / 100);
			iWorldTourismAverage += GET_PLAYER(eMajor).GetCulture()->GetTourism() / 100;
			iNumPlayersAlive++;
		}
	}
	iWorldCultureAverage /= iNumPlayersAlive;
	iWorldTourismAverage /= iNumPlayersAlive;

	// Establish world Tech progress average
	iNumPlayersAlive = 0;
	int iWorldNumTechsAverage = 0;
	TeamTypes eTeam;
	for(int iTeamLoop = 0; iTeamLoop < MAX_MAJOR_CIVS; iTeamLoop++)	// Looping over all MAJOR teams
	{
		eTeam = (TeamTypes) iTeamLoop;

		if(GET_TEAM(eTeam).isAlive())
		{
			iWorldNumTechsAverage += GET_TEAM(eTeam).GetTeamTechs()->GetNumTechsKnown();
			iNumPlayersAlive++;
		}
	}
	iWorldNumTechsAverage /= iNumPlayersAlive;

	// Look at every Major we've met
	for(iMajorLoop = 0; iMajorLoop < MAX_MAJOR_CIVS; iMajorLoop++)
	{
		eMajor = (PlayerTypes) iMajorLoop;

		if(GET_PLAYER(eMajor).isAlive() && iMajorLoop != GetPlayer()->GetID())
		{
			if(pTeam.isHasMet(GET_PLAYER(eMajor).getTeam()))
			{
				for(iGrandStrategiesLoop = 0; iGrandStrategiesLoop < GetAIGrandStrategies()->GetNumAIGrandStrategies(); iGrandStrategiesLoop++)
				{
					eGrandStrategy = (AIGrandStrategyTypes) iGrandStrategiesLoop;
					pGrandStrategy = GetAIGrandStrategies()->GetEntry(iGrandStrategiesLoop);
					strGrandStrategyName = (CvString) pGrandStrategy->GetType();

					if(strGrandStrategyName == "AIGRANDSTRATEGY_CONQUEST")
					{
						iPriority = GetGuessOtherPlayerConquestPriority(eMajor, iWorldMilitaryAverage);
					}
					else if(strGrandStrategyName == "AIGRANDSTRATEGY_CULTURE")
					{
						iPriority = GetGuessOtherPlayerCulturePriority(eMajor, iWorldCultureAverage, iWorldTourismAverage);
					}
					else if(strGrandStrategyName == "AIGRANDSTRATEGY_UNITED_NATIONS")
					{
						iPriority = GetGuessOtherPlayerUnitedNationsPriority(eMajor);
					}
					else if(strGrandStrategyName == "AIGRANDSTRATEGY_SPACESHIP")
					{
						iPriority = GetGuessOtherPlayerSpaceshipPriority(eMajor, iWorldNumTechsAverage);
					}

					vGrandStrategyPriorities.push_back(iGrandStrategiesLoop, iPriority);
					vGrandStrategyPrioritiesForLogging.push_back(iPriority);
				}

				if(vGrandStrategyPriorities.size() > 0)
				{
					// Add "No Grand Strategy" in case we just don't have enough info to go on
					iPriority = /*40*/ GD_INT_GET(AI_GRAND_STRATEGY_GUESS_NO_CLUE_WEIGHT);

					vGrandStrategyPriorities.push_back(NO_AIGRANDSTRATEGY, iPriority);
					vGrandStrategyPrioritiesForLogging.push_back(iPriority);

					vGrandStrategyPriorities.StableSortItems();

					eGrandStrategy = (AIGrandStrategyTypes) vGrandStrategyPriorities.GetElement(0);
					iPriority = vGrandStrategyPriorities.GetWeight(0);
					eGuessConfidence = GUESS_CONFIDENCE_UNSURE;

					// How confident are we in our Guess?
					if(eGrandStrategy != NO_AIGRANDSTRATEGY)
					{
						if(iPriority >= /*150*/ GD_INT_GET(AI_GRAND_STRATEGY_GUESS_POSITIVE_THRESHOLD))
						{
							eGuessConfidence = GUESS_CONFIDENCE_POSITIVE;
						}
						else if(iPriority >= /*80*/ GD_INT_GET(AI_GRAND_STRATEGY_GUESS_LIKELY_THRESHOLD))
						{
							eGuessConfidence = GUESS_CONFIDENCE_LIKELY;
						}
						else
						{
							eGuessConfidence = GUESS_CONFIDENCE_UNSURE;
						}
					}

					SetGuessOtherPlayerActiveGrandStrategy(eMajor, eGrandStrategy, eGuessConfidence);

					LogGuessOtherPlayerGrandStrategy(vGrandStrategyPrioritiesForLogging, eMajor);
				}

				vGrandStrategyPriorities.clear();
				vGrandStrategyPrioritiesForLogging.clear();
			}
		}
	}
}

/// What does this AI BELIEVE another player's Active Grand Strategy to be?
AIGrandStrategyTypes CvGrandStrategyAI::GetGuessOtherPlayerActiveGrandStrategy(PlayerTypes ePlayer) const
{
	PRECONDITION(ePlayer < MAX_MAJOR_CIVS);
	return (AIGrandStrategyTypes) m_eGuessOtherPlayerActiveGrandStrategy[ePlayer];
}

/// How confident is the AI in its guess of what another player's Active Grand Strategy is?
GuessConfidenceTypes CvGrandStrategyAI::GetGuessOtherPlayerActiveGrandStrategyConfidence(PlayerTypes ePlayer) const
{
	PRECONDITION(ePlayer < MAX_MAJOR_CIVS);
	return (GuessConfidenceTypes) m_eGuessOtherPlayerActiveGrandStrategyConfidence[ePlayer];
}

/// Sets what this AI BELIEVES another player's Active Grand Strategy to be
void CvGrandStrategyAI::SetGuessOtherPlayerActiveGrandStrategy(PlayerTypes ePlayer, AIGrandStrategyTypes eGrandStrategy, GuessConfidenceTypes eGuessConfidence)
{
	PRECONDITION(ePlayer < MAX_MAJOR_CIVS);
	m_eGuessOtherPlayerActiveGrandStrategy[ePlayer] = eGrandStrategy;
	m_eGuessOtherPlayerActiveGrandStrategyConfidence[ePlayer] = eGuessConfidence;
}

bool CvGrandStrategyAI::OtherPlayerDoingBetterThanUs(PlayerTypes ePlayer, AIGrandStrategyTypes eGrandStrategy)
{
	CvAIGrandStrategyXMLEntry* pGrandStrategy = 0;
	CvString strGrandStrategyName;

	if(ePlayer == NO_PLAYER || eGrandStrategy == NO_AIGRANDSTRATEGY)
		return false;

	pGrandStrategy = GetAIGrandStrategies()->GetEntry((int)eGrandStrategy);
	if(pGrandStrategy)
	{
		strGrandStrategyName = (CvString) pGrandStrategy->GetType();
		if(strGrandStrategyName == "AIGRANDSTRATEGY_CONQUEST")
		{
			int iNumCapitalsUs = GetPlayer()->GetNumCapitalCities();
			int iNumCapitalsThem = GET_PLAYER(ePlayer).GetNumCapitalCities();
			int iMilitaryPowerUs = GetPlayer()->GetMilitaryMight();
			int iMilitaryPowerThem = GET_PLAYER(ePlayer).GetMilitaryMight();
			iMilitaryPowerUs *= max(1, iNumCapitalsUs);
			iMilitaryPowerThem *= max(1, iNumCapitalsThem);
			if(iMilitaryPowerThem > iMilitaryPowerUs)
			{
				return true;
			}
		}
		else if(strGrandStrategyName == "AIGRANDSTRATEGY_CULTURE")
		{
			int iOurInfluence = GetPlayer()->GetCulture()->GetNumCivsInfluentialOn();
			int iOurTourism = GetPlayer()->GetCulture()->GetTourism() / 100;
			int iOurCulture = GetPlayer()->GetTotalJONSCulturePerTurnTimes100() / 100;
			int iOurTotal = (iOurCulture + iOurTourism) * (max(1, iOurInfluence));
			
			int iTheirInfluence = GET_PLAYER(ePlayer).GetCulture()->GetNumCivsInfluentialOn();
			int iTheirTourism = GET_PLAYER(ePlayer).GetCulture()->GetTourism() / 100;
			int iTheirCulture = GET_PLAYER(ePlayer).GetTotalJONSCulturePerTurnTimes100() / 100;
			int iTheirTotal = (iTheirCulture + iTheirTourism) * (max(1, iTheirInfluence));

			if(iTheirTotal > iOurTotal)
			{
				return true;
			}
		}
		else if(strGrandStrategyName == "AIGRANDSTRATEGY_UNITED_NATIONS")
		{
			int iTheirVotes = 0;
			int iMyVotes = 0;
			CvLeague* pLeague = GC.getGame().GetGameLeagues()->GetActiveLeague();
			if(pLeague != NULL)
			{
				iTheirVotes = pLeague->CalculateStartingVotesForMember(ePlayer);
				iMyVotes = pLeague->CalculateStartingVotesForMember(m_pPlayer->GetID());
				if((iTheirVotes * 2) > (iMyVotes * 3))
				{
					return true;
				}
			}	
		}
		else if(strGrandStrategyName == "AIGRANDSTRATEGY_SPACESHIP")
		{
			ProjectTypes eApolloProgram = (ProjectTypes) GC.getInfoTypeForString("PROJECT_APOLLO_PROGRAM", true);
			if(eApolloProgram != NO_PROJECT)
			{
				//They have Apollo, and we don't.
				if(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount(eApolloProgram) > 0)
				{
					if(GET_TEAM(m_pPlayer->getTeam()).getProjectCount(eApolloProgram) <= 0)
					{
						return true;
					}
				}
				else
				{
					int iTheirTechNum = GET_TEAM(GET_PLAYER(ePlayer).getTeam()).GetTeamTechs()->GetNumTechsKnown();
					int iOurTechNum = GET_TEAM(GetPlayer()->getTeam()).GetTeamTechs()->GetNumTechsKnown();
					int iOurScience = GetPlayer()->GetScience();
					int iTheirScience = GET_PLAYER(ePlayer).GetScience();
					int iOurTotal = (iOurTechNum * iOurScience);
					int iTheirTotal = (iTheirTechNum * iTheirScience);

					if(iTheirTotal > iOurTotal)
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}

/// Guess as to how much another Player is prioritizing Conquest as his means of winning the game
int CvGrandStrategyAI::GetGuessOtherPlayerConquestPriority(PlayerTypes ePlayer, int iWorldMilitaryAverage)
{
	// If they can't attempt Domination Victory because of game options, then don't bother with any of this.
	if (!GC.getGame().CanPlayerAttemptDominationVictory(ePlayer, NO_PLAYER, GC.getGame().areNoVictoriesValid()))
		return 0;

	int iConquestPriority = 0;

	// Compare their Military to the world average; Possible range is 100 to -100 (but will typically be around -20 to 20)
	if(iWorldMilitaryAverage > 0)
	{
		iConquestPriority += (GET_PLAYER(ePlayer).GetMilitaryMight() - iWorldMilitaryAverage) * /*100*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_POWER_RATIO_MULTIPLIER) / iWorldMilitaryAverage;
	}

	// Minors attacked
	iConquestPriority += GetPlayer()->GetDiplomacyAI()->GetOtherPlayerNumMinorsAttacked(ePlayer) * /*5*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_WEIGHT_PER_MINOR_ATTACKED);

	// Minors Conquered
	iConquestPriority += GetPlayer()->GetDiplomacyAI()->GetPlayerNumMinorsConquered(ePlayer) * /*10*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_WEIGHT_PER_MINOR_CONQUERED);

	// Majors attacked
	iConquestPriority += GetPlayer()->GetDiplomacyAI()->GetOtherPlayerNumMajorsAttacked(ePlayer) * /*10*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_WEIGHT_PER_MAJOR_ATTACKED);

	// Majors Conquered
	iConquestPriority += GetPlayer()->GetDiplomacyAI()->GetPlayerNumMajorsConquered(ePlayer) * /*15*/ GD_INT_GET(AI_GRAND_STRATEGY_CONQUEST_WEIGHT_PER_MAJOR_CONQUERED);

	//Autocracy is usually a sure bet for conquest strategy.
	//More than half of all Capitals?
	if(GET_PLAYER(ePlayer).GetNumCapitalCities() >= 1 && (GET_PLAYER(ePlayer).GetNumCapitalCities() >= (GC.getGame().GetNumMajorCivsEver() / 2)))
	{
		iConquestPriority *= GET_PLAYER(ePlayer).GetNumCapitalCities() * 20;
	}
	if(GET_PLAYER(ePlayer).GetDiplomacyAI()->GetWarmongerThreat(ePlayer) >= THREAT_MAJOR)
	{
		iConquestPriority *= GET_PLAYER(ePlayer).GetDiplomacyAI()->GetWarmongerThreat(ePlayer) * 5;
	}
	PolicyBranchTypes eCurrentBranchType = GET_PLAYER(ePlayer).GetPlayerPolicies()->GetLateGamePolicyTree();

	if (eCurrentBranchType == (PolicyBranchTypes)GD_INT_GET(POLICY_BRANCH_AUTOCRACY) || eCurrentBranchType == (PolicyBranchTypes)GD_INT_GET(POLICY_BRANCH_ORDER))
	{
		iConquestPriority *= 3;
		iConquestPriority /= 2;
	}

	return max(iConquestPriority, 0);
}

/// Guess as to how much another Player is prioritizing Culture as his means of winning the game
int CvGrandStrategyAI::GetGuessOtherPlayerCulturePriority(PlayerTypes ePlayer, int iWorldCultureAverage, int iWorldTourismAverage)
{
	VictoryTypes eVictory = (VictoryTypes) GC.getInfoTypeForString("VICTORY_CULTURAL", true);

	// If Culture Victory isn't even available then don't bother with anything
	if (eVictory == NO_VICTORY)
		return 0;

	int iCulturePriority = 0;
	int iRatio = 0;

	// Compare their Culture to the world average; Possible range is 75 to -75
	if(iWorldCultureAverage > 0)
	{
		iRatio = ((int)(GET_PLAYER(ePlayer).GetJONSCultureEverGeneratedTimes100() / 100) - iWorldCultureAverage) * /*60*/ GD_INT_GET(AI_GS_CULTURE_RATIO_MULTIPLIER) / iWorldCultureAverage;
		if (iRatio > GD_INT_GET(AI_GS_CULTURE_RATIO_MULTIPLIER))
		{
			iCulturePriority += GD_INT_GET(AI_GS_CULTURE_RATIO_MULTIPLIER);
		}
		else if (iRatio < -GD_INT_GET(AI_GS_CULTURE_RATIO_MULTIPLIER))
		{
			iCulturePriority += -GD_INT_GET(AI_GS_CULTURE_RATIO_MULTIPLIER);
		}
		iCulturePriority += iRatio;
	}

	// Compare their Tourism to the world average; Possible range is 75 to -75
	if(iWorldTourismAverage > 0)
	{
		iRatio = (GET_PLAYER(ePlayer).GetCulture()->GetTourism() / 100 - iWorldTourismAverage) * /*80*/ GD_INT_GET(AI_GS_TOURISM_RATIO_MULTIPLIER) / iWorldTourismAverage;
		if (iRatio > GD_INT_GET(AI_GS_TOURISM_RATIO_MULTIPLIER))
		{
			iCulturePriority += GD_INT_GET(AI_GS_TOURISM_RATIO_MULTIPLIER);
		}
		else if (iRatio < -GD_INT_GET(AI_GS_TOURISM_RATIO_MULTIPLIER))
		{
			iCulturePriority += -GD_INT_GET(AI_GS_TOURISM_RATIO_MULTIPLIER);
		}
		iCulturePriority += iRatio;	
	}

	//Influential on a lot of civs?
	if(GET_PLAYER(ePlayer).GetCulture()->GetNumCivsInfluentialOn() >= (GC.getGame().GetNumMajorCivsEver() / 2))
	{
		iCulturePriority *= (GET_PLAYER(ePlayer).GetCulture()->GetNumCivsInfluentialOn() * 5);
	}

	//Freedom is usually a sure bet for culture strategy.
	PolicyBranchTypes eCurrentBranchType = GET_PLAYER(ePlayer).GetPlayerPolicies()->GetLateGamePolicyTree();

	if (eCurrentBranchType == (PolicyBranchTypes)GD_INT_GET(POLICY_BRANCH_FREEDOM) || eCurrentBranchType == (PolicyBranchTypes)GD_INT_GET(POLICY_BRANCH_AUTOCRACY))
	{
		iCulturePriority *= 3;
		iCulturePriority /= 2;
	}

	return max(iCulturePriority, 0);
}

/// Guess as to how much another Player is prioritizing the UN as his means of winning the game
int CvGrandStrategyAI::GetGuessOtherPlayerUnitedNationsPriority(PlayerTypes ePlayer)
{
	VictoryTypes eVictory = (VictoryTypes) GC.getInfoTypeForString("VICTORY_DIPLOMATIC", true);

	// If UN Victory isn't even available then don't bother with anything
	if (eVictory == NO_VICTORY)
		return 0;

	int iTheirCityStateAllies = 0;
	int iTheirCityStateFriends = 0;
	int iCityStatesAlive = 0;
	for(int iPlayerLoop = 0; iPlayerLoop < MAX_CIV_PLAYERS; iPlayerLoop++)
	{
		PlayerTypes eLoopPlayer = (PlayerTypes) iPlayerLoop;

		if (eLoopPlayer != ePlayer && GET_PLAYER(eLoopPlayer).isAlive() && GET_PLAYER(eLoopPlayer).isMinorCiv())
		{
			iCityStatesAlive++;
			if (GET_PLAYER(eLoopPlayer).GetMinorCivAI()->IsAllies(ePlayer))
			{
				iTheirCityStateAllies++;
			}
			else if (GET_PLAYER(eLoopPlayer).GetMinorCivAI()->IsFriends(ePlayer))
			{
				iTheirCityStateFriends++;
			}
		}
	}
	iCityStatesAlive = MAX(iCityStatesAlive, 1);

	int iPriority = (iTheirCityStateAllies + (iTheirCityStateFriends / 3)) * /*300*/ GD_INT_GET(AI_GS_UN_SECURED_VOTE_MOD) / iCityStatesAlive;

	//Freedom is usually a sure bet for diplomatic strategy.
	PolicyBranchTypes eCurrentBranchType = GET_PLAYER(ePlayer).GetPlayerPolicies()->GetLateGamePolicyTree();

	if (eCurrentBranchType == (PolicyBranchTypes)GD_INT_GET(POLICY_BRANCH_FREEDOM) || eCurrentBranchType == (PolicyBranchTypes)GD_INT_GET(POLICY_BRANCH_AUTOCRACY))
	{
		iPriority *= 3;
		iPriority /= 2;
	}

	//Lots of votes? That's not good.
	if(GC.getGame().GetGameLeagues()->GetActiveLeague() != NULL)
	{
		if(GC.getGame().GetGameLeagues()->GetActiveLeague()->CalculateStartingVotesForMember(ePlayer) > GC.getGame().GetVotesNeededForDiploVictory())
		{
			iPriority *= (GC.getGame().GetGameLeagues()->GetActiveLeague()->CalculateStartingVotesForMember(ePlayer) / 4);
		}
	}

	return max(iPriority, 0);
}

/// Guess as to how much another Player is prioritizing the SS as his means of winning the game
int CvGrandStrategyAI::GetGuessOtherPlayerSpaceshipPriority(PlayerTypes ePlayer, int iWorldNumTechsAverage)
{
	VictoryTypes eVictory = (VictoryTypes) GC.getInfoTypeForString("VICTORY_SPACE_RACE", true);

	// If SS Victory isn't even available then don't bother with anything
	if (eVictory == NO_VICTORY)
		return 0;

	TeamTypes eTeam = GET_PLAYER(ePlayer).getTeam();

	// If the player has the Apollo Program we're pretty sure he's going for the SS
	ProjectTypes eApolloProgram = (ProjectTypes) GC.getInfoTypeForString("PROJECT_APOLLO_PROGRAM", true);
	if(eApolloProgram != NO_PROJECT)
	{
		if(GET_TEAM(eTeam).getProjectCount(eApolloProgram) > 0)
		{
			return /*150*/ GD_INT_GET(AI_GS_SS_HAS_APOLLO_PROGRAM);
		}
	}

	int iNumTechs = GET_TEAM(eTeam).GetTeamTechs()->GetNumTechsKnown();
	int iSSPriority = (iNumTechs - iWorldNumTechsAverage) * /*600*/ GD_INT_GET(AI_GS_SS_TECH_PROGRESS_MOD) / max(iWorldNumTechsAverage, 1);

	PolicyBranchTypes eCurrentBranchType = GET_PLAYER(ePlayer).GetPlayerPolicies()->GetLateGamePolicyTree();
	if (eCurrentBranchType == (PolicyBranchTypes)GD_INT_GET(POLICY_BRANCH_ORDER) || eCurrentBranchType == (PolicyBranchTypes)GD_INT_GET(POLICY_BRANCH_FREEDOM))
	{
		iSSPriority *= 3;
		iSSPriority /= 2;
	}

	return max(iSSPriority, 0);
}


// PRIVATE METHODS

/// Log GrandStrategy state: what are the Priorities and who is Active?
void CvGrandStrategyAI::LogGrandStrategies(const vector<int>& vModifiedGrandStrategyPriorities)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strTemp;
		CvString playerName;
		CvString strDesc;
		CvString strLogName;

		// Find the name of this civ and city
		playerName = GetPlayer()->getCivilizationShortDescription();

		// Open the log file
		if(GC.getPlayerAndCityAILogSplit())
		{
			strLogName = "GrandStrategyAI_Log_" + playerName + ".csv";
		}
		else
		{
			strLogName = "GrandStrategyAI_Log.csv";
		}

		FILogFile* pLog = NULL;
		pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);

		AIGrandStrategyTypes eGrandStrategy;

		// Loop through Grand Strategies
		for(int iGrandStrategyLoop = 0; iGrandStrategyLoop < GC.getNumAIGrandStrategyInfos(); iGrandStrategyLoop++)
		{
			// Get the leading info for this line
			strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
			strBaseString += playerName + ", ";

			eGrandStrategy = (AIGrandStrategyTypes) iGrandStrategyLoop;

			// GrandStrategy Info
			CvAIGrandStrategyXMLEntry* pEntry = GC.getAIGrandStrategyInfo(eGrandStrategy);
			const char* szAIGrandStrategyType = (pEntry != NULL)? pEntry->GetType() : "Unknown Type";

			if(GetActiveGrandStrategy() == eGrandStrategy)
			{
				strTemp.Format("*** %s, %d, %d", szAIGrandStrategyType, GetGrandStrategyPriority(eGrandStrategy), vModifiedGrandStrategyPriorities[eGrandStrategy]);
			}
			else
			{
				strTemp.Format("%s, %d, %d", szAIGrandStrategyType, GetGrandStrategyPriority(eGrandStrategy), vModifiedGrandStrategyPriorities[eGrandStrategy]);
			}
			strOutBuf = strBaseString + strTemp;
			pLog->Msg(strOutBuf);
		}
	}
}

/// Log our guess as to other Players' Active Grand Strategy
void CvGrandStrategyAI::LogGuessOtherPlayerGrandStrategy(const vector<int>& vGrandStrategyPriorities, PlayerTypes ePlayer)
{
	if(GC.getLogging() && GC.getAILogging())
	{
		CvString strOutBuf;
		CvString strBaseString;
		CvString strTemp;
		CvString playerName;
		CvString otherPlayerName;
		CvString strDesc;
		CvString strLogName;

		// Find the name of this civ and city
		playerName = GetPlayer()->getCivilizationShortDescription();

		// Open the log file
		if(GC.getPlayerAndCityAILogSplit())
		{
			strLogName = "GrandStrategyAI_Guess_Log_" + playerName + ".csv";
		}
		else
		{
			strLogName = "GrandStrategyAI_Guess_Log.csv";
		}

		FILogFile* pLog = NULL;
		pLog = LOGFILEMGR.GetLog(strLogName, FILogFile::kDontTimeStamp);

		AIGrandStrategyTypes eGrandStrategy;
		int iPriority = 0;

		// Loop through Grand Strategies
		for(int iGrandStrategyLoop = 0; iGrandStrategyLoop < GC.getNumAIGrandStrategyInfos(); iGrandStrategyLoop++)
		{
			// Get the leading info for this line
			strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
			strBaseString += playerName + ", ";

			eGrandStrategy = (AIGrandStrategyTypes) iGrandStrategyLoop;
			iPriority = vGrandStrategyPriorities[iGrandStrategyLoop];

			CvAIGrandStrategyXMLEntry* pEntry = GC.getAIGrandStrategyInfo(eGrandStrategy);
			const char* szGrandStrategyType = (pEntry != NULL)? pEntry->GetType() : "Unknown Strategy";

			// GrandStrategy Info
			if(GetActiveGrandStrategy() == eGrandStrategy)
			{
				strTemp.Format("*** %s, %d", szGrandStrategyType, iPriority);
			}
			else
			{
				strTemp.Format("%s, %d", szGrandStrategyType, iPriority);
			}
			otherPlayerName = GET_PLAYER(ePlayer).getCivilizationShortDescription();
			strOutBuf = strBaseString + otherPlayerName + ", " + strTemp;

			if(GetGuessOtherPlayerActiveGrandStrategy(ePlayer) == eGrandStrategy)
			{
				// Confidence in our guess
				switch(GetGuessOtherPlayerActiveGrandStrategyConfidence(ePlayer))
				{
				case GUESS_CONFIDENCE_POSITIVE:
					strTemp.Format("Positive");
					break;
				case GUESS_CONFIDENCE_LIKELY:
					strTemp.Format("Likely");
					break;
				case GUESS_CONFIDENCE_UNSURE:
					strTemp.Format("Unsure");
					break;
				}

				strOutBuf += ", " + strTemp;
			}

			pLog->Msg(strOutBuf);
		}

		// One more entry for NO GRAND STRATEGY
		// Get the leading info for this line
		strBaseString.Format("%03d, ", GC.getGame().getElapsedGameTurns());
		strBaseString += playerName + ", ";

		iPriority = vGrandStrategyPriorities[GC.getNumAIGrandStrategyInfos()];

		// GrandStrategy Info
		strTemp.Format("NO_GRAND_STRATEGY, %d", iPriority);
		otherPlayerName = GET_PLAYER(ePlayer).getCivilizationShortDescription();
		strOutBuf = strBaseString + otherPlayerName + ", " + strTemp;
		pLog->Msg(strOutBuf);
	}
}