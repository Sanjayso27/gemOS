#include "wc.h"


extern struct team teams[NUM_TEAMS];
extern int test;
extern int finalTeam1;
extern int finalTeam2;

int processType = HOST;
const char *team_names[] = {
  "India", "Australia", "New Zealand", "Sri Lanka",   // Group A
  "Pakistan", "South Africa", "England", "Bangladesh" // Group B
};


void teamPlay(void)
{
  char name[40],buf[1000];
  strcpy(name,team_names[processType]);
  char path[1000]; path[0]='\0';
  strcat(path,"./test/");
  char num[10];
  sprintf(num,"%d",test);
  strcat(path,num);
  strcat(path,"/inp/");
  strcat(path,name);
  char dig[2];
  int src_fd=open(path,O_RDONLY);
  //printf("%d\n",processType);
  while(1){
    read(teams[processType].commpipe[0],buf,2);
    if(strcmp(buf,"-1")==0){
      exit(-1);
    }
    read(src_fd,buf,1);
    dig[0]=buf[0];
    write(teams[processType].matchpipe[1],dig,1);
  }
  //printf("%d\n",processType);
}

void endTeam(int teamID)
{
  char *buf="-1";
  write(teams[teamID].commpipe[1],buf,2);
}

int match(int team1, int team2)
{
  char dig1[2],dig2[2];
  int r;
  //printf("Yo");
  write(teams[team1].commpipe[1],"ok",2);
  r=read(teams[team1].matchpipe[0],dig1,1);
  if(r==-1){
    printf("read error\n");
    exit(-1);
  }
  write(teams[team2].commpipe[1],"ok",2);
  r=read(teams[team2].matchpipe[0],dig2,1);
  if(r==-1){
    printf("read error\n");
    exit(-1);
  }
  int t1=dig1[0]-'0',t2=dig2[0]-'0';
  //dig[1]='\0',dig[2]
  //printf("Match %d:%d , t1: %d, t2: %d",team1,team2,t1,t2);
  if((t1+t2)%2==0){
    int temp=team2;
    team2=team1;
    team1=temp;
  }
  char name[40];
  strcpy(name,team_names[team1]);
  strcat(name,"v");
  strcat(name,team_names[team2]);
  if((finalTeam1==team1 || finalTeam1==team2) && 
        ((finalTeam2==team1 || finalTeam2==team2))){
          strcat(name,"-Final");
  }
  char path[1000]; path[0]='\0';
  strcat(path,"./test/");
  char num[10];
  sprintf(num,"%d",test);
  strcat(path,num);
  strcat(path,"/out/");
  strcat(path,name);
  int dst_fd=open(path,O_CREAT|O_WRONLY,0777);
  char buf[1000];
  buf[0]='\0';
  strcat(buf,"Innings1: ");
  strcat(buf,team_names[team1]);
  strcat(buf," bats\n");
  //printf("Match %d:%d , %s",team1,team2,buf);
  write(dst_fd,buf,strlen(buf));
  int player=1,run=0,tot1=0,tot2=0;
  char score[11];
  for(int i=0;i<120;i++){
    if(player>10)break;
    write(teams[team1].commpipe[1],"ok",2);
    write(teams[team2].commpipe[1],"ok",2);
    read(teams[team1].matchpipe[0],dig1,1);
    read(teams[team2].matchpipe[0],dig2,1);
    t1=dig1[0]-'0',t2=dig2[0]-'0';
    if(t1!=t2){
      run+=t1;
    }
    else {
      if(player<10){
        buf[0]=player+'0';
        buf[1]=':';
        buf[2]='\0';
        sprintf(score,"%d",run);
        strcat(buf,score);
        strcat(buf,"\n");
        write(dst_fd,buf,strlen(buf));
      }
      else {
        buf[0]='1';buf[1]='0';
        buf[2]=':';
        buf[3]='\0';
        sprintf(score,"%d",run);
        strcat(buf,score);
        strcat(buf,"\n");
        write(dst_fd,buf,strlen(buf));
      }
      tot1+=run;
      run=0;player++;
    }
  }
  if(player<=10){
    sprintf(score,"%d",player);
    buf[0]='\0';
    strcat(buf,score);
    strcat(buf,":");
    sprintf(score,"%d",run);
    strcat(buf,score);
    strcat(buf,"*\n");
    write(dst_fd,buf,strlen(buf));
    tot1+=run;
  }
  sprintf(score,"%d",tot1);
  buf[0]='\0';
  strcat(buf,team_names[team1]);
  strcat(buf," TOTAL: ");
  strcat(buf,score);
  strcat(buf,"\n");
  write(dst_fd,buf,strlen(buf));
  // 2nd innings
  buf[0]='\0';
  strcat(buf,"Innings2: ");
  strcat(buf,team_names[team2]);
  strcat(buf," bats\n");
  write(dst_fd,buf,strlen(buf));
  player=1,run=0;
  for(int i=0;i<120;i++){
    if(player>10)break;
    write(teams[team1].commpipe[1],"ok",2);
    write(teams[team2].commpipe[1],"ok",2);
    read(teams[team1].matchpipe[0],dig1,1);
    read(teams[team2].matchpipe[0],dig2,1);
    t1=dig1[0]-'0',t2=dig2[0]-'0';
    if(t1!=t2){
      run+=t2;
      tot2+=t2;
      if(tot2>tot1){
        break;
      }
    }
    else {
      if(player<10){
        buf[0]='0'+player;
        buf[1]=':';
        buf[2]='\0';
        sprintf(score,"%d",run);
        strcat(buf,score);
        strcat(buf,"\n");
        write(dst_fd,buf,strlen(buf));
      }
      else {
        buf[0]='1';buf[1]='0';
        buf[2]=':';
        buf[3]='\0';
        sprintf(score,"%d",run);
        strcat(buf,score);
        strcat(buf,"\n");
        write(dst_fd,buf,strlen(buf));
      }
      run=0;player++;
    }
  }
  if(player<=10){
    sprintf(score,"%d",player);
    buf[0]='\0';
    strcat(buf,score);
    strcat(buf,":");
    sprintf(score,"%d",run);
    strcat(buf,score);
    strcat(buf,"*\n");
    write(dst_fd,buf,strlen(buf));
  }
  sprintf(score,"%d",tot2);
  buf[0]='\0';
  strcat(buf,team_names[team2]);
  strcat(buf," TOTAL: ");
  strcat(buf,score);
  strcat(buf,"\n");
  write(dst_fd,buf,strlen(buf));
  if(tot2>tot1){
    buf[0]='\0';
    strcat(buf,team_names[team2]);
    strcat(buf," beats ");
    strcat(buf,team_names[team1]);
    strcat(buf," by ");
    int wick=11-player;
    sprintf(score,"%d",wick);
    strcat(buf,score);
    strcat(buf," wickets");
    write(dst_fd,buf,strlen(buf));
    close(dst_fd);
    return team2;
  }
  else if(tot1>tot2){
    buf[0]='\0';
    strcat(buf,team_names[team1]);
    strcat(buf," beats ");
    strcat(buf,team_names[team2]);
    strcat(buf," by ");
    run=tot1-tot2;
    sprintf(score,"%d",run);
    strcat(buf,score);
    strcat(buf," runs");
    write(dst_fd,buf,strlen(buf));
    close(dst_fd);
    return team1;
  }
  else {
    buf[0]='\0';
    strcat(buf,"TIE: ");
    if(team1>team2){
      int temp=team2;
      team2=team1;
      team1=temp;
    }
    strcat(buf,team_names[team1]);
    strcat(buf," beats ");
    strcat(buf,team_names[team2]);
    write(dst_fd,buf,strlen(buf));
    close(dst_fd);
    return team1;
  }
  return 0;
}

