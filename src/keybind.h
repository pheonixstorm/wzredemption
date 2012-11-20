#ifndef _keybind_h
#define _keybind_h

// --------------- All those keyboard mappable functions */
extern void	kf_HalveHeights( void );
extern void	kf_DebugDroidInfo( void );
extern void	kf_BuildInfo( void );
extern void	kf_UpdateWindow( void );
extern void	kf_FrameRate( void );
extern void kf_ShowNumObjects( void );
extern void	kf_ToggleRadar( void );
extern void	kf_ToggleOutline( void );
extern void	kf_TogglePower( void );
extern void	kf_RecalcLighting( void );
extern void	kf_RaiseGamma( void );
extern void	kf_LowerGamma( void );
extern void	kf_ScreenDump( void );
extern void	kf_AllAvailable( void );
extern void	kf_TriFlip( void );
extern void	kf_ToggleWidgets( void );
extern void	kf_ToggleBackgroundFog( void );
extern void	kf_ToggleDistanceFog( void );
extern void	kf_ToggleMistFog( void );
extern void	kf_ToggleFogColour( void );
extern void	kf_ToggleFog( void );
extern void	kf_ToggleCamera( void );
extern void	kf_SimCloseDown( void );
extern void	kf_ToggleGouraud( void );
extern void kf_RaiseTile( void );
extern void kf_LowerTile( void );
extern void	kf_SystemClose( void );
extern void kf_ZoomOut( void );
extern void kf_ZoomIn( void );
extern void kf_ShrinkScreen( void );
extern void kf_ExpandScreen( void );
extern void kf_RotateLeft( void );
extern void kf_RotateRight( void );
extern void kf_PitchBack( void );
extern void kf_PitchForward( void );
extern void kf_ResetPitch( void );
extern void	kf_ToggleDimension( void );
extern void	kf_ShowMappings( void );
extern void	kf_SelectGrouping( void );
extern void	kf_AssignGrouping( void );
extern void	kf_SelectMoveGrouping( void );
extern void	kf_ToggleDroidInfo( void );
extern void	kf_addInGameOptions( void );
extern void	kf_AddMissionOffWorld( void );
extern void	kf_EndMissionOffWorld( void );
extern void	kf_NewPlayerPower( void );
extern void kf_addMultiMenu( void );
extern void	kf_multiAudioStart(void);
extern void	kf_multiAudioStop(void);
extern void	kf_JumpToMapMarker( void );
extern void	kf_TogglePowerBar( void );
extern void	kf_UpGeoOffset( void );
extern void	kf_DownGeoOffset( void );
extern void	kf_UpDroidScale( void );
extern void	kf_DownDroidScale( void );
extern void	kf_ToggleDebugMappings( void );
extern void	kf_ToggleGodMode( void );
extern void	kf_SeekNorth( void );
extern void kf_MaxScrollLimits( void );
extern void	kf_LevelSea( void );
extern void	kf_TestWater( void );
extern void	kf_TogglePauseMode( void );
extern void	kf_ToggleRadarAllign( void );

extern void	kf_ToggleEnergyBars( void );
extern void	kf_ToggleReloadBars( void );
extern void	kf_FinishResearch( void );
extern void	kf_ToggleOverlays( void );
extern void	kf_ChooseOptions( void );
extern void	kf_ChooseCommand( void );
extern void	kf_ChooseManufacture( void );
extern void	kf_ChooseResearch( void );
extern void	kf_ChooseBuild( void );
extern void	kf_ChooseDesign( void );
extern void	kf_ChooseIntelligence( void );
extern void	kf_ChooseCancel( void );
extern void	kf_ToggleWeather( void );
extern void kf_KillSelected(void);
extern void kf_ShowGridInfo(void);
extern void kf_GiveTemplateSet(void);
extern void kf_SendTextMessage(void);
extern void	kf_SelectPlayer( void );
extern void	kf_ToggleDrivingMode( void );
extern void	kf_ToggleConsole( void );
extern void	kf_SelectAllOnScreenUnits( void );
extern void	kf_SelectAllUnits( void );
extern void	kf_SelectAllVTOLs( void );
extern void	kf_SelectAllHovers( void );
extern void	kf_SelectAllWheeled( void );
extern void	kf_SelectAllTracked( void );
extern void	kf_SelectAllHalfTracked( void );
extern void	kf_SelectAllCombatUnits( void );
extern void	kf_SelectAllSameType( void );

