// MQ2GMCheck.cpp : Check for GM's in the zone
//
// Author: Bushdaka - but I'll deny it if a GM asks me
//
// Version:
// 2.2 - changed the timing components since they weren't working anyway, fixed DoCommand error
// 2.1 - added popup to the alert system, separated out popup, chat channel, and audio alerts
//     - added ability to set timers for each type of alert
// 2.0 - added audio alert - in your MQ2Sound INI make an entry for GMCHECK
// 1.9 - code updated to stop crashes where the plugin is loaded before entering the game
//     - changed the timing to use OnPulse rather than the /dotimer component
//     - increased the default scanning for GM's from 5s to 2s due to the increase of GM's popping into zones
//     - new /gmcheck timer [seconds] command - /gmcheck timer 5 will check for GM's every 5s
// 1.8 - adjusted for the new server definitions and struct changes
// 1.7 - new commands, plus a fix for the times where a GM is found while zoning
// 1.6 - new history methods, much better
// 1.5 - new scanning routine, store names and ID, new options, fix while loop
// 1.4 - updated with a 5s delay for checking the GM condition
// 1.3 - updated with suggestions from CheckinThingsOut, HTW, and Cronic
// 1.2 - updated with suggestions from CheckinThingsOut and HTW
// 1.1 - updated to allow toggling, added DISABLED to status
// 1.0 - original release

// Check to see if a GM is in the zone. This is not fool proof. It is absolutely
// true that a GM could be right in front of you and you'd never know it. This
// plugin will simply find those who are in the zone and not gm-invis, or who
// just came into the zone and were not gm-invised at the time. If a GM comes
// into the zone already gm-invised, we will not know about that.

#include <mq/Plugin.h>
#include "mmsystem.h"

#include <ctime>

PreSetup("MQ2GMCheck");
PLUGIN_VERSION(2.2);

constexpr auto GMLIMIT = 10;

struct GMCHECK {
   char Name[MAX_STRING];
   int ID;
   uint64_t Timer;
};
GMCHECK GMSpawns[GMLIMIT];

bool bGMInTheZone=false;
bool bCheck4GM=true;
bool bCheck4GMOldState=true;
bool bAlertPopup=true;
bool bAlertChat=true;
bool bAlertAudio=true;
unsigned int gAlertPopupDelay=30000;
unsigned int gAlertChatDelay=15000;
unsigned int gAlertAudioDelay=30000;
uint64_t gAlertPopupTimer=0;
uint64_t gAlertChatTimer=0;
uint64_t gAlertAudioTimer=0;
uint64_t gPulseTimer=0;
unsigned int gPulseDelay=2000;

char GMSound[MAX_STRING]; // GM Sound File

int NumGMOnList();

class MQ2GMCheckType *pGMCheckType = 0;

class MQ2GMCheckType : public MQ2Type
{
public:
   enum GMCheckMembers {
      Status=1,
      Count=2,
   };

   MQ2GMCheckType():MQ2Type("GMCheck")
   {
      TypeMember(Status);
      TypeMember(Count);
   }

   ~MQ2GMCheckType()
   {
   }

   bool GetMember(MQVarPtr VarPtr, PCHAR Member, PCHAR Index, MQTypeVar &Dest)
   {
      MQTypeMember* pMember=MQ2GMCheckType::FindMember(Member);
      if (!pMember)
         return false;
      switch((GMCheckMembers)pMember->ID)
      {
         case Status:
            strcpy_s(DataTypeTemp,"FALSE");
            if (bGMInTheZone)
               strcpy_s(DataTypeTemp,"TRUE");
            if (!bCheck4GM)
               strcpy_s(DataTypeTemp,"DISABLED");
            Dest.Ptr=DataTypeTemp;
            Dest.Type=mq::datatypes::pStringType;
            return true;
         case Count:
            sprintf_s(DataTypeTemp,"%d",NumGMOnList());
            Dest.Ptr=DataTypeTemp;
            Dest.Type=mq::datatypes::pStringType;
            return true;
      }
      return false;
   }

