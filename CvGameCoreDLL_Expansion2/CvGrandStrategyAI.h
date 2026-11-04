/*	-------------------------------------------------------------------------------------------------------
	© 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#pragma once

#ifndef CIV5_GRAND_STRATEGY_AI_H
#define CIV5_GRAND_STRATEGY_AI_H

#include "CvDiplomacyAI.h"

// AI_AB_TEST (Personality): the strategy profile (pack repo tools/ab/RESULTS.md, "The strategy profile: spec").
// A flavour archetype (a distinctive way to play) and a position on the meta spectrum from greedy (-100) through
// balanced (0) to aggressive (+100), plus victory weights, risk appetite and expansion style. Worked out from game
// data only (traits, leader personality, CP's victory pursuit pick), so every computer gets the same profile.
enum StrategyArchetypeTypes
{
	NO_STRATEGY_ARCHETYPE = -1,
	STRATEGY_ARCHETYPE_CONQUEROR,		// repeated wars all game
	STRATEGY_ARCHETYPE_OPPORTUNIST,		// builds, joins wars against weakened, isolated or runaway neighbours
	STRATEGY_ARCHETYPE_EXPANSIONIST,	// settles every free spot, fights only whoever blocks its land
	STRATEGY_ARCHETYPE_ZEALOT,			// founds early, spreads its faith everywhere
	STRATEGY_ARCHETYPE_DIPLOMAT,		// city-states, World Congress, organises coalitions
	STRATEGY_ARCHETYPE_TOURISM,			// wonders, great works, tourism
	STRATEGY_ARCHETYPE_SCIENTIST,		// tall science
	STRATEGY_ARCHETYPE_RAIDER,			// wars for loot: pillage, plunder, raze; doesn't need to hold cities (the user, 2026-11-04)
	NUM_STRATEGY_ARCHETYPES
};
enum StrategyVictoryWeightTypes
{
	STRATEGY_VICTORY_DOMINATION,
	STRATEGY_VICTORY_SCIENCE,
	STRATEGY_VICTORY_CULTURE,
	STRATEGY_VICTORY_DIPLOMACY,
	NUM_STRATEGY_VICTORY_WEIGHTS
};
enum StrategyRiskTypes
{
	STRATEGY_RISK_LOW,
	STRATEGY_RISK_MEDIUM,
	STRATEGY_RISK_HIGH
};
enum StrategyExpansionTypes
{
	STRATEGY_EXPANSION_TALL,
	STRATEGY_EXPANSION_MEDIUM,
	STRATEGY_EXPANSION_WIDE,
	STRATEGY_EXPANSION_CONQUEST
};
// AI_AB_TEST (Personality, GreedyWide/GreedyTall): the two kinds of greedy (the user, 2026-11-04)
enum StrategyGreedyKindTypes
{
	STRATEGY_GREEDY_TALL,	// Tradition, 3-4 cities, a tech and policy rush
	STRATEGY_GREEDY_WIDE,	// Liberty, settles as fast as happiness and growth allow; out of room, grows tall and trades
	NUM_STRATEGY_GREEDY_KINDS
};
struct CvStrategyProfile
{
	StrategyArchetypeTypes eArchetype;
	int iHomeMeta;	// -100 greedy, 0 balanced, +100 aggressive
	int aiVictoryWeight[NUM_STRATEGY_VICTORY_WEIGHTS];	// adds up to 100
	StrategyRiskTypes eRisk;
	StrategyExpansionTypes eExpansion;
	StrategyGreedyKindTypes eGreedyKind;	// which kind of greedy this leader plays when it plays greedy
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvGrandStrategyAI
//!  \brief		Information about the Grand Strategy of a single AI player
//
//!  Author:	Jon Shafer
//
//!  Key Attributes:
//!  - Object created by CvPlayer
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvGrandStrategyAI
{
public:
	CvGrandStrategyAI(void);
	~CvGrandStrategyAI(void);
	void Init(CvAIGrandStrategyXMLEntries* pAIGrandStrategies, CvPlayer* pPlayer);
	void Uninit();
	void Reset();
	template<typename GrandStrategyAI, typename Visitor>
	static void Serialize(GrandStrategyAI& grandStrategyAI, Visitor& visitor);
	void Read(FDataStream& kStream);
	void Write(FDataStream& kStream) const;

	CvPlayer* GetPlayer();
	CvAIGrandStrategyXMLEntries* GetAIGrandStrategies();

	void DoTurn();

	int GetConquestPriority();
	int GetCulturePriority();
	int GetUnitedNationsPriority();
	int GetSpaceshipPriority();

	int GetBaseGrandStrategyPriority(AIGrandStrategyTypes eGrandStrategy);

	AIGrandStrategyTypes GetActiveGrandStrategy() const;
	void SetActiveGrandStrategy(AIGrandStrategyTypes eGrandStrategy);
	int GetNumTurnsSinceActiveSet() const;
	void SetNumTurnsSinceActiveSet(int iValue);
	void ChangeNumTurnsSinceActiveSet(int iChange);

	int GetGrandStrategyPriority(AIGrandStrategyTypes eGrandStrategy) const;
	void SetGrandStrategyPriority(AIGrandStrategyTypes eGrandStrategy, int iValue);
	void ChangeGrandStrategyPriority(AIGrandStrategyTypes eGrandStrategy, int iChange);
	int GetPersonalityAndGrandStrategy(FlavorTypes eFlavorType, bool bBoostGSMainFlavor = false);

	// AI_AB_TEST (Personality): the strategy profile; nothing reads it yet (logged only)
	CvStrategyProfile GetStrategyProfile() const;
	static const char* GetStrategyArchetypeName(StrategyArchetypeTypes eArchetype);
	int GetMetaPosition() const;
	StrategyGreedyKindTypes GetGreedyKind() const;
	bool IsGreedyWideOutOfRoom() const;
	int GetWarDefenceLandUnitCap() const;

	// **********
	// Stuff relating to guessing what other Players are up to
	// **********

	void DoGuessOtherPlayersActiveGrandStrategy();

	AIGrandStrategyTypes GetGuessOtherPlayerActiveGrandStrategy(PlayerTypes ePlayer) const;
	GuessConfidenceTypes GetGuessOtherPlayerActiveGrandStrategyConfidence(PlayerTypes ePlayer) const;
	void SetGuessOtherPlayerActiveGrandStrategy(PlayerTypes ePlayer, AIGrandStrategyTypes eGrandStrategy, GuessConfidenceTypes eGuessConfidence);
	bool OtherPlayerDoingBetterThanUs(PlayerTypes ePlayer, AIGrandStrategyTypes eGrandStrategy);

	int GetGuessOtherPlayerConquestPriority(PlayerTypes ePlayer, int iWorldMilitaryAverage);
	int GetGuessOtherPlayerCulturePriority(PlayerTypes ePlayer, int iWorldCultureAverage, int iWorldTourismAverage);
	int GetGuessOtherPlayerUnitedNationsPriority(PlayerTypes ePlayer);
	int GetGuessOtherPlayerSpaceshipPriority(PlayerTypes ePlayer, int iWorldNumTechsAverage);

	int GetGoldFlavor() const { return m_iFlavorGold; }
	int GetScienceFlavor() const { return m_iFlavorScience; }
	int GetCultureFlavor() const { return m_iFlavorCulture; }
	int GetProductionFlavor() const { return m_iFlavorProduction; }
	int GetFaithFlavor() const { return m_iFlavorFaith; }
	int GetHappinessFlavor() const { return m_iFlavorHappiness; }
	int GetGrowthFlavor() const { return m_iFlavorGrowth; }
	int GetDiploFlavor() const { return m_iFlavorDiplomacy; }

private:

	void LogGrandStrategies(const vector<int>& vGrandStrategyPriorities);
	void LogGuessOtherPlayerGrandStrategy(const vector<int>& vGrandStrategyPriorities, PlayerTypes ePlayer);
	void LogStrategyProfile();
	void DoStyleCityStateUnitGifts();
	void DoGreedyDisbandToCap();

	CvPlayer* m_pPlayer;
	CvAIGrandStrategyXMLEntries* m_pAIGrandStrategies;

	int m_iNumTurnsSinceActiveSet;

	AIGrandStrategyTypes m_eActiveGrandStrategy;

	int* m_paiGrandStrategyPriority;

	// **********
	// Stuff relating to guessing what other Players are up to
	// **********

	int m_eGuessOtherPlayerActiveGrandStrategy[MAX_MAJOR_CIVS];
	int m_eGuessOtherPlayerActiveGrandStrategyConfidence[MAX_MAJOR_CIVS];

	// caching for faster accesss without a hash lookup
	int m_iFlavorGold;
	int m_iFlavorScience;
	int m_iFlavorCulture;
	int m_iFlavorProduction;
	int m_iFlavorFaith;
	int m_iFlavorHappiness;
	int m_iFlavorGrowth;
	int m_iFlavorDiplomacy;

	// AI_AB_TEST (Personality, MetaDial): not saved, worked out again from the game data
	mutable int m_iCachedMetaPosition;
	mutable int m_iCachedMetaTurn;
	mutable int m_iCachedGreedyKind;	// GreedyWide/GreedyTall: worked out with the meta position, once per turn
	mutable int m_iCachedOutOfRoom;	// GreedyWide: -1 not worked out this turn, else 0 or 1
	mutable int m_iCachedOutOfRoomTurn;
	// AI_AB_TEST (Personality, GreedyWarArmy): not saved, worked out again from the game state
	mutable int m_iCachedWarDefenceCap;
	mutable int m_iCachedWarDefenceTurn;
};

FDataStream& operator>>(FDataStream&, CvGrandStrategyAI&);
FDataStream& operator<<(FDataStream&, const CvGrandStrategyAI&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvAIGrandStrategyXMLEntry
//!  \brief		A single entry in the AI Grand Strategy XML file
//
//!  Key Attributes:
//!  - Populated from XML\???? (not sure what path/name you want)
//!  - Array of these contained in CvAIGrandStrategyXMLEntries class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvAIGrandStrategyXMLEntry: public CvBaseInfo
{
public:
	CvAIGrandStrategyXMLEntry();
	virtual ~CvAIGrandStrategyXMLEntry();

	virtual bool CacheResults(Database::Results& kResults, CvDatabaseUtility& kUtility);

	// Accessor Functions
	int GetFlavorValue(int i) const;
	int GetFlavorModValue(int i) const;
	int GetSpecializationBoost(YieldTypes eYield) const;

private:
	int* m_piFlavorValue;
	int* m_piSpecializationBoost;
	int* m_piFlavorModValue;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CLASS:      CvAIGrandStrategyXMLEntries
//!  \brief		Game-wide information about possible AI strategies
//
//! Key Attributes:
//! - Plan is it will be contained in CvGameRules object within CvGame class
//! - Populated from XML\???? (not sure what path/name you want)
//! - Contains an array of CvAIGrandStrategyXMLEntry from the above XML file
//! - One instance for the entire game
//! - Accessed heavily by CvPlayerAIStrategy class (which stores the AI strategy state for 1 player)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class CvAIGrandStrategyXMLEntries
{
public:
	CvAIGrandStrategyXMLEntries(void);
	~CvAIGrandStrategyXMLEntries(void);

	// Accessor functions
	std::vector<CvAIGrandStrategyXMLEntry*>& GetAIGrandStrategyEntries();
	int GetNumAIGrandStrategies();
	CvAIGrandStrategyXMLEntry* GetEntry(int index);

	void DeleteArray();

private:
	std::vector<CvAIGrandStrategyXMLEntry*> m_paAIGrandStrategyEntries;
};

#endif //CIV5_GRAND_STRATEGY_AI_H