# Measuring maps map for map: m25N (it. 26 + 25) against nx22N (it. 22), Songhai new AI to t100. Run from ai-lab/siege/results; change the prefixes for a new build.
import glob,re
def row(pre,seed):
    g=sorted(glob.glob('%s%d-*-logs'%(pre,seed)))
    if not g: return None
    L=[l.split('[SiegeLab] ',1)[1].strip().split(',') for l in open(g[-1]+'/Lua.log',errors='replace') if '[SiegeLab] ' in l]
    op=[f for f in L if f[0]=='OPEN' and len(f)==24]
    if not op: return None
    act=[int(f[2]) for f in op if f[3]=='Songhai'][0]
    mine=[f for f in op if int(f[2])==act]
    army=next((int(f[1]) for f in mine if int(f[14])>=6 and int(f[15])>=4),None)
    war=next((int(f[1]) for f in mine if int(f[22])>0),None)
    caps=[f for f in L if f[0]=='CAPTURE' and int(f[3])==act and int(f[2])<22]
    first=min((int(c[1]) for c in caps),default=None)
    capital=min((int(c[1]) for c in caps if c[4]=='1'),default=None)
    f=([x for x in mine if int(x[1])==100] or mine[-1:])[0]
    return dict(army=army,war=war,first=first,capital=capital,n=len(caps),cities=int(f[5]),pop=int(f[6]),techs=int(f[7]),sci=float(f[11]),gold=int(f[9]),end=int(f[1]))
seeds=range(1919,6667,101); A=[];B=[]
def s(v): return '-' if v is None else str(v)
for sd in seeds:
    a,b=row('nx22N',sd),row('m25N',sd)
    if not b: continue
    A.append(a);B.append(b)
    tag=''
    if a['first'] and not b['first']: tag='LOST'
    elif b['first'] and not a['first']: tag='GAINED'
    print(sd,'old war %s cap %s capital %s sci %.0f pop %d | new war %s cap %s capital %s sci %.0f pop %d %s'%(s(a['war']),s(a['first']),s(a['capital']),a['sci'],a['pop'],s(b['war']),s(b['first']),s(b['capital']),b['sci'],b['pop'],tag))
def summ(R,name):
    n=len(R)
    print(name,n,'maps: capture',sum(1 for r in R if r['first']),'capital',sum(1 for r in R if r['capital']),'capital<=60',sum(1 for r in R if r['capital'] and r['capital']<=60),
      'war',sum(1 for r in R if r['war']),'army',sum(1 for r in R if r['army']),'sci %.1f pop %.1f cities %.2f gold %.0f'%(sum(r['sci'] for r in R)/n,sum(r['pop'] for r in R)/n,sum(r['cities'] for r in R)/n,sum(r['gold'] for r in R)/n))
summ(A,'it22');summ(B,'it26+25')