   bool ToString(MQVarPtr VarPtr, PCHAR Destination)
   {
      strcpy_s(Destination,MAX_STRING, "FALSE");
      if (bGMInTheZone)
         strcpy_s(Destination, MAX_STRING, "TRUE");
      if (!bCheck4GM)
         strcpy_s(Destination, MAX_STRING, "DISABLED");
      return true;
   }

   bool FromData(MQVarPtr &VarPtr, MQTypeVar &Source)
   {
      return false;
   }
   bool FromString(MQVarPtr &VarPtr, PCHAR Source)
   {
      return false;
   }
};

bool dataGMCheck(const char* szName, MQTypeVar &Ret)
{
   Ret.DWord=1;
   Ret.Type=pGMCheckType;
   return true;
}

bool G_PluginLoaded(PCHAR szPlugin)
{
   // Check for plugins
   MQPlugin* pPlugin=pPlugins;
   while(pPlugin)
   {
      if (!_stricmp(pPlugin->szFilename,szPlugin)) return true;
      pPlugin=pPlugin->pNext;
   }
   return false;
}

void TrackGMs(PCHAR GMName)
{
   char szSection[MAX_STRING] = {0};
   char szTemp[MAX_STRING] = {0};
   int iCount = 0;
   char szLookup[MAX_STRING] = {0};
   char szTime[MAX_STRING] = {0};
   errno_t err;

   time_t rawtime;
   struct tm timeinfo = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   time(&rawtime);
   err = localtime_s(&timeinfo, &rawtime);
   //strncpy(szTime,asctime(timeinfo),24);
   err = asctime_s(szTime, MAX_STRING, &timeinfo);


   // Store GM count by Server-Zone
   sprintf_s(szSection,"%s-%s",EQADDR_SERVERNAME,((PZONEINFO)pZoneInfo)->LongName);
   sprintf_s(szLookup,"%s",GMName);
   iCount = GetPrivateProfileInt(szSection,szLookup,0,INIFileName) + 1;
   sprintf_s(szTemp,"%d,%s",iCount,szTime);
   WritePrivateProfileString(szSection,szLookup,szTemp,INIFileName);

   // Store total GM count regardless of server
   strcpy_s(szSection,"GM");
   sprintf_s(szLookup,"%s",GMName);
   iCount = GetPrivateProfileInt(szSection,szLookup,0,INIFileName) + 1;
   sprintf_s(szTemp,"%d,%s,%s",iCount,EQADDR_SERVERNAME,szTime);
   WritePrivateProfileString(szSection,szLookup,szTemp,INIFileName);

   // Store GM count by Server
   sprintf_s(szSection,"%s",EQADDR_SERVERNAME);
   sprintf_s(szLookup,"%s",GMName);
   iCount = GetPrivateProfileInt(szSection,szLookup,0,INIFileName) + 1;
   sprintf_s(szTemp,"%d,%s",iCount,szTime);
   WritePrivateProfileString(szSection,szLookup,szTemp,INIFileName);

   return;   
}

void ResetGMArray()
{
   bGMInTheZone=false;
   for (int x=0;x<GMLIMIT;x++) {
      GMSpawns[x].ID = 0;
      strcpy_s(GMSpawns[x].Name,"\0");
      GMSpawns[x].Timer = 0;
   }
   WriteChatf("\arGMCHECK: Resetting GM array!\ax");
   return;
}

bool GMAlreadyInList(int GMID)
{
   for (int x=0;x<GMLIMIT;x++)
      if (GMSpawns[x].ID==GMID)
         return true;
   return false;
}

int NumGMOnList()
{
   int iNum=0;
   for (int x=0;x<GMLIMIT;x++)
      if (GMSpawns[x].ID!=0) iNum++;
   return iNum;
}