extern void	kf_SetDroidRangeShort( void );
extern void	kf_SetDroidRangeDefault( void );
extern void	kf_SetDroidRangeLong( void );

extern void	kf_SetDroidRetreatMedium( void );
extern void	kf_SetDroidRetreatHeavy( void );
extern void	kf_SetDroidRetreatNever( void );

extern void	kf_SetDroidAttackAtWill( void );
extern void	kf_SetDroidAttackReturn( void );
extern void	kf_SetDroidAttackCease( void );

extern void	kf_SetDroidMoveHold( void );
extern void	kf_SetDroidMovePursue( void ); //not there?
extern void	kf_SetDroidMovePatrol( void ); // not there?

extern void	kf_SetDroidReturnToBase( void );
extern void	kf_SetDroidGoForRepair( void );
extern void	kf_SetDroidRecycle( void );
extern void	kf_ScatterDroids( void );
extern void	kf_CentreOnBase( void );
extern void kf_ToggleFog( void );
extern void	kf_MoveToLastMessagePos( void );
extern void	kf_SelectAllDamaged( void );
extern void	kf_RightOrderMenu( void );

extern BOOL	bAllowOtherKeyPresses;

extern void	kf_TriggerRayCast( void );
extern void kf_ToggleFormationSpeedLimiting( void );
//extern void	kf_ToggleSensorDisplay( void );
extern void kf_SensorDisplayOn( void );
extern void kf_SensorDisplayOff( void );
extern void	kf_JumpToResourceExtractor( void );
extern void	kf_JumpToRepairUnits( void );
extern void	kf_JumpToConstructorUnits( void );
extern void	kf_JumpToCommandUnits( void );
extern void	kf_JumpToSensorUnits( void );

extern void	kf_JumpToUnassignedUnits( void );
extern void kf_ScriptTest( void );
extern void kf_TriggerShockWave( void );
extern void	kf_ToggleVisibility( void );
extern void kf_RadarZoomIn( void );
extern void kf_RadarZoomOut( void );
extern	void	kf_SelectNextFactory(void);
extern	void	kf_SelectNextCyborgFactory(void);
extern	void	kf_SelectNextPowerStation(void);
extern	void	kf_SelectNextResearch(void);
extern	void	kf_ToggleConsoleDrop( void );
extern	void	kf_ToggleShakeStatus( void );
extern	void	kf_ToggleMouseInvert( void );
extern	void	kf_SetKillerLevel( void );
extern	void	kf_SetEasyLevel( void );
extern	void	kf_SetNormalLevel( void );
extern	void	kf_SetToughUnitsLevel( void );
extern  void	kf_UpThePower( void );
extern  void    kf_MaxPower( void );
extern	void	kf_KillEnemy( void );
extern void		kf_ToggleMissionTimer( void );


extern	void	kf_SetHardLevel( void );
extern	void	kf_NoFaults( void );
extern void	kf_SelectCommander( void );
void kf_ToggleReopenBuildMenu( void );

// dirty but necessary
extern	STRING	sTextToSend[MAX_CONSOLE_STRING_LENGTH];	
extern	void	kf_FaceNorth(void);
extern	void	kf_FaceSouth(void);
extern	void	kf_FaceEast(void);
extern	void	kf_FaceWest(void);
extern	void	kf_ToggleRadarJump( void );
extern	void	kf_MovePause( void );

extern void kf_SetDoubleUnitsLevel( void );
extern void kf_SetBakerLevel( void );

void kf_SpeedUp( void );
void kf_SlowDown( void );
void kf_NormalSpeed( void );

#define SPIN_SCALING	(360*DEG_1)
#define	SECS_PER_SPIN	2
#define MAP_SPIN_RATE	(SPIN_SCALING/SECS_PER_SPIN)


extern int fogCol;

#endif
