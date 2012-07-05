#ifndef _miscimd_h
#define _miscimd_h

extern BOOL initMiscImds( void );
extern iIMDShape	*getImdFromIndex(UDWORD	index);
extern iIMDShape	*getRandomWreckageImd( void );
extern iIMDShape	*getRandomDebrisImd( void );

#define	MAX_DEBRIS		5
#define	MAX_WRECKAGE	5

extern iIMDShape	*explosionSmallImd;	// Set this up to point to the explosion imd
extern iIMDShape	*explosionMediumImd;	// Set this up to point to the explosion imd
extern iIMDShape	*constructionImd;
extern iIMDShape	*smallSmokeImd;
extern iIMDShape	*babaHeadImd;
extern iIMDShape	*babaBodyImd;
extern iIMDShape	*babaLegsImd;
extern iIMDShape	*babaArmImd;
extern iIMDShape	*cyborgHeadImd;
extern iIMDShape	*cyborgBodyImd;
extern iIMDShape	*cyborgLegsImd;
extern iIMDShape	*cyborgArmImd;
extern iIMDShape	*waterImd;
extern iIMDShape	*droidDamageImd;
extern iIMDShape	*smallSteamImd;
extern iIMDShape	*plasmaImd;
//extern iIMDShape	*pAssemblyPointIMDs[NUM_FACTORY_TYPES][MAX_FACTORY];
extern iIMDShape	*pAssemblyPointIMDs[NUM_FLAG_TYPES][MAX_FACTORY];
extern iIMDShape	*blipImd;
extern iIMDShape	*shadowImd;
extern iIMDShape	*transporterShadowImd;
extern iIMDShape	*bloodImd;
extern iIMDShape	*trailImd;
extern iIMDShape	*cameraImd;
extern iIMDShape	*debrisImds[MAX_DEBRIS];
extern iIMDShape	*flameImd;
extern iIMDShape	*wreckageImds[MAX_WRECKAGE];
extern iIMDShape	*proximityImds[PROX_TYPES];
extern iIMDShape	*teslaImd;
extern iIMDShape	*mFlareImd;
extern iIMDShape	*snowImd;
extern iIMDShape	*rainImd;
extern iIMDShape	*splashImd;
extern iIMDShape	*kickImd;
extern iIMDShape	*landingImd;
extern iIMDShape	*shockImd;

/* An imd entry */
typedef struct	_misc_imd
{
iIMDShape	*pImd;
STRING		*pName;
} MISC_IMD;


enum {
MI_EXPLOSION_SMALL,       
MI_EXPLOSION_MEDIUM,     
MI_CONSTRUCTION,          
MI_SMALL_SMOKE,           
MI_BABA_HEAD,             
MI_BABA_LEGS,             
MI_BABA_ARM,             
MI_BABA_BODY,             
MI_CYBORG_HEAD,           
MI_CYBORG_LEGS,           
MI_CYBORG_ARM,            
MI_CYBORG_BODY,           
MI_WATER,                 
MI_DROID_DAMAGE,          
MI_SMALL_STEAM,           
MI_PLASMA,                
MI_BLIP,                  
MI_SHADOW,                
MI_TRANSPORTER_SHADOW,    
MI_BLOOD,                 
MI_TRAIL,                 
MI_FLAME,                 
MI_TESLA,                 
MI_MFLARE,                
MI_RAIN,                  
MI_SNOW,                  
MI_SPLASH,                
MI_KICK,                  
MI_LANDING,               
MI_SHOCK,
MI_BLIP_ENEMY,    
MI_BLIP_RESOURCE, 
MI_BLIP_ARTEFACT,
#ifdef WIN32
MI_WRECK0,
MI_WRECK1, 
MI_WRECK2, 
MI_WRECK3, 
MI_WRECK4, 
#endif
MI_DEBRIS0,  
MI_DEBRIS1,  
MI_DEBRIS2,  
MI_DEBRIS3,  
MI_DEBRIS4,  
#ifdef WIN32
MI_FIREWORK,
#endif
#ifdef PSX
MI_MODULE,  
#endif
MI_TOO_MANY  
};

 #endif
               