void AddGMToList(int GMID,PCHAR GMName)
{
   if (!GMAlreadyInList(GMID)) {
      for (int x=0;x<GMLIMIT;x++)
         if (GMSpawns[x].ID==0) {
            GMSpawns[x].ID=GMID;
            strcpy_s(GMSpawns[x].Name,GMName);
            GMSpawns[x].Timer=GetTickCount64();
            TrackGMs(GMName);
            break;
         }
      WriteChatf("\arGMCHECK: GM in the zone: %s (%d)\ax", GMName, GMID);
      bGMInTheZone=true;
   }
   return;
}

void RemoveGMFromList(int GMID)
{
   if (!GMAlreadyInList(GMID)) return;
   for (int x=0;x<GMLIMIT;x++)
      if (GMSpawns[x].ID==GMID) {
         ULONGLONG c,h,m,s,t;
         c=GetTickCount64();
         t=(c-GMSpawns[x].Timer)/1000;
         h=m=s=0;
         if (t>3600) h=(t/3600);
         t=t-(h*3600);
         if (t>60) m=(t/60);
         t=t-(m*60);
         if (t<0) t=0;
         s=t;
         WriteChatf("\arGMCHECK: GM has left the zone: %s (%d) after %dh:%dm:%ds\ax", GMSpawns[x].Name, GMSpawns[x].ID,h,m,s);
         GMSpawns[x].ID=0;
         strcpy_s(GMSpawns[x].Name,"\0");
         GMSpawns[x].Timer=0;
      }
   if (NumGMOnList())
      bGMInTheZone=true;
   else
      bGMInTheZone=false;
   return;
}

void GMScanner()
{
   PSPAWNINFO pSpawn = (PSPAWNINFO)pSpawnList;
   while (pSpawn) {
      if (pSpawn->GM)
         AddGMToList(pSpawn->SpawnID,pSpawn->DisplayedName);
      pSpawn = pSpawn->pNext;
   }
   return;
}

bool Toggle(PCHAR szOnOff)
{
   bool oldGMCheck = false;
   oldGMCheck=bCheck4GM;
   if (!_stricmp(szOnOff,"ON"))
      bCheck4GM=true;
   if (!_stricmp(szOnOff,"OFF"))
      bCheck4GM=false;
   if (oldGMCheck!=bCheck4GM) {
      WriteChatf("\arGMCHECK: Checking for GM's is now %s!\ax", bCheck4GM ? "on" : "off");
      ResetGMArray();
      return true;
   }
   return false;
}

void HistoryGMAllServers()
{
   char szSection[MAX_STRING] = {0};
   char szTemp[MAX_STRING] = {0};
   char szServer[MAX_STRING] = {0};
   char szTime[MAX_STRING] = {0};
   char szCount[MAX_STRING] = {0};
   char szKeys[MAX_STRING*25] = {0};
   char *pch, *next_pch;
   int count = 0;

   // What GM's have been seen on all servers?
   WriteChatf("\arGMCHECK: History of GM's on ALL servers\ax");
   strcpy_s(szSection,"GM");
   GetPrivateProfileString(szSection,NULL,"",szKeys,MAX_STRING*25,INIFileName);
    PCHAR pKeys = szKeys;
    while (pKeys[0]!=0) {
        GetPrivateProfileString(szSection,pKeys,"",szTemp,MAX_STRING,INIFileName);
      if ((strstr(szTemp,",")!=NULL) && (szTemp[0]!=0)) {
         count++;
         pch=strtok_s(szTemp,",",&next_pch);
         strcpy_s(szCount,pch);
         pch=strtok_s(NULL,",", &next_pch);
         strcpy_s(szServer,pch);
         pch=strtok_s(NULL,",", &next_pch);
         strcpy_s(szTime,pch);
         WriteChatf("\arGM %s - seen %s times on server %s, last seen %s\ax",pKeys,szCount,szServer,szTime);
        }
        pKeys+=strlen(pKeys)+1;
    }
   if (!count) WriteChatf("\arNo GM's seen yet!\ax");
   return;
}