void spawnTeams(void)
{
  int pid;
  for(int i=0;i<NUM_TEAMS;i++){
    strcpy(teams[i].name,team_names[i]);
    pipe(teams[i].matchpipe);
    pipe(teams[i].commpipe);
    pid=fork();
    if(pid==0){
      processType=i;
      teamPlay();
    }
  }
}

void conductGroupMatches(void)
{
  int pid,i,j;
  char buf[1000],temp[11];
  int groupA[2];
  pipe(groupA);
  pid=fork();
  if(pid==0){
    close(groupA[0]);
    //printf("GroupA: \n");
    int win[4],lead,mx=0;
    for(i=0;i<4;i++)win[i]=0;
    for(i=0;i<4;i++){
      for(j=i+1;j<4;j++){
        // printf("\nMatch %d:%d\n",i,j);
        win[match(i,j)]++;
      }
    }
    for(i=0;i<4;i++){
      if(win[i]>mx){
        mx=win[i];
        lead=i;
      }
    }
    for(int i=0;i<4;i++){
      if(i!=lead){
        endTeam(i);
      }
    }
    sprintf(temp,"%d",lead);
    //printf("%s",temp);
    write(groupA[1],temp,2);
    exit(-1);
  }
  int groupB[2];
  pipe(groupB);
  pid=fork();
  if(pid==0){
    close(groupB[0]);
    //printf("GroupB: \n");
    int win[4],lead,mx=0;
    for(i=0;i<4;i++)win[i]=0;
    for(i=0;i<4;i++){
      for(j=i+1;j<4;j++){
        //printf("Match %d:%d\n",4+i,4+j);
        win[match(4+i,4+j)-4]++;
      }
    }
    for(i=0;i<4;i++){
      if(win[i]>mx){
        mx=win[i];
        lead=i;
      }
    }
    for(int i=0;i<4;i++){
      if(i!=lead){
        endTeam(i+4);
      }
    }
    lead+=4;
    sprintf(temp,"%d",lead);
    //printf("%s",temp);
    write(groupB[1],temp,2);
    exit(-1);
  }
  close(groupA[1]);
  close(groupB[1]);
  read(groupA[0],buf,1);
  buf[1]='\0';
  finalTeam1=atoi(buf);
  read(groupB[0],buf,1);
  buf[1]='\0';
  finalTeam2=atoi(buf);
}
