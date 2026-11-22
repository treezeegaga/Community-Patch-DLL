#!/usr/bin/env python3
"""Recount the wars that took nothing on the 48 measuring maps (Songhai new AI, runs nx22N<seed>), from the branch.

Usage (from a clone that has fetched origin/ai/v2):  python3 ai-lab/review/recount_stalls.py [cache.json]

For every game with a war against a major and no capture of a major's city, prints Songhai's declarations, its
city-attack operations (target and owner: an owner that is not a major is a city-state), and for every city with a
siege estimate: turns, TAKE turns, turns with 5+ / 3+ of our units within 4 tiles, the peak army, the lowest hit
points. "impl" is the kind implementer 9 gave the game on 2026-11-22. Judge the kind over all cities of majors, not
only the one with the most TAKE turns (that rule misfiled 5555), and check the owner (2727 and 5858 are city-states).
"""
import subprocess, re, sys, json
REF="origin/ai/v2"; BASE="ai-lab/siege/results"
def git(*a): return subprocess.run(["git",*a],capture_output=True,text=True,errors="replace").stdout
DIRS=[l.rsplit("/",1)[-1] for l in git("ls-tree","--name-only",REF,BASE+"/").splitlines()]
def latest(label,kind):
    r=sorted(d for d in DIRS if d.startswith(label+"-") and d.endswith("-"+kind))
    return r[-1] if r else None
EST=re.compile(r"^(\d+), Songhai, siege estimate for ([^:]+): (\w+),.*?city hp (\d+) of (\d+).*?within 4: melee (\d+), ranged (\d+)")
out={}
for seed in range(1919,6667,101):
    lab=f"nx22N{seed}"; run=latest(lab,"run"); logs=latest(lab,"logs")
    g={"run":run,"logs":logs}
    if not run or not logs: out[seed]=g; continue
    lua=git("show",f"{REF}:{BASE}/{run}/Lua.log")
    majors=set(); war=None; caps=[]; ids={}; last=0
    for line in lua.splitlines():
        i=line.find("[SiegeLab] ")
        if i<0: continue
        b=line[i+11:].strip().split(",")
        if b[0]=="OPEN":
            majors.add(b[3]); ids[int(b[2])]=b[3]; last=max(last,int(b[1]))
            if b[2]=="0" and int(b[22])>0 and war is None: war=int(b[1])
        elif b[0]=="CAPTURE": caps.append(b[1:])
    g.update(war=war,caps=caps,last=last,majors=sorted(majors),ids=ids,done="[SiegeLab] DONE" in lua)
    dip=git("show",f"{REF}:{BASE}/{logs}/DiplomacyAI_Log.csv")
    g["decl"]=[(int(m.group(1)),m.group(2).strip(),m.group(3).strip()) for m in re.finditer(r"^(\d+), ([^,]+), , \*+ WAR DECLARATION! \*+, (.*)$",dip,re.M) if "Songhai" in m.group(0)]
    g["peace"]=[l[:120] for l in dip.splitlines() if "PEACE" in l.upper() and "Songhai" in l and "*****" in l][:6]
    op=git("show",f"{REF}:{BASE}/{logs}/OperationalAILog.csv")
    g["ops"]=[(int(m.group(1)),int(m.group(2)),m.group(3),m.group(4)) for m in re.finditer(r"^(\d+), Songhai, OP_CITY_ATTACK\w*, \d+, Started Army \d+; Units Recruited (\d+);.*?target city (.*?) \(owner (.*?)\)",op,re.M)]
    owner={c:o for _,_,c,o in g["ops"]}
    tac=git("show",f"{REF}:{BASE}/{logs}/PlayerTacticalAILog.csv")
    cities={}
    for line in tac.splitlines():
        m=EST.match(line)
        if not m: continue
        t=int(m.group(1)); c=m.group(2).strip()
        d=cities.setdefault(c,{}).setdefault(t,{"v":set(),"hp":10**9,"max":0,"n":0,"mel":0,"rng":0})
        d["v"].add(m.group(3)); d["hp"]=min(d["hp"],int(m.group(4))); d["max"]=int(m.group(5))
        if int(m.group(6))+int(m.group(7))>=d["n"]: d["n"]=int(m.group(6))+int(m.group(7)); d["mel"]=int(m.group(6)); d["rng"]=int(m.group(7))
    g["cities"]={c:{str(t):{**d,"v":sorted(d["v"])} for t,d in ts.items()} for c,ts in cities.items()}
    g["owner"]=owner
    # shots at cities: count Songhai ranged "can shoot it from here" per city
    out[seed]=g
    print(seed,"ok",file=sys.stderr)
CACHE=sys.argv[1] if len(sys.argv)>1 else "/tmp/recount_stalls.json"
json.dump(out,open(CACHE,"w"))
G=json.load(open(CACHE))
impl={**{s:"NO" for s in (2020,3737,5353,5858,6262)},**{s:"<90" for s in (2727,4141,4848,5454,5555,6060,6464)},**{s:"dent" for s in (2525,2929,3131,3232,3535,4343,4444,5757,5959)},**{s:"<half" for s in (3434,4545,4747,4949)}}
nwar=ncap=0
for s,g in sorted(G.items(),key=lambda x:int(x[0])):
    s=int(s)
    if not g.get("run") or "war" not in g: print(s,"MISSING",g); continue
    maj=set(g["majors"]); ids={int(k):v for k,v in g["ids"].items()}
    mcaps=[c for c in g["caps"] if int(c[2])==0 and int(c[1]) in ids]   # turn, old, new, capital
    allc=[c for c in g["caps"] if int(c[2])==0]
    if g["war"] is None: continue
    nwar+=1
    if mcaps: ncap+=1; continue
    minors={o for o in g["owner"].values() if o not in maj}|{d[2] for d in g["decl"] if d[2] not in maj}|{d[1] for d in g["decl"] if d[1] not in maj}
    print(f"\n== {s}  impl:{impl.get(s)}  war t{g['war']} last t{g['last']}  caps by Songhai (any): {[(c[0],c[1]) for c in allc]}")
    print("   decl:",[d for d in g["decl"]])
    print("   ops:",[o for o in g["ops"]])
    for c,ts in g["cities"].items():
        kind="CS" if c.replace("_"," ") in minors else ("maj" if g["owner"].get(c.replace("_"," ")) in maj else "?")
        T=sorted(int(t) for t in ts)
        take=[t for t in T if "TAKE" in ts[str(t)]["v"]]
        five=[t for t in T if ts[str(t)]["n"]>=5]
        three=[t for t in T if ts[str(t)]["n"]>=3]
        mn=min((ts[str(t)]["hp"]/ts[str(t)]["max"],t) for t in T)
        peak=max((ts[str(t)]["n"],ts[str(t)]["mel"],ts[str(t)]["rng"],t) for t in T)
        print(f"   {kind:3} {c:18} turns {T[0]}-{T[-1]} ({len(T)})  TAKE {len(take)} {('t%d-%d'%(take[0],take[-1])) if take else ''}  >=5 near: {len(five)}  >=3: {len(three)}  peak {peak[0]} ({peak[1]}m+{peak[2]}r) t{peak[3]}  min hp {mn[0]*100:.0f}% t{mn[1]} of {ts[str(T[0])]['max']}")
print("\nwars",nwar,"with major capture",ncap)