void HistoryGMThisServer()
{
   char szSection[MAX_STRING] = {0};
   char szTemp[MAX_STRING] = {0};
   char szTime[MAX_STRING] = {0};
   char szCount[MAX_STRING] = {0};
   char szKeys[MAX_STRING*25] = {0};
   char *pch, *next_pch;
   int count = 0;

   // What GM's have been seen on this server?
   WriteChatf("\arGMCHECK: History of GM's on this server\ax");
   sprintf_s(szSection,"%s",EQADDR_SERVERNAME);
   GetPrivateProfileString(szSection,NULL,"",szKeys,MAX_STRING*25,INIFileName);
    PCHAR pKeys = szKeys;
    while (pKeys[0]!=0) {
        GetPrivateProfileString(szSection,pKeys,"",szTemp,MAX_STRING,INIFileName);
      if ((strstr(szTemp,",")!=NULL) && (szTemp[0]!=0)) {
         count++;
         pch=strtok_s(szTemp,",",&next_pch);
         strcpy_s(szCount,pch);
         pch=strtok_s(NULL,",", &next_pch);
         strcpy_s(szTime,pch);
         WriteChatf("\arGM %s - seen %s times on this server, last seen %s\ax",pKeys,szCount,szTime);
        }
        pKeys+=strlen(pKeys)+1;
    }
   if (!count) WriteChatf("\arNo GM's seen on this server!\ax");
   return;
}

void HistoryGMThisZone()
{
   char szSection[MAX_STRING] = {0};
   char szTemp[MAX_STRING] = {0};
   char szTime[MAX_STRING] = {0};
   char szCount[MAX_STRING] = {0};
   char szKeys[MAX_STRING*25] = {0};
   char *pch, *next_pch;
   int count = 0;

   // What GM's have been seen on this server in this zone?
   WriteChatf("\arGMCHECK: History of GM's in this zone on this server\ax");
   sprintf_s(szSection,"%s-%s",EQADDR_SERVERNAME,((PZONEINFO)pZoneInfo)->LongName);
   GetPrivateProfileString(szSection,NULL,"",szKeys,MAX_STRING*25,INIFileName);
    PCHAR pKeys = szKeys;
    while (pKeys[0]!=0) {
        GetPrivateProfileString(szSection,pKeys,"",szTemp,MAX_STRING,INIFileName);
      if ((strstr(szTemp,",")!=NULL) && (szTemp[0]!=0)) {
         count++;
         pch=strtok_s(szTemp,",",&next_pch);
         strcpy_s(szCount,pch);
         pch=strtok_s(NULL,",",&next_pch);
         strcpy_s(szTime,pch);
         WriteChatf("\arGM %s - seen %s times in this zone, last seen %s\ax",pKeys,szCount,szTime);
        }
        pKeys+=strlen(pKeys)+1;
    }
   if (!count) WriteChatf("\arNo GM's seen in this zone!\ax");
   return;
}

void HistoryGMs()
{
   HistoryGMAllServers();
   HistoryGMThisServer();
   HistoryGMThisZone();
   return;
}

