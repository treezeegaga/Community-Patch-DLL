# Stage table for the LOG (review relay 11:10 PM Nov 21): python3 ai-lab/runs/stages.py <label>:<Civ_Name> ...
import sys,glob,re,os
B="/home/user/Community-Patch-DLL/ai-lab/siege/results/"
def last(pat):
    g=sorted(glob.glob(B+pat)); return g[-1] if g else None
def rd(p):
    try: return open(p,errors="replace").read().splitlines()
    except: return []
def t(line):
    m=re.match(r"\s*0*(\d+),",line); return int(m.group(1)) if m else None
def stages(label,civ):
    R=last(label+"-*-run"); D=last(label+"-*-logs")
    if not R or not D: return None
    lua=rd(R+"/Lua.log")
    majors=set()
    for l in lua:
        if "STARTS," in l:
            for x in l.split("STARTS,")[1].split(",",1)[1].split(";"): majors.add(x.split(":")[0].strip().replace(" ","_"))
    civk=civ.replace("_"," ")
    rows=[]
    for l in lua:
        if "OPEN," in l:
            f=l.split("OPEN,")[1].split(",")
            if f[2]==civk or f[2]==civ: rows.append((int(f[0]),int(f[13]),int(f[14])))
    S={}
    if rows:
        base=rows[0][1]+rows[0][2]
        S["a"]=next((r[0] for r in rows if r[1]+r[2]>base),None)
        S["c"]=next((r[0] for r in rows if r[1]>=6 and r[2]>=4),None)
        S["end"]=rows[-1][0]
    for l in rd(D+"/TechAILog.csv"):
        if f", {civ}, COMPLETED, TECH_CONSTRUCTION" in l or f", {civk}, COMPLETED, TECH_CONSTRUCTION" in l: S["b"]=t(l); break
    mil=rd(D+"/MilitaryAILog.csv")
    ups=[t(l) for l in mil if (f", {civ}, plan upgrade:" in l or f", {civk}, plan upgrade:" in l)]
    S["d"]=max(ups) if ups else None; S["d1"]=min(ups) if ups else None
    def ismaj(n): return n.strip().replace(" ","_") in majors
    for l in rd(D+"/OperationalAILog.csv"):
        if re.search(rf", ({civ}|{civk}), OP_CITY_ATTACK\w*, \d+, Started Army",l):
            m=re.search(r"owner ([^)]+)\)",l)
            if m and ismaj(m.group(1)): S["e"]=t(l); break
    for l in mil:
        m=re.match(rf"\s*0*(\d+), ({civ}|{civk}), Declared War on: (.+?)\s*$",l)
        if m and ismaj(m.group(3)): S["f"]=int(m.group(1)); break
        m=re.match(r"\s*0*(\d+), ([^,]+), Declared War on: (.+?)\s*$",l)
        if m and m.group(3).strip() in (civ,civk) and ismaj(m.group(2)): S["f"]=int(m.group(1)); S["fby"]=m.group(2); break
    for l in mil:
        m=re.match(rf"\s*0*(\d+), ({civ}|{civk}), Captured: ([^,]+), From: ([^,]+)",l)
        if m and ismaj(m.group(4)): S["i"]=int(m.group(1)); S["icity"]=m.group(3); break
    tac=rd(D+"/PlayerTacticalAILog.csv")
    for l in tac:
        if not re.search(rf", ({civ}|{civk}), siege estimate for",l): continue
        if "fi" in S and t(l)>S["fi"]: pass
        m=re.search(r"city hp (\d+) of (\d+).*within 4: melee (\d+), ranged (\d+)",l)
        if not m: continue
        hp,mx,me,ra=map(int,m.groups())
        if "f" not in S or t(l)<S["f"]: continue
        if "g" not in S and me+ra>=5: S["g"]=t(l)
        if "h" not in S and hp*2<mx: S["h"]=t(l)
    S["fby"]=S.get("fby","")
    return S
def fmt(v): return "-" if v is None else str(v)
if __name__=="__main__":
    for arg in sys.argv[1:]:
        label,civ=arg.split(":")
        S=stages(label,civ)
        if S is None: print(label,"missing"); continue
        print(label,civ,{k:S.get(k) for k in ["a","b","c","d1","d","e","f","fby","g","h","i","icity","end"]})
