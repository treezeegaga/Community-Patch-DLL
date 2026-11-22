# Dashboard data: one row per measuring map and AI (Songhai new AI nx22N*, stock stx22S*) from Lua.log; run from ai-lab/siege/results. Prefixes are the build of it. 22.
import glob,re,csv,sys
out=csv.writer(open('/tmp/claude-0/dashdata/full_games.csv','w',newline=''))
out.writerow(['Seed','AI','ArmyTurn','WarTurn','FirstCaptureTurn','CapitalTurn','Captures','Cities100','Pop100','Techs100','Science100','Gold100'])
for arm,pre in (('New AI','nx22N'),('Stock','stx22S')):
    for d in sorted(glob.glob(pre+'*-logs')):
        seed=int(re.match(pre+r'(\d+)',d).group(1))
        L=[l.split('[SiegeLab] ',1)[1].strip().split(',') for l in open(d+'/Lua.log',errors='replace') if '[SiegeLab] ' in l]
        op=[f for f in L if f[0]=='OPEN' and len(f)==24]
        act=[int(f[2]) for f in op if f[3]=='Songhai'][0]
        mine=[f for f in op if int(f[2])==act]
        army=next((int(f[1]) for f in mine if int(f[14])>=6 and int(f[15])>=4),'')
        war=next((int(f[1]) for f in mine if int(f[22])>0),'')
        caps=[f for f in L if f[0]=='CAPTURE' and int(f[3])==act and int(f[2])<22]
        first=min((int(c[1]) for c in caps),default='')
        capital=min((int(c[1]) for c in caps if c[4]=='1'),default='')
        last=[f for f in mine if int(f[1])==100] or mine[-1:]
        f=last[0]
        out.writerow([seed,arm,army,war,first,capital,len(caps),f[5],f[6],f[7],f[11],f[9]])