void ListGMs()
{
   char szTemp[MAX_STRING] = {0};
   char szOutput[MAX_STRING] = {0};
   bool bShown = false;
   for (int x=0;x<GMLIMIT;x++)
      if (GMSpawns[x].ID!=0) {
         ULONGLONG c,h,m,s,t;
         c=GetTickCount64();
         t=(c-GMSpawns[x].Timer)/1000;
         h=m=s=0;
         if (t>3600) h=(t/3600);
         t=t-(h*3600);
         if (t>60) m=(t/60);
         t=t-(m*60);
         if (t<0) t=0;
         s=t;
         sprintf_s(szTemp,"GMCHECK: GM FOUND: %s (%d) - %lluh:%llum:%llus", GMSpawns[x].Name, GMSpawns[x].ID,h,m,s);
         sprintf_s(szOutput,"\ar%s\ax",szTemp);
         if (bAlertChat) if (gAlertChatTimer<=GetTickCount64()) WriteChatf(szOutput);
         sprintf_s(szOutput,"/popup %s",szTemp);
         if (bAlertPopup) if (gAlertPopupTimer<=GetTickCount64()) DoCommand((PSPAWNINFO)pCharSpawn,szOutput);
         bShown = true;
      }
   if (bShown) {
      if (bAlertAudio) if (gAlertAudioTimer<=GetTickCount64()) PlaySound(GMSound,0,SND_ASYNC);
   } else {
      if (bAlertChat) WriteChatf("\arGMCHECK: No known GM's in this zone at this time!\ax");
      if (bAlertPopup) DoCommand((PSPAWNINFO)pCharSpawn,"/popup GMCHECK: No known GM's in this zone at this time!");
   }
   if (bAlertChat) if (gAlertChatTimer<=GetTickCount64()) gAlertChatTimer=GetTickCount64()+gAlertChatDelay;
   if (bAlertPopup) if (gAlertPopupTimer<=GetTickCount64()) gAlertPopupTimer=GetTickCount64()+gAlertPopupDelay;
   if (bAlertAudio) if (gAlertAudioTimer<=GetTickCount64()) gAlertAudioTimer=GetTickCount64()+gAlertAudioDelay;
   return;
}

void ScanGMs()
{
   if (bCheck4GM)
      GMScanner();
   return;
}

void Help()
{
   WriteChatColor("MQ2GMCheck 2.2 - Check/Alert/History of GM's",USERCOLOR_DEFAULT);
   WriteChatColor(" ",USERCOLOR_DEFAULT);
   WriteChatColor("Usage:",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck on|off        - enable/disable scanning for GMs",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck chat on|off   - toggle chat channel alert",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck chattimer X   - set frequency of chat channel alert, default=15s",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck popup on|off  - toggle popup alert",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck popuptimer X  - set frequency of popup alert, default=30s",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck audio on|off  - toggle audio alert",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck testaudio     - test audio sound",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck audiotimer X  - set frequency of audio alert, default=30s",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck list          - list all known GMs currently in the zone",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck reset         - reset list of known GMs",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck history       - complete history dump",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck zone          - history of GM's in this zone",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck server        - history of GM's on this server",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck servers       - history of GM's on all servers",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck timer X       - set scan delay to X seconds, default=2s",USERCOLOR_DEFAULT);
   WriteChatColor("/gmcheck help          - show the available commands",USERCOLOR_DEFAULT);
   WriteChatColor(" ",USERCOLOR_DEFAULT);
   WriteChatColor("Datatype:",USERCOLOR_DEFAULT);
   WriteChatColor("${GMCheck} returns TRUE, FALSE, or DISABLED.",USERCOLOR_DEFAULT);
   WriteChatColor("${GMCheck.Status} returns TRUE, FALSE, or DISABLED.",USERCOLOR_DEFAULT);
   WriteChatColor("${GMCheck.Count} returns the number of GM's known in the zone.",USERCOLOR_DEFAULT);
  // WriteChatColor("",USERCOLOR_DEFAULT);
  // WriteChatColor("audio - Put an entry for GMCHECK in the MQ2Sound.ini",USERCOLOR_DEFAULT);
  // WriteChatColor(" ",USERCOLOR_DEFAULT);
  // WriteChatColor("Check to see if a GM is in the zone. This is not fool proof. It is absolutely",USERCOLOR_DEFAULT);
  // WriteChatColor("true that a GM could be right in front of you and you'd never know it. This",USERCOLOR_DEFAULT);
  // WriteChatColor("plugin will simply find those who are in the zone and not gm-invis, or who",USERCOLOR_DEFAULT);
  // WriteChatColor("just came into the zone and were not gm-invised at the time. If a GM comes",USERCOLOR_DEFAULT);
  // WriteChatColor("into the zone already gm-invised, we will not know about that.",USERCOLOR_DEFAULT);
   return;
}

