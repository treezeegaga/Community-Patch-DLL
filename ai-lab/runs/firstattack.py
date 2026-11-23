# First attack on a major (review relay 5:23 PM Nov 23): python3 ai-lab/runs/firstattack.py <label> ...
# turn the operation started, units at start and at the march (Composite Bowmen), turn of war, what happened, capture
import sys,re,os
sys.path.insert(0,os.path.dirname(__file__))
import stages
B=stages.B
def first(label,civ="Songhai"):
    S=stages.stages(label,civ)
    if S is None: return None
    D=stages.last(label+"-*-logs"); R=stages.last(label+"-*-run")
    majors=set()
    for l in stages.rd(R+"/Lua.log"):
        if "STARTS," in l:
            for x in l.split("STARTS,")[1].split(",",1)[1].split(";"): majors.add(x.split(":")[0].strip())
    ops=stages.rd(D+"/OperationalAILog.csv")
    op=None
    for l in ops:
        m=re.match(rf"\s*0*(\d+), {civ}, (OP_CITY_ATTACK\w*), (\d+), Started Army \d+; Units Recruited (\d+).*target city ([^(]+)\(owner ([^)]+)\)",l)
        if m and m.group(6).strip() in majors: op=m.groups(); break
    if not op: return dict(label=label,start=None,war=S.get("f"),cap=S.get("i"))
    t0,kind,oid,nrec,city,owner=op
    added=[];march=None;cbm=None;end=None;start_cb=0
    for l in ops:
        m=re.match(rf"\s*0*(\d+), {civ}, \w+, {oid}, (.*)",l)
        if not m: continue
        t,msg=int(m.group(1)),m.group(2)
        a=re.match(r"Added (.+?) \d+ to slot",msg)
        if a:
            added.append((t,a.group(1)))
        if msg.startswith("Transition to movement stage") and march is None:
            march=(t,len(added),sum(1 for x in added if x[1]=="Composite Bowman"))
        if msg.startswith("Ended:") and end is None: end=(t,msg.split(";")[0][7:])
        if msg.startswith("Transition to finished stage") and end is None: end=(t,"arrived")
    start=[x for x in added if x[0]==int(t0)]
    #at the city: over the 15 turns after the war, most of our units within 4 tiles and the city's lowest hp (siege estimates)
    near=0;low=None;w=S.get("f")
    if w:
        for l in stages.rd(D+"/PlayerTacticalAILog.csv"):
            m=re.match(rf"\s*0*(\d+), {civ}, siege estimate for {re.escape(city.strip())}: .*city hp (\d+) of (\d+).*within 4: melee (\d+), ranged (\d+)",l)
            if m and w<=int(m.group(1))<=w+15:
                near=max(near,int(m.group(4))+int(m.group(5))); pc=100*int(m.group(2))//int(m.group(3)); low=pc if low is None else min(low,pc)
    return dict(label=label,start=int(t0),city=city.strip(),n0=len(start),cb0=sum(1 for x in start if x[1]=="Composite Bowman"),
        march=march,near=near,low=low,war=S.get("f"),end=end,cap=S.get("i"),capcity=S.get("icity"))
if __name__=="__main__":
    for lab in sys.argv[1:]:
        r=first(lab)
        if r is None: print(lab,"missing"); continue
        if r["start"] is None: print(f"{lab:10} no attack on a major; war {r['war']} capture {r['cap']}"); continue
        mt=f"t{r['march'][0]} {r['march'][1]} ({r['march'][2]} CB)" if r['march'] else "never marched"
        print(f"{lab:10} start t{r['start']} {r['city']}: {r['n0']} units ({r['cb0']} CB); march {mt}; war t{r['war']}; op end {r['end']}; at {r['city']} (15 turns from war) most units within 4: {r['near']}, lowest hp {r['low']}%; capture {r['cap']} {r['capcity'] or ''}")