void Switches(PSPAWNINFO pChar, PCHAR szLine)
{
   if (gGameState!=GAMESTATE_INGAME) return;
   char szArg1[MAX_STRING] = {0};
   char szArg2[MAX_STRING] = {0};

   GetArg(szArg1,szLine,1);

   if (strlen(szArg1)==0 || !_strnicmp(szArg1,"HELP",4)) {
      Help();
      return;
   }

   // Handle On/Off
   if (Toggle(szLine)) return;

   // This option used for testing, but might be useful
   if (!_strnicmp(szArg1,"RESET",5))
      ResetGMArray();

   // List GMs we know about!
   if (!_strnicmp(szArg1,"LIST",4))
      ListGMs();

   // List GMs history!
   if (!_strnicmp(szArg1,"THISZONE",8) || !_strnicmp(szArg1,"LASTSEEN",8) || !_strnicmp(szArg1,"ZONE",4) || !_strnicmp(szArg1,"ZONES",5))
      HistoryGMThisZone();

   // List GMs history!
   if (!_strnicmp(szArg1,"THISSERVER",10) || !_strnicmp(szArg1,"SERVER",6))
      HistoryGMThisServer();

   // List GMs history!
   if (!_strnicmp(szArg1,"SERVERS",7) || !_strnicmp(szArg1,"ALLSERVER",9) || !_strnicmp(szArg1,"ALLSERVERS",10))
      HistoryGMAllServers();

   // List GMs history!
   if (!_strnicmp(szArg1,"HISTORY",7) || !_strnicmp(szArg1,"ALL",3))
      HistoryGMs();

   // Use it all you want
   if (!_strnicmp(szArg1,"SCAN",4))
      ScanGMs();

   // Adjust the timer
   if (!_strnicmp(szLine,"CHATTIMER",9)) {
      GetArg(szArg2,szLine,2);
      gAlertChatDelay=atoi(szArg2);
      if (gAlertChatDelay<2) gAlertChatDelay=2;
      WriteChatf("\arGMCHECK: Will generate chat channel alerts every %ds!\ax",gAlertChatDelay);
      gAlertChatDelay*=1000;
      return;
   }

   // Adjust the timer
   if (!_strnicmp(szLine,"POPUPTIMER",10)) {
      GetArg(szArg2,szLine,2);
      gAlertPopupDelay=atoi(szArg2);
      if (gAlertPopupDelay<2) gAlertPopupDelay=2;
      WriteChatf("\arGMCHECK: Will generate popup alerts every %ds!\ax",gAlertPopupDelay);
      gAlertPopupDelay*=1000;
      return;
   }

   // Adjust the timer
   if (!_strnicmp(szLine,"AUDIOTIMER",10)) {
      GetArg(szArg2,szLine,2);
      gAlertAudioDelay=atoi(szArg2);
      if (gAlertAudioDelay<2) gAlertAudioDelay=2;
      WriteChatf("\arGMCHECK: Will generate audio alerts every %ds!\ax",gAlertAudioDelay);
      gAlertAudioDelay*=1000;
      return;
   }

   // Turning chat spam alert on/off
   if (!_strnicmp(szArg1,"CHAT",4)) {
      bAlertChat=!bAlertChat;
      if (bAlertChat)
         WriteChatf("\arGMCHECK: Chat channel alerting enabled!");
      else
         WriteChatf("\arGMCHECK: Chat channel alerting disabled!");
      return;
   }

   // Turning audio spam alert on/off
   if (!_strnicmp(szArg1,"AUDIO",5)) {
      bAlertAudio=!bAlertAudio;
      if (bAlertAudio)
         WriteChatf("\arGMCHECK: Audio alerting enabled!");
      else
         WriteChatf("\arGMCHECK: Audio alerting disabled!");
      return;
   }
  // Test audio alert
   if (!_strnicmp(szArg1,"TESTAUDIO",9)) {
        WriteChatf("\arGMCHECK: Testing Audio Alert!");
        PlaySound(GMSound,0,SND_ASYNC);
      return;
   }

   // Turning popup spam alert on/off
   if (!_strnicmp(szArg1,"POPUP",5)) {
      bAlertPopup=!bAlertPopup;
      if (bAlertPopup)
         WriteChatf("\arGMCHECK: Popup alerting enabled!");
      else
         WriteChatf("\arGMCHECK: Popup alerting disabled!");
      return;
   }

   // Adjust the timer
   if (!_strnicmp(szLine,"TIMER",5)) {
      GetArg(szArg2,szLine,2);
      gPulseDelay=atoi(szArg2);
      if (gPulseDelay<2) gPulseDelay=2;
      WriteChatf("\arGMCHECK: Will check for GM's every %ds!\ax",gPulseDelay);
      gPulseDelay*=1000;
   }

   return;
}

PLUGIN_API VOID SetGameState(DWORD GameState) {
    if(GameState==GAMESTATE_INGAME) {
        GetPrivateProfileString("Audio","Sound","",GMSound,2047,INIFileName);
    }
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
   DebugSpewAlways("Initializing MQ2GMCheck");
   AddCommand("/gmcheck",Switches);
   pGMCheckType = new MQ2GMCheckType;
   AddMQ2Data("GMCheck", dataGMCheck);
   ResetGMArray();
   WriteChatColor("MQ2GMCheck 2.1 - Check/Alert/History of GM's",USERCOLOR_DEFAULT);
   return;
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
   DebugSpewAlways("Shutting down MQ2GMCheck");
   RemoveMQ2Data("GMCheck");
   delete pGMCheckType;
   RemoveCommand("/gmcheck");
   return;
}

PLUGIN_API VOID OnAddSpawn(PSPAWNINFO pNewSpawn)
{
   if (gGameState!=GAMESTATE_INGAME) return;
   if (bCheck4GM && pNewSpawn->GM)
      AddGMToList(pNewSpawn->SpawnID,pNewSpawn->DisplayedName);
   return;
}

PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pOldSpawn)
{
   if (gGameState!=GAMESTATE_INGAME) return;
   if (bCheck4GM && pOldSpawn->GM)
      RemoveGMFromList(pOldSpawn->SpawnID);
   return;
}

PLUGIN_API VOID BeginZone(VOID)
{
   if (gGameState!=GAMESTATE_INGAME) return;
   bCheck4GMOldState=bCheck4GM;
   bCheck4GM=false;
   return;
}

PLUGIN_API VOID OnZoned(VOID)
{
   if (gGameState!=GAMESTATE_INGAME) return;
   ResetGMArray();
   return;
}

PLUGIN_API VOID EndZone(VOID)
{
   if (gGameState!=GAMESTATE_INGAME) return;
   // Must delay here. This is to prevent a GM from the previous zone from being
   // seen in the new zone simply because the spawn table hasn't completely flushed.
   // False positive prevention.
   if (bCheck4GMOldState) DoCommand(GetCharInfo()->pSpawn,"/timed 50 /gmcheck on");
   return;
}

PLUGIN_API VOID OnPulse(VOID)
{
   if (gGameState!=GAMESTATE_INGAME) return;

   // Scan for GMs once every gPulseDelay seconds (1000=1s, 2000=2s, etc.)
   if (gPulseTimer>GetTickCount64()) return;
   gPulseTimer=GetTickCount64()+gPulseDelay;
   ScanGMs();
   if (bGMInTheZone) ListGMs();
   return;
}
